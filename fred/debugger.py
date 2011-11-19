import signal
import os

import fredutil

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
        self._n_pid = -1

    def personality_name(self):
        """Return the name of the personality."""
        return self._p.s_name

    def get_debugger_pid(self):
        """Return the pid of the debugger process."""
        return self._n_pid

    def set_debugger_pid(self, n_pid):
        """Set the pid of the debugger process."""
        self._n_pid = n_pid

    def _next(self, n):
        """Perform n 'next' commands. Returns output."""
        return self._p.do_next(n)
        
    def _step(self, n):
        """Perform n 'step' commands. Returns output."""
        return self._p.do_step(n)
        
    def _continue(self, b_wait_for_prompt):
        """Perform 'continue' command. Returns output."""
        return self._p.do_continue(b_wait_for_prompt)
        
    def _breakpoint(self, expr):
        """Perform 'break expr' command. Returns output."""
        return self._p.do_breakpoint(expr)

    def _finish(self):
        """Perform 'finish' command. Returns output."""
        return self._p.do_finish()

    def _where(self):
        """Perform 'where' command. Returns output."""
        return self._p.do_where()

    def _info_breakpoints(self):
        """Perform 'info_breakpoints' command. Returns output."""
        return self._p.do_info_breakpoints()

    def _print(self, expr):
        """Perform 'print expr' command. Returns output."""
        return self._p.do_print(expr)

    def _switch_to_thread(self, n_tid):
        """Perform thread switch command to given tid. No output returned."""
        self._p.switch_to_thread(n_tid)

    def execute_until_thread(self, n_tid):
        """Execute until the given thread is alive."""
        # This is an imperfect solution; the newly created thread has a chance
        # to execute a few instructions before the "finish" returns.
        import pdb
        pdb.set_trace()
        self._breakpoint("pthread_create")
        self._continue(True)
        self._finish()
        self._switch_to_thread(n_tid)
        
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

    def stop_inferior(self):
        """Sends SIGSTOP to inferior process."""
        n_pid = fredutil.get_inferior_pid(self.get_debugger_pid())
        fredutil.fred_assert(n_pid != -1)
        os.kill(n_pid, signal.SIGSTOP)
        fredio.wait_for_prompt()

    def interrupt_inferior(self):
        """Sends a ^C to the inferior process."""
        n_pid = fredutil.get_inferior_pid(self.get_debugger_pid())
        if n_pid != -1:
            os.kill(n_pid, signal.SIGINT)

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

    def get_backtrace(self):
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
