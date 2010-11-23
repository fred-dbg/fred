import personality
import re

import freddebugger
import fredio
import fredutil

GS_WHERE_COMMAND="where"
gre_prompt=re.compile("\(gdb\) ")
# Basic stack trace format, matches this kind:
# "#0  *__GI___libc_malloc (bytes=8) at malloc.c:3551"
gre_backtrace_frame = '#(\d+)\s+(.+?)\s+\((.*)\)\s+at\s+(' + \
                      fredutil.gs_file_path_re + '):(\d+)'

class PersonalityGdb(personality.Personality):
    def __init__(self):
        personality.Personality.__init__(self)
        self.s_name = "gdb"

    def update_state(self):
        """Update the DebuggerState to the current time."""
        self.state().backtrace = parse_backtrace(self.where())
        
    def where(self):
        """Return output of 'where' command."""
        return fredio.get_child_response(GS_WHERE_COMMAND+'\n')

def parse_backtrace(where_str):
    """Return a Backtrace instance parsed from output of 'where' cmd."""
    backtrace = re.sub(gre_prompt, '', where_str)
    bt_list = re.findall(gre_backtrace_frame, backtrace, re.MULTILINE)
    frame_list = []
    bt = freddebugger.Backtrace()
    if len(bt_list) == 0:    
        return bt
    for i in range(0, len(bt_list)):
        frame_list[i] = freddebugger.BacktraceFrame()
        frame_list[i].n_frame_num = bt_list[i].group(0)
        frame_list[i].s_function = bt_list[i].group(1)
        frame_list[i].s_args = bt_list[i].group(2)
        frame_list[i].s_file = bt_list[i].group(3)
        frame_list[i].s_line = bt_list[i].group(4)
    bt.l_frames = frame_list
    return bt
