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

// TODO: Better way to do this. I think it was only a problem on dekaksi.
// Remove this, and see the compile error.
#define read _libc_read
#include <stdarg.h>
#include <stdlib.h>
#include <vector>
#include <list>
#include <string>
#include <fcntl.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/version.h>
#include <limits.h>
#include "constants.h"
#include "util.h"
#include  "jassert.h"
#include  "jfilesystem.h"

#include "fred_wrappers.h"
#include "synchronizationlogging.h"
#include <sys/mman.h>
#include <sys/syscall.h>
// TODO: hack to be able to compile this (fcntl wrapper).
#define open _libc_open
#define open64 _libc_open64
#define openat _libc_openat
#include <fcntl.h>
#undef open
#undef open64
#undef openat
#undef read

#ifdef ENABLE_MEM_ACCURACY
extern "C" char *getcwd(char *buf, size_t size)
{
  WRAPPER_HEADER(char *, getcwd, _real_getcwd, buf, size);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(char*, getcwd);
    if (retval != NULL) {
      /* retval will be a pointer to whatever buffer was used. If the
         user provided a NULL buffer, _real_getcwd allocated one on
         RECORD, but the optional event handler allocated it on REPLAY
         before we arrive here. Memory accurate replay allows us to
         depend on 'retval' pointing to the allocated buffer by the
         optional event handler. If the user provided a buffer, retval
         points to it. */
      WRAPPER_REPLAY_READ_FROM_READ_LOG(getcwd, retval, size);
    }
    WRAPPER_REPLAY_END(getcwd);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_getcwd(buf, size);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    if (retval != NULL) {
      /* retval will be a pointer to whatever buffer was used. If the
         user provided a NULL buffer, _real_getcwd will allocate one
         and retval points to it. If the user provided a buffer,
         retval points to it. */
      JASSERT(size > 0).Text("Unimplemented.");
      WRAPPER_LOG_WRITE_INTO_READ_LOG(getcwd, retval, size);
    }
    WRAPPER_LOG_WRITE_ENTRY(getcwd);
  }
  return retval;
}

extern "C" FILE *fdopen(int fd, const char *mode)
{
  WRAPPER_HEADER(FILE*, fdopen, _real_fdopen, fd, mode);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(FILE*, fdopen);
    if (retval != NULL) {
      *retval = GET_FIELD(my_entry, fdopen, fdopen_retval);
    }
    WRAPPER_REPLAY_END(fdopen);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_fdopen(fd, mode);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    if (retval != NULL) {
      SET_FIELD2(my_entry, fdopen, fdopen_retval, *retval);
    }
    WRAPPER_LOG_WRITE_ENTRY(fdopen);
  }
  return retval;
}

extern "C" int __open_2 (const char *path, int flags)
{
  BASIC_SYNC_WRAPPER(int, open, _real_open, path, flags, 0);
}

extern "C" int __open64_2 (const char *path, int flags)
{
  BASIC_SYNC_WRAPPER(int, open64, _real_open64, path, flags, 0);
}

extern "C" int open (const char *path, int flags, ... )
{
  mode_t mode = 0;
  // Handling the variable number of arguments
  if (flags & O_CREAT) {
    va_list arg;
    va_start (arg, flags);
    mode = va_arg (arg, int);
    va_end (arg);
  }
  BASIC_SYNC_WRAPPER(int, open, _real_open, path, flags, mode);
}

// FIXME: The 'fn64' version of functions is defined only when within
// __USE_LARGEFILE64 is #defined. The wrappers in this file need to consider
// this fact. The problem can occur, for example, when DMTCP is not compiled
// with __USE_LARGEFILE64 whereas the user-binary is. In that case the open64()
// call from user will come to DMTCP and DMTCP might fail to execute it
// properly.

// FIXME: Add the 'fn64' wrapper test cases to dmtcp test suite.
extern "C" int open64 (const char *path, int flags, ... )
{
  mode_t mode;
  // Handling the variable number of arguments
  if (flags & O_CREAT) {
    va_list arg;
    va_start (arg, flags);
    mode = va_arg (arg, int);
    va_end (arg);
  }
  BASIC_SYNC_WRAPPER(int, open64, _real_open64, path, flags, mode);
}

