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
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <utime.h>
#include <sys/time.h>
#include <net/if.h>
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
#include <dlfcn.h>

#include "constants.h"
#include "fred_types.h"

#define LIB_PRIVATE __attribute__ ((visibility ("hidden")))

#if __GLIBC_PREREQ(2,5)
# define READLINK_RET_TYPE ssize_t
#else
# define READLINK_RET_TYPE int
#endif

#include "autogen/fred_wrappers_raw.h"

#ifdef __cplusplus
extern "C"
{
#endif

  extern int fred_wrappers_initializing;
  void _dmtcp_setup_trampolines();
  void initialize_wrappers();
  void *_real_dlsym(void *handle, const char *symbol);
  void *get_real_func_addr(event_code_t e, const char *name);
  void *dmtcp_get_libc_dlsym_addr();
  void prepareFredWrappers();

  void *fred_mmap(void *addr, size_t length, int prot, int flags,
                  int fd, off_t offset);
  void *fred_mremap(void *old_address, size_t old_size,
                    size_t new_size, int flags, ...);
  int fred_munmap(void *addr, size_t length);



#ifdef __cplusplus
}
#endif

#endif
