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
#include "autogen/wrapper_util.h"

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

#define MAX_LOG_LENGTH ((size_t)250 * 1024 * 1024)
#define INVALID_LOG_OFFSET (~0U)
#define SYNC_NOOP   0
#define SYNC_RECORD 1
#define SYNC_REPLAY 2
#define SYNC_IS_REPLAY    (sync_logging_branch == SYNC_REPLAY)
#define SYNC_IS_RECORD    (sync_logging_branch == SYNC_RECORD)
#define SYNC_IS_NOOP      (sync_logging_branch == SYNC_NOOP)
#define GET_RETURN_ADDRESS() __builtin_return_address(0)

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

#define NO_LOG_ENTRY_TO_BUFFER

#ifdef NO_LOG_ENTRY_TO_BUFFER
# define log_event_common_size (sizeof(log_entry_header_t))
#else
# define log_event_common_size                                         \
  (sizeof(event_code_t)  +  /* event */                                \
   sizeof(unsigned char) +  /* isOptional */                           \
   sizeof(log_off_t)     +  /* log_offset */                           \
   sizeof(clone_id_t)    +  /* clone_id */                             \
   sizeof(int)           +  /* my_errno */                             \
   sizeof(void *))          /* retval */
#endif



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

/* Thread locals: */
LIB_PRIVATE extern __thread clone_id_t my_clone_id;

/* Volatiles: */
LIB_PRIVATE extern volatile off_t         read_log_pos;

/* Functions */
LIB_PRIVATE void   addNextLogEntry(log_entry_t&);
LIB_PRIVATE void   set_sync_mode(int mode);
LIB_PRIVATE int    get_sync_mode();
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



#endif // SYNCHRONIZATION_LOGGING_H