extern "C" int close ( int fd )
{
  BASIC_SYNC_WRAPPER(int, close, _real_close, fd);
}

extern "C" int fchdir(int fd)
{
  BASIC_SYNC_WRAPPER(int, fchdir, _real_fchdir, fd);
}

#if 0
/* Until we fix the readdir() bug for tar, this is commented out.  If
   we don't comment this out (and fdopendir also), readdir() does not
   function properly in tar.  This is a "special case hack" for tar 1.26. */
// TODO: handle the variable argument here.
extern "C" int openat(int dirfd, const char *pathname, int flags, ...)
{
  BASIC_SYNC_WRAPPER(int, openat, _real_openat, dirfd, pathname, flags);
}
#endif

#endif

extern "C" int truncate(const char *path, off_t length)
{
  BASIC_SYNC_WRAPPER(int, truncate, _real_truncate, path, length);
}

extern "C" int ftruncate(int fd, off_t length)
{
  BASIC_SYNC_WRAPPER(int, ftruncate, _real_ftruncate, fd, length);
}

extern "C" int truncate64(const char *path, off64_t length)
{
  BASIC_SYNC_WRAPPER(int, truncate64, _real_truncate64, path, length);
}

extern "C" int ftruncate64(int fd, off64_t length)
{
  BASIC_SYNC_WRAPPER(int, ftruncate64, _real_ftruncate64, fd, length);
}

extern "C" int chmod(const char *path, mode_t mode)
{
  BASIC_SYNC_WRAPPER(int, chmod, _real_chmod, path, mode);
}

extern "C" int chown(const char *path, uid_t owner, gid_t group)
{
  BASIC_SYNC_WRAPPER(int, chown, _real_chown, path, owner, group);
}

#ifdef ENABLE_MEM_ACCURACY
#define _XSTAT_COMMON_SYNC_WRAPPER(name, ...)                               \
  do {                                                                      \
    if (SYNC_IS_REPLAY) {                                                   \
      WRAPPER_REPLAY_START(name);                                           \
      if (retval == 0 && buf != NULL) {                                     \
        *buf = GET_FIELD(my_entry, name, ret_buf);                          \
      }                                                                     \
      WRAPPER_REPLAY_END(name);                                             \
    } else if (SYNC_IS_RECORD) {                                            \
      retval = _real_ ## name(__VA_ARGS__);                                 \
      if (retval != -1 && buf != NULL) {                                    \
        SET_FIELD2(my_entry, name, ret_buf, *buf);                          \
      }                                                                     \
      WRAPPER_LOG_WRITE_ENTRY(name);                                        \
    }                                                                       \
  }  while(0)

extern "C"
int __xstat(int vers, const char *path, struct stat *buf)
{
  WRAPPER_HEADER(int, xstat, _real_xstat, vers, path, buf);
  _XSTAT_COMMON_SYNC_WRAPPER(xstat, vers, path, buf);
  return retval;
}

extern "C"
int __xstat64(int vers, const char *path, struct stat64 *buf)
{
  WRAPPER_HEADER(int, xstat64, _real_xstat64, vers, path, buf);
  _XSTAT_COMMON_SYNC_WRAPPER(xstat64, vers, path, buf);
  return retval;
}

extern "C"
int __fxstat(int vers, int fd, struct stat *buf)
{
  WRAPPER_HEADER(int, fxstat, _real_fxstat, vers, fd, buf);
  _XSTAT_COMMON_SYNC_WRAPPER(fxstat, vers, fd, buf);
  return retval;
}

extern "C"
int __fxstat64(int vers, int fd, struct stat64 *buf)
{
  WRAPPER_HEADER(int, fxstat64, _real_fxstat64, vers, fd, buf);
  _XSTAT_COMMON_SYNC_WRAPPER(fxstat64, vers, fd, buf);
  return retval;
}

extern "C"
int __lxstat(int vers, const char *path, struct stat *buf)
{
  WRAPPER_HEADER(int, lxstat, _real_lxstat, vers, path, buf);
  _XSTAT_COMMON_SYNC_WRAPPER(lxstat, vers, path, buf);
  return retval;
}

