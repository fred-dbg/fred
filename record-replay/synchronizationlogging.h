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

#include "constants.h"
#include "dmtcpalloc.h"
#include "util.h"
#include "dmtcpmodule.h"
#include "jfilesystem.h"

#undef WRAPPER_EXECUTION_ENABLE_CKPT
#undef WRAPPER_EXECUTION_DISABLE_CKPT
#define WRAPPER_EXECUTION_ENABLE_CKPT dmtcp_module_enable_ckpt
#define WRAPPER_EXECUTION_DISABLE_CKPT dmtcp_module_disable_ckpt
typedef long int clone_id_t;
typedef unsigned long int log_off_t;

namespace dmtcp { class SynchronizationLog; }

static pthread_mutex_t read_data_mutex = PTHREAD_MUTEX_INITIALIZER;

static inline bool isProcessGDB() {
  static bool isGDB = jalib::Filesystem::GetProgramName() == "gdb";
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
#define SET_IN_MMAP_WRAPPER()   (in_mmap_wrapper = 1)
#define UNSET_IN_MMAP_WRAPPER() (in_mmap_wrapper = 0)
#define IN_MMAP_WRAPPER         (in_mmap_wrapper == 1)

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
    if (!shouldSynchronize(return_addr) ||                              \
        jalib::Filesystem::GetProgramName() == "gdb") {                 \
      real_func(__VA_ARGS__);                                           \
      return;                                                           \
    }                                                                   \
  } while(0)

#define WRAPPER_HEADER_RAW(ret_type, name, real_func, ...)              \
  void *return_addr = GET_RETURN_ADDRESS();                             \
  do {                                                                  \
    if (!shouldSynchronize(return_addr) ||                              \
        jalib::Filesystem::GetProgramName() == "gdb") {                 \
      return real_func(__VA_ARGS__);                                    \
    }                                                                   \
  } while(0)

#define WRAPPER_HEADER_NO_ARGS(ret_type, name, real_func)                  \
  void *return_addr = GET_RETURN_ADDRESS();                             \
  if (!shouldSynchronize(return_addr) ||                                \
      jalib::Filesystem::GetProgramName() == "gdb") {                   \
    return real_func();                                             \
  }                                                                     \
  ret_type retval;                                                      \
  log_entry_t my_entry = create_##name##_entry(my_clone_id,             \
      name##_event);

#define WRAPPER_HEADER_NO_RETURN(name, real_func, ...)                  \
  void *return_addr = GET_RETURN_ADDRESS();                             \
  if (!shouldSynchronize(return_addr) ||                                \
      jalib::Filesystem::GetProgramName() == "gdb") {                   \
    real_func(__VA_ARGS__);                                             \
  }                                                                     \
  log_entry_t my_entry = create_##name##_entry(my_clone_id,             \
      name##_event, __VA_ARGS__);

#define WRAPPER_HEADER(ret_type, name, real_func, ...)                  \
  WRAPPER_HEADER_RAW(ret_type, name, real_func, __VA_ARGS__);           \
  ret_type retval;                                                      \
  log_entry_t my_entry = create_##name##_entry(my_clone_id,             \
      name##_event, __VA_ARGS__);

