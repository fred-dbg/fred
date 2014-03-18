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

import pdb
import re

from .. import fredio
from .. import freddebugger
from .. import debugger
from .. import fredutil

class Personality:
    def __init__(self):
        self.s_name = "Abstract personality"
        self.GS_NEXT = None
        self.GS_STEP = None
        self.GS_CONTINUE = None
        self.GS_BREAKPOINT = None
        self.GS_WHERE = None
        self.GS_INFO_BREAKPOINTS = None
        self.GS_INFO_THREADS = None
        self.GS_PRINT = None
        self.GS_FINISH = None
        self.GS_CURRENT_POS = None
        self.GS_SWITCH_THREAD = None

        self.gs_next_re = None
        self.gs_step_re = None
        self.gs_continue_re = None
        self.gs_program_not_running_re = None

        self.GS_PROMPT = None
        self.gre_prompt = None
        self.gre_backtrace_frame = None
        self.gre_breakpoint = None
        self.gre_thread = None
        # List of regexes that match debugger prompts for user input:
        self.ls_needs_user_input = []
        # Things like 'next 5' are allowed:
        self.b_has_count_commands = False
        self.b_coalesce_support = False
        # List index which is the topmost frame in a backtrace. Will be 0 for
        # gdb and -1 for python, because of the way they order their
        # backtraces. -2 is to check for initialization.
        self.n_top_backtrace_frame = -2
        
    def destroy(self):
        """Destroy any state associated with the Personality instance.
        May be overloaded for personality-specific behavior."""
        pass

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
        for m in l_matches:
            l_breakpoints.append(self._parse_one_breakpoint(m))
        return l_breakpoints

    def get_threads(self):
        """Return a list of 2-tuples (b_active, integer) where b_active is True
        for the currently active thread, and the integer is the gdb thread id
        (NOT system tid)."""
        return self._parse_thread_ids(self.do_info_threads())

    def _parse_thread_ids(self, s_threads):
        """Return a list of 2-tuples (b_active, integer) where b_active is True
        for the currently active thread, and the integer is the gdb thread id
        (NOT system tid) parsed from output of 'info threads' cmd."""
        l_threads = []
        l_matches = re.findall(self.gre_thread, s_threads, re.MULTILINE)
        for i in range(0, len(l_matches)):
            l_threads.append(self._parse_one_thread(l_matches[i]))
        return l_threads

    def _parse_one_thread(self, match_obj):
        """Return a thread id from the given re Match object.
        The Match object should be a tuple (the result of gre_thread)."""
        fredutil.fred_assert(False, "Must be implemented in subclass.")

    def _parse_one_breakpoint(self, match_obj):
        """Return a Breakpoint from the given re Match object.
        The Match object should be a tuple (the result of gre_breakpoint)."""
        fredutil.fred_assert(False, "Must be implemented in subclass.")

    def _parse_backtrace_internal(self, backtrace):
        """Return the backtrace list"""
        fredutil.fred_assert(False, "Must be implemented in subclass.")

    def _parse_backtrace(self, where_str):
        """Return a Backtrace instance parsed from output of 'where' cmd."""
        backtrace = re.sub(self.gre_prompt, '', where_str)
        bt_list = self._parse_backtrace_internal(backtrace)
        bt = debugger.Backtrace()
        for f in bt_list:
            bt.add_frame(self._parse_backtrace_frame(f))
        return bt

    def _parse_backtrace_frame(self, match_obj):
        """Return a BacktraceFrame from the given re Match object.
        The Match object should be a tuple (result of gre_backtrace_frame.)"""
        fredutil.fred_assert(False, "Must be implemented in subclass.")
        
    def execute_command(self, s_cmd, b_timeout=False, b_prompt=True,
                        b_drain_first=False):
        """Send the given string to debugger and return its output."""
        # Ensure it has strictly one newline:
        s_cmd = s_cmd.strip() + "\n"
        return fredio.get_child_response(s_cmd, b_timeout=b_timeout,
                                         b_wait_for_prompt=b_prompt,
                                         b_drain_first=b_drain_first)

    def do_next(self, n, b_timeout_prompt):
        """Perform n 'next' commands. Returns output."""
        cmd = self.GS_NEXT
        if self.b_has_count_commands:
            cmd += " " + str(n)
        return self.execute_command(cmd, b_timeout=b_timeout_prompt)
        
    def do_step(self, n, b_timeout_prompt):
        """Perform n 'step' commands. Returns output."""
        cmd = self.GS_STEP
        if self.b_has_count_commands:
            cmd += " " + str(n)
        return self.execute_command(cmd, b_timeout=b_timeout_prompt)
        
    def do_continue(self, b_wait_for_prompt):
        """Perform 'continue' command. Returns output."""
        return self.execute_command(self.GS_CONTINUE,
                                    b_prompt=b_wait_for_prompt)
        
    def do_breakpoint(self, expr):
        """Perform 'break expr' command. Returns output."""
        return self.execute_command(self.GS_BREAKPOINT + " " + str(expr))

    def do_finish(self):
        """Perform 'finish' command. Returns output."""
        return self.execute_command(self.GS_FINISH)

    def do_where(self):
        """Perform 'where' command. Returns output."""
        return self.execute_command(self.GS_WHERE)

    def do_info_breakpoints(self):
        """Perform 'info_breakpoints' command. Returns output."""
        return self.execute_command(self.GS_INFO_BREAKPOINTS)

    def do_info_threads(self):
        """Perform 'info threads' command. Returns output."""
        return self.execute_command(self.GS_INFO_THREADS)

    def do_print(self, expr, b_drain_first=False):
        """Perform 'print expr' command. Returns output."""
        return self.execute_command(self.GS_PRINT + " " + str(expr),
                                    b_drain_first=b_drain_first)

    def current_position(self):
        """Return a BacktraceFrame representing current debugger position."""
        fredutil.fred_assert(self.n_top_backtrace_frame != -2)
        l_frames = self.get_backtrace().get_frames()
        if len(l_frames) == 0:
            return None
        return l_frames[self.n_top_backtrace_frame]

    def switch_to_thread(self, n_tid):
        """Switch debugger to given thread."""
        return self.execute_command(self.GS_SWITCH_THREAD + " " + str(n_tid))

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
        else:
            cmd = freddebugger.fred_unknown_cmd()
        cmd.set_native(s_command.partition(' ')[0])
        cmd.s_args = s_command.partition(' ')[2]
        if cmd.b_count_cmd and cmd.s_args != "":
            cmd.set_count(int(cmd.s_args))
        return cmd        

    def get_native(self, fred_cmd):
        """Return native representation of given FredCommand."""
        if fred_cmd.is_next():
            return self.GS_NEXT
        elif fred_cmd.is_step():
            return self.GS_STEP
        elif fred_cmd.is_continue():
            return self.GS_CONTINUE
        elif fred_cmd.is_switch_thread():
            return self.GS_SWITCH_THREAD
        else:
            fredutil.fred_assert(False,
                                 "Don't know native representation of %s" %
                                 fred_cmd)

    def sanitize_print_result(self, s_printed):
        """Sanitize the result of a debugger 'print' command.
        This is to normalize out things like gdb's print result:
          $XX = 16
        Where $XX changes with each command executed."""
        fredutil.fred_assert(False, "Must be implemented in subclass.")

    def at_breakpoint(self, bt_frame, breakpoints):
        """Returns True if at a breakpoint"""
        for breakpoint in breakpoints:
            if breakpoint.s_file == bt_frame.s_file and \
               breakpoint.n_line == bt_frame.n_line:
                return True
        return False

