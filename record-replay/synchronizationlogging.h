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

#ifndef SYNCHRONIZATION_LOGGING_H
#define SYNCHRONIZATION_LOGGING_H

// Needed for getpeername() etc.
#include <sys/socket.h>
// Needed for *xstat() to store 'struct stat' fields.
#include <sys/stat.h>
// Needed for readdir:
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <poll.h>
#include <pwd.h>
#include <grp.h>
// Needed for ioctl:
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <net/if.h>
#include <netdb.h>
#include <signal.h>
#include <semaphore.h>

#include "constants.h"
#include "dmtcpalloc.h"
#include "util.h"
#include "dmtcpplugin.h"
#include "jfilesystem.h"
#include "fred_wrappers.h"
#include "threadinfo.h"

#undef WRAPPER_EXECUTION_ENABLE_CKPT
#undef WRAPPER_EXECUTION_DISABLE_CKPT
#define WRAPPER_EXECUTION_ENABLE_CKPT DMTCP_PLUGIN_ENABLE_CKPT
#define WRAPPER_EXECUTION_DISABLE_CKPT DMTCP_PLUGIN_DISABLE_CKPT

namespace dmtcp { class SynchronizationLog; }

static inline bool isProcessGDB() {
  static dmtcp::string progName = jalib::Filesystem::GetProgramName();
  static bool isGDB = progName == "gdb";
  return isGDB;
}

#define LIB_PRIVATE __attribute__ ((visibility ("hidden")))

#define MAX_LOG_LENGTH ((size_t)250 * 1024 * 1024)
#define INVALID_LOG_OFFSET (~0U)
#define SYNC_NOOP   0
#define SYNC_RECORD 1
#define SYNC_REPLAY 2
#define SYNC_IS_REPLAY    (sync_logging_branch == SYNC_REPLAY)
#define SYNC_IS_RECORD    (sync_logging_branch == SYNC_RECORD)
#define SYNC_IS_NOOP      (sync_logging_branch == SYNC_NOOP)
#define GET_RETURN_ADDRESS() __builtin_return_address(0)

#define TURN_CHECK_P(name) int name(log_entry_t *e1, log_entry_t *e2)

#define SYNC_TIMINGS