extern "C"
int __lxstat64(int vers, const char *path, struct stat64 *buf)
{
  WRAPPER_HEADER(int, lxstat64, _real_lxstat64, vers, path, buf);
  _XSTAT_COMMON_SYNC_WRAPPER(lxstat64, vers, path, buf);
  return retval;
}
#endif

#if __GLIBC_PREREQ(2,5)
# define READLINK_RET_TYPE ssize_t
#else
# define READLINK_RET_TYPE int
#endif

extern "C" READLINK_RET_TYPE readlink(const char *path, char *buf,
                                      size_t bufsiz)
{
  WRAPPER_HEADER(READLINK_RET_TYPE, readlink, _real_readlink,
                 path, buf, bufsiz);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(READLINK_RET_TYPE, readlink);
    if (retval > 0 && buf != NULL) {
      WRAPPER_REPLAY_READ_FROM_READ_LOG(readlink, buf, retval);
    }
    WRAPPER_REPLAY_END(readlink);
  } else if (SYNC_IS_RECORD) {
    retval = _real_readlink(path, buf, bufsiz);
    if (retval > 0 && buf != NULL) {
      WRAPPER_LOG_WRITE_INTO_READ_LOG(readlink, buf, retval);
    }
    WRAPPER_LOG_WRITE_ENTRY(readlink);
  }
  return retval;
}

extern "C" char *realpath(const char *path, char *resolved_path)
{
  WRAPPER_HEADER(char *, realpath, _real_realpath, path, resolved_path);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(char*, realpath);
    if (retval != NULL) {
      int len = GET_FIELD(my_entry, realpath, len);
      WRAPPER_REPLAY_READ_FROM_READ_LOG(realpath, retval, len);
      // NULL-terminate as per man page.
      *(resolved_path + len) = '\0';
    }
    WRAPPER_REPLAY_END(realpath);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_realpath(path, resolved_path);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    if (retval != NULL) {
      int len = strlen(retval);
      SET_FIELD(my_entry, realpath, len);
      WRAPPER_LOG_WRITE_INTO_READ_LOG(realpath, retval, len);
    }
    WRAPPER_LOG_WRITE_ENTRY(realpath);
  }
  return retval;
}

extern "C" int select(int nfds, fd_set *readfds, fd_set *writefds,
                      fd_set *exceptfds, struct timeval *timeout)
{
  if (isPassthroughFd(nfds - 1)) {
    return _real_select(nfds, readfds, writefds, exceptfds, timeout);
  }

  WRAPPER_HEADER(int, select, _real_select, nfds, readfds, writefds, exceptfds, timeout);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(select);
    if (retval != -1) {
      copyFdSet(&GET_FIELD(my_entry, select, ret_readfds), readfds);
      copyFdSet(&GET_FIELD(my_entry, select, ret_writefds), writefds);
    }
    WRAPPER_REPLAY_END(select);
  } else if (SYNC_IS_RECORD) {
    retval = _real_select(nfds, readfds, writefds, exceptfds, timeout);
    int saved_errno = errno;
    if (retval != -1) {
      // Note that we're logging the *changed* fd set, so on replay we can
      // just read that from the log, load it into user's location and return.
      copyFdSet(readfds, &GET_FIELD(my_entry, select, ret_readfds));
      copyFdSet(writefds, &GET_FIELD(my_entry, select, ret_writefds));
    }
    errno = saved_errno;
    WRAPPER_LOG_WRITE_ENTRY(select);
  }
  return retval;
}

