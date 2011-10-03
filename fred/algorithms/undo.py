from .. import fredutil
from .. import dmtcpmanager

def undo(dbg, n=1):
    """Undo the last n commands."""
    b_restart = True
    if len(dbg.l_checkpoints) == 0:
        fredutil.fred_error("No checkpoints found for undo.")
        return
    fredutil.fred_debug("Undoing %d command(s)." % n)
    while n > dbg.checkpoint.number_non_ignore_cmds():
        b_restart = False
        n -= dbg.checkpoint.number_non_ignore_cmds()
        # Back up to previous checkpoint
        if dbg.checkpoint.n_index == 0:
            fredutil.fred_error("No undo possible (empty command history "
                                "and no previous checkpoints).")
            return
        else:
            dbg.do_restart_previous()
    if b_restart:
        dbg.do_restart()
    # Trim history by n non-ignore commands
    dbg.checkpoint.trim_non_ignore(n)
    dbg.replay_history()
    # Erase everything from the future (TODO: currently no-op)
    dmtcpmanager.erase_checkpoints(dbg.checkpoint.n_index+1,
                                   len(dbg.l_checkpoints))
    del dbg.l_checkpoints[dbg.checkpoint.n_index+1:]
    fredutil.fred_debug("Done undoing %d command(s)." % n)
