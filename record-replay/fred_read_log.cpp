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
// Needed for 'off64_t' and 'struct stat64'
//#define _GNU_SOURCE

#include <dirent.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <map>
#include "constants.h"
#include "jalib.h"
#include "dmtcpalloc.h"

//namespace dmtcp = std;
//using namespace dmtcp;
#include "synchronizationlogging.h"
#include "log.h"

// Must be in sync with synchronizationlogging.h definition.
//#define READLINK_MAX_LENGTH 256

#define IFNAME_PRINT_ENTRY(name, idx, entry)                                   \
  do {                                                                         \
    if (GET_COMMON_PTR(entry, event) == name##_event)                          \
      print_log_entry_##name(idx, entry);                                      \
  } while(0)

#define TOSTRING(name) #name

#define IFNAME_EVENT_TO_STRING(name, event_type, e)                            \
  do {                                                                         \
    if (e == name##_event)                                                     \
      event_type.assign(TOSTRING(name));                                       \
  } while(0)

#define EVENT_TO_STRING(event_type, e)                                         \
  do {                                                                         \
    FOREACH_NAME(IFNAME_EVENT_TO_STRING, event_type, e);                       \
  } while(0)

#define PRINT_ENTRIES(idx, entry)                                              \
  do {                                                                         \
    FOREACH_NAME(IFNAME_PRINT_ENTRY, idx, entry);                              \
  } while(0)


void print_log_entry_common(int idx, log_entry_t *entry) {
  std::string event_type;
  EVENT_TO_STRING(event_type, GET_COMMON_PTR(entry, event));
  printf("%2d: clone_id=%ld, [%-20.20s]: ",
         idx, GET_COMMON_PTR(entry, clone_id), event_type.c_str());

  switch ((long) (unsigned long) GET_COMMON_PTR(entry, retval)) {
    case 0:
      printf("retval=  0     , "); break;
    case -1:
      printf("retval= -1     , "); break;
    default:
      printf("retval=%p, ", GET_COMMON_PTR(entry, retval)); break;
  }
  printf("log_offset=%2ld, my_errno=%d, isOptional=%d",
         GET_COMMON_PTR(entry, log_offset), GET_COMMON_PTR(entry, my_errno),
         GET_COMMON_PTR(entry, isOptional));
}

void print_log_entry_accept(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", sockfd=%d, sockaddr=%p, addrlen=%p\n",
         GET_FIELD_PTR(entry, accept, sockfd),
         GET_FIELD_PTR(entry, accept, addr),
         GET_FIELD_PTR(entry, accept, addrlen));
}

void print_log_entry_accept4(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", sockfd=%d, sockaddr=%p, addrlen=%p, flags:%d\n",
         GET_FIELD_PTR(entry, accept4, sockfd),
         GET_FIELD_PTR(entry, accept4, addr),
         GET_FIELD_PTR(entry, accept4, addrlen),
         GET_FIELD_PTR(entry, accept4, flags));
}

void print_log_entry_access(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", pathname=%p, mode=%d\n",
         GET_FIELD_PTR(entry, access, pathname),
         GET_FIELD_PTR(entry, access, mode));
}

void print_log_entry_bind(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", sockfd=%d, my_addr=%p, addrlen=%d\n",
         GET_FIELD_PTR(entry, bind, sockfd),
         GET_FIELD_PTR(entry, bind, addr),
         GET_FIELD_PTR(entry, bind, addrlen));
}

void print_log_entry_calloc(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", nmemb=%Zu, size=%Zu\n",
         GET_FIELD_PTR(entry, calloc, nmemb),
         GET_FIELD_PTR(entry, calloc, size));
}

void print_log_entry_connect(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", sockfd=%d, serv_addr=%p, addrlen=%d\n",
         GET_FIELD_PTR(entry, connect, sockfd),
         GET_FIELD_PTR(entry, connect, serv_addr),
         GET_FIELD_PTR(entry, connect, addrlen));
}

void print_log_entry_dup(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", oldfd=%d\n",
         GET_FIELD_PTR(entry, dup, oldfd));
}

void print_log_entry_dup2(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", oldfd=%d, newfd:%d\n",
         GET_FIELD_PTR(entry, dup2, oldfd),
         GET_FIELD_PTR(entry, dup2, newfd));
}

void print_log_entry_dup3(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", oldfd=%d, newfd:%d, flags:0x%x\n",
         GET_FIELD_PTR(entry, dup3, oldfd),
         GET_FIELD_PTR(entry, dup3, newfd),
         GET_FIELD_PTR(entry, dup3, flags));
}

void print_log_entry_close(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", fd=%d\n", GET_FIELD_PTR(entry, close, fd));
}

void print_log_entry_chmod(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", path=%p, mode=%u\n",
         GET_FIELD_PTR(entry, chmod, path),
         GET_FIELD_PTR(entry, chmod, mode));
}