extern "C" int ppoll(struct pollfd *fds, nfds_t nfds,
                     const struct timespec *timeout_ts,
                     const sigset_t *sigmask)
{
  WRAPPER_HEADER(int, ppoll, _real_ppoll, fds, nfds, timeout_ts, sigmask)
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(ppoll);
    if (retval > 0 && fds != NULL) {
      WRAPPER_REPLAY_READ_FROM_READ_LOG(ppoll, (void*)fds,
                                        nfds * sizeof(struct pollfd));
    }
    WRAPPER_REPLAY_END(ppoll);
  } else if (SYNC_IS_RECORD) {
    retval = _real_ppoll(fds, nfds, timeout_ts, sigmask);
    int saved_errno = errno;
    if (retval > 0 && fds != NULL) {
      WRAPPER_LOG_WRITE_INTO_READ_LOG(ppoll, (void*)fds,
                                      nfds * sizeof(struct pollfd));
    }
    errno = saved_errno;
    WRAPPER_LOG_WRITE_ENTRY(ppoll);
  }
  return retval;
}

extern "C" int poll(struct pollfd *fds, nfds_t nfds, int time)
{
  struct timespec time_ts;
  struct timespec *timeout_ts;

  if (time <= -1) {
    timeout_ts = NULL;
  } else {
    timeout_ts = &time_ts;
    timeout_ts->tv_sec = time/1000;
    timeout_ts->tv_nsec = (time % 1000) * 1000000;
  }
  dmtcp::ThreadInfo::setOkToLogNextFnc();
  return ppoll(fds, nfds, timeout_ts, NULL);
}

extern "C" ssize_t read(int fd, void *buf, size_t count)
{
  if (isPassthroughFd(fd)) {
    return _real_read(fd, buf, count);
  }

  WRAPPER_HEADER(ssize_t, read, _real_read, fd, buf, count);

  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(ssize_t, read);
    // NOTE: We never actually call the user's _real_read. We don't
    // need to. We wait for the next event in the log that is the
    // READ_data_event, read from the read data log, and return the
    // corresponding value.
    if (retval > 0) {
      WRAPPER_REPLAY_READ_FROM_READ_LOG(read, buf, retval);
    }
    WRAPPER_REPLAY_END(read);
  } else if (SYNC_IS_RECORD) {
    // Note we don't call readAll here. It should be the responsibility of
    // the user code to handle EINTR if needed.
    retval = _real_read(fd, buf, count);
    if (retval > 0) {
      WRAPPER_LOG_WRITE_INTO_READ_LOG(read, buf, retval);
    }
    WRAPPER_LOG_WRITE_ENTRY(read);
  }
  return retval;
}

extern "C" ssize_t write(int fd, const void *buf, size_t count)
{
  if (isPassthroughFd(fd)) {
    return _real_write(fd, buf, count);
  }
  BASIC_SYNC_WRAPPER(ssize_t, write, _real_write, fd, buf, count);
}

extern "C" ssize_t pread(int fd, void *buf, size_t count, off_t offset)
{
  if (isPassthroughFd(fd)) {
    return _real_pread(fd, buf, count, offset);
  }
  WRAPPER_HEADER(ssize_t, pread, _real_pread, fd, buf, count, offset);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(ssize_t, pread);
    if (retval > 0) {
      WRAPPER_REPLAY_READ_FROM_READ_LOG(pread, buf, retval);
    }
    WRAPPER_REPLAY_END(pread);
  } else if (SYNC_IS_RECORD) {
    retval = _real_pread(fd, buf, count, offset);
    if (retval > 0) {
      WRAPPER_LOG_WRITE_INTO_READ_LOG(pread, buf, retval);
    }
    WRAPPER_LOG_WRITE_ENTRY(pread);
  }
  return retval;
}

extern "C" ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset)
{
  if (isPassthroughFd(fd)) {
    return _real_pwrite(fd, buf, count, offset);
  }
  BASIC_SYNC_WRAPPER(ssize_t, pwrite, _real_pwrite, fd, buf, count, offset);
}

extern "C" ssize_t readv(int fd, const struct iovec *iov, int iovcnt)
{
  if (isPassthroughFd(fd)) {
    return _real_readv(fd, iov, iovcnt);
  }

  WRAPPER_HEADER(ssize_t, readv, _real_readv, fd, iov, iovcnt);

  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(ssize_t, readv);
    if (retval > 0) {
      WRAPPER_REPLAY_READ_VECTOR_FROM_READ_LOG(readv, iov, iovcnt);
    }
    WRAPPER_REPLAY_END(readv);
  } else if (SYNC_IS_RECORD) {
    retval = _real_readv(fd, iov, iovcnt);
    if (retval > 0) {
      WRAPPER_LOG_WRITE_VECTOR_INTO_READ_LOG(readv, iov, iovcnt, retval);
    }
    WRAPPER_LOG_WRITE_ENTRY(readv);
  }
  return retval;
}

