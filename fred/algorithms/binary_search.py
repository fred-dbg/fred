from .. import fredutil
from .. import freddebugger
from .. import fredmanager

"""
This file contains all algorithms for performing binary search over a
debugger session.

Each function takes as its first parameter "dbg" which should be an
instance of ReversibleDebugger, representing the debugger session on
which to perform the binary search.
"""

# This exception happens only for reverse_watch currently.
class BinarySearchTooFarAtStartError(Exception):
    pass

def _binary_search_regular_next_expansion(dbg, testIfTooFar):
    """Performs regular next expansion from the current point in time,
    no questions asked."""
    fredutil.fred_assert(not testIfTooFar())
    l_history = dbg.copy_current_checkpoint_history()
    repeatNextCmd = dbg._p.get_personality_cmd(freddebugger.fred_next_cmd())
    (l_history, n_min) = \
        NEW_binary_search_until(dbg, l_history, repeatNextCmd,
                                testIfTooFar)
    l_history = NEW_binary_search_history(dbg, l_history,
                                          n_min, testIfTooFar)
    fredutil.fred_assert(not testIfTooFar())

    while l_history[-1].is_next():
        level = dbg.state().level()
        testIfTooFar2 = \
            lambda: dbg.state().level() <= level or testIfTooFar()
        (l_history, n_min) = \
            NEW_binary_search_expand_next(dbg, l_history, testIfTooFar2)
        if len(l_history) - n_min > 1:
            l_history = NEW_binary_search_history(dbg, l_history, n_min,
                                                  testIfTooFar2)
    fredutil.fred_assert(l_history[-1].is_step())
    del l_history[-1]
    dbg.do_restart()
    dbg.replay_history(l_history)
    return l_history

def _binary_search_round_robin(dbg, s_expr, s_expr_val):
    """Perform a binary search using the "round robin" algorithm:
         For each thread Ti ever alive:
           Bring program to the beginning of Ti lifetime.
           Perform "next" expansion in Ti.
           If "next" expansion succeeds:
           |  Disable other threads from executing
           |  Execute "step" command in Ti
           |  If expression "E" changed with the step:
           |  | Thread Ti is the correct thread; exit.
           |  Else:
           |    Some other thread is the correct thread. Goto beginning.
           Else:
             Some other thread is the correct thread. Goto beginning.
    """
    testIfTooFar = lambda: dbg.test_expression(s_expr, s_expr_val)

    n_total_threads = fredmanager.get_total_threads()
    if n_total_threads <= 1:
        # Regular reverse-watch.
        l_history = dbg.copy_current_checkpoint_history()
        return NEW_binary_search_since_last_checkpoint(dbg, l_history,
                                                       0, s_expr, s_expr_val)

    for n_tid in range(1, n_total_threads+1):
        dbg.do_restart(b_clear_history=True)
        dbg.execute_until_thread(n_tid)
        # XXX: Test if the thread is a user thread. If not, skip it.
        # Use personalityGdb.within_user_code() for that test.
        fredutil.fred_assert(not testIfTooFar())
        # Regular next expansion.
        l_history = _binary_search_regular_next_expansion(dbg, testIfTooFar)
        dbg.set_scheduler_locking(True)
        fredutil.fred_assert(not testIfTooFar())
        dbg.do_step()
        if testIfTooFar():
            # This is the culprit thread.
            dbg.set_scheduler_locking(False)
            break
        dbg.set_scheduler_locking(False)

    fredutil.fred_assert(l_history[-1].is_step())
    del l_history[-1]
    dbg.do_restart()
    dbg.replay_history(l_history)
    return l_history


