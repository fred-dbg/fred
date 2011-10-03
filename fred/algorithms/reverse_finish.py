from .. import fredutil
import binary_search
import reverse_next

def reverse_finish(dbg, n=1):
    """Perform 'reverse-finish' command."""
    dbg.update_state()
    orig_state = dbg.state().copy()
    while dbg.state().level() >= orig_state.level():
        reverse_next.reverse_next(dbg)
    dbg.update_state()
    fredutil.fred_debug("Reverse finish finished.")

def NEW_reverse_finish(dbg, n=1):
    """Perform n 'reverse-finish' commands."""
    if len(dbg.l_checkpoints) == 0:
        fredutil.fred_error("No checkpoints found for reverse-finish.")
        return
    while n > 0:
        n -= 1
        dbg.update_state()
        orig_state = dbg.state().copy()
        l_history = dbg._copy_fred_commands(dbg.checkpoint.l_history)
        level = dbg.state().level()
        while dbg.state().level() >= level:
            # Trimming ignore commands. TODO: This could delete commands
            # with side effects like "p var++".
            while len(l_history)>0 and \
                    not l_history[-1].is_step() and \
                    not l_history[-1].is_next() and \
                    not l_history[-1].is_continue():
                del l_history[-1]
            if len(l_history) == 0:
                fred_util.fred_warning("Reverse-finish:" +
                                       "  can't reverse past last checkpoint")
            if l_history[-1].is_continue():
                # expand 'c' and continue to other cases
                fredutil.fred_error("This case not yet implemented.")
                # but see comment about BUG in binary_search_expand_continue
                testIfTooFar = lambda: dbg.at_breakpoint()
                (l_history, n_min) = \
                    binary_search.NEW_binary_search_expand_continue(dbg, l_history,
                                                                    testIfTooFar)
                # l_history ==
                #     l_history[0:n_min] + (len(l_history)-n_min)*['n']
                if len(l_history) - n_min > 1:
                    l_history = binary_search.NEW_binary_search_history(dbg, l_history,
                                                                        n_min, testIfTooFar)
                # execution stopped with n_min; l_history went one further
                continue
            elif l_history[-1].is_next() and dbg.at_breakpoint():
                # 'n' can hit a breakpoint deeper in stack.
                dbg.trim_n_cmds(l_history, 1)
                dbg.do_restart()
                dbg.replay_history(l_history)
                # This can be made more efficient.
                while dbg.state().level() < level-1:
                    dbg.append_step_over_libc(l_history)
                fredutil.fred_assert(dbg.state().level() == level-1)
            elif l_history[-1].is_next():
                # Search for time in past when level was one higher.
                # This can be optimized for fewer ckpt/restarts.
                while dbg.state().level() >= level and \
                        not dbg.at_breakpoint() and len(l_history)>0 and \
                        (l_history[-1].is_next() or l_history[-1].is_step()):
                    dbg.trim_n_cmds(l_history, 1)
                    dbg.do_restart(b_clear_history = True)
                    dbg.replay_history(l_history)
                fredutil.fred_assert(dbg.state().level() >= level-1)
            elif l_history[-1].is_step():
                dbg.trim_n_cmds(l_history, 1)
                dbg.do_restart(b_clear_history = True)
                dbg.replay_history(l_history)
                dbg.update_state()
    fredutil.fred_assert(dbg.state().level() == level-1 or
                         len(l_history) == 0)
    # Gene - Am I using the next four lines correctly?
    dbg.checkpoint.l_history = l_history
    dbg.do_restart()
    dbg.replay_history()
    dbg.update_state()
    fredutil.fred_debug("Reverse finish finished.")
