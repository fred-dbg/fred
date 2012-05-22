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
#include <sys/resource.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <algorithm>
#include "fred_wrappers.h"
#include "dmtcpplugin.h"
#include "util.h"
#include  "jassert.h"
#include  "jtimer.h"
#include  "jfilesystem.h"
#include <sys/select.h>
#include "synchronizationlogging.h"
#include "log.h"
#include "threadinfo.h"


// TODO: Do we need LIB_PRIVATE again here if we had already specified it in
// the header file?
/* Library private: */
LIB_PRIVATE dmtcp::map<pthread_t, pthread_join_retval_t> pthread_join_retvals;
LIB_PRIVATE char RECORD_LOG_PATH[RECORD_LOG_PATH_MAX];
LIB_PRIVATE char RECORD_READ_DATA_LOG_PATH[RECORD_LOG_PATH_MAX];
LIB_PRIVATE int             read_data_fd = -1;
LIB_PRIVATE int             sync_logging_branch = 0;

/* Setting this will log/replay *ALL* malloc family
   functions (i.e. including ones from DMTCP, std C++ lib, etc.). */
LIB_PRIVATE int             log_all_allocs = 0;
LIB_PRIVATE pthread_mutex_t read_data_mutex = PTHREAD_MUTEX_INITIALIZER;

LIB_PRIVATE dmtcp::SynchronizationLog global_log;

/* Volatiles: */
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