def _binary_search_with_log(dbg, s_expr, s_expr_val):
    # XXX: This hasn't been tested except with gdb, so I am
    # leaving this assertion until we test. -Tyler
    fredutil.fred_assert(dbg.personality_name() == "gdb")

    testIfTooFar = lambda: dbg.test_expression(s_expr, s_expr_val)
    # First, perform binary search over log events using fred_command.
    n_min = 0
    n_count = n_max = fredmanager.get_total_entries()
    fredutil.fred_assert(n_max != None)

    while n_max - n_min > 1:
        n_count = (n_min + n_max) / 2
        dbg.do_restart(b_clear_history = True)
        dbg.set_log_breakpoint(n_count)
        # XXX: This sends a SIGSTOP to the inferior. If this algorithm is ever
        # modified to need to *continue* execution after hitting a log
        # breakpoint, we must use gdb "signal 0" to continue instead of
        # "continue" so the SIGSTOP is not actually delivered to the inferior.
        dbg.do_log_continue()
        if not dbg.program_is_running() or testIfTooFar():
            fredutil.fred_debug("Setting max bound %d" % n_count)
            n_max = n_count
        else:
            fredutil.fred_debug("Setting min bound %d" % n_count)
            n_min = n_count
        # Not strictly necessary (since we restart), but cleaner:
        fredmanager.send_fred_continue()
        
    # Only use log breakpoints if there is more than one log entry to work with
    if n_max <= 1:
        return _binary_search_round_robin(dbg, s_expr, s_expr_val)

    n_culprit_tid = fredmanager.get_current_thread()
    fredutil.fred_debug("Expression changed with log event # %d "
                        "in thread %d" % (n_count, n_culprit_tid))
    # At this point, we know the expression changes value at log
    # event n_count. Restart and replay to the previous log event.
    dbg.do_restart(b_clear_history = True)
    dbg.set_log_breakpoint(n_count - 1)
    dbg.do_log_continue()
    # Switch to the "culprit" thread.
    dbg.do_switch_to_thread(n_culprit_tid)

    # Perform regular next expansion.
    return _binary_search_regular_next_expansion(dbg, testIfTooFar)

def NEW_binary_search_since_last_checkpoint(dbg, l_history, n_min,
                                            s_expr, s_expr_val):
    testIfTooFar = lambda: dbg.test_expression(s_expr, s_expr_val)
    # After replaying l_history([0:n_min]), testIfTooFar() should be False
    l_history = NEW_binary_search_history(dbg, l_history,
                                          n_min, testIfTooFar)
    # l_history[-1] now guaranteed to be 'c', 'n', or 's'
    #   and testIfTooFar changes upon executing l_history[-1]
    # Note that if we're at breakpoint and l_history[-1] == 'n',
    #   then the stack level after 'n' is same or DEEPER.  Since this acts
    #   the same as if l_history[-1] == 'c', we treat it as 'c'.
    # In gdb, since we're stopped at a breakpoint, repeated 'next' commands
    #   (generated by FReD coalesce in form  like 'n 37')
    #   can never go beyond this breakpoint.
    # Gene - SHOULD INVENT next_or_breakpoint.  FOR DEBUGGERS WITH NO
    #   COALESCING OF NEXT, USE NEXT_OR_BREAKPOINT, WHICH CHECKS FOR
    #   BREAKPOINT AFTER NEXT.  iF IT HIT A BREAKPOINT, REMOVE ALL FURTHER
    #   next_or_breakpoint COMMANDS OF l_history IMMEDIATELY AFTER THIS.
    while l_history[-1].is_continue() or \
            (dbg.at_breakpoint() and l_history[-1].is_next()):
        (l_history, n_min) = \
            NEW_binary_search_expand_continue(dbg, l_history, testIfTooFar)
        # l_history == l_history[0:n_min] + (len(l_history)-n_min)*['n']
        if len(l_history) - n_min > 1:
            l_history = NEW_binary_search_history(dbg, l_history, n_min,
                                                       testIfTooFar)

    fredutil.fred_assert(l_history[-1].is_step() or l_history[-1].is_next())
    while l_history[-1].is_next():
        # Note that the next binary search function expands ['n'].
        # If 'n' finishes a fnc, ['n'] expands to ['s'].
        # If 'n' doesn't finish a fnc and is not at fnc call, still to ['s']
        # Otherwise 'n' is at a fnc call; ['n'] expands to ['s', 'n', ...]
        # In all cases, testIfTooFar2 does the right thing.
        level = dbg.state().level()
        testIfTooFar2 = \
            lambda: dbg.state().level() <= level or testIfTooFar()
        # TODO:  Last search was NEW_binary_search_history().
        #        This stopped before the final 'next'.
        #        So, NEW_binary_search_expand_next doesn't need to
        #          do_restart() in order to expand last 'next' into 'step'.
        #        Or else, NEW_binary_search_history() didn't need to
        #          do_restart() and replay until just before final 'next'.
        #        In fact, maybe it should return l_history with undefined
        #          current time, and require next fnc to do_restart().
        (l_history, n_min) = \
            NEW_binary_search_expand_next(dbg, l_history, testIfTooFar2)
        if len(l_history) - n_min > 1:
            l_history = NEW_binary_search_history(dbg, l_history, n_min,
                                                  testIfTooFar2)

    fredutil.fred_assert(l_history[-1].is_step())
    del l_history[-1]
    dbg.do_restart()
    dbg.replay_history(l_history)
    return l_history

