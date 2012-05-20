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

#ifndef WRAPPER_UTIL_H
#define WRAPPER_UTIL_H

#include "fred_wrappers.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
} log_event_empty_t;

typedef struct {
  int sockfd;
  struct sockaddr* addr;
  struct sockaddr ret_addr;
  socklen_t* addrlen;
  socklen_t ret_addrlen;
} log_event_accept_t;

typedef struct {
  int sockfd;
  struct sockaddr* addr;
  struct sockaddr ret_addr;
  socklen_t* addrlen;
  socklen_t ret_addrlen;
  int flags;
} log_event_accept4_t;

typedef struct {
  const char* pathname;
  int mode;
} log_event_access_t;

typedef struct {
  int sockfd;
  const struct sockaddr* my_addr;
  socklen_t addrlen;
} log_event_bind_t;

typedef struct {
  size_t nmemb;
  size_t size;
} log_event_calloc_t;

typedef struct {
  const char* path;
  mode_t mode;
} log_event_chmod_t;

typedef struct {
  const char* path;
  uid_t owner;
  gid_t group;
} log_event_chown_t;

typedef struct {
  int fd;
} log_event_close_t;

typedef struct {
  int sockfd;
  const struct sockaddr* serv_addr;
  socklen_t addrlen;
} log_event_connect_t;

typedef struct {
  int oldfd;
} log_event_dup_t;

typedef struct {
  int oldfd;
  int newfd;
} log_event_dup2_t;

typedef struct {
  int oldfd;
  int newfd;
  int flags;
} log_event_dup3_t;

typedef struct {
  int fd;
  int cmd;
  void* arg;
  struct flock ret_flock;
} log_event_fcntl_t;

typedef struct {
  int fd;
} log_event_fchdir_t;

typedef struct {
  int fd;
} log_event_fdatasync_t;

typedef struct {
  char* buf;
  size_t size;
  off_t data_offset;
} log_event_getcwd_t;

typedef struct {
  struct timeval* tv;
  struct timeval ret_tv;
  struct timezone* tz;
  struct timezone ret_tz;
} log_event_gettimeofday_t;

typedef struct {
  int sockfd;
  struct sockaddr* addr;
  struct sockaddr ret_addr;
  socklen_t* addrlen;
  socklen_t ret_addrlen;
} log_event_getpeername_t;

typedef struct {
  int sockfd;
  struct sockaddr* addr;
  struct sockaddr ret_addr;
  socklen_t* addrlen;
  socklen_t ret_addrlen;
} log_event_getsockname_t;

typedef struct {
  const char* oldpath;
  const char* newpath;
} log_event_link_t;

typedef struct {
  const char* oldpath;
  const char* newpath;
} log_event_symlink_t;

typedef struct {
  int sockfd;
  int backlog;
} log_event_listen_t;

typedef struct {
  const time_t* timep;
  struct tm* result;
  struct tm ret_result;
} log_event_localtime_r_t;

typedef struct {
  const char* filename;
  const struct utimbuf* times;
} log_event_utime_t;

typedef struct {
  const char* filename;
  const struct timeval* times;
} log_event_utimes_t;

typedef struct {
  const char* filename;
  const struct timeval* tv;
} log_event_lutimes_t;

typedef struct {
  int fd;
  const struct timeval* tv;
} log_event_futimes_t;

typedef struct {
  clockid_t clk_id;
  struct timespec* res;
  struct timespec ret_res;
} log_event_clock_getres_t;

typedef struct {
  clockid_t clk_id;
  struct timespec* tp;
  struct timespec ret_tp;
} log_event_clock_gettime_t;

typedef struct {
  clockid_t clk_id;
  const struct timespec* tp;
} log_event_clock_settime_t;

typedef struct {
  int fd;
  off_t offset;
  int whence;
} log_event_lseek_t;

typedef struct {
  int fd;
  off64_t offset;
  int whence;
} log_event_lseek64_t;

typedef struct {
  int fd;
  loff_t offset;
  int whence;
} log_event_llseek_t;

typedef struct {
  size_t size;
} log_event_malloc_t;

typedef struct {
  void* ptr;
} log_event_free_t;

typedef struct {
  const char* pathname;
  mode_t mode;
} log_event_mkdir_t;

typedef struct {
  char* temp;
} log_event_mkstemp_t;

typedef struct {
  void* addr;
  size_t length;
  int prot;
  int flags;
  int fd;
  off_t offset;
  off_t data_offset;
} log_event_mmap_t;

typedef struct {
  void* addr;
  size_t length;
  int prot;
  int flags;
  int fd;
  off64_t offset;
  off_t data_offset;
} log_event_mmap64_t;

typedef struct {
  void* addr;
  size_t length;
} log_event_munmap_t;

typedef struct {
  void* old_address;
  size_t old_size;
  size_t new_size;
  int flags;
  void* new_address;
} log_event_mremap_t;

typedef struct {
  const char* pathname;
  int flags;
  mode_t mode;
} log_event_open_t;

typedef struct {
  const char* pathname;
  int flags;
  mode_t mode;
} log_event_open64_t;

typedef struct {
  int dirfd;
  const char* pathname;
  int flags;
} log_event_openat_t;

typedef struct {
  int fd;
  void* buf;
  size_t count;
  off_t offset;
  off_t data_offset;
} log_event_pread_t;

typedef struct {
  int fd;
  const struct iovec* iov;
  int iovcnt;
  off_t offset;
  off_t data_offset;
} log_event_preadv_t;

typedef struct {
  int fd;
  const void* buf;
  size_t count;
  off_t offset;
} log_event_pwrite_t;

typedef struct {
  int fd;
  const struct iovec* iov;
  int iovcnt;
  off_t offset;
} log_event_pwritev_t;

typedef struct {
  pthread_rwlock_t* rwlock;
  pthread_rwlock_t ret_rwlock;
} log_event_pthread_rwlock_unlock_t;

typedef struct {
  pthread_rwlock_t* rwlock;
  pthread_rwlock_t ret_rwlock;
} log_event_pthread_rwlock_rdlock_t;

typedef struct {
  pthread_rwlock_t* rwlock;
  pthread_rwlock_t ret_rwlock;
} log_event_pthread_rwlock_wrlock_t;

typedef struct {
  pthread_t* thread;
  const pthread_attr_t* attr;
  pthread_start_routine_t start_routine;
  void* arg;
  void *stack_addr;
  size_t stack_size;
} log_event_pthread_create_t;

typedef struct {
  pthread_t thread;
} log_event_pthread_detach_t;

typedef struct {
  void* value_ptr;
} log_event_pthread_exit_t;

typedef struct {
  pthread_t thread;
  void** value_ptr;
} log_event_pthread_join_t;

typedef struct {
  pthread_t thread;
  int sig;
} log_event_pthread_kill_t;

typedef struct {
  pthread_mutex_t* mutex;
  pthread_mutex_t ret_mutex;
} log_event_pthread_mutex_lock_t;

typedef struct {
  pthread_mutex_t* mutex;
  pthread_mutex_t ret_mutex;
} log_event_pthread_mutex_trylock_t;

typedef struct {
  pthread_mutex_t* mutex;
  pthread_mutex_t ret_mutex;
} log_event_pthread_mutex_unlock_t;

typedef struct {
} log_event_rand_t;

typedef struct {
  int fd;
  void* buf;
  size_t count;
  off_t data_offset;
} log_event_read_t;

typedef struct {
  int fd;
  const struct iovec* iov;
  int iovcnt;
  off_t data_offset;
} log_event_readv_t;

typedef struct {
  const char* path;
  char* buf;
  size_t bufsiz;
  off_t data_offset;
} log_event_readlink_t;

typedef struct {
  const char* path;
  char* resolved_path;
  off_t data_offset;
  size_t len;
} log_event_realpath_t;

typedef struct {
  void* ptr;
  size_t size;
} log_event_realloc_t;

typedef struct {
  const char* oldpath;
  const char* newpath;
} log_event_rename_t;

