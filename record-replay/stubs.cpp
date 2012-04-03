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

#include "dmtcpplugin.h"
#include "constants.h"
#include "jassert.h"

// dmtcp_checkpoint, and dmtcp_coordinator, and dmtcp_command do not
//   need to load dmtcpworker.cpp
// libdmtcpinternal.a contains code needed by dmtcpworker and the utilities
//    alike.
// libnohijack.a contains stub functions (mostly empty definitions
//   corresponding to definitions in dmtcphijack.so.  It includes
//   nosyscallsreal.c and this file (dmtcpworkerstubs.cpp).
// dmtcphijack.so and libsyscallsreal.a contain the wrappers and other code
//   that executes within the end user process

// dmtcphijack.so defines this differently
void _dmtcp_setup_trampolines() {}

void dmtcp_process_event(DmtcpEvent_t id, void* data)
{
  return;
}

int  dmtcp_get_ckpt_signal()
{
  JASSERT(false) .Text ("NOT REACHED");
  return -1;
}

const char* dmtcp_get_tmpdir()
{
  // FIXME: Once the issue with fred_read_log and log.cpp:shm gets resolved, we
  // won't need the following return statement.
  return "/tmp/";
  JASSERT(false) .Text("NOT REACHED");
  return NULL;
}

const char* dmtcp_get_uniquepid_str()
{
  JASSERT(false) .Text("NOT REACHED");
  return NULL;
}

int  dmtcp_is_running_state()
{
  JASSERT(false) .Text("NOT REACHED");
  return 0;
}

int  dmtcp_get_readlog_fd()
{
  JASSERT(false) .Text("NOT REACHED");
  return -1;
}