void print_log_entry_chown(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", path=%p, owner=%u, group=%u\n",
         GET_FIELD_PTR(entry, chown, path),
         GET_FIELD_PTR(entry, chown, owner),
         GET_FIELD_PTR(entry, chown, group));
}

void print_log_entry_closedir(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", dirp=%p\n", GET_FIELD_PTR(entry, closedir, dirp));
}

void print_log_entry_exec_barrier(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf("\n");
}

void print_log_entry_fclose(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", fp=%p\n",
         GET_FIELD_PTR(entry, fclose, fp));
}

void print_log_entry_fchdir(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", fd=%d\n",
         GET_FIELD_PTR(entry, fchdir, fd));
}

void print_log_entry_fcntl(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", fd=%d, cmd=%d, arg_3_l=%ld, arg_3_f=%p\n",
         GET_FIELD_PTR(entry, fcntl, fd),
         GET_FIELD_PTR(entry, fcntl, cmd),
         GET_FIELD_PTR(entry, fcntl, arg_3_l),
         GET_FIELD_PTR(entry, fcntl, arg_3_f));
}

void print_log_entry_fdatasync(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", fd=%d\n",
         GET_FIELD_PTR(entry, fdatasync, fd));
}

void print_log_entry_fdopen(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", fd=%d, mode=%p\n",
         GET_FIELD_PTR(entry, fdopen, fd),
         GET_FIELD_PTR(entry, fdopen, mode));
}

void print_log_entry_fdopendir(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", fd=%d\n",
         GET_FIELD_PTR(entry, fdopendir, fd));
}

void print_log_entry_fgets(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", s=%p, size=%d, stream=%p\n",
         GET_FIELD_PTR(entry, fgets, s),
         GET_FIELD_PTR(entry, fgets, size),
         GET_FIELD_PTR(entry, fgets, stream));
}

void print_log_entry_ferror(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", stream=%p\n",
         GET_FIELD_PTR(entry, ferror, stream));
}

void print_log_entry_feof(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", stream=%p\n",
         GET_FIELD_PTR(entry, feof, stream));
}

void print_log_entry_fileno(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", stream=%p\n",
         GET_FIELD_PTR(entry, fileno, stream));
}

void print_log_entry_fopen(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", name=%p, mode=%p\n",
         GET_FIELD_PTR(entry, fopen, name),
         GET_FIELD_PTR(entry, fopen, mode));
}

void print_log_entry_fprintf(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", stream=%p, format=%p\n",
         GET_FIELD_PTR(entry, fprintf, stream),
         GET_FIELD_PTR(entry, fprintf, format));
}

void print_log_entry_fputs(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", s=%p, stream=%p\n",
         GET_FIELD_PTR(entry, fputs, s),
         GET_FIELD_PTR(entry, fputs, stream));
}

void print_log_entry_fputc(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", c=%d, stream=%p\n",
         GET_FIELD_PTR(entry, fputc, c),
         GET_FIELD_PTR(entry, fputc, stream));
}

void print_log_entry_free(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", ptr=%p\n", GET_FIELD_PTR(entry, free, ptr));
}

void print_log_entry_ftell(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", stream=%p\n", GET_FIELD_PTR(entry, ftell, stream));
}

void print_log_entry_fwrite(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", ptr=%p, size=%Zu, nmemb=%Zu, stream=%p\n",
         GET_FIELD_PTR(entry, fwrite, ptr),
         GET_FIELD_PTR(entry, fwrite, size),
         GET_FIELD_PTR(entry, fwrite, nmemb),
         GET_FIELD_PTR(entry, fwrite, stream));
}

void print_log_entry_fsync(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", fd=%d\n", GET_FIELD_PTR(entry, fsync, fd));
}

void print_log_entry_fseek(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", stream=%p\n, offset=%ld, whence=%d\n",
         GET_FIELD_PTR(entry, fseek, stream),
         GET_FIELD_PTR(entry, fseek, offset),
         GET_FIELD_PTR(entry, fseek, whence));
}

void print_log_entry_fread(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", ptr=%p\n, size=%Zu, nmemb=%Zu, stream=%p\n",
         GET_FIELD_PTR(entry, fread, ptr),
         GET_FIELD_PTR(entry, fread, size),
         GET_FIELD_PTR(entry, fread, nmemb),
         GET_FIELD_PTR(entry, fread, stream));
}

void print_log_entry_freopen(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", path=%p, mode=%p, stream=%p\n",
         GET_FIELD_PTR(entry, freopen, path),
         GET_FIELD_PTR(entry, freopen, mode),
         GET_FIELD_PTR(entry, freopen, stream));
}

void print_log_entry_fxstat(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", vers=%d, fd=%d\n",
         GET_FIELD_PTR(entry, fxstat, vers),
         GET_FIELD_PTR(entry, fxstat, fd));
}