typedef struct {
  const char* pathname;
} log_event_rmdir_t;

typedef struct {
  int nfds;
  fd_set* readfds;
  fd_set ret_readfds;
  fd_set* writefds;
  fd_set ret_writefds;
  fd_set* exceptfds;
  struct timeval* timeout;
} log_event_select_t;

typedef struct {
  struct pollfd* fds;
  nfds_t nfds;
  const struct timespec* timeout_ts;
  const sigset_t* sigmask;
  off_t data_offset;
} log_event_ppoll_t;

typedef struct {
  int s;
  int level;
  int optname;
  const void* optval;
  socklen_t optlen;
} log_event_setsockopt_t;

typedef struct {
  int s;
  int level;
  int optname;
  void* optval;
  socklen_t* optlen;
  socklen_t ret_optlen;
  off_t data_offset;
} log_event_getsockopt_t;

typedef struct {
  int d;
  int request;
  void* arg;
  off_t data_offset;
  struct winsize win_val;
  struct ifconf ifconf_val;
  int fionread_val;
} log_event_ioctl_t;

typedef struct {
  const sigset_t* set;
  int* sig;
  int ret_sig;
} log_event_sigwait_t;

typedef struct {
  unsigned int seed;
} log_event_srand_t;

typedef struct {
  int domain;
  int type;
  int protocol;
} log_event_socket_t;

typedef struct {
  int d;
  int type;
  int protocol;
  int* sv;
  int ret_sv[2];
} log_event_socketpair_t;

typedef struct {
  time_t* tloc;
  time_t ret_tloc;
} log_event_time_t;

typedef struct {
  const char* path;
  off_t length;
} log_event_truncate_t;

typedef struct {
  int fd;
  off_t length;
} log_event_ftruncate_t;

typedef struct {
  const char* path;
  off64_t length;
} log_event_truncate64_t;

typedef struct {
  int fd;
  off64_t length;
} log_event_ftruncate64_t;

typedef struct {
  const char* pathname;
} log_event_unlink_t;

typedef struct {
  int fd;
  const void* buf;
  size_t count;
} log_event_write_t;

typedef struct {
  int fd;
  const struct iovec* iov;
  int iovcnt;
} log_event_writev_t;

typedef struct {
  int size;
} log_event_epoll_create_t;

typedef struct {
  int flags;
} log_event_epoll_create1_t;

typedef struct {
  int epfd;
  int op;
  int fd;
  struct epoll_event* ep;
} log_event_epoll_ctl_t;

typedef struct {
  int epfd;
  struct epoll_event* events;
  int maxevents;
  int timeout;
  off_t data_offset;
} log_event_epoll_wait_t;

typedef struct {
  const char* name;
  struct passwd* pwd;
  struct passwd ret_pwd;
  char* buf;
  size_t buflen;
  struct passwd** result;
  struct passwd* ret_result;
  off_t data_offset;
} log_event_getpwnam_r_t;

typedef struct {
  uid_t uid;
  struct passwd* pwd;
  struct passwd ret_pwd;
  char* buf;
  size_t buflen;
  struct passwd** result;
  struct passwd* ret_result;
  off_t data_offset;
} log_event_getpwuid_r_t;

typedef struct {
  const char* name;
  struct group* grp;
  struct group ret_grp;
  char* buf;
  size_t buflen;
  struct group** result;
  struct group* ret_result;
  off_t data_offset;
} log_event_getgrnam_r_t;

typedef struct {
  gid_t gid;
  struct group* grp;
  struct group ret_grp;
  char* buf;
  size_t buflen;
  struct group** result;
  struct group* ret_result;
  off_t data_offset;
} log_event_getgrgid_r_t;

typedef struct {
  const char* node;
  const char* service;
  const struct addrinfo* hints;
  struct addrinfo** res;
  struct addrinfo* ret_res;
  off_t data_offset;
  int num_results;
} log_event_getaddrinfo_t;

typedef struct {
  struct addrinfo* res;
} log_event_freeaddrinfo_t;

typedef struct {
  const struct sockaddr* sa;
  socklen_t salen;
  char* host;
  size_t hostlen;
  char* serv;
  size_t servlen;
  int flags;
  off_t data_offset;
  char ret_host[NI_MAXHOST];
  char ret_serv[NI_MAXSERV];
} log_event_getnameinfo_t;

typedef struct {
  int sockfd;
  const void* buf;
  size_t len;
  int flags;
  const struct sockaddr* dest_addr;
  socklen_t addrlen;
} log_event_sendto_t;

typedef struct {
  int sockfd;
  const struct msghdr* msg;
  int flags;
} log_event_sendmsg_t;

typedef struct {
  int sockfd;
  void* buf;
  size_t len;
  int flags;
  struct sockaddr* src_addr;
  struct sockaddr ret_src_addr;
  socklen_t* addrlen;
  socklen_t ret_addrlen;
  off_t data_offset;
} log_event_recvfrom_t;

typedef struct {
  int sockfd;
  struct msghdr* msg;
  struct msghdr ret_msg;
  int flags;
  off_t data_offset;
  off_t control_buf_offset;
} log_event_recvmsg_t;

typedef struct {
  idtype_t idtype;
  id_t id;
  siginfo_t* infop;
  siginfo_t ret_infop;
  int options;
} log_event_waitid_t;

typedef struct {
  pid_t pid;
  __WAIT_STATUS status;
  void* ret_status;
  int options;
  struct rusage* rusage;
  struct rusage ret_rusage;
} log_event_wait4_t;

typedef struct {
  int signum;
  const struct sigaction* act;
  struct sigaction* oldact;
} log_event_sigaction_t;

typedef struct {
  int signum;
  sighandler_t handler;
} log_event_signal_t;

typedef struct {
  int sig;
  sighandler_t disp;
} log_event_sigset_t;

typedef struct {
  const char* path;
  const char* mode;
  FILE fopen_retval;
} log_event_fopen_t;

typedef struct {
  const char* path;
  const char* mode;
  FILE fopen64_retval;
} log_event_fopen64_t;

typedef struct {
  const char* path;
  const char* mode;
  FILE* stream;
  FILE freopen_retval;
} log_event_freopen_t;

typedef struct {
  FILE* fp;
} log_event_fclose_t;

typedef struct {
  int fd;
  const char* mode;
  FILE fdopen_retval;
} log_event_fdopen_t;

typedef struct {
  char* s;
  int size;
  FILE* stream;
  off_t data_offset;
} log_event_fgets_t;

typedef struct {
  FILE* stream;
} log_event_ferror_t;

typedef struct {
  FILE* stream;
} log_event_feof_t;

typedef struct {
  FILE* stream;
} log_event_fileno_t;

typedef struct {
  FILE* stream;
} log_event_fflush_t;

typedef struct {
  FILE* stream;
  char* buf;
  int mode;
  size_t size;
} log_event_setvbuf_t;

typedef struct {
  FILE* stream;
  long offset;
  int whence;
} log_event_fseek_t;

typedef struct {
  const char* s;
  FILE* stream;
} log_event_fputs_t;

typedef struct {
  int c;
  FILE* stream;
} log_event_fputc_t;

typedef struct {
  int fd;
} log_event_fsync_t;

typedef struct {
  FILE* stream;
} log_event_ftell_t;

typedef struct {
  FILE* stream;
  fpos_t* pos;
} log_event_fgetpos_t;

typedef struct {
  FILE* stream;
  fpos64_t* pos;
} log_event_fgetpos64_t;

typedef struct {
  FILE* stream;
  const fpos_t* pos;
} log_event_fsetpos_t;

typedef struct {
  FILE* stream;
  const fpos64_t* pos;
} log_event_fsetpos64_t;

typedef struct {
  const void* ptr;
  size_t size;
  size_t nmemb;
  FILE* stream;
} log_event_fwrite_t;

typedef struct {
  void* ptr;
  size_t size;
  size_t nmemb;
  FILE* stream;
  off_t data_offset;
} log_event_fread_t;