def NEW_binary_search_history(dbg, l_history, n_min, testIfTooFar,
                              itersToLive = -1):
    """Perform binary search on given history to identify time where
    expression changed value.  Return l_history for that point in time,
    but current time will be one cmd earlier when testIfTooFar() == False.
    If itersToLive is set, returns None if no convergence in those iters."""
    fredutil.fred_debug("Start binary search on history: %s" % str(l_history))
    n_min_orig = n_min
    n_count = n_max = len(l_history)
    # Invariant:  TestIfTooFar() is always True at n_max and False at n_min
    while n_max - n_min > 1:
        if (itersToLive == 0):
            return None
        else:
            itersToLive = itersToLive - 1
        n_count = (n_min + n_max) / 2
	# Gene - why do we need to clear the history here?
        dbg.do_restart(b_clear_history = True)
        dbg.replay_history(l_history, n_count)
        # FIX ME:  testIfTooFar() may depend on local variables on stack.
        # If stack is currently shallower, or if same function is not
        # available at corresponding call frame, then possibly
        # can declare this "not far enough" in logic below.
        # But does this still do the right thing if testIfTooFar depends
        # only on traditional global vars?
        if not dbg.program_is_running() or testIfTooFar():
            fredutil.fred_debug("Setting max bound %d" % n_count)
            n_max = n_count
        else:
            fredutil.fred_debug("Setting min bound %d" % n_count)
            n_min = n_count
    # XXX: deviate here
    fredutil.fred_assert(n_max - n_min == 1)
    # Since TestIfTooFar() changes at l_history[1], following assert holds:
    fredutil.fred_assert(l_history[n_min].is_step() or \
			     l_history[n_min].is_next() or \
			     l_history[n_min].is_continue())
    l_history = l_history[:n_max]
    if n_min != n_count:  # This was already done for n_min == n_count
        dbg.do_restart(b_clear_history = True)
        dbg.replay_history(l_history, n_min)
    if n_min == n_min_orig and \
            (not dbg.program_is_running or testIfTooFar()):
        fredutil.fred_debug("testIfTooFar() true at n_min_orig on entry.")
        raise BinarySearchTooFarAtStartError()
    fredutil.fred_assert( len(l_history) - n_min == 1 )
    fredutil.fred_debug("Done searching history.")
    return l_history

def NEW_binary_search_until(dbg, l_history, repeatCmd, testIfTooFar,
                            itersToLive = -1):
    """Let init_history = initial l_history on entry.
    On entry, current time is init_history[0:] (CHECK THIS) and expr
    will change upon executing sufficiently many iterations of repeatCmd.
    Returns (l_history, n_min)
      such that n_min >= len(init_history), n_min <= len(l_history)-1,
      and l_history ==
        init_history + len(l_history) - len(init_history))*[repeatCmd],
    Returns at point in time when testIfTooFar() == True, and
      testIfTooFar() == False after executing only l_history[0:n_min]"""
    fredutil.fred_debug("Starting expansion with '%s' on %s" % \
                            (str(repeatCmd), str(l_history)))
    n_min = len(l_history)
    l_expanded_history = [repeatCmd]
    dbg.replay_history(l_expanded_history)
    l_history += l_expanded_history
    while dbg.program_is_running() and not testIfTooFar():
        dbg.replay_history(l_expanded_history)
        n_min = len(l_history)
        l_history += l_expanded_history
        l_expanded_history += l_expanded_history
    fredutil.fred_debug("Done '%s' expansion: %s"
                        % (str(repeatCmd), str(l_history)))
    return (l_history, n_min)

