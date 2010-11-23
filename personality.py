import freddebugger

class Personality:
    def __init__(self):
        self._state = freddebugger.DebuggerState()
        self.s_name = "Abstract debugger"

    def state(self):
        """Return the DebuggerState representing current state of debugger."""
        return self._state

    def update_state(self):
        """Update the DebuggerState to the current time."""
        assert False, "Must be implemented in subclass."

    def where(self):
        """Return stack trace from debugger."""
        assert False, "Must be implemented in subclass."
