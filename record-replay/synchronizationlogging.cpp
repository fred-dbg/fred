/****************************************************************************
 * Copyright (C) 2009, 2010, 2011, 2012 by Kapil Arya, Gene Cooperman,      *
 *                                     Tyler Denniston, and Ana-Maria Visan *
 * {kapil,gene,tyler,amvisan}@ccs.neu.edu                                   *
 *                                                                          *
 * This file is part of FReD.                                               *
 *                                                                          *
 * FReD is free software: you can redistribute it and/or modify             *
 * it under the terms of the GNU General Public License as published by     *
 * the Free Software Foundation, either version 3 of the License, or        *
 * (at your option) any later version.                                      *
 *                                                                          *
 * FReD is distributed in the hope that it will be useful,                  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 * GNU General Public License for more details.                             *
 *                                                                          *
 * You should have received a copy of the GNU General Public License        *
 * along with FReD.  If not, see <http://www.gnu.org/licenses/>.            *
 ****************************************************************************/

#include <errno.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <algorithm>
#include "fred_wrappers.h"
#include "dmtcpmodule.h"
#include "util.h"
#include  "jassert.h"
#include  "jtimer.h"
#include  "jfilesystem.h"
#include <sys/select.h>
#include "synchronizationlogging.h"
#include "log.h"
#include <sys/resource.h>


// TODO: Do we need LIB_PRIVATE again here if we had already specified it in
// the header file?
/* Library private: */
LIB_PRIVATE dmtcp::map<clone_id_t, pthread_t> *clone_id_to_tid_table = NULL;
LIB_PRIVATE dmtcp::map<pthread_t, clone_id_t> *tid_to_clone_id_table = NULL;
LIB_PRIVATE dmtcp::map<pthread_t, pthread_join_retval_t> pthread_join_retvals;
LIB_PRIVATE char RECORD_LOG_PATH[RECORD_LOG_PATH_MAX];
LIB_PRIVATE char RECORD_READ_DATA_LOG_PATH[RECORD_LOG_PATH_MAX];
LIB_PRIVATE int             read_data_fd = -1;
LIB_PRIVATE int             sync_logging_branch = 0;

/* Setting this will log/replay *ALL* malloc family
   functions (i.e. including ones from DMTCP, std C++ lib, etc.). */
LIB_PRIVATE int             log_all_allocs = 0;
LIB_PRIVATE pthread_cond_t  reap_cv = PTHREAD_COND_INITIALIZER;
LIB_PRIVATE pthread_mutex_t global_clone_counter_mutex = PTHREAD_MUTEX_INITIALIZER;
LIB_PRIVATE pthread_mutex_t reap_mutex = PTHREAD_MUTEX_INITIALIZER;
LIB_PRIVATE pthread_t       thread_to_reap;

LIB_PRIVATE void           *stack_base_addr = NULL;


LIB_PRIVATE dmtcp::SynchronizationLog global_log;

/* Thread locals: */
LIB_PRIVATE __thread clone_id_t my_clone_id = -1;
LIB_PRIVATE __thread int in_mmap_wrapper = 0;
LIB_PRIVATE __thread unsigned char isOptionalEvent = 0;


/* Volatiles: */
LIB_PRIVATE volatile clone_id_t global_clone_counter = 0;
LIB_PRIVATE volatile off_t         read_log_pos = 0;

static inline void memfence() {  asm volatile ("mfence" ::: "memory"); }

# ifndef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
// gcc-4.1 and later has __sync_fetch_and_add, __sync_fetch_and_xor, etc.
// We need it for atomic_increment and atomic_decrement
// The version below is slow, but works.  It uses GNU extensions.

static log_mutex_t sync_fetch_mutex = PTHREAD_MUTEX_INITIALIZER;

#define __sync_fetch_and_add(ptr,val) \
  ({ __typeof__(*(ptr)) tmp; \
    _real_pthread_mutex_lock(&sync_fetch_mutex); \
    tmp = *(ptr); *(ptr) += (val); \
    _real_pthread_mutex_unlock(&sync_fetch_mutex); \
    tmp; \
  })
#define __sync_fetch_and_xor(ptr,val) \
  ({ __typeof__(*(ptr)) tmp; \
    _real_pthread_mutex_lock(&sync_fetch_mutex); \
    tmp = *(ptr); *(ptr) ^= (val); \
    _real_pthread_mutex_unlock(&sync_fetch_mutex); \
    tmp; \
  })
#warning **********************************************************************
#warning * __sync_fetch_and_add not supported -- This will execute more slowly.
#warning * THIS CODE HAS NOT YET BEEN TESTED.  BEWARE.
#warning **********************************************************************
// Alternatively, we could copy and adjust some assembly language that we
// generate elsewhere.
# endif

/* Switch record/replay to specified mode. mode should be one of:
   SYNC_NOOP, SYNC_RECORD, SYNC_REPLAY. */
inline void set_sync_mode(int mode)
{
  char *x = getenv(ENV_VAR_LOG_REPLAY);
  // Don't call setenv() to avoid hidden malloc()
  if (mode == SYNC_NOOP) {
    x[0] = '0';
  } else if (mode == SYNC_RECORD) {
    x[0] = '1';
  } else if (mode == SYNC_REPLAY) {
    x[0] = '2';
  } else {
    JASSERT ( false ) ( mode ).Text("Invalid mode request.");
  }
  x[1] = '\0';
  sync_logging_branch = mode;
}

static inline clone_id_t get_next_clone_id()
{
  return __sync_fetch_and_add (&global_clone_counter, 1);
}

int shouldSynchronize(void *return_addr)
{
  // Returns 1 if we should synchronize this call, instead of calling _real
  // version. 0 if we should not.
  if (!dmtcp_is_running_state()) {
    return 0;
  }
  if (SYNC_IS_NOOP) {
    return 0;
  }
  if (isProcessGDB()) {
    return 0;
  }
  if (!validAddress(return_addr)) {
    return 0;
  }
  return 1;
}


/* Initializes log pathnames. One log per process. */
void initializeLogNames()
{
  pid_t pid = getpid();
  dmtcp::string tmpdir = dmtcp_get_tmpdir();
  snprintf(RECORD_LOG_PATH, RECORD_LOG_PATH_MAX,
      "%s/synchronization-log-%d", tmpdir.c_str(), pid);
  snprintf(RECORD_READ_DATA_LOG_PATH, RECORD_LOG_PATH_MAX,
      "%s/synchronization-read-log-%d", tmpdir.c_str(), pid);
}

/* Truncate all logs to their current positions. */
void truncate_all_logs()
{
  JASSERT ( SYNC_IS_REPLAY );
  if (global_log.isMappedIn()) {
    global_log.truncate();
  }
}

/* Unmap all open logs, if any are in memory. Return whether any were
   unmapped. */
bool close_all_logs()
{
  if (global_log.isMappedIn()) {
    global_log.destroy();
    return true;
  }
  return false;
}

void initLogsForRecordReplay()
{
  global_log.initialize(RECORD_LOG_PATH, MAX_LOG_LENGTH);
}


#define MAX_PROC_MAPS_AREAS 1024
LIB_PRIVATE pthread_mutex_t procMapsAreasMutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct ProcMapsAreaInfo {
  dmtcp::Util::ProcMapsArea area;
  bool                      shouldLog;
} ProcMapsAreaInfo;

static ProcMapsAreaInfo procMapsAreaInfo[MAX_PROC_MAPS_AREAS];
static size_t procMapsAreaInfoLen = 0;

/* Specify the patterns that you wish to log. The current logic uses
 * strStartsWith() and so add accordingly.
 * The current Approach is _WHITE-LIST_ based, we might want to switch to
 * _BLACK-LIST_ based approach here."
 */
static const char *logLibraryPattern[] = {
  "/usr/lib64/firefox",
  "/usr/lib64/libxcb",
  "/usr/lib64/libX11",
  "/usr/lib64/libg",
  "/usr/lib64/libgnome",
  "/usr/lib64/libgconf",
  "/usr/lib64/libICE",
  "/usr/lib64/libSM",
  "/lib64/libglib",
  "/lib64/libgio",
  "/lib64/libgthread",
  "/lib64/libgobject",
  "/lib64/libgmodule",
  "[stack]"
};

static bool shouldLogArea(char *area_name)
{
  dmtcp::string progpath = jalib::Filesystem::GetProgramPath();
  int n = sizeof(logLibraryPattern) / sizeof(char*);

  if (dmtcp::Util::strStartsWith(area_name, progpath.c_str())) {
    return true;
  }
  for (int i = 0; i < n; i++) {
    if (dmtcp::Util::strStartsWith(area_name, logLibraryPattern[i])) {
      return true;
    }
  }
  return false;
}

static bool compareProcMemAreaInfo(ProcMapsAreaInfo info1,
                                   ProcMapsAreaInfo info2) {
  return info1.area.addr < info2.area.addr;
}

static void sortProcMapsAreaInfo() {
  std::sort(procMapsAreaInfo, procMapsAreaInfo + procMapsAreaInfoLen,
       compareProcMemAreaInfo);
}

static ProcMapsAreaInfo *searchAddrInProcMapsAreaInfo(void *addr) {
  int min = 0;
  int max = procMapsAreaInfoLen - 1;
  if (procMapsAreaInfo[max].area.endAddr <= addr) {
    return NULL;
  }

  while (max >= min) {
    int mid = (min + max) / 2;
    if (procMapsAreaInfo[mid].area.endAddr <= addr) {
      min = mid + 1;
    } else {
      if (addr >= procMapsAreaInfo[mid].area.addr) {
        return &procMapsAreaInfo[mid];
      }
      max = mid - 1;
    }
  }

  return NULL;
}

static void recordMemArea(dmtcp::Util::ProcMapsArea& area)
{
  JASSERT(procMapsAreaInfoLen < MAX_PROC_MAPS_AREAS);
  procMapsAreaInfo[procMapsAreaInfoLen].area = area;
  procMapsAreaInfo[procMapsAreaInfoLen].shouldLog = shouldLogArea(area.name);
  procMapsAreaInfoLen++;
}

static void analyzeAndRecordAddress(void *addr)
{
  int mapsFd = -1;
  dmtcp::Util::ProcMapsArea area;

  if ((mapsFd = _real_open("/proc/self/maps", O_RDONLY, S_IRUSR)) == -1) {
    perror("open");
    exit(1);
  }

  // There is a potential race in calling analyzeAndRecordAddress(). Although
  // the data structures are protected by a mutex, the acquisition order is
  // not guaranteed at REPLAY.
  _real_pthread_mutex_lock(&procMapsAreasMutex);
  while (dmtcp::Util::readProcMapsLine(mapsFd, &area)) {
    if (addr >= area.addr && addr < area.endAddr) {
      JASSERT((area.prot & PROT_EXEC) != 0) // && strlen(area.name) != 0)
        (area.name) (area.addr) (area.prot);
      recordMemArea(area);
      break;
    }
  }
  sortProcMapsAreaInfo();
  _real_pthread_mutex_unlock(&procMapsAreasMutex);
  _real_close(mapsFd);
}

