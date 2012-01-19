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

#ifndef FRED_WRAPPERS_H
#define FRED_WRAPPERS_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <utime.h>
#include <sys/time.h>
#include <stdarg.h>
#include <asm/ldt.h>
#include <stdio.h>
#include <thread_db.h>
#include <sys/procfs.h>
#include <syslog.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/epoll.h>
#include <dirent.h>
#include <unistd.h>
#include <poll.h>
#include <pwd.h>
#include <grp.h>
#include <netdb.h>

#include "constants.h"

#define LIB_PRIVATE __attribute__ ((visibility ("hidden")))

#ifdef __cplusplus
extern "C"
{
#endif

  extern int fred_wrappers_initializing;
  void _dmtcp_setup_trampolines();
  void initialize_wrappers();

#if __GLIBC_PREREQ(2,5)
# define READLINK_RET_TYPE ssize_t
#else
# define READLINK_RET_TYPE int
#endif


#define FOREACH_RECORD_REPLAY_WRAPPER_1(MACRO) \
  MACRO(void, empty) \
  MACRO(int, accept, int sockfd, struct sockaddr *addr, socklen_t *addrlen) \
  MACRO(int, accept4, int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags) \
  MACRO(int, access, const char *pathname, int mode) \
  MACRO(int, bind, int sockfd,  const struct  sockaddr  *my_addr, socklen_t addrlen) \
  MACRO(void*, calloc, size_t nmemb, size_t size) \
  MACRO(int, chmod, const char *path, mode_t mode) \
  MACRO(int, chown, const char *path, uid_t owner, gid_t group) \
  MACRO(int, close, int fd) \
  MACRO(int, closedir, DIR *dirp) \
  MACRO(int, connect, int sockfd,  const  struct sockaddr *serv_addr, socklen_t addrlen) \
  MACRO(int, dup, int oldfd) \
  MACRO(int, dup2, int oldfd, int newfd) \
  MACRO(int, dup3, int oldfd, int newfd, int flags) \
  MACRO(int, fcntl, int fd, int cmd, void *arg) \
  MACRO(int, fclose, FILE *fp) \
  MACRO(int, fchdir, int fd) \
  MACRO(int, fdatasync, int fd) \
  MACRO(FILE*, fdopen, int fd, const char *mode) \
  MACRO(DIR*, fdopendir, int fd) \
  MACRO(char*, fgets, char *s, int size, FILE *stream) \
  MACRO(int, ferror, FILE *stream) \
  MACRO(int, feof, FILE *stream) \
  MACRO(int, fileno, FILE *stream) \
  MACRO(int, fflush, FILE *stream) \
  MACRO(int, setvbuf, FILE *stream, char *buf, int mode, size_t size) \
  MACRO(FILE*, fopen, const char *path, const char *mode) \
  MACRO(FILE*, fopen64, const char *path, const char *mode) \
  MACRO(FILE*, freopen, const char *path, const char *mode, FILE *stream) \
  MACRO(int, fseek, FILE *stream, long offset, int whence) \
  MACRO(int, fputs, const char *s, FILE *stream) \
  MACRO(int, fputc, int, FILE *stream) \
  MACRO(void, free, void *ptr) \
  MACRO(int, fsync, int fd) \
  MACRO(long, ftell, FILE *stream) \
  MACRO(size_t, fwrite, const void *ptr, size_t size, size_t nmemb, FILE *stream) \
  MACRO(size_t, fread, void *ptr, size_t size, size_t nmemb, FILE *stream) \
  MACRO(int, getc, FILE *stream) \
  MACRO(char*, getcwd, char *buf, size_t size) \
  MACRO(int, gettimeofday, struct timeval *tv, struct timezone *tz) \
  MACRO(int, fgetc, FILE *stream) \
  MACRO(int, ungetc, int c, FILE *stream) \
  MACRO(ssize_t, getline, char **lineptr, size_t *n, FILE *stream) \
  MACRO(ssize_t, getdelim, char **lineptr, size_t *n, int delim, FILE *stream) \
  MACRO(int, getpeername, int sockfd, struct sockaddr *addr, socklen_t *addrlen) \
  MACRO(int, getsockname, int sockfd, struct sockaddr *addr, socklen_t *addrlen) \
  MACRO(int, link, const char *oldpath, const char *newpath) \
  MACRO(int, symlink, const char *oldpath, const char *newpath) \
  MACRO(int, listen, int sockfd, int backlog) \
  MACRO(struct tm*, localtime_r, const time_t *timep, struct tm *result) \
  MACRO(int, utime, const char *filename, const struct utimbuf *times) \
  MACRO(int, utimes, const char *filename, const struct timeval *times) \
  MACRO(int, futimes, int fd, const struct timeval tv[2]) \
  MACRO(int, lutimes, const char *filename, const struct timeval tv[2]) \
  MACRO(int, clock_getres, clockid_t clk_id, struct timespec *res) \
  MACRO(int, clock_gettime, clockid_t clk_id, struct timespec *tp) \
  MACRO(int, clock_settime, clockid_t clk_id, const struct timespec *tp) \
  MACRO(off_t, lseek, int fd, off_t offset, int whence) \
  MACRO(off64_t, lseek64, int fd, off64_t offset, int whence) \
  MACRO(loff_t, llseek, int fd, loff_t offset, int whence) \
  MACRO(void*, malloc, size_t size) \
  MACRO(int, mkdir, const char *pathname, mode_t mode) \
  MACRO(int, mkstemp, char *temp) \
  MACRO(void*, mmap, void *addr, size_t length, int prot, int flags, int fd, off_t offset) \
  MACRO(void*, mmap64, void *addr, size_t length, int prot, int flags, int fd, off64_t offset) \
  MACRO(int, munmap, void *addr, size_t length) \
  MACRO(void*, mremap, void *old_address, size_t old_size, size_t new_size, int flags, void *new_address) \
  MACRO(int, open, const char *pathname, int flags, mode_t mode) \
  MACRO(int, open64, const char *pathname, int flags, mode_t mode) \
  MACRO(int, openat, int dirfd, const char *pathname, int flags) \
  MACRO(DIR*, opendir, const char *name) \
  MACRO(ssize_t, pread, int fd, void *buf, size_t count, off_t offset) \
  MACRO(ssize_t, preadv, int fd, const struct iovec *iov, int iovcnt, off_t offset) \
  MACRO(int, putc, int c, FILE *stream) \
  MACRO(ssize_t, pwrite, int fd, const void *buf, size_t count, off_t offset) \
  MACRO(ssize_t, pwritev, int fd, const struct iovec *iov, int iovcnt, off_t offset) \
  MACRO(int, pthread_cond_broadcast, pthread_cond_t *cond) \
  MACRO(int, pthread_cond_signal, pthread_cond_t *cond) \
  MACRO(int, pthread_cond_wait, pthread_cond_t *cond, pthread_mutex_t *mutex) \
  MACRO(int, pthread_cond_timedwait, pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime) \
  MACRO(int, pthread_cond_destroy, pthread_cond_t *cond) \
  MACRO(int, pthread_rwlock_unlock, pthread_rwlock_t *rwlock) \
  MACRO(int, pthread_rwlock_rdlock, pthread_rwlock_t *rwlock) \
  MACRO(int, pthread_rwlock_wrlock, pthread_rwlock_t *rwlock) \
  MACRO(int, pthread_create, pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg) \
  MACRO(int, pthread_detach, pthread_t thread) \
  MACRO(void, pthread_exit, void *value_ptr) \
  MACRO(int, pthread_join, pthread_t thread, void **value_ptr) \
  MACRO(int, pthread_kill, pthread_t thread, int sig) \
  MACRO(int, pthread_mutex_lock, pthread_mutex_t *mutex) \
  MACRO(int, pthread_mutex_trylock, pthread_mutex_t *mutex) \
  MACRO(int, pthread_mutex_unlock, pthread_mutex_t *mutex) \
  MACRO(int, rand) \
  MACRO(ssize_t, read, int fd, void *buf, size_t count) \
  MACRO(ssize_t, readv, int fd, const struct iovec *iov, int iovcnt) \
  MACRO(struct dirent*, readdir, DIR *dirp) \
  MACRO(int, readdir_r, DIR *dirp, struct dirent *entry, struct dirent **result) \
  MACRO(ssize_t, readlink, const char *path, char *buf, size_t bufsiz) \
  MACRO(void*, realloc, void *ptr, size_t size) \
  MACRO(int, rename, const char *oldpath, const char *newpath) \
  MACRO(void, rewind, FILE *stream) \
  MACRO(int, rmdir, const char *pathname) \
  MACRO(int, select, int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) \
  MACRO(int, ppoll, struct pollfd *fds, nfds_t nfds, const struct timespec *timeout_ts, const sigset_t *sigmask) \
  MACRO(int, setsockopt, int s, int level, int optname, const void *optval, socklen_t optlen) \
  MACRO(int, getsockopt, int s, int level, int optname, void *optval, socklen_t *optlen) \
  MACRO(int, ioctl, int d,  int request, void *arg) \
  MACRO(int, sigwait, const sigset_t *set, int *sig) \
  MACRO(void, srand, unsigned int seed) \
  MACRO(int, socket, int domain, int type, int protocol) \
  MACRO(int, socketpair, int d, int type, int protocol, int sv[2]) \
  MACRO(time_t, time, time_t *tloc) \
  MACRO(FILE*, tmpfile) \
  MACRO(int, truncate, const char *path, off_t length) \
  MACRO(int, ftruncate, int fd, off_t length) \
  MACRO(int, truncate64, const char *path, off64_t length) \
  MACRO(int, ftruncate64, int fd, off64_t length) \
  MACRO(int, unlink, const char *pathname) \
  MACRO(ssize_t, write, int fd, const void *buf, size_t count) \
  MACRO(ssize_t, writev, int fd, const struct iovec *iov, int iovcnt) \
  MACRO(int, epoll_create, int size) \
  MACRO(int, epoll_create1, int flags) \
  MACRO(int, epoll_ctl, int epfd, int op, int fd, struct epoll_event *e) \
  MACRO(int, epoll_wait, int epfd, struct epoll_event *events, int maxevents, int timeout) \
  MACRO(int, getpwnam_r, const char *name, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result) \
  MACRO(int, getpwuid_r, uid_t uid, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result) \
  MACRO(int, getgrnam_r, const char *name, struct group *grp, char *buf, size_t buflen, struct group **result) \
  MACRO(int, getgrgid_r, gid_t gid, struct group *grp, char *buf, size_t buflen, struct group **result) \
  MACRO(int, getaddrinfo, const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) \
  MACRO(void, freeaddrinfo, struct addrinfo *res) \
  MACRO(int, getnameinfo, const struct sockaddr *sa, socklen_t salen, char *host, size_t hostlen, char *serv, size_t servlen, int flags) \
  MACRO(ssize_t, sendto, int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen) \
  MACRO(ssize_t, sendmsg, int sockfd, const struct msghdr *msg, int flags) \
  MACRO(ssize_t, recvfrom, int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) \
  MACRO(ssize_t, recvmsg, int sockfd, struct msghdr *msg, int flags) \
  MACRO(int, waitid, idtype_t idtype, id_t id, siginfo_t *infop, int options) \
  MACRO(pid_t, wait4, pid_t pid, __WAIT_STATUS status, int options, struct rusage *rusage) \
  MACRO(void, flockfile, FILE *filehandle) \
  MACRO(int, ftrylockfile, FILE *filehandle) \
  MACRO(void, funlockfile, FILE *filehandle) \
  MACRO(int, sigaction, int signum, const struct sigaction *act, struct sigaction *oldact) \
  MACRO(sighandler_t, signal, int signum, sighandler_t handler) \
  MACRO(sighandler_t, sigset, int sig, sighandler_t disp)

#define FOREACH_RECORD_REPLAY_WRAPPER_2(MACRO) \
  MACRO(int, fxstat, int vers, int fd, struct stat *buf) \
  MACRO(int, fxstat64, int vers, int fd, struct stat64 *buf) \
  MACRO(int, lxstat, int vers, const char *path, struct stat *buf) \
  MACRO(int, lxstat64, int vers, const char *path, struct stat64 *buf) \
  MACRO(int, xstat, int vers, const char *path, struct stat *buf) \
  MACRO(int, xstat64, int vers, const char *path, struct stat64 *buf) \
  MACRO(void*, libc_memalign, size_t boundary, size_t size)

#define FOREACH_RECORD_REPLAY_WRAPPER_3(MACRO) \
  MACRO(int, fprintf, FILE *stream, const char *format, va_list ap) \
  MACRO(int, fscanf, FILE *stream, const char *format, va_list ap)

#define FOREACH_RECORD_REPLAY_WRAPPER_4(MACRO) \
  MACRO(void, exec_barrier) \
  MACRO(void, signal_handler, int sig, siginfo_t *info, void *data) \
  MACRO(void, user)

#define FOREACH_RECORD_REPLAY_WRAPPER(MACRO) \
  FOREACH_RECORD_REPLAY_WRAPPER_1(MACRO) \
  FOREACH_RECORD_REPLAY_WRAPPER_2(MACRO) \
  FOREACH_RECORD_REPLAY_WRAPPER_3(MACRO) \
  FOREACH_RECORD_REPLAY_WRAPPER_4(MACRO)

# define ENUM(x) enum_ ## x
# define GEN_ENUM(type, name, ...) ENUM(name),
  typedef enum {
    FOREACH_RECORD_REPLAY_WRAPPER(GEN_ENUM)
    numTotalWrappers
  } LibcWrapperOffset;

#define DECLARE_REAL_FUNCTION(type, name, ...) \
  type _real_##name (__VA_ARGS__);

  FOREACH_RECORD_REPLAY_WRAPPER(DECLARE_REAL_FUNCTION);

#ifdef __cplusplus
}
#endif

#endif