typedef struct {
  FILE* stream;
} log_event_getc_t;

typedef struct {
  FILE* stream;
} log_event_fgetc_t;

typedef struct {
  int c;
  FILE* stream;
} log_event_ungetc_t;

typedef struct {
  char** lineptr;
  char* ret_lineptr;
  size_t* n;
  size_t ret_n;
  FILE* stream;
  off_t data_offset;
} log_event_getline_t;

typedef struct {
  char** lineptr;
  char* ret_lineptr;
  size_t* n;
  size_t ret_n;
  int delim;
  FILE* stream;
  off_t data_offset;
} log_event_getdelim_t;

typedef struct {
  int c;
  FILE* stream;
} log_event_putc_t;

typedef struct {
  FILE* stream;
} log_event_rewind_t;

typedef struct {
  FILE tmpfile_retval;
} log_event_tmpfile_t;

typedef struct {
  FILE* filehandle;
} log_event_flockfile_t;

typedef struct {
  FILE* filehandle;
} log_event_ftrylockfile_t;

typedef struct {
  FILE* filehandle;
} log_event_funlockfile_t;

typedef struct {
  DIR* dirp;
} log_event_closedir_t;

typedef struct {
  const char* name;
} log_event_opendir_t;

typedef struct {
  int fd;
} log_event_fdopendir_t;

typedef struct {
  DIR* dirp;
  struct dirent readdir_retval;
} log_event_readdir_t;

typedef struct {
  DIR* dirp;
  struct dirent* entry;
  struct dirent ret_entry;
  struct dirent** result;
  struct dirent* ret_result;
} log_event_readdir_r_t;

typedef struct {
  pthread_cond_t* cond;
  pthread_cond_t ret_cond;
} log_event_pthread_cond_broadcast_t;

typedef struct {
  pthread_cond_t* cond;
  pthread_cond_t ret_cond;
} log_event_pthread_cond_signal_t;

typedef struct {
  pthread_cond_t* cond;
  pthread_cond_t ret_cond;
  pthread_mutex_t* mutex;
  pthread_mutex_t ret_mutex;
} log_event_pthread_cond_wait_t;

typedef struct {
  pthread_cond_t* cond;
  pthread_cond_t ret_cond;
  pthread_mutex_t* mutex;
  pthread_mutex_t ret_mutex;
  const struct timespec* abstime;
} log_event_pthread_cond_timedwait_t;

typedef struct {
  pthread_cond_t* cond;
  pthread_cond_t ret_cond;
} log_event_pthread_cond_destroy_t;

typedef struct {
  int vers;
  int fd;
  struct stat* buf;
  struct stat ret_buf;
} log_event_fxstat_t;

typedef struct {
  int vers;
  int fd;
  struct stat64* buf;
  struct stat64 ret_buf;
} log_event_fxstat64_t;

typedef struct {
  int vers;
  const char* path;
  struct stat* buf;
  struct stat ret_buf;
} log_event_lxstat_t;

typedef struct {
  int vers;
  const char* path;
  struct stat64* buf;
  struct stat64 ret_buf;
} log_event_lxstat64_t;

typedef struct {
  int vers;
  const char* path;
  struct stat* buf;
  struct stat ret_buf;
} log_event_xstat_t;

typedef struct {
  int vers;
  const char* path;
  struct stat64* buf;
  struct stat64 ret_buf;
} log_event_xstat64_t;

typedef struct {
  size_t boundary;
  size_t size;
} log_event_libc_memalign_t;

typedef struct {
  FILE* stream;
  const char* format;
  va_list ap;
} log_event_vfprintf_t;

typedef struct {
  FILE* stream;
  const char* format;
  va_list ap;
  off_t data_offset;
  int bytes;
} log_event_vfscanf_t;

typedef struct {
} log_event_exec_barrier_t;

typedef struct {
  int sig;
  siginfo_t* info;
  void* data;
} log_event_signal_handler_t;

typedef struct {
} log_event_user_t;

typedef struct {
  int num;
  void* a1;
  void* a2;
  void* a3;
  void* a4;
  void* a5;
  void* a6;
  void* a7;
} log_event_syscall_t;