LIB_PRIVATE
void initSyncAddresses()
{
  int mapsFd = -1;
  dmtcp::Util::ProcMapsArea area;

  if (isProcessGDB()) {
    return;
  }
  if ((mapsFd = _real_open("/proc/self/maps", O_RDONLY, S_IRUSR)) == -1) {
    perror("open");
    exit(1);
  }

  _real_pthread_mutex_lock(&procMapsAreasMutex);
  procMapsAreaInfoLen = 0;

  while (dmtcp::Util::readProcMapsLine(mapsFd, &area)) {
    /* Initialize the stack_base addr. The label '[stack]' might be missing
     * after restart and so we can't rely on /proc/self/maps to provide us with
     * the correct stack bounds.
     * Good news is that the stack_base_addr will not change in _most_ sane programs.
     *
     * FIXME: On some systems, stack might not be labelled as "[stack]".
     * Instead, use environ[NN] to find an address in the stack and then use
     * /proc/self/maps to find the mmap() location within which this address
     * falls.
     */
    if (stack_base_addr == NULL) {
      if (area.name == "[stack]") {
        stack_base_addr == area.endAddr;
      }
    }

    if (area.endAddr == stack_base_addr) {
      strcpy(area.name, "[stack]");
    }

    if ((area.prot & PROT_EXEC) != 0  && strlen(area.name) != 0) {
      recordMemArea(area);
    }
  }
  _real_pthread_mutex_unlock(&procMapsAreasMutex);
  _real_close(mapsFd);
}

int validAddress(void *addr)
{
  ProcMapsAreaInfo *info;
  _real_pthread_mutex_unlock(&procMapsAreasMutex);
  info = searchAddrInProcMapsAreaInfo(addr);
  if (info == NULL) {
    analyzeAndRecordAddress(addr);
    info = searchAddrInProcMapsAreaInfo(addr);
  }
  JASSERT(info != NULL);
  int result = info->shouldLog;
  _real_pthread_mutex_unlock(&procMapsAreasMutex);

  return result;
}

void copyFdSet(fd_set *src, fd_set *dest)
{
  // fd_set struct has one member: __fds_bits. which is an array of longs.
  // length of that array is FD_SETSIZE/NFDBITS
  // see /usr/include/sys/select.h
  if (src == NULL)
    dest = NULL;
  if (dest == NULL)
    return;
  int length_fd_bits = FD_SETSIZE/NFDBITS;
  int i = 0;
  for (i = 0; i < length_fd_bits; i++) {
    __FDS_BITS(dest)[i] = __FDS_BITS(src)[i];
  }
}

void addNextLogEntry(log_entry_t& e)
{
  if (GET_COMMON(e, log_offset) == INVALID_LOG_OFFSET) {
    global_log.appendEntry(e);
  } else {
    global_log.updateEntry(e);
  }
}

void getNextLogEntry()
{
  // If log is empty, don't do anything
  if (global_log.numEntries() == 0) {
    return;
  }
  if (global_log.advanceToNextEntry() == 0) {
    JTRACE ( "Switching back to record." );
    set_sync_mode(SYNC_RECORD);
  }
}

void logReadData(void *buf, int count)
{
  if (SYNC_IS_REPLAY) {
    JASSERT (false).Text("Asked to log read data while in replay. "
        "This is probably not intended.");
  }
  if (read_data_fd == -1) {
    int fd = _real_open(RECORD_READ_DATA_LOG_PATH,
        O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    read_data_fd = _real_dup2(fd, dmtcp_get_readlog_fd());
    _real_close(fd);
  }
  int written = _real_write(read_data_fd, buf, count);
  JASSERT ( written == count );
  read_log_pos += written;
}

static void setupCommonFields(log_entry_t *e, clone_id_t clone_id, event_code_t event)
{
  SET_COMMON_PTR(e, clone_id);
  SET_COMMON_PTR(e, event);
  // Zero out all other fields:
  // FIXME: Shouldn't we replace the memset with a simpler SET_COMMON_PTR()?
  SET_COMMON_PTR2(e, log_offset, INVALID_LOG_OFFSET);
  memset(&(GET_COMMON_PTR(e, my_errno)), 0, sizeof(GET_COMMON_PTR(e, my_errno)));
  memset(&(GET_COMMON_PTR(e, retval)), 0, sizeof(GET_COMMON_PTR(e, retval)));
}

log_entry_t create_accept_entry(clone_id_t clone_id, event_code_t event, int sockfd,
                                sockaddr *addr, socklen_t *addrlen)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, accept, sockfd);
  SET_FIELD(e, accept, addr);
  SET_FIELD(e, accept, addrlen);
  return e;
}

log_entry_t create_accept4_entry(clone_id_t clone_id, event_code_t event, int sockfd,
                                sockaddr *addr, socklen_t *addrlen, int flags)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, accept4, sockfd);
  SET_FIELD(e, accept4, addr);
  SET_FIELD(e, accept4, addrlen);
  SET_FIELD(e, accept4, flags);
  return e;
}

log_entry_t create_access_entry(clone_id_t clone_id, event_code_t event,
   const char *pathname, int mode)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, access, pathname, (char*)pathname);
  SET_FIELD(e, access, mode);
  return e;
}

log_entry_t create_bind_entry(clone_id_t clone_id, event_code_t event,
    int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, bind, sockfd);
  SET_FIELD2(e, bind, addr, (struct sockaddr*)addr);
  SET_FIELD(e, bind, addrlen);
  return e;
}

log_entry_t create_calloc_entry(clone_id_t clone_id, event_code_t event, size_t nmemb,
    size_t size)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, calloc, nmemb);
  SET_FIELD(e, calloc, size);
  return e;
}

log_entry_t create_chmod_entry(clone_id_t clone_id, event_code_t event,
                               const char *path, mode_t mode)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, chmod, path, (char *)path);
  SET_FIELD(e, chmod, mode);
  return e;
}

log_entry_t create_chown_entry(clone_id_t clone_id, event_code_t event,
                               const char *path, uid_t owner, gid_t group)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, chown, path, (char *)path);
  SET_FIELD(e, chown, owner);
  SET_FIELD(e, chown, group);
  return e;
}

log_entry_t create_close_entry(clone_id_t clone_id, event_code_t event, int fd)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, close, fd);
  return e;
}

log_entry_t create_closedir_entry(clone_id_t clone_id, event_code_t event, DIR *dirp)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, closedir, dirp, dirp);
  return e;
}

log_entry_t create_connect_entry(clone_id_t clone_id, event_code_t event, int sockfd,
                                 const struct sockaddr *serv_addr, socklen_t addrlen)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, connect, sockfd);
  SET_FIELD2(e, connect, serv_addr, (struct sockaddr*)serv_addr);
  SET_FIELD(e, connect, addrlen);
  return e;
}

log_entry_t create_dup_entry(clone_id_t clone_id, event_code_t event, int oldfd)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, dup, oldfd);
  return e;
}

log_entry_t create_dup2_entry(clone_id_t clone_id, event_code_t event,
                              int oldfd, int newfd)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, dup2, oldfd);
  SET_FIELD(e, dup2, newfd);
  return e;
}

log_entry_t create_dup3_entry(clone_id_t clone_id, event_code_t event,
                              int oldfd, int newfd, int flags)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, dup3, oldfd);
  SET_FIELD(e, dup3, newfd);
  SET_FIELD(e, dup3, flags);
  return e;
}

log_entry_t create_exec_barrier_entry()
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, CLONE_ID_ANYONE, exec_barrier_event);
  return e;
}

log_entry_t create_fclose_entry(clone_id_t clone_id, event_code_t event, FILE *fp)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, fclose, fp, fp);
  return e;
}

log_entry_t create_fchdir_entry(clone_id_t clone_id, event_code_t event, int fd)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fchdir, fd);
  return e;
}

log_entry_t create_fcntl_entry(clone_id_t clone_id, event_code_t event, int fd, int cmd,
    long arg_3_l, struct flock *arg_3_f)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fcntl, fd);
  SET_FIELD(e, fcntl, cmd);
  SET_FIELD(e, fcntl, arg_3_l);
  SET_FIELD(e, fcntl, arg_3_f);
  return e;
}

log_entry_t create_fdatasync_entry(clone_id_t clone_id, event_code_t event, int fd)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fdatasync, fd);
  return e;
}

log_entry_t create_fdopen_entry(clone_id_t clone_id, event_code_t event, int fd,
    const char *mode)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fdopen, fd);
  SET_FIELD2(e, fdopen, mode, (char*)mode);
  return e;
}

log_entry_t create_fdopendir_entry(clone_id_t clone_id, event_code_t event, int fd)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fdopendir, fd);
  return e;
}

log_entry_t create_fgets_entry(clone_id_t clone_id, event_code_t event, char *s, int size,
    FILE *stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, fgets, s, s);
  SET_FIELD(e, fgets, size);
  SET_FIELD2(e, fgets, stream, stream);
  return e;
}

log_entry_t create_ferror_entry(clone_id_t clone_id, event_code_t event, FILE *stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, ferror, stream, stream);
  return e;
}

log_entry_t create_feof_entry(clone_id_t clone_id, event_code_t event, FILE *stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, feof, stream, stream);
  return e;
}

log_entry_t create_fileno_entry(clone_id_t clone_id, event_code_t event, FILE *stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, fileno, stream, stream);
  return e;
}

log_entry_t create_fflush_entry(clone_id_t clone_id, event_code_t event, FILE *stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, fflush, stream, stream);
  return e;
}

log_entry_t create_fopen_entry(clone_id_t clone_id, event_code_t event,
    const char *name, const char *mode)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, fopen, name, (char*)name);
  SET_FIELD2(e, fopen, mode, (char*)mode);
  return e;
}

log_entry_t create_fopen64_entry(clone_id_t clone_id, event_code_t event,
    const char *name, const char *mode)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, fopen64, name, (char*)name);
  SET_FIELD2(e, fopen64, mode, (char*)mode);
  return e;
}

log_entry_t create_freopen_entry(clone_id_t clone_id, event_code_t event,
   const char *path, const char *mode, FILE *stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, freopen, path, (char*)path);
  SET_FIELD2(e, freopen, mode, (char*)mode);
  SET_FIELD(e, freopen, stream);
  return e;
}

log_entry_t create_fprintf_entry(clone_id_t clone_id, event_code_t event,
    FILE *stream, const char *format, va_list ap)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, fprintf, stream, stream);
  SET_FIELD2(e, fprintf, format, (char*)format);
  return e;
}

