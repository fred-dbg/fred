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

#include "wrapper_util.h"
#include "synchronizationlogging.h"

static size_t log_event_size[numTotalWrappers] = {
  sizeof(log_event_empty_t),
  sizeof(log_event_accept_t),
  sizeof(log_event_accept4_t),
  sizeof(log_event_access_t),
  sizeof(log_event_bind_t),
  sizeof(log_event_calloc_t),
  sizeof(log_event_chmod_t),
  sizeof(log_event_chown_t),
  sizeof(log_event_close_t),
  sizeof(log_event_connect_t),
  sizeof(log_event_dup_t),
  sizeof(log_event_dup2_t),
  sizeof(log_event_dup3_t),
  sizeof(log_event_fcntl_t),
  sizeof(log_event_fchdir_t),
  sizeof(log_event_fdatasync_t),
  sizeof(log_event_getcwd_t),
  sizeof(log_event_gettimeofday_t),
  sizeof(log_event_getpeername_t),
  sizeof(log_event_getsockname_t),
  sizeof(log_event_link_t),
  sizeof(log_event_symlink_t),
  sizeof(log_event_listen_t),
  sizeof(log_event_localtime_r_t),
  sizeof(log_event_utime_t),
  sizeof(log_event_utimes_t),
  sizeof(log_event_lutimes_t),
  sizeof(log_event_futimes_t),
  sizeof(log_event_clock_getres_t),
  sizeof(log_event_clock_gettime_t),
  sizeof(log_event_clock_settime_t),
  sizeof(log_event_lseek_t),
  sizeof(log_event_lseek64_t),
  sizeof(log_event_llseek_t),
  sizeof(log_event_malloc_t),
  sizeof(log_event_free_t),
  sizeof(log_event_mkdir_t),
  sizeof(log_event_mkstemp_t),
  sizeof(log_event_mmap_t),
  sizeof(log_event_mmap64_t),
  sizeof(log_event_munmap_t),
  sizeof(log_event_mremap_t),
  sizeof(log_event_open_t),
  sizeof(log_event_open64_t),
  sizeof(log_event_openat_t),
  sizeof(log_event_pread_t),
  sizeof(log_event_preadv_t),
  sizeof(log_event_pwrite_t),
  sizeof(log_event_pwritev_t),
  sizeof(log_event_pthread_rwlock_unlock_t),
  sizeof(log_event_pthread_rwlock_rdlock_t),
  sizeof(log_event_pthread_rwlock_wrlock_t),
  sizeof(log_event_pthread_create_t),
  sizeof(log_event_pthread_detach_t),
  sizeof(log_event_pthread_exit_t),
  sizeof(log_event_pthread_join_t),
  sizeof(log_event_pthread_kill_t),
  sizeof(log_event_pthread_mutex_lock_t),
  sizeof(log_event_pthread_mutex_trylock_t),
  sizeof(log_event_pthread_mutex_unlock_t),
  sizeof(log_event_rand_t),
  sizeof(log_event_read_t),
  sizeof(log_event_readv_t),
  sizeof(log_event_readlink_t),
  sizeof(log_event_realpath_t),
  sizeof(log_event_realloc_t),
  sizeof(log_event_rename_t),
  sizeof(log_event_rmdir_t),
  sizeof(log_event_select_t),
  sizeof(log_event_ppoll_t),
  sizeof(log_event_setsockopt_t),
  sizeof(log_event_getsockopt_t),
  sizeof(log_event_ioctl_t),
  sizeof(log_event_sigwait_t),
  sizeof(log_event_srand_t),
  sizeof(log_event_socket_t),
  sizeof(log_event_socketpair_t),
  sizeof(log_event_time_t),
  sizeof(log_event_truncate_t),
  sizeof(log_event_ftruncate_t),
  sizeof(log_event_truncate64_t),
  sizeof(log_event_ftruncate64_t),
  sizeof(log_event_unlink_t),
  sizeof(log_event_write_t),
  sizeof(log_event_writev_t),
  sizeof(log_event_epoll_create_t),
  sizeof(log_event_epoll_create1_t),
  sizeof(log_event_epoll_ctl_t),
  sizeof(log_event_epoll_wait_t),
  sizeof(log_event_getpwnam_r_t),
  sizeof(log_event_getpwuid_r_t),
  sizeof(log_event_getgrnam_r_t),
  sizeof(log_event_getgrgid_r_t),
  sizeof(log_event_getaddrinfo_t),
  sizeof(log_event_freeaddrinfo_t),
  sizeof(log_event_getnameinfo_t),
  sizeof(log_event_sendto_t),
  sizeof(log_event_sendmsg_t),
  sizeof(log_event_recvfrom_t),
  sizeof(log_event_recvmsg_t),
  sizeof(log_event_waitid_t),
  sizeof(log_event_wait4_t),
  sizeof(log_event_sigaction_t),
  sizeof(log_event_signal_t),
  sizeof(log_event_sigset_t),
  sizeof(log_event_fopen_t),
  sizeof(log_event_fopen64_t),
  sizeof(log_event_freopen_t),
  sizeof(log_event_fclose_t),
  sizeof(log_event_fdopen_t),
  sizeof(log_event_fgets_t),
  sizeof(log_event_ferror_t),
  sizeof(log_event_feof_t),
  sizeof(log_event_fileno_t),
  sizeof(log_event_fflush_t),
  sizeof(log_event_setvbuf_t),
  sizeof(log_event_fseek_t),
  sizeof(log_event_fputs_t),
  sizeof(log_event_fputc_t),
  sizeof(log_event_fsync_t),
  sizeof(log_event_ftell_t),
  sizeof(log_event_fgetpos_t),
  sizeof(log_event_fgetpos64_t),
  sizeof(log_event_fsetpos_t),
  sizeof(log_event_fsetpos64_t),
  sizeof(log_event_fwrite_t),
  sizeof(log_event_fread_t),
  sizeof(log_event_getc_t),
  sizeof(log_event_fgetc_t),
  sizeof(log_event_ungetc_t),
  sizeof(log_event_getline_t),
  sizeof(log_event_getdelim_t),
  sizeof(log_event_putc_t),
  sizeof(log_event_rewind_t),
  sizeof(log_event_tmpfile_t),
  sizeof(log_event_flockfile_t),
  sizeof(log_event_ftrylockfile_t),
  sizeof(log_event_funlockfile_t),
  sizeof(log_event_closedir_t),
  sizeof(log_event_opendir_t),
  sizeof(log_event_fdopendir_t),
  sizeof(log_event_readdir_t),
  sizeof(log_event_readdir_r_t),
  sizeof(log_event_pthread_cond_broadcast_t),
  sizeof(log_event_pthread_cond_signal_t),
  sizeof(log_event_pthread_cond_wait_t),
  sizeof(log_event_pthread_cond_timedwait_t),
  sizeof(log_event_pthread_cond_destroy_t),
  sizeof(log_event_fxstat_t),
  sizeof(log_event_fxstat64_t),
  sizeof(log_event_lxstat_t),
  sizeof(log_event_lxstat64_t),
  sizeof(log_event_xstat_t),
  sizeof(log_event_xstat64_t),
  sizeof(log_event_libc_memalign_t),
  sizeof(log_event_vfprintf_t),
  sizeof(log_event_vfscanf_t),
  sizeof(log_event_exec_barrier_t),
  sizeof(log_event_signal_handler_t),
  sizeof(log_event_user_t),
  sizeof(log_event_syscall_t),
};