union log_entry_data {
  log_event_empty_t log_event_empty;
  log_event_accept_t log_event_accept;
  log_event_accept4_t log_event_accept4;
  log_event_access_t log_event_access;
  log_event_bind_t log_event_bind;
  log_event_calloc_t log_event_calloc;
  log_event_chmod_t log_event_chmod;
  log_event_chown_t log_event_chown;
  log_event_close_t log_event_close;
  log_event_connect_t log_event_connect;
  log_event_dup_t log_event_dup;
  log_event_dup2_t log_event_dup2;
  log_event_dup3_t log_event_dup3;
  log_event_fcntl_t log_event_fcntl;
  log_event_fchdir_t log_event_fchdir;
  log_event_fdatasync_t log_event_fdatasync;
  log_event_getcwd_t log_event_getcwd;
  log_event_gettimeofday_t log_event_gettimeofday;
  log_event_getpeername_t log_event_getpeername;
  log_event_getsockname_t log_event_getsockname;
  log_event_link_t log_event_link;
  log_event_symlink_t log_event_symlink;
  log_event_listen_t log_event_listen;
  log_event_localtime_r_t log_event_localtime_r;
  log_event_utime_t log_event_utime;
  log_event_utimes_t log_event_utimes;
  log_event_lutimes_t log_event_lutimes;
  log_event_futimes_t log_event_futimes;
  log_event_clock_getres_t log_event_clock_getres;
  log_event_clock_gettime_t log_event_clock_gettime;
  log_event_clock_settime_t log_event_clock_settime;
  log_event_lseek_t log_event_lseek;
  log_event_lseek64_t log_event_lseek64;
  log_event_llseek_t log_event_llseek;
  log_event_malloc_t log_event_malloc;
  log_event_free_t log_event_free;
  log_event_mkdir_t log_event_mkdir;
  log_event_mkstemp_t log_event_mkstemp;
  log_event_mmap_t log_event_mmap;
  log_event_mmap64_t log_event_mmap64;
  log_event_munmap_t log_event_munmap;
  log_event_mremap_t log_event_mremap;
  log_event_open_t log_event_open;
  log_event_open64_t log_event_open64;
  log_event_openat_t log_event_openat;
  log_event_pread_t log_event_pread;
  log_event_preadv_t log_event_preadv;
  log_event_pwrite_t log_event_pwrite;
  log_event_pwritev_t log_event_pwritev;
  log_event_pthread_rwlock_unlock_t log_event_pthread_rwlock_unlock;
  log_event_pthread_rwlock_rdlock_t log_event_pthread_rwlock_rdlock;
  log_event_pthread_rwlock_wrlock_t log_event_pthread_rwlock_wrlock;
  log_event_pthread_create_t log_event_pthread_create;
  log_event_pthread_detach_t log_event_pthread_detach;
  log_event_pthread_exit_t log_event_pthread_exit;
  log_event_pthread_join_t log_event_pthread_join;
  log_event_pthread_kill_t log_event_pthread_kill;
  log_event_pthread_mutex_lock_t log_event_pthread_mutex_lock;
  log_event_pthread_mutex_trylock_t log_event_pthread_mutex_trylock;
  log_event_pthread_mutex_unlock_t log_event_pthread_mutex_unlock;
  log_event_rand_t log_event_rand;
  log_event_read_t log_event_read;
  log_event_readv_t log_event_readv;
  log_event_readlink_t log_event_readlink;
  log_event_realpath_t log_event_realpath;
  log_event_realloc_t log_event_realloc;
  log_event_rename_t log_event_rename;
  log_event_rmdir_t log_event_rmdir;
  log_event_select_t log_event_select;
  log_event_ppoll_t log_event_ppoll;
  log_event_setsockopt_t log_event_setsockopt;
  log_event_getsockopt_t log_event_getsockopt;
  log_event_ioctl_t log_event_ioctl;
  log_event_sigwait_t log_event_sigwait;
  log_event_srand_t log_event_srand;
  log_event_socket_t log_event_socket;
  log_event_socketpair_t log_event_socketpair;
  log_event_time_t log_event_time;
  log_event_truncate_t log_event_truncate;
  log_event_ftruncate_t log_event_ftruncate;
  log_event_truncate64_t log_event_truncate64;
  log_event_ftruncate64_t log_event_ftruncate64;
  log_event_unlink_t log_event_unlink;
  log_event_write_t log_event_write;
  log_event_writev_t log_event_writev;
  log_event_epoll_create_t log_event_epoll_create;
  log_event_epoll_create1_t log_event_epoll_create1;
  log_event_epoll_ctl_t log_event_epoll_ctl;
  log_event_epoll_wait_t log_event_epoll_wait;
  log_event_getpwnam_r_t log_event_getpwnam_r;
  log_event_getpwuid_r_t log_event_getpwuid_r;
  log_event_getgrnam_r_t log_event_getgrnam_r;
  log_event_getgrgid_r_t log_event_getgrgid_r;
  log_event_getaddrinfo_t log_event_getaddrinfo;
  log_event_freeaddrinfo_t log_event_freeaddrinfo;
  log_event_getnameinfo_t log_event_getnameinfo;
  log_event_sendto_t log_event_sendto;
  log_event_sendmsg_t log_event_sendmsg;
  log_event_recvfrom_t log_event_recvfrom;
  log_event_recvmsg_t log_event_recvmsg;
  log_event_waitid_t log_event_waitid;
  log_event_wait4_t log_event_wait4;
  log_event_sigaction_t log_event_sigaction;
  log_event_signal_t log_event_signal;
  log_event_sigset_t log_event_sigset;
  log_event_fopen_t log_event_fopen;
  log_event_fopen64_t log_event_fopen64;
  log_event_freopen_t log_event_freopen;
  log_event_fclose_t log_event_fclose;
  log_event_fdopen_t log_event_fdopen;
  log_event_fgets_t log_event_fgets;
  log_event_ferror_t log_event_ferror;
  log_event_feof_t log_event_feof;
  log_event_fileno_t log_event_fileno;
  log_event_fflush_t log_event_fflush;
  log_event_setvbuf_t log_event_setvbuf;
  log_event_fseek_t log_event_fseek;
  log_event_fputs_t log_event_fputs;
  log_event_fputc_t log_event_fputc;
  log_event_fsync_t log_event_fsync;
  log_event_ftell_t log_event_ftell;
  log_event_fgetpos_t log_event_fgetpos;
  log_event_fgetpos64_t log_event_fgetpos64;
  log_event_fsetpos_t log_event_fsetpos;
  log_event_fsetpos64_t log_event_fsetpos64;
  log_event_fwrite_t log_event_fwrite;
  log_event_fread_t log_event_fread;
  log_event_getc_t log_event_getc;
  log_event_fgetc_t log_event_fgetc;
  log_event_ungetc_t log_event_ungetc;
  log_event_getline_t log_event_getline;
  log_event_getdelim_t log_event_getdelim;
  log_event_putc_t log_event_putc;
  log_event_rewind_t log_event_rewind;
  log_event_tmpfile_t log_event_tmpfile;
  log_event_flockfile_t log_event_flockfile;
  log_event_ftrylockfile_t log_event_ftrylockfile;
  log_event_funlockfile_t log_event_funlockfile;
  log_event_closedir_t log_event_closedir;
  log_event_opendir_t log_event_opendir;
  log_event_fdopendir_t log_event_fdopendir;
  log_event_readdir_t log_event_readdir;
  log_event_readdir_r_t log_event_readdir_r;
  log_event_pthread_cond_broadcast_t log_event_pthread_cond_broadcast;
  log_event_pthread_cond_signal_t log_event_pthread_cond_signal;
  log_event_pthread_cond_wait_t log_event_pthread_cond_wait;
  log_event_pthread_cond_timedwait_t log_event_pthread_cond_timedwait;
  log_event_pthread_cond_destroy_t log_event_pthread_cond_destroy;
  log_event_fxstat_t log_event_fxstat;
  log_event_fxstat64_t log_event_fxstat64;
  log_event_lxstat_t log_event_lxstat;
  log_event_lxstat64_t log_event_lxstat64;
  log_event_xstat_t log_event_xstat;
  log_event_xstat64_t log_event_xstat64;
  log_event_libc_memalign_t log_event_libc_memalign;
  log_event_vfprintf_t log_event_vfprintf;
  log_event_vfscanf_t log_event_vfscanf;
  log_event_exec_barrier_t log_event_exec_barrier;
  log_event_signal_handler_t log_event_signal_handler;
  log_event_user_t log_event_user;
  log_event_syscall_t log_event_syscall;
};


typedef struct {
  event_code_t event;
  unsigned char isOptional;
  log_off_t log_offset;
  clone_id_t clone_id;
  int my_errno;
  void* retval;
} log_entry_header_t;

typedef struct {
  // Shared among all events ("common area"):
  /* IMPORTANT: Adding new fields to the common area requires that you also
   * update the log_event_common_size definition. */
  log_entry_header_t header;

  union log_entry_data edata;
} log_entry_t;



