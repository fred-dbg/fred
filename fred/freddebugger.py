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
import debugger

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

class ReversibleDebugger(debugger.Debugger):
    """Represents control and management of a reversible Debugger.

    This class knows about checkpoints, command histories, and reversible
    debugger commands.

    It contains an instance of Checkpoint which should always represent the
    current checkpoint.  It also contains a list of all Checkpoint objects
    such that Checkpoint.n_index refers to the index into that list.
    """
    def __init__(self, personality):
        debugger.Debugger.__init__(self, personality)
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