void print_log_entry_fxstat64(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", vers=%d, fd=%d\n",
         GET_FIELD_PTR(entry, fxstat64, vers),
         GET_FIELD_PTR(entry, fxstat64, fd));
}

void print_log_entry_getpeername(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", sockfd=%d, sockaddr=%p, addrlen=%p\n",
         GET_FIELD_PTR(entry, getpeername, sockfd),
         GET_FIELD_PTR(entry, getpeername, addr),
         GET_FIELD_PTR(entry, getpeername, addrlen));
}

void print_log_entry_getsockname(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", sockfd=%d, sockaddr=%p, addrlen=%p\n",
         GET_FIELD_PTR(entry, getsockname, sockfd),
         GET_FIELD_PTR(entry, getsockname, addr),
         GET_FIELD_PTR(entry, getsockname, addrlen));
}

void print_log_entry_getcwd(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", buf=%p, size=%Zu\n",
         GET_FIELD_PTR(entry, getcwd, buf),
         GET_FIELD_PTR(entry, getcwd, size));
}

void print_log_entry_libc_memalign(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", boundary=%Zu, size=%Zu, return_ptr=%p\n",
         GET_FIELD_PTR(entry, libc_memalign, boundary),
         GET_FIELD_PTR(entry, libc_memalign, size),
         (void *)GET_FIELD_PTR(entry, libc_memalign, return_ptr));
}

void print_log_entry_lseek(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", fd=%d, offset=%Zu, whence=%d\n",
         GET_FIELD_PTR(entry, lseek, fd),
         GET_FIELD_PTR(entry, lseek, offset),
         GET_FIELD_PTR(entry, lseek, whence));
}

void print_log_entry_lseek64(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", fd=%d, offset=%Zu, whence=%d\n",
         GET_FIELD_PTR(entry, lseek64, fd),
         GET_FIELD_PTR(entry, lseek64, offset),
         GET_FIELD_PTR(entry, lseek64, whence));
}

void print_log_entry_llseek(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", fd=%d, offset=%Zu, whence=%d\n",
         GET_FIELD_PTR(entry, llseek, fd),
         GET_FIELD_PTR(entry, llseek, offset),
         GET_FIELD_PTR(entry, llseek, whence));
}

void print_log_entry_link(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", oldpath=%p, newpath=%p\n",
         GET_FIELD_PTR(entry, link, oldpath),
         GET_FIELD_PTR(entry, link, newpath));
}

void print_log_entry_symlink(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", oldpath=%p, newpath=%p\n",
         GET_FIELD_PTR(entry, symlink, oldpath),
         GET_FIELD_PTR(entry, symlink, newpath));
}

void print_log_entry_listen(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", sockfd=%d, backlog=%d\n",
         GET_FIELD_PTR(entry, listen, sockfd),
         GET_FIELD_PTR(entry, listen, backlog));
}

void print_log_entry_localtime(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", timep=%p\n",
         GET_FIELD_PTR(entry, localtime, timep));
}

void print_log_entry_lxstat(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", vers=%d, path=%p\n",
         GET_FIELD_PTR(entry, lxstat, vers),
         GET_FIELD_PTR(entry, lxstat, path));
}

void print_log_entry_lxstat64(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", vers=%d, path=%p\n",
         GET_FIELD_PTR(entry, lxstat64, vers),
         GET_FIELD_PTR(entry, lxstat64, path));
}

void print_log_entry_malloc(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", size=%Zu\n",
         GET_FIELD_PTR(entry, malloc, size));
}

void print_log_entry_mkdir(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", pathname=%p, mode=%d",
         GET_FIELD_PTR(entry, mkdir, pathname),
         GET_FIELD_PTR(entry, mkdir, mode));
}

void print_log_entry_mkstemp(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", temp=%p\n",
         GET_FIELD_PTR(entry, mkstemp, temp));
}

void print_log_entry_mmap(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", addr=%p, length=%Zu, prot=%d, flags=%d, fd=%d, offset=%ld\n",
         GET_FIELD_PTR(entry, mmap, addr),
         GET_FIELD_PTR(entry, mmap, length),
         GET_FIELD_PTR(entry, mmap, prot),
         GET_FIELD_PTR(entry, mmap, flags),
         GET_FIELD_PTR(entry, mmap, fd),
         GET_FIELD_PTR(entry, mmap, offset));
}

void print_log_entry_mmap64(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", addr=%p, length=%Zu, prot=%d, flags=%d, fd=%d, offset=%ld\n",
         GET_FIELD_PTR(entry, mmap64, addr),
         GET_FIELD_PTR(entry, mmap64, length),
         GET_FIELD_PTR(entry, mmap64, prot),
         GET_FIELD_PTR(entry, mmap64, flags),
         GET_FIELD_PTR(entry, mmap64, fd),
         GET_FIELD_PTR(entry, mmap64, offset));
}