extern "C" ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
{
  if (isPassthroughFd(fd)) {
    return _real_writev(fd, iov, iovcnt);
  }
  BASIC_SYNC_WRAPPER(ssize_t, writev, _real_writev, fd, iov, iovcnt);
}

extern "C" ssize_t preadv(int fd, const struct iovec *iov, int iovcnt,
                          off_t offset)
{
  if (isPassthroughFd(fd)) {
    return _real_preadv(fd, iov, iovcnt, offset);
  }

  WRAPPER_HEADER_RAW(ssize_t, preadv, _real_preadv, fd, iov, iovcnt, offset);
  ssize_t retval;
  log_entry_t my_entry = create_preadv_entry(my_clone_id, preadv_event, fd, iov, iovcnt, offset);

  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(ssize_t, preadv);
    if (retval > 0) {
      WRAPPER_REPLAY_READ_VECTOR_FROM_READ_LOG(preadv, iov, iovcnt);
    }
    WRAPPER_REPLAY_END(preadv);
  } else if (SYNC_IS_RECORD) {
    retval = _real_preadv(fd, iov, iovcnt, offset);
    if (retval > 0) {
      WRAPPER_LOG_WRITE_VECTOR_INTO_READ_LOG(preadv, iov, iovcnt, retval);
    }
    WRAPPER_LOG_WRITE_ENTRY(preadv);
  }
  return retval;
}

extern "C" ssize_t pwritev(int fd, const struct iovec *iov, int iovcnt,
                           off_t offset)
{
  if (isPassthroughFd(fd)) {
    return _real_pwritev(fd, iov, iovcnt, offset);
  }
  BASIC_SYNC_WRAPPER(ssize_t, pwritev, _real_pwritev, fd, iov, iovcnt, offset);
}

extern "C" int access(const char *pathname, int mode)
{
  BASIC_SYNC_WRAPPER(int, access, _real_access, pathname, mode);
}

extern "C" int dup(int oldfd)
{
  BASIC_SYNC_WRAPPER(int, dup, _real_dup, oldfd);
}

extern "C" int dup2(int oldfd, int newfd)
{
  BASIC_SYNC_WRAPPER(int, dup2, _real_dup2, oldfd, newfd);
}

extern "C" int dup3(int oldfd, int newfd, int flags)
{
  BASIC_SYNC_WRAPPER(int, dup3, _real_dup3, oldfd, newfd, flags);
}

extern "C" off_t lseek(int fd, off_t offset, int whence)
{
  BASIC_SYNC_WRAPPER(off_t, lseek, _real_lseek, fd, offset, whence);
}

// FIXME: Add proper wrapper for lseek64 and llseek
extern "C" off64_t lseek64(int fd, off64_t offset, int whence)
{
  BASIC_SYNC_WRAPPER(off64_t, lseek64, _real_lseek64, fd, offset, whence);
}

extern "C" loff_t llseek(int fd, loff_t offset, int whence)
{
  BASIC_SYNC_WRAPPER(loff_t, llseek, _real_llseek, fd, offset, whence);
}

extern "C" int unlink(const char *pathname)
{
  BASIC_SYNC_WRAPPER(int, unlink, _real_unlink, pathname);
}

extern "C" int fdatasync(int fd)
{
  BASIC_SYNC_WRAPPER(int, fdatasync, _real_fdatasync, fd);
}

extern "C" int fsync(int fd)
{
  BASIC_SYNC_WRAPPER(int, fsync, _real_fsync, fd);
}

extern "C" int link(const char *oldpath, const char *newpath)
{
  BASIC_SYNC_WRAPPER(int, link, _real_link, oldpath, newpath);
}

extern "C" int symlink(const char *oldpath, const char *newpath)
{
  BASIC_SYNC_WRAPPER(int, symlink, _real_symlink, oldpath, newpath);
}

