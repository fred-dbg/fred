from .. import fredutil
from .. import freddebugger
import binary_search
import undo

# MUST STILL CHECK IF n>1 HANDLED CORRECTLY.
# ALGORITHM: (Note core commands: 's', 'n', 'n' at_bkpt, 'c' at_bkpt)
#   IDEA:  expand to equivalent 's' or 's' ... 'n'.  In latter case,
#            then delete final 'n' since it's going from a level one deeper
#            back to the original level.
#   IMPLEMENTATION:
# .* 'c' at_bkpt -> .* 's' 'n'* at_bkpt   and continue with rule below
# .* 'n' level1 -> .* 's' 'n'*   until 'n' returns to level1 on stack
#       and repeat rule to force final 's', then delete 's' via rule below.
#       [ instead of repeating rule, could 
# .* 'n' at_bkpt -> NOT YET IMPLEMENTED; MUST DO AS WITH reverse_next
# .* 's' -> .*
def reverse_step(dbg, n=1):
    """Perform n 'reverse-step' commands."""
    while n > 0:
        n -= 1
        dbg.update_state()
        orig_state = dbg.state().copy()
        if dbg.branch.get_num_checkpoints() == 0:
            fredutil.fred_error("No checkpoints found for reverse-step.")
            return
        l_history = dbg.copy_current_checkpoint_history()
        while len(l_history)>0 and \
                not l_history[-1].is_step() and \
                not l_history[-1].is_next() and \
                not l_history[-1].is_continue():
            del l_history[-1]
        if len(l_history) == 0:
            break
        if l_history[-1].is_continue():
            fredutil.fred_debug("Last command continue. Expanding")
            # Since we last executed 'continue', we must be at a breakpoint.
            # Use that to expand the last 'continue' command until we
            #   reach the breakpoint.
            # FIXME:  This is inefficient.  In gdb, we should be
            #   doing 'n 2', 'n 4', 'n 8', etc., and be depending
            #   on gdb stopping at breakpoint.  But gdb doesn't always
            #   stop at breakpoint (e.g. at beginning of 'for' loop)
            del l_history[-1]
            dbg.do_restart(b_clear_history = True)
            dbg.replay_history(l_history)
            dbg.append_step_over_libc(l_history)
            while dbg.program_is_running() and not dbg.at_breakpoint():
                l_history += [dbg._p.get_personality_cmd(freddebugger.fred_next_cmd())]
                dbg.replay_history([l_history[-1]])
        fredutil.fred_debug("Done next doubling; starting search.")
        while l_history[-1].is_next():
            level = dbg.state().level()
            fredutil.fred_debug("Expanding next, level is %d" % level)
            # expand_next replaces last 'n' by ['s', 'n', ...]
            # dbg.state().level() can never increase under repeated 'n'
            # BUG:  Actually, 'n' can hit a breakpoint deeper in stack.
            testIfTooFar = lambda: dbg.state().level() < level or \
                dbg.at_breakpoint()
            (l_history, n_min) = \
                binary_search.NEW_binary_search_expand_next(dbg, l_history,
                                                            testIfTooFar)
            if not l_history[-1].is_step():
                l_history = binary_search.NEW_binary_search_history(dbg,
                                                                    l_history,
                                                                    n_min,
                                                                    testIfTooFar)
        fredutil.fred_debug("Done reverse step.")
        fredutil.fred_assert(l_history[-1].is_step())
        del l_history[-1]
    # Gene - Am I using the next four lines correctly?
    dbg.current_checkpoint().set_history(l_history)
    dbg.do_restart()
    dbg.replay_history()
    dbg.update_state()
    fredutil.fred_debug("Reverse step finished.")
