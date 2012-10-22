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
  sizeof(log_event_fork_t),
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
  sizeof(log_event_shutdown_t),
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
  sizeof(log_event_puts_t),
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
  return log_event_size[entry->eventId()];
}
static void setupCommonFields(log_entry_t *e, clone_id_t cloneId,
                              event_code_t event)
{
  // Zero out all fields:
  memset(&(e->header), 0, sizeof(e->header));
  e->setCloneId(cloneId);
  e->setEventId(event);
}

log_entry_t create_empty_entry(clone_id_t cloneId, event_code_t event)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_accept_entry(clone_id_t cloneId, event_code_t event,
                                int sockfd, struct sockaddr* addr, socklen_t* addrlen)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_accept4_entry(clone_id_t cloneId, event_code_t event,
                                 int sockfd, struct sockaddr* addr, socklen_t* addrlen, int flags)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_access_entry(clone_id_t cloneId, event_code_t event,
                                const char* pathname, int mode)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_bind_entry(clone_id_t cloneId, event_code_t event,
                              int sockfd, const struct sockaddr* my_addr, socklen_t addrlen)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_calloc_entry(clone_id_t cloneId, event_code_t event,
                                size_t nmemb, size_t size)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  SET_FIELD(e, calloc, nmemb);
  SET_FIELD(e, calloc, size);
  return e;
}

log_entry_t create_chmod_entry(clone_id_t cloneId, event_code_t event,
                               const char* path, mode_t mode)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_chown_entry(clone_id_t cloneId, event_code_t event,
                               const char* path, uid_t owner, gid_t group)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_close_entry(clone_id_t cloneId, event_code_t event,
                               int fd)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_connect_entry(clone_id_t cloneId, event_code_t event,
                                 int sockfd, const struct sockaddr* serv_addr, socklen_t addrlen)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_dup_entry(clone_id_t cloneId, event_code_t event,
                             int oldfd)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_dup2_entry(clone_id_t cloneId, event_code_t event,
                              int oldfd, int newfd)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_dup3_entry(clone_id_t cloneId, event_code_t event,
                              int oldfd, int newfd, int flags)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_fcntl_entry(clone_id_t cloneId, event_code_t event,
                               int fd, int cmd, void* arg)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_fchdir_entry(clone_id_t cloneId, event_code_t event,
                                int fd)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_fdatasync_entry(clone_id_t cloneId, event_code_t event,
                                   int fd)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_getcwd_entry(clone_id_t cloneId, event_code_t event,
                                char* buf, size_t size)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_gettimeofday_entry(clone_id_t cloneId, event_code_t event,
                                      struct timeval* tv, struct timezone* tz)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_getpeername_entry(clone_id_t cloneId, event_code_t event,
                                     int sockfd, struct sockaddr* addr, socklen_t* addrlen)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_getsockname_entry(clone_id_t cloneId, event_code_t event,
                                     int sockfd, struct sockaddr* addr, socklen_t* addrlen)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_link_entry(clone_id_t cloneId, event_code_t event,
                              const char* oldpath, const char* newpath)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_symlink_entry(clone_id_t cloneId, event_code_t event,
                                 const char* oldpath, const char* newpath)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_listen_entry(clone_id_t cloneId, event_code_t event,
                                int sockfd, int backlog)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_localtime_r_entry(clone_id_t cloneId, event_code_t event,
                                     const time_t* timep, struct tm* result)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_utime_entry(clone_id_t cloneId, event_code_t event,
                               const char* filename, const struct utimbuf* times)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_utimes_entry(clone_id_t cloneId, event_code_t event,
                                const char* filename, const struct timeval* times)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_lutimes_entry(clone_id_t cloneId, event_code_t event,
                                 const char* filename, const struct timeval* tv)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_futimes_entry(clone_id_t cloneId, event_code_t event,
                                 int fd, const struct timeval* tv)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_clock_getres_entry(clone_id_t cloneId, event_code_t event,
                                      clockid_t clk_id, struct timespec* res)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_clock_gettime_entry(clone_id_t cloneId, event_code_t event,
                                       clockid_t clk_id, struct timespec* tp)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_clock_settime_entry(clone_id_t cloneId, event_code_t event,
                                       clockid_t clk_id, const struct timespec* tp)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_lseek_entry(clone_id_t cloneId, event_code_t event,
                               int fd, off_t offset, int whence)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_lseek64_entry(clone_id_t cloneId, event_code_t event,
                                 int fd, off64_t offset, int whence)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_llseek_entry(clone_id_t cloneId, event_code_t event,
                                int fd, loff_t offset, int whence)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_malloc_entry(clone_id_t cloneId, event_code_t event,
                                size_t size)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  SET_FIELD(e, malloc, size);
  return e;
}