# Gene - binary_search_expand_continue() is a better name.
def NEW_binary_search_expand_continue(dbg, l_history, testIfTooFar,
                                      itersToLive = -1):
    """On entry, l_history[-1] == 'c' and testIfTooFar() is True.
    Expands [..., 'c'] -> [..., 'n', ...].  Returns (l_history, n_min).
    Note that the returned l_history often goes beyond original l_history,
    while at n_min, will be at earlier point for which not testIfTooFar().
    You will probably want to call binary_search_history() afterward."""
    fredutil.fred_assert(l_history[-1].is_continue())
    # The assert above guarantees that we're at a brakpoint
    # So, repeated 'next' commands will never take us beyond current time.
    repeatNextCmd = dbg._p.get_personality_cmd(freddebugger.fred_next_cmd())
    del l_history[-1]
    dbg.do_restart(b_clear_history = True)
    dbg.replay_history(l_history)
    return NEW_binary_search_until(dbg, l_history, repeatNextCmd,
                                   testIfTooFar, itersToLive)

def NEW_binary_search_expand_next(dbg, l_history, testIfTooFar,
                                  itersToLive = -1):
    """On entry, l_history[-1] == 'n' and testIfTooFar() is True.
    Expands [..., 'n'] -> [..., 's', 'n', ...].  Returns (l_history, n_min).
    Note that the returned l_history often goes beyond original l_history,
    while at n_min, will be at earlier point for which not testIfTooFar().
    You will probably want to call binary_search_history() afterward."""
    fredutil.fred_assert(l_history[-1].is_next())
    del l_history[-1]
    n_min = len(l_history)
    dbg.do_restart(b_clear_history = True)
    dbg.replay_history(l_history)
    dbg.append_step_over_libc(l_history)
    if not dbg.program_is_running() or testIfTooFar():
        return (l_history, n_min)
    repeatNextCmd = dbg._p.get_personality_cmd(freddebugger.fred_next_cmd())
    # BUG: testIfTooFar() should also test if dbg.at_breakpoint()
    #      If that happens even once, return immediately, since original
    #      'next' command could never have gone beyond breakpoint.
    return \
        NEW_binary_search_until(dbg, l_history, repeatNextCmd, testIfTooFar)

#END OF NEW:  Will replace other methods later
#================================================================

# Gene - This method calls do_restart() before returning.
#        Is that necessary?  It makes FReD slower.
def _binary_search_checkpoints(dbg, s_expr, s_expr_val):
    """Perform binary search over checkpoints to identify interval where
    expression changes value."""
    fredutil.fred_debug("Starting binary search for checkpoint interval.")
    fredutil.fred_assert(dbg.current_checkpoint(),
        "No initial checkpoint taken.  Please start new debugging session.")
    n_right_ckpt = dbg.current_checkpoint().get_index()
    if n_right_ckpt == 0:
        fredutil.fred_debug("Only one checkpoint.")
        dbg.do_restart(n_right_ckpt)
        return
    n_left_ckpt = 0
    # Repeat until the interval is 1 checkpoint long. That means the left
    # checkpoint has the "correct" value and the right one has the
    # "incorrect" value. "Incorrect" in this case means different
    # from the "correct" value.  The start value is also guaranteed
    # different from the "correct" value.
    while (n_right_ckpt - n_left_ckpt) != 1:
        n_diff = (n_right_ckpt - n_left_ckpt) / 2
        n_new_index = int(math.ceil(n_diff) + n_left_ckpt)
        dbg.do_restart(n_new_index)
        s_expr_new_val = dbg.evaluate_expression(s_expr)
        if s_expr_new_val != s_expr_val:
            # correct
            n_left_ckpt = n_new_index
        else:
            n_right_ckpt = n_new_index
    # Now n_left_ckpt contains index of the target checkpoint.
    # Restart and return.
    fredutil.fred_debug("Found checkpoint: %d" % n_left_ckpt)
    dbg.do_restart(n_left_ckpt)

