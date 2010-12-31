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

import fredio
import freddebugger

import re

class Personality:
    def __init__(self):
        self.s_name = "Abstract personality"
        self.GS_NEXT = None
        self.GS_STEP = None
        self.GS_CONTINUE = None
        self.GS_BREAKPOINT = None
        self.GS_WHERE = None
        self.GS_INFO_BREAKPOINTS = None
        self.GS_PRINT = None

        self.gs_next_re = None
        self.gs_step_re = None
        self.gs_continue_re = None
        self.gs_breakpoint_re = None
        self.gs_where_re = None
        self.gs_info_breakpoints_re = None
        self.gs_print_re = None

        self.GS_PROMPT = None
        self.gre_prompt = None
        self.gre_backtrace_frame = None
        self.gre_breakpoint = None

    def get_backtrace(self):
        """Return a Backtrace object representing the current backtrace."""
        assert False, "Must be implemented in subclass."

    def get_breakpoints(self):
        """Return a list of Breakpoint objects of the current breakpoints."""
        assert False, "Must be implemented in subclass."


    def do_next(self, n):
        """Perform n 'next' commands. Returns output."""
        return fredio.get_child_response(self.GS_NEXT + " " + str(n))
        
    def do_step(self, n):
        """Perform n 'step' commands. Returns output."""
        return fredio.get_child_response(self.GS_STEP + " " + str(n))
        
    def do_continue(self, n):
        """Perform n 'continue' commands. Returns output."""
        return fredio.get_child_response(self.GS_CONTINUE + " " + str(n))
        
    def do_breakpoint(self, expr):
        """Perform 'break expr' command. Returns output."""
        return fredio.get_child_response(self.GS_BREAKPOINT + " " + str(expr))

    def do_where(self):
        """Perform 'where' command. Returns output."""
        return fredio.get_child_response(self.GS_WHERE)

    def do_info_breakpoints(self):
        """Perform 'info_breakpoints' command. Returns output."""
        return fredio.get_child_response(self.GS_INFO_BREAKPOINTS)

    def do_print(self, expr):
        """Perform 'print expr' command. Returns output."""
        return fredio.get_child_response(self.GS_PRINT + " " + str(expr))

    def contains_prompt_str(self, string):
        """Return True if given string matches the prompt string."""
        return re.search(self.gre_prompt, string) != None

    def prompt_string(self):
        """Return the debugger's prompt string."""
        assert False, "Must be implemented in subclass."

    def prompt(self):
        """Bring user back to debugger prompt."""
        assert False, "Must be implemented in subclass."

    def identify_command(self, s_command):
        """Return a FredCommand representing given personality command.
        Also sets 'native' string appropriately."""
        cmd = None
        if re.search(self.gs_next_re, s_command) != None:
            cmd = freddebugger.fred_next_cmd()
            cmd.set_native(self.GS_NEXT)
        elif re.search(self.gs_step_re, s_command) != None:
            cmd = freddebugger.fred_step_cmd()
            cmd.set_native(self.GS_STEP)
        elif re.search(self.gs_continue_re, s_command) != None:
            cmd = freddebugger.fred_continue_cmd()
            cmd.set_native(self.GS_CONTINUE)
        elif re.search(self.gs_breakpoint_re, s_command) != None:
            cmd = freddebugger.fred_breakpoint_cmd()
            cmd.set_native(self.GS_BREAKPOINT)
        elif re.search(self.gs_where_re, s_command) != None:
            cmd = freddebugger.fred_where_cmd()
            cmd.set_native(self.GS_WHERE)
            cmd.set_ignore()
        elif re.search(self.gs_info_breakpoints_re, s_command) != None:
            cmd = freddebugger.fred_info_breakpoints_cmd()
            cmd.set_native(self.GS_INFO_BREAKPOINTS)
            cmd.set_ignore()
        elif re.search(self.gs_print_re, s_command) != None:
            cmd = freddebugger.fred_print_cmd()
            cmd.set_native(self.GS_PRINT)
            cmd.set_ignore()
        else:
            cmd = freddebugger.fred_unknown_cmd()
        cmd.set_native(s_command.partition(' ')[0])
        return cmd        

    def execute_command(self, s_cmd):
        """Send the given string to debugger and return its output."""
        return fredio.get_child_response(s_cmd)
