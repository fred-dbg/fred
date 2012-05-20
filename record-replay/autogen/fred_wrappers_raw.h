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

/* This file is auto-generated. Please modify wrappers.py and re-execute it
 * to make any changes to this file.
 */

#ifndef FRED_WRAPPERS_H
# error "Never use <fred_wrappers_raw.h> directly;" \
        "include <fred_wrappers.h> instead."
#endif

#ifdef __cplusplus
extern "C"
{
#endif

  typedef enum {
    empty_event,
    accept_event,
    accept4_event,
    access_event,
    bind_event,
    calloc_event,
    chmod_event,
    chown_event,
    close_event,
    connect_event,
    dup_event,
    dup2_event,
    dup3_event,
    fcntl_event,
    fchdir_event,
    fdatasync_event,
    getcwd_event,
    gettimeofday_event,
    getpeername_event,
    getsockname_event,
    link_event,
    symlink_event,
    listen_event,
    localtime_r_event,
    utime_event,
    utimes_event,
    lutimes_event,
    futimes_event,
    clock_getres_event,
    clock_gettime_event,
    clock_settime_event,
    lseek_event,
    lseek64_event,
    llseek_event,
    malloc_event,
    free_event,
    mkdir_event,
    mkstemp_event,
    mmap_event,
    mmap64_event,
    munmap_event,
    mremap_event,
    open_event,
    open64_event,
    openat_event,
    pread_event,
    preadv_event,
    pwrite_event,
    pwritev_event,
    pthread_rwlock_unlock_event,
    pthread_rwlock_rdlock_event,
    pthread_rwlock_wrlock_event,
    pthread_create_event,
    pthread_detach_event,
    pthread_exit_event,
    pthread_join_event,
    pthread_kill_event,
    pthread_mutex_lock_event,
    pthread_mutex_trylock_event,
    pthread_mutex_unlock_event,
    rand_event,
    read_event,
    readv_event,
    readlink_event,
    realpath_event,
    realloc_event,
    rename_event,
    rmdir_event,
    select_event,
    ppoll_event,
    setsockopt_event,
    getsockopt_event,
    ioctl_event,
    sigwait_event,
    srand_event,
    socket_event,
    socketpair_event,
    time_event,
    truncate_event,
    ftruncate_event,
    truncate64_event,
    ftruncate64_event,
    unlink_event,
    write_event,
    writev_event,
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
    recvmsg_event,
    waitid_event,
    wait4_event,
    sigaction_event,
    signal_event,
    sigset_event,
    fopen_event,
    fopen64_event,
    freopen_event,
    fclose_event,
    fdopen_event,
    fgets_event,
    ferror_event,
    feof_event,
    fileno_event,
    fflush_event,
    setvbuf_event,
    fseek_event,
    fputs_event,
    fputc_event,
    fsync_event,
    ftell_event,
    fgetpos_event,
    fgetpos64_event,
    fsetpos_event,
    fsetpos64_event,
    fwrite_event,
    fread_event,
    getc_event,
    fgetc_event,
    ungetc_event,
    getline_event,
    getdelim_event,
    putc_event,
    rewind_event,
    tmpfile_event,
    flockfile_event,
    ftrylockfile_event,
    funlockfile_event,
    closedir_event,
    opendir_event,
    fdopendir_event,
    readdir_event,
    readdir_r_event,
    pthread_cond_broadcast_event,
    pthread_cond_signal_event,
    pthread_cond_wait_event,
    pthread_cond_timedwait_event,
    pthread_cond_destroy_event,
    fxstat_event,
    fxstat64_event,
    lxstat_event,
    lxstat64_event,
    xstat_event,
    xstat64_event,
    libc_memalign_event,
    vfprintf_event,
    vfscanf_event,
    exec_barrier_event,
    signal_handler_event,
    user_event,
    syscall_event,
    numTotalWrappers
  } event_code_t;

  void _real_empty();;
  int _real_accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen);;
  int _real_accept4(int sockfd, struct sockaddr* addr, socklen_t* addrlen, int flags);;
  int _real_access(const char* pathname, int mode);;
  int _real_bind(int sockfd, const struct sockaddr* my_addr, socklen_t addrlen);;
  void* _real_calloc(size_t nmemb, size_t size);;
  int _real_chmod(const char* path, mode_t mode);;
  int _real_chown(const char* path, uid_t owner, gid_t group);;
  int _real_close(int fd);;
  int _real_connect(int sockfd, const struct sockaddr* serv_addr, socklen_t addrlen);;
  int _real_dup(int oldfd);;
  int _real_dup2(int oldfd, int newfd);;
  int _real_dup3(int oldfd, int newfd, int flags);;
  int _real_fcntl(int fd, int cmd, void* arg);;
  int _real_fchdir(int fd);;
  int _real_fdatasync(int fd);;
  char* _real_getcwd(char* buf, size_t size);;
  int _real_gettimeofday(struct timeval* tv, struct timezone* tz);;
  int _real_getpeername(int sockfd, struct sockaddr* addr, socklen_t* addrlen);;
  int _real_getsockname(int sockfd, struct sockaddr* addr, socklen_t* addrlen);;
  int _real_link(const char* oldpath, const char* newpath);;
  int _real_symlink(const char* oldpath, const char* newpath);;
  int _real_listen(int sockfd, int backlog);;
  struct tm* _real_localtime_r(const time_t* timep, struct tm* result);;
  int _real_utime(const char* filename, const struct utimbuf* times);;
  int _real_utimes(const char* filename, const struct timeval* times);;
  int _real_lutimes(const char* filename, const struct timeval* tv);;
  int _real_futimes(int fd, const struct timeval* tv);;
  int _real_clock_getres(clockid_t clk_id, struct timespec* res);;
  int _real_clock_gettime(clockid_t clk_id, struct timespec* tp);;
  int _real_clock_settime(clockid_t clk_id, const struct timespec* tp);;
  off_t _real_lseek(int fd, off_t offset, int whence);;
  off64_t _real_lseek64(int fd, off64_t offset, int whence);;
  loff_t _real_llseek(int fd, loff_t offset, int whence);;
  void* _real_malloc(size_t size);;
  void _real_free(void* ptr);;
  int _real_mkdir(const char* pathname, mode_t mode);;
  int _real_mkstemp(char* temp);;
  void* _real_mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset);;
  void* _real_mmap64(void* addr, size_t length, int prot, int flags, int fd, off64_t offset);;
  int _real_munmap(void* addr, size_t length);;
  void* _real_mremap(void* old_address, size_t old_size, size_t new_size, int flags, void* new_address);;
  int _real_open(const char* pathname, int flags, mode_t mode);;
  int _real_open64(const char* pathname, int flags, mode_t mode);;
  int _real_openat(int dirfd, const char* pathname, int flags);;
  ssize_t _real_pread(int fd, void* buf, size_t count, off_t offset);;
  ssize_t _real_preadv(int fd, const struct iovec* iov, int iovcnt, off_t offset);;
  ssize_t _real_pwrite(int fd, const void* buf, size_t count, off_t offset);;
  ssize_t _real_pwritev(int fd, const struct iovec* iov, int iovcnt, off_t offset);;
  int _real_pthread_rwlock_unlock(pthread_rwlock_t* rwlock);;
  int _real_pthread_rwlock_rdlock(pthread_rwlock_t* rwlock);;
  int _real_pthread_rwlock_wrlock(pthread_rwlock_t* rwlock);;
  int _real_pthread_create(pthread_t* thread, const pthread_attr_t* attr, pthread_start_routine_t start_routine, void* arg);;
  int _real_pthread_detach(pthread_t thread);;
  void _real_pthread_exit(void* value_ptr);;
  int _real_pthread_join(pthread_t thread, void** value_ptr);;
  int _real_pthread_kill(pthread_t thread, int sig);;
  int _real_pthread_mutex_lock(pthread_mutex_t* mutex);;
  int _real_pthread_mutex_trylock(pthread_mutex_t* mutex);;
  int _real_pthread_mutex_unlock(pthread_mutex_t* mutex);;
  int _real_rand();;
  ssize_t _real_read(int fd, void* buf, size_t count);;
  ssize_t _real_readv(int fd, const struct iovec* iov, int iovcnt);;
  ssize_t _real_readlink(const char* path, char* buf, size_t bufsiz);;
  char* _real_realpath(const char* path, char* resolved_path);;
  void* _real_realloc(void* ptr, size_t size);;
  int _real_rename(const char* oldpath, const char* newpath);;
  int _real_rmdir(const char* pathname);;
  int _real_select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout);;
  int _real_ppoll(struct pollfd* fds, nfds_t nfds, const struct timespec* timeout_ts, const sigset_t* sigmask);;
  int _real_setsockopt(int s, int level, int optname, const void* optval, socklen_t optlen);;
  int _real_getsockopt(int s, int level, int optname, void* optval, socklen_t* optlen);;
  int _real_ioctl(int d, int request, void* arg);;
  int _real_sigwait(const sigset_t* set, int* sig);;
  void _real_srand(unsigned int seed);;
  int _real_socket(int domain, int type, int protocol);;
  int _real_socketpair(int d, int type, int protocol, int* sv);;
  time_t _real_time(time_t* tloc);;
  int _real_truncate(const char* path, off_t length);;
  int _real_ftruncate(int fd, off_t length);;
  int _real_truncate64(const char* path, off64_t length);;
  int _real_ftruncate64(int fd, off64_t length);;
  int _real_unlink(const char* pathname);;
  ssize_t _real_write(int fd, const void* buf, size_t count);;
  ssize_t _real_writev(int fd, const struct iovec* iov, int iovcnt);;
  int _real_epoll_create(int size);;
  int _real_epoll_create1(int flags);;
  int _real_epoll_ctl(int epfd, int op, int fd, struct epoll_event* ep);;
  int _real_epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout);;
  int _real_getpwnam_r(const char* name, struct passwd* pwd, char* buf, size_t buflen, struct passwd** result);;
  int _real_getpwuid_r(uid_t uid, struct passwd* pwd, char* buf, size_t buflen, struct passwd** result);;
  int _real_getgrnam_r(const char* name, struct group* grp, char* buf, size_t buflen, struct group** result);;
  int _real_getgrgid_r(gid_t gid, struct group* grp, char* buf, size_t buflen, struct group** result);;
  int _real_getaddrinfo(const char* node, const char* service, const struct addrinfo* hints, struct addrinfo** res);;
  void _real_freeaddrinfo(struct addrinfo* res);;
  int _real_getnameinfo(const struct sockaddr* sa, socklen_t salen, char* host, size_t hostlen, char* serv, size_t servlen, int flags);;
  ssize_t _real_sendto(int sockfd, const void* buf, size_t len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen);;
  ssize_t _real_sendmsg(int sockfd, const struct msghdr* msg, int flags);;
  ssize_t _real_recvfrom(int sockfd, void* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen);;
  ssize_t _real_recvmsg(int sockfd, struct msghdr* msg, int flags);;
  int _real_waitid(idtype_t idtype, id_t id, siginfo_t* infop, int options);;
  pid_t _real_wait4(pid_t pid, __WAIT_STATUS status, int options, struct rusage* rusage);;
  int _real_sigaction(int signum, const struct sigaction* act, struct sigaction* oldact);;
  sighandler_t _real_signal(int signum, sighandler_t handler);;
  sighandler_t _real_sigset(int sig, sighandler_t disp);;
  FILE* _real_fopen(const char* path, const char* mode);;
  FILE* _real_fopen64(const char* path, const char* mode);;
  FILE* _real_freopen(const char* path, const char* mode, FILE* stream);;
  int _real_fclose(FILE* fp);;
  FILE* _real_fdopen(int fd, const char* mode);;
  char* _real_fgets(char* s, int size, FILE* stream);;
  int _real_ferror(FILE* stream);;
  int _real_feof(FILE* stream);;
  int _real_fileno(FILE* stream);;
  int _real_fflush(FILE* stream);;
  int _real_setvbuf(FILE* stream, char* buf, int mode, size_t size);;
  int _real_fseek(FILE* stream, long offset, int whence);;
  int _real_fputs(const char* s, FILE* stream);;
  int _real_fputc(int c, FILE* stream);;
  int _real_fsync(int fd);;
  long _real_ftell(FILE* stream);;
  int _real_fgetpos(FILE* stream, fpos_t* pos);;
  int _real_fgetpos64(FILE* stream, fpos64_t* pos);;
  int _real_fsetpos(FILE* stream, const fpos_t* pos);;
  int _real_fsetpos64(FILE* stream, const fpos64_t* pos);;
  size_t _real_fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream);;
  size_t _real_fread(void* ptr, size_t size, size_t nmemb, FILE* stream);;
  int _real_getc(FILE* stream);;
  int _real_fgetc(FILE* stream);;
  int _real_ungetc(int c, FILE* stream);;
  ssize_t _real_getline(char** lineptr, size_t* n, FILE* stream);;
  ssize_t _real_getdelim(char** lineptr, size_t* n, int delim, FILE* stream);;
  int _real_putc(int c, FILE* stream);;
  void _real_rewind(FILE* stream);;
  FILE* _real_tmpfile();;
  void _real_flockfile(FILE* filehandle);;
  int _real_ftrylockfile(FILE* filehandle);;
  void _real_funlockfile(FILE* filehandle);;
  int _real_closedir(DIR* dirp);;
  DIR* _real_opendir(const char* name);;
  DIR* _real_fdopendir(int fd);;
  struct dirent* _real_readdir(DIR* dirp);;
  int _real_readdir_r(DIR* dirp, struct dirent* entry, struct dirent** result);;
  int _real_pthread_cond_broadcast(pthread_cond_t* cond);;
  int _real_pthread_cond_signal(pthread_cond_t* cond);;
  int _real_pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex);;
  int _real_pthread_cond_timedwait(pthread_cond_t* cond, pthread_mutex_t* mutex, const struct timespec* abstime);;
  int _real_pthread_cond_destroy(pthread_cond_t* cond);;
  int _real_fxstat(int vers, int fd, struct stat* buf);;
  int _real_fxstat64(int vers, int fd, struct stat64* buf);;
  int _real_lxstat(int vers, const char* path, struct stat* buf);;
  int _real_lxstat64(int vers, const char* path, struct stat64* buf);;
  int _real_xstat(int vers, const char* path, struct stat* buf);;
  int _real_xstat64(int vers, const char* path, struct stat64* buf);;
  void* _real_libc_memalign(size_t boundary, size_t size);;
  int _real_vfprintf(FILE* stream, const char* format, va_list ap);;
  int _real_vfscanf(FILE* stream, const char* format, va_list ap);;
  void _real_exec_barrier();;
  void _real_signal_handler(int sig, siginfo_t* info, void* data);;
  void _real_user();;
  long int _real_syscall(long int sys_num, ...);

#ifdef __cplusplus
}
#endif