#ifdef SYNC_TIMINGS
/* To be used when the timer is started and finished in the same function. */
#define SYNC_TIMER_START(name)                  \
  struct timeval name##_start;                  \
  gettimeofday(&name##_start, NULL);

/* To be used when the timer is started in one function and finished
 * in another. The struct timeval should be declared in this file. */
#define SYNC_TIMER_START_GLOBAL(name)           \
  gettimeofday(&name##_start, NULL);

#define SYNC_TIMER_STOP(name)                                           \
  struct timeval name##_end;                                            \
  gettimeofday(&name##_end, NULL);                                      \
  double name##_sec = name##_end.tv_sec - name##_start.tv_sec;          \
  name##_sec += (name##_end.tv_usec - name##_start.tv_usec)/1000000.0;  \
  JNOTE ( "Timer " #name ) ( name##_sec );
#else
#define SYNC_TIMER_START(name)
#define SYNC_TIMER_STOP(name)
#endif

#define WRAPPER_HEADER_VOID_RAW(name, real_func, ...)                   \
  void *return_addr = GET_RETURN_ADDRESS();                             \
  do {                                                                  \
    if (!shouldSynchronize(return_addr) || isProcessGDB()) {            \
      real_func(__VA_ARGS__);                                           \
      return;                                                           \
    }                                                                   \
  } while(0)

#define WRAPPER_HEADER_RAW(ret_type, name, real_func, ...)              \
  void *return_addr = GET_RETURN_ADDRESS();                             \
  do {                                                                  \
    if (!shouldSynchronize(return_addr) || isProcessGDB()) {            \
      return real_func(__VA_ARGS__);                                    \
    }                                                                   \
  } while(0)

#define WRAPPER_HEADER_NO_RETURN(name, real_func, ...)                  \
  void *return_addr = GET_RETURN_ADDRESS();                             \
  if (!shouldSynchronize(return_addr) || isProcessGDB()) {              \
    real_func(__VA_ARGS__);                                             \
  }                                                                     \
  log_entry_t my_entry = create_##name##_entry(my_clone_id,             \
      name##_event, __VA_ARGS__);

#define WRAPPER_HEADER(ret_type, name, real_func, ...)                  \
  WRAPPER_HEADER_RAW(ret_type, name, real_func, ##__VA_ARGS__);         \
  ret_type retval;                                                      \
  log_entry_t my_entry = create_##name##_entry(my_clone_id,             \
      name##_event, ##__VA_ARGS__);

#define WRAPPER_HEADER_CKPT_DISABLED(ret_type, name, real_func, ...)    \
  void *return_addr = GET_RETURN_ADDRESS();                             \
  ret_type retval;                                                      \
  if (!shouldSynchronize(return_addr) || isProcessGDB()) {              \
    retval = real_func(__VA_ARGS__);                                    \
    WRAPPER_EXECUTION_ENABLE_CKPT();                                    \
    return retval;                                                      \
    }                                                                   \
    log_entry_t my_entry = create_##name##_entry(my_clone_id,           \
      name##_event, __VA_ARGS__);

#define WRAPPER_HEADER_VOID(name, real_func, ...)                     \
  WRAPPER_HEADER_VOID_RAW(name, real_func, __VA_ARGS__);              \
  log_entry_t my_entry = create_##name##_entry(my_clone_id,           \
      name##_event, __VA_ARGS__);

#define WRAPPER_REPLAY_START_TYPED(ret_type, name)                                    \
  do {                                                                \
    waitForTurn(&my_entry, &name##_turn_check);                        \
    retval = (ret_type) (unsigned long) GET_COMMON(my_entry,          \
                                                   retval);           \
  } while (0)

#define WRAPPER_REPLAY_START(name)                                    \
  WRAPPER_REPLAY_START_TYPED(int, name)

#define WRAPPER_REPLAY_END(name)                                      \
  do {                                                                \
    int saved_errno = GET_COMMON(my_entry, my_errno);                 \
    getNextLogEntry();                                              \
    if (saved_errno != 0) {                                         \
      errno = saved_errno;                                          \
    }                                                               \
  } while (0)


#define WRAPPER_REPLAY_TYPED(ret_type, name)                        \
  do {                                                              \
    WRAPPER_REPLAY_START_TYPED(ret_type, name);                     \
    WRAPPER_REPLAY_END(name);                                       \
  } while (0)

#define WRAPPER_REPLAY(name) WRAPPER_REPLAY_TYPED(int, name)

#define WRAPPER_REPLAY_VOID(name)                                   \
  do {                                                              \
    waitForTurn(&my_entry, &name##_turn_check);                     \
    int saved_errno = GET_COMMON(my_entry, my_errno);               \
    getNextLogEntry();                                              \
    if (saved_errno != 0) {                                         \
      errno = saved_errno;                                          \
    }                                                               \
  } while (0)

#define WRAPPER_REPLAY_READ_FROM_READ_LOG(name, ptr, len)               \
  do {                                                                  \
    JASSERT ( read_data_fd != -1 );                                     \
    lseek(read_data_fd,                                                 \
          GET_FIELD(my_entry, name, data_offset), SEEK_SET);            \
    ssize_t dmtcp_retval = dmtcp::Util::readAll(read_data_fd, ptr, len); \
    /* Hackish. Instead of JASSERTing here, enter infinite loop.        \
       This is for the benefit of fredtest.py, which cannot currently   \
       detect if a program exited normally or abnormally. */            \
    if ( dmtcp_retval != len ) while (1);                               \
  } while (0)

#define WRAPPER_LOG_WRITE_INTO_READ_LOG(name, ptr, len)             \
  do {                                                              \
    int saved_errno = errno;                                        \
    _real_pthread_mutex_lock(&read_data_mutex);                     \
    SET_FIELD2(my_entry, name, data_offset, read_log_pos);          \
    logReadData(ptr, len);                                          \
    _real_pthread_mutex_unlock(&read_data_mutex);                   \
    errno = saved_errno;                                            \
  } while (0)

#define WRAPPER_REPLAY_READ_VECTOR_FROM_READ_LOG(name, iov, iovcnt)           \
  do {                                                              \
    JASSERT ( read_data_fd != -1 );                                 \
    lseek(read_data_fd,                                             \
          GET_FIELD(my_entry, name, data_offset), SEEK_SET);        \
    JASSERT(_real_readv(read_data_fd, iov, iovcnt) != -1);          \
  } while (0)

#define WRAPPER_LOG_WRITE_VECTOR_INTO_READ_LOG(name, iov, iovcnt, retval) \
  do {                                                              \
    int saved_errno = errno;                                        \
    if (SYNC_IS_REPLAY) {                                           \
      JASSERT (false).Text("Asked to log read data while in replay."\
                           "\nThis is probably not intended.");     \
    }                                                               \
    JASSERT ( read_data_fd != -1 );                                 \
    _real_pthread_mutex_lock(&read_data_mutex);                     \
    SET_FIELD2(my_entry, name, data_offset, read_log_pos);          \
    int written = _real_writev(read_data_fd, iov, iovcnt);          \
    JASSERT ( written >= retval );                                  \
    read_log_pos += written;                                        \
    _real_pthread_mutex_unlock(&read_data_mutex);                   \
    errno = saved_errno;                                            \
  } while (0)


#define WRAPPER_LOG_WRITE_ENTRY_VOID(my_entry)                      \
  do {                                                              \
    SET_COMMON2(my_entry, my_errno, errno);                         \
    SET_COMMON2(my_entry, isOptional,                               \
                dmtcp::ThreadInfo::isOptionalEvent());              \
    addNextLogEntry(my_entry);                                      \
    errno = GET_COMMON(my_entry, my_errno);                         \
  } while (0)

#define WRAPPER_LOG_WRITE_ENTRY(my_entry)                           \
  do {                                                              \
    SET_COMMON2(my_entry, retval, (void*)(unsigned long)retval);    \
    WRAPPER_LOG_WRITE_ENTRY_VOID(my_entry);                         \
  } while (0)

#define WRAPPER_LOG_UPDATE_ENTRY WRAPPER_LOG_WRITE_ENTRY

#define WRAPPER_LOG(real_func, ...)                                 \
  do {                                                              \
    retval = real_func(__VA_ARGS__);                                \
    WRAPPER_LOG_WRITE_ENTRY(my_entry);                              \
  } while (0)

#define WRAPPER_LOG_VOID(real_func, ...)                            \
  do {                                                              \
    real_func(__VA_ARGS__);                                         \
    SET_COMMON2(my_entry, my_errno, errno);                         \
    addNextLogEntry(my_entry);                                      \
    errno = GET_COMMON(my_entry, my_errno);                         \
  } while (0)


/* Your basic record wrapper template. Does not call _real_func on
   replay, but restores the return value and errno from the log. Also, the
   create_func_entry() function must handle the variable arguments and casting
   to correct types. */

#define BASIC_SYNC_WRAPPER_WITH_CKPT_LOCK(ret_type, name, real_func, ...)\
  WRAPPER_EXECUTION_DISABLE_CKPT();                                 \
  WRAPPER_HEADER_CKPT_DISABLED(ret_type, name, real_func,           \
                               __VA_ARGS__);                        \
  if (SYNC_IS_REPLAY) {                                             \
    WRAPPER_REPLAY_TYPED(ret_type, name);                           \
  } else if (SYNC_IS_RECORD) {                                         \
    WRAPPER_LOG(real_func, __VA_ARGS__);                            \
  }                                                                 \
  WRAPPER_EXECUTION_ENABLE_CKPT();                                  \
  return retval;

#define BASIC_SYNC_WRAPPER(ret_type, name, real_func, ...)          \
  WRAPPER_HEADER(ret_type, name, real_func, __VA_ARGS__);           \
  if (SYNC_IS_REPLAY) {                                             \
    WRAPPER_REPLAY_TYPED(ret_type, name);                           \
  } else if (SYNC_IS_RECORD) {                                         \
    WRAPPER_LOG(real_func, __VA_ARGS__);                            \
  }                                                                 \
  return retval;

#define BASIC_SYNC_WRAPPER_NO_RETURN(ret_type, name, real_func, ...)          \
  WRAPPER_HEADER(ret_type, name, real_func, __VA_ARGS__);           \
  if (SYNC_IS_REPLAY) {                                             \
    WRAPPER_REPLAY_TYPED(ret_type, name);                           \
  } else if (SYNC_IS_RECORD) {                                         \
    WRAPPER_LOG(real_func, __VA_ARGS__);                            \
  }                                                                 \

#define BASIC_SYNC_WRAPPER_VOID(name, real_func, ...)               \
  WRAPPER_HEADER_VOID(name, real_func, __VA_ARGS__);                \
  if (SYNC_IS_REPLAY) {                                             \
    WRAPPER_REPLAY_VOID(name);                                      \
  } else if (SYNC_IS_RECORD) {                                         \
    WRAPPER_LOG_VOID(real_func, __VA_ARGS__);                       \
  }

#define FAKE_BASIC_SYNC_WRAPPER(ret_type, name, ...)                \
  do {                                                              \
    ret_type retval;                                                \
    log_entry_t my_entry = create_##name##_entry(my_clone_id,       \
                                                 name##_event,      \
                                                 __VA_ARGS__);      \
    if (SYNC_IS_REPLAY) {                                           \
      WRAPPER_REPLAY(name);                                         \
    } else if (SYNC_IS_RECORD) {                                    \
      WRAPPER_LOG_WRITE_ENTRY(my_entry);                            \
    }                                                               \
  } while (0)

#define EVENT_CODE(ret_type, name, ...) name##_event,
/* Event codes: */
typedef enum {
  FOREACH_RECORD_REPLAY_WRAPPER(EVENT_CODE)
  numTotalEvents
} event_code_t;
/* end event codes */

#define DECL_EVENT_SIZE(ret_type, x, ...) \
  static const int log_event_##x##_size = sizeof(log_event_##x##_t);

typedef struct {
  // For pthread_mutex_{lock,trylock,unlock}():
  pthread_mutex_t *addr;
  pthread_mutex_t mutex;
} log_event_pthread_mutex_lock_t,
  log_event_pthread_mutex_trylock_t,
  log_event_pthread_mutex_unlock_t;

typedef struct {
  // For pthread_rwlock_{rdlock,wrlock,unlock}():
  pthread_rwlock_t *addr;
  pthread_rwlock_t rwlock;
} log_event_pthread_rwlock_rdlock_t,
  log_event_pthread_rwlock_wrlock_t,
  log_event_pthread_rwlock_unlock_t;

typedef struct {
  // For pthread_cond_signal():
  // For pthread_cond_broadcast():
  pthread_cond_t *addr;
  pthread_cond_t cond;
  int signal_target;
} log_event_pthread_cond_signal_t, log_event_pthread_cond_broadcast_t;

typedef struct {
  // For pthread_cond_wait():
  pthread_mutex_t *mutex_addr;
  pthread_cond_t *cond_addr;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
} log_event_pthread_cond_wait_t;

typedef struct {
  // For pthread_cond_timedwait():
  pthread_mutex_t *mutex_addr;
  pthread_cond_t *cond_addr;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  struct timespec *abstime;
} log_event_pthread_cond_timedwait_t;

typedef struct {
  // For pthread_cond_destroy():
  pthread_cond_t *cond_addr;
  pthread_cond_t cond;
} log_event_pthread_cond_destroy_t;

typedef struct {
  // For pthread_exit():
  void *value_ptr;
} log_event_pthread_exit_t;

typedef struct {
  // For pthread_join():
  pthread_t thread;
  void *value_ptr;
} log_event_pthread_join_t;

typedef struct {
  // For pthread_kill():
  pthread_t thread;
  int sig;
} log_event_pthread_kill_t;

typedef struct {
  // For rand():
  int x; // unused, but prevents optimizing away this struct.
} log_event_rand_t;

typedef struct {
  // For rename():
  char *oldpath;
  char *newpath;
} log_event_rename_t;

typedef struct {
  // For rewind():
  FILE *stream;
} log_event_rewind_t;

typedef struct {
  // For rmdir():
  char *pathname;
} log_event_rmdir_t;

typedef struct {
  // For select():
  int nfds;
  fd_set readfds;
  fd_set writefds;
  fd_set *exceptfds; // just save address for now
  struct timeval *timeout;
} log_event_select_t;

typedef struct {
  // For ppoll():
  struct pollfd *fds;
  nfds_t nfds;
  const struct timespec *timeout_ts;
  const sigset_t *sigmask;
  off_t data_offset; // offset into read saved data file
} log_event_ppoll_t;

typedef struct {
  // For signal handlers:
  int sig;
  siginfo_t *info;
  void *data;
} log_event_signal_handler_t;

typedef struct {
  // For sigwait():
  sigset_t *set;
  int *sigwait_sig;
  int sig;
} log_event_sigwait_t;

typedef struct {
  // For read():
  int fd;
  void* buf_addr;
  size_t count;
  off_t data_offset; // offset into read saved data file
} log_event_read_t;

typedef struct {
  // For readv():
  int fd;
  const struct iovec *iov;
  int iovcnt;
  off_t data_offset; // offset into read saved data file
} log_event_readv_t;

typedef struct {
  // For readdir():
  DIR *dirp;
  struct dirent retval;
} log_event_readdir_t;

typedef struct {
  // For readdir_r():
  DIR *dirp;
  struct dirent *entry;
  struct dirent **result;
  struct dirent ret_entry;
  struct dirent *ret_result;
} log_event_readdir_r_t;

typedef struct {
  // For readlink():
  char *path;
  char *buf;
  size_t bufsiz;
  off_t data_offset;
} log_event_readlink_t;

typedef struct {
  // For realpath():
  const char *path;
  char *resolved_path;
  size_t len;
  off_t data_offset;
} log_event_realpath_t;

typedef struct {
  // For unlink():
  char *pathname;
} log_event_unlink_t;

typedef struct {
  // For user event:
  int x; // unused, but prevents optimizing away this struct.
} log_event_user_t;

typedef struct {
  // For write():
  int fd;
  void* buf_addr;
  size_t count;
} log_event_write_t;

typedef struct {
  // For writev():
  int fd;
  const struct iovec *iov;
  int iovcnt;
} log_event_writev_t;

typedef struct {
  // For accept():
  int sockfd;
  struct sockaddr *addr;
  socklen_t *addrlen;
  struct sockaddr ret_addr;
  socklen_t ret_addrlen;
} log_event_accept_t;

typedef struct {
  // For accept4():
  int sockfd;
  struct sockaddr *addr;
  socklen_t *addrlen;
  int flags;
  struct sockaddr ret_addr;
  socklen_t ret_addrlen;
} log_event_accept4_t;

typedef struct {
  // For access():
  char *pathname;
  int mode;
} log_event_access_t;

typedef struct {
  // For bind():
  int sockfd;
  struct sockaddr *addr;
  socklen_t addrlen;
} log_event_bind_t;

typedef struct {
  // For getpeername():
  int sockfd;
  struct sockaddr *addr;
  socklen_t *addrlen;
  struct sockaddr ret_addr;
  socklen_t ret_addrlen;
} log_event_getpeername_t;

typedef struct {
  // For getsockname():
  int sockfd;
  struct sockaddr *addr;
  socklen_t *addrlen;
  struct sockaddr ret_addr;
  socklen_t ret_addrlen;
} log_event_getsockname_t;

typedef struct {
  // For setsockopt():
  int sockfd;
  int level;
  int optname;
  void *optval;
  socklen_t optlen;
} log_event_setsockopt_t;

typedef struct {
  // For getsockopt():
  int sockfd;
  int level;
  int optname;
  void *optval;
  socklen_t *optlen;
  socklen_t ret_optlen;
  off_t data_offset;
} log_event_getsockopt_t;

typedef struct {
  // For ioctl():
  int d;
  int request;
  void *arg;
  struct winsize win_val;
  struct ifconf ifconf_val;
  int fionread_val;
  off_t data_offset;
} log_event_ioctl_t;

typedef struct {
  // For pthread_create():
  pthread_t *thread;
  pthread_attr_t *attr;
  void *(*start_routine)(void*);
  void *arg;
  void *stack_addr;
  size_t stack_size;
} log_event_pthread_create_t;

typedef struct {
  // For pthread_detach():
  pthread_t thread;
} log_event_pthread_detach_t;

typedef struct {
  // For __libc_memalign():
  size_t boundary;
  size_t size;
  void *return_ptr;
} log_event_libc_memalign_t;

typedef struct {
  // For fclose():
  FILE *fp;
} log_event_fclose_t;

typedef struct {
  // For fchdir():
  int fd;
} log_event_fchdir_t;

typedef struct {
  // For fcntl():
  int fd;
  int cmd;
  void *arg;
  struct flock ret_flock;
} log_event_fcntl_t;

typedef struct {
  // For fdatasync():
  int fd;
} log_event_fdatasync_t;

typedef struct {
  // For fdopen():
  int fd;
  char *mode;
  // Size is approximately 216 bytes:
  FILE fdopen_retval;
} log_event_fdopen_t;

typedef struct {
  // For fdopendir():
  int fd;
} log_event_fdopendir_t;

typedef struct {
  // For fgets():
  char *s;
  int size;
  FILE *stream;
  off_t data_offset;
} log_event_fgets_t;

typedef struct {
  // For ferror():
  FILE *stream;
} log_event_ferror_t;

typedef struct {
  // For feof():
  FILE *stream;
} log_event_feof_t;

typedef struct {
  // For fileno():
  FILE *stream;
} log_event_fileno_t;

typedef struct {
  // For fflush():
  FILE *stream;
} log_event_fflush_t;

typedef struct {
  // For setvbuf():
  FILE *stream;
  char *buf;
  int mode;
  size_t size;
} log_event_setvbuf_t;

typedef struct {
  // For fopen():
  char *name;
  char *mode;
  // Size is approximately 216 bytes:
  FILE fopen_retval;
} log_event_fopen_t;

typedef struct {
  // For fopen64():
  char *name;
  char *mode;
  // Size is approximately 216 bytes:
  FILE fopen64_retval;
} log_event_fopen64_t;

typedef struct {
  // For freopen():
  char *path;
  char *mode;
  FILE *stream;
  // Size is approximately 216 bytes:
  FILE freopen_retval;
} log_event_freopen_t;

typedef struct {
  // For fprintf():
  FILE *stream;
  char *format;
  va_list ap;
} log_event_fprintf_t;

typedef struct {
  // For fscanf():
  FILE *stream;
  char *format;
  int bytes;
  off_t data_offset;
} log_event_fscanf_t;

typedef struct {
  // For fseek():
  FILE *stream;
  long offset;
  int whence;
} log_event_fseek_t;

typedef struct {
  // For fputs():
  char *s;
  FILE *stream;
} log_event_fputs_t;

typedef struct {
  // For fputc():
  int c;
  FILE *stream;
} log_event_fputc_t;

typedef struct {
  // For getc():
  FILE *stream;
} log_event_getc_t;

typedef struct {
  // For getcwd():
  char *buf;
  size_t size;
  off_t data_offset;
} log_event_getcwd_t;

typedef struct {
  // For gettimeofday():
  struct timeval *tv;
  struct timezone *tz;
  struct timeval tv_val;
  struct timezone tz_val;
  int gettimeofday_retval;
} log_event_gettimeofday_t;

typedef struct {
  // For fgetc():
  FILE *stream;
} log_event_fgetc_t;

typedef struct {
  // For ungetc():
  int c;
  FILE *stream;
} log_event_ungetc_t;

typedef struct {
  // For getline():
  char *lineptr;
  char *new_lineptr;
  size_t n;
  size_t new_n;
  FILE *stream;
  off_t data_offset;
} log_event_getline_t;

typedef struct {
  // For getdelim():
  char *lineptr;
  char *new_lineptr;
  size_t n;
  size_t new_n;
  int delim;
  FILE *stream;
  off_t data_offset;
} log_event_getdelim_t;

typedef struct {
  // For link():
  char *oldpath;
  char *newpath;
} log_event_link_t;

typedef struct {
  // For symlink():
  char *oldpath;
  char *newpath;
} log_event_symlink_t;

typedef struct {
  // For listen():
  int sockfd;
  int backlog;
} log_event_listen_t;

typedef struct {
  // For utime():
  char *filename;
  struct utimbuf *times;
} log_event_utime_t;

typedef struct {
  // For utimes():
  char *filename;
  struct timeval *times;
} log_event_utimes_t;

typedef struct {
  // For futimes():
  int fd;
  struct timeval *times;
} log_event_futimes_t;

typedef struct {
  // For lutimes():
  char *filename;
  struct timeval *times;
} log_event_lutimes_t;

typedef struct {
  // For localtime_r():
  time_t *timep;
  struct tm *result;
  struct tm ret_result;
} log_event_localtime_r_t;

typedef struct {
  // For clock_getres():
  clockid_t clk_id;
  struct timespec *res;
  struct timespec ret_res;
} log_event_clock_getres_t;

typedef struct {
  // For clock_gettime():
  clockid_t clk_id;
  struct timespec *tp;
  struct timespec ret_tp;
} log_event_clock_gettime_t;

typedef struct {
  // For clock_settime():
  clockid_t clk_id;
  struct timespec *tp;
} log_event_clock_settime_t;

typedef struct {
  // For lseek():
  int fd;
  off_t offset;
  int whence;
} log_event_lseek_t;

typedef struct {
  // For lseek64():
  int fd;
  off64_t offset;
  int whence;
} log_event_lseek64_t;

typedef struct {
  // For llseek():
  int fd;
  loff_t offset;
  int whence;
} log_event_llseek_t;

typedef struct {
  // For lxstat():
  int vers;
  char *path;
  struct stat buf;
} log_event_lxstat_t;

typedef struct {
  // For lxstat64():
  int vers;
  char *path;
  struct stat64 buf;
} log_event_lxstat64_t;

typedef struct {
  // For malloc():
  size_t size;
} log_event_malloc_t;

typedef struct {
  // For mkdir():
  char *pathname;
  mode_t mode;
} log_event_mkdir_t;

typedef struct {
  // For mkstemp():
  char *temp;
} log_event_mkstemp_t;

typedef struct {
  // For mmap():
  void *addr;
  size_t length;
  int prot;
  int flags;
  int fd;
  off_t offset;
  off_t data_offset; // offset into read saved data file
} log_event_mmap_t;

typedef struct {
  // For mmap64():
  void *addr;
  size_t length;
  int prot;
  int flags;
  int fd;
  off64_t offset;
  off_t data_offset; // offset into read saved data file
} log_event_mmap64_t;

typedef struct {
  // For mremap():
  void *old_address;
  size_t old_size;
  size_t new_size;
  int flags;
} log_event_mremap_t;

typedef struct {
  // For munmap():
  void *addr;
  size_t length;
} log_event_munmap_t;

typedef struct {
  // For open():
  char *path;
  int flags;
  mode_t open_mode;
} log_event_open_t;

typedef struct {
  // For open64():
  char *path;
  int flags;
  mode_t open_mode;
} log_event_open64_t;

typedef struct {
  // For opendir():
  char *name;
} log_event_opendir_t;

typedef struct {
  // For openat():
  int dirfd;
  char *pathname;
  int flags;
} log_event_openat_t;

typedef struct {
  // For pread():
  int fd;
  void* buf;
  size_t count;
  off_t offset;
  off_t data_offset; // offset into read saved data file
} log_event_pread_t;

typedef struct {
  // For preadv():
  int fd;
  const struct iovec *iov;
  int iovcnt;
  off_t offset;
  off_t data_offset; // offset into read saved data file
} log_event_preadv_t;

typedef struct {
  // For putc():
  int c;
  FILE *stream;
} log_event_putc_t;

typedef struct {
  // For pwrite():
  int fd;
  void* buf;
  size_t count;
  off_t offset;
} log_event_pwrite_t;

typedef struct {
  // For pwritev():
  int fd;
  const struct iovec *iov;
  int iovcnt;
  off_t offset;
} log_event_pwritev_t;

typedef struct {
  // For calloc():
  size_t nmemb;
  size_t size;
} log_event_calloc_t;

typedef struct {
  // For chmod():
  char *path;
  mode_t mode;
} log_event_chmod_t;

typedef struct {
  // For chown():
  char *path;
  uid_t owner;
  gid_t group;
} log_event_chown_t;

typedef struct {
  // For close():
  int fd;
} log_event_close_t;

typedef struct {
  // For closedir():
  DIR *dirp;
} log_event_closedir_t;

typedef struct {
  // For connect():
  int sockfd;
  struct sockaddr *serv_addr;
  socklen_t addrlen;
} log_event_connect_t;

typedef struct {
  // For dup():
  int oldfd;
} log_event_dup_t;

typedef struct {
  // For dup2():
  int oldfd;
  int newfd;
} log_event_dup2_t;

typedef struct {
  // For dup3():
  int oldfd;
  int newfd;
  int flags;
} log_event_dup3_t;

typedef struct {
  // For exec_barrier: special case.
} log_event_exec_barrier_t;

typedef struct {
  // For realloc():
  size_t size;
  void *ptr;
} log_event_realloc_t;

typedef struct {
  // For free():
  void *ptr;
} log_event_free_t;

typedef struct {
  // For ftell():
  FILE *stream;
} log_event_ftell_t;

typedef struct {
  // For fwrite():
  void *ptr;
  size_t size;
  size_t nmemb;
  FILE *stream;
} log_event_fwrite_t;

typedef struct {
  // For fread():
  void *ptr;
  size_t size;
  size_t nmemb;
  FILE *stream;
  off_t data_offset; // offset into read saved data file
} log_event_fread_t;

typedef struct {
  // For fsync():
  int fd;
} log_event_fsync_t;

typedef struct {
  // For fxstat():
  int vers;
  int fd;
  struct stat buf;
} log_event_fxstat_t;

typedef struct {
  // For fxstat64():
  int vers;
  int fd;
  struct stat64 buf;
} log_event_fxstat64_t;

typedef struct {
  // For time():
  time_t time_retval;
  time_t *tloc;
} log_event_time_t;

typedef struct {
  // For tmpfile():
  FILE tmpfile_retval;
} log_event_tmpfile_t;

typedef struct {
  // For truncate():
  char *path;
  off_t length;
} log_event_truncate_t;

typedef struct {
  // For ftruncate():
  int fd;
  off_t length;
} log_event_ftruncate_t;

typedef struct {
  // For truncate64():
  char *path;
  off_t length;
} log_event_truncate64_t;

typedef struct {
  // For ftruncate64():
  int fd;
  off_t length;
} log_event_ftruncate64_t;

typedef struct {
  // For srand():
  unsigned int seed;
} log_event_srand_t;

typedef struct {
  // For socket():
  int domain;
  int type;
  int protocol;
} log_event_socket_t;

typedef struct {
  // For socketpair():
  int domain;
  int type;
  int protocol;
  int *sv;
  int ret_sv[2];
} log_event_socketpair_t;

typedef struct {
  // For xstat():
  int vers;
  char *path;
  struct stat buf;
} log_event_xstat_t;

typedef struct {
  // For xstat64():
  int vers;
  char *path;
  struct stat64 buf;
} log_event_xstat64_t;

typedef struct {
  // For epoll_create():
  int size;
} log_event_epoll_create_t;

typedef struct {
  // For epoll_create1():
  int flags;
} log_event_epoll_create1_t;

typedef struct {
  // For epoll_ctl():
  int epfd;
  int op;
  int fd;
  struct epoll_event *_event;
} log_event_epoll_ctl_t;

typedef struct {
  // For epoll_wait():
  int epfd;
  struct epoll_event *events;
  int maxevents;
  int timeout;
  off_t data_offset; // offset into read saved data file
} log_event_epoll_wait_t;

typedef struct {
  // For getpwnam_r():
  const char *name;
  struct passwd *pwd;
  char *buf;
  size_t buflen;
  struct passwd **result;
  struct passwd ret_pwd;
  struct passwd *ret_result;
  off_t data_offset; // offset into read saved data file
} log_event_getpwnam_r_t;

typedef struct {
  // For getpwuid_r():
  uid_t uid;
  struct passwd *pwd;
  char *buf;
  size_t buflen;
  struct passwd **result;
  struct passwd ret_pwd;
  struct passwd *ret_result;
  off_t data_offset; // offset into read saved data file
} log_event_getpwuid_r_t;

typedef struct {
  // For getgrnam_r():
  const char *name;
  struct group *grp;
  char *buf;
  size_t buflen;
  struct group **result;
  struct group ret_grp;
  struct group *ret_result;
  off_t data_offset; // offset into read saved data file
} log_event_getgrnam_r_t;

typedef struct {
  // For getgrgid_r():
  gid_t gid;
  struct group *grp;
  char *buf;
  size_t buflen;
  struct group **result;
  struct group ret_grp;
  struct group *ret_result;
  off_t data_offset; // offset into read saved data file
} log_event_getgrgid_r_t;

typedef struct {
  // For getaddrinfo():
  const char *node;
  const char *service;
  const struct addrinfo *hints;
  struct addrinfo **res;
  struct addrinfo *ret_res;
  off_t data_offset; // offset into read saved data file
  int num_res;
} log_event_getaddrinfo_t;

typedef struct {
  // For freeaddrinfo():
  struct addrinfo *res;
} log_event_freeaddrinfo_t;

typedef struct {
  // For getnameinfo():
  const struct sockaddr *sa;
  socklen_t salen;
  char *host;
  socklen_t hostlen;
  char *serv;
  socklen_t servlen;
  int flags;
  char ret_host[NI_MAXHOST];
  char ret_serv[NI_MAXSERV];
} log_event_getnameinfo_t;

typedef struct {
  // For sendto();
  int sockfd;
  const void *buf;
  size_t len;
  int flags;
  const struct sockaddr *dest_addr;
  socklen_t addrlen;
} log_event_sendto_t;

typedef struct {
    // For sendmsg();
  int sockfd;
  const struct msghdr *msg;
  int flags;
} log_event_sendmsg_t;

typedef struct {
  // For recvfrom();
  int sockfd;
  void *buf;
  size_t len;
  int flags;
  struct sockaddr *src_addr;
  socklen_t *addrlen;
  void* return_addr;
  socklen_t ret_addrlen;
  off_t data_offset;
} log_event_recvfrom_t;

typedef struct {
  // For recvmsg();
  int sockfd;
  struct msghdr *msg;
  int flags;
  struct msghdr ret_msg;
  off_t data_offset;
  off_t control_buf_offset;
} log_event_recvmsg_t;

typedef struct {
  // For waitid();
  idtype_t idtype;
  id_t id;
  siginfo_t *infop;
  int options;
  siginfo_t ret_infop;
} log_event_waitid_t;

typedef struct {
  // For wait4();
  pid_t pid;
  __WAIT_STATUS status;
  int options;
  struct rusage *rusage;

  int ret_status;
  struct rusage ret_rusage;
} log_event_wait4_t;

typedef struct {
  // For flockfile();
  FILE *filehandle;
} log_event_flockfile_t;
typedef log_event_flockfile_t log_event_ftrylockfile_t;
typedef log_event_flockfile_t log_event_funlockfile_t;

typedef struct {
} log_event_dummy_t;

typedef log_event_dummy_t log_event_sigaction_t;
typedef log_event_dummy_t log_event_signal_t;
typedef log_event_dummy_t log_event_sigset_t;
typedef log_event_dummy_t log_event_empty_t;

FOREACH_RECORD_REPLAY_WRAPPER(DECL_EVENT_SIZE)

typedef struct {
  event_code_t event;
  unsigned char isOptional;
  log_off_t log_offset;
  clone_id_t clone_id;
  int my_errno;
  void* retval;
} log_entry_header_t;

#define DECL_LOG_EVENT_TYPE(ret_type, x, ...) \
  log_event_##x##_t       log_event_##x;

typedef struct {
  // We aren't going to map more than 256 system calls/functions.
  // We can expand it to 4 bytes if needed. However a single byte makes
  // things easier.
  // Shared among all events ("common area"):
  /* IMPORTANT: Adding new fields to the common area requires that you also
   * update the log_event_common_size definition. */
  log_entry_header_t header;

  union {
    FOREACH_RECORD_REPLAY_WRAPPER(DECL_LOG_EVENT_TYPE)
  } edata;
} log_entry_t;

#define log_event_common_size                                          \
  (sizeof(event_code_t)  +  /* event */                                \
   sizeof(unsigned char) +  /* isOptional */                           \
   sizeof(log_off_t)     +  /* log_offset */                           \
   sizeof(clone_id_t)    +  /* clone_id */                             \
   sizeof(int)           +  /* my_errno */                             \
   sizeof(void *))          /* retval */


#define GET_FIELD(entry, event, field) \
  (entry.edata.log_event_##event.field)
#define GET_FIELD_PTR(entry, event, field) \
  (entry->edata.log_event_##event.field)
#define ARE_FIELDS_EQUAL_PTR(e1, e2, event, field) \
  (GET_FIELD_PTR(e1, event, field) == GET_FIELD_PTR(e2, event, field))

#define SET_FIELD2(entry,event,field,field2) \
  GET_FIELD(entry, event, field) = field2

#define SET_FIELD(entry, event, field) \
  SET_FIELD2(entry, event, field, field)

#define SET_FIELD_FROM(entry, event, field, source) \
  GET_FIELD(entry, event, field) = GET_FIELD(source, event, field)

#define GET_COMMON(entry, field) (entry.header.field)
#define GET_COMMON_PTR(entry, field) (entry->header.field)

#define SET_COMMON_PTR(entry, field) GET_COMMON_PTR(entry, field) = field
#define SET_COMMON_PTR2(entry, field, field2) \
  GET_COMMON_PTR(entry, field) = field2

#define SET_COMMON2(entry, field, field2) GET_COMMON(entry, field) = field2
#define SET_COMMON(entry, field) SET_COMMON2(entry, field, field)

#define IS_EQUAL_COMMON(e1, e2, field) \
  (GET_COMMON(e1, field) == GET_COMMON(e2, field))
#define IS_EQUAL_FIELD(e1, e2, event, field) \
  (GET_FIELD(e1, event, field) == GET_FIELD(e2, event, field))
#define IS_EQUAL_FIELD_PTR(e1, e2, event, field) \
  (GET_FIELD_PTR(e1, event, field) == GET_FIELD_PTR(e2, event, field))

/* Typedefs */
// Type for predicate to check for a turn in the log.
typedef int (*turn_pred_t) (log_entry_t*, log_entry_t*);
typedef struct {
  int retval;
  int my_errno;
  void *value_ptr;
} pthread_join_retval_t;

/* Static constants: */
// Clone id to indicate anyone may do this event (used for exec):
static const int         CLONE_ID_ANYONE = -2;
static const log_entry_t EMPTY_LOG_ENTRY = {{empty_event, 0, 0, 0, 0, 0}};
// Number to start clone_ids at:
static const int         GLOBAL_CLONE_COUNTER_INIT = 1;
static const int         RECORD_LOG_PATH_MAX = 256;

/* Library private: */
typedef struct new_sem new_sem_t;
LIB_PRIVATE extern dmtcp::map<clone_id_t, pthread_t> *clone_id_to_tid_table;
LIB_PRIVATE extern dmtcp::map<pthread_t, clone_id_t> *tid_to_clone_id_table;
LIB_PRIVATE extern dmtcp::map<clone_id_t, sem_t>     *clone_id_to_sem_table;
LIB_PRIVATE extern dmtcp::map<pthread_t, pthread_join_retval_t> pthread_join_retvals;
LIB_PRIVATE extern char RECORD_LOG_PATH[RECORD_LOG_PATH_MAX];
LIB_PRIVATE extern char RECORD_READ_DATA_LOG_PATH[RECORD_LOG_PATH_MAX];
LIB_PRIVATE extern int             read_data_fd;
LIB_PRIVATE extern int             sync_logging_branch;
LIB_PRIVATE extern int             log_all_allocs;
LIB_PRIVATE extern pthread_mutex_t read_data_mutex;

LIB_PRIVATE extern dmtcp::SynchronizationLog global_log;

// TODO: rename this, since a log entry is not a char. maybe log_event_TYPE_SIZE?
#define LOG_ENTRY_SIZE sizeof(char)

/* Thread locals: */
LIB_PRIVATE extern __thread clone_id_t my_clone_id;

/* Volatiles: */
LIB_PRIVATE extern volatile clone_id_t    global_clone_counter;
LIB_PRIVATE extern volatile off_t         read_log_pos;

/* Functions */
LIB_PRIVATE void   addNextLogEntry(log_entry_t&);
LIB_PRIVATE void   set_sync_mode(int mode);
LIB_PRIVATE int    get_sync_mode();
LIB_PRIVATE clone_id_t get_next_clone_id();
LIB_PRIVATE void   copyFdSet(fd_set *src, fd_set *dest);
LIB_PRIVATE void   getNextLogEntry();
LIB_PRIVATE void   initializeLogNames();
LIB_PRIVATE void   initLogsForRecordReplay();
LIB_PRIVATE void   logReadData(void *buf, int count);
LIB_PRIVATE void   reapThisThread();
LIB_PRIVATE void   recordDataStackLocations();
LIB_PRIVATE int    shouldSynchronize(void *return_addr);
LIB_PRIVATE void   initSyncAddresses();
LIB_PRIVATE void   userSynchronizedEvent();
LIB_PRIVATE void   userSynchronizedEventBegin();
LIB_PRIVATE void   userSynchronizedEventEnd();
LIB_PRIVATE ssize_t writeAll(int fd, const void *buf, size_t count);
LIB_PRIVATE bool validAddress(void *addr);
LIB_PRIVATE void initialize_thread();
LIB_PRIVATE void close_read_log();
LIB_PRIVATE void waitForTurn(log_entry_t *my_entry, turn_pred_t pred);
LIB_PRIVATE void waitForExecBarrier();

/* These 'create_XXX_entry' functions are used library-wide by their
   respective wrapper functions. Their usages are hidden by the
   WRAPPER_HEADER macro. Thus, unless we think of a better design, the
   visibility of these functions is LIB_PRIVATE.

   XXX: These functions should only be needed in their respective
   wrapper files. Couldn't we define these functions only in those
   files, instead of making them globally (within the library) visible
   here? */
#define CREATE_ENTRY_FUNC(ret_type, name, ...) \
  LIB_PRIVATE TURN_CHECK_P(name##_turn_check); \
  LIB_PRIVATE log_entry_t create_##name##_entry(clone_id_t clone_id, \
                                                event_code_t event, ##__VA_ARGS__);


FOREACH_RECORD_REPLAY_WRAPPER(CREATE_ENTRY_FUNC)

#endif // SYNCHRONIZATION_LOGGING_H
