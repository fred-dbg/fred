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
