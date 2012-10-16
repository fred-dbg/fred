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

#include "synchronizationlogging.h"

void print_log_entry_common(int idx, log_entry_t *entry);

static const char *log_event_str[] = {
  "empty",
  "accept",
  "accept4",
  "access",
  "bind",
  "calloc",
  "chmod",
  "chown",
  "close",
  "connect",
  "dup",
  "dup2",
  "dup3",
  "fcntl",
  "fchdir",
  "fdatasync",
  "getcwd",
  "gettimeofday",
  "getpeername",
  "getsockname",
  "link",
  "symlink",
  "listen",
  "localtime_r",
  "utime",
  "utimes",
  "lutimes",
  "futimes",
  "clock_getres",
  "clock_gettime",
  "clock_settime",
  "lseek",
  "lseek64",
  "llseek",
  "malloc",
  "free",
  "mkdir",
  "mkstemp",
  "mmap",
  "mmap64",
  "munmap",
  "mremap",
  "open",
  "open64",
  "openat",
  "pread",
  "preadv",
  "pwrite",
  "pwritev",
  "pthread_rwlock_unlock",
  "pthread_rwlock_rdlock",
  "pthread_rwlock_wrlock",
  "pthread_create",
  "pthread_detach",
  "pthread_exit",
  "pthread_join",
  "pthread_kill",
  "pthread_mutex_lock",
  "pthread_mutex_trylock",
  "pthread_mutex_unlock",
  "rand",
  "fork",
  "read",
  "readv",
  "readlink",
  "realpath",
  "realloc",
  "rename",
  "rmdir",
  "select",
  "ppoll",
  "setsockopt",
  "getsockopt",
  "ioctl",
  "shutdown",
  "sigwait",
  "srand",
  "socket",
  "socketpair",
  "time",
  "truncate",
  "ftruncate",
  "truncate64",
  "ftruncate64",
  "unlink",
  "write",
  "writev",
  "epoll_create",
  "epoll_create1",
  "epoll_ctl",
  "epoll_wait",
  "getpwnam_r",
  "getpwuid_r",
  "getgrnam_r",
  "getgrgid_r",
  "getaddrinfo",
  "freeaddrinfo",
  "getnameinfo",
  "sendto",
  "sendmsg",
  "recvfrom",
  "recvmsg",
  "waitid",
  "wait4",
  "sigaction",
  "signal",
  "sigset",
  "fopen",
  "fopen64",
  "freopen",
  "fclose",
  "fdopen",
  "fgets",
  "ferror",
  "feof",
  "fileno",
  "fflush",
  "setvbuf",
  "fseek",
  "fputs",
  "puts",
  "fputc",
  "fsync",
  "ftell",
  "fgetpos",
  "fgetpos64",
  "fsetpos",
  "fsetpos64",
  "fwrite",
  "fread",
  "getc",
  "fgetc",
  "ungetc",
  "getline",
  "getdelim",
  "putc",
  "rewind",
  "tmpfile",
  "flockfile",
  "ftrylockfile",
  "funlockfile",
  "closedir",
  "opendir",
  "fdopendir",
  "readdir",
  "readdir_r",
  "pthread_cond_broadcast",
  "pthread_cond_signal",
  "pthread_cond_wait",
  "pthread_cond_timedwait",
  "pthread_cond_destroy",
  "fxstat",
  "fxstat64",
  "lxstat",
  "lxstat64",
  "xstat",
  "xstat64",
  "libc_memalign",
  "vfprintf",
  "vfscanf",
  "exec_barrier",
  "signal_handler",
  "user",
  "syscall",
};

