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

    def where(self):
        """Return stack trace from debugger."""
        assert False, "Must be implemented in subclass."

    def list_breakpoints(self):
        """Return output of 'info breakpoints' command."""
        assert False, "Must be implemented in subclass."

    def contains_prompt_str(self, string):
        """Return True if given string matches the prompt string."""
        assert False, "Must be implemented in subclass."

    def prompt(self):
        """Bring user back to debugger prompt."""
        assert False, "Must be implemented in subclass."