void print_log_entry_mremap(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", old_address=%p, old_size=%Zu, new_size=%Zu, flags=%d\n",
         GET_FIELD_PTR(entry, mremap, old_address),
         GET_FIELD_PTR(entry, mremap, old_size),
         GET_FIELD_PTR(entry, mremap, new_size),
         GET_FIELD_PTR(entry, mremap, flags));
}

void print_log_entry_munmap(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", addr=%p, length=%Zu\n",
         GET_FIELD_PTR(entry, munmap, addr),
         GET_FIELD_PTR(entry, munmap, length));
}

void print_log_entry_open(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", pathname=%p, flags=%d, mode=%d\n",
         GET_FIELD_PTR(entry, open, path),
         GET_FIELD_PTR(entry, open, flags),
         GET_FIELD_PTR(entry, open, open_mode));
}

void print_log_entry_open64(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", pathname=%p, flags=%d, mode=%d\n",
         GET_FIELD_PTR(entry, open64, path),
         GET_FIELD_PTR(entry, open64, flags),
         GET_FIELD_PTR(entry, open64, open_mode));
}

void print_log_entry_openat(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", dirfd=%d, pathname=%p, flags=%d\n",
         GET_FIELD_PTR(entry, openat, dirfd),
         GET_FIELD_PTR(entry, openat, pathname),
         GET_FIELD_PTR(entry, openat, flags));
}

void print_log_entry_opendir(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", name=%p\n",
         GET_FIELD_PTR(entry, opendir, name));
}

void print_log_entry_pread(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", fd=%d, buf=%p, count=%Zu, offset=%ld\n",
         GET_FIELD_PTR(entry, pread, fd),
         GET_FIELD_PTR(entry, pread, buf),
         GET_FIELD_PTR(entry, pread, count),
         GET_FIELD_PTR(entry, pread, offset));
}

void print_log_entry_preadv(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", fd=%d, iov=%p, iovcnt=%d, offset=%ld\n",
         GET_FIELD_PTR(entry, preadv, fd),
         GET_FIELD_PTR(entry, preadv, iov),
         GET_FIELD_PTR(entry, preadv, iovcnt),
         GET_FIELD_PTR(entry, preadv, offset));
}

void print_log_entry_putc(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", c=%d, stream=%p\n",
         GET_FIELD_PTR(entry, putc, c),
         GET_FIELD_PTR(entry, putc, stream));
}

void print_log_entry_pwrite(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", fd=%d, buf=%p, count=%ld, offset=%Zu\n",
         GET_FIELD_PTR(entry, pwrite, fd),
         GET_FIELD_PTR(entry, pwrite, buf),
         GET_FIELD_PTR(entry, pwrite, count),
         GET_FIELD_PTR(entry, pwrite, offset));
}

void print_log_entry_pwritev(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", fd=%d, iov=%p, iovcnt=%d, offset=%ld\n",
         GET_FIELD_PTR(entry, pwritev, fd),
         GET_FIELD_PTR(entry, pwritev, iov),
         GET_FIELD_PTR(entry, pwritev, iovcnt),
         GET_FIELD_PTR(entry, pwritev, offset));
}

void print_log_entry_pthread_detach(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", thread=%lu\n", GET_FIELD_PTR(entry, pthread_detach, thread));
}

void print_log_entry_pthread_create(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", thread=%p, attr=%p, start_routine=%p, arg=%p, stack_addr=%p\n",
         GET_FIELD_PTR(entry, pthread_create, thread),
         GET_FIELD_PTR(entry, pthread_create, attr),
         GET_FIELD_PTR(entry, pthread_create, start_routine),
         GET_FIELD_PTR(entry, pthread_create, arg),
         GET_FIELD_PTR(entry, pthread_create, stack_addr));
}

void print_log_entry_pthread_cond_broadcast(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", cond_addr=%p, signal_target=%d\n",
         GET_FIELD_PTR(entry, pthread_cond_broadcast, addr),
         GET_FIELD_PTR(entry, pthread_cond_broadcast, signal_target));
}

void print_log_entry_pthread_cond_signal(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", cond_addr=%p, signal_target=%d\n",
         GET_FIELD_PTR(entry, pthread_cond_signal, addr),
         GET_FIELD_PTR(entry, pthread_cond_signal, signal_target));
}

void print_log_entry_pthread_mutex_lock(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", mutex=%p\n", GET_FIELD_PTR(entry, pthread_mutex_lock, addr));
}

void print_log_entry_pthread_mutex_trylock(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", mutex=%p\n", GET_FIELD_PTR(entry, pthread_mutex_trylock, addr));
}

void print_log_entry_pthread_mutex_unlock(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", mutex=%p\n", GET_FIELD_PTR(entry, pthread_mutex_unlock, addr));
}

