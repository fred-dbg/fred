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

import personality
import re
import sys

import freddebugger
import fredutil

class PersonalityGdb(personality.Personality):
    def __init__(self):
        personality.Personality.__init__(self)
        self.s_name = "gdb"
        self.GS_NEXT = "next"
        self.GS_STEP = "step"
        self.GS_CONTINUE = "continue"
        self.GS_BREAKPOINT = "break"
        self.GS_WHERE = "where"
        self.GS_INFO_BREAKPOINTS = "info breakpoints"
        self.GS_PRINT = "print"
        
        self.gs_next_re = fredutil.getRE(self.GS_NEXT, 4) + "|^n"
        self.gs_step_re = fredutil.getRE(self.GS_STEP, 4) + "|^s"
        self.gs_continue_re = fredutil.getRE(self.GS_CONTINUE, 4) + "|^c"
        self.gs_breakpoint_re = fredutil.getRE(self.GS_BREAKPOINT)
        self.gs_where_re = fredutil.getRE(self.GS_WHERE, 3) + "|^bt"
        self.gs_info_breakpoints_re = \
            fredutil.getRE(self.GS_INFO_BREAKPOINTS, 5) + "|^i b"
        self.gs_print_re = fredutil.getRE(self.GS_PRINT, 5) + "|^p(/\w)?"
        
        self.GS_PROMPT = "(gdb) "
        self.gre_prompt = re.compile("\(gdb\) ")
        # Basic stack trace format, matches this kind:
        # "#0  *__GI___libc_malloc (bytes=8) at malloc.c:3551"
        self.gre_backtrace_frame = re.compile("#(\d+)\s+(.+?)\s+\((.*)\)"
                                              "\s+at\s+("
                                              + fredutil.GS_FILE_PATH_RE +
                                              "):(\d+)")
        self.gre_breakpoint = re.compile("(\d+)\s*(\w+)\s*(\w+)\s*(\w+)\s*"
                                         "(0x[0-9A-Fa-f]+)"
                                         "in ([a-zA-Z0-9_]+)\s+at ("
                                         + fredutil.GS_FILE_PATH_RE +
                                         "):(\D+)\s+(?:breakpoint already hit "
                                         "(\d+) time)?")

    def get_backtrace(self):
        """Return a Backtrace object representing the current backtrace."""
        return self._parse_backtrace(self.do_where())

    def get_breakpoints(self):
        """Return a list of Breakpoint objects for the current breakpoints."""
        return self._parse_breakpoints(self.do_info_breakpoints())

    def prompt_string(self):
        """Return the debugger's prompt string."""
        return self.GS_PROMPT

    def prompt(self):
        """Bring user back to debugger prompt."""
        sys.stdout.write(self.GS_PROMPT)
        sys.stdout.flush()

    def _parse_backtrace(self, where_str):
        """Return a Backtrace instance parsed from output of 'where' cmd."""
        backtrace = re.sub(self.gre_prompt, '', where_str)
        bt_list = re.findall(self.gre_backtrace_frame, backtrace, re.MULTILINE)
        frame_list = []
        bt = freddebugger.Backtrace()
        for i in range(0, len(bt_list)):
            frame_list[i] = self._parse_backtrace_frame(bt_list[i])
        bt.l_frames = frame_list
        return bt

    def _parse_backtrace_frame(self, match_obj):
        """Return a BacktraceFrame from the given re Match object.
        The Match object should be the result of gre_backtrace_frame."""
        frame = freddebugger.BacktraceFrame()
        frame.n_frame_num = match_obj.group(0)
        frame.s_function  = match_obj.group(1)
        frame.s_args      = match_obj.group(2)
        frame.s_file      = match_obj.group(3)
        frame.s_line      = match_obj.group(4)
        return frame

    def _parse_breakpoints(self, info_str):
        """Return a list of Breakpoint objects parsed from output of 'info
        breakpoints' cmd."""
        l_breakpoints = []
        l_matches = re.findall(self.gre_breakpoint, info_str, re.MULTILINE)
        for i in range(0, len(l_matches)):
            l_breakpoints[i] = self._parse_one_breakpoint(l_matches[i])
        return l_breakpoints

    def _parse_one_breakpoint(self, match_obj):
        """Return a Breakpoint from the given re Match object.
        The Match object should be the result of gre_breakpoint."""
        breakpoint = freddebugger.Breakpoint()
        breakpoint.n_number   = match_obj.group(0)
        breakpoint.s_type     = match_obj.group(1)
        breakpoint.s_display  = match_obj.group(2)
        breakpoint.s_enable   = match_obj.group(3)
        breakpoint.s_address  = match_obj.group(4)
        breakpoint.s_function = match_obj.group(5)
        breakpoint.s_file     = match_obj.group(6)
        breakpoint.n_line     = match_obj.group(7)
        breakpoint.n_count    = match_obj.group(8)
        return breakpoint