size_t getLogEventSize(const log_entry_t *entry) {
  return log_event_size[entry->header.event];
}
static void setupCommonFields(log_entry_t *e, clone_id_t clone_id,
                              event_code_t event)
{
  // Zero out all fields:
  memset(&(e->header), 0, sizeof(e->header));
  SET_COMMON_PTR(e, clone_id);
  SET_COMMON_PTR(e, event);
  SET_COMMON_PTR2(e, log_offset, INVALID_LOG_OFFSET);
}

log_entry_t create_empty_entry(clone_id_t clone_id, event_code_t event)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  return e;
}

log_entry_t create_accept_entry(clone_id_t clone_id, event_code_t event,
                                int sockfd, struct sockaddr* addr, socklen_t* addrlen)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, accept, sockfd);
  SET_FIELD(e, accept, addr);
  SET_FIELD(e, accept, addrlen);
  return e;
}

log_entry_t create_accept4_entry(clone_id_t clone_id, event_code_t event,
                                 int sockfd, struct sockaddr* addr, socklen_t* addrlen, int flags)
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
                                const char* pathname, int mode)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, access, pathname);
  SET_FIELD(e, access, mode);
  return e;
}

log_entry_t create_bind_entry(clone_id_t clone_id, event_code_t event,
                              int sockfd, const struct sockaddr* my_addr, socklen_t addrlen)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, bind, sockfd);
  SET_FIELD(e, bind, my_addr);
  SET_FIELD(e, bind, addrlen);
  return e;
}

log_entry_t create_calloc_entry(clone_id_t clone_id, event_code_t event,
                                size_t nmemb, size_t size)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, calloc, nmemb);
  SET_FIELD(e, calloc, size);
  return e;
}

log_entry_t create_chmod_entry(clone_id_t clone_id, event_code_t event,
                               const char* path, mode_t mode)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, chmod, path);
  SET_FIELD(e, chmod, mode);
  return e;
}

log_entry_t create_chown_entry(clone_id_t clone_id, event_code_t event,
                               const char* path, uid_t owner, gid_t group)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, chown, path);
  SET_FIELD(e, chown, owner);
  SET_FIELD(e, chown, group);
  return e;
}

log_entry_t create_close_entry(clone_id_t clone_id, event_code_t event,
                               int fd)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, close, fd);
  return e;
}

log_entry_t create_connect_entry(clone_id_t clone_id, event_code_t event,
                                 int sockfd, const struct sockaddr* serv_addr, socklen_t addrlen)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, connect, sockfd);
  SET_FIELD(e, connect, serv_addr);
  SET_FIELD(e, connect, addrlen);
  return e;
}

log_entry_t create_dup_entry(clone_id_t clone_id, event_code_t event,
                             int oldfd)
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

log_entry_t create_fcntl_entry(clone_id_t clone_id, event_code_t event,
                               int fd, int cmd, void* arg)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fcntl, fd);
  SET_FIELD(e, fcntl, cmd);
  SET_FIELD(e, fcntl, arg);
  return e;
}

log_entry_t create_fchdir_entry(clone_id_t clone_id, event_code_t event,
                                int fd)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fchdir, fd);
  return e;
}

log_entry_t create_fdatasync_entry(clone_id_t clone_id, event_code_t event,
                                   int fd)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fdatasync, fd);
  return e;
}

log_entry_t create_getcwd_entry(clone_id_t clone_id, event_code_t event,
                                char* buf, size_t size)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, getcwd, buf);
  SET_FIELD(e, getcwd, size);
  return e;
}

log_entry_t create_gettimeofday_entry(clone_id_t clone_id, event_code_t event,
                                      struct timeval* tv, struct timezone* tz)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, gettimeofday, tv);
  SET_FIELD(e, gettimeofday, tz);
  return e;
}

log_entry_t create_getpeername_entry(clone_id_t clone_id, event_code_t event,
                                     int sockfd, struct sockaddr* addr, socklen_t* addrlen)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, getpeername, sockfd);
  SET_FIELD(e, getpeername, addr);
  SET_FIELD(e, getpeername, addrlen);
  return e;
}

log_entry_t create_getsockname_entry(clone_id_t clone_id, event_code_t event,
                                     int sockfd, struct sockaddr* addr, socklen_t* addrlen)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, getsockname, sockfd);
  SET_FIELD(e, getsockname, addr);
  SET_FIELD(e, getsockname, addrlen);
  return e;
}

log_entry_t create_link_entry(clone_id_t clone_id, event_code_t event,
                              const char* oldpath, const char* newpath)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, link, oldpath);
  SET_FIELD(e, link, newpath);
  return e;
}

log_entry_t create_symlink_entry(clone_id_t clone_id, event_code_t event,
                                 const char* oldpath, const char* newpath)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, symlink, oldpath);
  SET_FIELD(e, symlink, newpath);
  return e;
}

log_entry_t create_listen_entry(clone_id_t clone_id, event_code_t event,
                                int sockfd, int backlog)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, listen, sockfd);
  SET_FIELD(e, listen, backlog);
  return e;
}

log_entry_t create_localtime_r_entry(clone_id_t clone_id, event_code_t event,
                                     const time_t* timep, struct tm* result)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, localtime_r, timep);
  SET_FIELD(e, localtime_r, result);
  return e;
}

log_entry_t create_utime_entry(clone_id_t clone_id, event_code_t event,
                               const char* filename, const struct utimbuf* times)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, utime, filename);
  SET_FIELD(e, utime, times);
  return e;
}

log_entry_t create_utimes_entry(clone_id_t clone_id, event_code_t event,
                                const char* filename, const struct timeval* times)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, utimes, filename);
  SET_FIELD(e, utimes, times);
  return e;
}

log_entry_t create_lutimes_entry(clone_id_t clone_id, event_code_t event,
                                 const char* filename, const struct timeval* tv)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, lutimes, filename);
  SET_FIELD(e, lutimes, tv);
  return e;
}

log_entry_t create_futimes_entry(clone_id_t clone_id, event_code_t event,
                                 int fd, const struct timeval* tv)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, futimes, fd);
  SET_FIELD(e, futimes, tv);
  return e;
}

log_entry_t create_clock_getres_entry(clone_id_t clone_id, event_code_t event,
                                      clockid_t clk_id, struct timespec* res)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, clock_getres, clk_id);
  SET_FIELD(e, clock_getres, res);
  return e;
}

log_entry_t create_clock_gettime_entry(clone_id_t clone_id, event_code_t event,
                                       clockid_t clk_id, struct timespec* tp)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, clock_gettime, clk_id);
  SET_FIELD(e, clock_gettime, tp);
  return e;
}

log_entry_t create_clock_settime_entry(clone_id_t clone_id, event_code_t event,
                                       clockid_t clk_id, const struct timespec* tp)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, clock_settime, clk_id);
  SET_FIELD(e, clock_settime, tp);
  return e;
}