log_entry_t create_fscanf_entry(clone_id_t clone_id, event_code_t event,
    FILE *stream, const char *format, va_list ap)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, fscanf, stream, stream);
  SET_FIELD2(e, fscanf, format, (char*)format);
  return e;
}

log_entry_t create_fseek_entry(clone_id_t clone_id, event_code_t event,
                               FILE *stream, long offset, int whence)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fseek, stream);
  SET_FIELD(e, fseek, offset);
  SET_FIELD(e, fseek, whence);
  return e;
}

log_entry_t create_fputs_entry(clone_id_t clone_id, event_code_t event,
    const char *s, FILE *stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, fputs, s, (char*)s);
  SET_FIELD2(e, fputs, stream, stream);
  return e;
}

log_entry_t create_fputc_entry(clone_id_t clone_id, event_code_t event,
    int c, FILE *stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, fputc, c, c);
  SET_FIELD2(e, fputc, stream, stream);
  return e;
}

log_entry_t create_free_entry(clone_id_t clone_id, event_code_t event, void *ptr)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, free, ptr);
  return e;
}

log_entry_t create_ftell_entry(clone_id_t clone_id, event_code_t event, FILE *stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, ftell, stream, stream);
  return e;
}

log_entry_t create_fwrite_entry(clone_id_t clone_id, event_code_t event, const void *ptr,
    size_t size, size_t nmemb, FILE *stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, fwrite, ptr, (void*)ptr);
  SET_FIELD(e, fwrite, size);
  SET_FIELD(e, fwrite, nmemb);
  SET_FIELD2(e, fwrite, stream, stream);
  return e;
}

log_entry_t create_fread_entry(clone_id_t clone_id, event_code_t event, void *ptr,
    size_t size, size_t nmemb, FILE *stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fread, ptr);
  SET_FIELD(e, fread, size);
  SET_FIELD(e, fread, nmemb);
  SET_FIELD(e, fread, stream);
  return e;
}

log_entry_t create_fsync_entry(clone_id_t clone_id, event_code_t event, int fd)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fsync, fd);
  return e;
}

log_entry_t create_fxstat_entry(clone_id_t clone_id, event_code_t event, int vers, int fd,
     struct stat *buf)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fxstat, vers);
  SET_FIELD(e, fxstat, fd);
  memset(&GET_FIELD(e, fxstat, buf), '\0', sizeof(struct stat));
  return e;
}

log_entry_t create_fxstat64_entry(clone_id_t clone_id, event_code_t event, int vers, int fd,
     struct stat64 *buf)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fxstat64, vers);
  SET_FIELD(e, fxstat64, fd);
  memset(&GET_FIELD(e, fxstat64, buf), '\0', sizeof(struct stat64));
  return e;
}

log_entry_t create_getc_entry(clone_id_t clone_id, event_code_t event, FILE *stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, getc, stream, stream);
  return e;
}

log_entry_t create_getcwd_entry(clone_id_t clone_id, event_code_t event,
                                char *buf, size_t size)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, getcwd, buf);
  SET_FIELD(e, getcwd, size);
  return e;
}

log_entry_t create_gettimeofday_entry(clone_id_t clone_id, event_code_t event,
    struct timeval *tv, struct timezone *tz)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, gettimeofday, tv, tv);
  SET_FIELD2(e, gettimeofday, tz, tz);
  return e;
}

log_entry_t create_fgetc_entry(clone_id_t clone_id, event_code_t event, FILE *stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, fgetc, stream, stream);
  return e;
}

log_entry_t create_ungetc_entry(clone_id_t clone_id, event_code_t event, int c, FILE *stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, ungetc, stream, stream);
  SET_FIELD2(e, ungetc, c, c);
  return e;
}

log_entry_t create_getline_entry(clone_id_t clone_id, event_code_t event, char **lineptr, size_t *n,
    FILE *stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, getline, lineptr, *lineptr);
  SET_FIELD2(e, getline, n, *n);
  SET_FIELD2(e, getline, stream, stream);
  return e;
}

log_entry_t create_getpeername_entry(clone_id_t clone_id, event_code_t event, int sockfd,
                                     struct sockaddr *addr, socklen_t *addrlen)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, getpeername, sockfd);
  SET_FIELD2(e, getpeername, addr, addr);
  SET_FIELD(e, getpeername, addrlen);
  return e;
}

log_entry_t create_getsockname_entry(clone_id_t clone_id, event_code_t event, int sockfd,
                                     sockaddr *addr, socklen_t *addrlen)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, getsockname, sockfd);
  SET_FIELD(e, getsockname, addr);
  SET_FIELD(e, getsockname, addrlen);
  return e;
}

log_entry_t create_libc_memalign_entry(clone_id_t clone_id, event_code_t event, size_t boundary,
    size_t size)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, libc_memalign, boundary);
  SET_FIELD(e, libc_memalign, size);
  return e;
}

log_entry_t create_lseek_entry(clone_id_t clone_id, event_code_t event, int fd, off_t offset,
     int whence)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, lseek, fd);
  SET_FIELD(e, lseek, offset);
  SET_FIELD(e, lseek, whence);
  return e;
}

log_entry_t create_link_entry(clone_id_t clone_id, event_code_t event, const char *oldpath,
    const char *newpath)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, link, oldpath, (char*)oldpath);
  SET_FIELD2(e, link, newpath, (char*)newpath);
  return e;
}

log_entry_t create_listen_entry(clone_id_t clone_id, event_code_t event, int sockfd, int backlog)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, listen, sockfd);
  SET_FIELD(e, listen, backlog);
  return e;
}

log_entry_t create_localtime_entry(clone_id_t clone_id, event_code_t event,
                                   const time_t *timep)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, localtime, timep, (time_t *)timep);
  return e;
}

log_entry_t create_lxstat_entry(clone_id_t clone_id, event_code_t event, int vers,
    const char *path, struct stat *buf)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, lxstat, vers);
  SET_FIELD2(e, lxstat, path, (char*)path);
  memset(&GET_FIELD(e, lxstat, buf), '\0', sizeof(struct stat));
  return e;
}

log_entry_t create_lxstat64_entry(clone_id_t clone_id, event_code_t event, int vers,
    const char *path, struct stat64 *buf)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, lxstat64, vers);
  SET_FIELD2(e, lxstat64, path, (char*)path);
  memset(&GET_FIELD(e, lxstat64, buf), '\0', sizeof(struct stat64));
  return e;
}

log_entry_t create_malloc_entry(clone_id_t clone_id, event_code_t event, size_t size)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, malloc, size);
  return e;
}

log_entry_t create_mkdir_entry(clone_id_t clone_id, event_code_t event, const char *pathname,
    mode_t mode)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, mkdir, pathname, (char*)pathname);
  SET_FIELD2(e, mkdir, mode, mode);
  return e;
}

log_entry_t create_mkstemp_entry(clone_id_t clone_id, event_code_t event, char *temp)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, mkstemp, temp, temp);
  return e;
}

log_entry_t create_mmap_entry(clone_id_t clone_id, event_code_t event, void *addr,
    size_t length, int prot, int flags, int fd, off_t offset)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, mmap, addr, addr);
  SET_FIELD(e, mmap, length);
  SET_FIELD(e, mmap, prot);
  SET_FIELD(e, mmap, flags);
  SET_FIELD(e, mmap, fd);
  SET_FIELD(e, mmap, offset);
  return e;
}

log_entry_t create_mmap64_entry(clone_id_t clone_id, event_code_t event, void *addr,
    size_t length, int prot, int flags, int fd, off64_t offset)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, mmap64, addr, addr);
  SET_FIELD(e, mmap64, length);
  SET_FIELD(e, mmap64, prot);
  SET_FIELD(e, mmap64, flags);
  SET_FIELD(e, mmap64, fd);
  SET_FIELD(e, mmap64, offset);
  return e;
}

log_entry_t create_mremap_entry(clone_id_t clone_id, event_code_t event, void *old_address,
    size_t old_size, size_t new_size, int flags, void *new_addr)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, mremap, old_address, old_address);
  SET_FIELD(e, mremap, old_size);
  SET_FIELD(e, mremap, new_size);
  SET_FIELD(e, mremap, flags);
  return e;
}

log_entry_t create_munmap_entry(clone_id_t clone_id, event_code_t event, void *addr,
    size_t length)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, munmap, addr, addr);
  SET_FIELD(e, munmap, length);
  return e;
}

log_entry_t create_open_entry(clone_id_t clone_id, event_code_t event, const char *path,
   int flags, mode_t open_mode)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, open, path, (char*)path);
  SET_FIELD(e, open, flags);
  SET_FIELD(e, open, open_mode);
  return e;
}

log_entry_t create_open64_entry(clone_id_t clone_id, event_code_t event, const char *path,
   int flags, mode_t open_mode)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, open64, path, (char*)path);
  SET_FIELD(e, open64, flags);
  SET_FIELD(e, open64, open_mode);
  return e;
}

log_entry_t create_openat_entry(clone_id_t clone_id, event_code_t event, int dirfd, const char *pathname, int flags)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, openat, dirfd);
  SET_FIELD2(e, openat, pathname, (char*)pathname);
  SET_FIELD(e, openat, flags);
  return e;
}

log_entry_t create_opendir_entry(clone_id_t clone_id, event_code_t event, const char *name)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, opendir, name, (char*)name);
  return e;
}

log_entry_t create_pread_entry(clone_id_t clone_id, event_code_t event, int fd,
    void* buf, size_t count, off_t offset)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, pread, fd);
  SET_FIELD(e, pread, buf);
  SET_FIELD(e, pread, count);
  SET_FIELD(e, pread, offset);
  return e;
}

log_entry_t create_putc_entry(clone_id_t clone_id, event_code_t event, int c,
    FILE *stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, putc, c);
  SET_FIELD2(e, putc, stream, stream);
  return e;
}

log_entry_t create_pwrite_entry(clone_id_t clone_id, event_code_t event, int fd,
    const void* buf, size_t count, off_t offset)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, pwrite, fd);
  SET_FIELD2(e, pwrite, buf, (void*)buf);
  SET_FIELD(e, pwrite, count);
  SET_FIELD(e, pwrite, offset);
  return e;
}

log_entry_t create_pthread_cond_broadcast_entry(clone_id_t clone_id, event_code_t event,
    pthread_cond_t *cond_var)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, pthread_cond_broadcast, addr, cond_var);
  return e;
}

log_entry_t create_pthread_cond_signal_entry(clone_id_t clone_id, event_code_t event,
    pthread_cond_t *cond_var)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, pthread_cond_signal, addr, cond_var);
  return e;
}

log_entry_t create_pthread_cond_wait_entry(clone_id_t clone_id, event_code_t event,
    pthread_cond_t *cond_var, pthread_mutex_t *mutex)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, pthread_cond_wait, mutex_addr, mutex);
  SET_FIELD2(e, pthread_cond_wait, cond_addr, cond_var);
  return e;
}

