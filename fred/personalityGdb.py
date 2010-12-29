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
import fredio
import fredutil

GS_NEXT = "next"
GS_STEP = "step"
GS_CONTINUE = "continue"
GS_BREAKPOINT = "break"
GS_WHERE = "where"
GS_INFO_BREAKPOINTS = "info breakpoints"
GS_PRINT = "print"

gs_next_re = fredutil.getRE(GS_NEXT, 4) + "|^n"
gs_step_re = fredutil.getRE(GS_STEP, 4) + "|^s"
gs_continue_re = fredutil.getRE(GS_CONTINUE, 4) + "|^c"
gs_breakpoint_re = fredutil.getRE(GS_BREAKPOINT)
gs_where_re = fredutil.getRE(GS_WHERE, 3) + "|^bt"
gs_info_breakpoints_re = fredutil.getRE(GS_INFO_BREAKPOINTS, 5) + "|^i b"
gs_print_re = fredutil.getRE(GS_PRINT, 5) + "|^p(/\w)?"

GS_PROMPT = "(gdb) "
gre_prompt = re.compile("\(gdb\) ")
# Basic stack trace format, matches this kind:
# "#0  *__GI___libc_malloc (bytes=8) at malloc.c:3551"
gre_backtrace_frame = re.compile("#(\d+)\s+(.+?)\s+\((.*)\)\s+at\s+(" +
                                 fredutil.GS_FILE_PATH_RE + "):(\d+)")
gre_breakpoint = re.compile("(\d+)\s*(\w+)\s*(\w+)\s*(\w+)\s*(0x[0-9A-Fa-f]+)" +
                            "in ([a-zA-Z0-9_]+)\s+at (" + fredutil.GS_FILE_PATH_RE +
                            "):(\D+)\s+(?:breakpoint already hit (\d+) time)?")

class PersonalityGdb(personality.Personality):
    def __init__(self):
        personality.Personality.__init__(self)
        self.s_name = "gdb"

    def get_backtrace(self):
        """Return a Backtrace object representing the current backtrace."""
        return _parse_backtrace(self.do_where())

    def get_breakpoints(self):
        """Return a list of Breakpoint objects for the current breakpoints."""
        return _parse_breakpoints(self.do_info_breakpoints())

    def do_next(self, n):
        """Perform n 'next' commands. Returns output."""
        return fredio.get_child_response(GS_NEXT + " " + str(n))
        
    def do_step(self, n):
        """Perform n 'step' commands. Returns output."""
        return fredio.get_child_response(GS_STEP + " " + str(n))
        
    def do_continue(self, n):
        """Perform n 'continue' commands. Returns output."""
        return fredio.get_child_response(GS_CONTINUE + " " + str(n))
        
    def do_breakpoint(self, expr):
        """Perform 'break expr' command. Returns output."""
        return fredio.get_child_response(GS_BREAKPOINT + " " + str(expr))

    def do_where(self):
        """Perform 'where' command. Returns output."""
        return fredio.get_child_response(GS_WHERE)

    def do_info_breakpoints(self):
        """Perform 'info_breakpoints' command. Returns output."""
        return fredio.get_child_response(GS_INFO_BREAKPOINTS)

    def do_print(self, expr):
        """Perform 'print expr' command. Returns output."""
        return fredio.get_child_response(GS_PRINT + " " + str(expr))

    def identify_command(self, s_command):
        """Returns a FredCommand representing given personality command."""
        if re.search(gs_next_re, s_command) != None:
            return freddebugger.fred_next_cmd()
        if re.search(gs_step_re, s_command) != None:
            return freddebugger.fred_step_cmd()
        if re.search(gs_continue_re, s_command) != None:
            return freddebugger.fred_continue_cmd()
        if re.search(gs_breakpoint_re, s_command) != None:
            return freddebugger.fred_breakpoint_cmd()
        if re.search(gs_where_re, s_command) != None:
            return freddebugger.fred_where_cmd()
        if re.search(gs_info_breakpoints_re, s_command) != None:
            return freddebugger.fred_info_breakpoints_cmd()
        if re.search(gs_print_re, s_command) != None:
            return freddebugger.fred_print_cmd()
        return freddebugger.fred_unknown_cmd()

    def contains_prompt_str(self, string):
        """Return True if given string matches the prompt string."""
        return re.search(gre_prompt, string) != None

    def prompt(self):
        """Bring user back to debugger prompt."""
        display_prompt()

def _parse_backtrace(where_str):
    """Return a Backtrace instance parsed from output of 'where' cmd."""
    global gre_prompt, gre_backtrace_frame
    backtrace = re.sub(gre_prompt, '', where_str)
    bt_list = re.findall(gre_backtrace_frame, backtrace, re.MULTILINE)
    frame_list = []
    bt = freddebugger.Backtrace()
    for i in range(0, len(bt_list)):
        frame_list[i] = _parse_backtrace_frame(bt_list[i])
    bt.l_frames = frame_list
    return bt

def _parse_backtrace_frame(match_obj):
    """Return a BacktraceFrame from the given re Match object.
    The Match object should be the result of gre_backtrace_frame."""
    frame = freddebugger.BacktraceFrame()
    frame.n_frame_num = match_obj.group(0)
    frame.s_function  = match_obj.group(1)
    frame.s_args      = match_obj.group(2)
    frame.s_file      = match_obj.group(3)
    frame.s_line      = match_obj.group(4)
    return frame

def _parse_breakpoints(info_str):
    """Return a list of Breakpoint objects parsed from output of 'info
    breakpoints' cmd."""
    global gre_breakpoint
    l_breakpoints = []
    l_matches = re.findall(gre_breakpoint, info_str, re.MULTILINE)
    for i in range(0, len(l_matches)):
        l_breakpoints[i] = _parse_one_breakpoint(l_matches[i])
    return l_breakpoints

def _parse_one_breakpoint(match_obj):
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

def display_prompt():
    """Prints the prompt to the screen."""
    sys.stdout.write(GS_PROMPT)
    sys.stdout.flush()