void print_log_entry_pthread_cond_wait(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", mutex_addr=%p, cond_addr=%p\n",
         GET_FIELD_PTR(entry, pthread_cond_wait, mutex_addr),
         GET_FIELD_PTR(entry, pthread_cond_wait, cond_addr));
}

void print_log_entry_pthread_cond_timedwait(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", mutex_addr=%p, cond_addr=%p, abstime=%p\n",
         GET_FIELD_PTR(entry, pthread_cond_timedwait, mutex_addr),
         GET_FIELD_PTR(entry, pthread_cond_timedwait, cond_addr),
         GET_FIELD_PTR(entry, pthread_cond_timedwait, abstime));
}

void print_log_entry_pthread_exit(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", value_ptr=%p\n", GET_FIELD_PTR(entry, pthread_exit, value_ptr));
}

void print_log_entry_pthread_join(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", thread=%lu, value_ptr=%p\n",
         GET_FIELD_PTR(entry, pthread_join, thread),
         GET_FIELD_PTR(entry, pthread_join, value_ptr));
}

void print_log_entry_pthread_kill(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", thread=%lu, sig=%d\n",
         GET_FIELD_PTR(entry, pthread_kill, thread),
         GET_FIELD_PTR(entry, pthread_kill, sig));
}

void print_log_entry_pthread_rwlock_unlock(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", rwlock=%p\n",
         GET_FIELD_PTR(entry, pthread_rwlock_unlock, addr));
}

void print_log_entry_pthread_rwlock_rdlock(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", rwlock=%p\n",
         GET_FIELD_PTR(entry, pthread_rwlock_rdlock, addr));
}

void print_log_entry_pthread_rwlock_wrlock(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", rwlock=%p\n",
         GET_FIELD_PTR(entry, pthread_rwlock_wrlock, addr));
}

void print_log_entry_rand(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf("\n");
}

void print_log_entry_read(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", fd=%d, buf_addr=%p, count=%Zu, data_offset=%ld\n",
         GET_FIELD_PTR(entry, read, fd),
         GET_FIELD_PTR(entry, read, buf_addr),
         GET_FIELD_PTR(entry, read, count),
         GET_FIELD_PTR(entry, read, data_offset));
}

void print_log_entry_readv(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", fd=%d, iov=%p, iovcnt=%d, data_offset=%ld\n",
         GET_FIELD_PTR(entry, readv, fd),
         GET_FIELD_PTR(entry, readv, iov),
         GET_FIELD_PTR(entry, readv, iovcnt),
         GET_FIELD_PTR(entry, readv, data_offset));
}

void print_log_entry_readdir(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", dirp=%p\n",
         GET_FIELD_PTR(entry, readdir, dirp));
}

void print_log_entry_readdir_r(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", dirp=%p, result=%p\n",
         GET_FIELD_PTR(entry, readdir_r, dirp),
         GET_FIELD_PTR(entry, readdir_r, result));
}

void print_log_entry_readlink(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", path=%p, bufsiz=%Zu\n",
         GET_FIELD_PTR(entry, readlink, path),
         GET_FIELD_PTR(entry, readlink, bufsiz));
}

void print_log_entry_realloc(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", size=%Zu, ptr=%p\n",
         GET_FIELD_PTR(entry, realloc, size),
         GET_FIELD_PTR(entry, realloc, ptr));
}

void print_log_entry_rename(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", oldpath=%p, newpath=%p\n",
         GET_FIELD_PTR(entry, rename, oldpath),
         GET_FIELD_PTR(entry, rename, newpath));
}

void print_log_entry_rewind(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", stream=%p\n",
         GET_FIELD_PTR(entry, rewind, stream));
}

void print_log_entry_rmdir(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", pathname=%p\n",
         GET_FIELD_PTR(entry, rmdir, pathname));
}

void print_log_entry_select(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", nfds=%d, exceptfds=%p, timeout=%p\n",
         GET_FIELD_PTR(entry, select, nfds),
         GET_FIELD_PTR(entry, select, exceptfds),
         GET_FIELD_PTR(entry, select, timeout));
}

void print_log_entry_ppoll(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", fds=%p, timeout_ts=%p, sigmask=%p\n",
         GET_FIELD_PTR(entry, ppoll, fds),
         GET_FIELD_PTR(entry, ppoll, timeout_ts),
         GET_FIELD_PTR(entry, ppoll, sigmask));
}

void print_log_entry_signal_handler(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", sig=%d\n",
         GET_FIELD_PTR(entry, signal_handler, sig));
}

void print_log_entry_sigwait(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", set=%p, sigwait_sig=%p\n",
         GET_FIELD_PTR(entry, sigwait, set),
         GET_FIELD_PTR(entry, sigwait, sigwait_sig));
}