log_entry_t create_pthread_cond_timedwait_entry(clone_id_t clone_id, event_code_t event,
    pthread_cond_t *cond_var, pthread_mutex_t *mutex,
    const struct timespec *abstime)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, pthread_cond_timedwait, mutex_addr, mutex);
  SET_FIELD2(e, pthread_cond_timedwait, cond_addr, cond_var);
  SET_FIELD2(e, pthread_cond_timedwait, abstime, (struct timespec*) abstime);
  return e;
}

log_entry_t create_pthread_rwlock_unlock_entry(clone_id_t clone_id, event_code_t event,
    pthread_rwlock_t *rwlock)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, pthread_rwlock_unlock, addr, rwlock);
  return e;
}

log_entry_t create_pthread_rwlock_rdlock_entry(clone_id_t clone_id, event_code_t event,
    pthread_rwlock_t *rwlock)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, pthread_rwlock_rdlock, addr, rwlock);
  return e;
}

log_entry_t create_pthread_rwlock_wrlock_entry(clone_id_t clone_id, event_code_t event,
    pthread_rwlock_t *rwlock)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, pthread_rwlock_wrlock, addr, rwlock);
  return e;
}

log_entry_t create_pthread_create_entry(clone_id_t clone_id, event_code_t event,
    pthread_t *thread, const pthread_attr_t *attr,
    void *(*start_routine)(void*), void *arg)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, pthread_create, thread);
  SET_FIELD2(e, pthread_create, attr, (pthread_attr_t*) attr);
  SET_FIELD(e, pthread_create, start_routine);
  SET_FIELD(e, pthread_create, arg);
  return e;
}

log_entry_t create_pthread_detach_entry(clone_id_t clone_id, event_code_t event,
    pthread_t thread)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, pthread_detach, thread);
  return e;
}

log_entry_t create_pthread_exit_entry(clone_id_t clone_id, event_code_t event,
    void *value_ptr)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, pthread_exit, value_ptr);
  return e;
}

log_entry_t create_pthread_join_entry(clone_id_t clone_id, event_code_t event,
    pthread_t thread, void *value_ptr)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, pthread_join, thread);
  SET_FIELD(e, pthread_join, value_ptr);
  return e;
}

log_entry_t create_pthread_kill_entry(clone_id_t clone_id, event_code_t event,
    pthread_t thread, int sig)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, pthread_kill, thread);
  SET_FIELD(e, pthread_kill, sig);
  return e;
}

log_entry_t create_pthread_mutex_lock_entry(clone_id_t clone_id, event_code_t event, pthread_mutex_t *mutex) {
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, pthread_mutex_lock, addr, mutex);
  return e;
}

log_entry_t create_pthread_mutex_trylock_entry(clone_id_t clone_id, event_code_t event, pthread_mutex_t *mutex) {
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, pthread_mutex_trylock, addr, mutex);
  return e;
}

log_entry_t create_pthread_mutex_unlock_entry(clone_id_t clone_id, event_code_t event, pthread_mutex_t *mutex) {
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, pthread_mutex_unlock, addr, mutex);
  return e;
}

log_entry_t create_rand_entry(clone_id_t clone_id, event_code_t event)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  return e;
}

log_entry_t create_read_entry(clone_id_t clone_id, event_code_t event, int readfd,
    void* buf_addr, size_t count)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, read, readfd);
  SET_FIELD(e, read, buf_addr);
  SET_FIELD(e, read, count);
  return e;
}

log_entry_t create_readdir_entry(clone_id_t clone_id, event_code_t event, DIR *dirp)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, readdir, dirp, dirp);
  return e;
}

log_entry_t create_readdir_r_entry(clone_id_t clone_id, event_code_t event, DIR *dirp,
    struct dirent *entry, struct dirent **result)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, readdir_r, dirp, dirp);
  SET_FIELD2(e, readdir_r, entry, entry);
  SET_FIELD2(e, readdir_r, result, result);
  return e;
}

log_entry_t create_readlink_entry(clone_id_t clone_id, event_code_t event,
    const char *path, char *buf, size_t bufsiz)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, readlink, path, (char*)path);
  SET_FIELD(e, readlink, buf);
  SET_FIELD(e, readlink, bufsiz);
  return e;
}

log_entry_t create_realloc_entry(clone_id_t clone_id, event_code_t event,
    void *ptr, size_t size)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, realloc, ptr);
  SET_FIELD(e, realloc, size);
  return e;
}

log_entry_t create_rename_entry(clone_id_t clone_id, event_code_t event, const char *oldpath,
    const char *newpath)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, rename, oldpath, (char*)oldpath);
  SET_FIELD2(e, rename, newpath, (char*)newpath);
  return e;
}

log_entry_t create_rewind_entry(clone_id_t clone_id, event_code_t event, FILE *stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, rewind, stream, stream);
  return e;
}

log_entry_t create_rmdir_entry(clone_id_t clone_id, event_code_t event, const char *pathname)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, rmdir, pathname, (char*)pathname);
  return e;
}

log_entry_t create_select_entry(clone_id_t clone_id, event_code_t event, int nfds,
    fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
    struct timeval *timeout)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, select, nfds);
  // We have to do something special for 'readfds' and 'writefds' since we
  // do a deep copy.
  copyFdSet(readfds, &GET_FIELD(e, select, readfds));
  copyFdSet(writefds, &GET_FIELD(e, select, writefds));
  SET_FIELD2(e, select, exceptfds, exceptfds);
  SET_FIELD2(e, select, timeout, timeout);
  return e;
}

log_entry_t create_setsockopt_entry(clone_id_t clone_id, event_code_t event, int sockfd,
    int level, int optname, const void *optval, socklen_t optlen) {
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, setsockopt, sockfd);
  SET_FIELD(e, setsockopt, level);
  SET_FIELD(e, setsockopt, optname);
  SET_FIELD2(e, setsockopt, optval, (void*) optval);
  SET_FIELD(e, setsockopt, optlen);
  return e;
}

log_entry_t create_getsockopt_entry(clone_id_t clone_id, event_code_t event, int sockfd,
    int level, int optname, void *optval, socklen_t* optlen) {
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, getsockopt, sockfd);
  SET_FIELD(e, getsockopt, level);
  SET_FIELD(e, getsockopt, optname);
  SET_FIELD2(e, getsockopt, optval, (void*) optval);
  SET_FIELD2(e, getsockopt, optlen, (socklen_t*) optlen);
  return e;
}

log_entry_t create_ioctl_entry(clone_id_t clone_id, event_code_t event, int d,
    int request, void *arg) {
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, ioctl, d);
  SET_FIELD(e, ioctl, request);
  SET_FIELD(e, ioctl, arg);
  return e;
}

log_entry_t create_signal_handler_entry(clone_id_t clone_id, event_code_t event, int sig)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, signal_handler, sig);
  return e;
}

log_entry_t create_sigwait_entry(clone_id_t clone_id, event_code_t event, const sigset_t *set,
    int *sigwait_sig)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, sigwait, set, (sigset_t*)set);
  SET_FIELD(e, sigwait, sigwait_sig);
  return e;
}

log_entry_t create_srand_entry(clone_id_t clone_id, event_code_t event, unsigned int seed)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, srand, seed);
  return e;
}

log_entry_t create_socket_entry(clone_id_t clone_id, event_code_t event, int domain, int type,
    int protocol)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, socket, domain);
  SET_FIELD(e, socket, type);
  SET_FIELD(e, socket, protocol);
  return e;
}

log_entry_t create_xstat_entry(clone_id_t clone_id, event_code_t event, int vers,
    const char *path, struct stat *buf)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, xstat, vers);
  SET_FIELD2(e, xstat, path, (char*)path);
  memset(&GET_FIELD(e, xstat, buf), '\0', sizeof(struct stat));
  return e;
}

log_entry_t create_xstat64_entry(clone_id_t clone_id, event_code_t event, int vers,
    const char *path, struct stat64 *buf)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, xstat64, vers);
  SET_FIELD2(e, xstat64, path, (char*)path);
  memset(&GET_FIELD(e, xstat64, buf), '\0', sizeof(struct stat64));
  return e;
}

log_entry_t create_time_entry(clone_id_t clone_id, event_code_t event, time_t *tloc)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, time, tloc);
  return e;
}

log_entry_t create_tmpfile_entry(clone_id_t clone_id, event_code_t event)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  return e;
}

log_entry_t create_truncate_entry(clone_id_t clone_id, event_code_t event,
                                  const char *path, off_t length)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, truncate, path, (char *)path);
  SET_FIELD(e, truncate, length);
  return e;
}

log_entry_t create_unlink_entry(clone_id_t clone_id, event_code_t event,
     const char *pathname)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD2(e, unlink, pathname, (char*)pathname);
  return e;
}

log_entry_t create_user_entry(clone_id_t clone_id, event_code_t event)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  return e;
}

log_entry_t create_write_entry(clone_id_t clone_id, event_code_t event, int writefd,
    const void* buf_addr, size_t count)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, write, writefd);
  SET_FIELD2(e, write, buf_addr, (void*)buf_addr);
  SET_FIELD(e, write, count);
  return e;
}

log_entry_t create_epoll_create_entry(clone_id_t clone_id, event_code_t event, int size)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, epoll_create, size);
  return e;
}

log_entry_t create_epoll_create1_entry(clone_id_t clone_id, event_code_t event,
                                       int flags)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, epoll_create1, flags);
  return e;
}

log_entry_t create_epoll_ctl_entry(clone_id_t clone_id, event_code_t event,
                                   int epfd, int op, int fd,
                                   struct epoll_event *_event)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, epoll_ctl, epfd);
  SET_FIELD(e, epoll_ctl, op);
  SET_FIELD(e, epoll_ctl, fd);
  SET_FIELD(e, epoll_ctl, _event);
  return e;
}

log_entry_t create_epoll_wait_entry(clone_id_t clone_id, event_code_t event,
                                    int epfd, struct epoll_event *events,
                                    int maxevents, int timeout)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, epoll_wait, epfd);
  SET_FIELD(e, epoll_wait, events);
  SET_FIELD(e, epoll_wait, maxevents);
  SET_FIELD(e, epoll_wait, timeout);
  return e;
}

log_entry_t create_getpwnam_r_entry(clone_id_t clone_id, event_code_t event,
                                    const char *name, struct passwd *pwd,
                                    char *buf, size_t buflen,
                                    struct passwd **result)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, getpwnam_r, name);
  SET_FIELD(e, getpwnam_r, pwd);
  SET_FIELD(e, getpwnam_r, buf);
  SET_FIELD(e, getpwnam_r, buflen);
  SET_FIELD(e, getpwnam_r, result);
  return e;
}

