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

// TODO: Better way to do this. I think it was only a problem on dekaksi.
// Remove this, and see the compile error.
#include <stdlib.h>
#include <vector>
#include <list>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/version.h>
#include <limits.h>
#include "constants.h"
#include "util.h"
#include "jassert.h"
#include "jfilesystem.h"

#include "fred_wrappers.h"
#include "synchronizationlogging.h"

extern "C" pid_t wait (__WAIT_STATUS stat_loc)
{
  dmtcp::ThreadInfo::setOkToLogNextFnc();
  return wait4(-1, stat_loc, 0, NULL);
}

extern "C" pid_t waitpid(pid_t pid, int *status, int options)
{
  dmtcp::ThreadInfo::setOkToLogNextFnc();
  return wait4(pid, status, options, NULL);
}

extern "C" int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options)
{
  WRAPPER_HEADER(int, waitid, _real_waitid, idtype, id, infop, options);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(int, waitid);
    if (retval != -1 && infop != NULL) {
      *infop = GET_FIELD(my_entry, waitid, ret_infop);
    }
    WRAPPER_REPLAY_END(waitid);
  } else if (SYNC_IS_RECORD) {
    retval = _real_waitid (idtype, id, infop, options);
    if (retval != -1 && infop != NULL) {
      SET_FIELD2(my_entry, waitid, ret_infop, *infop);
    }
    WRAPPER_LOG_WRITE_ENTRY(waitid);
  }
  return retval;
}

extern "C" pid_t wait3(__WAIT_STATUS status, int options, struct rusage *rusage)
{
  dmtcp::ThreadInfo::setOkToLogNextFnc();
  return wait4 (-1, status, options, rusage);
}

extern "C" pid_t wait4(pid_t pid, __WAIT_STATUS status, int options,
                       struct rusage *rusage)
{
  WRAPPER_HEADER(pid_t, wait4, _real_wait4, pid, status, options, rusage);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(pid_t, wait4);
    if (retval != -1 && status != NULL) {
      *(int*)status = (int) (unsigned long) GET_FIELD(my_entry, wait4, ret_status);
    }
    if (retval != -1 && rusage != NULL) {
      *rusage = GET_FIELD(my_entry, wait4, ret_rusage);
    }
    WRAPPER_REPLAY_END(wait4);
  } else if (SYNC_IS_RECORD) {
    pid_t retval = _real_wait4(pid, status, options, rusage);
    if (retval != -1 && status != NULL) {
      SET_FIELD2(my_entry, wait4, ret_status, (void*) (unsigned long) *(int*)status);
    }
    if (retval != -1 && rusage != NULL) {
      SET_FIELD2(my_entry, wait4, ret_rusage, *rusage);
    }
    WRAPPER_LOG_WRITE_ENTRY(wait4);
  }
  return retval;
}

