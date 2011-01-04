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
        self.ls_needs_user_input = []
        self.b_has_count_commands = False
        self.b_coalesce_support = False
        
    def get_backtrace(self):
        """Return a Backtrace object representing the current backtrace."""
        return self._parse_backtrace(self.do_where())

    def get_breakpoints(self):
        """Return a list of Breakpoint objects for the current breakpoints."""
        return self._parse_breakpoints(self.do_info_breakpoints())

    def _parse_breakpoints(self, info_str):
        """Return a list of Breakpoint objects parsed from output of 'info
        breakpoints' cmd."""
        l_breakpoints = []
        l_matches = re.findall(self.gre_breakpoint, info_str, re.MULTILINE)
        for i in range(0, len(l_matches)):
            l_breakpoints.append(self._parse_one_breakpoint(l_matches[i]))
        return l_breakpoints

    def _parse_backtrace(self, where_str):
        """Return a Backtrace instance parsed from output of 'where' cmd."""
        backtrace = re.sub(self.gre_prompt, '', where_str)
        bt_list = re.findall(self.gre_backtrace_frame,
                             backtrace, re.MULTILINE | re.DOTALL)
        frame_list = []
        bt = freddebugger.Backtrace()
        for i in range(0, len(bt_list)):
            frame_list.append(self._parse_backtrace_frame(bt_list[i]))
        bt.l_frames = frame_list
        return bt

    def do_next(self, n):
        """Perform n 'next' commands. Returns output."""
        return fredio.get_child_response(self.GS_NEXT + " " + str(n) + "\n",
                                         b_wait_for_prompt=True)
        
    def do_step(self, n):
        """Perform n 'step' commands. Returns output."""
        return fredio.get_child_response(self.GS_STEP + " " + str(n) + "\n",
                                         b_wait_for_prompt=True)
        
    def do_continue(self, n):
        """Perform n 'continue' commands. Returns output."""
        return fredio.get_child_response(self.GS_CONTINUE + " " + str(n) + "\n",
                                         b_wait_for_prompt=True)
        
    def do_breakpoint(self, expr):
        """Perform 'break expr' command. Returns output."""
        return fredio.get_child_response(self.GS_BREAKPOINT + " " + str(expr) + "\n",
                                         b_wait_for_prompt=True)

    def do_where(self):
        """Perform 'where' command. Returns output."""
        return fredio.get_child_response(self.GS_WHERE + "\n", b_wait_for_prompt=True)

    def do_info_breakpoints(self):
        """Perform 'info_breakpoints' command. Returns output."""
        return fredio.get_child_response(self.GS_INFO_BREAKPOINTS + "\n",
                                         b_wait_for_prompt=True)

    def do_print(self, expr):
        """Perform 'print expr' command. Returns output."""
        return fredio.get_child_response(self.GS_PRINT + " " + str(expr) + "\n",
                                         b_wait_for_prompt=True)

    def contains_prompt_str(self, string):
        """Return True if given string matches the prompt string."""
        return re.search(self.gre_prompt, string) != None

    def prompt_string(self):
        """Return the debugger's prompt string."""
        fredutil.fred_assert(False, "Must be implemented in subclass.")

    def prompt(self):
        """Bring user back to debugger prompt."""
        fredutil.fred_assert(False, "Must be implemented in subclass.")

    def program_is_running(self):
        """Return True if inferior is still running."""
        output = self.execute_command(self.GS_CURRENT_POS + "\n")
        # Bit confusing... but if the program is running then the regular
        # expression here WON'T match
        return re.search(self.gs_program_not_running_re, output) == None

    def get_personality_cmd(self, generic_cmd):
        """Return a FredCommand (with native representation filled out) from
        the given FredCommand."""
        cmd = self.identify_command(self.get_native(generic_cmd))
        return cmd

    def identify_command(self, s_command):
        """Return a FredCommand representing given personality command.
        Also sets 'native' string appropriately."""
        cmd = None
        if re.search(self.gs_next_re, s_command) != None:
            cmd = freddebugger.fred_next_cmd()
            cmd.set_count_cmd(self.b_has_count_commands)
        elif re.search(self.gs_step_re, s_command) != None:
            cmd = freddebugger.fred_step_cmd()
            cmd.set_count_cmd(self.b_has_count_commands)
        elif re.search(self.gs_continue_re, s_command) != None:
            cmd = freddebugger.fred_continue_cmd()
            cmd.set_count_cmd(self.b_has_count_commands)
        elif re.search(self.gs_breakpoint_re, s_command) != None:
            cmd = freddebugger.fred_breakpoint_cmd()
        elif re.search(self.gs_where_re, s_command) != None:
            cmd = freddebugger.fred_where_cmd()
            cmd.set_ignore()
        elif re.search(self.gs_info_breakpoints_re, s_command) != None:
            cmd = freddebugger.fred_info_breakpoints_cmd()
            cmd.set_ignore()
        elif re.search(self.gs_print_re, s_command) != None:
            cmd = freddebugger.fred_print_cmd()
            cmd.set_ignore()
        else:
            cmd = freddebugger.fred_unknown_cmd()
        cmd.set_native(s_command.partition(' ')[0])
        return cmd        

    def get_native(self, fred_cmd):
        """Return native representation of given FredCommand."""
        if fred_cmd.is_next():
            return self.GS_NEXT
        elif fred_cmd.is_step():
            return self.GS_STEP
        elif fred_cmd.is_continue():
            return self.GS_CONTINUE
        elif fred_cmd.is_breakpoint():
            return self.GS_BREAKPOINT
        elif fred_cmd.is_where():
            return self.GS_WHERE
        elif fred_cmd.is_info_breakpoints():
            return self.GS_INFO_BREAKPOINTS
        elif fred_cmd.is_print():
            return self.GS_PRINT
        else:
            fredutil.fred_debug("Don't know native representation of %s" % \
                                fred_cmd)
            return ""

    def execute_command(self, s_cmd):
        """Send the given string to debugger and return its output."""
        return fredio.get_child_response(s_cmd, b_wait_for_prompt=True)

    def sanitize_print_result(self, s_printed):
        """Sanitize the result of a debugger 'print' command.
        This is to normalize out things like gdb's print result:
          $XX = 16
        Where $XX changes with each command executed."""
        fredutil.fred_assert(False, "Must be implemented in subclass.")
