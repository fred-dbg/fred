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
from .. import fredio
from .. import fredutil
from .. import debugger

gn_user_code_min = 0
gn_user_code_max = 0
gs_inferior_name = ""

class PersonalityPython(personality.Personality):
    def __init__(self):
        personality.Personality.__init__(self)
        self.s_name = "Pdb"
        self.GS_NEXT = "next"
        self.GS_STEP = "step"
        self.GS_CONTINUE = "continue"
        self.GS_BREAKPOINT = "break"
        self.GS_WHERE = "where"
        self.GS_INFO_BREAKPOINTS = "b"
        self.GS_PRINT = "print"
        self.GS_FINISH = "return\nnext"
        self.GS_CURRENT_POS = "where"

        self.gs_next_re = fredutil.getRE(self.GS_NEXT, 4) + "|^n$|^n\s+.*$"
        self.gs_step_re = fredutil.getRE(self.GS_STEP, 4) + "|^s$|^s\s+.*$"
        self.gs_continue_re = fredutil.getRE(self.GS_CONTINUE, 3) + "|^c$"
        self.gs_breakpoint_re = fredutil.getRE(self.GS_BREAKPOINT)
        self.gs_where_re = fredutil.getRE(self.GS_WHERE, 3) + "|^bt"
        self.gs_info_breakpoints_re = fredutil.getRE(self.GS_INFO_BREAKPOINTS)
        self.gs_print_re = fredutil.getRE(self.GS_PRINT, 5) + "|^p(/\w)?"
        self.gs_program_not_running_re = "The program finished and will be restarted"

        self.GS_PROMPT = "(Pdb) "
        self.gre_prompt = re.compile("\(Pdb\) $")
        self.gre_backtrace_frame = ".+/(.+?)\((\d+)\)(.+?)\(.*?\).*?\n-\>"
        self.gre_breakpoint = "(\d+)\s+(\w+)\s+(\w+)\s+(\w+)\s+at .+/(.+):(\d+)\s+(?:breakpoint already hit (\d+) time)?"
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
        exp = "\$[0-9]+ = (.+)"
        m = re.search(exp, s_printed)
        if m == None:
            return s_printed
        else:
            return m.group(1)

    def _parse_backtrace_frame(self, match_obj):
        """Return a BacktraceFrame from the given re Match object.
        The Match object should be a tuple (result of gre_backtrace_frame.)"""
        frame = debugger.BacktraceFrame()
        frame.s_file      = match_obj[0]
        frame.n_line      = int(match_obj[1])
        frame.s_function  = match_obj[2]
        return frame

    def _parse_one_breakpoint(self, match_obj):
        """Return a Breakpoint from the given re Match object.
        The Match object should be a tuple (the result of gre_breakpoint)."""
        breakpoint = debugger.Breakpoint()
        breakpoint.n_number   = int(match_obj[0])
        breakpoint.s_type     = match_obj[1]
        breakpoint.s_display  = match_obj[2]
        breakpoint.s_enable   = match_obj[3]
        breakpoint.s_file     = match_obj[4]
        breakpoint.n_line     = int(match_obj[5])
        breakpoint.n_count    = fredutil.to_int(match_obj[6])
        return breakpoint

    def _parse_backtrace_internal(self, backtrace):
        return re.findall(self.gre_backtrace_frame, backtrace, re.MULTILINE)