log_entry_t create_getpwuid_r_entry(clone_id_t clone_id, event_code_t event,
                                    uid_t uid, struct passwd *pwd,
                                    char *buf, size_t buflen,
                                    struct passwd **result)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, getpwuid_r, uid);
  SET_FIELD(e, getpwuid_r, pwd);
  SET_FIELD(e, getpwuid_r, buf);
  SET_FIELD(e, getpwuid_r, buflen);
  SET_FIELD(e, getpwuid_r, result);
  return e;
}

log_entry_t create_getgrnam_r_entry(clone_id_t clone_id, event_code_t event,
                                    const char *name, struct group *grp,
                                    char *buf, size_t buflen,
                                    struct group **result)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, getgrnam_r, name);
  SET_FIELD(e, getgrnam_r, grp);
  SET_FIELD(e, getgrnam_r, buf);
  SET_FIELD(e, getgrnam_r, buflen);
  SET_FIELD(e, getgrnam_r, result);
  return e;
}

log_entry_t create_getgrgid_r_entry(clone_id_t clone_id, event_code_t event,
                                    gid_t gid, struct group *grp,
                                    char *buf, size_t buflen,
                                    struct group **result)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, getgrgid_r, gid);
  SET_FIELD(e, getgrgid_r, grp);
  SET_FIELD(e, getgrgid_r, buf);
  SET_FIELD(e, getgrgid_r, buflen);
  SET_FIELD(e, getgrgid_r, result);
  return e;
}

log_entry_t create_getaddrinfo_entry(clone_id_t clone_id, event_code_t event,
                                     const char *node, const char *service,
                                     const struct addrinfo *hints,
                                     struct addrinfo **res)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, getaddrinfo, node);
  SET_FIELD(e, getaddrinfo, service);
  SET_FIELD(e, getaddrinfo, hints);
  SET_FIELD(e, getaddrinfo, res);
  return e;
}

log_entry_t create_freeaddrinfo_entry(clone_id_t clone_id, event_code_t event,
                                      struct addrinfo *res)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, freeaddrinfo, res);
  return e;
}

log_entry_t create_getnameinfo_entry(clone_id_t clone_id, event_code_t event,
                                     const struct sockaddr *sa, socklen_t salen,
                                     char *host, socklen_t hostlen,
                                     char *serv, socklen_t servlen,
                                     unsigned int flags)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, getnameinfo, sa);
  SET_FIELD(e, getnameinfo, salen);
  SET_FIELD(e, getnameinfo, host);
  SET_FIELD(e, getnameinfo, hostlen);
  SET_FIELD(e, getnameinfo, serv);
  SET_FIELD(e, getnameinfo, servlen);
  SET_FIELD(e, getnameinfo, flags);
  return e;
}

static TURN_CHECK_P(base_turn_check)
{
  // Predicate function for a basic check -- event # and clone id.
  return GET_COMMON_PTR(e1,clone_id) == GET_COMMON_PTR(e2,clone_id) &&
         GET_COMMON_PTR(e1,event) == GET_COMMON_PTR(e2,event);
}

TURN_CHECK_P(pthread_mutex_lock_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, pthread_mutex_lock, addr) ==
      GET_FIELD_PTR(e2, pthread_mutex_lock, addr);
}

TURN_CHECK_P(pthread_mutex_trylock_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, pthread_mutex_trylock, addr) ==
      GET_FIELD_PTR(e2, pthread_mutex_trylock, addr);
}

TURN_CHECK_P(pthread_mutex_unlock_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, pthread_mutex_unlock, addr) ==
      GET_FIELD_PTR(e2, pthread_mutex_unlock, addr);
}

TURN_CHECK_P(pthread_cond_signal_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, pthread_cond_signal, addr) ==
      GET_FIELD_PTR(e2, pthread_cond_signal, addr);
}

TURN_CHECK_P(pthread_cond_broadcast_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, pthread_cond_broadcast, addr) ==
      GET_FIELD_PTR(e2, pthread_cond_broadcast, addr);
}

TURN_CHECK_P(pthread_cond_wait_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, pthread_cond_wait, mutex_addr) ==
      GET_FIELD_PTR(e2, pthread_cond_wait, mutex_addr) &&
    GET_FIELD_PTR(e1, pthread_cond_wait, cond_addr) ==
      GET_FIELD_PTR(e2, pthread_cond_wait, cond_addr);
}

TURN_CHECK_P(pthread_cond_timedwait_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, pthread_cond_timedwait, mutex_addr) ==
      GET_FIELD_PTR(e2, pthread_cond_timedwait, mutex_addr) &&
    GET_FIELD_PTR(e1, pthread_cond_timedwait, cond_addr) ==
      GET_FIELD_PTR(e2, pthread_cond_timedwait, cond_addr) &&
    GET_FIELD_PTR(e1, pthread_cond_timedwait, abstime) ==
      GET_FIELD_PTR(e2, pthread_cond_timedwait, abstime);
}

TURN_CHECK_P(pthread_rwlock_unlock_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, pthread_rwlock_unlock, addr) ==
      GET_FIELD_PTR(e2, pthread_rwlock_unlock, addr);
}

TURN_CHECK_P(pthread_rwlock_rdlock_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, pthread_rwlock_rdlock, addr) ==
      GET_FIELD_PTR(e2, pthread_rwlock_rdlock, addr);
}

TURN_CHECK_P(pthread_rwlock_wrlock_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, pthread_rwlock_wrlock, addr) ==
      GET_FIELD_PTR(e2, pthread_rwlock_wrlock, addr);
}

TURN_CHECK_P(pthread_create_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, pthread_create, thread) ==
      GET_FIELD_PTR(e2, pthread_create, thread) &&
    GET_FIELD_PTR(e1, pthread_create, attr) ==
      GET_FIELD_PTR(e2, pthread_create, attr) &&
    GET_FIELD_PTR(e1, pthread_create, start_routine) ==
      GET_FIELD_PTR(e2, pthread_create, start_routine) &&
    GET_FIELD_PTR(e1, pthread_create, arg) ==
      GET_FIELD_PTR(e2, pthread_create, arg);
}

TURN_CHECK_P(pthread_detach_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, pthread_detach, thread) ==
      GET_FIELD_PTR(e2, pthread_detach, thread);
}

TURN_CHECK_P(pthread_exit_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, pthread_exit, value_ptr) ==
      GET_FIELD_PTR(e2, pthread_exit, value_ptr);
}


TURN_CHECK_P(pthread_join_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, pthread_join, thread) ==
      GET_FIELD_PTR(e2, pthread_join, thread) &&
    GET_FIELD_PTR(e1, pthread_join, value_ptr) ==
      GET_FIELD_PTR(e2, pthread_join, value_ptr);
}

TURN_CHECK_P(pthread_kill_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, pthread_kill, thread) ==
      GET_FIELD_PTR(e2, pthread_kill, thread) &&
    GET_FIELD_PTR(e1, pthread_kill, sig) ==
      GET_FIELD_PTR(e2, pthread_kill, sig);
}

TURN_CHECK_P(read_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, read, buf_addr) ==
      GET_FIELD_PTR(e2, read, buf_addr) &&
    GET_FIELD_PTR(e1, read, count) ==
      GET_FIELD_PTR(e2, read, count);
}

TURN_CHECK_P(readdir_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, readdir, dirp) ==
      GET_FIELD_PTR(e2, readdir, dirp);
}

TURN_CHECK_P(readdir_r_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, readdir_r, dirp) ==
      GET_FIELD_PTR(e2, readdir_r, dirp);
}

TURN_CHECK_P(readlink_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, readlink, path) ==
      GET_FIELD_PTR(e2, readlink, path) &&
    GET_FIELD_PTR(e1, readlink, bufsiz) ==
      GET_FIELD_PTR(e2, readlink, bufsiz);
}

TURN_CHECK_P(unlink_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, unlink, pathname) ==
      GET_FIELD_PTR(e2, unlink, pathname);
}

TURN_CHECK_P(user_turn_check)
{
  return base_turn_check(e1, e2);
}

TURN_CHECK_P(write_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, write, buf_addr) ==
      GET_FIELD_PTR(e2, write, buf_addr) &&
    GET_FIELD_PTR(e1, write, count) ==
      GET_FIELD_PTR(e2, write, count);
}

TURN_CHECK_P(close_turn_check)
{
  return base_turn_check(e1, e2);// && e1->fd == e2->fd;
}

TURN_CHECK_P(closedir_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, closedir, dirp) ==
      GET_FIELD_PTR(e2, closedir, dirp);
}

TURN_CHECK_P(connect_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, connect, serv_addr) ==
      GET_FIELD_PTR(e2, connect, serv_addr) &&
    GET_FIELD_PTR(e1, connect, addrlen) ==
      GET_FIELD_PTR(e2, connect, addrlen);
}

TURN_CHECK_P(dup_turn_check)
{
  return base_turn_check(e1, e2) &&
    IS_EQUAL_FIELD_PTR(e1, e2, dup, oldfd);
}

TURN_CHECK_P(dup2_turn_check)
{
  return base_turn_check(e1, e2) &&
    IS_EQUAL_FIELD_PTR(e1, e2, dup2, oldfd) &&
    IS_EQUAL_FIELD_PTR(e1, e2, dup2, newfd);
}

TURN_CHECK_P(dup3_turn_check)
{
  return base_turn_check(e1, e2) &&
    IS_EQUAL_FIELD_PTR(e1, e2, dup3, oldfd) &&
    IS_EQUAL_FIELD_PTR(e1, e2, dup3, newfd) &&
    IS_EQUAL_FIELD_PTR(e1, e2, dup3, flags);
}

TURN_CHECK_P(rand_turn_check)
{
  return base_turn_check(e1, e2);
}

TURN_CHECK_P(srand_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, srand, seed) ==
      GET_FIELD_PTR(e2, srand, seed);
}

TURN_CHECK_P(socket_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, socket, domain) ==
      GET_FIELD_PTR(e2, socket, domain) &&
    GET_FIELD_PTR(e1, socket, type) ==
      GET_FIELD_PTR(e2, socket, type) &&
    GET_FIELD_PTR(e1, socket, protocol) ==
      GET_FIELD_PTR(e2, socket, protocol);
}

TURN_CHECK_P(xstat_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, xstat, vers) ==
      GET_FIELD_PTR(e2, xstat, vers) &&
    GET_FIELD_PTR(e1, xstat, path) ==
      GET_FIELD_PTR(e2, xstat, path);
  /*GET_FIELD_PTR(e1, xstat, buf) ==
    GET_FIELD_PTR(e2, xstat, buf);*/
}

TURN_CHECK_P(xstat64_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, xstat64, vers) ==
      GET_FIELD_PTR(e2, xstat64, vers) &&
    GET_FIELD_PTR(e1, xstat64, path) ==
      GET_FIELD_PTR(e2, xstat64, path);
  /*GET_FIELD_PTR(e1, xstat64, buf) ==
    GET_FIELD_PTR(e2, xstat64, buf);*/
}

