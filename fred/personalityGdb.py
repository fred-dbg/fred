import personality
import re

import freddebugger
import fredio
import fredutil

GS_INFO_BREAKPOINTS_COMMAND = "info breakpoints"
GS_WHERE_COMMAND = "where"
gre_prompt = re.compile("\(gdb\) ")
# Basic stack trace format, matches this kind:
# "#0  *__GI___libc_malloc (bytes=8) at malloc.c:3551"
gre_backtrace_frame = re.compile("#(\d+)\s+(.+?)\s+\((.*)\)\s+at\s+(" +
                                 fredutil.gs_file_path_re + "):(\d+)")
gre_breakpoint = re.compile("(\d+)\s*(\w+)\s*(\w+)\s*(\w+)\s*(0x[0-9A-Fa-f]+)" +
                            "in ([a-zA-Z0-9_]+)\s+at (" + fredutil.gs_file_path_re +
                            "):(\d+)\s+(?:breakpoint already hit (\d+) time)?")

class PersonalityGdb(personality.Personality):
    def __init__(self):
        personality.Personality.__init__(self)
        self.s_name = "gdb"

    def update_state(self):
        """Update the DebuggerState to the current time."""
        self.state().backtrace     = _parse_backtrace(self.where())
        self.state().l_breakpoints = _parse_breakpoints(self.list_breakpoints())
        
    def where(self):
        """Return output of 'where' command."""
        return fredio.get_child_response(GS_WHERE_COMMAND+'\n')

    def list_breakpoints(self):
        """Return output of 'info breakpoints' command."""
        return fredio.get_child_response(GS_INFO_BREAKPOINTS_COMMAND+'\n')

    def contains_prompt_str(self, string):
        """Return True if given string matches the prompt string."""
        return re.search(gre_prompt, string) != None

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