log_entry_t create_free_entry(clone_id_t cloneId, event_code_t event,
                              void* ptr)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  SET_FIELD(e, free, ptr);
  return e;
}

log_entry_t create_mkdir_entry(clone_id_t cloneId, event_code_t event,
                               const char* pathname, mode_t mode)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_mkstemp_entry(clone_id_t cloneId, event_code_t event,
                                 char* temp)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_mmap_entry(clone_id_t cloneId, event_code_t event,
                              void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  SET_FIELD(e, mmap, addr);
  SET_FIELD(e, mmap, length);
  SET_FIELD(e, mmap, prot);
  SET_FIELD(e, mmap, flags);
  SET_FIELD(e, mmap, fd);
  SET_FIELD(e, mmap, offset);
  return e;
}

log_entry_t create_mmap64_entry(clone_id_t cloneId, event_code_t event,
                                void* addr, size_t length, int prot, int flags, int fd, off64_t offset)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  SET_FIELD(e, mmap64, addr);
  SET_FIELD(e, mmap64, length);
  SET_FIELD(e, mmap64, prot);
  SET_FIELD(e, mmap64, flags);
  SET_FIELD(e, mmap64, fd);
  SET_FIELD(e, mmap64, offset);
  return e;
}

log_entry_t create_munmap_entry(clone_id_t cloneId, event_code_t event,
                                void* addr, size_t length)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  SET_FIELD(e, munmap, addr);
  SET_FIELD(e, munmap, length);
  return e;
}

log_entry_t create_mremap_entry(clone_id_t cloneId, event_code_t event,
                                void* old_address, size_t old_size, size_t new_size, int flags, void* new_address)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_open_entry(clone_id_t cloneId, event_code_t event,
                              const char* pathname, int flags, mode_t mode)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_open64_entry(clone_id_t cloneId, event_code_t event,
                                const char* pathname, int flags, mode_t mode)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_openat_entry(clone_id_t cloneId, event_code_t event,
                                int dirfd, const char* pathname, int flags)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_pread_entry(clone_id_t cloneId, event_code_t event,
                               int fd, void* buf, size_t count, off_t offset)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_preadv_entry(clone_id_t cloneId, event_code_t event,
                                int fd, const struct iovec* iov, int iovcnt, off_t offset)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_pwrite_entry(clone_id_t cloneId, event_code_t event,
                                int fd, const void* buf, size_t count, off_t offset)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_pwritev_entry(clone_id_t cloneId, event_code_t event,
                                 int fd, const struct iovec* iov, int iovcnt, off_t offset)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_pthread_rwlock_unlock_entry(clone_id_t cloneId, event_code_t event,
                                               pthread_rwlock_t* rwlock)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_pthread_rwlock_rdlock_entry(clone_id_t cloneId, event_code_t event,
                                               pthread_rwlock_t* rwlock)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_pthread_rwlock_wrlock_entry(clone_id_t cloneId, event_code_t event,
                                               pthread_rwlock_t* rwlock)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_pthread_create_entry(clone_id_t cloneId, event_code_t event,
                                        pthread_t* thread, const pthread_attr_t* attr, pthread_start_routine_t start_routine, void* arg)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_pthread_detach_entry(clone_id_t cloneId, event_code_t event,
                                        pthread_t thread)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_pthread_exit_entry(clone_id_t cloneId, event_code_t event,
                                      void* value_ptr)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_pthread_join_entry(clone_id_t cloneId, event_code_t event,
                                      pthread_t thread, void** value_ptr)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  SET_FIELD(e, pthread_join, thread);
  return e;
}