log_entry_t create_lseek_entry(clone_id_t clone_id, event_code_t event,
                               int fd, off_t offset, int whence)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, lseek, fd);
  SET_FIELD(e, lseek, offset);
  SET_FIELD(e, lseek, whence);
  return e;
}

log_entry_t create_lseek64_entry(clone_id_t clone_id, event_code_t event,
                                 int fd, off64_t offset, int whence)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, lseek64, fd);
  SET_FIELD(e, lseek64, offset);
  SET_FIELD(e, lseek64, whence);
  return e;
}

log_entry_t create_llseek_entry(clone_id_t clone_id, event_code_t event,
                                int fd, loff_t offset, int whence)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, llseek, fd);
  SET_FIELD(e, llseek, offset);
  SET_FIELD(e, llseek, whence);
  return e;
}

log_entry_t create_malloc_entry(clone_id_t clone_id, event_code_t event,
                                size_t size)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, malloc, size);
  return e;
}

log_entry_t create_free_entry(clone_id_t clone_id, event_code_t event,
                              void* ptr)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, free, ptr);
  return e;
}

log_entry_t create_mkdir_entry(clone_id_t clone_id, event_code_t event,
                               const char* pathname, mode_t mode)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, mkdir, pathname);
  SET_FIELD(e, mkdir, mode);
  return e;
}

log_entry_t create_mkstemp_entry(clone_id_t clone_id, event_code_t event,
                                 char* temp)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, mkstemp, temp);
  return e;
}

log_entry_t create_mmap_entry(clone_id_t clone_id, event_code_t event,
                              void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, mmap, addr);
  SET_FIELD(e, mmap, length);
  SET_FIELD(e, mmap, prot);
  SET_FIELD(e, mmap, flags);
  SET_FIELD(e, mmap, fd);
  SET_FIELD(e, mmap, offset);
  return e;
}

log_entry_t create_mmap64_entry(clone_id_t clone_id, event_code_t event,
                                void* addr, size_t length, int prot, int flags, int fd, off64_t offset)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, mmap64, addr);
  SET_FIELD(e, mmap64, length);
  SET_FIELD(e, mmap64, prot);
  SET_FIELD(e, mmap64, flags);
  SET_FIELD(e, mmap64, fd);
  SET_FIELD(e, mmap64, offset);
  return e;
}

log_entry_t create_munmap_entry(clone_id_t clone_id, event_code_t event,
                                void* addr, size_t length)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, munmap, addr);
  SET_FIELD(e, munmap, length);
  return e;
}

log_entry_t create_mremap_entry(clone_id_t clone_id, event_code_t event,
                                void* old_address, size_t old_size, size_t new_size, int flags, void* new_address)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, mremap, old_address);
  SET_FIELD(e, mremap, old_size);
  SET_FIELD(e, mremap, new_size);
  SET_FIELD(e, mremap, flags);
  SET_FIELD(e, mremap, new_address);
  return e;
}

log_entry_t create_open_entry(clone_id_t clone_id, event_code_t event,
                              const char* pathname, int flags, mode_t mode)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, open, pathname);
  SET_FIELD(e, open, flags);
  SET_FIELD(e, open, mode);
  return e;
}

log_entry_t create_open64_entry(clone_id_t clone_id, event_code_t event,
                                const char* pathname, int flags, mode_t mode)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, open64, pathname);
  SET_FIELD(e, open64, flags);
  SET_FIELD(e, open64, mode);
  return e;
}

log_entry_t create_openat_entry(clone_id_t clone_id, event_code_t event,
                                int dirfd, const char* pathname, int flags)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, openat, dirfd);
  SET_FIELD(e, openat, pathname);
  SET_FIELD(e, openat, flags);
  return e;
}

log_entry_t create_pread_entry(clone_id_t clone_id, event_code_t event,
                               int fd, void* buf, size_t count, off_t offset)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, pread, fd);
  SET_FIELD(e, pread, buf);
  SET_FIELD(e, pread, count);
  SET_FIELD(e, pread, offset);
  return e;
}

log_entry_t create_preadv_entry(clone_id_t clone_id, event_code_t event,
                                int fd, const struct iovec* iov, int iovcnt, off_t offset)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, preadv, fd);
  SET_FIELD(e, preadv, iov);
  SET_FIELD(e, preadv, iovcnt);
  SET_FIELD(e, preadv, offset);
  return e;
}

log_entry_t create_pwrite_entry(clone_id_t clone_id, event_code_t event,
                                int fd, const void* buf, size_t count, off_t offset)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, pwrite, fd);
  SET_FIELD(e, pwrite, buf);
  SET_FIELD(e, pwrite, count);
  SET_FIELD(e, pwrite, offset);
  return e;
}

log_entry_t create_pwritev_entry(clone_id_t clone_id, event_code_t event,
                                 int fd, const struct iovec* iov, int iovcnt, off_t offset)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, pwritev, fd);
  SET_FIELD(e, pwritev, iov);
  SET_FIELD(e, pwritev, iovcnt);
  SET_FIELD(e, pwritev, offset);
  return e;
}

log_entry_t create_pthread_rwlock_unlock_entry(clone_id_t clone_id, event_code_t event,
                                               pthread_rwlock_t* rwlock)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, pthread_rwlock_unlock, rwlock);
  return e;
}

log_entry_t create_pthread_rwlock_rdlock_entry(clone_id_t clone_id, event_code_t event,
                                               pthread_rwlock_t* rwlock)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, pthread_rwlock_rdlock, rwlock);
  return e;
}

log_entry_t create_pthread_rwlock_wrlock_entry(clone_id_t clone_id, event_code_t event,
                                               pthread_rwlock_t* rwlock)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, pthread_rwlock_wrlock, rwlock);
  return e;
}

log_entry_t create_pthread_create_entry(clone_id_t clone_id, event_code_t event,
                                        pthread_t* thread, const pthread_attr_t* attr, pthread_start_routine_t start_routine, void* arg)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, pthread_create, thread);
  SET_FIELD(e, pthread_create, attr);
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
                                      void* value_ptr)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, pthread_exit, value_ptr);
  return e;
}

log_entry_t create_pthread_join_entry(clone_id_t clone_id, event_code_t event,
                                      pthread_t thread, void** value_ptr)
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

log_entry_t create_pthread_mutex_lock_entry(clone_id_t clone_id, event_code_t event,
                                            pthread_mutex_t* mutex)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, pthread_mutex_lock, mutex);
  return e;
}

log_entry_t create_pthread_mutex_trylock_entry(clone_id_t clone_id, event_code_t event,
                                               pthread_mutex_t* mutex)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, pthread_mutex_trylock, mutex);
  return e;
}

log_entry_t create_pthread_mutex_unlock_entry(clone_id_t clone_id, event_code_t event,
                                              pthread_mutex_t* mutex)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, pthread_mutex_unlock, mutex);
  return e;
}

log_entry_t create_rand_entry(clone_id_t clone_id, event_code_t event)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  return e;
}

log_entry_t create_read_entry(clone_id_t clone_id, event_code_t event,
                              int fd, void* buf, size_t count)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, read, fd);
  SET_FIELD(e, read, buf);
  SET_FIELD(e, read, count);
  return e;
}

