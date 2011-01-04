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
import pdb

import dmtcpmanager
import fredutil

class Debugger():
    """Represents control and management of an actual debugger.

    This provides a consistent interface to different debuggers, based on the
    particular Personality instance. Each Personality instance has a
    well-defined set of required functions, and this class calls those.

    If the semantics of personality-specific things change, we may change the
    usage here, and keep the interface unchanged."""
    def __init__(self, personality):
        self._p = personality
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

    def state(self):
        """Return the DebuggerState representing the current state of
        the debugger."""
        return self._state

    def update_state(self):
        """Update the underlying DebuggerState."""
        fredutil.fred_debug("Updating DebuggerState.")
        self.state().backtrace     = self._p.get_backtrace()
        self.state().l_breakpoints = self._p.get_breakpoints()

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

    def do_checkpoint(self):
        """Perform a new checkpoint."""
        n_new_index = -1
        if self.checkpoint != None:
            n_new_index = self.checkpoint.n_index + 1
            new_ckpt = Checkpoint(n_new_index)
            self.checkpoint.next = new_ckpt
        else:
            n_new_index = 0
            new_ckpt = Checkpoint(n_new_index)            
        new_ckpt.previous = self.checkpoint
        self.l_checkpoints.append(new_ckpt)
        self.checkpoint = self.l_checkpoints[new_ckpt.n_index]
        dmtcpmanager.do_checkpoint()
        fredutil.fred_debug("Created checkpoint #%d." % new_ckpt.n_index)
        self.update_state()

    def reset_on_restart(self):
        """Perform any reset functions that should happen on restart."""
        if self.personality_name() == "gdb":
            self._p.reset_user_code_interval()

    def do_restart(self, b_clear_history=False, n_index=-1):
        """Restart from the current or specified checkpoint."""
        fredutil.fred_debug("Restarting from checkpoint index %d." % \
                            self.checkpoint.n_index)
        self.reset_on_restart()
        if (n_index == -1):
            # Restart from current checkpoint.
            dmtcpmanager.restart_last_ckpt()
        else:
            dmtcpmanager.restart_ckpt(n_index)
            self.checkpoint = self.l_checkpoints[n_index]
        if b_clear_history:
            self.clear_history()
        self.update_state()

    def do_restart_previous(self):
        """Restart from the previous checkpoint."""
        self.checkpoint = self.l_checkpoints[self.checkpoint.n_index - 1]
        fredutil.fred_debug("Restarting from checkpoint index %d." % \
                            self.checkpoint.n_index)
        self.reset_on_restart()
        dmtcpmanager.restart_ckpt(self.checkpoint.n_index)
        self.clear_history()
        self.update_state()
        
    def list_checkpoints(self):
        """Return the list of available Checkpoint files."""
        return self.l_checkpoints
    
    def history(self):
        """Return the history of all Checkpoints."""
        l_history = []
        if self.checkpoint != None:
            for ckpt in self.l_checkpoints:
                l_history.append("*ckpt*")
                l_history.extend(ckpt.l_history)
        return l_history

    def clear_history(self):
        """Clear the current checkpoint's history."""
        del self.checkpoint.l_history[:]

    def last_command(self):
        """Return the last command of the current history."""
        if len(self.checkpoint.l_history) == 0:
            if self.checkpoint.previous != None:
                fredutil.fred_assert(len(self.checkpoint.previous.l_history) > 0, \
                       "Unimplemented.")
                return self.checkpoint.previous.l_history[-1]
            else:
                fredutil.fred_assert(False, "Unimplemented branch.")
        else:
            return self.checkpoint.l_history[-1]
            
    
    def log_command(self, s_command):
        """Convert given command to FredCommand instance and add to current
        history."""
        # identify_command() sets native repr.
        cmd = self._p.identify_command(s_command)
        cmd.s_args = s_command.partition(' ')[2]
        if self.checkpoint != None:
            self.checkpoint.log_command(cmd)

    def log_fred_command(self, cmd):
        """Directly log the given FredCommand instance."""
        if self.checkpoint != None:
            self.checkpoint.log_command(cmd)
    
    def execute_fred_command(self, cmd):
        """Execute the given FredCommand."""
        if cmd.s_native == "":
            # This should never happen.
            fredutil.fred_fatal("FredCommand instance has null native string.")
        elif cmd.b_ignore:
            return
        self._p.execute_command(cmd.s_native + " " + cmd.s_args + "\n")
        self.update_state()

    def do_next(self, n=1):
        """Perform n 'next' commands. Returns output."""
        cmd = fred_next_cmd()
        cmd.set_native(self._p.get_native(cmd))
        cmd.s_args = str(n)
        self.log_fred_command(cmd)
        output = self._next(n)
        self.update_state()
        return output
        
    def do_step(self, n=1):
        """Perform n 'step' commands. Returns output."""
        cmd = fred_step_cmd()
        cmd.set_native(self._p.get_native(cmd))
        cmd.s_args = str(n)
        # TODO: Special case for gdb so we don't step into libc. Think of
        # more portable way to do this.
        output = self._step(n)
        if output == "DO-NOT-STEP":
            # Log a next instead of step so we don't step into libc again.
            cmd = fred_next_cmd()
            cmd.set_native(self._p.get_native(cmd))
        self.log_fred_command(cmd)
        self.update_state()
        return output
        
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
    
    def do_where(self):
        """Perform 'where' command. Returns output."""
        return self._where()

    def do_info_breakpoints(self):
        """Perform 'info_breakpoints' command. Returns output."""
        return self._info_breakpoints()

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
            n = len(l_history)
        l_temp = self._coalesce_history(l_history)
        fredutil.fred_debug("Replaying the following history: %s" % \
                            str(l_temp[0:n]))
        for cmd in l_temp[0:n]:
            self.execute_fred_command(cmd)

    def trim_non_ignore(self, n):
        """Trim last n non-ignore commands.
        Also adjust things like 'next 5' to be 'next 4'."""
        while n > 0:
            if not self.last_command().b_ignore:
                if self.last_command().b_count_cmd:
                    count = self.last_command().count()
                    if count != 0:
                        self.last_command().set_count(count - 1)
                        n -= 1
                        continue
                n -= 1
            self.checkpoint.l_history.pop()

    def undo(self, n=1):
        """Undo the last n commands."""
        b_restart = True
        fredutil.fred_debug("Undoing %d command(s)." % n)
        while len(self.checkpoint.l_history) == 0 or \
              n > len(self.checkpoint.l_history):
            b_restart = False
            n -= len(self.checkpoint.l_history)
            # Back up to previous checkpoint
            if self.checkpoint.previous == None:
                fredutil.fred_error("No undo possible (empty command history "
                                    "and no previous checkpoints).")
                return
            else:
                self.do_restart_previous()
        if b_restart:
            self.do_restart(b_clear_history = True)
        # Trim history by n non-ignore commands
        self.trim_non_ignore(n)
        self.replay_history()
        self.update_state()
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
                        if self.last_command().is_next() or \
                               self.last_command().is_step():
                            fredutil.fred_debug("RN: AFTER NEXT OR STEP")
                            n_lvl = self.state().level()
                            self.undo()
                            if n_lvl >= self.state().level():
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
        if n != 1:
            fredutil.fred_fatal("Unimplemented.")
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
                    if self.last_command().is_step():
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

    def reverse_finish(self):
        """Perform 'reverse-finish' command."""
        self.update_state()
        orig_state = self.state().copy()
        while self.state().level() >= orig_state.level():
            self.reverse_next()
        self.update_state()
        fredutil.fred_debug("Reverse finish finished.")
        
    def reverse_continue(self):
        """Perform 'reverse-continue' command."""
        fredutil.fred_error("Unimplemented command.")
        
    def reverse_watch(self, s_expr):
        """Perform 'reverse-watch' command on expression."""
        s_expr_val = self._evaluate_expression(s_expr)
        fredutil.fred_debug("RW: Starting with expr value '%s'" % \
                            s_expr_val)
        # Find starting checkpoint using binary search:
        self._binary_search_checkpoints(s_expr, s_expr_val)

        self.checkpoint.l_history = \
            self._binary_search_history(self._copy_fred_commands(self.checkpoint.l_history),
                                        0, s_expr, s_expr_val)

        self.update_state()
        fredutil.fred_debug("Reverse watch finished.")

    def _binary_search_checkpoints(self, s_expr, s_expr_val):
        """Perform binary search over checkpoints to identify interval where
        expression changes value."""
        fredutil.fred_debug("Starting binary search for checkpoint interval.")
        n_right_ckpt = self.checkpoint.n_index
        if n_right_ckpt == 0:
            fredutil.fred_debug("Only one checkpoint.")
            self.do_restart(n_right_ckpt)
            return
        n_left_ckpt = 0
        # Repeat until the interval is 1 checkpoint long. That means the left
        # checkpoint has the "correct" value and the right one has the
        # "incorrect" value. "Incorrect" in this case means the same as the
        # start value.
        while (n_right_ckpt - n_left_ckpt) != 1:
            n_diff = (n_right_ckpt - n_left_ckpt) / 2
            n_new_index = int(math.ceil(n_diff) + n_left_ckpt)
            self.do_restart(n_new_index)
            s_expr_new_val = self._evaluate_expression(s_expr)
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
        expression changes value."""
        fredutil.fred_debug("Start binary search on history: %s" % \
                            str(l_history))
        n_count = n_max = len(l_history)
        while n_max - n_min > 1:
            n_count = (n_min + n_max) / 2
            self.do_restart(b_clear_history = True)
            self.replay_history(l_history, n_count)
            if self._evaluate_expression(s_expr) == s_expr_val:
                fredutil.fred_debug("Setting max bound %d" % n_count)
                n_max = n_count
            else:
                fredutil.fred_debug("Setting min bound %d" % n_count)
                n_min = n_count
        # XXX: deviate here
        self.do_restart(b_clear_history = True)
        l_history = l_history[:n_min+1]
        self.replay_history(l_history, n_min)
        if n_min == 0 and self._evaluate_expression(s_expr) == s_expr_val:
            fredutil.fred_error("Reverse-watch failed to search history.")
            return None
        fredutil.fred_assert(n_max - n_min == 1)
        fredutil.fred_debug("Done searching history.")
        return self._binary_search_expand_history(l_history, s_expr, s_expr_val)

    def _binary_search_expand_history(self, l_history, s_expr, s_expr_val):
        """On entry, current time is history[0:-1] and expr will change upon
        executing last command, history[-1]. Last command must be 'c', 'n', or
        's'.
        Expands [..., 'c'] -> [..., 'n', ...]
             or [..., 'n'] -> [..., 's', 'n', ...]
             
        Returns history such that s_expr != s_expr_val at end of
        history, and if 's' were executed, then s_expr_val would be
        True."""
        fredutil.fred_debug("Start expanding history: %s" % str(l_history))
        if l_history[-1].is_step():
            fredutil.fred_debug("Last command was step.")
            return l_history
        fredutil.fred_assert(l_history[-1].is_next() or \
                             l_history[-1].is_continue())
        if l_history[-1].is_continue():
            fredutil.fred_debug("Last command continue.")
            l_history = \
                self._binary_search_expand_with_next(l_history[0:-1], s_expr, s_expr_val)
        while l_history[-1].is_next():
            l_history[-1] = self._p.get_personality_cmd(fred_step_cmd())
            self.replay_history([self._p.get_personality_cmd(fred_step_cmd())])
            if self._evaluate_expression(s_expr) == s_expr_val:
                # Done: return debugger at time: l_history[0:-1]
                self.checkpoint.l_history = l_history[0:-1]
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
              self._evaluate_expression(s_expr) != s_expr_val:
            self.replay_history(l_expanded_history)
            n_min = len(l_history)
            l_history += l_expanded_history
            l_expanded_history += l_expanded_history
        fredutil.fred_debug("Done next expansion: %s" % str(l_history))
        return self._binary_search_history(l_history, n_min, s_expr, s_expr_val)

    def _evaluate_expression(self, s_expr):
        """Returns sanitized value of expression in debugger. Used by
        reverse-watch."""
        s_val = self.do_print(s_expr + "\n")
        s_val = self._p.sanitize_print_result(s_val)
        return s_val
            
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

    def __eq__(self, other):
        return self.backtrace == other.backtrace and \
               self.l_breakpoints == other.l_breakpoints

    def __repr__(self):
        s = "---Backtrace:---\n" +str(self.backtrace)+ "\n---Breakpoints:---\n"
        s += str(self.l_breakpoints)
        return s

    def copy(self):
        """Return a deep copy of this instance."""
        new_state = DebuggerState()
        new_state.backtrace = self.backtrace.copy()
        for b in self.l_breakpoints:
            new_state.l_breakpoints.append(b.copy())
        return new_state

    def level(self):
        """Return stack depth."""
        return len(self.backtrace.l_frames)
    
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
        bp.n_number = self.n_number
        bp.s_type = self.s_type
        bp.s_display = self.s_display
        bp.s_enable = self.s_enable
        bp.s_address = self.s_address
        bp.s_function = self.s_function
        bp.s_file = self.s_file
        bp.n_line = self.n_line
        bp.n_count = self.n_count
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

    def is_where(self):
        return self.s_name == fred_where_cmd().s_name

    def is_info_breakpoints(self):
        return self.s_name == fred_info_breakpoints_cmd().s_name

    def is_print(self):
        return self.s_name == fred_print_cmd().s_name

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
    """ This class will represent a linked list of checkpoints.  A
    checkpoint has an index number and a command history."""
    def __init__(self, idx=-1):
        self.previous   = None # Pointer to the previous Checkpoint
        self.next       = None # Pointer to next Checkpoint
        self.n_index    = idx  # Index number
        # The history is a list of FredCommands sent to the debugger
        # from the beginning of this checkpoint.
        self.l_history  = []

    def __repr__(self):
        return str(self.n_index)
    
    def log_command(self, cmd):
        """Adds the given FredCommand to the history."""
        self.l_history.append(cmd)

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
