from .. import fredutil
import binary_search

def _reverse_watch_main_thread(dbg, s_expr, s_expr_val):
    """Perform 'reverse-watch' command on expression, *only* in the
    main thread of the program. Return True if reverse-watch
    succeeded, or False if it failed."""
    fredutil.fred_debug("Starting reverse-watch on main thread.")
    try:
        l_history_copy = dbg.copy_current_checkpoint_history()
        l_new_history = \
        binary_search.NEW_binary_search_since_last_checkpoint(dbg,
                                                              l_history_copy,
                                                              0, s_expr,
                                                              s_expr_val)
        dbg.current_checkpoint().set_history(l_new_history)
    except binary_search.BinarySearchTooFarAtStartError:
        fredutil.fred_debug("BinarySearchTooFarAtStartError handled")
        fredutil.fred_info("reverse-watch failed; expr \"%s\""
                           "is same at start and end." % s_expr )
    fredutil.fred_assert(not dbg.test_expression(s_expr, s_expr_val))
    # Test to see if the main thread did cause expr to change:
    dbg.do_checkpoint() # Checkpoint so we can come back easily.
    dbg.set_scheduler_locking(True)
    (b_deadlock, s_output) = dbg.do_step_no_deadlock()
    if b_deadlock:
        dbg.stop_inferior()
    dbg.set_scheduler_locking(False)
    b_success = dbg.test_expression(s_expr, s_expr_val)
    # We're done, one way or another. Back up to before the step.
    dbg.do_restart(b_clear_history=True)
    dbg.update_state()
    return b_success

def _reverse_watch_round_robin(dbg, testIfTooFar):
    """Perform a round-robin reverse-watch on all threads currently
    alive. Return True if the round-robin algorithm identified the
    thread which changed the expression, or false if it failed."""
    fredutil.fred_debug("Beginning round-robin search through threads.")

    fredutil.fred_assert(not testIfTooFar())
    
    # Checkpoint so we can come back to this point easily
    n_begin_rr_ckpt = dbg.do_checkpoint()

    # Remove the main thread (already tried it above)
    l_threads = dbg.get_alive_threads()[:-1]
    # XXX: Write function to remove non-user threads from the list:
    #debugger.remove_non_user_threads(l_threads)
    b_found_thread = False
    for t in l_threads:
        dbg.do_switch_to_thread(t)
        while True:
            try:
                binary_search._binary_search_repeated_next(dbg, testIfTooFar)
            except binary_search.BinarySearchTooFarAtStartError:
                # This is the wrong thread.
                break
            except binary_search.BinarySearchFailedError:
                break
            n_after_repeat_next_ckpt = dbg.do_checkpoint()
            dbg.set_scheduler_locking(True)
            fredutil.fred_assert(not testIfTooFar())
            (b_deadlock, s_output) = dbg.do_next_no_deadlock()
            if b_deadlock:
                dbg.interrupt_inferior()
            if testIfTooFar():
                # This is the correct thread.
                dbg.set_scheduler_locking(False)
                dbg.do_restart(n_after_repeat_next_ckpt)
                b_found_thread = True
                break
            elif not testIfTooFar() and not b_deadlock:
                # This is the wrong thread.
                dbg.set_scheduler_locking(False)
                break
            # We don't yet know if this is the right thread. Replace the last
            # "next" with "step" and repeated "next", and try it all again.
            dbg.do_restart(n_after_repeat_next_ckpt, b_clear_history=True)
            dbg.do_step()
        if b_found_thread:
            break
        dbg.do_restart(n_begin_rr_ckpt, b_clear_history=True)
    dbg.update_state()
    return b_found_thread

def reverse_watch_for_mt(dbg, s_expr):
    """Perform 'reverse-watch' command on expression, with support for
    an expression changing in a thread which is not the main thread."""
    if dbg.branch.get_num_checkpoints() == 0:
        fredutil.fred_error("No checkpoints found for reverse-watch.")
        return
    
    s_expr_val = dbg.evaluate_expression(s_expr)
    testIfTooFar = lambda: dbg.test_expression(s_expr, s_expr_val)
    fredutil.fred_debug("RW: Starting with expr value '%s'" % s_expr_val)

    # ---------------------------- Binary search of checkpoints
    if dbg.branch.get_num_checkpoints() > 1:
        # Find starting checkpoint using binary search:
        binary_search._binary_search_checkpoints(dbg, s_expr, s_expr_val)
        
    # ---------------------------- Binary search on the main thread
    if _reverse_watch_main_thread(dbg, s_expr, s_expr_val):
        fredutil.fred_debug("Main thread changed expression.")
        fredutil.fred_debug("Reverse-watch finished.")
        fredutil.fred_assert(not testIfTooFar())
        return
    
    # If we're here, it means a secondary thread caused expr to change.
    fredutil.fred_debug("Secondary thread caused expr to change. "
                        "Starting binary search of system calls.")
    
    # ---------------------------- Binary search through system calls
    binary_search.binary_search_log_events(dbg, testIfTooFar)
    # We are now at a point in time where at some point after the next
    # system call the expression will change. We are guaranteed that
    # the culprit thread is alive, because a "pthread_create" call is
    # an event in the system call log, and the binary search will
    # consider that in determining the lower bound.

    # ---------------------------- Round-robin search for culprit thread
    if not _reverse_watch_round_robin(dbg, testIfTooFar):
        fredutil.fred_error("Reverse-watch failed to determine the thread "
                            "which caused the expression '%s' to change." %
                            s_expr)
        return
            
    # At this point, whatever thread we are currently in is the correct thread.
    fredutil.fred_debug("Identified thread %d as the one changing expr." %
                        dbg.get_current_thread())
    fredutil.fred_assert(not testIfTooFar())
    dbg.current_checkpoint().set_history(
        binary_search._binary_search_regular_next_expansion(dbg, testIfTooFar))
    dbg.update_state()
    fredutil.fred_assert(not testIfTooFar())
    fredutil.fred_debug("Reverse watch finished.")
    return

def reverse_watch(dbg, s_expr):
    """Perform 'reverse-watch' command on expression.
    'dbg' is the instance of ReversibleDebugger on which to operate."""
    s_expr_val = dbg.evaluate_expression(s_expr)
    fredutil.fred_debug("RW: Starting with expr value '%s'" % s_expr_val)
    # Find starting checkpoint using binary search:
    binary_search._binary_search_checkpoints(dbg, s_expr, s_expr_val)

    # STILL TESTING:  When "else" branch works well, it will become permanent,
    #  and the remaining branch of "if" can be removed.
    if False:
        dbg.current_checkpoint().set_history(
            binary_search._binary_search_history(dbg,
              dbg.copy_current_checkpoint_history(),
              0, s_expr, s_expr_val))
    else:
        if dbg.branch.get_num_checkpoints() == 0:
            fredutil.fred_error("No checkpoints found for reverse-watch.")
            return
        l_history_copy = dbg.copy_current_checkpoint_history()
        try:
	    dbg.current_checkpoint().set_history(
                binary_search.NEW_binary_search_since_last_checkpoint(dbg, l_history_copy,
                                                               0, s_expr, s_expr_val))
        except binary_search.BinarySearchTooFarAtStartError:
	    fredutil.fred_debug("BinarySearchTooFarAtStartError handled")
            fredutil.fred_info(
	        'reverse-watch failed; expr "%s"\n  is same at start and end.'
		% s_expr )
    dbg.update_state()
    fredutil.fred_debug("Reverse watch finished.")