TURN_CHECK_P(time_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, time, tloc) ==
      GET_FIELD_PTR(e2, time, tloc);
}

TURN_CHECK_P(tmpfile_turn_check)
{
  return base_turn_check(e1, e2);
}

TURN_CHECK_P(truncate_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, truncate, path) ==
      GET_FIELD_PTR(e2, truncate, path) &&
    GET_FIELD_PTR(e1, truncate, length) ==
      GET_FIELD_PTR(e2, truncate, length);
}

TURN_CHECK_P(accept_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, accept, addr) ==
      GET_FIELD_PTR(e2, accept, addr) &&
    GET_FIELD_PTR(e1, accept, addrlen) ==
      GET_FIELD_PTR(e2, accept, addrlen);
}

TURN_CHECK_P(accept4_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, accept4, addr) ==
      GET_FIELD_PTR(e2, accept4, addr) &&
    GET_FIELD_PTR(e1, accept4, addrlen) ==
      GET_FIELD_PTR(e2, accept4, addrlen) &&
    GET_FIELD_PTR(e1, accept4, flags) ==
      GET_FIELD_PTR(e2, accept4, flags);
}

TURN_CHECK_P(access_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, access, pathname) ==
      GET_FIELD_PTR(e2, access, pathname) &&
    GET_FIELD_PTR(e1, access, mode) ==
      GET_FIELD_PTR(e2, access, mode);
}

TURN_CHECK_P(bind_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, bind, addr) ==
      GET_FIELD_PTR(e2, bind, addr) &&
    GET_FIELD_PTR(e1, bind, addrlen) ==
      GET_FIELD_PTR(e2, bind, addrlen);
}

TURN_CHECK_P(getpeername_turn_check)
{
  return base_turn_check(e1, e2) &&
    /*GET_FIELD_PTR(e1, getpeername, sockfd) ==
      GET_FIELD_PTR(e2, getpeername, sockfd) &&*/
    GET_FIELD_PTR(e1, getpeername, addrlen) ==
      GET_FIELD_PTR(e2, getpeername, addrlen);
    // TODO: How to compare these:
  /*GET_FIELD_PTR(e1, getpeername, addr) ==
      GET_FIELD_PTR(e2, getpeername, addr)*/
}

TURN_CHECK_P(getsockname_turn_check)
{
  return base_turn_check(e1, e2) &&
    /*GET_FIELD_PTR(e1, getsockname, sockfd) ==
      GET_FIELD_PTR(e2, getsockname, sockfd) &&*/
    GET_FIELD_PTR(e1, getsockname, addrlen) ==
      GET_FIELD_PTR(e2, getsockname, addrlen) &&
    GET_FIELD_PTR(e1, getsockname, addr) ==
      GET_FIELD_PTR(e2, getsockname, addr);
}

TURN_CHECK_P(setsockopt_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, setsockopt, level) ==
      GET_FIELD_PTR(e2, setsockopt, level) &&
    GET_FIELD_PTR(e1, setsockopt, optname) ==
      GET_FIELD_PTR(e2, setsockopt, optname) &&
    GET_FIELD_PTR(e1, setsockopt, optval) ==
      GET_FIELD_PTR(e2, setsockopt, optval) &&
    GET_FIELD_PTR(e1, setsockopt, optlen) ==
      GET_FIELD_PTR(e2, setsockopt, optlen);
}

TURN_CHECK_P(getsockopt_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, getsockopt, level) ==
      GET_FIELD_PTR(e2, getsockopt, level) &&
    GET_FIELD_PTR(e1, getsockopt, optname) ==
      GET_FIELD_PTR(e2, getsockopt, optname) &&
    GET_FIELD_PTR(e1, getsockopt, optval) ==
      GET_FIELD_PTR(e2, getsockopt, optval) &&
    GET_FIELD_PTR(e1, getsockopt, optlen) ==
      GET_FIELD_PTR(e2, getsockopt, optlen);
}

TURN_CHECK_P(ioctl_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, ioctl, d) ==
      GET_FIELD_PTR(e2, ioctl, d) &&
    GET_FIELD_PTR(e1, ioctl, request) ==
      GET_FIELD_PTR(e2, ioctl, request) &&
    GET_FIELD_PTR(e1, ioctl, arg) ==
      GET_FIELD_PTR(e2, ioctl, arg);
}

TURN_CHECK_P(signal_handler_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, signal_handler, sig) ==
    GET_FIELD_PTR(e2, signal_handler, sig);
}

TURN_CHECK_P(sigwait_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, sigwait, set) ==
      GET_FIELD_PTR(e2, sigwait, set) &&
    GET_FIELD_PTR(e1, sigwait, sigwait_sig) ==
      GET_FIELD_PTR(e2, sigwait, sigwait_sig);
}

TURN_CHECK_P(fclose_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, fclose, fp) ==
      GET_FIELD_PTR(e2, fclose, fp);
}

TURN_CHECK_P(fchdir_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, fchdir, fd) ==
      GET_FIELD_PTR(e2, fchdir, fd);
}

TURN_CHECK_P(fcntl_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, fcntl, cmd) ==
      GET_FIELD_PTR(e2, fcntl, cmd) &&
    GET_FIELD_PTR(e1, fcntl, arg_3_l) ==
      GET_FIELD_PTR(e2, fcntl, arg_3_l) &&
    GET_FIELD_PTR(e1, fcntl, arg_3_f) ==
      GET_FIELD_PTR(e2, fcntl, arg_3_f);
}

TURN_CHECK_P(fdatasync_turn_check)
{
  return base_turn_check(e1,e2);
}

TURN_CHECK_P(fdopen_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, fdopen, fd) ==
      GET_FIELD_PTR(e2, fdopen, fd) &&
    GET_FIELD_PTR(e1, fdopen, mode) ==
      GET_FIELD_PTR(e2, fdopen, mode);
}

TURN_CHECK_P(fdopendir_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, fdopendir, fd) ==
      GET_FIELD_PTR(e2, fdopendir, fd);
}

TURN_CHECK_P(fgets_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, fgets, s) ==
      GET_FIELD_PTR(e2, fgets, s) &&
    GET_FIELD_PTR(e1, fgets, stream) ==
      GET_FIELD_PTR(e2, fgets, stream) &&
    GET_FIELD_PTR(e1, fgets, size) ==
      GET_FIELD_PTR(e2, fgets, size);
}

TURN_CHECK_P(ferror_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, ferror, stream) ==
      GET_FIELD_PTR(e2, ferror, stream);
}

TURN_CHECK_P(feof_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, feof, stream) ==
      GET_FIELD_PTR(e2, feof, stream);
}

TURN_CHECK_P(fileno_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, fileno, stream) ==
      GET_FIELD_PTR(e2, fileno, stream);
}

TURN_CHECK_P(fflush_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, fflush, stream) ==
      GET_FIELD_PTR(e2, fflush, stream);
}

TURN_CHECK_P(getc_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, getc, stream) ==
      GET_FIELD_PTR(e2, getc, stream);
}

TURN_CHECK_P(getcwd_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, getcwd, buf) ==
      GET_FIELD_PTR(e2, getcwd, buf) &&
    GET_FIELD_PTR(e1, getcwd, size) ==
      GET_FIELD_PTR(e2, getcwd, size);
}

TURN_CHECK_P(gettimeofday_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, gettimeofday, tv) ==
      GET_FIELD_PTR(e2, gettimeofday, tv) &&
    GET_FIELD_PTR(e1, gettimeofday, tz) ==
      GET_FIELD_PTR(e2, gettimeofday, tz);
}

TURN_CHECK_P(fgetc_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, fgetc, stream) ==
      GET_FIELD_PTR(e2, fgetc, stream);
}

TURN_CHECK_P(ungetc_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, ungetc, stream) ==
      GET_FIELD_PTR(e2, ungetc, stream) &&
    GET_FIELD_PTR(e1, ungetc, c) ==
      GET_FIELD_PTR(e2, ungetc, c);
}

TURN_CHECK_P(getline_turn_check)
{
  /* We don't check for n because it might change, in case lineptr gets
     reallocated. */
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, getline, lineptr) ==
      GET_FIELD_PTR(e2, getline, lineptr) &&
    GET_FIELD_PTR(e1, getline, stream) ==
      GET_FIELD_PTR(e2, getline, stream);
}

TURN_CHECK_P(fopen_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, fopen, name) ==
      GET_FIELD_PTR(e2, fopen, name) &&
    GET_FIELD_PTR(e1, fopen, mode) ==
      GET_FIELD_PTR(e2, fopen, mode);
}

TURN_CHECK_P(fopen64_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, fopen64, name) ==
      GET_FIELD_PTR(e2, fopen64, name) &&
    GET_FIELD_PTR(e1, fopen64, mode) ==
      GET_FIELD_PTR(e2, fopen64, mode);
}

TURN_CHECK_P(freopen_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, freopen, path) ==
      GET_FIELD_PTR(e2, freopen, path) &&
    GET_FIELD_PTR(e1, freopen, mode) ==
      GET_FIELD_PTR(e2, freopen, mode) &&
    GET_FIELD_PTR(e1, freopen, stream) ==
      GET_FIELD_PTR(e2, freopen, stream);
}

TURN_CHECK_P(fprintf_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, fprintf, stream) ==
      GET_FIELD_PTR(e2, fprintf, stream) &&
    GET_FIELD_PTR(e1, fprintf, format) ==
      GET_FIELD_PTR(e2, fprintf, format);
}

TURN_CHECK_P(fscanf_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, fscanf, stream) ==
      GET_FIELD_PTR(e2, fscanf, stream) &&
    GET_FIELD_PTR(e1, fscanf, format) ==
      GET_FIELD_PTR(e2, fscanf, format);
}

TURN_CHECK_P(fseek_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, fseek, stream) ==
      GET_FIELD_PTR(e2, fseek, stream) &&
    GET_FIELD_PTR(e1, fseek, offset) ==
      GET_FIELD_PTR(e2, fseek, offset) &&
    GET_FIELD_PTR(e1, fseek, whence) ==
      GET_FIELD_PTR(e2, fseek, whence);
}

TURN_CHECK_P(fputs_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, fputs, s) ==
      GET_FIELD_PTR(e2, fputs, s) &&
    GET_FIELD_PTR(e1, fputs, stream) ==
      GET_FIELD_PTR(e2, fputs, stream);
}

TURN_CHECK_P(fputc_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, fputc, c) ==
      GET_FIELD_PTR(e2, fputc, c) &&
    GET_FIELD_PTR(e1, fputc, stream) ==
      GET_FIELD_PTR(e2, fputc, stream);
}

