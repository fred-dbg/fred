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

#include "fred_wrappers.h"

void * _real_dlsym(void *handle, const char *symbol);
extern LIB_PRIVATE void *_real_func_addr[];

LIB_PRIVATE void fred_get_libc_func_addr() {
  _real_func_addr[empty_event] = _real_dlsym(RTLD_NEXT, "empty");
  _real_func_addr[accept_event] = _real_dlsym(RTLD_NEXT, "accept");
  _real_func_addr[accept4_event] = _real_dlsym(RTLD_NEXT, "accept4");
  _real_func_addr[access_event] = _real_dlsym(RTLD_NEXT, "access");
  _real_func_addr[bind_event] = _real_dlsym(RTLD_NEXT, "bind");
  _real_func_addr[calloc_event] = _real_dlsym(RTLD_NEXT, "calloc");
  _real_func_addr[chmod_event] = _real_dlsym(RTLD_NEXT, "chmod");
  _real_func_addr[chown_event] = _real_dlsym(RTLD_NEXT, "chown");
  _real_func_addr[close_event] = _real_dlsym(RTLD_NEXT, "close");
  _real_func_addr[connect_event] = _real_dlsym(RTLD_NEXT, "connect");
  _real_func_addr[dup_event] = _real_dlsym(RTLD_NEXT, "dup");
  _real_func_addr[dup2_event] = _real_dlsym(RTLD_NEXT, "dup2");
  _real_func_addr[dup3_event] = _real_dlsym(RTLD_NEXT, "dup3");
  _real_func_addr[fcntl_event] = _real_dlsym(RTLD_NEXT, "fcntl");
  _real_func_addr[fchdir_event] = _real_dlsym(RTLD_NEXT, "fchdir");
  _real_func_addr[fdatasync_event] = _real_dlsym(RTLD_NEXT, "fdatasync");
  _real_func_addr[getcwd_event] = _real_dlsym(RTLD_NEXT, "getcwd");
  _real_func_addr[gettimeofday_event] = _real_dlsym(RTLD_NEXT, "gettimeofday");
  _real_func_addr[getpeername_event] = _real_dlsym(RTLD_NEXT, "getpeername");
  _real_func_addr[getsockname_event] = _real_dlsym(RTLD_NEXT, "getsockname");
  _real_func_addr[link_event] = _real_dlsym(RTLD_NEXT, "link");
  _real_func_addr[symlink_event] = _real_dlsym(RTLD_NEXT, "symlink");
  _real_func_addr[listen_event] = _real_dlsym(RTLD_NEXT, "listen");
  _real_func_addr[localtime_r_event] = _real_dlsym(RTLD_NEXT, "localtime_r");
  _real_func_addr[utime_event] = _real_dlsym(RTLD_NEXT, "utime");
  _real_func_addr[utimes_event] = _real_dlsym(RTLD_NEXT, "utimes");
  _real_func_addr[lutimes_event] = _real_dlsym(RTLD_NEXT, "lutimes");
  _real_func_addr[futimes_event] = _real_dlsym(RTLD_NEXT, "futimes");
  _real_func_addr[clock_getres_event] = _real_dlsym(RTLD_NEXT, "clock_getres");
  _real_func_addr[clock_gettime_event] = _real_dlsym(RTLD_NEXT, "clock_gettime");
  _real_func_addr[clock_settime_event] = _real_dlsym(RTLD_NEXT, "clock_settime");
  _real_func_addr[lseek_event] = _real_dlsym(RTLD_NEXT, "lseek");
  _real_func_addr[lseek64_event] = _real_dlsym(RTLD_NEXT, "lseek64");
  _real_func_addr[llseek_event] = _real_dlsym(RTLD_NEXT, "llseek");
  _real_func_addr[malloc_event] = _real_dlsym(RTLD_NEXT, "malloc");
  _real_func_addr[free_event] = _real_dlsym(RTLD_NEXT, "free");
  _real_func_addr[mkdir_event] = _real_dlsym(RTLD_NEXT, "mkdir");
  _real_func_addr[mkstemp_event] = _real_dlsym(RTLD_NEXT, "mkstemp");
  _real_func_addr[mmap_event] = _real_dlsym(RTLD_NEXT, "mmap");
  _real_func_addr[mmap64_event] = _real_dlsym(RTLD_NEXT, "mmap64");
  _real_func_addr[munmap_event] = _real_dlsym(RTLD_NEXT, "munmap");
  _real_func_addr[mremap_event] = _real_dlsym(RTLD_NEXT, "mremap");
  _real_func_addr[open_event] = _real_dlsym(RTLD_NEXT, "open");
  _real_func_addr[open64_event] = _real_dlsym(RTLD_NEXT, "open64");
  _real_func_addr[openat_event] = _real_dlsym(RTLD_NEXT, "openat");
  _real_func_addr[pread_event] = _real_dlsym(RTLD_NEXT, "pread");
  _real_func_addr[preadv_event] = _real_dlsym(RTLD_NEXT, "preadv");
  _real_func_addr[pwrite_event] = _real_dlsym(RTLD_NEXT, "pwrite");
  _real_func_addr[pwritev_event] = _real_dlsym(RTLD_NEXT, "pwritev");
  _real_func_addr[pthread_rwlock_unlock_event] = _real_dlsym(RTLD_NEXT, "pthread_rwlock_unlock");
  _real_func_addr[pthread_rwlock_rdlock_event] = _real_dlsym(RTLD_NEXT, "pthread_rwlock_rdlock");
  _real_func_addr[pthread_rwlock_wrlock_event] = _real_dlsym(RTLD_NEXT, "pthread_rwlock_wrlock");
  _real_func_addr[pthread_create_event] = _real_dlsym(RTLD_NEXT, "pthread_create");
  _real_func_addr[pthread_detach_event] = _real_dlsym(RTLD_NEXT, "pthread_detach");
  _real_func_addr[pthread_exit_event] = _real_dlsym(RTLD_NEXT, "pthread_exit");
  _real_func_addr[pthread_join_event] = _real_dlsym(RTLD_NEXT, "pthread_join");
  _real_func_addr[pthread_kill_event] = _real_dlsym(RTLD_NEXT, "pthread_kill");
  _real_func_addr[pthread_mutex_lock_event] = _real_dlsym(RTLD_NEXT, "pthread_mutex_lock");
  _real_func_addr[pthread_mutex_trylock_event] = _real_dlsym(RTLD_NEXT, "pthread_mutex_trylock");
  _real_func_addr[pthread_mutex_unlock_event] = _real_dlsym(RTLD_NEXT, "pthread_mutex_unlock");
  _real_func_addr[rand_event] = _real_dlsym(RTLD_NEXT, "rand");
  _real_func_addr[read_event] = _real_dlsym(RTLD_NEXT, "read");
  _real_func_addr[readv_event] = _real_dlsym(RTLD_NEXT, "readv");
  _real_func_addr[readlink_event] = _real_dlsym(RTLD_NEXT, "readlink");
  _real_func_addr[realpath_event] = _real_dlsym(RTLD_NEXT, "realpath");
  _real_func_addr[realloc_event] = _real_dlsym(RTLD_NEXT, "realloc");
  _real_func_addr[rename_event] = _real_dlsym(RTLD_NEXT, "rename");
  _real_func_addr[rmdir_event] = _real_dlsym(RTLD_NEXT, "rmdir");
  _real_func_addr[select_event] = _real_dlsym(RTLD_NEXT, "select");
  _real_func_addr[ppoll_event] = _real_dlsym(RTLD_NEXT, "ppoll");
  _real_func_addr[setsockopt_event] = _real_dlsym(RTLD_NEXT, "setsockopt");
  _real_func_addr[getsockopt_event] = _real_dlsym(RTLD_NEXT, "getsockopt");
  _real_func_addr[ioctl_event] = _real_dlsym(RTLD_NEXT, "ioctl");
  _real_func_addr[sigwait_event] = _real_dlsym(RTLD_NEXT, "sigwait");
  _real_func_addr[srand_event] = _real_dlsym(RTLD_NEXT, "srand");
  _real_func_addr[socket_event] = _real_dlsym(RTLD_NEXT, "socket");
  _real_func_addr[socketpair_event] = _real_dlsym(RTLD_NEXT, "socketpair");
  _real_func_addr[time_event] = _real_dlsym(RTLD_NEXT, "time");
  _real_func_addr[truncate_event] = _real_dlsym(RTLD_NEXT, "truncate");
  _real_func_addr[ftruncate_event] = _real_dlsym(RTLD_NEXT, "ftruncate");
  _real_func_addr[truncate64_event] = _real_dlsym(RTLD_NEXT, "truncate64");
  _real_func_addr[ftruncate64_event] = _real_dlsym(RTLD_NEXT, "ftruncate64");
  _real_func_addr[unlink_event] = _real_dlsym(RTLD_NEXT, "unlink");
  _real_func_addr[write_event] = _real_dlsym(RTLD_NEXT, "write");
  _real_func_addr[writev_event] = _real_dlsym(RTLD_NEXT, "writev");
  _real_func_addr[epoll_create_event] = _real_dlsym(RTLD_NEXT, "epoll_create");
  _real_func_addr[epoll_create1_event] = _real_dlsym(RTLD_NEXT, "epoll_create1");
  _real_func_addr[epoll_ctl_event] = _real_dlsym(RTLD_NEXT, "epoll_ctl");
  _real_func_addr[epoll_wait_event] = _real_dlsym(RTLD_NEXT, "epoll_wait");
  _real_func_addr[getpwnam_r_event] = _real_dlsym(RTLD_NEXT, "getpwnam_r");
  _real_func_addr[getpwuid_r_event] = _real_dlsym(RTLD_NEXT, "getpwuid_r");
  _real_func_addr[getgrnam_r_event] = _real_dlsym(RTLD_NEXT, "getgrnam_r");
  _real_func_addr[getgrgid_r_event] = _real_dlsym(RTLD_NEXT, "getgrgid_r");
  _real_func_addr[getaddrinfo_event] = _real_dlsym(RTLD_NEXT, "getaddrinfo");
  _real_func_addr[freeaddrinfo_event] = _real_dlsym(RTLD_NEXT, "freeaddrinfo");
  _real_func_addr[getnameinfo_event] = _real_dlsym(RTLD_NEXT, "getnameinfo");
  _real_func_addr[sendto_event] = _real_dlsym(RTLD_NEXT, "sendto");
  _real_func_addr[sendmsg_event] = _real_dlsym(RTLD_NEXT, "sendmsg");
  _real_func_addr[recvfrom_event] = _real_dlsym(RTLD_NEXT, "recvfrom");
  _real_func_addr[recvmsg_event] = _real_dlsym(RTLD_NEXT, "recvmsg");
  _real_func_addr[waitid_event] = _real_dlsym(RTLD_NEXT, "waitid");
  _real_func_addr[wait4_event] = _real_dlsym(RTLD_NEXT, "wait4");
  _real_func_addr[sigaction_event] = _real_dlsym(RTLD_NEXT, "sigaction");
  _real_func_addr[signal_event] = _real_dlsym(RTLD_NEXT, "signal");
  _real_func_addr[sigset_event] = _real_dlsym(RTLD_NEXT, "sigset");
  _real_func_addr[fopen_event] = _real_dlsym(RTLD_NEXT, "fopen");
  _real_func_addr[fopen64_event] = _real_dlsym(RTLD_NEXT, "fopen64");
  _real_func_addr[freopen_event] = _real_dlsym(RTLD_NEXT, "freopen");
  _real_func_addr[fclose_event] = _real_dlsym(RTLD_NEXT, "fclose");
  _real_func_addr[fdopen_event] = _real_dlsym(RTLD_NEXT, "fdopen");
  _real_func_addr[fgets_event] = _real_dlsym(RTLD_NEXT, "fgets");
  _real_func_addr[ferror_event] = _real_dlsym(RTLD_NEXT, "ferror");
  _real_func_addr[feof_event] = _real_dlsym(RTLD_NEXT, "feof");
  _real_func_addr[fileno_event] = _real_dlsym(RTLD_NEXT, "fileno");
  _real_func_addr[fflush_event] = _real_dlsym(RTLD_NEXT, "fflush");
  _real_func_addr[setvbuf_event] = _real_dlsym(RTLD_NEXT, "setvbuf");
  _real_func_addr[fseek_event] = _real_dlsym(RTLD_NEXT, "fseek");
  _real_func_addr[fputs_event] = _real_dlsym(RTLD_NEXT, "fputs");
  _real_func_addr[fputc_event] = _real_dlsym(RTLD_NEXT, "fputc");
  _real_func_addr[fsync_event] = _real_dlsym(RTLD_NEXT, "fsync");
  _real_func_addr[ftell_event] = _real_dlsym(RTLD_NEXT, "ftell");
  _real_func_addr[fgetpos_event] = _real_dlsym(RTLD_NEXT, "fgetpos");
  _real_func_addr[fgetpos64_event] = _real_dlsym(RTLD_NEXT, "fgetpos64");
  _real_func_addr[fsetpos_event] = _real_dlsym(RTLD_NEXT, "fsetpos");
  _real_func_addr[fsetpos64_event] = _real_dlsym(RTLD_NEXT, "fsetpos64");
  _real_func_addr[fwrite_event] = _real_dlsym(RTLD_NEXT, "fwrite");
  _real_func_addr[fread_event] = _real_dlsym(RTLD_NEXT, "fread");
  _real_func_addr[getc_event] = _real_dlsym(RTLD_NEXT, "getc");
  _real_func_addr[fgetc_event] = _real_dlsym(RTLD_NEXT, "fgetc");
  _real_func_addr[ungetc_event] = _real_dlsym(RTLD_NEXT, "ungetc");
  _real_func_addr[getline_event] = _real_dlsym(RTLD_NEXT, "getline");
  _real_func_addr[getdelim_event] = _real_dlsym(RTLD_NEXT, "getdelim");
  _real_func_addr[putc_event] = _real_dlsym(RTLD_NEXT, "putc");
  _real_func_addr[rewind_event] = _real_dlsym(RTLD_NEXT, "rewind");
  _real_func_addr[tmpfile_event] = _real_dlsym(RTLD_NEXT, "tmpfile");
  _real_func_addr[flockfile_event] = _real_dlsym(RTLD_NEXT, "flockfile");
  _real_func_addr[ftrylockfile_event] = _real_dlsym(RTLD_NEXT, "ftrylockfile");
  _real_func_addr[funlockfile_event] = _real_dlsym(RTLD_NEXT, "funlockfile");
  _real_func_addr[closedir_event] = _real_dlsym(RTLD_NEXT, "closedir");
  _real_func_addr[opendir_event] = _real_dlsym(RTLD_NEXT, "opendir");
  _real_func_addr[fdopendir_event] = _real_dlsym(RTLD_NEXT, "fdopendir");
  _real_func_addr[readdir_event] = _real_dlsym(RTLD_NEXT, "readdir");
  _real_func_addr[readdir_r_event] = _real_dlsym(RTLD_NEXT, "readdir_r");
  _real_func_addr[pthread_cond_broadcast_event] = dlvsym(RTLD_NEXT, "pthread_cond_broadcast", "GLIBC_2.3.2");
  _real_func_addr[pthread_cond_signal_event] = dlvsym(RTLD_NEXT, "pthread_cond_signal", "GLIBC_2.3.2");
  _real_func_addr[pthread_cond_wait_event] = dlvsym(RTLD_NEXT, "pthread_cond_wait", "GLIBC_2.3.2");
  _real_func_addr[pthread_cond_timedwait_event] = dlvsym(RTLD_NEXT, "pthread_cond_timedwait", "GLIBC_2.3.2");
  _real_func_addr[pthread_cond_destroy_event] = dlvsym(RTLD_NEXT, "pthread_cond_destroy", "GLIBC_2.3.2");
  _real_func_addr[fxstat_event] = _real_dlsym(RTLD_NEXT, "__fxstat");
  _real_func_addr[fxstat64_event] = _real_dlsym(RTLD_NEXT, "__fxstat64");
  _real_func_addr[lxstat_event] = _real_dlsym(RTLD_NEXT, "__lxstat");
  _real_func_addr[lxstat64_event] = _real_dlsym(RTLD_NEXT, "__lxstat64");
  _real_func_addr[xstat_event] = _real_dlsym(RTLD_NEXT, "__xstat");
  _real_func_addr[xstat64_event] = _real_dlsym(RTLD_NEXT, "__xstat64");
  _real_func_addr[libc_memalign_event] = _real_dlsym(RTLD_NEXT, "__libc_memalign");
  _real_func_addr[vfprintf_event] = _real_dlsym(RTLD_NEXT, "vfprintf");
  _real_func_addr[vfscanf_event] = _real_dlsym(RTLD_NEXT, "vfscanf");
  _real_func_addr[exec_barrier_event] = _real_dlsym(RTLD_NEXT, "exec_barrier");
  _real_func_addr[signal_handler_event] = _real_dlsym(RTLD_NEXT, "signal_handler");
  _real_func_addr[user_event] = _real_dlsym(RTLD_NEXT, "user");
  _real_func_addr[syscall_event] = _real_dlsym(RTLD_NEXT, "syscall");
}