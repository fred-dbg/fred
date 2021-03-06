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

#include "fred_wrappers.h"
#include  "jassert.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "dmtcp.h"
#include "fred_wrappers.h"
#include "synchronizationlogging.h"
#include "jfilesystem.h"

#ifndef EXTERNC
#define EXTERNC extern "C"
#endif

typedef void (*sa_sigaction_t)(int, siginfo_t *, void *);

static dmtcp::map<int, sighandler_t> user_sig_handlers;
static dmtcp::map<int, sa_sigaction_t> user_sa_sigaction;

static inline sigset_t patchPOSIXMask(const sigset_t* mask){
  JASSERT(mask != NULL);
  sigset_t t = *mask;

  sigdelset(&t, dmtcp_get_ckpt_signal());
  return t;
}

static void sig_handler_wrapper(int sig)
{
  // FIXME: Why is the following  commented out?
  /*void *return_addr = GET_RETURN_ADDRESS();
if (!shouldSynchronize(return_addr)) {
    kill(getpid(), SIGSEGV);
    return (*user_sig_handlers[sig]) (sig);
    }*/
  if (isProcessGDB()) {
    JASSERT ( false ) .Text("don't want this");
    return (*user_sig_handlers[sig]) (sig);
  }
  int retval = 0;
  log_entry_t my_entry = create_signal_handler_entry(my_clone_id,
                                                     signal_handler_event,
                                                     sig, NULL, NULL);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY(signal_handler);
    int s = GET_FIELD(my_entry, signal_handler, savedSig);
    (*user_sig_handlers[s]) (s);
  } else if (SYNC_IS_RECORD) {
    (*user_sig_handlers[sig]) (sig);
    SET_FIELD2(my_entry, signal_handler, savedSig, sig);
    WRAPPER_LOG_WRITE_ENTRY(signal_handler);
  }
}

static void sig_sa_sigaction_wrapper(int sig, siginfo_t *info, void *data)
{
  // FIXME: Why is the following  commented out?
  /*void *return_addr = GET_RETURN_ADDRESS();
if (!shouldSynchronize(return_addr)) {
    kill(getpid(), SIGSEGV);
    return (*user_sig_handlers[sig]) (sig);
    }*/
  if (isProcessGDB()) {
    JASSERT ( false ) .Text("don't want this");
    return (*user_sa_sigaction[sig]) (sig, info, data);
  }
  int retval = 0;
  log_entry_t my_entry = create_signal_handler_entry(my_clone_id,
                                                     signal_handler_event,
                                                     sig, info, data);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY(signal_handler);
    (*user_sa_sigaction[sig]) (sig, info, data);
  } else if (SYNC_IS_RECORD) {
    (*user_sa_sigaction[sig]) (sig, info, data);
    WRAPPER_LOG_WRITE_ENTRY(signal_handler);
  }
}

//set the handler
EXTERNC sighandler_t signal(int signum, sighandler_t handler)
{
  if(signum == dmtcp_get_ckpt_signal()){
    return SIG_IGN;
  }
  WRAPPER_HEADER_RAW(sighandler_t, signal, _real_signal, signum, handler);
  // We don't need to log and replay this call, we just need to note the user's
  // signal handler so that our signal handler wrapper can call that function.
  user_sig_handlers[signum] = handler;
  return _real_signal( signum, sig_handler_wrapper );
}


EXTERNC sighandler_t sigset(int sig, sighandler_t disp)
{
  void *return_addr = GET_RETURN_ADDRESS();
  if (!shouldSynchronize(return_addr) || isProcessGDB()) {
    // Don't use our wrapper for non-user signal() calls:
    return _real_sigset (sig, disp);
  } else {
    // We don't need to log and replay this call, we just need to note the
    // user's  signal handler so that our signal handler wrapper can call that
    // function.
    user_sig_handlers[sig] = disp;
    return _real_sigset( sig, sig_handler_wrapper );
  }
}

EXTERNC int sigaction(int signum, const struct sigaction *act,
                      struct sigaction *oldact)
{
  if(signum == dmtcp_get_ckpt_signal()){
    act = NULL;
  }
  void *return_addr = GET_RETURN_ADDRESS();
  if (act != NULL && shouldSynchronize(return_addr) &&
      isProcessGDB()) {
    struct sigaction newact;
    memset(&newact, 0, sizeof(struct sigaction));
    if (act->sa_handler == SIG_DFL || act->sa_handler == SIG_IGN) {
      // Remove it from our map.
      user_sig_handlers.erase(signum);
    } else {
      // Save user's signal handler
      if (act->sa_flags & SA_SIGINFO) {
        //JASSERT ( false ).Text("Unimplemented.");
        user_sa_sigaction[signum] = act->sa_sigaction;
        newact.sa_sigaction = &sig_sa_sigaction_wrapper;
      } else {
        user_sig_handlers[signum] = act->sa_handler;
        newact.sa_handler = &sig_handler_wrapper;
      }
      // Create our own action with our own signal handler, but copy user's
      // other fields.
      newact.sa_mask = act->sa_mask;
      newact.sa_flags = act->sa_flags;
      newact.sa_restorer = act->sa_restorer;
    }
    return _real_sigaction( signum, &newact, oldact);
  } else {
    return _real_sigaction( signum, act, oldact);
  }
}

EXTERNC int sigwait(const sigset_t *set, int *sig)
{
  if (set != NULL) {
    sigset_t tmp = patchPOSIXMask(set);
    set = &tmp;
  }
  WRAPPER_HEADER(int, sigwait, _real_sigwait, set, sig);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(sigwait);
    if (sig != NULL) {
      *sig = GET_FIELD(my_entry, sigwait, ret_sig);
    }
    WRAPPER_REPLAY_END(sigwait);
  } else if (SYNC_IS_RECORD) {
    retval = _real_sigwait(set, sig);
    if (sig != NULL) {
      SET_FIELD2(my_entry, sigwait, ret_sig, *sig);
    }
    WRAPPER_LOG_WRITE_ENTRY(sigwait);
  }
  return retval;
}