extern "C" int rename(const char *oldpath, const char *newpath)
{
  BASIC_SYNC_WRAPPER(int, rename, _real_rename, oldpath, newpath);
}

extern "C" int rmdir(const char *pathname)
{
  BASIC_SYNC_WRAPPER(int, rmdir, _real_rmdir, pathname);
}

extern "C" int mkdir(const char *pathname, mode_t mode)
{
  BASIC_SYNC_WRAPPER(int, mkdir, _real_mkdir, pathname, mode);
}

extern "C" struct dirent * /*__attribute__ ((optimize(0)))*/ readdir(DIR *dirp)
{
  WRAPPER_HEADER(struct dirent*, readdir, _real_readdir, dirp);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(struct dirent*, readdir);
    if (retval != NULL) {
      *retval = GET_FIELD(my_entry, readdir, readdir_retval);
    }
    WRAPPER_REPLAY_END(readdir);
  } else if (SYNC_IS_RECORD) {
    retval = _real_readdir(dirp);
    if (retval != NULL) {
      JASSERT(retval->d_reclen < 256);
      SET_FIELD2(my_entry, readdir, readdir_retval, *retval);
    }
    WRAPPER_LOG_WRITE_ENTRY(readdir);
  }
  return retval;
}

extern "C" int readdir_r(DIR *dirp, struct dirent *entry,
                         struct dirent **result)
{
  void *return_addr = GET_RETURN_ADDRESS();
  if (!shouldSynchronize(return_addr) || isProcessGDB()) {
    return _real_readdir_r(dirp, entry, result);
  }
  int retval;
  log_entry_t my_entry = create_readdir_r_entry(my_clone_id,
      readdir_r_event, dirp, entry, result);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(readdir_r);
    if (retval == 0 && entry != NULL) {
      *entry = GET_FIELD(my_entry, readdir_r, ret_entry);
    }
    if (retval == 0 && result != NULL) {
      *result = GET_FIELD(my_entry, readdir_r, ret_result);
    }
    if (retval != 0) {
      *result = NULL;
    }
    WRAPPER_REPLAY_END(readdir_r);
  } else if (SYNC_IS_RECORD) {
    retval = _real_readdir_r(dirp, entry, result);
    if (retval == 0 && entry != NULL) {
      SET_FIELD2(my_entry, readdir_r, ret_entry, *entry);
    }
    if (retval == 0 && result != NULL) {
      SET_FIELD2(my_entry, readdir_r, ret_result, *result);
    }
    WRAPPER_LOG_WRITE_ENTRY(readdir_r);
  }
  return retval;
}

#ifdef ENABLE_MEM_ACCURACY
extern "C" DIR *opendir(const char *name)
{
  WRAPPER_HEADER(DIR*, opendir, _real_opendir, name);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_TYPED(DIR*, opendir);
    //TODO: May be we should restore data in *retval;
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_opendir(name);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    WRAPPER_LOG_WRITE_ENTRY(opendir);
  }
  return retval;
}

extern "C" int closedir(DIR *dirp)
{
  WRAPPER_HEADER(int, closedir, _real_closedir, dirp);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY(closedir);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_closedir(dirp);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    WRAPPER_LOG_WRITE_ENTRY(closedir);
  }
  return retval;
}
#endif

extern "C" int mkstemp(char *temp)
{
  BASIC_SYNC_WRAPPER(int, mkstemp, _real_mkstemp, temp);
}

extern "C" int fcntl(int fd, int cmd, ...)
{
  va_list ap;
  void *arg = NULL;
  va_start(ap, cmd);
  arg = va_arg(ap, void*);
  va_end(ap);

  if (isPassthroughFd(fd - 1)) {
    return _real_fcntl(fd, cmd, arg);
  }

  WRAPPER_HEADER(int, fcntl, _real_fcntl, fd, cmd, arg);

  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(fcntl);
    if (cmd == F_GETLK && retval != -1 && arg != NULL) {
      *(struct flock*)arg = GET_FIELD(my_entry, fcntl, ret_flock);
    }
    WRAPPER_REPLAY_END(fcntl);
  } else if (SYNC_IS_RECORD) {
    retval = _real_fcntl(fd, cmd, arg);
    if (cmd == F_GETLK && retval != -1 && arg != NULL) {
      SET_FIELD2(my_entry, fcntl, ret_flock, *(struct flock*)arg);
    }
    WRAPPER_LOG_WRITE_ENTRY(fcntl);
  }

  return retval;
}


