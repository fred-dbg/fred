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

from .. import fredmanager
from .. import freddebugger
from .. import debugger
from .. import fredio
from .. import fredutil

GL_LIBRARY_BLACKLIST_NAMES = ["dmtcphijack",
                              "libmtcp",
                              "fredhijack",
                              "ptracehijack",
                              "pidvirt",
                              "libc-2",
                              "libpthread-2"]
gl_library_blacklist_code_ranges = []
gs_inferior_name = ""

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
        self.GS_INFO_THREADS = "info threads"
        self.GS_PRINT = "print"
        self.GS_FINISH = "finish"
        self.GS_CURRENT_POS = "where 1"
        self.GS_SWITCH_THREAD = "thread"
        
        self.gs_next_re = fredutil.getRE(self.GS_NEXT, 4) + "|^n$|^n\s+.*$"
        self.gs_step_re = fredutil.getRE(self.GS_STEP, 4) + "|^s$|^s\s+.*$"
        self.gs_continue_re = fredutil.getRE(self.GS_CONTINUE, 3) + "|^c$"
        self.gs_breakpoint_re = fredutil.getRE(self.GS_BREAKPOINT)
        self.gs_where_re = fredutil.getRE(self.GS_WHERE, 3) + "|^bt"
        self.gs_info_breakpoints_re = \
            fredutil.getRE(self.GS_INFO_BREAKPOINTS, 5) + "|^i b"
        self.gs_print_re = fredutil.getRE(self.GS_PRINT, 5) + "|^p(/\w)?"
        self.gs_program_not_running_re = "No stack."
        
        self.GS_PROMPT = "(gdb) "
        self.gre_prompt = re.compile("\(gdb\) $")
        # Basic stack trace format, matches this kind:
        # "#0  *__GI___libc_malloc (bytes=8) at malloc.c:3551"
        self.gre_backtrace_frame = "^#(\d+)\s+(0x[0-9a-f]+ in )?(.+?)\s+\((.*?)\)\s+at\s+(" \
                                   + fredutil.GS_FILE_PATH_RE + \
                                   "):(\d+)"
        self.gre_breakpoint = "(\d+)\s*(\w+)\s*(\w+)\s*(\w+)\s*" \
                              "(0x[0-9A-Fa-f]+)" \
                              " in ([a-zA-Z0-9_]+)\s+at (" \
                              + fredutil.GS_FILE_PATH_RE + \
                              "):(\d+)\s+(?:breakpoint already hit " \
                              "(\d+) time)?"
        # Matches gdb thread ids from "info threads":
        self.gre_thread = "^(\*?)\s*(\d+)\s*Thread"
        # List of regexes that match debugger prompts for user input
        self.ls_needs_user_input = \
        [ "---Type <return> to continue, or q <return> to quit---",
          ".+ \(\[?y\]? or \[?n\]?\)" ]
        # Things like 'next 5' are allowed:
        self.b_has_count_commands = True
        self.b_coalesce_support = True
        # Gdb orders backtraces with topmost at the beginning (list idx 0):
        self.n_top_backtrace_frame = 0
        # GDB only: name of inferior process.
        self.s_inferior_name = ""
        
    def destroy(self):
        """Destroy any state associated with the Personality instance."""
        global gs_inferior_name
        self.reset_user_code_interval()
        gs_inferior_name = ""
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

    def do_step(self, n, b_timeout_prompt=False):
        """Override generic do_step() from personality.py so we can avoid
        stepping into libc, etc."""
        # If the 'step' results in an address of something that is outside of
        # the user's code, execute a 'finish', and replace the 'step' in
        # history with a 'next', so on replay only the next is executed.
        output = self.execute_command(self.GS_STEP + " " + str(n),
                                      b_timeout=b_timeout_prompt)
        if not self.within_user_code():
            self.execute_command(self.GS_FINISH,
                                 b_timeout=b_timeout_prompt)
            # TODO: Think of more portable way to do this:
            return "DO-NOT-STEP"
        return output

    def _parse_backtrace_frame(self, match_obj):
        """Return a BacktraceFrame from the given re Match object.
        The Match object should be a tuple (result of gre_backtrace_frame.)"""
        frame = debugger.BacktraceFrame()
        frame.n_frame_num = int(match_obj[0])
        frame.s_addr      = match_obj[1]
        frame.s_function  = match_obj[2]
        frame.s_args      = match_obj[3]
        frame.s_file      = match_obj[4]
        frame.n_line      = int(match_obj[5])
        return frame

    def _parse_one_thread(self, match_obj):
        """Return a 2-tuple: (b_active, tid) from the given re Match object.
        The Match object should be a tuple (the result of gre_thread).
        b_active is True if the tid is the current active thread."""
        b_active = (match_obj[0] != "")
        return (b_active, int(match_obj[1]))

    def _parse_one_breakpoint(self, match_obj):
        """Return a Breakpoint from the given re Match object.
        The Match object should be a tuple (the result of gre_breakpoint)."""
        breakpoint = debugger.Breakpoint()
        breakpoint.n_number   = int(match_obj[0])
        breakpoint.s_type     = match_obj[1]
        breakpoint.s_display  = match_obj[2]
        breakpoint.s_enable   = match_obj[3]
        breakpoint.s_address  = match_obj[4]
        breakpoint.s_function = match_obj[5]
        breakpoint.s_file     = match_obj[6]
        breakpoint.n_line     = int(match_obj[7])
        breakpoint.n_count    = fredutil.to_int(match_obj[8])
        return breakpoint

    def set_inferior_name(self):
        """Set the inferior name to what 'info inferiors' tells us."""
        exp = "Local exec file:\s+`(.+?)'"
        s_info_files = fredio.get_child_response("info files\n",
                                                 b_multi_page=True,
                                                 b_wait_for_prompt=True)
        match = re.search(exp, s_info_files, re.MULTILINE)
        if match == None:
            fredutil.fred_fatal("Unable to get name of inferior process. "
                                "Is executable available?")
        self.s_inferior_name = match.group(1).strip()

    def reset_user_code_interval(self):
        """Reset code intervals (for restarts)."""
        global gl_library_blacklist_code_ranges
        del gl_library_blacklist_code_ranges[:]

    def is_blacklisted_address(self, n_addr):
        """Return True if the given address is within some blacklisted address
        range (i.e. do not step into)."""
        global gl_library_blacklist_code_ranges
        for code_range in gl_library_blacklist_code_ranges:
            if n_addr >= code_range[0] and n_addr < code_range[1]:
                return True
        return False

    def within_user_code(self, n_addr=-1):
        """Return True if n_addr is within the user program's code segment."""
        # XXX: TODO: Need to check all user libraries too (firefox)
        global gl_library_blacklist_code_ranges, gs_inferior_name
        if gs_inferior_name == "":
            fredutil.fred_assert(self.s_inferior_name != "",
                                 "Empty inferior name.")
            gs_inferior_name = self.s_inferior_name
        s_cur_func = ""
        if n_addr == -1:
            bt = self.get_backtrace()
            s_cur_func = bt.l_frames[0].s_function
            n_addr = self.parse_address(self.do_print("&'%s'" % s_cur_func))
        if len(gl_library_blacklist_code_ranges) == 0:
            self.get_code_addresses()
        result = self.is_blacklisted_address(n_addr)
        fredutil.fred_debug("Address 0x%x is blacklisted? %s" % (n_addr, str(result)))
        if not result and s_cur_func == "pthread_mutex_lock":
            # This is a bug that occurs very rarely, so I'm leaving
            # this trace in until I can figure it out. -Tyler
            pdb.set_trace()
        # Return the negation of result because result is True if the address
        # is blacklisted.
        return not result

    def set_scheduler_locking(self, b_value):
        """Set gdb scheduler locking to b_value."""
        s_value = "on" if b_value else "off"
        self.execute_command("set scheduler-locking %s" % s_value)

    def enable_sigstop(self):
        """Enable passing of SIGSTOP to inferior."""
        self.execute_command("handle SIGSTOP stop noignore")

    def disable_sigstop(self):
        """Disable passing of SIGSTOP to inferior."""
        self.execute_command("handle SIGSTOP nostop ignore")

    def parse_address(self, s_addr):
        """Parse the given address string from gdb and return a number."""
        # Example input: "$2 = (int (*)(item *)) 0x8048508 <list_len>"
        exp = ".+\(.+\)\s+(0x[0-9A-Fa-f]+).+"
        # Use DOTALL mode so the '.' character matches across
        # newlines, in case the output by gdb spanned multiple lines.
        m = re.search(exp, s_addr, re.DOTALL)
        if m != None:
            n_addr = int(m.group(1), 16)
            return n_addr
        # TODO: hackish: return an address within the user space to fool whoever
        # is calling this.  need to investigate why m can be None.
        fredutil.fred_assert(False, "Unimplemented");

    def is_blacklisted_library(self, s_maps_line):
        """Given a line from /proc/pid/maps, return True if the image name is
        in the blacklist."""
        global GL_LIBRARY_BLACKLIST_NAMES
        for blacklisted in GL_LIBRARY_BLACKLIST_NAMES:
            if re.search(blacklisted, s_maps_line):
                return True
        return False

    def get_code_addresses(self):
        """Get code ranges from /proc/pid/maps."""
        global gl_library_blacklist_code_ranges
        n_inferior_pid = fredmanager.get_real_inferior_pid()
        fredutil.fred_assert(n_inferior_pid != -1,
                             "Error finding inferior pid.")
        permissions_re = "\sr.xp\s" # Matches an executable segment in proc maps
        interval_re = "([0-9A-Fa-f]+)-([0-9A-Fa-f]+)\s.+"
        f = open("/proc/%d/maps" % n_inferior_pid, "r")
        executable_lines = []
        for line in f:
            if self.is_blacklisted_library(line):
                # There may be more than one executable segment (for instance,
                # the current iteration of DMTCP trampolines splits the code
                # segment into several).
                if re.search(permissions_re, line) != None:
                    executable_lines.append(line)
        f.close()
        fredutil.fred_assert(len(executable_lines) > 0,
            "Failed to find executable in /proc/%d/maps :" % n_inferior_pid)
        for executable_line in executable_lines:
            n_min = \
                int(re.search(interval_re, executable_line).group(1), 16)
            n_max = \
                int(re.search(interval_re, executable_line).group(2), 16)
            fredutil.fred_debug("Blacklist: (0x%x, 0x%x)" % (n_min, n_max))
            gl_library_blacklist_code_ranges.append((n_min, n_max))

    def at_breakpoint(self, bt_frame, breakpoints):
        for breakpoint in breakpoints:
            if breakpoint.s_function == bt_frame.s_function and \
               breakpoint.s_file == bt_frame.s_file and \
               breakpoint.n_line == bt_frame.n_line:
                return True
        return False

    def _parse_backtrace_internal(self, backtrace):
        return re.findall(self.gre_backtrace_frame,backtrace,
                          re.MULTILINE | re.DOTALL)
    