void print_log_entry_setsockopt(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", sockfd=%d, level=%d, optname=%d, optval=%p\n",
         GET_FIELD_PTR(entry, setsockopt, sockfd),
         GET_FIELD_PTR(entry, setsockopt, level),
         GET_FIELD_PTR(entry, setsockopt, optname),
         GET_FIELD_PTR(entry, setsockopt, optval));
}

void print_log_entry_getsockopt(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", sockfd=%d, level=%d, optname=%d, optval=%p\n",
         GET_FIELD_PTR(entry, getsockopt, sockfd),
         GET_FIELD_PTR(entry, getsockopt, level),
         GET_FIELD_PTR(entry, getsockopt, optname),
         GET_FIELD_PTR(entry, getsockopt, optval));
}

void print_log_entry_ioctl(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", d=%d, request=%d, arg=%p\n",
         GET_FIELD_PTR(entry, ioctl, d),
         GET_FIELD_PTR(entry, ioctl, request),
         GET_FIELD_PTR(entry, ioctl, arg));
}

void print_log_entry_srand(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", seed=%d\n", GET_FIELD_PTR(entry, srand, seed));
}

void print_log_entry_socket(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", domain=%d, type=%d, protocol=%d\n",
         GET_FIELD_PTR(entry, socket, domain),
         GET_FIELD_PTR(entry, socket, type),
         GET_FIELD_PTR(entry, socket, protocol));
}

void print_log_entry_socketpair(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", domain=%d, type=%d, protocol=%d, sv:%p\n",
         GET_FIELD_PTR(entry, socketpair, domain),
         GET_FIELD_PTR(entry, socketpair, type),
         GET_FIELD_PTR(entry, socketpair, protocol),
         GET_FIELD_PTR(entry, socketpair, sv));
}

void print_log_entry_xstat(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", vers=%d, path=%p\n",
         GET_FIELD_PTR(entry, xstat, vers),
         GET_FIELD_PTR(entry, xstat, path));
}

void print_log_entry_xstat64(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", vers=%d, path=%p\n",
         GET_FIELD_PTR(entry, xstat64, vers),
         GET_FIELD_PTR(entry, xstat64, path));
}

void print_log_entry_time(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", tloc=%p\n", GET_FIELD_PTR(entry, time, tloc));
}

void print_log_entry_tmpfile(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf("\n");
}

void print_log_entry_gettimeofday(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", tv=%p, tz=%p\n", GET_FIELD_PTR(entry, gettimeofday, tv),
         GET_FIELD_PTR(entry, gettimeofday, tz));
}

void print_log_entry_fflush(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", stream=%p\n", GET_FIELD_PTR(entry, fflush, stream));
}

void print_log_entry_setvbuf(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", stream=%p, buf:%p, mode:%d, size:%zu\n",
         GET_FIELD_PTR(entry, setvbuf, stream),
         GET_FIELD_PTR(entry, setvbuf, buf),
         GET_FIELD_PTR(entry, setvbuf, mode),
         GET_FIELD_PTR(entry, setvbuf, size));
}

void print_log_entry_unlink(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", pathname=%p\n",
         GET_FIELD_PTR(entry, unlink, pathname));
}

void print_log_entry_truncate(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", path=%p, length=%zu\n",
         GET_FIELD_PTR(entry, truncate, path),
         GET_FIELD_PTR(entry, truncate, length));
}

void print_log_entry_user(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf("\n");
}

void print_log_entry_write(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", fd=%d, buf_addr=%p, count=%Zu\n",
         GET_FIELD_PTR(entry, write, fd),
         GET_FIELD_PTR(entry, write, buf_addr),
         GET_FIELD_PTR(entry, write, count));
}

void print_log_entry_writev(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", fd=%d, iov=%p, iovcnt=%d\n",
         GET_FIELD_PTR(entry, writev, fd),
         GET_FIELD_PTR(entry, writev, iov),
         GET_FIELD_PTR(entry, writev, iovcnt));
}

void print_log_entry_getline(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", lineptr=%d, n=%Zu, stream=%p\n",
         *(GET_FIELD_PTR(entry, getline, lineptr)),
         GET_FIELD_PTR(entry, getline, n),
         GET_FIELD_PTR(entry, getline, stream));
}

void print_log_entry_fscanf(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", stream=%p, format=%p\n",
         GET_FIELD_PTR(entry, fscanf, stream),
         GET_FIELD_PTR(entry, fscanf, format));
}

void print_log_entry_getc(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", stream=%p\n",
         GET_FIELD_PTR(entry, getc, stream));
}

void print_log_entry_fgetc(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", stream=%p\n",
         GET_FIELD_PTR(entry, fgetc, stream));
}

void print_log_entry_ungetc(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", c=%d, stream=%p\n",
         GET_FIELD_PTR(entry, ungetc, c),
         GET_FIELD_PTR(entry, ungetc, stream));
}