log_entry_t create_readv_entry(clone_id_t clone_id, event_code_t event,
                               int fd, const struct iovec* iov, int iovcnt)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, readv, fd);
  SET_FIELD(e, readv, iov);
  SET_FIELD(e, readv, iovcnt);
  return e;
}

log_entry_t create_readlink_entry(clone_id_t clone_id, event_code_t event,
                                  const char* path, char* buf, size_t bufsiz)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, readlink, path);
  SET_FIELD(e, readlink, buf);
  SET_FIELD(e, readlink, bufsiz);
  return e;
}

log_entry_t create_realpath_entry(clone_id_t clone_id, event_code_t event,
                                  const char* path, char* resolved_path)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, realpath, path);
  SET_FIELD(e, realpath, resolved_path);
  return e;
}

log_entry_t create_realloc_entry(clone_id_t clone_id, event_code_t event,
                                 void* ptr, size_t size)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, realloc, ptr);
  SET_FIELD(e, realloc, size);
  return e;
}

log_entry_t create_rename_entry(clone_id_t clone_id, event_code_t event,
                                const char* oldpath, const char* newpath)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, rename, oldpath);
  SET_FIELD(e, rename, newpath);
  return e;
}

log_entry_t create_rmdir_entry(clone_id_t clone_id, event_code_t event,
                               const char* pathname)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, rmdir, pathname);
  return e;
}

log_entry_t create_select_entry(clone_id_t clone_id, event_code_t event,
                                int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, select, nfds);
  SET_FIELD(e, select, readfds);
  SET_FIELD(e, select, writefds);
  SET_FIELD(e, select, exceptfds);
  SET_FIELD(e, select, timeout);
  return e;
}

log_entry_t create_ppoll_entry(clone_id_t clone_id, event_code_t event,
                               struct pollfd* fds, nfds_t nfds, const struct timespec* timeout_ts, const sigset_t* sigmask)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, ppoll, fds);
  SET_FIELD(e, ppoll, nfds);
  SET_FIELD(e, ppoll, timeout_ts);
  SET_FIELD(e, ppoll, sigmask);
  return e;
}

log_entry_t create_setsockopt_entry(clone_id_t clone_id, event_code_t event,
                                    int s, int level, int optname, const void* optval, socklen_t optlen)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, setsockopt, s);
  SET_FIELD(e, setsockopt, level);
  SET_FIELD(e, setsockopt, optname);
  SET_FIELD(e, setsockopt, optval);
  SET_FIELD(e, setsockopt, optlen);
  return e;
}

log_entry_t create_getsockopt_entry(clone_id_t clone_id, event_code_t event,
                                    int s, int level, int optname, void* optval, socklen_t* optlen)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, getsockopt, s);
  SET_FIELD(e, getsockopt, level);
  SET_FIELD(e, getsockopt, optname);
  SET_FIELD(e, getsockopt, optval);
  SET_FIELD(e, getsockopt, optlen);
  return e;
}

log_entry_t create_ioctl_entry(clone_id_t clone_id, event_code_t event,
                               int d, int request, void* arg)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, ioctl, d);
  SET_FIELD(e, ioctl, request);
  SET_FIELD(e, ioctl, arg);
  return e;
}

log_entry_t create_sigwait_entry(clone_id_t clone_id, event_code_t event,
                                 const sigset_t* set, int* sig)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, sigwait, set);
  SET_FIELD(e, sigwait, sig);
  return e;
}

log_entry_t create_srand_entry(clone_id_t clone_id, event_code_t event,
                               unsigned int seed)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, srand, seed);
  return e;
}

log_entry_t create_socket_entry(clone_id_t clone_id, event_code_t event,
                                int domain, int type, int protocol)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, socket, domain);
  SET_FIELD(e, socket, type);
  SET_FIELD(e, socket, protocol);
  return e;
}

log_entry_t create_socketpair_entry(clone_id_t clone_id, event_code_t event,
                                    int d, int type, int protocol, int* sv)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, socketpair, d);
  SET_FIELD(e, socketpair, type);
  SET_FIELD(e, socketpair, protocol);
  SET_FIELD(e, socketpair, sv);
  return e;
}

log_entry_t create_time_entry(clone_id_t clone_id, event_code_t event,
                              time_t* tloc)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, time, tloc);
  return e;
}

log_entry_t create_truncate_entry(clone_id_t clone_id, event_code_t event,
                                  const char* path, off_t length)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, truncate, path);
  SET_FIELD(e, truncate, length);
  return e;
}

log_entry_t create_ftruncate_entry(clone_id_t clone_id, event_code_t event,
                                   int fd, off_t length)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, ftruncate, fd);
  SET_FIELD(e, ftruncate, length);
  return e;
}

log_entry_t create_truncate64_entry(clone_id_t clone_id, event_code_t event,
                                    const char* path, off64_t length)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, truncate64, path);
  SET_FIELD(e, truncate64, length);
  return e;
}

log_entry_t create_ftruncate64_entry(clone_id_t clone_id, event_code_t event,
                                     int fd, off64_t length)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, ftruncate64, fd);
  SET_FIELD(e, ftruncate64, length);
  return e;
}

log_entry_t create_unlink_entry(clone_id_t clone_id, event_code_t event,
                                const char* pathname)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, unlink, pathname);
  return e;
}

log_entry_t create_write_entry(clone_id_t clone_id, event_code_t event,
                               int fd, const void* buf, size_t count)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, write, fd);
  SET_FIELD(e, write, buf);
  SET_FIELD(e, write, count);
  return e;
}

log_entry_t create_writev_entry(clone_id_t clone_id, event_code_t event,
                                int fd, const struct iovec* iov, int iovcnt)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, writev, fd);
  SET_FIELD(e, writev, iov);
  SET_FIELD(e, writev, iovcnt);
  return e;
}

log_entry_t create_epoll_create_entry(clone_id_t clone_id, event_code_t event,
                                      int size)
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
                                   int epfd, int op, int fd, struct epoll_event* ep)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, epoll_ctl, epfd);
  SET_FIELD(e, epoll_ctl, op);
  SET_FIELD(e, epoll_ctl, fd);
  SET_FIELD(e, epoll_ctl, ep);
  return e;
}

log_entry_t create_epoll_wait_entry(clone_id_t clone_id, event_code_t event,
                                    int epfd, struct epoll_event* events, int maxevents, int timeout)
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
                                    const char* name, struct passwd* pwd, char* buf, size_t buflen, struct passwd** result)
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
                                    uid_t uid, struct passwd* pwd, char* buf, size_t buflen, struct passwd** result)
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
                                    const char* name, struct group* grp, char* buf, size_t buflen, struct group** result)
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
                                    gid_t gid, struct group* grp, char* buf, size_t buflen, struct group** result)
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
                                     const char* node, const char* service, const struct addrinfo* hints, struct addrinfo** res)
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
                                      struct addrinfo* res)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, freeaddrinfo, res);
  return e;
}

log_entry_t create_getnameinfo_entry(clone_id_t clone_id, event_code_t event,
                                     const struct sockaddr* sa, socklen_t salen, char* host, size_t hostlen, char* serv, size_t servlen, int flags)
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

