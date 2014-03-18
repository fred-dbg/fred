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

import glob
import os
import personality
import re
import sys
import pdb

from .. import freddebugger
from .. import debugger
from .. import fredio
from .. import fredutil

gn_user_code_min = 0
gn_user_code_max = 0
gs_inferior_name = ""

class PersonalityPerl(personality.Personality):
    def __init__(self):
        personality.Personality.__init__(self)
        self.s_name = "perl"
        self.GS_NEXT = "n"
        self.GS_STEP = "s"
        self.GS_CONTINUE = "c"
        self.GS_BREAKPOINT = "b"
        self.GS_WHERE = "T"
        self.GS_INFO_BREAKPOINTS = "L b"
        self.GS_PRINT = "p"
        self.GS_FINISH = "r"
        self.GS_CURRENT_POS = "."

        self.gs_next_re = fredutil.getRE(self.GS_NEXT)
        self.gs_step_re = fredutil.getRE(self.GS_STEP)
        self.gs_continue_re = fredutil.getRE(self.GS_CONTINUE)
        self.gs_breakpoint_re = fredutil.getRE(self.GS_BREAKPOINT)
        self.gs_where_re = fredutil.getRE(self.GS_WHERE)
        self.gs_info_breakpoints_re = fredutil.getRE(self.GS_INFO_BREAKPOINTS)
        self.gs_print_re = fredutil.getRE(self.GS_PRINT)
        self.gs_program_not_running_re = \
  """Debugged program terminated.  Use q to quit or R to restart,
  use o inhibit_exit to avoid stopping after program termination,
  h q, h R or h o to get additional info."""

        self.GS_PROMPT = "DB"
        self.gre_prompt = self.GS_PROMPT + "<\d+?>"
        self.gre_backtrace_frame = ". = (\w+::\w+)\(?.*?\)? called from file `(.+)\' line (\d+)"
        self.gre_breakpoint = "\s*(\d+):\s+.+$"
        # List of regexes that match debugger prompts for user input
        self.ls_needs_user_input = []
        # Things like 'next 5' are allowed:
        self.b_has_count_commands = False 
        self.b_coalesce_support = False
        self.n_top_backtrace_frame = 0

    def prompt_string(self):
        """Return the debugger's prompt string."""
        return self.GS_PROMPT

    def prompt(self):
        """Bring user back to debugger prompt."""
        sys.stdout.write(self.GS_PROMPT)
        sys.stdout.flush()

    def sanitize_print_result(self, s_printed):
        """Sanitize the result of a debugger 'print' command.
        This is to normalize out things like gdb's print result:
          $XX = 16
        Where $XX changes with each command executed."""
        # Strip the control characters and \r
        s_printed = re.sub("\\x1b\[\d+?m", "", s_printed)
        s_printed = s_printed.replace("\r", "")
        # Perl gives us no print result formatting. We use the
        # following heuristic: find the first instance of the prompt
        # string, and take the output to be the text between the
        # prompt string and the previous newline char.
        # Ex: 'p $i < 3\n1\n DB<4>' should return '1'.
        lines = s_printed.split("\n")
        prevline = lines[0]
        for line in lines[1:]:
            if re.search(self.gre_prompt, line):
                return prevline
            prevline = line

    def _parse_backtrace_frame(self, match_obj):
        """Return a BacktraceFrame from the given re Match object.
        The Match object should be a tuple (result of gre_backtrace_frame.)"""
        frame = debugger.BacktraceFrame()
        frame.s_function  = match_obj[0]
        frame.s_file      = match_obj[1]
        frame.n_line      = int(match_obj[2])
        return frame

    def _parse_backtrace_internal(self, backtrace):
        result = re.findall(self.gre_backtrace_frame, backtrace)
        # Perl backtraces are empty at the top-level. The reverse-*
        # algorithms assume that a top-level backtrace has depth
        # 1. Therefore, we always prepend a dummy top-level backtrace
        # frame, although it is not displayed to the user.
        result = [("FReD::dummy", "fred-nofile", 0)] + result
        return result

    def _parse_one_breakpoint(self, match_obj):
        """Return a Breakpoint from the given re Match object.
        The Match object should be a tuple (the result of gre_breakpoint)."""
        breakpoint = debugger.Breakpoint()
        breakpoint.s_file     = match_obj[0]
        breakpoint.n_line     = int(match_obj[1])
        breakpoint.n_number   = int(match_obj[1])
        return breakpoint

    def _parse_breakpoints(self, info_str):
        """Return a list of Breakpoint objects parsed from output of 'info
        breakpoints' cmd."""
        perlFileRE = "(.+):"
        file = re.search(perlFileRE, info_str)
        l_breakpoints = []
        l_matches = re.findall(self.gre_breakpoint, info_str, re.MULTILINE)
        for m in l_matches:
            ff = file.group(1)
            ff = ff.replace("\x1b[m", "")
            l_breakpoints.append(self._parse_one_breakpoint((ff, m)))
        return l_breakpoints