void print_log_entry_fopen64(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", name=%p, mode=%p\n",
         GET_FIELD_PTR(entry, fopen64, name),
         GET_FIELD_PTR(entry, fopen64, mode));
}

void print_log_entry_epoll_create(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", size=%d\n",
         GET_FIELD_PTR(entry, epoll_create, size));
}

void print_log_entry_epoll_create1(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", flags=%d\n",
         GET_FIELD_PTR(entry, epoll_create1, flags));
}

void print_log_entry_epoll_ctl(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", epfd=%d, op=%d, fd=%d, event=%p\n",
         GET_FIELD_PTR(entry, epoll_ctl, epfd),
         GET_FIELD_PTR(entry, epoll_ctl, op),
         GET_FIELD_PTR(entry, epoll_ctl, fd),
         GET_FIELD_PTR(entry, epoll_ctl, _event));
}

void print_log_entry_epoll_wait(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", epfd=%d, events=%p, maxevents:%d, timeout=%d\n",
         GET_FIELD_PTR(entry, epoll_wait, epfd),
         GET_FIELD_PTR(entry, epoll_wait, events),
         GET_FIELD_PTR(entry, epoll_wait, maxevents),
         GET_FIELD_PTR(entry, epoll_wait, timeout));
}

void print_log_entry_getpwnam_r(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", name=%p, pwd=%p, buf:%p, buflen:%zu, result:%p\n",
         GET_FIELD_PTR(entry, getpwnam_r, name),
         GET_FIELD_PTR(entry, getpwnam_r, pwd),
         GET_FIELD_PTR(entry, getpwnam_r, buf),
         GET_FIELD_PTR(entry, getpwnam_r, buflen),
         GET_FIELD_PTR(entry, getpwnam_r, result));
}

void print_log_entry_getpwuid_r(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", uid=%d, pwd=%p, buf:%p, buflen:%zu, result:%p\n",
         GET_FIELD_PTR(entry, getpwuid_r, uid),
         GET_FIELD_PTR(entry, getpwuid_r, pwd),
         GET_FIELD_PTR(entry, getpwuid_r, buf),
         GET_FIELD_PTR(entry, getpwuid_r, buflen),
         GET_FIELD_PTR(entry, getpwuid_r, result));
}

void print_log_entry_getgrnam_r(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", name=%p, grp=%p, buf:%p, buflen:%zu, result:%p\n",
         GET_FIELD_PTR(entry, getgrnam_r, name),
         GET_FIELD_PTR(entry, getgrnam_r, grp),
         GET_FIELD_PTR(entry, getgrnam_r, buf),
         GET_FIELD_PTR(entry, getgrnam_r, buflen),
         GET_FIELD_PTR(entry, getgrnam_r, result));
}

void print_log_entry_getgrgid_r(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", gid=%d, grp=%p, buf:%p, buflen:%zu, result:%p\n",
         GET_FIELD_PTR(entry, getgrgid_r, gid),
         GET_FIELD_PTR(entry, getgrgid_r, grp),
         GET_FIELD_PTR(entry, getgrgid_r, buf),
         GET_FIELD_PTR(entry, getgrgid_r, buflen),
         GET_FIELD_PTR(entry, getgrgid_r, result));
}

void print_log_entry_getaddrinfo(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", node=%p, service=%p, hints:%p, res:%p, num_res:%d\n",
         GET_FIELD_PTR(entry, getaddrinfo, node),
         GET_FIELD_PTR(entry, getaddrinfo, service),
         GET_FIELD_PTR(entry, getaddrinfo, hints),
         GET_FIELD_PTR(entry, getaddrinfo, res),
         GET_FIELD_PTR(entry, getaddrinfo, num_res));
}

void print_log_entry_freeaddrinfo(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", res:%p\n",
         GET_FIELD_PTR(entry, freeaddrinfo, res));
}

void print_log_entry_getnameinfo(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", sa:%p, salen:%d, host:%p, hostlen:%d, serv:%p, servlen:%d,"
         " flags:%d\n",
         GET_FIELD_PTR(entry, getnameinfo, sa),
         GET_FIELD_PTR(entry, getnameinfo, salen),
         GET_FIELD_PTR(entry, getnameinfo, host),
         GET_FIELD_PTR(entry, getnameinfo, hostlen),
         GET_FIELD_PTR(entry, getnameinfo, serv),
         GET_FIELD_PTR(entry, getnameinfo, servlen),
         GET_FIELD_PTR(entry, getnameinfo, flags));
}

void print_log_entry_sendto(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", sockfd:%d, buf:%p, len:%zu, flags:%d, dest_addr:%p, addrlen:%d\n",
         GET_FIELD_PTR(entry, sendto, sockfd),
         GET_FIELD_PTR(entry, sendto, buf),
         GET_FIELD_PTR(entry, sendto, len),
         GET_FIELD_PTR(entry, sendto, flags),
         GET_FIELD_PTR(entry, sendto, dest_addr),
         GET_FIELD_PTR(entry, sendto, addrlen));
}

