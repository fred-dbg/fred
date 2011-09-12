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

import freddebugger
import fredio
import fredutil

gn_user_code_min = 0
gn_user_code_max = 0
gs_inferior_name = ""

class PersonalityMatlab(personality.Personality):
    def __init__(self):
        personality.Personality.__init__(self)
        self.s_name = "MATLAB"
        self.GS_NEXT = "dbstep"
        self.GS_STEP = "dbstep in"
        self.GS_CONTINUE = "dbcont"
        self.GS_BREAKPOINT = "dbstop"
        self.GS_WHERE = "dbstack"
        self.GS_INFO_BREAKPOINTS = "dbstatus"
        self.GS_PRINT = None
        self.GS_FINISH = "dbstep out"
        self.GS_CURRENT_POS = "dbstack"
        
        self.gs_next_re = fredutil.getRE(self.GS_NEXT)
        self.gs_step_re = fredutil.getRE(self.GS_STEP)
        self.gs_continue_re = fredutil.getRE(self.GS_CONTINUE)
        self.gs_breakpoint_re = fredutil.getRE(self.GS_BREAKPOINT)
        self.gs_where_re = fredutil.getRE(self.GS_WHERE)
        self.gs_info_breakpoints_re = fredutil.getRE(self.GS_INFO_BREAKPOINTS)
        self.gs_program_not_running_re = "Error using ==>"
        
        self.GS_PROMPT = ">> "
        self.gre_prompt = "\w?" + self.GS_PROMPT
        self.gre_backtrace_frame = "^\>?\s*In (.+) (?:at|after) (\d+)"
        self.gre_breakpoint = "Breakpoints for (.+) (?:are|is) on lines? (.+)" 
        # List of regexes that match debugger prompts for user input
        self.ls_needs_user_input = []
        # Things like 'next 5' are allowed:
        self.b_has_count_commands = True
        self.b_coalesce_support = False
        # Gdb orders backtraces with topmost at the beginning (list idx 0):
        self.n_top_backtrace_frame = 0
        # GDB only: name of inferior process.
        self.s_inferior_name = ""
        
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
        frame = freddebugger.BacktraceFrame()
        frame.s_file      = match_obj[0]
        frame.n_line      = int(match_obj[1])
        return frame

    def _parse_one_breakpoint(self, match_obj):
        """Return a Breakpoint from the given re Match object.
        The Match object should be a tuple (the result of gre_breakpoint)."""
        breakpoint = freddebugger.Breakpoint()
        breakpoint.s_function     = match_obj[0]
        breakpoint.n_line         = int(match_obj[1])
        return breakpoint

    def _parse_breakpoints(self, info_str):
        """Return a list of Breakpoint objects parsed from output of 'info
        breakpoints' cmd."""
        info_str = info_str.replace("\r", "")
        brkpLineNumbersPerFiles = \
        re.findall(self.gre_breakpoint, info_str, re.MULTILINE)
        l_breakpoints = []
        for b in brkpLineNumbersPerFiles:
            if b[1].find(',') != -1:
                tmp = b[1].replace(", ", "\n")
            brkpLineNumbers = re.findall("(\d+)", tmp, re.MULTILINE)
            for brkpLineNumber in brkpLineNumbers:
                breakpoint = freddebugger.Breakpoint()
                breakpoint.s_file         = b[0]
                breakpoint.n_line         = int(brkpLineNumber)
                l_breakpoints.append(breakpoint)
        return l_breakpoints