log_entry_t create_sendto_entry(clone_id_t clone_id, event_code_t event,
                                int sockfd, const void* buf, size_t len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, sendto, sockfd);
  SET_FIELD(e, sendto, buf);
  SET_FIELD(e, sendto, len);
  SET_FIELD(e, sendto, flags);
  SET_FIELD(e, sendto, dest_addr);
  SET_FIELD(e, sendto, addrlen);
  return e;
}

log_entry_t create_sendmsg_entry(clone_id_t clone_id, event_code_t event,
                                 int sockfd, const struct msghdr* msg, int flags)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, sendmsg, sockfd);
  SET_FIELD(e, sendmsg, msg);
  SET_FIELD(e, sendmsg, flags);
  return e;
}

log_entry_t create_recvfrom_entry(clone_id_t clone_id, event_code_t event,
                                  int sockfd, void* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, recvfrom, sockfd);
  SET_FIELD(e, recvfrom, buf);
  SET_FIELD(e, recvfrom, len);
  SET_FIELD(e, recvfrom, flags);
  SET_FIELD(e, recvfrom, src_addr);
  SET_FIELD(e, recvfrom, addrlen);
  return e;
}

log_entry_t create_recvmsg_entry(clone_id_t clone_id, event_code_t event,
                                 int sockfd, struct msghdr* msg, int flags)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, recvmsg, sockfd);
  SET_FIELD(e, recvmsg, msg);
  SET_FIELD(e, recvmsg, flags);
  return e;
}

log_entry_t create_waitid_entry(clone_id_t clone_id, event_code_t event,
                                idtype_t idtype, id_t id, siginfo_t* infop, int options)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, waitid, idtype);
  SET_FIELD(e, waitid, id);
  SET_FIELD(e, waitid, infop);
  SET_FIELD(e, waitid, options);
  return e;
}

log_entry_t create_wait4_entry(clone_id_t clone_id, event_code_t event,
                               pid_t pid, __WAIT_STATUS status, int options, struct rusage* rusage)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, wait4, pid);
  SET_FIELD(e, wait4, status);
  SET_FIELD(e, wait4, options);
  SET_FIELD(e, wait4, rusage);
  return e;
}

log_entry_t create_sigaction_entry(clone_id_t clone_id, event_code_t event,
                                   int signum, const struct sigaction* act, struct sigaction* oldact)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, sigaction, signum);
  SET_FIELD(e, sigaction, act);
  SET_FIELD(e, sigaction, oldact);
  return e;
}

log_entry_t create_signal_entry(clone_id_t clone_id, event_code_t event,
                                int signum, sighandler_t handler)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, signal, signum);
  SET_FIELD(e, signal, handler);
  return e;
}

log_entry_t create_sigset_entry(clone_id_t clone_id, event_code_t event,
                                int sig, sighandler_t disp)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, sigset, sig);
  SET_FIELD(e, sigset, disp);
  return e;
}

log_entry_t create_fopen_entry(clone_id_t clone_id, event_code_t event,
                               const char* path, const char* mode)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fopen, path);
  SET_FIELD(e, fopen, mode);
  return e;
}

log_entry_t create_fopen64_entry(clone_id_t clone_id, event_code_t event,
                                 const char* path, const char* mode)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fopen64, path);
  SET_FIELD(e, fopen64, mode);
  return e;
}

log_entry_t create_freopen_entry(clone_id_t clone_id, event_code_t event,
                                 const char* path, const char* mode, FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, freopen, path);
  SET_FIELD(e, freopen, mode);
  SET_FIELD(e, freopen, stream);
  return e;
}

log_entry_t create_fclose_entry(clone_id_t clone_id, event_code_t event,
                                FILE* fp)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fclose, fp);
  return e;
}

log_entry_t create_fdopen_entry(clone_id_t clone_id, event_code_t event,
                                int fd, const char* mode)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fdopen, fd);
  SET_FIELD(e, fdopen, mode);
  return e;
}

log_entry_t create_fgets_entry(clone_id_t clone_id, event_code_t event,
                               char* s, int size, FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fgets, s);
  SET_FIELD(e, fgets, size);
  SET_FIELD(e, fgets, stream);
  return e;
}

log_entry_t create_ferror_entry(clone_id_t clone_id, event_code_t event,
                                FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, ferror, stream);
  return e;
}

log_entry_t create_feof_entry(clone_id_t clone_id, event_code_t event,
                              FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, feof, stream);
  return e;
}

log_entry_t create_fileno_entry(clone_id_t clone_id, event_code_t event,
                                FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fileno, stream);
  return e;
}

log_entry_t create_fflush_entry(clone_id_t clone_id, event_code_t event,
                                FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fflush, stream);
  return e;
}

log_entry_t create_setvbuf_entry(clone_id_t clone_id, event_code_t event,
                                 FILE* stream, char* buf, int mode, size_t size)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, setvbuf, stream);
  SET_FIELD(e, setvbuf, buf);
  SET_FIELD(e, setvbuf, mode);
  SET_FIELD(e, setvbuf, size);
  return e;
}

log_entry_t create_fseek_entry(clone_id_t clone_id, event_code_t event,
                               FILE* stream, long offset, int whence)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fseek, stream);
  SET_FIELD(e, fseek, offset);
  SET_FIELD(e, fseek, whence);
  return e;
}

log_entry_t create_fputs_entry(clone_id_t clone_id, event_code_t event,
                               const char* s, FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fputs, s);
  SET_FIELD(e, fputs, stream);
  return e;
}

log_entry_t create_fputc_entry(clone_id_t clone_id, event_code_t event,
                               int c, FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fputc, c);
  SET_FIELD(e, fputc, stream);
  return e;
}

log_entry_t create_fsync_entry(clone_id_t clone_id, event_code_t event,
                               int fd)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fsync, fd);
  return e;
}

log_entry_t create_ftell_entry(clone_id_t clone_id, event_code_t event,
                               FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, ftell, stream);
  return e;
}

log_entry_t create_fgetpos_entry(clone_id_t clone_id, event_code_t event,
                                 FILE* stream, fpos_t* pos)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fgetpos, stream);
  SET_FIELD(e, fgetpos, pos);
  return e;
}

log_entry_t create_fgetpos64_entry(clone_id_t clone_id, event_code_t event,
                                   FILE* stream, fpos64_t* pos)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fgetpos64, stream);
  SET_FIELD(e, fgetpos64, pos);
  return e;
}

log_entry_t create_fsetpos_entry(clone_id_t clone_id, event_code_t event,
                                 FILE* stream, const fpos_t* pos)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fsetpos, stream);
  SET_FIELD(e, fsetpos, pos);
  return e;
}

log_entry_t create_fsetpos64_entry(clone_id_t clone_id, event_code_t event,
                                   FILE* stream, const fpos64_t* pos)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fsetpos64, stream);
  SET_FIELD(e, fsetpos64, pos);
  return e;
}

log_entry_t create_fwrite_entry(clone_id_t clone_id, event_code_t event,
                                const void* ptr, size_t size, size_t nmemb, FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fwrite, ptr);
  SET_FIELD(e, fwrite, size);
  SET_FIELD(e, fwrite, nmemb);
  SET_FIELD(e, fwrite, stream);
  return e;
}

