class ReversibleDebugger():
    def __init__(self, debugger):
        self._d                 = debugger
        self.l_checkpoints      = []
        self.current_checkpoint = Checkpoint()
        
    def history(self):
        """Returns the history of the current Checkpoint."""
        return self.current_checkpoint.history

    def next(self, n):
        """Perform n 'next' commands."""
        self._d.next(n)

    def reverse_next(self, n):
        """Perform n 'reverse-next' commands."""
        assert False, "Unimplemented."

    def state(self):
        """Return the DebuggerState representing the current state of
        the debugger."""
        return self._d.state()

    def update_state(self):
        """Update the underlying DebuggerState."""
        self._d.update_state()

    def get_prompt_str_function(self):
        """Returns the 'contains_prompt_str' function from the personality."""
        return self._d.get_prompt_str_function()

class Debugger():
    def __init__(self, personality):
        self._p = personality

    def next(self, n):
        """Perform n 'next' commands."""
        self._p.next(n)

    def state(self):
        """Return the DebuggerState representing the current state of
        the debugger."""
        return self._p.state()

    def update_state(self):
        """Update the underlying DebuggerState."""
        self._p.update_state()

    def get_prompt_str_function(self):
        """Returns the 'contains_prompt_str' function from the personality."""
        return self._p.contains_prompt_str

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

    def update(self):
        """Updates the debugger state to the current time."""
        self._updateBacktrace()
        self._updateBreakpoints()

    def _updateBacktrace(self):
        assert False, "Unimplemented."

    def _updateBreakpoints(self):
        assert False, "Unimplemented."

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
        return other != None and self.frames == other.frames

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

class Checkpoint():
    """ This class will represent a linked list of checkpoints.  A
    checkpoint has an index number and a command history."""
    def __init__(self):
        self.previous   = None # Pointer to the previous Checkpoint
        self.next       = None # Pointer to next Checkpoint
        self.n_index    = -1   # Index number
        # The history is a list of the commands sent to the debugger
        # from the beginning of this checkpoint.
        self.l_history  = []