void print_log_entry_sendmsg(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", sockfd:%d, msg:%p, flags:%d\n",
         GET_FIELD_PTR(entry, sendmsg, sockfd),
         GET_FIELD_PTR(entry, sendmsg, msg),
         GET_FIELD_PTR(entry, sendmsg, flags));
}

void print_log_entry_recvfrom(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", sockfd:%d, buf:%p, len:%zu, flags:%d, src_addr:%p, addrlen:%p\n",
         GET_FIELD_PTR(entry, recvfrom, sockfd),
         GET_FIELD_PTR(entry, recvfrom, buf),
         GET_FIELD_PTR(entry, recvfrom, len),
         GET_FIELD_PTR(entry, recvfrom, flags),
         GET_FIELD_PTR(entry, recvfrom, src_addr),
         GET_FIELD_PTR(entry, recvfrom, addrlen));
}

void print_log_entry_recvmsg(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", sockfd:%d, msg:%p, flags:%d\n",
         GET_FIELD_PTR(entry, recvmsg, sockfd),
         GET_FIELD_PTR(entry, recvmsg, msg),
         GET_FIELD_PTR(entry, recvmsg, flags));
}

void print_log_entry_waitid(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", idtype:%d, id:%d, infop:%p, options:%d\n",
         GET_FIELD_PTR(entry, waitid, idtype),
         GET_FIELD_PTR(entry, waitid, id),
         GET_FIELD_PTR(entry, waitid, infop),
         GET_FIELD_PTR(entry, waitid, options));
}

void print_log_entry_wait4(int idx, log_entry_t *entry) {
  print_log_entry_common(idx, entry);
  printf(", pid:%d, status:%p, options:%d, rusage:%p\n",
         GET_FIELD_PTR(entry, wait4, pid),
         GET_FIELD_PTR(entry, wait4, status),
         GET_FIELD_PTR(entry, wait4, options),
         GET_FIELD_PTR(entry, wait4, rusage));
}


void printEntry(int idx, log_entry_t *entry)
{
  PRINT_ENTRIES(idx, entry);
}

void rewriteLog(char *log_path)
{
  dmtcp::SynchronizationLog log;
  /* Only need enough room for the metadata. */
  log.initialize(log_path, LOG_OFFSET_FROM_START);
  size_t logSize = log.getDataSize();
  log.destroy(SYNC_IS_RECORD);
  log.initialize(log_path, logSize + LOG_OFFSET_FROM_START + 1);
  printf("Metadata: dataSize=%Zu, numEntries=%Zu\n",
         log.getDataSize(), log.numEntries());
  log_entry_t entry = EMPTY_LOG_ENTRY;
  for (size_t i = 0; i < log.numEntries(); i++) {
    if (log.getCurrentEntry(entry) == 0) {
      printf("Error reading log file. numEntries: %zu, entriesRead: %zu\n",
             log.numEntries(), i);
      exit(1);
    }
    printEntry(i, &entry);
    log.advanceToNextEntry();
  }
}


void initializeJalib()
{
  jalib::JalibFuncPtrs jalibFuncPtrs;

#define INIT_JALIB_FPTR(name) jalibFuncPtrs.name = name;

  jalibFuncPtrs.dmtcp_get_tmpdir = dmtcp_get_tmpdir;
  jalibFuncPtrs.dmtcp_get_uniquepid_str = dmtcp_get_uniquepid_str;
  jalibFuncPtrs.writeAll = dmtcp::Util::writeAll;
  jalibFuncPtrs.readAll = dmtcp::Util::readAll;

  INIT_JALIB_FPTR(open);
  INIT_JALIB_FPTR(fopen);
  INIT_JALIB_FPTR(close);
  INIT_JALIB_FPTR(fclose);

  INIT_JALIB_FPTR(syscall);
  INIT_JALIB_FPTR(mmap);
  INIT_JALIB_FPTR(munmap);

  INIT_JALIB_FPTR(read);
  INIT_JALIB_FPTR(write);
  INIT_JALIB_FPTR(select);

  INIT_JALIB_FPTR(socket);
  INIT_JALIB_FPTR(connect);
  INIT_JALIB_FPTR(bind);
  INIT_JALIB_FPTR(listen);
  INIT_JALIB_FPTR(accept);

  INIT_JALIB_FPTR(pthread_mutex_lock);
  INIT_JALIB_FPTR(pthread_mutex_trylock);
  INIT_JALIB_FPTR(pthread_mutex_unlock);

  jalib_init(jalibFuncPtrs, STDERR_FILENO, -1, 99);
  JASSERT_INIT("");
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "USAGE: %s /path/to/sync-log\n", argv[0]);
    return 1;
  }
  initializeJalib();
  rewriteLog(argv[1]);
  return 0;
}