log_entry_t create_fread_entry(clone_id_t clone_id, event_code_t event,
                               void* ptr, size_t size, size_t nmemb, FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fread, ptr);
  SET_FIELD(e, fread, size);
  SET_FIELD(e, fread, nmemb);
  SET_FIELD(e, fread, stream);
  return e;
}

log_entry_t create_getc_entry(clone_id_t clone_id, event_code_t event,
                              FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, getc, stream);
  return e;
}

log_entry_t create_fgetc_entry(clone_id_t clone_id, event_code_t event,
                               FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fgetc, stream);
  return e;
}

log_entry_t create_ungetc_entry(clone_id_t clone_id, event_code_t event,
                                int c, FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, ungetc, c);
  SET_FIELD(e, ungetc, stream);
  return e;
}

log_entry_t create_getline_entry(clone_id_t clone_id, event_code_t event,
                                 char** lineptr, size_t* n, FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, getline, lineptr);
  SET_FIELD(e, getline, n);
  SET_FIELD(e, getline, stream);
  return e;
}

log_entry_t create_getdelim_entry(clone_id_t clone_id, event_code_t event,
                                  char** lineptr, size_t* n, int delim, FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, getdelim, lineptr);
  SET_FIELD(e, getdelim, n);
  SET_FIELD(e, getdelim, delim);
  SET_FIELD(e, getdelim, stream);
  return e;
}

log_entry_t create_putc_entry(clone_id_t clone_id, event_code_t event,
                              int c, FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, putc, c);
  SET_FIELD(e, putc, stream);
  return e;
}

log_entry_t create_rewind_entry(clone_id_t clone_id, event_code_t event,
                                FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, rewind, stream);
  return e;
}

log_entry_t create_tmpfile_entry(clone_id_t clone_id, event_code_t event)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  return e;
}

log_entry_t create_flockfile_entry(clone_id_t clone_id, event_code_t event,
                                   FILE* filehandle)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, flockfile, filehandle);
  return e;
}

log_entry_t create_ftrylockfile_entry(clone_id_t clone_id, event_code_t event,
                                      FILE* filehandle)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, ftrylockfile, filehandle);
  return e;
}

log_entry_t create_funlockfile_entry(clone_id_t clone_id, event_code_t event,
                                     FILE* filehandle)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, funlockfile, filehandle);
  return e;
}

log_entry_t create_closedir_entry(clone_id_t clone_id, event_code_t event,
                                  DIR* dirp)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, closedir, dirp);
  return e;
}

log_entry_t create_opendir_entry(clone_id_t clone_id, event_code_t event,
                                 const char* name)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, opendir, name);
  return e;
}

log_entry_t create_fdopendir_entry(clone_id_t clone_id, event_code_t event,
                                   int fd)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fdopendir, fd);
  return e;
}

log_entry_t create_readdir_entry(clone_id_t clone_id, event_code_t event,
                                 DIR* dirp)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, readdir, dirp);
  return e;
}

log_entry_t create_readdir_r_entry(clone_id_t clone_id, event_code_t event,
                                   DIR* dirp, struct dirent* entry, struct dirent** result)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, readdir_r, dirp);
  SET_FIELD(e, readdir_r, entry);
  SET_FIELD(e, readdir_r, result);
  return e;
}

log_entry_t create_pthread_cond_broadcast_entry(clone_id_t clone_id, event_code_t event,
                                                pthread_cond_t* cond)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, pthread_cond_broadcast, cond);
  return e;
}

log_entry_t create_pthread_cond_signal_entry(clone_id_t clone_id, event_code_t event,
                                             pthread_cond_t* cond)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, pthread_cond_signal, cond);
  return e;
}

log_entry_t create_pthread_cond_wait_entry(clone_id_t clone_id, event_code_t event,
                                           pthread_cond_t* cond, pthread_mutex_t* mutex)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, pthread_cond_wait, cond);
  SET_FIELD(e, pthread_cond_wait, mutex);
  return e;
}

log_entry_t create_pthread_cond_timedwait_entry(clone_id_t clone_id, event_code_t event,
                                                pthread_cond_t* cond, pthread_mutex_t* mutex, const struct timespec* abstime)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, pthread_cond_timedwait, cond);
  SET_FIELD(e, pthread_cond_timedwait, mutex);
  SET_FIELD(e, pthread_cond_timedwait, abstime);
  return e;
}

log_entry_t create_pthread_cond_destroy_entry(clone_id_t clone_id, event_code_t event,
                                              pthread_cond_t* cond)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, pthread_cond_destroy, cond);
  return e;
}

log_entry_t create_fxstat_entry(clone_id_t clone_id, event_code_t event,
                                int vers, int fd, struct stat* buf)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fxstat, vers);
  SET_FIELD(e, fxstat, fd);
  SET_FIELD(e, fxstat, buf);
  return e;
}

log_entry_t create_fxstat64_entry(clone_id_t clone_id, event_code_t event,
                                  int vers, int fd, struct stat64* buf)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, fxstat64, vers);
  SET_FIELD(e, fxstat64, fd);
  SET_FIELD(e, fxstat64, buf);
  return e;
}

log_entry_t create_lxstat_entry(clone_id_t clone_id, event_code_t event,
                                int vers, const char* path, struct stat* buf)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, lxstat, vers);
  SET_FIELD(e, lxstat, path);
  SET_FIELD(e, lxstat, buf);
  return e;
}

log_entry_t create_lxstat64_entry(clone_id_t clone_id, event_code_t event,
                                  int vers, const char* path, struct stat64* buf)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, lxstat64, vers);
  SET_FIELD(e, lxstat64, path);
  SET_FIELD(e, lxstat64, buf);
  return e;
}

log_entry_t create_xstat_entry(clone_id_t clone_id, event_code_t event,
                               int vers, const char* path, struct stat* buf)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, xstat, vers);
  SET_FIELD(e, xstat, path);
  SET_FIELD(e, xstat, buf);
  return e;
}

log_entry_t create_xstat64_entry(clone_id_t clone_id, event_code_t event,
                                 int vers, const char* path, struct stat64* buf)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, xstat64, vers);
  SET_FIELD(e, xstat64, path);
  SET_FIELD(e, xstat64, buf);
  return e;
}

log_entry_t create_libc_memalign_entry(clone_id_t clone_id, event_code_t event,
                                       size_t boundary, size_t size)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, libc_memalign, boundary);
  SET_FIELD(e, libc_memalign, size);
  return e;
}

log_entry_t create_vfprintf_entry(clone_id_t clone_id, event_code_t event,
                                  FILE* stream, const char* format, va_list ap)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, vfprintf, stream);
  SET_FIELD(e, vfprintf, format);
  return e;
}

log_entry_t create_vfscanf_entry(clone_id_t clone_id, event_code_t event,
                                 FILE* stream, const char* format, va_list ap)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, vfscanf, stream);
  SET_FIELD(e, vfscanf, format);
  return e;
}

log_entry_t create_exec_barrier_entry(clone_id_t clone_id, event_code_t event)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  return e;
}

log_entry_t create_signal_handler_entry(clone_id_t clone_id, event_code_t event,
                                        int sig, siginfo_t* info, void* data)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, signal_handler, sig);
  SET_FIELD(e, signal_handler, info);
  SET_FIELD(e, signal_handler, data);
  return e;
}

