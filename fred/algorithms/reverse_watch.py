from .. import fredutil
import binary_search

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
        dbg.checkpoint.l_history = \
            binary_search._binary_search_history(dbg,
              dbg._copy_fred_commands(dbg.checkpoint.l_history),
              0, s_expr, s_expr_val)
    else:
        if len(dbg.l_checkpoints) == 0:
            fredutil.fred_error("No checkpoints found for reverse-watch.")
            return
        l_history_copy = dbg._copy_fred_commands(dbg.checkpoint.l_history)
        try:
	    dbg.checkpoint.l_history = \
                binary_search.NEW_binary_search_since_last_checkpoint(dbg, l_history_copy,
                                                               0, s_expr, s_expr_val)
        except binary_search.BinarySearchTooFarAtStartError:
	    fredutil.fred_debug("BinarySearchTooFarAtStartError handled")
            fredutil.fred_info(
	        'reverse-watch failed; expr "%s"\n  is same at start and end.'
		% s_expr )
    dbg.update_state()
    fredutil.fred_debug("Reverse watch finished.")

def reverse_watch_with_log_support(dbg, s_expr):
    """Perform 'reverse-watch' with support from fred_command."""
    s_expr_val = dbg.evaluate_expression(s_expr)
    fredutil.fred_debug("RW: Starting with expr value '%s'" % s_expr_val)
    # Find starting checkpoint using binary search:
    binary_search._binary_search_checkpoints(dbg, s_expr, s_expr_val)
    dbg.checkpoint.l_history =  \
        binary_search._binary_search_with_log(dbg, s_expr, s_expr_val)
    dbg.update_state()
    fredutil.fred_debug("Reverse watch finished.")
