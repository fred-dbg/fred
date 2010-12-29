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
        self.state().backtrace     = self._p.get_backtrace()
        self.state().l_breakpoints = self._p.get_breakpoints()

    def get_prompt_str_function(self):
        """Returns the 'contains_prompt_str' function from the personality."""
        return self._p.contains_prompt_str

    def prompt(self):
        """Bring user back to debugger prompt."""
        self._p.prompt()

class ReversibleDebugger(Debugger):
    """Represents control and management of a reversible Debugger.

    This class knows about checkpoints, command histories, and reversible
    debugger commands.

    It contains an instance of Checkpoint which should always represent the
    current checkpoint.  A Checkpoint contains pointers to the previous and
    next checkpoints for navigation through time (doubly-linked list).
    """
    def __init__(self, personality):
        Debugger.__init__(self, personality)
        self.checkpoint = Checkpoint(0)

    def do_checkpoint(self):
        """Perform a new checkpoint."""
        new_ckpt = Checkpoint(self.checkpoint.n_index + 1)
        self.checkpoint.next = new_ckpt
        new_ckpt.previous = self.checkpoint
        self.checkpoint = new_ckpt
        dmtcpmanager.do_checkpoint()

    def do_restart(self):
        """Restart from the current checkpoint."""
        if self.checkpoint.previous != None:
            self.checkpoint = self.checkpoint.previous
        dmtcpmanager.restart_last_ckpt()
        
    def list_checkpoints(self):
        """Return the list of available Checkpoint files."""
        checkpoints = []
        orig_index = self.checkpoint.n_index
        ckpt = self.checkpoint
        while ckpt.n_index > 0:
            ckpt = ckpt.previous
        while ckpt.next != None:
            checkpoints.append(ckpt.n_index)
            ckpt = ckpt.next
        while ckpt.n_index != orig_index:
            ckpt = ckpt.previous
        return checkpoints

    def history(self):
        """Return the history of the current Checkpoint."""
        return self.checkpoint.l_history

    def log_command(self, s_command):
        """Convert given command to FredCommand instance and add to current
        history."""
        cmd = self._p.identify_command(s_command)
        cmd.s_args = s_command.partition(' ')[2]
        self.checkpoint.log_command(cmd)

    def log_fred_command(self, cmd):
        """Directly log the given FredCommand instance."""
        self.checkpoint.log_command(cmd)
    
    def do_next(self, n):
        """Perform n 'next' commands. Returns output."""
        cmd = fred_next_cmd()
        cmd.s_args = str(n)
        self.log_fred_command(cmd)
        return self._next(n)
        
    def do_step(self, n):
        """Perform n 'step' commands. Returns output."""
        cmd = fred_step_cmd()
        cmd.s_args = str(n)
        self.log_fred_command(cmd)
        return self._step(n)
        
    def do_continue(self, n):
        """Perform n 'continue' commands. Returns output."""
        cmd = fred_continue_cmd()
        cmd.s_args = str(n)
        self.log_fred_command(cmd)
        return self._continue(n)
        
    def do_breakpoint(self, expr):
        """Perform 'break expr' command. Returns output."""
        cmd = fred_breakpoint_cmd()
        cmd.s_args = expr
        self.log_fred_command(cmd)
        return self._breakpoint(expr)

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

    def undo(self, n):
        """Undo the last n commands."""
        fredutil.fred_error("Unimplemented command.")

    def reverse_next(self, n):
        """Perform n 'reverse-next' commands."""
        #cmd = FredCommand(GS_REVERSE_NEXT_COMMAND, str(n))
        #self.log_command(cmd)
        fredutil.fred_error("Unimplemented command.")

    def reverse_step(self, n):
        """Perform n 'reverse-step' commands."""
        #cmd = FredCommand(GS_REVERSE_STEP_COMMAND, str(n))
        #self.log_command(cmd)
        fredutil.fred_error("Unimplemented command.")

    def reverse_watch(self, s_expr):
        """Perform 'reverse-watch' command on expression."""
        fredutil.fred_error("Unimplemented command.")

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
        return str((self.n_number, self.s_type, self.s_display, self.s_enable,
                    self.s_address, self.s_function, self.s_file, self.n_line,
                    self.n_count))
    
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

class Backtrace():
    """Represents a stack trace (backtrace)."""
    def __init__(self):
        # List of BacktraceFrame objects:
        self.l_frames = []

    def __eq__(self, other):
        return other != None and self.l_frames == other.l_frames

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

class FredCommand():
    def __init__(self, name, args=""):
        self.s_name = name
        self.s_args = args

    def __repr__(self):
        s = self.s_name
        if self.s_args != "":
            s += " " + self.s_args
        return s
        
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