log_entry_t create_user_entry(clone_id_t clone_id, event_code_t event)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  return e;
}

log_entry_t create_syscall_entry(clone_id_t clone_id, event_code_t event,
                                 int num, void* a1, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, clone_id, event);
  SET_FIELD(e, syscall, num);
  SET_FIELD(e, syscall, a1);
  SET_FIELD(e, syscall, a2);
  SET_FIELD(e, syscall, a3);
  SET_FIELD(e, syscall, a4);
  SET_FIELD(e, syscall, a5);
  SET_FIELD(e, syscall, a6);
  SET_FIELD(e, syscall, a7);
  return e;
}
static int base_turn_check(log_entry_t *e1, log_entry_t *e2) {
  // Predicate function for a basic check -- event # and clone id.
  return GET_COMMON_PTR(e1,clone_id) == GET_COMMON_PTR(e2,clone_id) &&
         GET_COMMON_PTR(e1,event) == GET_COMMON_PTR(e2,event);
}

int empty_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int accept_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, accept, sockfd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, accept, addr)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, accept, addrlen);
}

int accept4_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, accept4, sockfd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, accept4, addr)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, accept4, addrlen)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, accept4, flags);
}

int access_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, access, pathname)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, access, mode);
}

int bind_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, bind, sockfd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, bind, my_addr)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, bind, addrlen);
}

int calloc_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, calloc, nmemb)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, calloc, size);
}

int chmod_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, chmod, path)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, chmod, mode);
}

int chown_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, chown, path)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, chown, owner)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, chown, group);
}

int close_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, close, fd);
}

int connect_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, connect, sockfd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, connect, serv_addr)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, connect, addrlen);
}

int dup_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, dup, oldfd);
}

int dup2_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, dup2, oldfd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, dup2, newfd);
}

int dup3_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, dup3, oldfd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, dup3, newfd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, dup3, flags);
}

int fcntl_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fcntl, fd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fcntl, cmd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fcntl, arg);
}

int fchdir_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fchdir, fd);
}

int fdatasync_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fdatasync, fd);
}

int getcwd_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getcwd, buf)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getcwd, size);
}

int gettimeofday_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, gettimeofday, tv)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, gettimeofday, tz);
}

int getpeername_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getpeername, sockfd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getpeername, addr)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getpeername, addrlen);
}

int getsockname_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getsockname, sockfd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getsockname, addr)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getsockname, addrlen);
}

int link_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, link, oldpath)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, link, newpath);
}

int symlink_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, symlink, oldpath)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, symlink, newpath);
}

int listen_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, listen, sockfd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, listen, backlog);
}

int localtime_r_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, localtime_r, timep)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, localtime_r, result);
}

int utime_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, utime, filename)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, utime, times);
}

int utimes_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, utimes, filename)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, utimes, times);
}

int lutimes_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, lutimes, filename)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, lutimes, tv);
}

int futimes_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, futimes, fd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, futimes, tv);
}

int clock_getres_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, clock_getres, clk_id)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, clock_getres, res);
}

int clock_gettime_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, clock_gettime, clk_id)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, clock_gettime, tp);
}

int clock_settime_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, clock_settime, clk_id)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, clock_settime, tp);
}

int lseek_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, lseek, fd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, lseek, offset)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, lseek, whence);
}

int lseek64_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, lseek64, fd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, lseek64, offset)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, lseek64, whence);
}

int llseek_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, llseek, fd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, llseek, offset)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, llseek, whence);
}

int malloc_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, malloc, size);
}

int free_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, free, ptr);
}

int mkdir_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, mkdir, pathname)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, mkdir, mode);
}

int mkstemp_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, mkstemp, temp);
}

int mmap_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, mmap, addr)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, mmap, length)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, mmap, prot)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, mmap, flags)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, mmap, fd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, mmap, offset);
}

int mmap64_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, mmap64, addr)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, mmap64, length)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, mmap64, prot)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, mmap64, flags)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, mmap64, fd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, mmap64, offset);
}

int munmap_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, munmap, addr)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, munmap, length);
}

int mremap_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, mremap, old_address)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, mremap, old_size)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, mremap, new_size)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, mremap, flags)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, mremap, new_address);
}

int open_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, open, pathname)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, open, flags)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, open, mode);
}

int open64_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, open64, pathname)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, open64, flags)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, open64, mode);
}

int openat_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, openat, dirfd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, openat, pathname)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, openat, flags);
}

int pread_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pread, fd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pread, buf)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pread, count)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pread, offset);
}

int preadv_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, preadv, fd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, preadv, iov)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, preadv, iovcnt)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, preadv, offset);
}

int pwrite_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pwrite, fd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pwrite, buf)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pwrite, count)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pwrite, offset);
}

int pwritev_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pwritev, fd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pwritev, iov)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pwritev, iovcnt)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pwritev, offset);
}

int pthread_rwlock_unlock_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pthread_rwlock_unlock, rwlock);
}

int pthread_rwlock_rdlock_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pthread_rwlock_rdlock, rwlock);
}

int pthread_rwlock_wrlock_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pthread_rwlock_wrlock, rwlock);
}

int pthread_create_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pthread_create, thread)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pthread_create, attr)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pthread_create, start_routine)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pthread_create, arg);
}

int pthread_detach_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pthread_detach, thread);
}

int pthread_exit_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pthread_exit, value_ptr);
}

int pthread_join_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pthread_join, thread)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pthread_join, value_ptr);
}

int pthread_kill_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pthread_kill, thread)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pthread_kill, sig);
}

int pthread_mutex_lock_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pthread_mutex_lock, mutex);
}

int pthread_mutex_trylock_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pthread_mutex_trylock, mutex);
}

int pthread_mutex_unlock_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pthread_mutex_unlock, mutex);
}

int rand_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int read_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, read, fd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, read, buf)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, read, count);
}

int readv_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, readv, fd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, readv, iov)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, readv, iovcnt);
}

int readlink_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, readlink, path)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, readlink, buf)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, readlink, bufsiz);
}

int realpath_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, realpath, path)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, realpath, resolved_path);
}

int realloc_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, realloc, ptr)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, realloc, size);
}

int rename_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, rename, oldpath)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, rename, newpath);
}

int rmdir_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, rmdir, pathname);
}

int select_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, select, nfds)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, select, readfds)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, select, writefds)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, select, exceptfds)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, select, timeout);
}

int ppoll_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, ppoll, fds)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, ppoll, nfds)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, ppoll, timeout_ts)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, ppoll, sigmask);
}

int setsockopt_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, setsockopt, s)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, setsockopt, level)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, setsockopt, optname)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, setsockopt, optval)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, setsockopt, optlen);
}

int getsockopt_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getsockopt, s)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getsockopt, level)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getsockopt, optname)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getsockopt, optval)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getsockopt, optlen);
}

int ioctl_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, ioctl, d)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, ioctl, request)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, ioctl, arg);
}

int sigwait_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, sigwait, set)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, sigwait, sig);
}

int srand_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, srand, seed);
}

int socket_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, socket, domain)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, socket, type)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, socket, protocol);
}

int socketpair_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, socketpair, d)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, socketpair, type)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, socketpair, protocol)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, socketpair, sv);
}

