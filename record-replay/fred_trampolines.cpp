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
#include "constants.h"
#include "trampolines.h"
#include "fred_wrappers.h"
#include "synchronizationlogging.h"
#include "threadinfo.h"

static trampoline_info_t mmap_trampoline_info;
static trampoline_info_t mmap64_trampoline_info;
static trampoline_info_t mremap_trampoline_info;
static trampoline_info_t munmap_trampoline_info;

/* Any trampolines which should be installed are done so via this function.
   Called from DmtcpWorker constructor. */
void fred_setup_trampolines()
{
#ifdef ENABLE_MEM_ACCURACY
  dmtcp_setup_trampoline("mmap", (void*) &fred_mmap,
                         &mmap_trampoline_info);
  dmtcp_setup_trampoline("mmap64", (void*) &fred_mmap,
                         &mmap64_trampoline_info);
  dmtcp_setup_trampoline("mremap", (void*) &fred_mremap,
                         &mremap_trampoline_info);
  dmtcp_setup_trampoline("munmap", (void*) &fred_munmap,
                         &munmap_trampoline_info);
#endif
}