#define WRAPPER_HEADER_CKPT_DISABLED(ret_type, name, real_func, ...)    \
  void *return_addr = GET_RETURN_ADDRESS();                             \
  ret_type retval;                                                      \
  if (!shouldSynchronize(return_addr) ||                                \
      jalib::Filesystem::GetProgramName() == "gdb") {                   \
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

#define WRAPPER_REPLAY_READ_FROM_READ_LOG(name, ptr, len)           \
  do {                                                              \
    JASSERT ( read_data_fd != -1 );                                 \
    lseek(read_data_fd,                                             \
          GET_FIELD(my_entry, name, data_offset), SEEK_SET);        \
    dmtcp::Util::readAll(read_data_fd, ptr, len);                   \
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
    SET_COMMON2(my_entry, isOptional, isOptionalEvent);             \
    addNextLogEntry(my_entry);                                      \
    errno = GET_COMMON(my_entry, my_errno);                         \
  } while (0)

#define WRAPPER_LOG_WRITE_ENTRY(my_entry)                           \
  do {                                                              \
    SET_COMMON2(my_entry, retval, (void*)retval);                   \
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

#define FOREACH_NAME(MACRO, ...)                                               \
  do {                                                                         \
    MACRO(accept, __VA_ARGS__);                                                \
    MACRO(accept4, __VA_ARGS__);                                               \
    MACRO(access, __VA_ARGS__);                                                \
    MACRO(bind, __VA_ARGS__);                                                  \
    MACRO(calloc, __VA_ARGS__);                                                \
    MACRO(chmod, __VA_ARGS__);                                                 \
    MACRO(chown, __VA_ARGS__);                                                 \
    MACRO(close, __VA_ARGS__);                                                 \
    MACRO(closedir, __VA_ARGS__);                                              \
    MACRO(connect, __VA_ARGS__);                                               \
    MACRO(dup, __VA_ARGS__);                                                   \
    MACRO(dup2, __VA_ARGS__);                                                  \
    MACRO(dup3, __VA_ARGS__);                                                  \
    MACRO(exec_barrier, __VA_ARGS__);                                          \
    MACRO(fclose, __VA_ARGS__);                                                \
    MACRO(fchdir, __VA_ARGS__);                                                \
    MACRO(fcntl, __VA_ARGS__);                                                 \
    MACRO(fdatasync, __VA_ARGS__);                                             \
    MACRO(fdopen, __VA_ARGS__);                                                \
    MACRO(fdopendir, __VA_ARGS__);                                             \
    MACRO(fgets, __VA_ARGS__);                                                 \
    MACRO(ferror, __VA_ARGS__);                                                \
    MACRO(feof, __VA_ARGS__);                                                  \
    MACRO(fileno, __VA_ARGS__);                                                \
    MACRO(fflush, __VA_ARGS__);                                                \
    MACRO(setvbuf, __VA_ARGS__);                                               \
    MACRO(setvbuf, __VA_ARGS__);                                               \
    MACRO(fopen, __VA_ARGS__);                                                 \
    MACRO(fopen64, __VA_ARGS__);                                               \
    MACRO(freopen, __VA_ARGS__);                                               \
    MACRO(fprintf, __VA_ARGS__);                                               \
    MACRO(fscanf, __VA_ARGS__);                                                \
    MACRO(fseek, __VA_ARGS__);                                                 \
    MACRO(fputs, __VA_ARGS__);                                                 \
    MACRO(fputc, __VA_ARGS__);                                                 \
    MACRO(free, __VA_ARGS__);                                                  \
    MACRO(fsync, __VA_ARGS__);                                                 \
    MACRO(ftell, __VA_ARGS__);                                                 \
    MACRO(fwrite, __VA_ARGS__);                                                \
    MACRO(fread, __VA_ARGS__);                                                 \
    MACRO(fxstat, __VA_ARGS__);                                                \
    MACRO(fxstat64, __VA_ARGS__);                                              \
    MACRO(getc, __VA_ARGS__);                                                  \
    MACRO(getcwd, __VA_ARGS__);                                                \
    MACRO(getsockopt, __VA_ARGS__);                                            \
    MACRO(gettimeofday, __VA_ARGS__);                                          \
    MACRO(fgetc, __VA_ARGS__);                                                 \
    MACRO(ungetc, __VA_ARGS__);                                                \
    MACRO(getline, __VA_ARGS__);                                               \
    MACRO(getpeername, __VA_ARGS__);                                           \
    MACRO(getsockname, __VA_ARGS__);                                           \
    MACRO(ioctl, __VA_ARGS__);                                                 \
    MACRO(libc_memalign, __VA_ARGS__);                                         \
    MACRO(lseek, __VA_ARGS__);                                                 \
    MACRO(lseek64, __VA_ARGS__);                                               \
    MACRO(llseek, __VA_ARGS__);                                                \
    MACRO(link, __VA_ARGS__);                                                  \
    MACRO(symlink, __VA_ARGS__);                                               \
    MACRO(listen, __VA_ARGS__);                                                \
    MACRO(localtime, __VA_ARGS__);                                             \
    MACRO(lxstat, __VA_ARGS__);                                                \
    MACRO(lxstat64, __VA_ARGS__);                                              \
    MACRO(malloc, __VA_ARGS__);                                                \
    MACRO(mkdir, __VA_ARGS__);                                                 \
    MACRO(mkstemp, __VA_ARGS__);                                               \
    MACRO(mmap, __VA_ARGS__);                                                  \
    MACRO(mmap64, __VA_ARGS__);                                                \
    MACRO(mremap, __VA_ARGS__);                                                \
    MACRO(munmap, __VA_ARGS__);                                                \
    MACRO(open, __VA_ARGS__);                                                  \
    MACRO(open64, __VA_ARGS__);                                                \
    MACRO(openat, __VA_ARGS__);                                                \
    MACRO(opendir, __VA_ARGS__);                                               \
    MACRO(pread, __VA_ARGS__);                                                 \
    MACRO(preadv, __VA_ARGS__);                                                \
    MACRO(putc, __VA_ARGS__);                                                  \
    MACRO(pwrite, __VA_ARGS__);                                                \
    MACRO(pwritev, __VA_ARGS__);                                               \
    MACRO(pthread_detach, __VA_ARGS__);                                        \
    MACRO(pthread_create, __VA_ARGS__);                                        \
    MACRO(pthread_cond_broadcast, __VA_ARGS__);                                \
    MACRO(pthread_cond_signal, __VA_ARGS__);                                   \
    MACRO(pthread_mutex_lock, __VA_ARGS__);                                    \
    MACRO(pthread_mutex_trylock, __VA_ARGS__);                                 \
    MACRO(pthread_mutex_unlock, __VA_ARGS__);                                  \
    MACRO(pthread_cond_wait, __VA_ARGS__);                                     \
    MACRO(pthread_cond_timedwait, __VA_ARGS__);                                \
    MACRO(pthread_exit, __VA_ARGS__);                                          \
    MACRO(pthread_join, __VA_ARGS__);                                          \
    MACRO(pthread_kill, __VA_ARGS__);                                          \
    MACRO(pthread_rwlock_unlock, __VA_ARGS__);                                 \
    MACRO(pthread_rwlock_rdlock, __VA_ARGS__);                                 \
    MACRO(pthread_rwlock_wrlock, __VA_ARGS__);                                 \
    MACRO(rand, __VA_ARGS__);                                                  \
    MACRO(read, __VA_ARGS__);                                                  \
    MACRO(readv, __VA_ARGS__);                                                 \
    MACRO(readdir, __VA_ARGS__);                                               \
    MACRO(readdir_r, __VA_ARGS__);                                             \
    MACRO(readlink, __VA_ARGS__);                                              \
    MACRO(realloc, __VA_ARGS__);                                               \
    MACRO(rename, __VA_ARGS__);                                                \
    MACRO(rewind, __VA_ARGS__);                                                \
    MACRO(rmdir, __VA_ARGS__);                                                 \
    MACRO(select, __VA_ARGS__);                                                \
    MACRO(ppoll, __VA_ARGS__);                                                \
    MACRO(signal_handler, __VA_ARGS__);                                        \
    MACRO(sigwait, __VA_ARGS__);                                               \
    MACRO(setsockopt, __VA_ARGS__);                                            \
    MACRO(srand, __VA_ARGS__);                                                 \
    MACRO(socket, __VA_ARGS__);                                                \
    MACRO(socketpair, __VA_ARGS__);                                            \
    MACRO(time, __VA_ARGS__);                                                  \
    MACRO(tmpfile, __VA_ARGS__);                                               \
    MACRO(truncate, __VA_ARGS__);                                              \
    MACRO(unlink, __VA_ARGS__);                                                \
    MACRO(write, __VA_ARGS__);                                                 \
    MACRO(writev, __VA_ARGS__);                                                \
    MACRO(xstat, __VA_ARGS__);                                                 \
    MACRO(xstat64, __VA_ARGS__);                                               \
    MACRO(user, __VA_ARGS__);                                                  \
    MACRO(epoll_create, __VA_ARGS__);                                          \
    MACRO(epoll_create1, __VA_ARGS__);                                         \
    MACRO(epoll_ctl, __VA_ARGS__);                                             \
    MACRO(epoll_wait, __VA_ARGS__);                                            \
    MACRO(getpwnam_r, __VA_ARGS__);                                            \
    MACRO(getpwuid_r, __VA_ARGS__);                                            \
    MACRO(getgrnam_r, __VA_ARGS__);                                            \
    MACRO(getgrgid_r, __VA_ARGS__);                                            \
    MACRO(getaddrinfo, __VA_ARGS__);                                           \
    MACRO(freeaddrinfo, __VA_ARGS__);                                          \
    MACRO(getnameinfo, __VA_ARGS__);                                           \
                                                                               \
    MACRO(sendto, __VA_ARGS__);                                                \
    MACRO(sendmsg, __VA_ARGS__);                                               \
    MACRO(recvfrom, __VA_ARGS__);                                              \
    MACRO(recvmsg, __VA_ARGS__);                                               \
  } while(0)

/* Event codes: */
typedef enum {
  unknown_event = -1,
  empty_event = 0,
  accept_event = 1,
  accept4_event,
  access_event,
  bind_event,
  calloc_event,
  chmod_event,
  chown_event,
  close_event,
  closedir_event,
  connect_event,
  dup_event,
  dup2_event,
  dup3_event,
  exec_barrier_event,
  fclose_event,
  fchdir_event,
  fcntl_event,
  fdatasync_event,
  fdopen_event,
  fdopendir_event,
  fgets_event,
  ferror_event,
  feof_event,
  fileno_event,
  fflush_event,
  setvbuf_event,
  fopen_event,
  fopen64_event,
  freopen_event,
  fprintf_event,
  fscanf_event,
  fseek_event,
  fputs_event,
  fputc_event,
  free_event,
  fsync_event,
  ftell_event,
  fwrite_event,
  fread_event,
  fxstat_event,
  fxstat64_event,
  getc_event,
  getcwd_event,
  gettimeofday_event,
  ioctl_event,
  fgetc_event,
  ungetc_event,
  getline_event,
  getpeername_event,
  getsockname_event,
  getsockopt_event,
  libc_memalign_event,
  link_event,
  symlink_event,
  listen_event,
  localtime_event,
  lseek_event,
  lseek64_event,
  llseek_event,
  lxstat_event,
  lxstat64_event,
  malloc_event,
  mkdir_event,
  mkstemp_event,
  mmap_event,
  mmap64_event,
  mremap_event,
  munmap_event,
  open_event,
  open64_event,
  openat_event,
  opendir_event,
  pread_event,
  preadv_event,
  putc_event,
  pwrite_event,
  pwritev_event,
  pthread_detach_event,
  pthread_create_event,
  pthread_cond_broadcast_event,
  pthread_mutex_lock_event,
  pthread_mutex_trylock_event,
  pthread_cond_signal_event,
  pthread_mutex_unlock_event,
  pthread_cond_wait_event,
  pthread_cond_timedwait_event,
  pthread_exit_event,
  pthread_join_event,
  pthread_kill_event, // no return event -- asynchronous
  pthread_rwlock_unlock_event,
  pthread_rwlock_rdlock_event,
  pthread_rwlock_wrlock_event,
  rand_event,
  read_event,
  readv_event,
  readdir_event,
  readdir_r_event,
  readlink_event,
  realloc_event,
  rename_event,
  rewind_event,
  rmdir_event,
  select_event,
  ppoll_event,
  signal_handler_event,
  sigwait_event,
  setsockopt_event,
  socket_event,
  socketpair_event,
  srand_event,
  time_event,
  tmpfile_event,
  truncate_event,
  unlink_event,
  user_event,
  write_event,
  writev_event,
  xstat_event,
  xstat64_event,
  epoll_create_event,
  epoll_create1_event,
  epoll_ctl_event,
  epoll_wait_event,
  getpwnam_r_event,
  getpwuid_r_event,
  getgrnam_r_event,
  getgrgid_r_event,
  getaddrinfo_event,
  freeaddrinfo_event,
  getnameinfo_event,
  sendto_event,
  sendmsg_event,
  recvfrom_event,
  recvmsg_event
} event_code_t;
/* end event codes */

typedef struct {
  // For pthread_mutex_{lock,trylock,unlock}():
  pthread_mutex_t *addr;
  pthread_mutex_t mutex;
} log_event_pthread_mutex_lock_t,
  log_event_pthread_mutex_trylock_t,
  log_event_pthread_mutex_unlock_t;

static const int
log_event_pthread_mutex_lock_size = sizeof(log_event_pthread_mutex_lock_t);

static const int
log_event_pthread_mutex_trylock_size = sizeof(log_event_pthread_mutex_trylock_t);

static const int
log_event_pthread_mutex_unlock_size = sizeof(log_event_pthread_mutex_unlock_t);

typedef struct {
  // For pthread_rwlock_{rdlock,wrlock,unlock}():
  pthread_rwlock_t *addr;
  pthread_rwlock_t rwlock;
} log_event_pthread_rwlock_rdlock_t,
  log_event_pthread_rwlock_wrlock_t,
  log_event_pthread_rwlock_unlock_t;

static const int
log_event_pthread_rwlock_unlock_size = sizeof(log_event_pthread_rwlock_unlock_t);

static const int
log_event_pthread_rwlock_rdlock_size = sizeof(log_event_pthread_rwlock_rdlock_t);

static const int
log_event_pthread_rwlock_wrlock_size = sizeof(log_event_pthread_rwlock_wrlock_t);

typedef struct {
  // For pthread_cond_signal():
  // For pthread_cond_broadcast():
  pthread_cond_t *addr;
  pthread_cond_t cond;
  int signal_target;
} log_event_pthread_cond_signal_t, log_event_pthread_cond_broadcast_t;

static const int
log_event_pthread_cond_signal_size = sizeof(log_event_pthread_cond_signal_t);

static const int
log_event_pthread_cond_broadcast_size = sizeof(log_event_pthread_cond_broadcast_t);

typedef struct {
  // For pthread_cond_wait():
  pthread_mutex_t *mutex_addr;
  pthread_cond_t *cond_addr;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
} log_event_pthread_cond_wait_t;

static const int log_event_pthread_cond_wait_size = sizeof(log_event_pthread_cond_wait_t);

typedef struct {
  // For pthread_cond_timedwait():
  pthread_mutex_t *mutex_addr;
  pthread_cond_t *cond_addr;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  struct timespec *abstime;
} log_event_pthread_cond_timedwait_t;

static const int log_event_pthread_cond_timedwait_size = sizeof(log_event_pthread_cond_timedwait_t);

typedef struct {
  // For pthread_exit():
  void *value_ptr;
} log_event_pthread_exit_t;

static const int log_event_pthread_exit_size = sizeof(log_event_pthread_exit_t);

typedef struct {
  // For pthread_join():
  pthread_t thread;
  void *value_ptr;
} log_event_pthread_join_t;

static const int log_event_pthread_join_size = sizeof(log_event_pthread_join_t);

typedef struct {
  // For pthread_kill():
  pthread_t thread;
  int sig;
} log_event_pthread_kill_t;

static const int log_event_pthread_kill_size = sizeof(log_event_pthread_kill_t);

typedef struct {
  // For rand():
  int x; // unused, but prevents optimizing away this struct.
} log_event_rand_t;

static const int log_event_rand_size = sizeof(log_event_rand_t);

typedef struct {
  // For rename():
  char *oldpath;
  char *newpath;
} log_event_rename_t;

static const int log_event_rename_size = sizeof(log_event_rename_t);

typedef struct {
  // For rewind():
  FILE *stream;
} log_event_rewind_t;

static const int log_event_rewind_size = sizeof(log_event_rewind_t);

typedef struct {
  // For rmdir():
  char *pathname;
} log_event_rmdir_t;

static const int log_event_rmdir_size = sizeof(log_event_rmdir_t);

typedef struct {
  // For select():
  int nfds;
  fd_set readfds;
  fd_set writefds;
  fd_set *exceptfds; // just save address for now
  struct timeval *timeout;
} log_event_select_t;

static const int log_event_select_size = sizeof(log_event_select_t);

typedef struct {
  // For ppoll():
  struct pollfd *fds;
  nfds_t nfds;
  const struct timespec *timeout_ts;
  const sigset_t *sigmask;
  off_t data_offset; // offset into read saved data file
} log_event_ppoll_t;

static const int log_event_ppoll_size = sizeof(log_event_ppoll_t);

typedef struct {
  // For signal handlers:
  int sig;
  siginfo_t *info;
  void *data;
} log_event_signal_handler_t;

static const int log_event_signal_handler_size = sizeof(log_event_signal_handler_t);

typedef struct {
  // For sigwait():
  sigset_t *set;
  int *sigwait_sig;
  int sig;
} log_event_sigwait_t;

static const int log_event_sigwait_size = sizeof(log_event_sigwait_t);

typedef struct {
  // For read():
  int fd;
  void* buf_addr;
  size_t count;
  off_t data_offset; // offset into read saved data file
} log_event_read_t;

static const int log_event_read_size = sizeof(log_event_read_t);

typedef struct {
  // For readv():
  int fd;
  const struct iovec *iov;
  int iovcnt;
  off_t data_offset; // offset into read saved data file
} log_event_readv_t;

static const int log_event_readv_size = sizeof(log_event_readv_t);

typedef struct {
  // For readdir():
  DIR *dirp;
  struct dirent retval;
} log_event_readdir_t;

static const int log_event_readdir_size = sizeof(log_event_readdir_t);

typedef struct {
  // For readdir_r():
  DIR *dirp;
  struct dirent *entry;
  struct dirent **result;
  struct dirent ret_entry;
  struct dirent *ret_result;
} log_event_readdir_r_t;

static const int log_event_readdir_r_size = sizeof(log_event_readdir_r_t);

typedef struct {
  // For readlink():
  char *path;
  char *buf;
  size_t bufsiz;
  off_t data_offset;
} log_event_readlink_t;

static const int log_event_readlink_size = sizeof(log_event_readlink_t);

typedef struct {
  // For unlink():
  char *pathname;
} log_event_unlink_t;

static const int log_event_unlink_size = sizeof(log_event_unlink_t);

typedef struct {
  // For user event:
  int x; // unused, but prevents optimizing away this struct.
} log_event_user_t;

static const int log_event_user_size = sizeof(log_event_user_t);

typedef struct {
  // For write():
  int fd;
  void* buf_addr;
  size_t count;
} log_event_write_t;

static const int log_event_write_size = sizeof(log_event_write_t);

typedef struct {
  // For writev():
  int fd;
  const struct iovec *iov;
  int iovcnt;
} log_event_writev_t;

static const int log_event_writev_size = sizeof(log_event_writev_t);

typedef struct {
  // For accept():
  int sockfd;
  struct sockaddr *addr;
  socklen_t *addrlen;
  struct sockaddr ret_addr;
  socklen_t ret_addrlen;
} log_event_accept_t;

static const int log_event_accept_size = sizeof(log_event_accept_t);

typedef struct {
  // For accept4():
  int sockfd;
  struct sockaddr *addr;
  socklen_t *addrlen;
  int flags;
  struct sockaddr ret_addr;
  socklen_t ret_addrlen;
} log_event_accept4_t;

static const int log_event_accept4_size = sizeof(log_event_accept4_t);

typedef struct {
  // For access():
  char *pathname;
  int mode;
} log_event_access_t;

static const int log_event_access_size = sizeof(log_event_access_t);

typedef struct {
  // For bind():
  int sockfd;
  struct sockaddr *addr;
  socklen_t addrlen;
} log_event_bind_t;

static const int log_event_bind_size = sizeof(log_event_bind_t);

typedef struct {
  // For getpeername():
  int sockfd;
  struct sockaddr *addr;
  socklen_t *addrlen;
  struct sockaddr ret_addr;
  socklen_t ret_addrlen;
} log_event_getpeername_t;

static const int log_event_getpeername_size = sizeof(log_event_getpeername_t);

typedef struct {
  // For getsockname():
  int sockfd;
  struct sockaddr *addr;
  socklen_t *addrlen;
  struct sockaddr ret_addr;
  socklen_t ret_addrlen;
} log_event_getsockname_t;

static const int log_event_getsockname_size = sizeof(log_event_getsockname_t);

typedef struct {
  // For setsockopt():
  int sockfd;
  int level;
  int optname;
  void *optval;
  socklen_t optlen;
} log_event_setsockopt_t;

static const int log_event_setsockopt_size = sizeof(log_event_setsockopt_t);

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

static const int log_event_getsockopt_size = sizeof(log_event_getsockopt_t);

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

static const int log_event_ioctl_size = sizeof(log_event_ioctl_t);

typedef struct {
  // For pthread_create():
  pthread_t *thread;
  pthread_attr_t *attr;
  void *(*start_routine)(void*);
  void *arg;
  void *stack_addr;
  size_t stack_size;
} log_event_pthread_create_t;

static const int log_event_pthread_create_size = sizeof(log_event_pthread_create_t);

typedef struct {
  // For pthread_detach():
  pthread_t thread;
} log_event_pthread_detach_t;

static const int log_event_pthread_detach_size = sizeof(log_event_pthread_detach_t);

typedef struct {
  // For __libc_memalign():
  size_t boundary;
  size_t size;
  void *return_ptr;
} log_event_libc_memalign_t;

static const int log_event_libc_memalign_size = sizeof(log_event_libc_memalign_t);

typedef struct {
  // For fclose():
  FILE *fp;
} log_event_fclose_t;

static const int log_event_fclose_size = sizeof(log_event_fclose_t);

typedef struct {
  // For fchdir():
  int fd;
} log_event_fchdir_t;

static const int log_event_fchdir_size = sizeof(log_event_fchdir_t);

typedef struct {
  // For fcntl():
  int fd;
  int cmd;
  long arg_3_l;
  struct flock *arg_3_f;
} log_event_fcntl_t;

static const int log_event_fcntl_size = sizeof(log_event_fcntl_t);

typedef struct {
  // For fdatasync():
  int fd;
} log_event_fdatasync_t;

static const int log_event_fdatasync_size = sizeof(log_event_fdatasync_t);

typedef struct {
  // For fdopen():
  int fd;
  char *mode;
  // Size is approximately 216 bytes:
  FILE fdopen_retval;
} log_event_fdopen_t;

static const int log_event_fdopen_size = sizeof(log_event_fdopen_t);

typedef struct {
  // For fdopendir():
  int fd;
} log_event_fdopendir_t;

static const int log_event_fdopendir_size = sizeof(log_event_fdopendir_t);

typedef struct {
  // For fgets():
  char *s;
  int size;
  FILE *stream;
  off_t data_offset;
} log_event_fgets_t;

static const int log_event_fgets_size = sizeof(log_event_fgets_t);

typedef struct {
  // For ferror():
  FILE *stream;
} log_event_ferror_t;

static const int log_event_ferror_size = sizeof(log_event_ferror_t);

typedef struct {
  // For feof():
  FILE *stream;
} log_event_feof_t;

static const int log_event_feof_size = sizeof(log_event_feof_t);

typedef struct {
  // For fileno():
  FILE *stream;
} log_event_fileno_t;

static const int log_event_fileno_size = sizeof(log_event_fileno_t);

typedef struct {
  // For fflush():
  FILE *stream;
} log_event_fflush_t;

static const int log_event_fflush_size = sizeof(log_event_fflush_t);

typedef struct {
  // For setvbuf():
  FILE *stream;
  char *buf;
  int mode;
  size_t size;
} log_event_setvbuf_t;

static const int log_event_setvbuf_size = sizeof(log_event_setvbuf_t);

typedef struct {
  // For fopen():
  char *name;
  char *mode;
  // Size is approximately 216 bytes:
  FILE fopen_retval;
} log_event_fopen_t;

static const int log_event_fopen_size = sizeof(log_event_fopen_t);

typedef struct {
  // For fopen64():
  char *name;
  char *mode;
  // Size is approximately 216 bytes:
  FILE fopen64_retval;
} log_event_fopen64_t;

static const int log_event_fopen64_size = sizeof(log_event_fopen64_t);

typedef struct {
  // For freopen():
  char *path;
  char *mode;
  FILE *stream;
  // Size is approximately 216 bytes:
  FILE freopen_retval;
} log_event_freopen_t;

static const int log_event_freopen_size = sizeof(log_event_freopen_t);

typedef struct {
  // For fprintf():
  FILE *stream;
  char *format;
  va_list ap;
} log_event_fprintf_t;

static const int log_event_fprintf_size = sizeof(log_event_fprintf_t);

typedef struct {
  // For fscanf():
  FILE *stream;
  char *format;
  int bytes;
  off_t data_offset;
} log_event_fscanf_t;

static const int log_event_fscanf_size = sizeof(log_event_fscanf_t);

typedef struct {
  // For fseek():
  FILE *stream;
  long offset;
  int whence;
} log_event_fseek_t;

static const int log_event_fseek_size = sizeof(log_event_fseek_t);

typedef struct {
  // For fputs():
  char *s;
  FILE *stream;
} log_event_fputs_t;

static const int log_event_fputs_size = sizeof(log_event_fputs_t);

typedef struct {
  // For fputc():
  int c;
  FILE *stream;
} log_event_fputc_t;

static const int log_event_fputc_size = sizeof(log_event_fputc_t);

typedef struct {
  // For getc():
  FILE *stream;
} log_event_getc_t;

static const int log_event_getc_size = sizeof(log_event_getc_t);

typedef struct {
  // For getcwd():
  char *buf;
  size_t size;
  off_t data_offset;
} log_event_getcwd_t;

static const int log_event_getcwd_size = sizeof(log_event_getcwd_t);

typedef struct {
  // For gettimeofday():
  struct timeval *tv;
  struct timezone *tz;
  struct timeval tv_val;
  struct timezone tz_val;
  int gettimeofday_retval;
} log_event_gettimeofday_t;

static const int log_event_gettimeofday_size = sizeof(log_event_gettimeofday_t);

typedef struct {
  // For fgetc():
  FILE *stream;
} log_event_fgetc_t;

static const int log_event_fgetc_size = sizeof(log_event_fgetc_t);

typedef struct {
  // For ungetc():
  int c;
  FILE *stream;
} log_event_ungetc_t;

static const int log_event_ungetc_size = sizeof(log_event_ungetc_t);

typedef struct {
  // For getline():
  char *lineptr;
  char *new_lineptr;
  size_t n;
  size_t new_n;
  FILE *stream;
  off_t data_offset;
} log_event_getline_t;

static const int log_event_getline_size = sizeof(log_event_getline_t);

typedef struct {
  // For link():
  char *oldpath;
  char *newpath;
} log_event_link_t;

static const int log_event_link_size = sizeof(log_event_link_t);

typedef struct {
  // For symlink():
  char *oldpath;
  char *newpath;
} log_event_symlink_t;

static const int log_event_symlink_size = sizeof(log_event_symlink_t);

typedef struct {
  // For listen():
  int sockfd;
  int backlog;
} log_event_listen_t;

static const int log_event_listen_size = sizeof(log_event_listen_t);

typedef struct {
  // For localtime():
  time_t *timep;
  struct tm localtime_retval;
} log_event_localtime_t;

static const int log_event_localtime_size = sizeof(log_event_localtime_t);

typedef struct {
  // For lseek():
  int fd;
  off_t offset;
  int whence;
} log_event_lseek_t;

static const int log_event_lseek_size = sizeof(log_event_lseek_t);

typedef struct {
  // For lseek64():
  int fd;
  off64_t offset;
  int whence;
} log_event_lseek64_t;

static const int log_event_lseek64_size = sizeof(log_event_lseek64_t);

typedef struct {
  // For llseek():
  int fd;
  loff_t offset;
  int whence;
} log_event_llseek_t;

static const int log_event_llseek_size = sizeof(log_event_llseek_t);

typedef struct {
  // For lxstat():
  int vers;
  char *path;
  struct stat buf;
} log_event_lxstat_t;

static const int log_event_lxstat_size = sizeof(log_event_lxstat_t);

typedef struct {
  // For lxstat64():
  int vers;
  char *path;
  struct stat64 buf;
} log_event_lxstat64_t;

static const int log_event_lxstat64_size = sizeof(log_event_lxstat64_t);

typedef struct {
  // For malloc():
  size_t size;
} log_event_malloc_t;

static const int log_event_malloc_size = sizeof(log_event_malloc_t);

typedef struct {
  // For mkdir():
  char *pathname;
  mode_t mode;
} log_event_mkdir_t;

static const int log_event_mkdir_size = sizeof(log_event_mkdir_t);

typedef struct {
  // For mkstemp():
  char *temp;
} log_event_mkstemp_t;

static const int log_event_mkstemp_size = sizeof(log_event_mkstemp_t);

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

static const int log_event_mmap_size = sizeof(log_event_mmap_t);

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

static const int log_event_mmap64_size = sizeof(log_event_mmap64_t);

typedef struct {
  // For mremap():
  void *old_address;
  size_t old_size;
  size_t new_size;
  int flags;
} log_event_mremap_t;

static const int log_event_mremap_size = sizeof(log_event_mremap_t);

typedef struct {
  // For munmap():
  void *addr;
  size_t length;
} log_event_munmap_t;

static const int log_event_munmap_size = sizeof(log_event_munmap_t);

typedef struct {
  // For open():
  char *path;
  int flags;
  mode_t open_mode;
} log_event_open_t;

static const int log_event_open_size = sizeof(log_event_open_t);

typedef struct {
  // For open64():
  char *path;
  int flags;
  mode_t open_mode;
} log_event_open64_t;

static const int log_event_open64_size = sizeof(log_event_open64_t);

typedef struct {
  // For opendir():
  char *name;
} log_event_opendir_t;

static const int log_event_opendir_size = sizeof(log_event_opendir_t);

typedef struct {
  // For openat():
  int dirfd;
  char *pathname;
  int flags;
} log_event_openat_t;

static const int log_event_openat_size = sizeof(log_event_openat_t);

typedef struct {
  // For pread():
  int fd;
  void* buf;
  size_t count;
  off_t offset;
  off_t data_offset; // offset into read saved data file
} log_event_pread_t;

static const int log_event_pread_size = sizeof(log_event_pread_t);

typedef struct {
  // For preadv():
  int fd;
  const struct iovec *iov;
  int iovcnt;
  off_t offset;
  off_t data_offset; // offset into read saved data file
} log_event_preadv_t;

static const int log_event_preadv_size = sizeof(log_event_preadv_t);

typedef struct {
  // For putc():
  int c;
  FILE *stream;
} log_event_putc_t;

static const int log_event_putc_size = sizeof(log_event_putc_t);

typedef struct {
  // For pwrite():
  int fd;
  void* buf;
  size_t count;
  off_t offset;
} log_event_pwrite_t;

static const int log_event_pwrite_size = sizeof(log_event_pwrite_t);

typedef struct {
  // For pwritev():
  int fd;
  const struct iovec *iov;
  int iovcnt;
  off_t offset;
} log_event_pwritev_t;

static const int log_event_pwritev_size = sizeof(log_event_pwritev_t);

typedef struct {
  // For calloc():
  size_t nmemb;
  size_t size;
} log_event_calloc_t;

static const int log_event_calloc_size = sizeof(log_event_calloc_t);

typedef struct {
  // For chmod():
  char *path;
  mode_t mode;
} log_event_chmod_t;

static const int log_event_chmod_size = sizeof(log_event_chmod_t);

typedef struct {
  // For chown():
  char *path;
  uid_t owner;
  gid_t group;
} log_event_chown_t;

static const int log_event_chown_size = sizeof(log_event_chown_t);

typedef struct {
  // For close():
  int fd;
} log_event_close_t;

static const int log_event_close_size = sizeof(log_event_close_t);

typedef struct {
  // For closedir():
  DIR *dirp;
} log_event_closedir_t;

static const int log_event_closedir_size = sizeof(log_event_closedir_t);

typedef struct {
  // For connect():
  int sockfd;
  struct sockaddr *serv_addr;
  socklen_t addrlen;
} log_event_connect_t;

static const int log_event_connect_size = sizeof(log_event_connect_t);

typedef struct {
  // For dup():
  int oldfd;
} log_event_dup_t;

static const int log_event_dup_size = sizeof(log_event_dup_t);

typedef struct {
  // For dup2():
  int oldfd;
  int newfd;
} log_event_dup2_t;

static const int log_event_dup2_size = sizeof(log_event_dup2_t);

typedef struct {
  // For dup3():
  int oldfd;
  int newfd;
  int flags;
} log_event_dup3_t;

static const int log_event_dup3_size = sizeof(log_event_dup3_t);

typedef struct {
  // For exec_barrier: special case.
} log_event_exec_barrier_t;

static const int log_event_exec_barrier_size = sizeof(log_event_exec_barrier_t);

typedef struct {
  // For realloc():
  size_t size;
  void *ptr;
} log_event_realloc_t;

static const int log_event_realloc_size = sizeof(log_event_realloc_t);

typedef struct {
  // For free():
  void *ptr;
} log_event_free_t;

static const int log_event_free_size = sizeof(log_event_free_t);

typedef struct {
  // For ftell():
  FILE *stream;
} log_event_ftell_t;

static const int log_event_ftell_size = sizeof(log_event_ftell_t);

typedef struct {
  // For fwrite():
  void *ptr;
  size_t size;
  size_t nmemb;
  FILE *stream;
} log_event_fwrite_t;

static const int log_event_fwrite_size = sizeof(log_event_fwrite_t);

typedef struct {
  // For fread():
  void *ptr;
  size_t size;
  size_t nmemb;
  FILE *stream;
  off_t data_offset; // offset into read saved data file
} log_event_fread_t;

static const int log_event_fread_size = sizeof(log_event_fread_t);

typedef struct {
  // For fsync():
  int fd;
} log_event_fsync_t;

static const int log_event_fsync_size = sizeof(log_event_fsync_t);

typedef struct {
  // For fxstat():
  int vers;
  int fd;
  struct stat buf;
} log_event_fxstat_t;

static const int log_event_fxstat_size = sizeof(log_event_fxstat_t);

typedef struct {
  // For fxstat64():
  int vers;
  int fd;
  struct stat64 buf;
} log_event_fxstat64_t;

static const int log_event_fxstat64_size = sizeof(log_event_fxstat64_t);

typedef struct {
  // For time():
  time_t time_retval;
  time_t *tloc;
} log_event_time_t;

static const int log_event_time_size = sizeof(log_event_time_t);

typedef struct {
  // For tmpfile():
  FILE tmpfile_retval;
} log_event_tmpfile_t;

static const int log_event_tmpfile_size = sizeof(log_event_tmpfile_t);

typedef struct {
  // For truncate():
  char *path;
  off_t length;
} log_event_truncate_t;

static const int log_event_truncate_size = sizeof(log_event_truncate_t);

typedef struct {
  // For srand():
  unsigned int seed;
} log_event_srand_t;

static const int log_event_srand_size = sizeof(log_event_srand_t);

typedef struct {
  // For socket():
  int domain;
  int type;
  int protocol;
} log_event_socket_t;

static const int log_event_socket_size = sizeof(log_event_socket_t);

typedef struct {
  // For socketpair():
  int domain;
  int type;
  int protocol;
  int *sv;
  int ret_sv[2];
} log_event_socketpair_t;

static const int log_event_socketpair_size = sizeof(log_event_socketpair_t);

typedef struct {
  // For xstat():
  int vers;
  char *path;
  struct stat buf;
} log_event_xstat_t;

static const int log_event_xstat_size = sizeof(log_event_xstat_t);

typedef struct {
  // For xstat64():
  int vers;
  char *path;
  struct stat64 buf;
} log_event_xstat64_t;

static const int log_event_xstat64_size = sizeof(log_event_xstat64_t);

typedef struct {
  // For epoll_create():
  int size;
} log_event_epoll_create_t;

static const int log_event_epoll_create_size = sizeof(log_event_epoll_create_t);

typedef struct {
  // For epoll_create1():
  int flags;
} log_event_epoll_create1_t;

static const int log_event_epoll_create1_size =
  sizeof(log_event_epoll_create1_t);

typedef struct {
  // For epoll_ctl():
  int epfd;
  int op;
  int fd;
  struct epoll_event *_event;
} log_event_epoll_ctl_t;

static const int log_event_epoll_ctl_size = sizeof(log_event_epoll_ctl_t);

typedef struct {
  // For epoll_wait():
  int epfd;
  struct epoll_event *events;
  int maxevents;
  int timeout;
  off_t data_offset; // offset into read saved data file
} log_event_epoll_wait_t;

static const int log_event_epoll_wait_size = sizeof(log_event_epoll_wait_t);

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

static const int log_event_getpwnam_r_size = sizeof(log_event_getpwnam_r_t);

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

static const int log_event_getpwuid_r_size = sizeof(log_event_getpwuid_r_t);

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

static const int log_event_getgrnam_r_size = sizeof(log_event_getgrnam_r_t);

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

static const int log_event_getgrgid_r_size = sizeof(log_event_getgrgid_r_t);

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

static const int log_event_getaddrinfo_size = sizeof(log_event_getaddrinfo_t);

typedef struct {
  // For freeaddrinfo():
  struct addrinfo *res;
} log_event_freeaddrinfo_t;

static const int log_event_freeaddrinfo_size = sizeof(log_event_freeaddrinfo_t);

typedef struct {
  // For getnameinfo():
  const struct sockaddr *sa;
  socklen_t salen;
  char *host;
  socklen_t hostlen;
  char *serv;
  socklen_t servlen;
  unsigned int flags;
  char ret_host[NI_MAXHOST];
  char ret_serv[NI_MAXSERV];
} log_event_getnameinfo_t;

static const int log_event_getnameinfo_size = sizeof(log_event_getnameinfo_t);

typedef struct {
  // For sendto();
  int sockfd;
  const void *buf;
  size_t len;
  int flags;
  const struct sockaddr *dest_addr;
  socklen_t addrlen;
} log_event_sendto_t;

static const int log_event_sendto_size = sizeof(log_event_sendto_t);

typedef struct {
    // For sendmsg();
  int sockfd;
  const struct msghdr *msg;
  int flags;
} log_event_sendmsg_t;

static const int log_event_sendmsg_size = sizeof(log_event_sendmsg_t);

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

static const int log_event_recvfrom_size = sizeof(log_event_recvfrom_t);

typedef struct {
  // For recvmsf();
  int sockfd;
  struct msghdr *msg;
  int flags;
  struct msghdr ret_msg;
  off_t data_offset;
  off_t control_buf_offset;
} log_event_recvmsg_t;

static const int log_event_recvmsg_size = sizeof(log_event_recvmsg_t);


typedef struct {
  // FIXME:
  event_code_t event;
  //unsigned char event;
  unsigned char isOptional;
  log_off_t log_offset;
  clone_id_t clone_id;
  int my_errno;
  void* retval;
} log_entry_header_t;

typedef struct {
  // We aren't going to map more than 256 system calls/functions.
  // We can expand it to 4 bytes if needed. However a single byte makes
  // things easier.
  // Shared among all events ("common area"):
  /* IMPORTANT: Adding new fields to the common area requires that you also
   * update the log_event_common_size definition. */
  log_entry_header_t header;

  union {
    log_event_access_t                           log_event_access;
    log_event_bind_t                             log_event_bind;
    log_event_pthread_mutex_lock_t               log_event_pthread_mutex_lock;
    log_event_pthread_mutex_trylock_t            log_event_pthread_mutex_trylock;
    log_event_pthread_mutex_unlock_t             log_event_pthread_mutex_unlock;
    log_event_pthread_rwlock_unlock_t            log_event_pthread_rwlock_unlock;
    log_event_pthread_rwlock_rdlock_t            log_event_pthread_rwlock_rdlock;
    log_event_pthread_rwlock_wrlock_t            log_event_pthread_rwlock_wrlock;
    log_event_pthread_cond_signal_t              log_event_pthread_cond_signal;
    log_event_pthread_cond_broadcast_t           log_event_pthread_cond_broadcast;
    log_event_pthread_cond_wait_t                log_event_pthread_cond_wait;
    log_event_pthread_cond_timedwait_t           log_event_pthread_cond_timedwait;
    log_event_select_t                           log_event_select;
    log_event_ppoll_t                           log_event_ppoll;
    log_event_read_t                             log_event_read;
    log_event_readv_t                            log_event_readv;
    log_event_readdir_t                          log_event_readdir;
    log_event_readdir_r_t                        log_event_readdir_r;
    log_event_readlink_t                         log_event_readlink;
    log_event_write_t                            log_event_write;
    log_event_writev_t                           log_event_writev;
    log_event_close_t                            log_event_close;
    log_event_closedir_t                         log_event_closedir;
    log_event_connect_t                          log_event_connect;
    log_event_dup_t                              log_event_dup;
    log_event_dup2_t                             log_event_dup2;
    log_event_dup3_t                             log_event_dup3;
    log_event_exec_barrier_t                     log_event_exec_barrier;
    log_event_accept_t                           log_event_accept;
    log_event_accept4_t                          log_event_accept4;
    log_event_getpeername_t                      log_event_getpeername;
    log_event_getsockname_t                      log_event_getsockname;
    log_event_setsockopt_t                       log_event_setsockopt;
    log_event_getsockopt_t                       log_event_getsockopt;
    log_event_ioctl_t                            log_event_ioctl;
    log_event_pthread_create_t                   log_event_pthread_create;
    log_event_libc_memalign_t                    log_event_libc_memalign;
    log_event_fclose_t                           log_event_fclose;
    log_event_fchdir_t                           log_event_fchdir;
    log_event_fcntl_t                            log_event_fcntl;
    log_event_fdatasync_t                        log_event_fdatasync;
    log_event_fdopen_t                           log_event_fdopen;
    log_event_fdopendir_t                        log_event_fdopendir;
    log_event_fgets_t                            log_event_fgets;
    log_event_ferror_t                           log_event_ferror;
    log_event_feof_t                             log_event_feof;
    log_event_fileno_t                           log_event_fileno;
    log_event_fflush_t                           log_event_fflush;
    log_event_setvbuf_t                          log_event_setvbuf;
    log_event_fopen_t                            log_event_fopen;
    log_event_fopen64_t                          log_event_fopen64;
    log_event_freopen_t                          log_event_freopen;
    log_event_fprintf_t                          log_event_fprintf;
    log_event_fscanf_t                           log_event_fscanf;
    log_event_fseek_t                            log_event_fseek;
    log_event_fputs_t                            log_event_fputs;
    log_event_fputc_t                            log_event_fputc;
    log_event_getc_t                             log_event_getc;
    log_event_getcwd_t                           log_event_getcwd;
    log_event_gettimeofday_t                     log_event_gettimeofday;
    log_event_fgetc_t                            log_event_fgetc;
    log_event_ungetc_t                           log_event_ungetc;
    log_event_getline_t                          log_event_getline;
    log_event_open_t                             log_event_open;
    log_event_open64_t                           log_event_open64;
    log_event_openat_t                           log_event_openat;
    log_event_opendir_t                          log_event_opendir;
    log_event_pread_t                            log_event_pread;
    log_event_preadv_t                           log_event_preadv;
    log_event_putc_t                             log_event_putc;
    log_event_pwrite_t                           log_event_pwrite;
    log_event_pwritev_t                          log_event_pwritev;
    log_event_pthread_kill_t                     log_event_pthread_kill;
    log_event_pthread_join_t                     log_event_pthread_join;
    log_event_pthread_exit_t                     log_event_pthread_exit;
    log_event_pthread_detach_t                   log_event_pthread_detach;
    log_event_link_t                             log_event_link;
    log_event_symlink_t                          log_event_symlink;
    log_event_listen_t                           log_event_listen;
    log_event_localtime_t                        log_event_localtime;
    log_event_lseek_t                            log_event_lseek;
    log_event_lseek64_t                          log_event_lseek64;
    log_event_llseek_t                           log_event_llseek;
    log_event_lxstat_t                           log_event_lxstat;
    log_event_lxstat64_t                         log_event_lxstat64;
    log_event_malloc_t                           log_event_malloc;
    log_event_mkdir_t                            log_event_mkdir;
    log_event_mkstemp_t                          log_event_mkstemp;
    log_event_mmap_t                             log_event_mmap;
    log_event_mmap64_t                           log_event_mmap64;
    log_event_mremap_t                           log_event_mremap;
    log_event_munmap_t                           log_event_munmap;
    log_event_calloc_t                           log_event_calloc;
    log_event_chmod_t                            log_event_chmod;
    log_event_chown_t                            log_event_chown;
    log_event_realloc_t                          log_event_realloc;
    log_event_free_t                             log_event_free;
    log_event_ftell_t                            log_event_ftell;
    log_event_fwrite_t                           log_event_fwrite;
    log_event_fread_t                            log_event_fread;
    log_event_fsync_t                            log_event_fsync;
    log_event_fxstat_t                           log_event_fxstat;
    log_event_fxstat64_t                         log_event_fxstat64;
    log_event_time_t                             log_event_time;
    log_event_tmpfile_t                          log_event_tmpfile;
    log_event_truncate_t                         log_event_truncate;
    log_event_unlink_t                           log_event_unlink;
    log_event_user_t                             log_event_user;
    log_event_srand_t                            log_event_srand;
    log_event_socket_t                           log_event_socket;
    log_event_socketpair_t                       log_event_socketpair;
    log_event_rand_t                             log_event_rand;
    log_event_rename_t                           log_event_rename;
    log_event_rewind_t                           log_event_rewind;
    log_event_rmdir_t                            log_event_rmdir;
    log_event_signal_handler_t                   log_event_signal_handler;
    log_event_sigwait_t                          log_event_sigwait;
    log_event_xstat_t                            log_event_xstat;
    log_event_xstat64_t                          log_event_xstat64;
    log_event_epoll_create_t                     log_event_epoll_create;
    log_event_epoll_create1_t                    log_event_epoll_create1;
    log_event_epoll_ctl_t                        log_event_epoll_ctl;
    log_event_epoll_wait_t                       log_event_epoll_wait;
    log_event_getpwnam_r_t                       log_event_getpwnam_r;
    log_event_getpwuid_r_t                       log_event_getpwuid_r;
    log_event_getgrnam_r_t                       log_event_getgrnam_r;
    log_event_getgrgid_r_t                       log_event_getgrgid_r;
    log_event_getaddrinfo_t                      log_event_getaddrinfo;
    log_event_freeaddrinfo_t                     log_event_freeaddrinfo;
    log_event_getnameinfo_t                      log_event_getnameinfo;
    log_event_sendto_t                           log_event_sendto;
    log_event_sendmsg_t                          log_event_sendmsg;
    log_event_recvfrom_t                         log_event_recvfrom;
    log_event_recvmsg_t                          log_event_recvmsg;
  } event_data;
} log_entry_t;

#define log_event_common_size                                          \
  (sizeof(event_code_t)  +  /* event */                                \
   sizeof(unsigned char) +  /* isOptional */                           \
   sizeof(log_off_t)     +  /* log_offset */                           \
   sizeof(clone_id_t)    +  /* clone_id */                             \
   sizeof(int)           +  /* my_errno */                             \
   sizeof(void *))          /* retval */


#define GET_FIELD(entry, event, field)     entry.event_data.log_event_##event.field
#define GET_FIELD_PTR(entry, event, field) entry->event_data.log_event_##event.field
#define ARE_FIELDS_EQUAL_PTR(e1, e2, event, field) (GET_FIELD_PTR(e1, event, field) == \
                                                      GET_FIELD_PTR(e2, event, field))

#define SET_FIELD2(entry,event,field,field2) GET_FIELD(entry, event, field) = field2

#define SET_FIELD(entry, event, field) SET_FIELD2(entry, event, field, field)

#define SET_FIELD_FROM(entry, event, field, source) \
  GET_FIELD(entry, event, field) = GET_FIELD(source, event, field)

#define GET_COMMON(entry, field) entry.header.field
#define GET_COMMON_PTR(entry, field) entry->header.field

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

#define IFNAME_GET_EVENT_SIZE(name, event, event_size)                  \
  do {                                                                  \
    if (event == name##_event)          \
      event_size = log_event_##name##_size;                             \
  } while(0)

#define IFNAME_READ_ENTRY_FROM_LOG(name, source, entry)                    \
  do {                                                                  \
    if (GET_COMMON(entry,event) == name##_event) {           \
      memcpy(&entry.event_data.log_event_##name, source,      \
             log_event_##name##_size);                                     \
    }                                                                   \
  } while(0)

#define IFNAME_WRITE_ENTRY_TO_LOG(name, dest, entry)                \
  do {                                                                  \
    if (GET_COMMON(entry,event) == name##_event) {               \
      memcpy(dest, &entry.event_data.log_event_##name,              \
             log_event_##name##_size);                                     \
    }                                                                   \
  } while(0)

#define GET_EVENT_SIZE(event, event_size)                               \
  do {                                                                  \
    FOREACH_NAME(IFNAME_GET_EVENT_SIZE, event, event_size);             \
  } while(0)

#define READ_ENTRY_FROM_LOG(source, entry)                          \
  do {                                                                  \
    FOREACH_NAME(IFNAME_READ_ENTRY_FROM_LOG, source, entry);        \
  } while(0)

#define WRITE_ENTRY_TO_LOG(dest, entry)                      \
  do {                                                                  \
    FOREACH_NAME(IFNAME_WRITE_ENTRY_TO_LOG, dest, entry);    \
  } while(0)

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
LIB_PRIVATE extern dmtcp::map<clone_id_t, pthread_t> *clone_id_to_tid_table;
LIB_PRIVATE extern dmtcp::map<pthread_t, clone_id_t> *tid_to_clone_id_table;
LIB_PRIVATE extern dmtcp::map<pthread_t, pthread_join_retval_t> pthread_join_retvals;
LIB_PRIVATE extern char RECORD_LOG_PATH[RECORD_LOG_PATH_MAX];
LIB_PRIVATE extern char RECORD_READ_DATA_LOG_PATH[RECORD_LOG_PATH_MAX];
LIB_PRIVATE extern int             read_data_fd;
LIB_PRIVATE extern int             sync_logging_branch;
LIB_PRIVATE extern int             log_all_allocs;

LIB_PRIVATE extern dmtcp::SynchronizationLog global_log;

// TODO: rename this, since a log entry is not a char. maybe log_event_TYPE_SIZE?
#define LOG_ENTRY_SIZE sizeof(char)
LIB_PRIVATE extern pthread_mutex_t global_clone_counter_mutex;

/* Thread locals: */
LIB_PRIVATE extern __thread clone_id_t my_clone_id;
LIB_PRIVATE extern __thread int in_mmap_wrapper;
LIB_PRIVATE extern __thread unsigned char isOptionalEvent;
LIB_PRIVATE extern __thread bool ok_to_log_next_func;

/* Volatiles: */
LIB_PRIVATE extern volatile clone_id_t    global_clone_counter;
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
LIB_PRIVATE int    validAddress(void *addr);
LIB_PRIVATE ssize_t writeAll(int fd, const void *buf, size_t count);

/* These 'create_XXX_entry' functions are used library-wide by their
   respective wrapper functions. Their usages are hidden by the
   WRAPPER_HEADER macro. Thus, unless we think of a better design, the
   visibility of these functions is LIB_PRIVATE.

   XXX: These functions should only be needed in their respective
   wrapper files. Couldn't we define these functions only in those
   files, instead of making them globally (within the library) visible
   here? */
#define CREATE_ENTRY_FUNC(name, ...) \
  LIB_PRIVATE TURN_CHECK_P(name##_turn_check); \
  LIB_PRIVATE log_entry_t create_##name##_entry(clone_id_t clone_id, \
                                                event_code_t event, ##__VA_ARGS__)
/* ##__VA_ARGS__ is a GNU extension -- it means omit the variadic
   arguments if the list is empty. It will also then delete the
   extra comma. */

CREATE_ENTRY_FUNC(accept, int sockfd,
                  struct sockaddr *addr, socklen_t *addrlen);
CREATE_ENTRY_FUNC(accept4, int sockfd,
                  struct sockaddr *addr, socklen_t *addrlen, int flags);
CREATE_ENTRY_FUNC(access, const char *pathname, int mode);
CREATE_ENTRY_FUNC(bind, int sockfd,
                  const struct sockaddr *my_addr, socklen_t addrlen);
CREATE_ENTRY_FUNC(calloc, size_t nmemb, size_t size);
CREATE_ENTRY_FUNC(chmod, const char *path, mode_t mode);
CREATE_ENTRY_FUNC(chown, const char *path, uid_t owner, gid_t group);
CREATE_ENTRY_FUNC(close, int fd);
CREATE_ENTRY_FUNC(closedir, DIR *dirp);
CREATE_ENTRY_FUNC(connect, int sockfd,
                  const struct sockaddr *serv_addr, socklen_t addrlen);
CREATE_ENTRY_FUNC(dup, int oldfd);
CREATE_ENTRY_FUNC(dup2, int oldfd, int newfd);
CREATE_ENTRY_FUNC(dup3, int oldfd, int newfd, int flags);
CREATE_ENTRY_FUNC(fcntl, int fd, int cmd, long arg_3_l, struct flock *arg_3_f);
CREATE_ENTRY_FUNC(fclose, FILE *fp);
CREATE_ENTRY_FUNC(fchdir, int fd);
CREATE_ENTRY_FUNC(fdatasync, int fd);
CREATE_ENTRY_FUNC(fdopen, int fd, const char *mode);
CREATE_ENTRY_FUNC(fdopendir, int fd);
CREATE_ENTRY_FUNC(fgets, char *s, int size, FILE *stream);
CREATE_ENTRY_FUNC(ferror, FILE *stream);
CREATE_ENTRY_FUNC(feof, FILE *stream);
CREATE_ENTRY_FUNC(fileno, FILE *stream);
CREATE_ENTRY_FUNC(fflush, FILE *stream);
CREATE_ENTRY_FUNC(setvbuf, FILE *stream, char *buf, int mode, size_t size);
CREATE_ENTRY_FUNC(fopen, const char *name, const char *mode);
CREATE_ENTRY_FUNC(fopen64, const char *name, const char *mode);
CREATE_ENTRY_FUNC(freopen, const char *path, const char *mode, FILE *stream);
CREATE_ENTRY_FUNC(fprintf, FILE *stream, const char *format, va_list ap);
CREATE_ENTRY_FUNC(fscanf, FILE *stream, const char *format, va_list ap);
CREATE_ENTRY_FUNC(fseek, FILE *stream, long offset, int whence);
CREATE_ENTRY_FUNC(fputs, const char *s, FILE *stream);
CREATE_ENTRY_FUNC(fputc, int c, FILE *stream);
CREATE_ENTRY_FUNC(free, void *ptr);
CREATE_ENTRY_FUNC(fsync, int fd);
CREATE_ENTRY_FUNC(ftell, FILE *stream);
CREATE_ENTRY_FUNC(fwrite, const void *ptr, size_t size, size_t nmemb,
                  FILE *stream);
CREATE_ENTRY_FUNC(fread, void *ptr, size_t size, size_t nmemb, FILE *stream);
CREATE_ENTRY_FUNC(fxstat, int vers, int fd, struct stat *buf);
CREATE_ENTRY_FUNC(fxstat64, int vers, int fd, struct stat64 *buf);
CREATE_ENTRY_FUNC(getc, FILE *stream);
CREATE_ENTRY_FUNC(getcwd, char *buf, size_t size);
CREATE_ENTRY_FUNC(gettimeofday, struct timeval *tv, struct timezone *tz);
CREATE_ENTRY_FUNC(fgetc, FILE *stream);
CREATE_ENTRY_FUNC(ungetc, int c, FILE *stream);
CREATE_ENTRY_FUNC(getline, char **lineptr, size_t *n, FILE *stream);
CREATE_ENTRY_FUNC(getpeername, int sockfd,
                  struct sockaddr *addr, socklen_t *addrlen);
CREATE_ENTRY_FUNC(getsockname, int sockfd,
                  struct sockaddr *addr, socklen_t *addrlen);
CREATE_ENTRY_FUNC(libc_memalign, size_t boundary, size_t size);
CREATE_ENTRY_FUNC(link, const char *oldpath, const char *newpath);
CREATE_ENTRY_FUNC(symlink, const char *oldpath, const char *newpath);
CREATE_ENTRY_FUNC(listen, int sockfd, int backlog);
CREATE_ENTRY_FUNC(localtime, const time_t *timep);
CREATE_ENTRY_FUNC(lseek, int fd, off_t offset, int whence);
CREATE_ENTRY_FUNC(lseek64, int fd, off64_t offset, int whence);
CREATE_ENTRY_FUNC(llseek, int fd, loff_t offset, int whence);
CREATE_ENTRY_FUNC(lxstat, int vers, const char *path, struct stat *buf);
CREATE_ENTRY_FUNC(lxstat64, int vers, const char *path, struct stat64 *buf);
CREATE_ENTRY_FUNC(malloc, size_t size);
CREATE_ENTRY_FUNC(mkdir, const char *pathname, mode_t mode);
CREATE_ENTRY_FUNC(mkstemp, char *temp);
CREATE_ENTRY_FUNC(mmap, void *addr,
                  size_t length, int prot, int flags, int fd, off_t offset);
CREATE_ENTRY_FUNC(mmap64, void *addr,
                  size_t length, int prot, int flags, int fd, off64_t offset);
CREATE_ENTRY_FUNC(munmap, void *addr, size_t length);
CREATE_ENTRY_FUNC(mremap,
                  void *old_address, size_t old_size, size_t new_size,
                  int flags, void *new_addr);
CREATE_ENTRY_FUNC(open, const char *path, int flags, mode_t mode);
CREATE_ENTRY_FUNC(open64, const char *path, int flags, mode_t mode);
CREATE_ENTRY_FUNC(openat, int dirfd, const char *pathname, int flags);
CREATE_ENTRY_FUNC(opendir, const char *name);
CREATE_ENTRY_FUNC(pread, int fd, void* buf, size_t count, off_t offset);
CREATE_ENTRY_FUNC(preadv, int fd, const struct iovec *iov, int iovcnt, off_t offset);
CREATE_ENTRY_FUNC(putc, int c, FILE *stream);
CREATE_ENTRY_FUNC(pwrite, int fd, const void* buf, size_t count, off_t offset);
CREATE_ENTRY_FUNC(pwritev, int fd, const struct iovec *iov, int iovcnt, off_t offset);
CREATE_ENTRY_FUNC(pthread_cond_broadcast, pthread_cond_t *cond_var);
CREATE_ENTRY_FUNC(pthread_cond_signal, pthread_cond_t *cond_var);
CREATE_ENTRY_FUNC(pthread_cond_wait,
                  pthread_cond_t *cond_var, pthread_mutex_t *mutex);
CREATE_ENTRY_FUNC(pthread_cond_timedwait,
                  pthread_cond_t *cond_var, pthread_mutex_t *mutex,
                  const struct timespec *abstime);
CREATE_ENTRY_FUNC(pthread_rwlock_unlock, pthread_rwlock_t *rwlock);
CREATE_ENTRY_FUNC(pthread_rwlock_rdlock, pthread_rwlock_t *rwlock);
CREATE_ENTRY_FUNC(pthread_rwlock_wrlock, pthread_rwlock_t *rwlock);
CREATE_ENTRY_FUNC(pthread_create,
                  pthread_t *thread, const pthread_attr_t *attr,
                  void *(*start_routine)(void*), void *arg);
CREATE_ENTRY_FUNC(pthread_detach, pthread_t thread);
CREATE_ENTRY_FUNC(pthread_exit, void *value_ptr);
CREATE_ENTRY_FUNC(pthread_join, pthread_t thread, void *value_ptr);
CREATE_ENTRY_FUNC(pthread_kill, pthread_t thread, int sig);
CREATE_ENTRY_FUNC(pthread_mutex_lock, pthread_mutex_t *mutex);
CREATE_ENTRY_FUNC(pthread_mutex_trylock, pthread_mutex_t *mutex);
CREATE_ENTRY_FUNC(pthread_mutex_unlock, pthread_mutex_t *mutex);
CREATE_ENTRY_FUNC(rand);
CREATE_ENTRY_FUNC(read, int fd, void* buf_addr, size_t count);
CREATE_ENTRY_FUNC(readv, int fd, const struct iovec *iov, int iovcnt);
CREATE_ENTRY_FUNC(readdir, DIR *dirp);
CREATE_ENTRY_FUNC(readdir_r,
                  DIR *dirp, struct dirent *entry, struct dirent **result);
CREATE_ENTRY_FUNC(readlink, const char *path, char *buf, size_t bufsiz);
CREATE_ENTRY_FUNC(realloc, void *ptr, size_t size);
CREATE_ENTRY_FUNC(rename, const char *oldpath, const char *newpath);
CREATE_ENTRY_FUNC(rewind, FILE *stream);
CREATE_ENTRY_FUNC(rmdir, const char *pathname);
CREATE_ENTRY_FUNC(select, int nfds,
                  fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                  struct timeval *timeout);
CREATE_ENTRY_FUNC(ppoll, struct pollfd *fds, nfds_t nfds,
                  const struct timespec *timeout_ts, const sigset_t *sigmask);
CREATE_ENTRY_FUNC(setsockopt,
                  int sockfd, int level, int optname,
                  const void* optval, socklen_t optlen);
CREATE_ENTRY_FUNC(getsockopt,
                  int sockfd, int level, int optname,
                  void* optval, socklen_t* optlen);
CREATE_ENTRY_FUNC(ioctl, int d, int request, void* arg);
CREATE_ENTRY_FUNC(signal_handler, int sig, siginfo_t *info, void *data);
CREATE_ENTRY_FUNC(sigwait, const sigset_t *set, int *sig);
CREATE_ENTRY_FUNC(srand, unsigned int seed);
CREATE_ENTRY_FUNC(socket, int domain, int type, int protocol);
CREATE_ENTRY_FUNC(socketpair, int domain, int type, int protocol, int sv[2]);
CREATE_ENTRY_FUNC(xstat, int vers, const char *path, struct stat *buf);
CREATE_ENTRY_FUNC(xstat64, int vers, const char *path, struct stat64 *buf);
CREATE_ENTRY_FUNC(time, time_t *tloc);
CREATE_ENTRY_FUNC(tmpfile);
CREATE_ENTRY_FUNC(truncate, const char *path, off_t length);
CREATE_ENTRY_FUNC(unlink, const char *pathname);
CREATE_ENTRY_FUNC(write, int fd, const void* buf_addr, size_t count);
CREATE_ENTRY_FUNC(writev, int fd, const struct iovec *iov, int iovcnt);
CREATE_ENTRY_FUNC(epoll_create, int size);
CREATE_ENTRY_FUNC(epoll_create1, int flags);
CREATE_ENTRY_FUNC(epoll_ctl,
                  int epfd, int op, int fd, struct epoll_event *_event);
CREATE_ENTRY_FUNC(epoll_wait, int epfd,
                  struct epoll_event *events, int maxevents, int timeout);
CREATE_ENTRY_FUNC(getpwnam_r, const char *name, struct passwd *pwd,
                  char *buf, size_t buflen, struct passwd **result);
CREATE_ENTRY_FUNC(getpwuid_r, uid_t uid, struct passwd *pwd,
                  char *buf, size_t buflen, struct passwd **result);
CREATE_ENTRY_FUNC(getgrnam_r, const char *name, struct group *grp,
                  char *buf, size_t buflen, struct group **result);
CREATE_ENTRY_FUNC(getgrgid_r, gid_t gid, struct group *grp,
                  char *buf, size_t buflen, struct group **result);
CREATE_ENTRY_FUNC(getaddrinfo, const char *node, const char *service,
                  const struct addrinfo *hints, struct addrinfo **res);
CREATE_ENTRY_FUNC(freeaddrinfo, struct addrinfo *res);
CREATE_ENTRY_FUNC(getnameinfo, const struct sockaddr *sa, socklen_t salen,
                  char *host, socklen_t hostlen, char *serv, socklen_t servlen,
                  unsigned int flags);

CREATE_ENTRY_FUNC(sendto, int sockfd, const void *buf, size_t len, int flags,
                  const struct sockaddr *dest_addr, socklen_t addrlen);
CREATE_ENTRY_FUNC(sendmsg, int sockfd, const struct msghdr *msg, int flags);
CREATE_ENTRY_FUNC(recvfrom, int sockfd, void *buf, size_t len, int flags,
                  struct sockaddr *src_addr, socklen_t *addrlen);
CREATE_ENTRY_FUNC(recvmsg, int sockfd, struct msghdr *msg, int flags);

/* Special case: user synchronized events. */
CREATE_ENTRY_FUNC(user);
/* Special case: exec barrier (notice no clone id or event). */
LIB_PRIVATE log_entry_t create_exec_barrier_entry();

LIB_PRIVATE void waitForTurn(log_entry_t *my_entry, turn_pred_t pred);
LIB_PRIVATE void waitForExecBarrier();
#endif // SYNCHRONIZATION_LOGGING_H