int empty_turn_check(log_entry_t *e1, log_entry_t *e2);
int accept_turn_check(log_entry_t *e1, log_entry_t *e2);
int accept4_turn_check(log_entry_t *e1, log_entry_t *e2);
int access_turn_check(log_entry_t *e1, log_entry_t *e2);
int bind_turn_check(log_entry_t *e1, log_entry_t *e2);
int calloc_turn_check(log_entry_t *e1, log_entry_t *e2);
int chmod_turn_check(log_entry_t *e1, log_entry_t *e2);
int chown_turn_check(log_entry_t *e1, log_entry_t *e2);
int close_turn_check(log_entry_t *e1, log_entry_t *e2);
int connect_turn_check(log_entry_t *e1, log_entry_t *e2);
int dup_turn_check(log_entry_t *e1, log_entry_t *e2);
int dup2_turn_check(log_entry_t *e1, log_entry_t *e2);
int dup3_turn_check(log_entry_t *e1, log_entry_t *e2);
int fcntl_turn_check(log_entry_t *e1, log_entry_t *e2);
int fchdir_turn_check(log_entry_t *e1, log_entry_t *e2);
int fdatasync_turn_check(log_entry_t *e1, log_entry_t *e2);
int getcwd_turn_check(log_entry_t *e1, log_entry_t *e2);
int gettimeofday_turn_check(log_entry_t *e1, log_entry_t *e2);
int getpeername_turn_check(log_entry_t *e1, log_entry_t *e2);
int getsockname_turn_check(log_entry_t *e1, log_entry_t *e2);
int link_turn_check(log_entry_t *e1, log_entry_t *e2);
int symlink_turn_check(log_entry_t *e1, log_entry_t *e2);
int listen_turn_check(log_entry_t *e1, log_entry_t *e2);
int localtime_r_turn_check(log_entry_t *e1, log_entry_t *e2);
int utime_turn_check(log_entry_t *e1, log_entry_t *e2);
int utimes_turn_check(log_entry_t *e1, log_entry_t *e2);
int lutimes_turn_check(log_entry_t *e1, log_entry_t *e2);
int futimes_turn_check(log_entry_t *e1, log_entry_t *e2);
int clock_getres_turn_check(log_entry_t *e1, log_entry_t *e2);
int clock_gettime_turn_check(log_entry_t *e1, log_entry_t *e2);
int clock_settime_turn_check(log_entry_t *e1, log_entry_t *e2);
int lseek_turn_check(log_entry_t *e1, log_entry_t *e2);
int lseek64_turn_check(log_entry_t *e1, log_entry_t *e2);
int llseek_turn_check(log_entry_t *e1, log_entry_t *e2);
int malloc_turn_check(log_entry_t *e1, log_entry_t *e2);
int free_turn_check(log_entry_t *e1, log_entry_t *e2);
int mkdir_turn_check(log_entry_t *e1, log_entry_t *e2);
int mkstemp_turn_check(log_entry_t *e1, log_entry_t *e2);
int mmap_turn_check(log_entry_t *e1, log_entry_t *e2);
int mmap64_turn_check(log_entry_t *e1, log_entry_t *e2);
int munmap_turn_check(log_entry_t *e1, log_entry_t *e2);
int mremap_turn_check(log_entry_t *e1, log_entry_t *e2);
int open_turn_check(log_entry_t *e1, log_entry_t *e2);
int open64_turn_check(log_entry_t *e1, log_entry_t *e2);
int openat_turn_check(log_entry_t *e1, log_entry_t *e2);
int pread_turn_check(log_entry_t *e1, log_entry_t *e2);
int preadv_turn_check(log_entry_t *e1, log_entry_t *e2);
int pwrite_turn_check(log_entry_t *e1, log_entry_t *e2);
int pwritev_turn_check(log_entry_t *e1, log_entry_t *e2);
int pthread_rwlock_unlock_turn_check(log_entry_t *e1, log_entry_t *e2);
int pthread_rwlock_rdlock_turn_check(log_entry_t *e1, log_entry_t *e2);
int pthread_rwlock_wrlock_turn_check(log_entry_t *e1, log_entry_t *e2);
int pthread_create_turn_check(log_entry_t *e1, log_entry_t *e2);
int pthread_detach_turn_check(log_entry_t *e1, log_entry_t *e2);
int pthread_exit_turn_check(log_entry_t *e1, log_entry_t *e2);
int pthread_join_turn_check(log_entry_t *e1, log_entry_t *e2);
int pthread_kill_turn_check(log_entry_t *e1, log_entry_t *e2);
int pthread_mutex_lock_turn_check(log_entry_t *e1, log_entry_t *e2);
int pthread_mutex_trylock_turn_check(log_entry_t *e1, log_entry_t *e2);
int pthread_mutex_unlock_turn_check(log_entry_t *e1, log_entry_t *e2);
int rand_turn_check(log_entry_t *e1, log_entry_t *e2);
int read_turn_check(log_entry_t *e1, log_entry_t *e2);
int readv_turn_check(log_entry_t *e1, log_entry_t *e2);
int readlink_turn_check(log_entry_t *e1, log_entry_t *e2);
int realpath_turn_check(log_entry_t *e1, log_entry_t *e2);
int realloc_turn_check(log_entry_t *e1, log_entry_t *e2);
int rename_turn_check(log_entry_t *e1, log_entry_t *e2);
int rmdir_turn_check(log_entry_t *e1, log_entry_t *e2);
int select_turn_check(log_entry_t *e1, log_entry_t *e2);
int ppoll_turn_check(log_entry_t *e1, log_entry_t *e2);
int setsockopt_turn_check(log_entry_t *e1, log_entry_t *e2);
int getsockopt_turn_check(log_entry_t *e1, log_entry_t *e2);
int ioctl_turn_check(log_entry_t *e1, log_entry_t *e2);
int sigwait_turn_check(log_entry_t *e1, log_entry_t *e2);
int srand_turn_check(log_entry_t *e1, log_entry_t *e2);
int socket_turn_check(log_entry_t *e1, log_entry_t *e2);
int socketpair_turn_check(log_entry_t *e1, log_entry_t *e2);
int time_turn_check(log_entry_t *e1, log_entry_t *e2);
int truncate_turn_check(log_entry_t *e1, log_entry_t *e2);
int ftruncate_turn_check(log_entry_t *e1, log_entry_t *e2);
int truncate64_turn_check(log_entry_t *e1, log_entry_t *e2);
int ftruncate64_turn_check(log_entry_t *e1, log_entry_t *e2);
int unlink_turn_check(log_entry_t *e1, log_entry_t *e2);
int write_turn_check(log_entry_t *e1, log_entry_t *e2);
int writev_turn_check(log_entry_t *e1, log_entry_t *e2);
int epoll_create_turn_check(log_entry_t *e1, log_entry_t *e2);
int epoll_create1_turn_check(log_entry_t *e1, log_entry_t *e2);
int epoll_ctl_turn_check(log_entry_t *e1, log_entry_t *e2);
int epoll_wait_turn_check(log_entry_t *e1, log_entry_t *e2);
int getpwnam_r_turn_check(log_entry_t *e1, log_entry_t *e2);
int getpwuid_r_turn_check(log_entry_t *e1, log_entry_t *e2);
int getgrnam_r_turn_check(log_entry_t *e1, log_entry_t *e2);
int getgrgid_r_turn_check(log_entry_t *e1, log_entry_t *e2);
int getaddrinfo_turn_check(log_entry_t *e1, log_entry_t *e2);
int freeaddrinfo_turn_check(log_entry_t *e1, log_entry_t *e2);
int getnameinfo_turn_check(log_entry_t *e1, log_entry_t *e2);
int sendto_turn_check(log_entry_t *e1, log_entry_t *e2);
int sendmsg_turn_check(log_entry_t *e1, log_entry_t *e2);
int recvfrom_turn_check(log_entry_t *e1, log_entry_t *e2);
int recvmsg_turn_check(log_entry_t *e1, log_entry_t *e2);
int waitid_turn_check(log_entry_t *e1, log_entry_t *e2);
int wait4_turn_check(log_entry_t *e1, log_entry_t *e2);
int sigaction_turn_check(log_entry_t *e1, log_entry_t *e2);
int signal_turn_check(log_entry_t *e1, log_entry_t *e2);
int sigset_turn_check(log_entry_t *e1, log_entry_t *e2);
int fopen_turn_check(log_entry_t *e1, log_entry_t *e2);
int fopen64_turn_check(log_entry_t *e1, log_entry_t *e2);
int freopen_turn_check(log_entry_t *e1, log_entry_t *e2);
int fclose_turn_check(log_entry_t *e1, log_entry_t *e2);
int fdopen_turn_check(log_entry_t *e1, log_entry_t *e2);
int fgets_turn_check(log_entry_t *e1, log_entry_t *e2);
int ferror_turn_check(log_entry_t *e1, log_entry_t *e2);
int feof_turn_check(log_entry_t *e1, log_entry_t *e2);
int fileno_turn_check(log_entry_t *e1, log_entry_t *e2);
int fflush_turn_check(log_entry_t *e1, log_entry_t *e2);
int setvbuf_turn_check(log_entry_t *e1, log_entry_t *e2);
int fseek_turn_check(log_entry_t *e1, log_entry_t *e2);
int fputs_turn_check(log_entry_t *e1, log_entry_t *e2);
int fputc_turn_check(log_entry_t *e1, log_entry_t *e2);
int fsync_turn_check(log_entry_t *e1, log_entry_t *e2);
int ftell_turn_check(log_entry_t *e1, log_entry_t *e2);
int fgetpos_turn_check(log_entry_t *e1, log_entry_t *e2);
int fgetpos64_turn_check(log_entry_t *e1, log_entry_t *e2);
int fsetpos_turn_check(log_entry_t *e1, log_entry_t *e2);
int fsetpos64_turn_check(log_entry_t *e1, log_entry_t *e2);
int fwrite_turn_check(log_entry_t *e1, log_entry_t *e2);
int fread_turn_check(log_entry_t *e1, log_entry_t *e2);
int getc_turn_check(log_entry_t *e1, log_entry_t *e2);
int fgetc_turn_check(log_entry_t *e1, log_entry_t *e2);
int ungetc_turn_check(log_entry_t *e1, log_entry_t *e2);
int getline_turn_check(log_entry_t *e1, log_entry_t *e2);
int getdelim_turn_check(log_entry_t *e1, log_entry_t *e2);
int putc_turn_check(log_entry_t *e1, log_entry_t *e2);
int rewind_turn_check(log_entry_t *e1, log_entry_t *e2);
int tmpfile_turn_check(log_entry_t *e1, log_entry_t *e2);
int flockfile_turn_check(log_entry_t *e1, log_entry_t *e2);
int ftrylockfile_turn_check(log_entry_t *e1, log_entry_t *e2);
int funlockfile_turn_check(log_entry_t *e1, log_entry_t *e2);
int closedir_turn_check(log_entry_t *e1, log_entry_t *e2);
int opendir_turn_check(log_entry_t *e1, log_entry_t *e2);
int fdopendir_turn_check(log_entry_t *e1, log_entry_t *e2);
int readdir_turn_check(log_entry_t *e1, log_entry_t *e2);
int readdir_r_turn_check(log_entry_t *e1, log_entry_t *e2);
int pthread_cond_broadcast_turn_check(log_entry_t *e1, log_entry_t *e2);
int pthread_cond_signal_turn_check(log_entry_t *e1, log_entry_t *e2);
int pthread_cond_wait_turn_check(log_entry_t *e1, log_entry_t *e2);
int pthread_cond_timedwait_turn_check(log_entry_t *e1, log_entry_t *e2);
int pthread_cond_destroy_turn_check(log_entry_t *e1, log_entry_t *e2);
int fxstat_turn_check(log_entry_t *e1, log_entry_t *e2);
int fxstat64_turn_check(log_entry_t *e1, log_entry_t *e2);
int lxstat_turn_check(log_entry_t *e1, log_entry_t *e2);
int lxstat64_turn_check(log_entry_t *e1, log_entry_t *e2);
int xstat_turn_check(log_entry_t *e1, log_entry_t *e2);
int xstat64_turn_check(log_entry_t *e1, log_entry_t *e2);
int libc_memalign_turn_check(log_entry_t *e1, log_entry_t *e2);
int vfprintf_turn_check(log_entry_t *e1, log_entry_t *e2);
int vfscanf_turn_check(log_entry_t *e1, log_entry_t *e2);
int exec_barrier_turn_check(log_entry_t *e1, log_entry_t *e2);
int signal_handler_turn_check(log_entry_t *e1, log_entry_t *e2);
int user_turn_check(log_entry_t *e1, log_entry_t *e2);
int syscall_turn_check(log_entry_t *e1, log_entry_t *e2);

