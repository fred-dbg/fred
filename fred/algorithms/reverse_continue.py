from .. import fredutil

def reverse_continue(dbg):
    """Perform 'reverse-continue' command. """
    dbg.update_state()
    orig_state = dbg.state().copy()
    n_to_restart = dbg.checkpoint.n_index
    n_breakpoints_found = 0
    b_finished = False
    while True:
        if b_finished:
            break
        dbg.do_restart(n_to_restart)
        # Count the number of breakpoints encountered
        for cmd in dbg.checkpoint.l_history:
            dbg.execute_fred_command(cmd)
            if dbg.at_breakpoint():
                n_breakpoints_found += 1
        # If we were at a breakpoint, when rc was issued, we want to exclude
        # it when we re-execute.
        if n_breakpoints_found > 1 and dbg.at_breakpoint() or \
                n_breakpoints_found > 0 and not dbg.at_breakpoint():
            if dbg.at_breakpoint():
                n_breakpoints_found -= 1
            n_recount = 0
            dbg.do_restart(n_to_restart)
            for i in range(0,len(dbg.checkpoint.l_history)):
                cmd = dbg.checkpoint.l_history[i]
                dbg.execute_fred_command(cmd)
                if dbg.at_breakpoint():
                    n_recount += 1
                if n_recount == n_breakpoints_found:
                    del dbg.checkpoint.l_history[i+1:]
                    b_finished = True
                    break
        else:
            n_to_restart -= 1
            if n_to_restart < 0:
                fredutil.fred_error("Reverse-continue failed.")
                break
    dbg.update_state()
    fredutil.fred_debug("Reverse continue finished.")
