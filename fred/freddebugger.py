GS_NEXT_COMMAND  = "next"
GS_REVERSE_NEXT_COMMAND  = "reverse-next"

class Debugger():
    """Class which represents control and management of the actual debugger.

    This provides a consistent interface to different debuggers, based on the
    particular Personality instance. Each Personality instance has a
    well-defined set of required functions, and this class calls those.

    If the semantics of personality-specific things change, we may change the
    usage here, and keep the interface unchanged."""
    def __init__(self, personality):
        self._p = personality
        self._state = DebuggerState()

    def next(self, n):
        """Perform n 'next' commands."""
        self._p.next(n)

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
    """Class which represents control and management of a reversible Debugger.

    This class knows about checkpoints, command histories, and reversible
    debugger commands.

    It contains an instance of Checkpoint which should always represent the
    current checkpoint.  A Checkpoint contains pointers to the previous and
    next checkpoints for navigation through time (doubly-linked list).
    """
    def __init__(self, personality):
        Debugger.__init__(self, personality)
        self.checkpoint = Checkpoint()
        
    def history(self):
        """Returns the history of the current Checkpoint."""
        return self.checkpoint.l_history

    def reverse_next(self, n):
        """Perform n 'reverse-next' commands."""
        #cmd = FredCommand(GS_REVERSE_NEXT_COMMAND, str(n))
        self.log_command(cmd)
        assert False, "Unimplemented."

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

'''
class FredCommand():
    def __init__(self, name, args="", native_repr, alias=""):
        self.s_name         = name
        self.s_args         = args
        self.s_native_repr  = native_repr
        self.s_native_alias = alias
'''

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

    def log_command(self, cmd):
        """Adds the given FredCommand to the history."""
        self.l_history.append(cmd)
