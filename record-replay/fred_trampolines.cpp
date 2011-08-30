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
#include <string.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <dlfcn.h>
//#define SYSCALLWRAPPERS_H
#include "constants.h"
#include "trampolines.h"
//#include "fred_wrappers.h"
#include "jassert.h"
#include "synchronizationlogging.h"

static trampoline_info_t mmap_trampoline_info;

/* Used by _mmap_no_sync(). */
__attribute__ ((visibility ("hidden"))) __thread int mmap_no_sync = 0;

/* This could either be a normal dmtcp wrapper, or a hook function which calls
   a normal dmtcp wrapper. In this case, this is just a hook function which
   calls the real mmap wrapper (in mallocwrappers.cpp). I did it this way so
   that the real mmap wrapper could be relatively unchanged. Also, this way the
   default is to go through the regular mmap wrapper, and only if a call to
   mmap misses the wrapper does it go through the trampoline maze. */
static void *mmap_wrapper(void *addr, size_t length, int prot,
                          int flags, int fd, off_t offset)
{
  void *retval;
  if (IN_MMAP_WRAPPER || MMAP_NO_SYNC) {
    retval = _real_mmap(addr,length,prot,flags,fd,offset);
  } else {
    retval = mmap(addr,length,prot,flags,fd,offset);
  }
  return retval;
}

/* Calls to mmap will land here. */
static void *mmap_trampoline(void *addr, size_t length, int prot,
                             int flags, int fd, off_t offset)
{
  /* Interesting note: we get the arguments set up for free, since mmap is
     patched to jump directly to this function. */
  /* Unpatch mmap. */
  UNINSTALL_TRAMPOLINE(mmap_trampoline_info);
  /* Call mmap mini trampoline, which will eventually call _real_mmap. */
  void *retval = mmap_wrapper(addr,length,prot,flags,fd,offset);
  /* Repatch mmap. */
  INSTALL_TRAMPOLINE(mmap_trampoline_info);
  return retval;
}

/* Any trampolines which should be installed are done so via this function.
   Called from DmtcpWorker constructor. */
void fred_setup_trampolines()
{
  dmtcp_setup_trampoline("mmap", (void*) &mmap_trampoline,
                         &mmap_trampoline_info);
}