void print_log_entry_empty(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_accept(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_accept4(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_access(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_bind(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_calloc(int idx, log_entry_t *entry)  {
  printf(" nmemb=%zu size=%zu\n" ,
         GET_FIELD_PTR(entry, calloc, nmemb),
         GET_FIELD_PTR(entry, calloc, size));
}

void print_log_entry_chmod(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_chown(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_close(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_connect(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_dup(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_dup2(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_dup3(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_fcntl(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_fchdir(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_fdatasync(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_getcwd(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_gettimeofday(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_getpeername(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_getsockname(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_link(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_symlink(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_listen(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_localtime_r(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_utime(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_utimes(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_lutimes(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_futimes(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_clock_getres(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_clock_gettime(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_clock_settime(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_lseek(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_lseek64(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_llseek(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_malloc(int idx, log_entry_t *entry)  {
  printf(" size=%zu\n" ,
         GET_FIELD_PTR(entry, malloc, size));
}

void print_log_entry_free(int idx, log_entry_t *entry)  {
  printf(" ptr=%p\n" ,
         GET_FIELD_PTR(entry, free, ptr));
}

void print_log_entry_mkdir(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_mkstemp(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_mmap(int idx, log_entry_t *entry)  {
  printf(" addr=%p length=%zu prot=%d flags=%d fd=%d offset=%ld\n" ,
         GET_FIELD_PTR(entry, mmap, addr),
         GET_FIELD_PTR(entry, mmap, length),
         GET_FIELD_PTR(entry, mmap, prot),
         GET_FIELD_PTR(entry, mmap, flags),
         GET_FIELD_PTR(entry, mmap, fd),
         GET_FIELD_PTR(entry, mmap, offset));
}

void print_log_entry_mmap64(int idx, log_entry_t *entry)  {
  printf(" addr=%p length=%zu prot=%d flags=%d fd=%d offset=%ld\n" ,
         GET_FIELD_PTR(entry, mmap64, addr),
         GET_FIELD_PTR(entry, mmap64, length),
         GET_FIELD_PTR(entry, mmap64, prot),
         GET_FIELD_PTR(entry, mmap64, flags),
         GET_FIELD_PTR(entry, mmap64, fd),
         GET_FIELD_PTR(entry, mmap64, offset));
}

void print_log_entry_munmap(int idx, log_entry_t *entry)  {
  printf(" addr=%p length=%zu\n" ,
         GET_FIELD_PTR(entry, munmap, addr),
         GET_FIELD_PTR(entry, munmap, length));
}

void print_log_entry_mremap(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_open(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_open64(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_openat(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_pread(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_preadv(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_pwrite(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_pwritev(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_pthread_rwlock_unlock(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_pthread_rwlock_rdlock(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_pthread_rwlock_wrlock(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_pthread_create(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_pthread_detach(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_pthread_exit(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_pthread_join(int idx, log_entry_t *entry)  {
  printf(" thread=%lud\n" ,
         GET_FIELD_PTR(entry, pthread_join, thread));
}

void print_log_entry_pthread_kill(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_pthread_mutex_lock(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_pthread_mutex_trylock(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_pthread_mutex_unlock(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_rand(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_fork(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_read(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_readv(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_readlink(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_realpath(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_realloc(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_rename(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_rmdir(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_select(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_ppoll(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_setsockopt(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_getsockopt(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_ioctl(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_shutdown(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_sigwait(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_srand(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_socket(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_socketpair(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_time(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_truncate(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_ftruncate(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_truncate64(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_ftruncate64(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_unlink(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_write(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_writev(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_epoll_create(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_epoll_create1(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_epoll_ctl(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_epoll_wait(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_getpwnam_r(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_getpwuid_r(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_getgrnam_r(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_getgrgid_r(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_getaddrinfo(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_freeaddrinfo(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_getnameinfo(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_sendto(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_sendmsg(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_recvfrom(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_recvmsg(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_waitid(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_wait4(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_sigaction(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_signal(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_sigset(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_fopen(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_fopen64(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_freopen(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_fclose(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_fdopen(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_fgets(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_ferror(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_feof(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_fileno(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_fflush(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_setvbuf(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_fseek(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_fputs(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_puts(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_fputc(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_fsync(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_ftell(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_fgetpos(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_fgetpos64(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_fsetpos(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_fsetpos64(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_fwrite(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_fread(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_getc(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_fgetc(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_ungetc(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_getline(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_getdelim(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_putc(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_rewind(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_tmpfile(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_flockfile(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_ftrylockfile(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_funlockfile(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_closedir(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_opendir(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_fdopendir(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_readdir(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_readdir_r(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_pthread_cond_broadcast(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_pthread_cond_signal(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_pthread_cond_wait(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_pthread_cond_timedwait(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_pthread_cond_destroy(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_fxstat(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_fxstat64(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_lxstat(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_lxstat64(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_xstat(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_xstat64(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_libc_memalign(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_vfprintf(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_vfscanf(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_exec_barrier(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_signal_handler(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_user(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void print_log_entry_syscall(int idx, log_entry_t *entry)  {
  printf("\n" );
}

void printEntry(int idx, log_entry_t *entry)
{
  print_log_entry_common(idx, entry);
  switch (entry->eventId()) {
    case empty_event: print_log_entry_empty(idx, entry); break;

    case accept_event: print_log_entry_accept(idx, entry); break;

    case accept4_event: print_log_entry_accept4(idx, entry); break;

    case access_event: print_log_entry_access(idx, entry); break;

    case bind_event: print_log_entry_bind(idx, entry); break;

    case calloc_event: print_log_entry_calloc(idx, entry); break;

    case chmod_event: print_log_entry_chmod(idx, entry); break;

    case chown_event: print_log_entry_chown(idx, entry); break;

    case close_event: print_log_entry_close(idx, entry); break;

    case connect_event: print_log_entry_connect(idx, entry); break;

    case dup_event: print_log_entry_dup(idx, entry); break;

    case dup2_event: print_log_entry_dup2(idx, entry); break;

    case dup3_event: print_log_entry_dup3(idx, entry); break;

    case fcntl_event: print_log_entry_fcntl(idx, entry); break;

    case fchdir_event: print_log_entry_fchdir(idx, entry); break;

    case fdatasync_event: print_log_entry_fdatasync(idx, entry); break;

    case getcwd_event: print_log_entry_getcwd(idx, entry); break;

    case gettimeofday_event: print_log_entry_gettimeofday(idx, entry); break;

    case getpeername_event: print_log_entry_getpeername(idx, entry); break;

    case getsockname_event: print_log_entry_getsockname(idx, entry); break;

    case link_event: print_log_entry_link(idx, entry); break;

    case symlink_event: print_log_entry_symlink(idx, entry); break;

    case listen_event: print_log_entry_listen(idx, entry); break;

    case localtime_r_event: print_log_entry_localtime_r(idx, entry); break;

    case utime_event: print_log_entry_utime(idx, entry); break;

    case utimes_event: print_log_entry_utimes(idx, entry); break;

    case lutimes_event: print_log_entry_lutimes(idx, entry); break;

    case futimes_event: print_log_entry_futimes(idx, entry); break;

    case clock_getres_event: print_log_entry_clock_getres(idx, entry); break;

    case clock_gettime_event: print_log_entry_clock_gettime(idx, entry); break;

    case clock_settime_event: print_log_entry_clock_settime(idx, entry); break;

    case lseek_event: print_log_entry_lseek(idx, entry); break;

    case lseek64_event: print_log_entry_lseek64(idx, entry); break;

    case llseek_event: print_log_entry_llseek(idx, entry); break;

    case malloc_event: print_log_entry_malloc(idx, entry); break;

    case free_event: print_log_entry_free(idx, entry); break;

    case mkdir_event: print_log_entry_mkdir(idx, entry); break;

    case mkstemp_event: print_log_entry_mkstemp(idx, entry); break;

    case mmap_event: print_log_entry_mmap(idx, entry); break;

    case mmap64_event: print_log_entry_mmap64(idx, entry); break;

    case munmap_event: print_log_entry_munmap(idx, entry); break;

    case mremap_event: print_log_entry_mremap(idx, entry); break;

    case open_event: print_log_entry_open(idx, entry); break;

    case open64_event: print_log_entry_open64(idx, entry); break;

    case openat_event: print_log_entry_openat(idx, entry); break;

    case pread_event: print_log_entry_pread(idx, entry); break;

    case preadv_event: print_log_entry_preadv(idx, entry); break;

    case pwrite_event: print_log_entry_pwrite(idx, entry); break;

    case pwritev_event: print_log_entry_pwritev(idx, entry); break;

    case pthread_rwlock_unlock_event: print_log_entry_pthread_rwlock_unlock(idx, entry); break;

    case pthread_rwlock_rdlock_event: print_log_entry_pthread_rwlock_rdlock(idx, entry); break;

    case pthread_rwlock_wrlock_event: print_log_entry_pthread_rwlock_wrlock(idx, entry); break;

    case pthread_create_event: print_log_entry_pthread_create(idx, entry); break;

    case pthread_detach_event: print_log_entry_pthread_detach(idx, entry); break;

    case pthread_exit_event: print_log_entry_pthread_exit(idx, entry); break;

    case pthread_join_event: print_log_entry_pthread_join(idx, entry); break;

    case pthread_kill_event: print_log_entry_pthread_kill(idx, entry); break;

    case pthread_mutex_lock_event: print_log_entry_pthread_mutex_lock(idx, entry); break;

    case pthread_mutex_trylock_event: print_log_entry_pthread_mutex_trylock(idx, entry); break;

    case pthread_mutex_unlock_event: print_log_entry_pthread_mutex_unlock(idx, entry); break;

    case rand_event: print_log_entry_rand(idx, entry); break;

    case fork_event: print_log_entry_fork(idx, entry); break;

    case read_event: print_log_entry_read(idx, entry); break;

    case readv_event: print_log_entry_readv(idx, entry); break;

    case readlink_event: print_log_entry_readlink(idx, entry); break;

    case realpath_event: print_log_entry_realpath(idx, entry); break;

    case realloc_event: print_log_entry_realloc(idx, entry); break;

    case rename_event: print_log_entry_rename(idx, entry); break;

    case rmdir_event: print_log_entry_rmdir(idx, entry); break;

    case select_event: print_log_entry_select(idx, entry); break;

    case ppoll_event: print_log_entry_ppoll(idx, entry); break;

    case setsockopt_event: print_log_entry_setsockopt(idx, entry); break;

    case getsockopt_event: print_log_entry_getsockopt(idx, entry); break;

    case ioctl_event: print_log_entry_ioctl(idx, entry); break;

    case shutdown_event: print_log_entry_shutdown(idx, entry); break;

    case sigwait_event: print_log_entry_sigwait(idx, entry); break;

    case srand_event: print_log_entry_srand(idx, entry); break;

    case socket_event: print_log_entry_socket(idx, entry); break;

    case socketpair_event: print_log_entry_socketpair(idx, entry); break;

    case time_event: print_log_entry_time(idx, entry); break;

    case truncate_event: print_log_entry_truncate(idx, entry); break;

    case ftruncate_event: print_log_entry_ftruncate(idx, entry); break;

    case truncate64_event: print_log_entry_truncate64(idx, entry); break;

    case ftruncate64_event: print_log_entry_ftruncate64(idx, entry); break;

    case unlink_event: print_log_entry_unlink(idx, entry); break;

    case write_event: print_log_entry_write(idx, entry); break;

    case writev_event: print_log_entry_writev(idx, entry); break;

    case epoll_create_event: print_log_entry_epoll_create(idx, entry); break;

    case epoll_create1_event: print_log_entry_epoll_create1(idx, entry); break;

    case epoll_ctl_event: print_log_entry_epoll_ctl(idx, entry); break;

    case epoll_wait_event: print_log_entry_epoll_wait(idx, entry); break;

    case getpwnam_r_event: print_log_entry_getpwnam_r(idx, entry); break;

    case getpwuid_r_event: print_log_entry_getpwuid_r(idx, entry); break;

    case getgrnam_r_event: print_log_entry_getgrnam_r(idx, entry); break;

    case getgrgid_r_event: print_log_entry_getgrgid_r(idx, entry); break;

    case getaddrinfo_event: print_log_entry_getaddrinfo(idx, entry); break;

    case freeaddrinfo_event: print_log_entry_freeaddrinfo(idx, entry); break;

    case getnameinfo_event: print_log_entry_getnameinfo(idx, entry); break;

    case sendto_event: print_log_entry_sendto(idx, entry); break;

    case sendmsg_event: print_log_entry_sendmsg(idx, entry); break;

    case recvfrom_event: print_log_entry_recvfrom(idx, entry); break;

    case recvmsg_event: print_log_entry_recvmsg(idx, entry); break;

    case waitid_event: print_log_entry_waitid(idx, entry); break;

    case wait4_event: print_log_entry_wait4(idx, entry); break;

    case sigaction_event: print_log_entry_sigaction(idx, entry); break;

    case signal_event: print_log_entry_signal(idx, entry); break;

    case sigset_event: print_log_entry_sigset(idx, entry); break;

    case fopen_event: print_log_entry_fopen(idx, entry); break;

    case fopen64_event: print_log_entry_fopen64(idx, entry); break;

    case freopen_event: print_log_entry_freopen(idx, entry); break;

    case fclose_event: print_log_entry_fclose(idx, entry); break;

    case fdopen_event: print_log_entry_fdopen(idx, entry); break;

    case fgets_event: print_log_entry_fgets(idx, entry); break;

    case ferror_event: print_log_entry_ferror(idx, entry); break;

    case feof_event: print_log_entry_feof(idx, entry); break;

    case fileno_event: print_log_entry_fileno(idx, entry); break;

    case fflush_event: print_log_entry_fflush(idx, entry); break;

    case setvbuf_event: print_log_entry_setvbuf(idx, entry); break;

    case fseek_event: print_log_entry_fseek(idx, entry); break;

    case fputs_event: print_log_entry_fputs(idx, entry); break;

    case puts_event: print_log_entry_puts(idx, entry); break;

    case fputc_event: print_log_entry_fputc(idx, entry); break;

    case fsync_event: print_log_entry_fsync(idx, entry); break;

    case ftell_event: print_log_entry_ftell(idx, entry); break;

    case fgetpos_event: print_log_entry_fgetpos(idx, entry); break;

    case fgetpos64_event: print_log_entry_fgetpos64(idx, entry); break;

    case fsetpos_event: print_log_entry_fsetpos(idx, entry); break;

    case fsetpos64_event: print_log_entry_fsetpos64(idx, entry); break;

    case fwrite_event: print_log_entry_fwrite(idx, entry); break;

    case fread_event: print_log_entry_fread(idx, entry); break;

    case getc_event: print_log_entry_getc(idx, entry); break;

    case fgetc_event: print_log_entry_fgetc(idx, entry); break;

    case ungetc_event: print_log_entry_ungetc(idx, entry); break;

    case getline_event: print_log_entry_getline(idx, entry); break;

    case getdelim_event: print_log_entry_getdelim(idx, entry); break;

    case putc_event: print_log_entry_putc(idx, entry); break;

    case rewind_event: print_log_entry_rewind(idx, entry); break;

    case tmpfile_event: print_log_entry_tmpfile(idx, entry); break;

    case flockfile_event: print_log_entry_flockfile(idx, entry); break;

    case ftrylockfile_event: print_log_entry_ftrylockfile(idx, entry); break;

    case funlockfile_event: print_log_entry_funlockfile(idx, entry); break;

    case closedir_event: print_log_entry_closedir(idx, entry); break;

    case opendir_event: print_log_entry_opendir(idx, entry); break;

    case fdopendir_event: print_log_entry_fdopendir(idx, entry); break;

    case readdir_event: print_log_entry_readdir(idx, entry); break;

    case readdir_r_event: print_log_entry_readdir_r(idx, entry); break;

    case pthread_cond_broadcast_event: print_log_entry_pthread_cond_broadcast(idx, entry); break;

    case pthread_cond_signal_event: print_log_entry_pthread_cond_signal(idx, entry); break;

    case pthread_cond_wait_event: print_log_entry_pthread_cond_wait(idx, entry); break;

    case pthread_cond_timedwait_event: print_log_entry_pthread_cond_timedwait(idx, entry); break;

    case pthread_cond_destroy_event: print_log_entry_pthread_cond_destroy(idx, entry); break;

    case fxstat_event: print_log_entry_fxstat(idx, entry); break;

    case fxstat64_event: print_log_entry_fxstat64(idx, entry); break;

    case lxstat_event: print_log_entry_lxstat(idx, entry); break;

    case lxstat64_event: print_log_entry_lxstat64(idx, entry); break;

    case xstat_event: print_log_entry_xstat(idx, entry); break;

    case xstat64_event: print_log_entry_xstat64(idx, entry); break;

    case libc_memalign_event: print_log_entry_libc_memalign(idx, entry); break;

    case vfprintf_event: print_log_entry_vfprintf(idx, entry); break;

    case vfscanf_event: print_log_entry_vfscanf(idx, entry); break;

    case exec_barrier_event: print_log_entry_exec_barrier(idx, entry); break;

    case signal_handler_event: print_log_entry_signal_handler(idx, entry); break;

    case user_event: print_log_entry_user(idx, entry); break;

    case syscall_event: print_log_entry_syscall(idx, entry); break;
  }
}