TURN_CHECK_P(calloc_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, calloc, nmemb) ==
      GET_FIELD_PTR(e2, calloc, nmemb) &&
    GET_FIELD_PTR(e1, calloc, size) ==
      GET_FIELD_PTR(e2, calloc, size);
}

TURN_CHECK_P(chmod_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, chmod, path) ==
      GET_FIELD_PTR(e2, chmod, path) &&
    GET_FIELD_PTR(e1, chmod, mode) ==
      GET_FIELD_PTR(e2, chmod, mode);
}

TURN_CHECK_P(chown_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, chown, path) ==
      GET_FIELD_PTR(e2, chown, path) &&
    GET_FIELD_PTR(e1, chown, owner) ==
      GET_FIELD_PTR(e2, chown, owner) &&
    GET_FIELD_PTR(e1, chown, group) ==
      GET_FIELD_PTR(e2, chown, group);
}

TURN_CHECK_P(lseek_turn_check)
{
  return base_turn_check(e1,e2) &&
    /*GET_FIELD_PTR(e1, lseek, fd) ==
      GET_FIELD_PTR(e2, lseek, fd) &&*/
    GET_FIELD_PTR(e1, lseek, offset) ==
      GET_FIELD_PTR(e2, lseek, offset) &&
    GET_FIELD_PTR(e1, lseek, whence) ==
      GET_FIELD_PTR(e2, lseek, whence);
}

TURN_CHECK_P(link_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, link, oldpath) ==
      GET_FIELD_PTR(e2, link, oldpath) &&
    GET_FIELD_PTR(e1, link, newpath) ==
      GET_FIELD_PTR(e2, link, newpath);
}

TURN_CHECK_P(listen_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, listen, sockfd) ==
      GET_FIELD_PTR(e2, listen, sockfd) &&
    GET_FIELD_PTR(e1, listen, backlog) ==
      GET_FIELD_PTR(e2, listen, backlog);
}

TURN_CHECK_P(localtime_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, localtime, timep) ==
      GET_FIELD_PTR(e2, localtime, timep);
}

TURN_CHECK_P(lxstat_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, lxstat, vers) ==
      GET_FIELD_PTR(e2, lxstat, vers) &&
    GET_FIELD_PTR(e1, lxstat, path) ==
      GET_FIELD_PTR(e2, lxstat, path);
  /*GET_FIELD_PTR(e1, lxstat, buf) ==
    GET_FIELD_PTR(e2, lxstat, buf);*/
}

TURN_CHECK_P(lxstat64_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, lxstat64, vers) ==
      GET_FIELD_PTR(e2, lxstat64, vers) &&
    GET_FIELD_PTR(e1, lxstat64, path) ==
      GET_FIELD_PTR(e2, lxstat64, path);
  /*GET_FIELD_PTR(e1, lxstat64, buf) ==
    GET_FIELD_PTR(e2, lxstat64, buf);*/
}

TURN_CHECK_P(malloc_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, malloc, size) ==
      GET_FIELD_PTR(e2, malloc, size);
}

TURN_CHECK_P(mkdir_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, mkdir, pathname) ==
      GET_FIELD_PTR(e2, mkdir, pathname) &&
    GET_FIELD_PTR(e1, mkdir, mode) ==
      GET_FIELD_PTR(e2, mkdir, mode);
}

TURN_CHECK_P(mkstemp_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, mkstemp, temp) ==
      GET_FIELD_PTR(e2, mkstemp, temp);
}

TURN_CHECK_P(mmap_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, mmap, addr) ==
      GET_FIELD_PTR(e2, mmap, addr) &&
    GET_FIELD_PTR(e1, mmap, length) ==
      GET_FIELD_PTR(e2, mmap, length) &&
    GET_FIELD_PTR(e1, mmap, prot) ==
      GET_FIELD_PTR(e2, mmap, prot) &&
    GET_FIELD_PTR(e1, mmap, flags) ==
      GET_FIELD_PTR(e2, mmap, flags) &&
    GET_FIELD_PTR(e1, mmap, fd) ==
      GET_FIELD_PTR(e2, mmap, fd) &&
    GET_FIELD_PTR(e1, mmap, offset) ==
      GET_FIELD_PTR(e2, mmap, offset);
}

TURN_CHECK_P(mmap64_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, mmap64, addr) ==
      GET_FIELD_PTR(e2, mmap64, addr) &&
    GET_FIELD_PTR(e1, mmap64, length) ==
      GET_FIELD_PTR(e2, mmap64, length) &&
    GET_FIELD_PTR(e1, mmap64, prot) ==
      GET_FIELD_PTR(e2, mmap64, prot) &&
    GET_FIELD_PTR(e1, mmap64, flags) ==
      GET_FIELD_PTR(e2, mmap64, flags) &&
    GET_FIELD_PTR(e1, mmap64, fd) ==
      GET_FIELD_PTR(e2, mmap64, fd) &&
    GET_FIELD_PTR(e1, mmap64, offset) ==
      GET_FIELD_PTR(e2, mmap64, offset);
}

TURN_CHECK_P(mremap_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, mremap, old_address) ==
      GET_FIELD_PTR(e2, mremap, old_address) &&
    GET_FIELD_PTR(e1, mremap, old_size) ==
      GET_FIELD_PTR(e2, mremap, old_size) &&
    GET_FIELD_PTR(e1, mremap, new_size) ==
      GET_FIELD_PTR(e2, mremap, new_size) &&
    GET_FIELD_PTR(e1, mremap, flags) ==
      GET_FIELD_PTR(e2, mremap, flags);
}

TURN_CHECK_P(munmap_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, munmap, addr) ==
      GET_FIELD_PTR(e2, munmap, addr) &&
    GET_FIELD_PTR(e1, munmap, length) ==
      GET_FIELD_PTR(e2, munmap, length);
}

TURN_CHECK_P(open_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, open, path) ==
      GET_FIELD_PTR(e2, open, path) &&
    GET_FIELD_PTR(e1, open, flags) ==
      GET_FIELD_PTR(e2, open, flags) &&
    GET_FIELD_PTR(e1, open, open_mode) ==
      GET_FIELD_PTR(e2, open, open_mode);
}

TURN_CHECK_P(open64_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, open64, path) ==
      GET_FIELD_PTR(e2, open64, path) &&
    GET_FIELD_PTR(e1, open64, flags) ==
      GET_FIELD_PTR(e2, open64, flags) &&
    GET_FIELD_PTR(e1, open64, open_mode) ==
      GET_FIELD_PTR(e2, open64, open_mode);
}

TURN_CHECK_P(openat_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, openat, dirfd) ==
      GET_FIELD_PTR(e2, openat, dirfd) &&
    GET_FIELD_PTR(e1, openat, pathname) ==
      GET_FIELD_PTR(e2, openat, pathname) &&
    GET_FIELD_PTR(e1, openat, flags) ==
      GET_FIELD_PTR(e2, openat, flags);

}

TURN_CHECK_P(opendir_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, opendir, name) ==
      GET_FIELD_PTR(e2, opendir, name);
}

TURN_CHECK_P(pread_turn_check)
{
  return base_turn_check(e1, e2) &&
    /*GET_FIELD_PTR(e1, pread, fd) ==
      GET_FIELD_PTR(e2, pread, fd) &&*/
    GET_FIELD_PTR(e1, pread, buf) ==
      GET_FIELD_PTR(e2, pread, buf) &&
    GET_FIELD_PTR(e1, pread, count) ==
      GET_FIELD_PTR(e2, pread, count) &&
    GET_FIELD_PTR(e1, pread, offset) ==
      GET_FIELD_PTR(e2, pread, offset);
}

TURN_CHECK_P(putc_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, putc, c) ==
      GET_FIELD_PTR(e2, putc, c) &&
    GET_FIELD_PTR(e1, putc, stream) ==
      GET_FIELD_PTR(e2, putc, stream);
}

TURN_CHECK_P(pwrite_turn_check)
{
  return base_turn_check(e1, e2) &&
    /*GET_FIELD_PTR(e1, pwrite, fd) ==
      GET_FIELD_PTR(e2, pwrite, fd) &&*/
    GET_FIELD_PTR(e1, pwrite, buf) ==
      GET_FIELD_PTR(e2, pwrite, buf) &&
    GET_FIELD_PTR(e1, pwrite, count) ==
      GET_FIELD_PTR(e2, pwrite, count) &&
    GET_FIELD_PTR(e1, pwrite, offset) ==
      GET_FIELD_PTR(e2, pwrite, offset);
}

TURN_CHECK_P(libc_memalign_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, libc_memalign, boundary) ==
      GET_FIELD_PTR(e2, libc_memalign, boundary) &&
    GET_FIELD_PTR(e1, libc_memalign, size) ==
      GET_FIELD_PTR(e2, libc_memalign, size);
}

TURN_CHECK_P(free_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, free, ptr) ==
      GET_FIELD_PTR(e2, free, ptr);
}

TURN_CHECK_P(ftell_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, ftell, stream) ==
      GET_FIELD_PTR(e2, ftell, stream);
}

TURN_CHECK_P(fwrite_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, fwrite, ptr) ==
      GET_FIELD_PTR(e2, fwrite, ptr) &&
    GET_FIELD_PTR(e1, fwrite, size) ==
      GET_FIELD_PTR(e2, fwrite, size) &&
    GET_FIELD_PTR(e1, fwrite, nmemb) ==
      GET_FIELD_PTR(e2, fwrite, nmemb) &&
    GET_FIELD_PTR(e1, fwrite, stream) ==
      GET_FIELD_PTR(e2, fwrite, stream);
}

TURN_CHECK_P(fread_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, fread, ptr) ==
      GET_FIELD_PTR(e2, fread, ptr) &&
    GET_FIELD_PTR(e1, fread, size) ==
      GET_FIELD_PTR(e2, fread, size) &&
    GET_FIELD_PTR(e1, fread, nmemb) ==
      GET_FIELD_PTR(e2, fread, nmemb) &&
    GET_FIELD_PTR(e1, fread, stream) ==
      GET_FIELD_PTR(e2, fread, stream);
}

TURN_CHECK_P(fsync_turn_check)
{
  return base_turn_check(e1, e2);/* &&
    GET_FIELD_PTR(e1, fsync, fd) ==
    GET_FIELD_PTR(e2, fsync, fd);*/
}

TURN_CHECK_P(fxstat_turn_check)
{
  return base_turn_check(e1, e2) &&
    /*GET_FIELD_PTR(e1, fxstat, fd) ==
      GET_FIELD_PTR(e2, fxstat, fd) &&*/
    GET_FIELD_PTR(e1, fxstat, vers) ==
      GET_FIELD_PTR(e2, fxstat, vers);
    /*GET_FIELD_PTR(e1, fxstat, buf) ==
      GET_FIELD_PTR(e2, fxstat, buf);*/
}