int get_sync_mode()
{
  return sync_logging_branch;
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
  if (dmtcp::ThreadInfo::isOkToLogNextFnc()) {
    dmtcp::ThreadInfo::unsetOkToLogNextFnc();
    return 1;
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

void initLogsForRecordReplay()
{
  global_log.initialize(RECORD_LOG_PATH, MAX_LOG_LENGTH);

  if (read_data_fd == -1) {
    int fd;
    JASSERT(SYNC_IS_RECORD || SYNC_IS_REPLAY);
    if (SYNC_IS_RECORD) {
      fd = _real_open(RECORD_READ_DATA_LOG_PATH,
                      O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    } else if (SYNC_IS_REPLAY) {
      fd = _real_open(RECORD_READ_DATA_LOG_PATH, O_RDONLY, 0);
    } else {
      JASSERT(false) .Text("Not Reached");
    }
    JASSERT(fd != -1);
    read_data_fd = _real_dup2(fd, dmtcp_get_readlog_fd());
    JASSERT(read_data_fd != -1);
    _real_close(fd);
  }
}


#define MAX_PROC_MAPS_AREAS 32
static dmtcp::Util::ProcMapsArea areasToNotLog[MAX_PROC_MAPS_AREAS];
static size_t areasToNotLogLen = 0;

/* Specify the patterns that you do not wish to log. The current logic uses
 * strStartsWith() and so add accordingly.
 */
static const char *logLibraryBlacklistPattern[] = {
  "/libc-2",
  "/libpthread-2",
  // FIXME: Shouldn't we log libdl and ld.so as well?
  "/libdl-2",
  "/ld-2"
};

static bool shouldLogArea(char *area_name)
{
  int n = sizeof(logLibraryBlacklistPattern) / sizeof(char*);

  if (dmtcp::Util::strEndsWith(area_name, "dmtcphijack.so")  ||
      dmtcp::Util::strEndsWith(area_name, "ptracehijack.so") ||
      dmtcp::Util::strEndsWith(area_name, "pidvirt.so")      ||
      dmtcp::Util::strEndsWith(area_name, "fredhijack.so")   ||
      strstr(area_name, "libmtcp.so.1") != NULL) {
    return false;
  }

  if (dmtcp::Util::strStartsWith(area_name, "/lib/") ||
      dmtcp::Util::strStartsWith(area_name, "/lib64/") ||
      dmtcp::Util::strStartsWith(area_name, "/lib32/")) {
    for (int i = 0; i < n; i++) {
      if (strstr(area_name, logLibraryBlacklistPattern[i]) != NULL) {
        return false;
      }
    }
  }
  return true;
}

LIB_PRIVATE
void initSyncAddresses()
{
  int mapsFd = -1;
  dmtcp::Util::ProcMapsArea area;

  if (isProcessGDB()) {
    return;
  }
  if (areasToNotLogLen > 0) {
    return;
  }

  if ((mapsFd = _real_open("/proc/self/maps", O_RDONLY, S_IRUSR)) == -1) {
    perror("open");
    exit(1);
  }

  while (dmtcp::Util::readProcMapsLine(mapsFd, &area)) {
    if ((area.prot & PROT_EXEC) != 0  && strlen(area.name) != 0 &&
        shouldLogArea(area.name) == false) {
      JASSERT(areasToNotLogLen < MAX_PROC_MAPS_AREAS) (areasToNotLogLen);
      areasToNotLog[areasToNotLogLen++] = area;
    }
  }
  _real_close(mapsFd);

  for (int i = 1; i < areasToNotLogLen; i++) {
    JASSERT(areasToNotLog[i].addr >= areasToNotLog[i-1].endAddr)
      (areasToNotLog[i].name) (areasToNotLog[i].addr) (areasToNotLog[i].endAddr)
      (areasToNotLog[i-1].name) (areasToNotLog[i-1].addr)
      (areasToNotLog[i-1].endAddr)
      .Text ("ERROR: Blacklisted library addresses not in ascending order.");
  }
}

LIB_PRIVATE
bool validAddress(void *addr)
{
  if (areasToNotLogLen == 0) {
    initSyncAddresses();
  }

  // Most user libraries are loaded at a lower memory address than the
  // blacklisted libraries.
  if (addr < areasToNotLog[0].addr ||
      addr >= areasToNotLog[areasToNotLogLen - 1].endAddr) {
    return true;
  }

  // Now do a binary search
  int min = 0;
  int max = areasToNotLogLen - 1;
  while (max >= min) {
    int mid = (min + max) / 2;
    if (addr >= areasToNotLog[mid].addr &&
        addr <  areasToNotLog[mid].endAddr) {
      return false;
    }
    if (addr < areasToNotLog[mid].addr) {
      max = mid - 1;
    } else {
      min = mid + 1;
    }
  }

  return true;
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

/* Close the "read log", which is done before a checkpoint is taken. */
void close_read_log()
{
  _real_close(read_data_fd);
  read_data_fd = -1;
}

void addNextLogEntry(log_entry_t& e)
{
  JASSERT(my_clone_id != -1);
  if (GET_COMMON(e, log_offset) == INVALID_LOG_OFFSET) {
    global_log.appendEntry(e);
  } else {
    global_log.updateEntry(e);
  }
}

void getNextLogEntry()
{
  JASSERT(my_clone_id != -1);
  // If log is empty, don't do anything
  if (global_log.numEntries() == 0) {
    return;
  }
  if (global_log.advanceToNextEntry() == 0) {
    JTRACE ( "Switching back to record." );
    set_sync_mode(SYNC_RECORD);
  } else {
    memfence();
    const log_entry_t& temp_entry = global_log.getCurrentEntry();
    clone_id_t clone_id = GET_COMMON(temp_entry, clone_id);
    dmtcp::ThreadInfo::wakeUpThread(clone_id);
  }
}

void logReadData(void *buf, int count)
{
  if (SYNC_IS_REPLAY) {
    JASSERT (false).Text("Asked to log read data while in replay. "
        "This is probably not intended.");
  }
  JASSERT(read_data_fd != -1);
  int written = _real_write(read_data_fd, buf, count);
  JASSERT ( written == count );
  read_log_pos += written;
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
  case vfprintf_event:
  case fputc_event:
  case fputs_event:
  case vfscanf_event:
  case fseek_event:
  case fwrite_event:
  case fread_event:
  case getc_event:
  case malloc_event:
  case calloc_event:
  case realloc_event:
    return query || opt_event == mmap_event;
  case fdopen_event:
    return query || opt_event == mmap_event || opt_event == malloc_event;
  case fopen64_event:
  case fopen_event:
  case freopen_event:
  case getsockopt_event:
  case localtime_r_event:
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
      opt_event == mmap_event || opt_event == ftell_event ||
      opt_event == fopen_event || opt_event == fclose_event;
  case getdelim_event:
    return query || opt_event == malloc_event || opt_event == realloc_event ||
      opt_event == mmap_event;
  default:
     return false;
  }
}

/* Given the event number of an optional event, executes the action to fulfill
   that event. */
static void execute_optional_event(int opt_event_num)
{
  const log_entry_t& temp_entry = global_log.getCurrentEntry();

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
  } else if (opt_event_num == fopen_event) {
    const char *path = GET_FIELD(temp_entry, fopen, path);
    const char *mode = GET_FIELD(temp_entry, fopen, mode);
    dmtcp::ThreadInfo::setOkToLogNextFnc();
    FILE *fp = fopen(path, mode);
  } else if (opt_event_num == fclose_event) {
    FILE *fp = GET_FIELD(temp_entry, fclose, fp);
    dmtcp::ThreadInfo::setOkToLogNextFnc();
    fclose(fp);
  } else if (opt_event_num == ftell_event) {
    FILE *fp = GET_FIELD(temp_entry, ftell, stream);
    dmtcp::ThreadInfo::setOkToLogNextFnc();
    // No need to execute ftell().  It has no side effects.
    long int offset = ftell(fp);
  } else {
    JASSERT (false)(opt_event_num).Text("No action known for optional event.");
  }
}

/* Waits until the head of the log contains an entry matching pertinent fields
   of 'my_entry'. When it does, 'my_entry' is modified to point to the head of
   the log. */
void waitForTurn(log_entry_t *my_entry, turn_pred_t pred)
{
  memfence();

  JASSERT(my_clone_id != -1);
  while (1) {
    dmtcp::ThreadInfo::waitForTurn();
    memfence();
    log_entry_t& temp_entry = global_log.getCurrentEntry();
    JASSERT(GET_COMMON(temp_entry, clone_id) == my_clone_id)
      (GET_COMMON(temp_entry, clone_id)) (my_clone_id);
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
      memfence();
      dmtcp::ThreadInfo::wakeUpThread(my_clone_id);
      execute_optional_event(GET_COMMON(temp_entry, event));
    }
  }

  *my_entry = global_log.getCurrentEntry();
}

void waitForExecBarrier()
{
  while (1) {
    const log_entry_t& temp_entry = global_log.getCurrentEntry();
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
