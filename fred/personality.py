import freddebugger

class Personality:
    def __init__(self):
        self.s_name = "Abstract personality"

    def get_backtrace(self):
        """Return a Backtrace object representing the current backtrace."""
        assert False, "Must be implemented in subclass."

    def get_breakpoints(self):
        """Return a list of Breakpoint objects of the current breakpoints."""
        assert False, "Must be implemented in subclass."

    def do_next(self, n):
        """Perform n 'next' commands. Returns output."""
        assert False, "Must be implemented in subclass."
        
    def do_step(self, n):
        """Perform n 'step' commands. Returns output."""
        assert False, "Must be implemented in subclass."
        
    def do_continue(self, n):
        """Perform n 'continue' commands. Returns output."""
        assert False, "Must be implemented in subclass."
        
    def do_breakpoint(self, expr):
        """Perform 'break expr' command. Returns output."""
        assert False, "Must be implemented in subclass."

    def do_where(self):
        """Perform 'where' command. Returns output."""
        assert False, "Must be implemented in subclass."

    def do_info_breakpoints(self):
        """Perform 'info_breakpoints' command. Returns output."""
        assert False, "Must be implemented in subclass."

    def do_print(self, expr):
        """Perform 'print expr' command. Returns output."""
        assert False, "Must be implemented in subclass."

    def contains_prompt_str(self, string):
        """Return True if given string matches the prompt string."""
        assert False, "Must be implemented in subclass."

    def prompt(self):
        """Bring user back to debugger prompt."""
        assert False, "Must be implemented in subclass."

    def identify_command(self, s_command):
        """Returns a FredCommand representing given personality command."""
        assert False, "Must be implemented in subclass."
