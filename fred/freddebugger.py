###############################################################################
# Copyright (C) 2009, 2010, 2011, 2012 by Kapil Arya, Gene Cooperman,         #
#                                        Tyler Denniston, and Ana-Maria Visan #
# {kapil,gene,tyler,amvisan}@ccs.neu.edu                                      #
#                                                                             #
# This file is part of FReD.                                                  #
#                                                                             #
# FReD is free software: you can redistribute it and/or modify                #
# it under the terms of the GNU General Public License as published by        #
# the Free Software Foundation, either version 3 of the License, or           #
# (at your option) any later version.                                         #
#                                                                             #
# FReD is distributed in the hope that it will be useful,                     #
# but WITHOUT ANY WARRANTY; without even the implied warranty of              #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               #
# GNU General Public License for more details.                                #
#                                                                             #
# You should have received a copy of the GNU General Public License           #
# along with FReD.  If not, see <http://www.gnu.org/licenses/>.               #
###############################################################################

import math
import time
import pdb

# Note we don't import fredio here. We should not load fredio unless absolutely
# necessary. This helps preserve modularity. Typically anything you want to do
# here with fredio should in fact be done in personality.py or
# personalityGdb.py.
import dmtcpmanager
import fredutil

# Use "--enable-debug" CLI flag to see debugging messages.
# Convention for variable names:
#    list: l_XXX, string: s_XXX, number: n_XXX, boolean: b_XXX
#    python dictionary: d_XXX
# Classes:  Debugger, ReversibleDebugger(Debugger), DebuggerState(),
#	    Breakpoint, Backtrace, BacktraceFrame, FredCommand, Checkpoint
# Subclasses:
#    ReversibleDebugger is subclass of Debugger.
# Constructors:
#    Debugger and ReversibleDebugger constructors require personality argument.
#    Checkpoint constructor requires checkpoint index argument.
#    FredCommand constructor requires name and args arguments.
# Class instance variables:
#    Debugger:       _p (personality), _state (DebuggerState)
#      ReversibleDebugger: checkpoint (current ckpt), l_checkpoints (all ckpts)
#    DebuggerState:  backtrace, l_breakpoints
#    Breakpoint:  n_number, s_type, s_display, s_enable, s_address,
#		  s_function, s_file, n_line, n_count
#    Backtrace:  l_frames
#    BacktraceFrame:  n_frame_num, s_addr, s_function, s_args, s_file, n_line
#    FredCommand:  s_name, s_args, s_native, b_ignore, b_count_cmd,
#    Checkpoint:  n_index (index into l_checkpoints), l_history (since ckpt)

# NOTE: This code does not yet handle search inside gdb's 'finish'
#   and 'until' commands.  These commands can be replayed, but not expanded
#   into smaller commands.
# BUG:  In gdb, if one sets a breakpoint at a "for" statement, continue will
#   stop at the "for" statement, but 'next 9' will not.  This is arguably
#   a bug in gdb, since 'next 9' will not go past other breakpoints.
#   One solution is to execute 'step', 'break' several times inside
#   binary_search_expand_continue, and then set those same breakpoints
#   at the beginning of each probe: ['b extra1', 'b extra2', 's', 'n', ...]
#   to create backup breakpoints in case we go too far.  Then inspecting which
#   breakpoint we landed at tells us where we went too far.

class Debugger():
    """Represents control and management of an actual debugger.

    This provides a consistent interface to different debuggers, based on the
    particular Personality instance. Each Personality instance has a
    well-defined set of required functions, and this class calls those.

    If the semantics of personality-specific things change, we may change the
    usage here, and keep the interface unchanged."""
    def __init__(self, personality):
	# Gene - Can we change the name _p to _personality ??
	#  Then methods like get_personality_cmd can be abbreviated to get_cmd
        self._p     = personality
        self._state = DebuggerState()

    def personality_name(self):
        """Return the name of the personality."""
        return self._p.s_name

    def _next(self, n):
        """Perform n 'next' commands. Returns output."""
        return self._p.do_next(n)
        
    def _step(self, n):
        """Perform n 'step' commands. Returns output."""
        return self._p.do_step(n)
        
    def _continue(self, n):
        """Perform n 'continue' commands. Returns output."""
        return self._p.do_continue(n)
        
    def _breakpoint(self, expr):
        """Perform 'break expr' command. Returns output."""
        return self._p.do_breakpoint(expr)

    def _where(self):
        """Perform 'where' command. Returns output."""
        return self._p.do_where()

    def _info_breakpoints(self):
        """Perform 'info_breakpoints' command. Returns output."""
        return self._p.do_info_breakpoints()

    def _print(self, expr):
        """Perform 'print expr' command. Returns output."""
        return self._p.do_print(expr)

    def current_position(self):
        """Return a BacktraceFrame representing current debugger position."""
        return self._p.current_position()
    
    def at_breakpoint(self):
        """Return True if debugger is currently on a breakpoint."""
        bt_frame = self._p.current_position()
        self.update_state()
        return self._p.at_breakpoint(bt_frame, self.state().get_breakpoints())

    def state(self):
        """Return the DebuggerState representing the current state of
        the debugger."""
        return self._state

    def update_state(self):
        """Update the underlying DebuggerState."""
        fredutil.fred_debug("Updating DebuggerState.")
        self.state().set_backtrace(self._p.get_backtrace())
        self.state().set_breakpoints(self._p.get_breakpoints())

    def get_find_prompt_function(self):
        """Return the 'contains_prompt_str' function from the personality."""
        return self._p.contains_prompt_str

    def get_prompt_string_function(self):
        """Return the 'prompt_string' function from the personality."""
        return self._p.prompt_string

    def get_prompt_regex(self):
        """Return a regex from the personality that will match the prompt."""
        return self._p.gre_prompt

    def get_ls_needs_input(self):
        """Return a list of regexes from the personality that match lines
        requesting additional user input."""
        return self._p.ls_needs_user_input
    
    def prompt(self):
        """Bring user back to debugger prompt."""
        self._p.prompt()

    def program_is_running(self):
        """Return True if inferior is still running."""
	# The extra debugging functions are gdb-specific.  When a gdb
	#  target app exits, it first returns to the call frames below.
        if self.personality_name() == "gdb":
            return self._p.program_is_running() and \
                self._p.current_position().s_function != "_start" and \
                self._p.current_position().s_function != "__libc_start_main"
        else:
            return self._p.program_is_running()