TURN_CHECK_P(fxstat64_turn_check)
{
  return base_turn_check(e1, e2) &&
    /*GET_FIELD_PTR(e1, fxstat64, fd) ==
      GET_FIELD_PTR(e2, fxstat64, fd) &&*/
    GET_FIELD_PTR(e1, fxstat64, vers) ==
      GET_FIELD_PTR(e2, fxstat64, vers);
    /*GET_FIELD_PTR(e1, fxstat64, buf) ==
      GET_FIELD_PTR(e2, fxstat64, buf);*/
}

TURN_CHECK_P(realloc_turn_check)
{
  return base_turn_check(e1, e2) &&
    GET_FIELD_PTR(e1, realloc, ptr) ==
      GET_FIELD_PTR(e2, realloc, ptr) &&
    GET_FIELD_PTR(e1, realloc, size) ==
      GET_FIELD_PTR(e2, realloc, size);
}

TURN_CHECK_P(rename_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, rename, oldpath) ==
      GET_FIELD_PTR(e2, rename, oldpath) &&
    GET_FIELD_PTR(e1, rename, newpath) ==
      GET_FIELD_PTR(e2, rename, newpath);
}

TURN_CHECK_P(rewind_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, rewind, stream) ==
      GET_FIELD_PTR(e2, rewind, stream);
}

TURN_CHECK_P(rmdir_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, rmdir, pathname) ==
      GET_FIELD_PTR(e2, rmdir, pathname);
}

TURN_CHECK_P(select_turn_check)
{
  return base_turn_check(e1,e2) &&
    GET_FIELD_PTR(e1, select, nfds) ==
      GET_FIELD_PTR(e2, select, nfds) &&
    GET_FIELD_PTR(e1, select, exceptfds) ==
      GET_FIELD_PTR(e2, select, exceptfds) &&
    GET_FIELD_PTR(e1, select, timeout) ==
      GET_FIELD_PTR(e2, select, timeout);
}

TURN_CHECK_P(epoll_create_turn_check)
{
  return base_turn_check(e1,e2) &&
    IS_EQUAL_FIELD_PTR(e1, e2, epoll_create, size);
}

TURN_CHECK_P(epoll_create1_turn_check)
{
  return base_turn_check(e1,e2) &&
    IS_EQUAL_FIELD_PTR(e1, e2, epoll_create1, flags);
}

TURN_CHECK_P(epoll_ctl_turn_check)
{
  return base_turn_check(e1,e2) &&
    IS_EQUAL_FIELD_PTR(e1, e2, epoll_ctl, epfd) &&
    IS_EQUAL_FIELD_PTR(e1, e2, epoll_ctl, op) &&
    IS_EQUAL_FIELD_PTR(e1, e2, epoll_ctl, fd) &&
    IS_EQUAL_FIELD_PTR(e1, e2, epoll_ctl, _event);
}

TURN_CHECK_P(epoll_wait_turn_check)
{
  return base_turn_check(e1,e2) &&
    IS_EQUAL_FIELD_PTR(e1, e2, epoll_wait, epfd) &&
    IS_EQUAL_FIELD_PTR(e1, e2, epoll_wait, events) &&
    IS_EQUAL_FIELD_PTR(e1, e2, epoll_wait, maxevents) &&
    IS_EQUAL_FIELD_PTR(e1, e2, epoll_wait, timeout);
}

TURN_CHECK_P(getpwnam_r_turn_check)
{
  return base_turn_check(e1,e2) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getpwnam_r, name) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getpwnam_r, pwd) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getpwnam_r, buf) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getpwnam_r, buflen) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getpwnam_r, result);
}

TURN_CHECK_P(getpwuid_r_turn_check)
{
  return base_turn_check(e1,e2) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getpwuid_r, uid) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getpwuid_r, pwd) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getpwuid_r, buf) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getpwuid_r, buflen) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getpwuid_r, result);
}

TURN_CHECK_P(getgrnam_r_turn_check)
{
  return base_turn_check(e1,e2) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getgrnam_r, name) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getgrnam_r, grp) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getgrnam_r, buf) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getgrnam_r, buflen) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getgrnam_r, result);
}
TURN_CHECK_P(getgrgid_r_turn_check)
{
  return base_turn_check(e1,e2) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getgrgid_r, gid) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getgrgid_r, grp) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getgrgid_r, buf) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getgrgid_r, buflen) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getgrgid_r, result);
}

TURN_CHECK_P(getaddrinfo_turn_check)
{
  return base_turn_check(e1,e2) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getaddrinfo, node) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getaddrinfo, service) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getaddrinfo, hints) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getaddrinfo, res);
}

TURN_CHECK_P(freeaddrinfo_turn_check)
{
  return base_turn_check(e1,e2) &&
    IS_EQUAL_FIELD_PTR(e1, e2, freeaddrinfo, res);
}

TURN_CHECK_P(getnameinfo_turn_check)
{
  return base_turn_check(e1,e2) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getnameinfo, sa) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getnameinfo, salen) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getnameinfo, host) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getnameinfo, hostlen) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getnameinfo, serv) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getnameinfo, servlen) &&
    IS_EQUAL_FIELD_PTR(e1, e2, getnameinfo, flags);
}

/* Returns true if 'opt_event' is a registered optional event for 'event'.
 * If 'query' is true, 'opt_event' is ignored, and returns true if 'event' has
 * any registered optional events.
 * This means you can call is_optional_event_for(evt, unknown_event, true)
 * to query if 'evt' has any optional events. */
static inline bool is_optional_event_for(event_code_t event,
                                         event_code_t opt_event,
                                         bool query)
{
  /* Group together events that share the same optional events. */
  switch (event) {
  case getcwd_event:
  case opendir_event:
    return query || opt_event == malloc_event;
  case pthread_cond_signal_event:
  case pthread_cond_timedwait_event:
  case pthread_cond_wait_event:
  case pthread_cond_broadcast_event:
    return query || opt_event == calloc_event;
  case closedir_event:
  case fclose_event:
    return query || opt_event == free_event;
  case accept4_event:
  case accept_event:
  case fgets_event:
  case fgetc_event:
  case ferror_event:
  case feof_event:
  case fileno_event:
  case fprintf_event:
  case fputc_event:
  case fputs_event:
  case fscanf_event:
  case fseek_event:
  case fwrite_event:
  case fread_event:
  case getc_event:
    return query || opt_event == mmap_event;
  case fdopen_event:
    return query || opt_event == mmap_event || opt_event == malloc_event;
  case fopen64_event:
  case fopen_event:
  case freopen_event:
  case getsockopt_event:
  case localtime_event:
  case setsockopt_event:
  case tmpfile_event:
    return query || opt_event == malloc_event ||
      opt_event == free_event || opt_event == mmap_event;
  case freeaddrinfo_event:
  case getaddrinfo_event:
  case getgrgid_r_event:
  case getgrnam_r_event:
  case getnameinfo_event:
  case getpwnam_r_event:
  case getpwuid_r_event:
    return query || opt_event == malloc_event ||
      opt_event == free_event || opt_event == calloc_event ||
      opt_event == mmap_event;
  default:
     return false;
  }
}

/* Given the event number of an optional event, executes the action to fulfill
   that event. */
static void execute_optional_event(int opt_event_num)
{
  log_entry_t temp_entry = EMPTY_LOG_ENTRY;
  global_log.getCurrentEntry(temp_entry);

  if (opt_event_num == mmap_event) {
    size_t length = GET_FIELD(temp_entry, mmap, length);
    int prot      = GET_FIELD(temp_entry, mmap, prot);
    int flags     = GET_FIELD(temp_entry, mmap, flags);
    int fd        = GET_FIELD(temp_entry, mmap, fd);
    off_t offset  = GET_FIELD(temp_entry, mmap, offset);
    JASSERT(mmap(NULL, length, prot, flags, fd, offset) != MAP_FAILED);
  } else if (opt_event_num == malloc_event) {
    size_t size = GET_FIELD(temp_entry, malloc, size);
    JASSERT(malloc(size) != NULL);
  } else if (opt_event_num == free_event) {
    /* The fact that this works depends on memory-accurate replay. */
    void *ptr = (void *)GET_FIELD(temp_entry, free, ptr);
    free(ptr);
  } else if (opt_event_num == calloc_event) {
    size_t size = GET_FIELD(temp_entry, calloc, size);
    size_t nmemb = GET_FIELD(temp_entry, calloc, nmemb);
    JASSERT(calloc(nmemb, size) != NULL);
  } else {
    JASSERT (false)(opt_event_num).Text("No action known for optional event.");
  }
}

/* Waits until the head of the log contains an entry matching pertinent fields
   of 'my_entry'. When it does, 'my_entry' is modified to point to the head of
   the log. */
void waitForTurn(log_entry_t *my_entry, turn_pred_t pred)
{
  log_entry_t temp_entry = EMPTY_LOG_ENTRY;
  memfence();

  while (1) {
    global_log.getCurrentEntry(temp_entry);
    if ((*pred)(&temp_entry, my_entry))
      break;
    /* Also check for an optional event for this clone_id. */
    if (GET_COMMON(temp_entry, clone_id) == my_clone_id &&
        GET_COMMON(temp_entry, isOptional) == 1) {
      if (!is_optional_event_for((event_code_t)GET_COMMON_PTR(my_entry, event),
                                 (event_code_t)GET_COMMON(temp_entry, event),
                                 false)) {
        JASSERT(false);
      }
      execute_optional_event(GET_COMMON(temp_entry, event));
    }

    memfence();
    usleep(1);
  }

  global_log.getCurrentEntry(*my_entry);
}

void waitForExecBarrier()
{
  log_entry_t temp_entry = EMPTY_LOG_ENTRY;
  while (1) {
    global_log.getCurrentEntry(temp_entry);
    if (GET_COMMON(temp_entry, event) == exec_barrier_event) {
      // We don't check clone ids because anyone can do an exec.
      break;
    }
    memfence();
    usleep(20);
  }
}

/* A do-nothing event that can be called from user-space via
   dmtcp_userSynchronizedEvent() in the dmtcp aware api library. This
   provides extra coarse-grained synchronization for the user program
   where a mutex is not wanted. */
void userSynchronizedEvent()
{
  log_entry_t my_entry = create_user_entry(my_clone_id, user_event);
  if (SYNC_IS_REPLAY) {
    waitForTurn(&my_entry, user_turn_check);
    getNextLogEntry();
  } else if (SYNC_IS_RECORD) {
    addNextLogEntry(my_entry);
  }
}

void userSynchronizedEventBegin()
{
  log_entry_t my_entry = create_user_entry(my_clone_id, user_event);
  if (SYNC_IS_REPLAY) {
    waitForTurn(&my_entry, user_turn_check);
    getNextLogEntry();
  } else if (SYNC_IS_RECORD) {
    addNextLogEntry(my_entry);
  }
}

void userSynchronizedEventEnd()
{
  JASSERT(false);
  return;
}