int time_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, time, tloc);
}

int truncate_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, truncate, path)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, truncate, length);
}

int ftruncate_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, ftruncate, fd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, ftruncate, length);
}

int truncate64_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, truncate64, path)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, truncate64, length);
}

int ftruncate64_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, ftruncate64, fd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, ftruncate64, length);
}

int unlink_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, unlink, pathname);
}

int write_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, write, fd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, write, buf)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, write, count);
}

int writev_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, writev, fd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, writev, iov)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, writev, iovcnt);
}

int epoll_create_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, epoll_create, size);
}

int epoll_create1_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, epoll_create1, flags);
}

int epoll_ctl_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, epoll_ctl, epfd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, epoll_ctl, op)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, epoll_ctl, fd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, epoll_ctl, ep);
}

int epoll_wait_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, epoll_wait, epfd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, epoll_wait, events)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, epoll_wait, maxevents)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, epoll_wait, timeout);
}

int getpwnam_r_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getpwnam_r, name)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getpwnam_r, pwd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getpwnam_r, buf)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getpwnam_r, buflen)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getpwnam_r, result);
}

int getpwuid_r_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getpwuid_r, uid)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getpwuid_r, pwd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getpwuid_r, buf)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getpwuid_r, buflen)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getpwuid_r, result);
}

int getgrnam_r_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getgrnam_r, name)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getgrnam_r, grp)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getgrnam_r, buf)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getgrnam_r, buflen)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getgrnam_r, result);
}

int getgrgid_r_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getgrgid_r, gid)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getgrgid_r, grp)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getgrgid_r, buf)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getgrgid_r, buflen)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getgrgid_r, result);
}

int getaddrinfo_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getaddrinfo, node)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getaddrinfo, service)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getaddrinfo, hints)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getaddrinfo, res);
}

int freeaddrinfo_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, freeaddrinfo, res);
}

int getnameinfo_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getnameinfo, sa)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getnameinfo, salen)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getnameinfo, host)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getnameinfo, hostlen)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getnameinfo, serv)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getnameinfo, servlen)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getnameinfo, flags);
}

int sendto_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, sendto, sockfd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, sendto, buf)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, sendto, len)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, sendto, flags)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, sendto, dest_addr)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, sendto, addrlen);
}

int sendmsg_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, sendmsg, sockfd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, sendmsg, msg)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, sendmsg, flags);
}

int recvfrom_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, recvfrom, sockfd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, recvfrom, buf)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, recvfrom, len)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, recvfrom, flags)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, recvfrom, src_addr)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, recvfrom, addrlen);
}

int recvmsg_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, recvmsg, sockfd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, recvmsg, msg)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, recvmsg, flags);
}

int waitid_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, waitid, idtype)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, waitid, id)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, waitid, infop)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, waitid, options);
}

int wait4_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, wait4, pid)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, wait4, status)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, wait4, options)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, wait4, rusage);
}

int sigaction_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, sigaction, signum)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, sigaction, act)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, sigaction, oldact);
}

int signal_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, signal, signum)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, signal, handler);
}

int sigset_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, sigset, sig)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, sigset, disp);
}

int fopen_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fopen, path)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fopen, mode);
}

int fopen64_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fopen64, path)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fopen64, mode);
}

int freopen_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, freopen, path)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, freopen, mode)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, freopen, stream);
}

int fclose_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fclose, fp);
}

int fdopen_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fdopen, fd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fdopen, mode);
}

int fgets_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fgets, s)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fgets, size)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fgets, stream);
}

int ferror_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, ferror, stream);
}

int feof_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, feof, stream);
}

int fileno_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fileno, stream);
}

int fflush_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fflush, stream);
}

int setvbuf_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, setvbuf, stream)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, setvbuf, buf)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, setvbuf, mode)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, setvbuf, size);
}

int fseek_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fseek, stream)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fseek, offset)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fseek, whence);
}

int fputs_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fputs, s)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fputs, stream);
}

int fputc_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fputc, c)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fputc, stream);
}

int fsync_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fsync, fd);
}

int ftell_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, ftell, stream);
}

int fgetpos_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fgetpos, stream)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fgetpos, pos);
}

int fgetpos64_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fgetpos64, stream)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fgetpos64, pos);
}

int fsetpos_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fsetpos, stream)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fsetpos, pos);
}

int fsetpos64_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fsetpos64, stream)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fsetpos64, pos);
}

int fwrite_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fwrite, ptr)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fwrite, size)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fwrite, nmemb)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fwrite, stream);
}

int fread_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fread, ptr)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fread, size)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fread, nmemb)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fread, stream);
}

int getc_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getc, stream);
}

int fgetc_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fgetc, stream);
}

int ungetc_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, ungetc, c)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, ungetc, stream);
}

int getline_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getline, lineptr)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getline, n)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getline, stream);
}

int getdelim_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getdelim, lineptr)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getdelim, n)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getdelim, delim)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, getdelim, stream);
}

int putc_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, putc, c)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, putc, stream);
}

int rewind_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, rewind, stream);
}

int tmpfile_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int flockfile_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, flockfile, filehandle);
}

int ftrylockfile_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, ftrylockfile, filehandle);
}

int funlockfile_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, funlockfile, filehandle);
}

int closedir_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, closedir, dirp);
}

int opendir_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, opendir, name);
}

int fdopendir_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fdopendir, fd);
}

int readdir_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, readdir, dirp);
}

int readdir_r_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, readdir_r, dirp)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, readdir_r, entry)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, readdir_r, result);
}

int pthread_cond_broadcast_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pthread_cond_broadcast, cond);
}

int pthread_cond_signal_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pthread_cond_signal, cond);
}

int pthread_cond_wait_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pthread_cond_wait, cond)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pthread_cond_wait, mutex);
}

int pthread_cond_timedwait_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pthread_cond_timedwait, cond)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pthread_cond_timedwait, mutex)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pthread_cond_timedwait, abstime);
}

int pthread_cond_destroy_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pthread_cond_destroy, cond);
}

int fxstat_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fxstat, vers)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fxstat, fd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fxstat, buf);
}

int fxstat64_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fxstat64, vers)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fxstat64, fd)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, fxstat64, buf);
}

int lxstat_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, lxstat, vers)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, lxstat, path)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, lxstat, buf);
}

int lxstat64_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, lxstat64, vers)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, lxstat64, path)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, lxstat64, buf);
}

int xstat_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, xstat, vers)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, xstat, path)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, xstat, buf);
}

int xstat64_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, xstat64, vers)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, xstat64, path)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, xstat64, buf);
}

int libc_memalign_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, libc_memalign, boundary)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, libc_memalign, size);
}

int vfprintf_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, vfprintf, stream)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, vfprintf, format)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, vfprintf, ap);
}

int vfscanf_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, vfscanf, stream)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, vfscanf, format)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, vfscanf, ap);
}

int exec_barrier_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int signal_handler_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, signal_handler, sig)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, signal_handler, info)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, signal_handler, data);
}

int user_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int syscall_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, syscall, num)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, syscall, a1)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, syscall, a2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, syscall, a3)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, syscall, a4)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, syscall, a5)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, syscall, a6)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, syscall, a7);
}