log_entry_t create_empty_entry(clone_id_t clone_id, event_code_t event);
log_entry_t create_accept_entry(clone_id_t clone_id, event_code_t event,
                                int sockfd, struct sockaddr* addr, socklen_t* addrlen);
log_entry_t create_accept4_entry(clone_id_t clone_id, event_code_t event,
                                 int sockfd, struct sockaddr* addr, socklen_t* addrlen, int flags);
log_entry_t create_access_entry(clone_id_t clone_id, event_code_t event,
                                const char* pathname, int mode);
log_entry_t create_bind_entry(clone_id_t clone_id, event_code_t event,
                              int sockfd, const struct sockaddr* my_addr, socklen_t addrlen);
log_entry_t create_calloc_entry(clone_id_t clone_id, event_code_t event,
                                size_t nmemb, size_t size);
log_entry_t create_chmod_entry(clone_id_t clone_id, event_code_t event,
                               const char* path, mode_t mode);
log_entry_t create_chown_entry(clone_id_t clone_id, event_code_t event,
                               const char* path, uid_t owner, gid_t group);
log_entry_t create_close_entry(clone_id_t clone_id, event_code_t event,
                               int fd);
log_entry_t create_connect_entry(clone_id_t clone_id, event_code_t event,
                                 int sockfd, const struct sockaddr* serv_addr, socklen_t addrlen);
log_entry_t create_dup_entry(clone_id_t clone_id, event_code_t event,
                             int oldfd);
log_entry_t create_dup2_entry(clone_id_t clone_id, event_code_t event,
                              int oldfd, int newfd);
log_entry_t create_dup3_entry(clone_id_t clone_id, event_code_t event,
                              int oldfd, int newfd, int flags);
log_entry_t create_fcntl_entry(clone_id_t clone_id, event_code_t event,
                               int fd, int cmd, void* arg);
log_entry_t create_fchdir_entry(clone_id_t clone_id, event_code_t event,
                                int fd);
log_entry_t create_fdatasync_entry(clone_id_t clone_id, event_code_t event,
                                   int fd);
log_entry_t create_getcwd_entry(clone_id_t clone_id, event_code_t event,
                                char* buf, size_t size);
log_entry_t create_gettimeofday_entry(clone_id_t clone_id, event_code_t event,
                                      struct timeval* tv, struct timezone* tz);
log_entry_t create_getpeername_entry(clone_id_t clone_id, event_code_t event,
                                     int sockfd, struct sockaddr* addr, socklen_t* addrlen);
log_entry_t create_getsockname_entry(clone_id_t clone_id, event_code_t event,
                                     int sockfd, struct sockaddr* addr, socklen_t* addrlen);
log_entry_t create_link_entry(clone_id_t clone_id, event_code_t event,
                              const char* oldpath, const char* newpath);
log_entry_t create_symlink_entry(clone_id_t clone_id, event_code_t event,
                                 const char* oldpath, const char* newpath);
log_entry_t create_listen_entry(clone_id_t clone_id, event_code_t event,
                                int sockfd, int backlog);
log_entry_t create_localtime_r_entry(clone_id_t clone_id, event_code_t event,
                                     const time_t* timep, struct tm* result);
log_entry_t create_utime_entry(clone_id_t clone_id, event_code_t event,
                               const char* filename, const struct utimbuf* times);
log_entry_t create_utimes_entry(clone_id_t clone_id, event_code_t event,
                                const char* filename, const struct timeval* times);
log_entry_t create_lutimes_entry(clone_id_t clone_id, event_code_t event,
                                 const char* filename, const struct timeval* tv);
log_entry_t create_futimes_entry(clone_id_t clone_id, event_code_t event,
                                 int fd, const struct timeval* tv);
log_entry_t create_clock_getres_entry(clone_id_t clone_id, event_code_t event,
                                      clockid_t clk_id, struct timespec* res);
log_entry_t create_clock_gettime_entry(clone_id_t clone_id, event_code_t event,
                                       clockid_t clk_id, struct timespec* tp);
log_entry_t create_clock_settime_entry(clone_id_t clone_id, event_code_t event,
                                       clockid_t clk_id, const struct timespec* tp);
log_entry_t create_lseek_entry(clone_id_t clone_id, event_code_t event,
                               int fd, off_t offset, int whence);
log_entry_t create_lseek64_entry(clone_id_t clone_id, event_code_t event,
                                 int fd, off64_t offset, int whence);
log_entry_t create_llseek_entry(clone_id_t clone_id, event_code_t event,
                                int fd, loff_t offset, int whence);
log_entry_t create_malloc_entry(clone_id_t clone_id, event_code_t event,
                                size_t size);
log_entry_t create_free_entry(clone_id_t clone_id, event_code_t event,
                              void* ptr);
log_entry_t create_mkdir_entry(clone_id_t clone_id, event_code_t event,
                               const char* pathname, mode_t mode);
log_entry_t create_mkstemp_entry(clone_id_t clone_id, event_code_t event,
                                 char* temp);
log_entry_t create_mmap_entry(clone_id_t clone_id, event_code_t event,
                              void* addr, size_t length, int prot, int flags, int fd, off_t offset);
log_entry_t create_mmap64_entry(clone_id_t clone_id, event_code_t event,
                                void* addr, size_t length, int prot, int flags, int fd, off64_t offset);
log_entry_t create_munmap_entry(clone_id_t clone_id, event_code_t event,
                                void* addr, size_t length);
log_entry_t create_mremap_entry(clone_id_t clone_id, event_code_t event,
                                void* old_address, size_t old_size, size_t new_size, int flags, void* new_address);
log_entry_t create_open_entry(clone_id_t clone_id, event_code_t event,
                              const char* pathname, int flags, mode_t mode);
log_entry_t create_open64_entry(clone_id_t clone_id, event_code_t event,
                                const char* pathname, int flags, mode_t mode);
log_entry_t create_openat_entry(clone_id_t clone_id, event_code_t event,
                                int dirfd, const char* pathname, int flags);