class ReversibleDebugger(Debugger):
    """Represents control and management of a reversible Debugger.

    This class knows about checkpoints, command histories, and reversible
    debugger commands.

    It contains an instance of Checkpoint which should always represent the
    current checkpoint.  It also contains a list of all Checkpoint objects
    such that Checkpoint.n_index refers to the index into that list.
    """
    def __init__(self, personality):
        Debugger.__init__(self, personality)
        self.checkpoint = None
        self.l_checkpoints = []

    def destroy(self):
        """Perform any cleanup associated with a ReversibleDebugger inst."""
        # XXX: Should get rid of this Checkpoint variable.
        Checkpoint.n_next_index = 0
        self._p.destroy()

    def setup_from_resume(self):
        """Set up data structures from a resume."""
        for i in range(0, dmtcpmanager.numCheckpoints):
            self.l_checkpoints.append(Checkpoint())
        self.checkpoint = self.l_checkpoints[-1]
        self.update_state()

    def do_checkpoint(self):
        """Perform a new checkpoint."""
        new_ckpt = Checkpoint()
        self.checkpoint = new_ckpt
        self.l_checkpoints.append(new_ckpt)
        dmtcpmanager.checkpoint()
        fredutil.fred_info("Created checkpoint #%d." % new_ckpt.n_index)

    def reset_on_restart(self):
        """Perform any reset functions that should happen on restart."""
        if self.personality_name() == "gdb":
            self._p.execute_command("signal SIGKILL")
            self._p.reset_user_code_interval()

    def do_restart(self, n_index=-1, b_clear_history=False):
        """Restart from the current or specified checkpoint.
        n_index defaults to -1, which means restart from current checkpoint."""
        if len(self.l_checkpoints) == 0:
            fredutil.fred_error("No checkpoints found for restart.")
            return
        self.reset_on_restart()
        if n_index == -1:
            fredutil.fred_debug("Restarting from checkpoint index %d." % \
                                self.checkpoint.n_index)
            dmtcpmanager.restart(self.checkpoint.n_index)
        else:
            if n_index > len(self.l_checkpoints) - 1:
                fredutil.fred_error("No such checkpoint index %d." % n_index)
                return
            fredutil.fred_debug("Restarting from checkpoint index %d." % \
                                n_index)
            dmtcpmanager.restart(n_index)
            self.checkpoint = self.l_checkpoints[n_index]
        if b_clear_history:
            self.clear_history()
        self.update_state()

    def do_restart_previous(self):
        """Restart from the previous checkpoint."""
        self.do_restart(self.checkpoint.n_index - 1)
        
    def list_checkpoints(self):
        """Return the list of available Checkpoint files."""
        return self.l_checkpoints
    
    # Gene - bad name?  Maybe checkpoint_history() or ckpt_history() ?
    def history(self):
        """Return the history of all Checkpoints."""
        l_history = []
        if self.checkpoint != None:
            for ckpt in self.l_checkpoints:
                l_history.append("*ckpt*")
                l_history.extend(ckpt.l_history)
        return l_history

    # Gene - see above.  clear_ckpt_history() might be a better name.
    def clear_history(self):
        """Clear the current checkpoint's history."""
        del self.checkpoint.l_history[:]            
    
    def log_command(self, s_command):
        """Convert given command to FredCommand instance and add to current
        history."""
        if self.checkpoint != None:
            # identify_command() sets native representation
            cmd = self._p.identify_command(s_command)
            self.checkpoint.log_command(cmd)

    def log_fred_command(self, cmd):
        """Directly log the given FredCommand instance."""
        if self.checkpoint != None:
            self.checkpoint.log_command(cmd)
    
    def execute_fred_command(self, cmd, b_update=True):
        """Execute the given FredCommand."""
        if cmd.s_native == "":
            fredutil.fred_fatal("FredCommand instance has null native string.")
        elif cmd.b_ignore:
            fredutil.fred_debug("Skipping ignore command '%s'" % \
                                (cmd.s_native + " " + cmd.s_args))
            return
        self._p.execute_command(cmd.s_native + " " + cmd.s_args + "\n")
        if b_update:
            self.update_state()

    def do_next(self, n=1):
        """Perform n 'next' commands. Returns output."""
        cmd = fred_next_cmd()
        cmd.set_native(self._p.get_native(cmd))
        cmd.set_count_cmd(self._p.b_has_count_commands)
        cmd.set_count(n)
        self.log_fred_command(cmd)
        output = self._next(n)
        self.update_state()
        return output
        
    def do_step(self, n=1):
        """Perform n 'step' commands. Returns output."""
        cmd = fred_step_cmd()
        cmd.set_native(self._p.get_native(cmd))
        cmd.set_count_cmd(self._p.b_has_count_commands)
        cmd.set_count(n)
        # TODO: Special case for gdb so we don't step into libc. Think of
        # more portable way to do this.
        output = self._step(n)
        if output == "DO-NOT-STEP":
            # Log a next instead of step so we don't step into libc again.
            cmd = fred_next_cmd()
            cmd.set_native(self._p.get_native(cmd))
            cmd.set_count_cmd(self._p.b_has_count_commands)
            cmd.set_count(1)
        self.log_fred_command(cmd)
        self.update_state()
        return output

    def append_step_over_libc(self, l_history):
	"""Append fred_step_cmd() to l_history.  if this would enter glibc
	or some other other lib, replace that 'step' by a special cmd
	with name 'step' that acts when executed like 'next'."""
        l_history.append(self._p.get_personality_cmd(fred_step_cmd()))
        # Hack (Ana): MATLAB was failing to expand a command into subcommands.
        # Reason: Before we expand we do a "step"; without re-executing it,
        # MATLAB was considering its job to be done.
        # I want to keep this in - until we learn more more about the bug.
        if self.personality_name() == "MATLAB":
            self.replay_history(l_history)
	# NOTE: do_step() logs "step", but we replace history.
        if self.do_step() == "DO-NOT-STEP":
	    l_history[-1].set_native(self._p.get_native(fred_next_cmd()))
	    # undo "step" and then replay "next"
            self.do_restart(b_clear_history = True)
            self.replay_history(l_history)
	return l_history
        
    def do_continue(self, n=1):
        """Perform n 'continue' commands. Returns output."""
        cmd = fred_continue_cmd()
        cmd.set_native(self._p.get_native(cmd))
        cmd.s_args = str(n)
        self.log_fred_command(cmd)
        output = self._continue(n)
        self.update_state()
        return output
        
    def do_breakpoint(self, expr):
        """Perform 'break expr' command. Returns output."""
        cmd = fred_breakpoint_cmd()
        cmd.set_native(self._p.get_native(cmd))
        cmd.s_args = expr
        self.log_fred_command(cmd)
        output = self._breakpoint(expr)
        self.update_state()
        return output
    
    def do_print(self, expr):
        """Perform 'print expr' command. Returns output."""
        # TODO: We should log some print statements, since they can contain
        # side effects. Example: "print var++"
        return self._print(expr)

    def _copy_fred_commands(self, l_cmds):
        """Perform a deep copy on the given list of FredCommands."""
        l_result = []
        for cmd in l_cmds:
            l_result.append(cmd.copy())
        return l_result

    def _coalesce_history(self, l_history):
        """Return a modified version of l_history with as much condensing done
        as possible. Example: [n,n,n,n,n] => [n 5]."""
        if not self._p.b_coalesce_support or len(l_history) == 0:
            return l_history
        l_result = self._copy_fred_commands(l_history)
        i = j = 0
        while i < len(l_result):
            if not l_result[i].b_count_cmd:
                i += 1
                continue
            n_count = l_result[i].count()
            j = i + 1
            while j < len(l_result):
                if l_result[j].s_name == l_result[i].s_name:
                    n_count += l_result[j].count()
                else:
                    break
                j += 1
            if n_count > 1:
                del l_result[i+1:i+n_count]
                l_result[i].set_count(n_count)
            i += 1
        return l_result

    def replay_history(self, l_history=[], n=-1):
        """Issue the commands in given or current checkpoint's history to
        debugger."""
        if len(l_history) == 0:
            l_history = self._copy_fred_commands(self.checkpoint.l_history)
        if n == -1:
            l_temp = self._coalesce_history(l_history)
        else:
            l_temp = self.first_n_commands(self._coalesce_history(l_history), n)
        fredutil.fred_debug("Replaying the following history: %s" % \
                            str(l_temp))
        for cmd in l_temp:
            self.execute_fred_command(cmd, b_update=False)
        self.update_state()

    def first_n_commands(self, l_history, n):
        """Return the first 'n' commands from given history."""
        # TODO: Clean this up a bit.
        l_result = []
        i = j = 0
        while i < n:
            # Make sure we do a copy of the command.
            l_result.append(l_history[j].copy())
            if l_history[j].b_count_cmd:
                count = l_history[j].count()
                if count != 1:
                    if i + count < n:
                        i += count
                        j += 1
                        continue
                    else:
                        l_result[-1].set_count(n - i)
                        break
            i += 1
            j += 1
        return l_result

    def trim_n_cmds(self, l_history, n):
        """Trim last n commands.
        Also adjust things like 'next 5' to be 'next 4'."""
        if len(l_history) == 0:
            return
        while n > 0:
            if l_history[-1].b_count_cmd:
                n_count = l_history[-1].count()
                if n_count != 1:
                    l_history[-1].set_count(n_count - 1)
                    n -= 1
                    continue
            n -= 1
            l_history.pop()

    def undo(self, n=1):
        """Undo the last n commands."""
        b_restart = True
        if len(self.l_checkpoints) == 0:
            fredutil.fred_error("No checkpoints found for undo.")
            return
        fredutil.fred_debug("Undoing %d command(s)." % n)
        while n > self.checkpoint.number_non_ignore_cmds():
            b_restart = False
            n -= self.checkpoint.number_non_ignore_cmds()
            # Back up to previous checkpoint
            if self.checkpoint.n_index == 0:
                fredutil.fred_error("No undo possible (empty command history "
                                    "and no previous checkpoints).")
                return
            else:
                self.do_restart_previous()
        if b_restart:
            self.do_restart()
        # Trim history by n non-ignore commands
        self.checkpoint.trim_non_ignore(n)
        self.replay_history()
        # Erase everything from the future (TODO: currently no-op)
        dmtcpmanager.erase_checkpoints(self.checkpoint.n_index+1,
                                       len(self.l_checkpoints))
        del self.l_checkpoints[self.checkpoint.n_index+1:]
        fredutil.fred_debug("Done undoing %d command(s)." % n)

    def reverse_next(self, n=1):
        """Perform n 'reverse-next' commands."""
        while n > 0:
            n -= 1
            self.update_state()
            orig_state = self.state().copy()
            debug_loop_counter = 0
            while True:
                debug_loop_counter += 1
                fredutil.fred_debug("RN: LOOP ITERATION %d" % debug_loop_counter)
                if self.state().level() > orig_state.level():
                    fredutil.fred_debug("RN: DEEPER")
                    self.do_next()
                elif self.state().level() < orig_state.level():
                    fredutil.fred_debug("RN: SHALLOWER")
                    self.do_step()
                else:
                    fredutil.fred_debug("RN: SAME")
                    if self.state() == orig_state:
                        fredutil.fred_debug("RN: AT ORIG STATE")
                        if self.checkpoint.last_command_non_ignore().is_next() or \
                               self.checkpoint.last_command_non_ignore().is_step():
                            fredutil.fred_debug("RN: AFTER NEXT OR STEP")
			    # Gene - n_lvl is apparently never used.
                            n_lvl = self.state().level()
                            self.undo()
                            break
                        else:
                            fredutil.fred_debug("RN: NOT AFTER NEXT OR STEP")
                        self.undo()
                    else:
                        fredutil.fred_debug("RN: NOT AT ORIG STATE")
                        self.do_next()
        self.update_state()
        fredutil.fred_debug("Reverse next finished.")

    def reverse_step(self, n=1):
        """Perform n 'reverse-step' commands."""
        while n > 0:
            n -= 1
            self.update_state()
            orig_state = self.state().copy()
            debug_loop_counter = 0
            while True:
                debug_loop_counter += 1
                fredutil.fred_debug("RS: LOOP ITERATION %d" % debug_loop_counter)
                if self.state().level() > orig_state.level():
                    fredutil.fred_debug("RS: DEEPER")
                    self.do_next()
                    if self.state() == orig_state:
                        self.undo()
                        break
                elif self.state().level() < orig_state.level():
                    fredutil.fred_debug("RS: SHALLOWER")
                    self.do_step()
                else:
                    fredutil.fred_debug("RS: SAME")
                    if self.state() == orig_state:
                        fredutil.fred_debug("RS: AT ORIG STATE")
                        if self.checkpoint.last_command_non_ignore().is_step():
                            fredutil.fred_debug("RS: AFTER STEP")
                            self.undo()
                            break
                        else:
                            fredutil.fred_debug("RS: NOT AFTER STEP")
                            self.undo()
                            self.do_step()
                    else:
                        fredutil.fred_debug("RS: NOT AT ORIG STATE")
                        # TODO: This is a very inefficient implementation.
                        self.do_step()
                        if self.state() == orig_state:
                            continue
                        self.undo()
                        self.do_next()
                    
        self.update_state()
        fredutil.fred_debug("Reverse step finished.")

    def reverse_finish(self, n=1):
        """Perform 'reverse-finish' command."""
        self.update_state()
        orig_state = self.state().copy()
        while self.state().level() >= orig_state.level():
            self.reverse_next()
        self.update_state()
        fredutil.fred_debug("Reverse finish finished.")
        
    def reverse_continue(self):
        """Perform 'reverse-continue' command. """
        self.update_state()
        orig_state = self.state().copy()
        n_to_restart = self.checkpoint.n_index
        n_breakpoints_found = 0
        b_finished = False
        while True:
            if b_finished:
                break
            self.do_restart(n_to_restart)
            # Count the number of breakpoints encountered
            for cmd in self.checkpoint.l_history:
                self.execute_fred_command(cmd)
                if self.at_breakpoint():
                    n_breakpoints_found += 1
            # If we were at a breakpoint, when rc was issued, we want to exclude
            # it when we re-execute.
            if n_breakpoints_found > 1 and self.at_breakpoint() or \
               n_breakpoints_found > 0 and not self.at_breakpoint():
                if self.at_breakpoint():
                    n_breakpoints_found -= 1
                n_recount = 0
                self.do_restart(n_to_restart)
                for i in range(0,len(self.checkpoint.l_history)):
                    cmd = self.checkpoint.l_history[i]
                    self.execute_fred_command(cmd)
                    if self.at_breakpoint():
                        n_recount += 1
                    if n_recount == n_breakpoints_found:
                        del self.checkpoint.l_history[i+1:]
                        b_finished = True
                        break
            else:
                n_to_restart -= 1
                if n_to_restart < 0:
                    fredutil.fred_error("Reverse-continue failed.")
                    break
        self.update_state()
        fredutil.fred_debug("Reverse continue finished.")
        
    # This exception happens only for reverse_watch currently.
    class BinarySearchTooFarAtStartError(Exception):
        pass

    def reverse_watch(self, s_expr):
        """Perform 'reverse-watch' command on expression."""
        s_expr_val = self.evaluate_expression(s_expr)
        fredutil.fred_debug("RW: Starting with expr value '%s'" % \
                            s_expr_val)
        # Find starting checkpoint using binary search:
        self._binary_search_checkpoints(s_expr, s_expr_val)

