from .. import fredutil
from .. import freddebugger
import undo
import reverse_finish

# ALGORITHM: (Note core commands: 's', 'n', 'n' at_bkpt, 'c' at_bkpt)
# .* 's' -> .* ; .* 'n' -> .*   if 'n' doesn't change stack level
# .* 'n' at_bkpt -> .*   if 'n' doesn't change stack level
# .* 'n' at_bkpt -> .* 's' 'n'* at_bkpt  if 'n' changes level, and recurse
# .* 'c' at_bkpt -> .* 's' 'n'* at_bkpt  and recurse to cases above
# .* 'n' -> reverse_finish(.*)   if 'n' returns to shallower stack level
def reverse_next(dbg, n=1):
    """Perform n 'reverse-next' commands."""
    if dbg.branch.get_num_checkpoints() == 0:
        fredutil.fred_error("No checkpoints found for reverse-next.")
        return
    while n > 0:
        n -= 1
        dbg.update_state()
        orig_state = dbg.state().copy()
        l_history = dbg.copy_current_checkpoint_history()
        while True:
            # Trimming ignore commands. TODO: This could delete commands
            # with side effects like "p var++".
            while len(l_history)>0 and \
                    not l_history[-1].is_step() and \
                    not l_history[-1].is_next() and \
                    not l_history[-1].is_continue():
                del l_history[-1]
            if len(l_history) == 0:
                break
            if l_history[-1].is_step():
                dbg.trim_n_cmds(l_history, 1)
                break
            elif l_history[-1].is_next():
                level = dbg.state().level()
                if not dbg.program_is_running():
                    level = -99
                dbg.trim_n_cmds(l_history, 1)
                dbg.do_restart(b_clear_history = True)
                dbg.replay_history(l_history)
                if dbg.state().level() == level:
                    break
                elif dbg.state().level() == level+1:
                    # The last 'n' returned from fnc; go to before fnc call
                    dbg.current_checkpoint().set_history(l_history)
                    reverse_finish.NEW_reverse_finish(dbg)
                    break
                else:
                    # The last 'n' entered fnc or exited program
                    fredutil.fred_assert(dbg.state().level() == level-1 or
                                         level == -99)

            fredutil.fred_assert(l_history[-1].is_continue() or
                                 (l_history[-1].is_next() and dbg.at_breakpoint())
                                 or level == -99)
            if True:  # if is_continue() or (is_next() and at_breakpoint())
                # if it was l_history[-1].is_next() and hit a breakpoint,
                # then it acts like 'c'.  Already undid last command for 'n'
                if l_history[-1].is_continue():
                    del l_history[-1]
                    dbg.do_restart(b_clear_history = True)
                    dbg.replay_history(l_history)
                dbg.append_step_over_libc(l_history)
                # FIXME:  This is inefficient.  In gdb, we should be
                #   doing 'n 2', 'n 4', 'n 8', etc., and be depending
                #   on gdb stopping at breakpoint.  But gdb doesn't always
                #   stop at breakpoint (e.g. at beginning of 'for' loop)
                while dbg.program_is_running() and \
                        not dbg.at_breakpoint():
                    l_history += \
                        [dbg._p.get_personality_cmd(freddebugger.fred_next_cmd())]
                    dbg.replay_history([l_history[-1]])
    # Gene - Am I using the next four lines correctly?
    dbg.current_checkpoint().set_history(l_history)
    dbg.do_restart()
    dbg.replay_history(l_history)
    dbg.update_state()
    fredutil.fred_debug("Reverse next finished.")