// FIXME: Ask Ana to write this wrapper.
#if 0
#ifdef PID_VIRTUALIZATION
int send_sigwinch;

void ioctl_helper(log_entry_t &my_entry, int &retval, int d, int request,
                  void *arg) {
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(ioctl);
    switch (request) {
      case SIOCGIFCONF: {
        *((struct ifconf *)arg) = GET_FIELD(my_entry, ioctl, ifconf_val);
        struct ifconf *i = (struct ifconf *)arg;
        WRAPPER_REPLAY_READ_FROM_READ_LOG(ioctl, i->ifc_buf, i->ifc_len);
        break;
      }
      case TIOCGWINSZ: {
        *((struct winsize *)arg) = GET_FIELD(my_entry, ioctl, win_val);
        break;
      }
      default:
        break;
    }
    WRAPPER_REPLAY_END(ioctl);
  } else if (SYNC_IS_RECORD) {
    retval = _real_ioctl(d, request, arg);
    switch (request) {
      case SIOCGIFCONF: {
        SET_FIELD2(my_entry, ioctl, ifconf_val, *((struct ifconf *)arg));
        struct ifconf *i = (struct ifconf *)arg;
        WRAPPER_LOG_WRITE_INTO_READ_LOG(ioctl, i->ifc_buf, i->ifc_len);
        break;
      }
      case TIOCGWINSZ: {
        SET_FIELD2(my_entry, ioctl, win_val, *((struct winsize *)arg));
        break;
      }
      default: {
        break;
      }
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
}
#endif
#endif

extern "C" int ioctl(int d,  unsigned long int request, ...)
{
  va_list ap;
  va_start(ap, request);
  void *arg = va_arg(ap, void*);
  va_end(ap);

  if (isPassthroughFd(d - 1)) {
    return _real_ioctl(d, request, arg);
  }

  WRAPPER_HEADER(int, ioctl, _real_ioctl, d, request, arg);

  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(ioctl);
    switch (request) {
      case SIOCGIFCONF: {
        *((struct ifconf *)arg) = GET_FIELD(my_entry, ioctl, ifconf_val);
        struct ifconf *i = (struct ifconf *)arg;
        WRAPPER_REPLAY_READ_FROM_READ_LOG(ioctl, i->ifc_buf, i->ifc_len);
        break;
      }
      case TIOCGWINSZ: {
        *((struct winsize *)arg) = GET_FIELD(my_entry, ioctl, win_val);
        break;
      }
      case FIONREAD: {
        *(int*) arg = GET_FIELD(my_entry, ioctl, fionread_val);
        break;
      }
      default:
        break;
    }
    WRAPPER_REPLAY_END(ioctl);
  } else if (SYNC_IS_RECORD) {
    retval = _real_ioctl(d, request, arg);
    switch (request) {
      case SIOCGIFCONF: {
        SET_FIELD2(my_entry, ioctl, ifconf_val, *((struct ifconf *)arg));
        struct ifconf *i = (struct ifconf *)arg;
        WRAPPER_LOG_WRITE_INTO_READ_LOG(ioctl, i->ifc_buf, i->ifc_len);
        break;
      }
      case TIOCGWINSZ: {
        SET_FIELD2(my_entry, ioctl, win_val, *((struct winsize *)arg));
        break;
      }
      case FIONREAD: {
        SET_FIELD2(my_entry, ioctl, win_val, *((struct winsize *)arg));
        break;
      }
      default: {
        break;
      }
    }
    WRAPPER_LOG_WRITE_ENTRY(ioctl);
  }

  return retval;
}

extern "C" int eventfd(unsigned int initval, int flags)
{
  errno = ENODEV;
  return -1;
}