log_entry_t create_pthread_kill_entry(clone_id_t cloneId, event_code_t event,
                                      pthread_t thread, int sig)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_pthread_mutex_lock_entry(clone_id_t cloneId, event_code_t event,
                                            pthread_mutex_t* mutex)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_pthread_mutex_trylock_entry(clone_id_t cloneId, event_code_t event,
                                               pthread_mutex_t* mutex)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_pthread_mutex_unlock_entry(clone_id_t cloneId, event_code_t event,
                                              pthread_mutex_t* mutex)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_rand_entry(clone_id_t cloneId, event_code_t event)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_fork_entry(clone_id_t cloneId, event_code_t event)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_read_entry(clone_id_t cloneId, event_code_t event,
                              int fd, void* buf, size_t count)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_readv_entry(clone_id_t cloneId, event_code_t event,
                               int fd, const struct iovec* iov, int iovcnt)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_readlink_entry(clone_id_t cloneId, event_code_t event,
                                  const char* path, char* buf, size_t bufsiz)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_realpath_entry(clone_id_t cloneId, event_code_t event,
                                  const char* path, char* resolved_path)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_realloc_entry(clone_id_t cloneId, event_code_t event,
                                 void* ptr, size_t size)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_rename_entry(clone_id_t cloneId, event_code_t event,
                                const char* oldpath, const char* newpath)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_rmdir_entry(clone_id_t cloneId, event_code_t event,
                               const char* pathname)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_select_entry(clone_id_t cloneId, event_code_t event,
                                int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_ppoll_entry(clone_id_t cloneId, event_code_t event,
                               struct pollfd* fds, nfds_t nfds, const struct timespec* timeout_ts, const sigset_t* sigmask)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_setsockopt_entry(clone_id_t cloneId, event_code_t event,
                                    int s, int level, int optname, const void* optval, socklen_t optlen)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_getsockopt_entry(clone_id_t cloneId, event_code_t event,
                                    int s, int level, int optname, void* optval, socklen_t* optlen)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_ioctl_entry(clone_id_t cloneId, event_code_t event,
                               int d, int request, void* arg)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_shutdown_entry(clone_id_t cloneId, event_code_t event,
                                  int sockfd, int how)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_sigwait_entry(clone_id_t cloneId, event_code_t event,
                                 const sigset_t* set, int* sig)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_srand_entry(clone_id_t cloneId, event_code_t event,
                               unsigned int seed)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_socket_entry(clone_id_t cloneId, event_code_t event,
                                int domain, int type, int protocol)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_socketpair_entry(clone_id_t cloneId, event_code_t event,
                                    int d, int type, int protocol, int* sv)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_time_entry(clone_id_t cloneId, event_code_t event,
                              time_t* tloc)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_truncate_entry(clone_id_t cloneId, event_code_t event,
                                  const char* path, off_t length)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_ftruncate_entry(clone_id_t cloneId, event_code_t event,
                                   int fd, off_t length)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_truncate64_entry(clone_id_t cloneId, event_code_t event,
                                    const char* path, off64_t length)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_ftruncate64_entry(clone_id_t cloneId, event_code_t event,
                                     int fd, off64_t length)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_unlink_entry(clone_id_t cloneId, event_code_t event,
                                const char* pathname)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_write_entry(clone_id_t cloneId, event_code_t event,
                               int fd, const void* buf, size_t count)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_writev_entry(clone_id_t cloneId, event_code_t event,
                                int fd, const struct iovec* iov, int iovcnt)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_epoll_create_entry(clone_id_t cloneId, event_code_t event,
                                      int size)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_epoll_create1_entry(clone_id_t cloneId, event_code_t event,
                                       int flags)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_epoll_ctl_entry(clone_id_t cloneId, event_code_t event,
                                   int epfd, int op, int fd, struct epoll_event* ep)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_epoll_wait_entry(clone_id_t cloneId, event_code_t event,
                                    int epfd, struct epoll_event* events, int maxevents, int timeout)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_getpwnam_r_entry(clone_id_t cloneId, event_code_t event,
                                    const char* name, struct passwd* pwd, char* buf, size_t buflen, struct passwd** result)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_getpwuid_r_entry(clone_id_t cloneId, event_code_t event,
                                    uid_t uid, struct passwd* pwd, char* buf, size_t buflen, struct passwd** result)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_getgrnam_r_entry(clone_id_t cloneId, event_code_t event,
                                    const char* name, struct group* grp, char* buf, size_t buflen, struct group** result)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_getgrgid_r_entry(clone_id_t cloneId, event_code_t event,
                                    gid_t gid, struct group* grp, char* buf, size_t buflen, struct group** result)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_getaddrinfo_entry(clone_id_t cloneId, event_code_t event,
                                     const char* node, const char* service, const struct addrinfo* hints, struct addrinfo** res)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_freeaddrinfo_entry(clone_id_t cloneId, event_code_t event,
                                      struct addrinfo* res)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_getnameinfo_entry(clone_id_t cloneId, event_code_t event,
                                     const struct sockaddr* sa, socklen_t salen, char* host, size_t hostlen, char* serv, size_t servlen, int flags)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_sendto_entry(clone_id_t cloneId, event_code_t event,
                                int sockfd, const void* buf, size_t len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_sendmsg_entry(clone_id_t cloneId, event_code_t event,
                                 int sockfd, const struct msghdr* msg, int flags)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_recvfrom_entry(clone_id_t cloneId, event_code_t event,
                                  int sockfd, void* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_recvmsg_entry(clone_id_t cloneId, event_code_t event,
                                 int sockfd, struct msghdr* msg, int flags)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_waitid_entry(clone_id_t cloneId, event_code_t event,
                                idtype_t idtype, id_t id, siginfo_t* infop, int options)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_wait4_entry(clone_id_t cloneId, event_code_t event,
                               pid_t pid, __WAIT_STATUS status, int options, struct rusage* rusage)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_sigaction_entry(clone_id_t cloneId, event_code_t event,
                                   int signum, const struct sigaction* act, struct sigaction* oldact)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_signal_entry(clone_id_t cloneId, event_code_t event,
                                int signum, sighandler_t handler)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_sigset_entry(clone_id_t cloneId, event_code_t event,
                                int sig, sighandler_t disp)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_fopen_entry(clone_id_t cloneId, event_code_t event,
                               const char* path, const char* mode)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_fopen64_entry(clone_id_t cloneId, event_code_t event,
                                 const char* path, const char* mode)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_freopen_entry(clone_id_t cloneId, event_code_t event,
                                 const char* path, const char* mode, FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_fclose_entry(clone_id_t cloneId, event_code_t event,
                                FILE* fp)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_fdopen_entry(clone_id_t cloneId, event_code_t event,
                                int fd, const char* mode)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_fgets_entry(clone_id_t cloneId, event_code_t event,
                               char* s, int size, FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_ferror_entry(clone_id_t cloneId, event_code_t event,
                                FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_feof_entry(clone_id_t cloneId, event_code_t event,
                              FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_fileno_entry(clone_id_t cloneId, event_code_t event,
                                FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_fflush_entry(clone_id_t cloneId, event_code_t event,
                                FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_setvbuf_entry(clone_id_t cloneId, event_code_t event,
                                 FILE* stream, char* buf, int mode, size_t size)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_fseek_entry(clone_id_t cloneId, event_code_t event,
                               FILE* stream, long offset, int whence)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_fputs_entry(clone_id_t cloneId, event_code_t event,
                               const char* s, FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_puts_entry(clone_id_t cloneId, event_code_t event,
                              const char* s)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_fputc_entry(clone_id_t cloneId, event_code_t event,
                               int c, FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_fsync_entry(clone_id_t cloneId, event_code_t event,
                               int fd)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_ftell_entry(clone_id_t cloneId, event_code_t event,
                               FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_fgetpos_entry(clone_id_t cloneId, event_code_t event,
                                 FILE* stream, fpos_t* pos)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_fgetpos64_entry(clone_id_t cloneId, event_code_t event,
                                   FILE* stream, fpos64_t* pos)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_fsetpos_entry(clone_id_t cloneId, event_code_t event,
                                 FILE* stream, const fpos_t* pos)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_fsetpos64_entry(clone_id_t cloneId, event_code_t event,
                                   FILE* stream, const fpos64_t* pos)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_fwrite_entry(clone_id_t cloneId, event_code_t event,
                                const void* ptr, size_t size, size_t nmemb, FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_fread_entry(clone_id_t cloneId, event_code_t event,
                               void* ptr, size_t size, size_t nmemb, FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_getc_entry(clone_id_t cloneId, event_code_t event,
                              FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_fgetc_entry(clone_id_t cloneId, event_code_t event,
                               FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_ungetc_entry(clone_id_t cloneId, event_code_t event,
                                int c, FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_getline_entry(clone_id_t cloneId, event_code_t event,
                                 char** lineptr, size_t* n, FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_getdelim_entry(clone_id_t cloneId, event_code_t event,
                                  char** lineptr, size_t* n, int delim, FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_putc_entry(clone_id_t cloneId, event_code_t event,
                              int c, FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_rewind_entry(clone_id_t cloneId, event_code_t event,
                                FILE* stream)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_tmpfile_entry(clone_id_t cloneId, event_code_t event)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_flockfile_entry(clone_id_t cloneId, event_code_t event,
                                   FILE* filehandle)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_ftrylockfile_entry(clone_id_t cloneId, event_code_t event,
                                      FILE* filehandle)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_funlockfile_entry(clone_id_t cloneId, event_code_t event,
                                     FILE* filehandle)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_closedir_entry(clone_id_t cloneId, event_code_t event,
                                  DIR* dirp)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_opendir_entry(clone_id_t cloneId, event_code_t event,
                                 const char* name)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_fdopendir_entry(clone_id_t cloneId, event_code_t event,
                                   int fd)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_readdir_entry(clone_id_t cloneId, event_code_t event,
                                 DIR* dirp)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_readdir_r_entry(clone_id_t cloneId, event_code_t event,
                                   DIR* dirp, struct dirent* entry, struct dirent** result)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_pthread_cond_broadcast_entry(clone_id_t cloneId, event_code_t event,
                                                pthread_cond_t* cond)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_pthread_cond_signal_entry(clone_id_t cloneId, event_code_t event,
                                             pthread_cond_t* cond)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_pthread_cond_wait_entry(clone_id_t cloneId, event_code_t event,
                                           pthread_cond_t* cond, pthread_mutex_t* mutex)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_pthread_cond_timedwait_entry(clone_id_t cloneId, event_code_t event,
                                                pthread_cond_t* cond, pthread_mutex_t* mutex, const struct timespec* abstime)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_pthread_cond_destroy_entry(clone_id_t cloneId, event_code_t event,
                                              pthread_cond_t* cond)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_fxstat_entry(clone_id_t cloneId, event_code_t event,
                                int vers, int fd, struct stat* buf)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_fxstat64_entry(clone_id_t cloneId, event_code_t event,
                                  int vers, int fd, struct stat64* buf)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_lxstat_entry(clone_id_t cloneId, event_code_t event,
                                int vers, const char* path, struct stat* buf)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_lxstat64_entry(clone_id_t cloneId, event_code_t event,
                                  int vers, const char* path, struct stat64* buf)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_xstat_entry(clone_id_t cloneId, event_code_t event,
                               int vers, const char* path, struct stat* buf)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_xstat64_entry(clone_id_t cloneId, event_code_t event,
                                 int vers, const char* path, struct stat64* buf)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_libc_memalign_entry(clone_id_t cloneId, event_code_t event,
                                       size_t boundary, size_t size)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_vfprintf_entry(clone_id_t cloneId, event_code_t event,
                                  FILE* stream, const char* format, va_list ap)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_vfscanf_entry(clone_id_t cloneId, event_code_t event,
                                 FILE* stream, const char* format, va_list ap)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_exec_barrier_entry(clone_id_t cloneId, event_code_t event)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_signal_handler_entry(clone_id_t cloneId, event_code_t event,
                                        int sig, siginfo_t* info, void* data)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_user_entry(clone_id_t cloneId, event_code_t event)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}

log_entry_t create_syscall_entry(clone_id_t cloneId, event_code_t event,
                                 int num, void* a1, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7)
{
  log_entry_t e = EMPTY_LOG_ENTRY;
  setupCommonFields(&e, cloneId, event);
  return e;
}
static int base_turn_check(log_entry_t *e1, log_entry_t *e2) {
  // Predicate function for a basic check -- event # and clone id.
  return e1->cloneId() == e2->cloneId() &&
         e1->eventId() == e2->eventId();
}

int empty_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int accept_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int accept4_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int access_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int bind_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int calloc_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, calloc, nmemb)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, calloc, size);
}

int chmod_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int chown_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int close_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int connect_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int dup_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int dup2_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int dup3_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int fcntl_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int fchdir_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int fdatasync_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int getcwd_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int gettimeofday_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int getpeername_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int getsockname_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int link_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int symlink_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int listen_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int localtime_r_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int utime_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int utimes_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int lutimes_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int futimes_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int clock_getres_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int clock_gettime_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int clock_settime_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int lseek_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int lseek64_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int llseek_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
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
  return base_turn_check(e1,e2);
}

int mkstemp_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
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
  return base_turn_check(e1,e2);
}

int open_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int open64_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int openat_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int pread_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int preadv_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int pwrite_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int pwritev_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int pthread_rwlock_unlock_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int pthread_rwlock_rdlock_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int pthread_rwlock_wrlock_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int pthread_create_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int pthread_detach_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int pthread_exit_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int pthread_join_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2)
    && ARE_FIELDS_EQUAL_PTR (e1, e2, pthread_join, thread);
}

int pthread_kill_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int pthread_mutex_lock_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int pthread_mutex_trylock_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int pthread_mutex_unlock_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int rand_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int fork_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int read_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int readv_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int readlink_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int realpath_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int realloc_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int rename_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int rmdir_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int select_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int ppoll_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int setsockopt_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int getsockopt_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int ioctl_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int shutdown_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int sigwait_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int srand_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int socket_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int socketpair_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int time_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int truncate_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int ftruncate_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int truncate64_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int ftruncate64_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int unlink_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int write_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int writev_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int epoll_create_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int epoll_create1_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int epoll_ctl_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int epoll_wait_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int getpwnam_r_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int getpwuid_r_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int getgrnam_r_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int getgrgid_r_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int getaddrinfo_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int freeaddrinfo_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int getnameinfo_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int sendto_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int sendmsg_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int recvfrom_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int recvmsg_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int waitid_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int wait4_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int sigaction_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int signal_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int sigset_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int fopen_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int fopen64_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int freopen_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int fclose_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int fdopen_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int fgets_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int ferror_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int feof_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int fileno_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int fflush_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int setvbuf_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int fseek_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int fputs_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int puts_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int fputc_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int fsync_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int ftell_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int fgetpos_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int fgetpos64_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int fsetpos_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int fsetpos64_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int fwrite_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int fread_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int getc_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int fgetc_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int ungetc_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int getline_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int getdelim_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int putc_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int rewind_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int tmpfile_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int flockfile_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int ftrylockfile_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int funlockfile_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int closedir_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int opendir_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int fdopendir_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int readdir_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int readdir_r_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int pthread_cond_broadcast_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int pthread_cond_signal_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int pthread_cond_wait_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int pthread_cond_timedwait_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int pthread_cond_destroy_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int fxstat_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int fxstat64_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int lxstat_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int lxstat64_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int xstat_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int xstat64_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int libc_memalign_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int vfprintf_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int vfscanf_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int exec_barrier_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int signal_handler_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int user_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}

int syscall_turn_check(log_entry_t *e1, log_entry_t *e2)
{
  return base_turn_check(e1,e2);
}