# STILL TESTING:  When "else" branch works well, it will become permanent,
#  and the remaining branch of "if" can be removed.
	if False:
          self.checkpoint.l_history = \
            self._binary_search_history(
		self._copy_fred_commands(self.checkpoint.l_history),
                0, s_expr, s_expr_val)
	else:
          if len(self.l_checkpoints) == 0:
              fredutil.fred_error("No checkpoints found for reverse-watch.")
              return
	  l_history_copy = self._copy_fred_commands(self.checkpoint.l_history)
	  try:
	    self.checkpoint.l_history = \
	      self.NEW_binary_search_since_last_checkpoint(l_history_copy,
							  0, s_expr, s_expr_val)
	  except self.BinarySearchTooFarAtStartError:
	    fredutil.fred_debug("BinarySearchTooFarAtStartError handled")
            fredutil.fred_info(
	        'reverse-watch failed; expr "%s"\n  is same at start and end.'
		% s_expr )
        self.update_state()
        fredutil.fred_debug("Reverse watch finished.")

    #====
    #NEW:  Will replace other methods later

    def NEW_binary_search_since_last_checkpoint(self,
					 l_history, n_min, s_expr, s_expr_val):
        testIfTooFar = lambda: self.test_expression(s_expr, s_expr_val)
	# After replaying l_history([0:n_min]), testIfTooFar() should be False
        l_history = self.NEW_binary_search_history(l_history,
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
	      (self.at_breakpoint() and l_history[-1].is_next()):
	    (l_history, n_min) = \
		self.NEW_binary_search_expand_continue(l_history, testIfTooFar)
	    # l_history == l_history[0:n_min] + (len(l_history)-n_min)*['n']
	    if len(l_history) - n_min > 1:
        	l_history = self.NEW_binary_search_history(l_history, n_min,
							   testIfTooFar)

        fredutil.fred_assert(l_history[-1].is_step() or l_history[-1].is_next())
	while l_history[-1].is_next():
	    # Note that the next binary search function expands ['n'].
	    # If 'n' finishes a fnc, ['n'] expands to ['s'].
	    # If 'n' doesn't finish a fnc and is not at fnc call, still to ['s']
	    # Otherwise 'n' is at a fnc call; ['n'] expands to ['s', 'n', ...]
	    # In all cases, testIfTooFar2 does the right thing.
            level = self.state().level()
	    testIfTooFar2 = \
		lambda: self.state().level() <= level or testIfTooFar()
	    # TODO:  Last search was NEW_binary_search_history().
	    #        This stopped before the final 'next'.
	    #        So, NEW_binary_search_expand_next doesn't need to
	    #          do_restart() in order to expand last 'next' into 'step'.
	    #        Or else, NEW_binary_search_history() didn't need to
	    #          do_restart() and replay until just before final 'next'.
	    #        In fact, maybe it should return l_history with undefined
	    #          current time, and require next fnc to do_restart().
	    (l_history, n_min) = \
		self.NEW_binary_search_expand_next(l_history, testIfTooFar2)
	    if len(l_history) - n_min > 1:
                l_history = self.NEW_binary_search_history(l_history, n_min,
						           testIfTooFar2)

        fredutil.fred_assert(l_history[-1].is_step())
	del l_history[-1]
        self.do_restart()
        self.replay_history(l_history)
	return l_history

    def NEW_binary_search_history(self, l_history, n_min, testIfTooFar,
				  itersToLive = -1):
        """Perform binary search on given history to identify time where
        expression changed value.  Return l_history for that point in time,
	but current time will be one cmd earlier when testIfTooFar() == False.
	If itersToLive is set, returns None if no convergence in those iters."""
        fredutil.fred_debug("Start binary search on history: %s" % \
                            str(l_history))
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
            self.do_restart(b_clear_history = True)
            self.replay_history(l_history, n_count)
            # FIX ME:  testIfTooFar() may depend on local variables on stack.
            # If stack is currently shallower, or if same function is not
            # available at corresponding call frame, then possibly
            # can declare this "not far enough" in logic below.
            # But does this still do the right thing if testIfTooFar depends
            # only on traditional global vars?
	    if not self.program_is_running() or testIfTooFar():
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
            self.do_restart(b_clear_history = True)
            self.replay_history(l_history, n_min)
        if n_min == n_min_orig and \
	   (not self.program_is_running or testIfTooFar()):
            fredutil.fred_debug("testIfTooFar() true at n_min_orig on entry.")
            raise self.BinarySearchTooFarAtStartError()
        fredutil.fred_assert( len(l_history) - n_min == 1 )
        fredutil.fred_debug("Done searching history.")
	return l_history

    def NEW_binary_search_until(self, l_history, repeatCmd, testIfTooFar,
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
        self.replay_history(l_expanded_history)
        l_history += l_expanded_history
        while self.program_is_running() and not testIfTooFar():
            self.replay_history(l_expanded_history)
            n_min = len(l_history)
            l_history += l_expanded_history
            l_expanded_history += l_expanded_history
        fredutil.fred_debug("Done '%s' expansion: %s"
			    % (str(repeatCmd), str(l_history)))
	return (l_history, n_min)

    # Gene - binary_search_expand_continue() is a better name.
    def NEW_binary_search_expand_continue(self, l_history, testIfTooFar,
					  itersToLive = -1):
        """On entry, l_history[-1] == 'c' and testIfTooFar() is True.
	Expands [..., 'c'] -> [..., 'n', ...].  Returns (l_history, n_min).
	Note that the returned l_history often goes beyond original l_history,
	while at n_min, will be at earlier point for which not testIfTooFar().
	You will probably want to call binary_search_history() afterward."""
        fredutil.fred_assert(l_history[-1].is_continue())
	# The assert above guarantees that we're at a brakpoint
	# So, repeated 'next' commands will never take us beyond current time.
	repeatNextCmd = self._p.get_personality_cmd(fred_next_cmd())
	del l_history[-1]
        self.do_restart(b_clear_history = True)
        self.replay_history(l_history)
	return self.NEW_binary_search_until(l_history, repeatNextCmd,
					    testIfTooFar, itersToLive)

    def NEW_binary_search_expand_next(self, l_history, testIfTooFar,
                                      itersToLive = -1):
        """On entry, l_history[-1] == 'n' and testIfTooFar() is True.
	Expands [..., 'n'] -> [..., 's', 'n', ...].  Returns (l_history, n_min).
	Note that the returned l_history often goes beyond original l_history,
	while at n_min, will be at earlier point for which not testIfTooFar().
	You will probably want to call binary_search_history() afterward."""
        fredutil.fred_assert(l_history[-1].is_next())
	del l_history[-1]
	n_min = len(l_history)
        self.do_restart(b_clear_history = True)
        self.replay_history(l_history)
	self.append_step_over_libc(l_history)
	if not self.program_is_running() or testIfTooFar():
	    return (l_history, n_min)
	repeatNextCmd = self._p.get_personality_cmd(fred_next_cmd())
	# BUG: testIfTooFar() should also test if self.at_breakpoint()
	#      If that happens even once, return immediately, since original
	#      'next' command could never have gone beyond breakpoint.
	return \
	    self.NEW_binary_search_until(l_history, repeatNextCmd, testIfTooFar)

    # MUST STILL CHECK IF n>1 HANDLED CORRECTLY.
    # ALGORITHM: (Note core commands: 's', 'n', 'n' at_bkpt, 'c' at_bkpt)
    #   IDEA:  expand to equivalent 's' or 's' ... 'n'.  In latter case,
    #	     then delete final 'n' since it's going from a level one deeper
    #	     back to the original level.
    #   IMPLEMENTATION:
    # .* 'c' at_bkpt -> .* 's' 'n'* at_bkpt   and continue with rule below
    # .* 'n' level1 -> .* 's' 'n'*   until 'n' returns to level1 on stack
    #       and repeat rule to force final 's', then delete 's' via rule below.
    #       [ instead of repeating rule, could 
    # .* 'n' at_bkpt -> NOT YET IMPLEMENTED; MUST DO AS WITH reverse_next
    # .* 's' -> .*
    def NEW_reverse_step(self, n=1):
        """Perform n 'reverse-step' commands."""
        while n > 0:
            n -= 1
            self.update_state()
            orig_state = self.state().copy()
            if len(self.l_checkpoints) == 0:
                fredutil.fred_error("No checkpoints found for reverse-step.")
                return
	    l_history = self._copy_fred_commands(self.checkpoint.l_history)
	    while len(l_history)>0 and \
		  not l_history[-1].is_step() and \
		  not l_history[-1].is_next() and \
		  not l_history[-1].is_continue():
		del l_history[-1]
	    if len(l_history) == 0:
		break
	    if l_history[-1].is_continue():
		# Since we last executed 'continue', we must be at a breakpoint.
		# Use that to expand the last 'continue' command until we
		#   reach the breakpoint.
		# FIXME:  This is inefficient.  In gdb, we should be
		#   doing 'n 2', 'n 4', 'n 8', etc., and be depending
		#   on gdb stopping at breakpoint.  But gdb doesn't always
		#   stop at breakpoint (e.g. at beginning of 'for' loop)
                del l_history[-1]
                self.do_restart(b_clear_history = True)
                self.replay_history(l_history)
		self.append_step_over_libc(l_history)
                while self.program_is_running() and not self.at_breakpoint():
                    l_history += [self._p.get_personality_cmd(fred_next_cmd())]
                    self.replay_history([l_history[-1]])
	    while l_history[-1].is_next():
                level = self.state().level()
		# expand_next replaces last 'n' by ['s', 'n', ...]
		# self.state().level() can never increase under repeated 'n'
		# BUG:  Actually, 'n' can hit a breakpoint deeper in stack.
	        testIfTooFar = lambda: self.state().level() <= level or \
				       self.at_breakpoint()
	        (l_history, n_min) = \
		    self.NEW_binary_search_expand_next(l_history, testIfTooFar)
		if not l_history[-1].is_step():
		    l_history = self.NEW_binary_search_history(l_history, n_min,
						               testIfTooFar)
            
            fredutil.fred_assert(l_history[-1].is_step())
	    del l_history[-1]
	# Gene - Am I using the next four lines correctly?
	self.checkpoint.l_history = l_history
	self.do_restart()
	self.replay_history()
	self.update_state()
	fredutil.fred_debug("Reverse step finished.")

    def NEW_reverse_finish(self, n=1):
        """Perform n 'reverse-finish' commands."""
        if len(self.l_checkpoints) == 0:
            fredutil.fred_error("No checkpoints found for reverse-finish.")
            return
        while n > 0:
            n -= 1
            self.update_state()
            orig_state = self.state().copy()
	    l_history = self._copy_fred_commands(self.checkpoint.l_history)
            level = self.state().level()
	    while self.state().level() >= level:
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
	            testIfTooFar = lambda: self.at_breakpoint()
		    (l_history, n_min) = \
			self.NEW_binary_search_expand_continue(l_history,
							       testIfTooFar)
		    # l_history ==
		    #     l_history[0:n_min] + (len(l_history)-n_min)*['n']
		    if len(l_history) - n_min > 1:
			l_history = self.NEW_binary_search_history(l_history,
							 n_min, testIfTooFar)
		    # execution stopped with n_min; l_history went one further
		    continue
	        elif l_history[-1].is_next() and self.at_breakpoint():
		    # 'n' can hit a breakpoint deeper in stack.
                    self.trim_n_cmds(l_history, 1)
		    self.do_restart()
		    self.replay_history(l_history)
		    # This can be made more efficient.
		    while self.state().level() < level-1:
			self.append_step_over_libc(l_history)
		    fredutil.fred_assert(self.state().level() == level-1)
	        elif l_history[-1].is_next():
		    # Search for time in past when level was one higher.
		    # This can be optimized for fewer ckpt/restarts.
		    while self.state().level() >= level and \
			  not self.at_breakpoint() and len(l_history)>0 and \
		          (l_history[-1].is_next() or l_history[-1].is_step()):
                        self.trim_n_cmds(l_history, 1)
		        self.do_restart(b_clear_history = True)
		        self.replay_history(l_history)
		    fredutil.fred_assert(self.state().level() >= level-1)
                elif l_history[-1].is_step():
                    self.trim_n_cmds(l_history, 1)
		    self.do_restart(b_clear_history = True)
		    self.replay_history(l_history)
                    self.update_state()
	fredutil.fred_assert(self.state().level() == level-1 or
			     len(l_history) == 0)
	# Gene - Am I using the next four lines correctly?
	self.checkpoint.l_history = l_history
	self.do_restart()
	self.replay_history()
	self.update_state()
	fredutil.fred_debug("Reverse finish finished.")

    # ALGORITHM: (Note core commands: 's', 'n', 'n' at_bkpt, 'c' at_bkpt)
    # .* 's' -> .* ; .* 'n' -> .*   if 'n' doesn't change stack level
    # .* 'n' at_bkpt -> .*   if 'n' doesn't change stack level
    # .* 'n' at_bkpt -> .* 's' 'n'* at_bkpt  if 'n' changes level, and recurse
    # .* 'c' at_bkpt -> .* 's' 'n'* at_bkpt  and recurse to cases above
    # .* 'n' -> reverse_finish(.*)   if 'n' returns to shallower stack level
    def NEW_reverse_next(self, n=1):
        """Perform n 'reverse-next' commands."""
        if len(self.l_checkpoints) == 0:
            fredutil.fred_error("No checkpoints found for reverse-next.")
            return
        while n > 0:
            n -= 1
            self.update_state()
            orig_state = self.state().copy()
	    l_history = self._copy_fred_commands(self.checkpoint.l_history)
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
	            self.trim_n_cmds(l_history, 1)
                    break
                elif l_history[-1].is_next():
                    level = self.state().level()
	            if not self.program_is_running():
                        level = -99
	            self.trim_n_cmds(l_history, 1)
                    self.do_restart(b_clear_history = True)
                    self.replay_history(l_history)
                    if self.state().level() == level:
                        break
                    elif self.state().level() == level+1:
			# The last 'n' returned from fnc; go to before fnc call
                        self.checkpoint.l_history = l_history
                        self.NEW_reverse_finish()
                        break
                    else:
			# The last 'n' entered fnc or exited program
                        fredutil.fred_assert(self.state().level() == level-1 or
					     level == -99)

                fredutil.fred_assert(l_history[-1].is_continue() or
                            (l_history[-1].is_next() and self.at_breakpoint())
			    or level == -99)
                if True:  # if is_continue() or (is_next() and at_breakpoint())
                    # if it was l_history[-1].is_next() and hit a breakpoint,
                    # then it acts like 'c'.  Already undid last command for 'n'
                    if l_history[-1].is_continue():
                        del l_history[-1]
                        self.do_restart(b_clear_history = True)
                        self.replay_history(l_history)
                    self.append_step_over_libc(l_history)
		    # FIXME:  This is inefficient.  In gdb, we should be
		    #   doing 'n 2', 'n 4', 'n 8', etc., and be depending
		    #   on gdb stopping at breakpoint.  But gdb doesn't always
		    #   stop at breakpoint (e.g. at beginning of 'for' loop)
                    while self.program_is_running() and \
			  not self.at_breakpoint():
                        l_history += \
                            [self._p.get_personality_cmd(fred_next_cmd())]
                        self.replay_history([l_history[-1]])
	# Gene - Am I using the next four lines correctly?
	self.checkpoint.l_history = l_history
	self.do_restart()
	self.replay_history(l_history)
	self.update_state()
	fredutil.fred_debug("Reverse next finished.")

    #END OF NEW:  Will replace other methods later
    #====

    # Gene - This method calls do_restart() before returning.
    #        Is that necessary?  It makes FReD slower.
    def _binary_search_checkpoints(self, s_expr, s_expr_val):
        """Perform binary search over checkpoints to identify interval where
        expression changes value."""
        fredutil.fred_debug("Starting binary search for checkpoint interval.")
	fredutil.fred_assert(self.checkpoint,
	  "No initial checkpoint taken.  Please start new debugging session.")
        n_right_ckpt = self.checkpoint.n_index
        if n_right_ckpt == 0:
            fredutil.fred_debug("Only one checkpoint.")
            self.do_restart(n_right_ckpt)
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
            self.do_restart(n_new_index)
            s_expr_new_val = self.evaluate_expression(s_expr)
            if s_expr_new_val != s_expr_val:
                # correct
                n_left_ckpt = n_new_index
            else:
                n_right_ckpt = n_new_index
        # Now n_left_ckpt contains index of the target checkpoint.
        # Restart and return.
        fredutil.fred_debug("Found checkpoint: %d" % n_left_ckpt)
        self.do_restart(n_left_ckpt)

    def _binary_search_history(self, l_history, n_min, s_expr, s_expr_val):
        """Perform binary search on given history to identify interval where
        expression changes value.  Return just before s_expr == s_expr_val."""
        fredutil.fred_debug("Start binary search on history: %s" % \
                            str(l_history))
        n_count = n_max = len(l_history)
        while n_max - n_min > 1:
            n_count = (n_min + n_max) / 2
            self.do_restart(b_clear_history = True)
            self.replay_history(l_history, n_count)
            if self.test_expression(s_expr, s_expr_val):
                fredutil.fred_debug("Setting max bound %d" % n_count)
                n_max = n_count
            else:
                fredutil.fred_debug("Setting min bound %d" % n_count)
                n_min = n_count
        # XXX: deviate here
        fredutil.fred_assert(n_max - n_min == 1)
        self.do_restart(b_clear_history = True)
        l_history = l_history[:n_max]
        self.replay_history(l_history, n_min)
        if n_min == 0 and self.test_expression(s_expr, s_expr_val):
            fredutil.fred_error("Reverse-watch failed to search history.")
            return None
        fredutil.fred_debug("Done searching history.")
        return self._binary_search_expand_history(l_history, s_expr, s_expr_val)

    def _binary_search_expand_history(self, l_history, s_expr, s_expr_val):
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
            l_history = self._binary_search_expand_with_next(l_history[0:-1],
							     s_expr, s_expr_val)
            return l_history
        while l_history[-1].is_next():
	    del l_history[-1]
	    self.append_step_over_libc(l_history)
            if self.test_expression(s_expr, s_expr_val):
                # Done: return debugger at time when if 's' were executed, then
                # expression would become true. We also change the final
                # command to 'step' so the rest of the call stack knows we have
                # gone as deep as possible (i.e. no further expansion is
                # possible).
                del l_history[-1]
	        # FIX ME:  This can step inside libc
                l_history[-1] = self._p.get_personality_cmd(fred_step_cmd())
                self.checkpoint.l_history = l_history
                self.do_restart()
                self.replay_history()
                break
            else:
                self.do_restart(b_clear_history = True)
                self.replay_history(l_history)
                l_history = \
                    self._binary_search_expand_with_next(l_history, s_expr,
                                                         s_expr_val)
        return l_history
        
    def _binary_search_expand_with_next(self, l_history, s_expr, s_expr_val):
        fredutil.fred_debug("Starting expansion with next on %s" % \
                            str(l_history))
        n_min = len(l_history)
        l_expanded_history = [self._p.get_personality_cmd(fred_next_cmd())]
        self.replay_history(l_expanded_history)
        l_history += l_expanded_history
        while self.program_is_running() and \
              not self.test_expression(s_expr, s_expr_val):
            self.replay_history(l_expanded_history)
            n_min = len(l_history)
            l_history += l_expanded_history
            l_expanded_history += l_expanded_history
        fredutil.fred_debug("Done next expansion: %s" % str(l_history))
        return self._binary_search_history(l_history, n_min, s_expr, s_expr_val)

    # Gene - We should generalize this.  self.evaluate_expression could
    #  convert "1" to "true" and "0" to "false".  Then test_expression
    #  does:  return self.evaluate_expression(s_expr) == s_expr_val
    #  Also, is compare_expressions a better name than test_expression ?
    def test_expression(self, s_expr, s_expr_val):
        s_result = self.evaluate_expression(s_expr)
        if s_result == "1" and s_expr_val == "true":
            return True
        elif s_result == "0" and s_expr_val == "false":
            return True
        else:
            return s_result == s_expr_val

    def evaluate_expression(self, s_expr):
        """Returns sanitized value of expression in debugger."""
        s_val = self.do_print(s_expr)
        s_val = self._p.sanitize_print_result(s_val)
        return s_val.strip()

class DebuggerState():
    """Represents the current state of a debugger.
    State of a debugger is represented by:
      - current backtrace
      - current breakpoints, if any
    """
    def __init__(self):
        # The current backtrace.
        self.backtrace = Backtrace()
        # Current breakpoints (list of Breakpoint objects)
        self.l_breakpoints = []

    def set_backtrace(self, bt):
        self.backtrace = bt

    def get_backtrace(self,):
        return self.backtrace

    def set_breakpoints(self, l_bps):
        self.l_breakpoints = l_bps

    def get_breakpoints(self):
        return self.l_breakpoints

    def add_breakpoint(self, bp):
        self.l_breakpoints.append(bp)

    def __eq__(self, other):
        return self.backtrace == other.backtrace and \
               self.get_breakpoints() == other.get_breakpoints()

    def __repr__(self):
        s = "---Backtrace:---\n%s\n---Breakpoints:---\n%s\n" % \
            (str(self.get_backtrace()), str(self.get_breakpoints()))
        return s

    def copy(self):
        """Return a deep copy of this instance."""
        new_state = DebuggerState()
        new_state.set_backtrace(self.get_backtrace().copy())
        for b in self.get_breakpoints():
            new_state.add_breakpoint(b.copy())
        return new_state

    def level(self):
        """Return stack depth."""
        return self.backtrace.depth()
    
class Breakpoint():
    """Represents one breakpoint in the debugger.
    It's not necessary to use all of these fields. gdb is currently the only
    one which does."""
    def __init__(self):
        self.n_number   = 0
        self.s_type     = ""
        self.s_display  = ""
        self.s_enable   = ""
        self.s_address  = ""
        self.s_function = ""
        self.s_file     = ""
        self.n_line     = 0
        self.n_count    = 0

    def __repr__(self):
        return "bp: " + str((self.n_number, self.s_type, self.s_display,
                             self.s_enable, self.s_address, self.s_function,
                             self.s_file, self.n_line, self.n_count))
    
    def __eq__(self, other):
        return other != None and \
               self.n_number == other.n_number and \
               self.s_type == other.s_type and \
               self.s_display == other.s_display and \
               self.s_enable == other.s_enable and \
               self.s_address == other.s_address and \
               self.s_function == other.s_function and \
               self.s_file == other.s_file and \
               self.n_line == other.n_line and \
               self.n_count == other.n_count

    def copy(self):
        """Return a deep copy of this instance."""
        bp = Breakpoint()
        bp.n_number   = self.n_number
        bp.s_type     = self.s_type
        bp.s_display  = self.s_display
        bp.s_enable   = self.s_enable
        bp.s_address  = self.s_address
        bp.s_function = self.s_function
        bp.s_file     = self.s_file
        bp.n_line     = self.n_line
        bp.n_count    = self.n_count
        return bp

class Backtrace():
    """Represents a stack trace (backtrace)."""
    def __init__(self):
        # List of BacktraceFrame objects:
        self.l_frames = []

    def __eq__(self, other):
        return other != None and self.l_frames == other.l_frames

    def __repr__(self):
        return str(self.l_frames)

    def _copy_frames(self, l_frames):
        """Return deep copy of this instance's l_frames."""
        l_result = []
        for frame in l_frames:
            l_result.append(frame.copy())
        return l_result

    def copy(self):
        """Return a deep copy of this instance."""
        new_bt = Backtrace()
        new_bt.l_frames = self._copy_frames(self.l_frames)
        return new_bt

    def add_frame(self, frame):
        """Add the given frame to this instance."""
        self.l_frames.append(frame)

    def get_frames(self):
        """Return the list of frames."""
        return self.l_frames

    def depth(self):
        """Return the depth of this instance."""
        return len(self.l_frames)

class BacktraceFrame():
    """Represents one frame in the stack trace (backtrace).
    It's not necessary to use all of these fields."""
    def __init__(self):
        self.n_frame_num = 0
        self.s_addr      = ""
        self.s_function  = ""
        self.s_args      = ""
        self.s_file      = ""
        self.n_line      = 0

    def __eq__(self, other):
        return other != None and \
               self.n_frame_num == other.n_frame_num and \
               self.s_addr == other.s_addr and \
               self.s_function == other.s_function and \
               self.s_args == other.s_args and \
               self.s_file == other.s_file and \
               self.n_line == other.n_line

    def __repr__(self):
        return "frame: " + str((self.n_frame_num, self.s_addr, self.s_function,
                                self.s_args, self.s_file, self.n_line))

    def addr(self):
        """Return frame address."""
        return self.s_line

    def function(self):
        """Return function name."""
        return self.s_function

    def args(self):
        """Return function arguments."""
        return self.s_args

    def file(self):
        """Return file name."""
        return self.s_file

    def line(self):
        """Return line number."""
        return self.n_line

    def copy(self):
        """Return deep copy of this instance."""
        new_frame = BacktraceFrame()
        new_frame.n_frame_num = self.n_frame_num
        new_frame.s_addr      = self.s_addr
        new_frame.s_function  = self.s_function
        new_frame.s_args      = self.s_args
        new_frame.s_file      = self.s_file
        new_frame.n_line      = self.n_line
        return new_frame

class FredCommand():
    """Represents one user command sent to the debugger.
    Used to abstract away personality-specific syntax. Each FredCommand retains
    the 'native' command, however, which is used during command replay."""
    def __init__(self, name, args=""):
        self.s_name = name
        self.s_args = args
        # Native is what the user entered.
        self.s_native = ""
        # When this flag is True, this command will not be replayed.
        self.b_ignore = False
        # When True, this command may take a numerical count argument
        self.b_count_cmd = False

    def __repr__(self):
        """Return a FReD-abstracted representation of this command."""
        if self.is_unknown():
            return self.native_repr()
        s = self.s_name
        if self.s_args != "":
            s += " " + self.s_args
        return s

    def copy(self):
        """Return a deep copy of this FredCommand."""
        new_cmd = FredCommand(self.s_name)
        new_cmd.s_args      = self.s_args
        new_cmd.s_native    = self.s_native
        new_cmd.b_ignore    = self.b_ignore
        new_cmd.b_count_cmd = self.b_count_cmd
        return new_cmd

    def native_repr(self):
        """Return a personality-native representation of this command.
        Native representation can be passed directly to the debugger."""
        s = self.s_native
        if self.s_args != "":
            s += " " + self.s_args
        return s        

    def set_native(self, s_repr):
        """Set the native representation to the given string."""
        self.s_native = s_repr

    def set_ignore(self):
        """Set the ignore flag to true."""
        self.b_ignore = True
        
    def set_count_cmd(self, b_allowed):
        """Set the count cmd flag to true if b_allowed."""
        if b_allowed:
            self.b_count_cmd = True

    def is_unknown(self):
        return self.s_name == fred_unknown_cmd().s_name

    def is_next(self):
        return self.s_name == fred_next_cmd().s_name

    def is_step(self):
        return self.s_name == fred_step_cmd().s_name

    def is_continue(self):
        return self.s_name == fred_continue_cmd().s_name

    def is_breakpoint(self):
        return self.s_name == fred_breakpoint_cmd().s_name

    def count(self):
        """Return integer representation of 'count' argument."""
        fredutil.fred_assert(self.b_count_cmd,
                             "Tried to get count of non-count cmd.")
        return fredutil.to_int(self.s_args, 1)

    def set_count(self, n):
        """Set s_args flag to the given count."""
        fredutil.fred_assert(self.b_count_cmd,
                             "Tried to set count of non-count cmd.")
        self.s_args = str(n)

class Checkpoint():
    """ This class will represent a checkpoint.  A checkpoint has an
    index number and a command history."""

    # Class-private "static" variable:
    # (make sure you don't access it as a member of a specifc instance)
    n_next_index = 0
    
    def __init__(self):
        # Index number:
        self.n_index = Checkpoint.n_next_index
        Checkpoint.n_next_index += 1
        # The history is a list of FredCommands sent to the debugger
        # from the beginning of this checkpoint.
        self.l_history  = []

    def __repr__(self):
        return str(self.n_index)
    
    def log_command(self, cmd):
        """Adds the given FredCommand to the history."""
        self.l_history.append(cmd)

    def number_non_ignore_cmds(self):
        """Return the number of non-ignore cmds from the history."""
        n = 0
        for cmd in self.l_history:
            if not cmd.b_ignore:
                n += 1 if not cmd.b_count_cmd else cmd.count()
        return n

    def last_command(self):
        """Return the last command of the history or None."""
        if len(self.l_history) == 0:
            return None
        else:
            return self.l_history[-1]

    def last_command_non_ignore(self):
        """Return the last non-ignore command of the history or None."""
        for i in range(-1, -(len(self.l_history)+1), -1):
            cmd = self.l_history[i]
            if not cmd.b_ignore:
                return cmd
        return None
    
    def trim_non_ignore(self, n):
        """Trim last n non-ignore commands.
        Also adjust things like 'next 5' to be 'next 4'."""
        while n > 0:
            if not self.last_command().b_ignore:
                if self.last_command().b_count_cmd:
                    count = self.last_command().count()
                    if count != 1:
                        self.last_command().set_count(count - 1)
                        n -= 1
                        continue
                n -= 1
            self.l_history.pop()



# These will be the abstract commands that should be used *everywhere*. The
# only place which does not operate on these commands is the personalityXXX.py
# file itself.
def fred_next_cmd():
    return FredCommand("next")
def fred_step_cmd():
    return FredCommand("step")
def fred_continue_cmd():
    return FredCommand("continue")
def fred_breakpoint_cmd():
    return FredCommand("breakpoint")
def fred_where_cmd():
    return FredCommand("where")
def fred_info_breakpoints_cmd():
    return FredCommand("info", "breakpoints")
def fred_print_cmd():
    return FredCommand("print")
def fred_unknown_cmd():
    return FredCommand("unknown")