log_entry_t create_pread_entry(clone_id_t clone_id, event_code_t event,
                               int fd, void* buf, size_t count, off_t offset);
log_entry_t create_preadv_entry(clone_id_t clone_id, event_code_t event,
                                int fd, const struct iovec* iov, int iovcnt, off_t offset);
log_entry_t create_pwrite_entry(clone_id_t clone_id, event_code_t event,
                                int fd, const void* buf, size_t count, off_t offset);
log_entry_t create_pwritev_entry(clone_id_t clone_id, event_code_t event,
                                 int fd, const struct iovec* iov, int iovcnt, off_t offset);
log_entry_t create_pthread_rwlock_unlock_entry(clone_id_t clone_id, event_code_t event,
                                               pthread_rwlock_t* rwlock);
log_entry_t create_pthread_rwlock_rdlock_entry(clone_id_t clone_id, event_code_t event,
                                               pthread_rwlock_t* rwlock);
log_entry_t create_pthread_rwlock_wrlock_entry(clone_id_t clone_id, event_code_t event,
                                               pthread_rwlock_t* rwlock);
log_entry_t create_pthread_create_entry(clone_id_t clone_id, event_code_t event,
                                        pthread_t* thread, const pthread_attr_t* attr, pthread_start_routine_t start_routine, void* arg);
log_entry_t create_pthread_detach_entry(clone_id_t clone_id, event_code_t event,
                                        pthread_t thread);
log_entry_t create_pthread_exit_entry(clone_id_t clone_id, event_code_t event,
                                      void* value_ptr);
log_entry_t create_pthread_join_entry(clone_id_t clone_id, event_code_t event,
                                      pthread_t thread, void** value_ptr);
log_entry_t create_pthread_kill_entry(clone_id_t clone_id, event_code_t event,
                                      pthread_t thread, int sig);
log_entry_t create_pthread_mutex_lock_entry(clone_id_t clone_id, event_code_t event,
                                            pthread_mutex_t* mutex);
log_entry_t create_pthread_mutex_trylock_entry(clone_id_t clone_id, event_code_t event,
                                               pthread_mutex_t* mutex);
log_entry_t create_pthread_mutex_unlock_entry(clone_id_t clone_id, event_code_t event,
                                              pthread_mutex_t* mutex);
log_entry_t create_rand_entry(clone_id_t clone_id, event_code_t event);
log_entry_t create_read_entry(clone_id_t clone_id, event_code_t event,
                              int fd, void* buf, size_t count);
log_entry_t create_readv_entry(clone_id_t clone_id, event_code_t event,
                               int fd, const struct iovec* iov, int iovcnt);
log_entry_t create_readlink_entry(clone_id_t clone_id, event_code_t event,
                                  const char* path, char* buf, size_t bufsiz);
log_entry_t create_realpath_entry(clone_id_t clone_id, event_code_t event,
                                  const char* path, char* resolved_path);
log_entry_t create_realloc_entry(clone_id_t clone_id, event_code_t event,
                                 void* ptr, size_t size);
log_entry_t create_rename_entry(clone_id_t clone_id, event_code_t event,
                                const char* oldpath, const char* newpath);
log_entry_t create_rmdir_entry(clone_id_t clone_id, event_code_t event,
                               const char* pathname);
log_entry_t create_select_entry(clone_id_t clone_id, event_code_t event,
                                int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout);
log_entry_t create_ppoll_entry(clone_id_t clone_id, event_code_t event,
                               struct pollfd* fds, nfds_t nfds, const struct timespec* timeout_ts, const sigset_t* sigmask);
log_entry_t create_setsockopt_entry(clone_id_t clone_id, event_code_t event,
                                    int s, int level, int optname, const void* optval, socklen_t optlen);
log_entry_t create_getsockopt_entry(clone_id_t clone_id, event_code_t event,
                                    int s, int level, int optname, void* optval, socklen_t* optlen);
log_entry_t create_ioctl_entry(clone_id_t clone_id, event_code_t event,
                               int d, int request, void* arg);
log_entry_t create_sigwait_entry(clone_id_t clone_id, event_code_t event,
                                 const sigset_t* set, int* sig);
log_entry_t create_srand_entry(clone_id_t clone_id, event_code_t event,
                               unsigned int seed);
log_entry_t create_socket_entry(clone_id_t clone_id, event_code_t event,
                                int domain, int type, int protocol);
log_entry_t create_socketpair_entry(clone_id_t clone_id, event_code_t event,
                                    int d, int type, int protocol, int* sv);
log_entry_t create_time_entry(clone_id_t clone_id, event_code_t event,
                              time_t* tloc);
log_entry_t create_truncate_entry(clone_id_t clone_id, event_code_t event,
                                  const char* path, off_t length);
log_entry_t create_ftruncate_entry(clone_id_t clone_id, event_code_t event,
                                   int fd, off_t length);
log_entry_t create_truncate64_entry(clone_id_t clone_id, event_code_t event,
                                    const char* path, off64_t length);
log_entry_t create_ftruncate64_entry(clone_id_t clone_id, event_code_t event,
                                     int fd, off64_t length);
log_entry_t create_unlink_entry(clone_id_t clone_id, event_code_t event,
                                const char* pathname);
log_entry_t create_write_entry(clone_id_t clone_id, event_code_t event,
                               int fd, const void* buf, size_t count);
log_entry_t create_writev_entry(clone_id_t clone_id, event_code_t event,
                                int fd, const struct iovec* iov, int iovcnt);
log_entry_t create_epoll_create_entry(clone_id_t clone_id, event_code_t event,
                                      int size);
log_entry_t create_epoll_create1_entry(clone_id_t clone_id, event_code_t event,
                                       int flags);
log_entry_t create_epoll_ctl_entry(clone_id_t clone_id, event_code_t event,
                                   int epfd, int op, int fd, struct epoll_event* ep);
log_entry_t create_epoll_wait_entry(clone_id_t clone_id, event_code_t event,
                                    int epfd, struct epoll_event* events, int maxevents, int timeout);
log_entry_t create_getpwnam_r_entry(clone_id_t clone_id, event_code_t event,
                                    const char* name, struct passwd* pwd, char* buf, size_t buflen, struct passwd** result);
log_entry_t create_getpwuid_r_entry(clone_id_t clone_id, event_code_t event,
                                    uid_t uid, struct passwd* pwd, char* buf, size_t buflen, struct passwd** result);
log_entry_t create_getgrnam_r_entry(clone_id_t clone_id, event_code_t event,
                                    const char* name, struct group* grp, char* buf, size_t buflen, struct group** result);
log_entry_t create_getgrgid_r_entry(clone_id_t clone_id, event_code_t event,
                                    gid_t gid, struct group* grp, char* buf, size_t buflen, struct group** result);
log_entry_t create_getaddrinfo_entry(clone_id_t clone_id, event_code_t event,
                                     const char* node, const char* service, const struct addrinfo* hints, struct addrinfo** res);
log_entry_t create_freeaddrinfo_entry(clone_id_t clone_id, event_code_t event,
                                      struct addrinfo* res);
log_entry_t create_getnameinfo_entry(clone_id_t clone_id, event_code_t event,
                                     const struct sockaddr* sa, socklen_t salen, char* host, size_t hostlen, char* serv, size_t servlen, int flags);
log_entry_t create_sendto_entry(clone_id_t clone_id, event_code_t event,
                                int sockfd, const void* buf, size_t len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen);
log_entry_t create_sendmsg_entry(clone_id_t clone_id, event_code_t event,
                                 int sockfd, const struct msghdr* msg, int flags);
log_entry_t create_recvfrom_entry(clone_id_t clone_id, event_code_t event,
                                  int sockfd, void* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen);
log_entry_t create_recvmsg_entry(clone_id_t clone_id, event_code_t event,
                                 int sockfd, struct msghdr* msg, int flags);