def _binary_search_history(dbg, l_history, n_min, s_expr, s_expr_val):
    """Perform binary search on given history to identify interval where
    expression changes value.  Return just before s_expr == s_expr_val."""
    fredutil.fred_debug("Start binary search on history: %s" % str(l_history))
    n_count = n_max = len(l_history)
    while n_max - n_min > 1:
        n_count = (n_min + n_max) / 2
        dbg.do_restart(b_clear_history = True)
        dbg.replay_history(l_history, n_count)
        if dbg.test_expression(s_expr, s_expr_val):
            fredutil.fred_debug("Setting max bound %d" % n_count)
            n_max = n_count
        else:
            fredutil.fred_debug("Setting min bound %d" % n_count)
            n_min = n_count
    # XXX: deviate here
    fredutil.fred_assert(n_max - n_min == 1)
    dbg.do_restart(b_clear_history = True)
    l_history = l_history[:n_max]
    dbg.replay_history(l_history, n_min)
    if n_min == 0 and dbg.test_expression(s_expr, s_expr_val):
        fredutil.fred_error("Reverse-watch failed to search history.")
        return None
    fredutil.fred_debug("Done searching history.")
    return _binary_search_expand_history(dbg, l_history, s_expr, s_expr_val)

def _binary_search_expand_history(dbg, l_history, s_expr, s_expr_val):
    """On entry, current time is history[0:-1] and expr will change upon
    executing last command, history[-1]. Last command must be 'c', 'n', or
    's'.
    Expands [..., 'c'] -> [..., 'n', ...]
        or [..., 'n'] -> [..., 's', 'n', ...]
    
    Returns history such that s_expr != s_expr_val at end of
    history, and if 's' were executed, then s_expr == s_expr_val would be
    True."""
    fredutil.fred_debug("Start expanding history: %s" % str(l_history))
    if l_history[-1].is_step():
        fredutil.fred_debug("Last command was step.")
        return l_history
    fredutil.fred_assert(l_history[-1].is_next() or \
                             l_history[-1].is_continue(),
                         "Trying to expand a last command that is not "
                         "'next' or 'continue': '%s'" % l_history[-1])
    if l_history[-1].is_continue():
        fredutil.fred_debug("Last command continue.")
        l_history = _binary_search_expand_with_next(dbg, l_history[0:-1],
                                                    s_expr, s_expr_val)
        return l_history
    while l_history[-1].is_next():
        del l_history[-1]
        dbg.append_step_over_libc(l_history)
        if dbg.test_expression(s_expr, s_expr_val):
            # Done: return debugger at time when if 's' were executed, then
            # expression would become true. We also change the final
            # command to 'step' so the rest of the call stack knows we have
            # gone as deep as possible (i.e. no further expansion is
            # possible).
            del l_history[-1]
            # FIX ME:  This can step inside libc
            l_history[-1] = dbg._p.get_personality_cmd(freddebugger.fred_step_cmd())
            dbg.current_checkpoint().set_history(l_history)
            dbg.do_restart()
            dbg.replay_history()
            break
        else:
            dbg.do_restart(b_clear_history = True)
            dbg.replay_history(l_history)
            l_history = \
                _binary_search_expand_with_next(dbg, l_history, s_expr,
                                                s_expr_val)
    return l_history

def _binary_search_expand_with_next(dbg, l_history, s_expr, s_expr_val):
    fredutil.fred_debug("Starting expansion with next on %s" % str(l_history))
    n_min = len(l_history)
    l_expanded_history = [dbg._p.get_personality_cmd(freddebugger.fred_next_cmd())]
    dbg.replay_history(l_expanded_history)
    l_history += l_expanded_history
    while dbg.program_is_running() and \
            not dbg.test_expression(s_expr, s_expr_val):
        dbg.replay_history(l_expanded_history)
        n_min = len(l_history)
        l_history += l_expanded_history
        l_expanded_history += l_expanded_history
    fredutil.fred_debug("Done next expansion: %s" % str(l_history))
    return _binary_search_history(dbg, l_history, n_min, s_expr, s_expr_val)