log_entry_t create_waitid_entry(clone_id_t clone_id, event_code_t event,
                                idtype_t idtype, id_t id, siginfo_t* infop, int options);
log_entry_t create_wait4_entry(clone_id_t clone_id, event_code_t event,
                               pid_t pid, __WAIT_STATUS status, int options, struct rusage* rusage);
log_entry_t create_sigaction_entry(clone_id_t clone_id, event_code_t event,
                                   int signum, const struct sigaction* act, struct sigaction* oldact);
log_entry_t create_signal_entry(clone_id_t clone_id, event_code_t event,
                                int signum, sighandler_t handler);
log_entry_t create_sigset_entry(clone_id_t clone_id, event_code_t event,
                                int sig, sighandler_t disp);
log_entry_t create_fopen_entry(clone_id_t clone_id, event_code_t event,
                               const char* path, const char* mode);
log_entry_t create_fopen64_entry(clone_id_t clone_id, event_code_t event,
                                 const char* path, const char* mode);
log_entry_t create_freopen_entry(clone_id_t clone_id, event_code_t event,
                                 const char* path, const char* mode, FILE* stream);
log_entry_t create_fclose_entry(clone_id_t clone_id, event_code_t event,
                                FILE* fp);
log_entry_t create_fdopen_entry(clone_id_t clone_id, event_code_t event,
                                int fd, const char* mode);
log_entry_t create_fgets_entry(clone_id_t clone_id, event_code_t event,
                               char* s, int size, FILE* stream);
log_entry_t create_ferror_entry(clone_id_t clone_id, event_code_t event,
                                FILE* stream);
log_entry_t create_feof_entry(clone_id_t clone_id, event_code_t event,
                              FILE* stream);
log_entry_t create_fileno_entry(clone_id_t clone_id, event_code_t event,
                                FILE* stream);
log_entry_t create_fflush_entry(clone_id_t clone_id, event_code_t event,
                                FILE* stream);
log_entry_t create_setvbuf_entry(clone_id_t clone_id, event_code_t event,
                                 FILE* stream, char* buf, int mode, size_t size);
log_entry_t create_fseek_entry(clone_id_t clone_id, event_code_t event,
                               FILE* stream, long offset, int whence);
log_entry_t create_fputs_entry(clone_id_t clone_id, event_code_t event,
                               const char* s, FILE* stream);
log_entry_t create_fputc_entry(clone_id_t clone_id, event_code_t event,
                               int c, FILE* stream);
log_entry_t create_fsync_entry(clone_id_t clone_id, event_code_t event,
                               int fd);
log_entry_t create_ftell_entry(clone_id_t clone_id, event_code_t event,
                               FILE* stream);
log_entry_t create_fgetpos_entry(clone_id_t clone_id, event_code_t event,
                                 FILE* stream, fpos_t* pos);
log_entry_t create_fgetpos64_entry(clone_id_t clone_id, event_code_t event,
                                   FILE* stream, fpos64_t* pos);
log_entry_t create_fsetpos_entry(clone_id_t clone_id, event_code_t event,
                                 FILE* stream, const fpos_t* pos);
log_entry_t create_fsetpos64_entry(clone_id_t clone_id, event_code_t event,
                                   FILE* stream, const fpos64_t* pos);
log_entry_t create_fwrite_entry(clone_id_t clone_id, event_code_t event,
                                const void* ptr, size_t size, size_t nmemb, FILE* stream);
log_entry_t create_fread_entry(clone_id_t clone_id, event_code_t event,
                               void* ptr, size_t size, size_t nmemb, FILE* stream);
log_entry_t create_getc_entry(clone_id_t clone_id, event_code_t event,
                              FILE* stream);
log_entry_t create_fgetc_entry(clone_id_t clone_id, event_code_t event,
                               FILE* stream);
log_entry_t create_ungetc_entry(clone_id_t clone_id, event_code_t event,
                                int c, FILE* stream);
log_entry_t create_getline_entry(clone_id_t clone_id, event_code_t event,
                                 char** lineptr, size_t* n, FILE* stream);
log_entry_t create_getdelim_entry(clone_id_t clone_id, event_code_t event,
                                  char** lineptr, size_t* n, int delim, FILE* stream);
log_entry_t create_putc_entry(clone_id_t clone_id, event_code_t event,
                              int c, FILE* stream);
log_entry_t create_rewind_entry(clone_id_t clone_id, event_code_t event,
                                FILE* stream);
log_entry_t create_tmpfile_entry(clone_id_t clone_id, event_code_t event);
log_entry_t create_flockfile_entry(clone_id_t clone_id, event_code_t event,
                                   FILE* filehandle);
log_entry_t create_ftrylockfile_entry(clone_id_t clone_id, event_code_t event,
                                      FILE* filehandle);
log_entry_t create_funlockfile_entry(clone_id_t clone_id, event_code_t event,
                                     FILE* filehandle);
log_entry_t create_closedir_entry(clone_id_t clone_id, event_code_t event,
                                  DIR* dirp);
log_entry_t create_opendir_entry(clone_id_t clone_id, event_code_t event,
                                 const char* name);
log_entry_t create_fdopendir_entry(clone_id_t clone_id, event_code_t event,
                                   int fd);
log_entry_t create_readdir_entry(clone_id_t clone_id, event_code_t event,
                                 DIR* dirp);
log_entry_t create_readdir_r_entry(clone_id_t clone_id, event_code_t event,
                                   DIR* dirp, struct dirent* entry, struct dirent** result);
log_entry_t create_pthread_cond_broadcast_entry(clone_id_t clone_id, event_code_t event,
                                                pthread_cond_t* cond);
log_entry_t create_pthread_cond_signal_entry(clone_id_t clone_id, event_code_t event,
                                             pthread_cond_t* cond);
log_entry_t create_pthread_cond_wait_entry(clone_id_t clone_id, event_code_t event,
                                           pthread_cond_t* cond, pthread_mutex_t* mutex);
log_entry_t create_pthread_cond_timedwait_entry(clone_id_t clone_id, event_code_t event,
                                                pthread_cond_t* cond, pthread_mutex_t* mutex, const struct timespec* abstime);
log_entry_t create_pthread_cond_destroy_entry(clone_id_t clone_id, event_code_t event,
                                              pthread_cond_t* cond);
log_entry_t create_fxstat_entry(clone_id_t clone_id, event_code_t event,
                                int vers, int fd, struct stat* buf);
log_entry_t create_fxstat64_entry(clone_id_t clone_id, event_code_t event,
                                  int vers, int fd, struct stat64* buf);
log_entry_t create_lxstat_entry(clone_id_t clone_id, event_code_t event,
                                int vers, const char* path, struct stat* buf);
log_entry_t create_lxstat64_entry(clone_id_t clone_id, event_code_t event,
                                  int vers, const char* path, struct stat64* buf);
log_entry_t create_xstat_entry(clone_id_t clone_id, event_code_t event,
                               int vers, const char* path, struct stat* buf);
log_entry_t create_xstat64_entry(clone_id_t clone_id, event_code_t event,
                                 int vers, const char* path, struct stat64* buf);
log_entry_t create_libc_memalign_entry(clone_id_t clone_id, event_code_t event,
                                       size_t boundary, size_t size);
log_entry_t create_vfprintf_entry(clone_id_t clone_id, event_code_t event,
                                  FILE* stream, const char* format, va_list ap);
log_entry_t create_vfscanf_entry(clone_id_t clone_id, event_code_t event,
                                 FILE* stream, const char* format, va_list ap);
log_entry_t create_exec_barrier_entry(clone_id_t clone_id, event_code_t event);
log_entry_t create_signal_handler_entry(clone_id_t clone_id, event_code_t event,
                                        int sig, siginfo_t* info, void* data);
log_entry_t create_user_entry(clone_id_t clone_id, event_code_t event);
log_entry_t create_syscall_entry(clone_id_t clone_id, event_code_t event,
                                 int num, void* a1, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7);
size_t getLogEventSize(const log_entry_t *entry);

#ifdef __cplusplus
}
#endif
#endif