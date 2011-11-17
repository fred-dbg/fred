#!/usr/bin/python

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

import fcntl
import os
import pty
import readline
import re
import sys
import signal
import threading
import pdb
import select

import fredutil
import fredmanager

GB_FRED_DEMO = False
GB_FRED_DEMO_FROM_USER = False

GS_FRED_DEMO_HIDE = ['info files\n', 'info breakpoints\n', 'where\n']
GS_FRED_DEMO_UNHIDE_PREFIX = ['next', 'step']

# Maximum length of a prompt string (from any debugger)
GN_MAX_PROMPT_LENGTH = 32

# Maximum length of a string for requesting additional user input
gn_max_need_input_length = 0
# Pid of child (debugger)
gn_child_pid = -1
# File descriptor of child stdin/stdout.
gn_child_fd = None
# Will be set when debugger prompt is waiting for user input.
g_prompt_ready_event = threading.Event()
# When True, all output from debugger is hidden.
gb_hide_output = False
# When True, all output from debugger is appended to gs_captured_output.
gb_capture_output = False
# When True, all output until the debugger prompt appears is captured.
gb_capture_output_til_prompt = False
# When True, all output until the debugger prompt appears is captured,
# correctly handling multi-page input that requires user intervetion
# to display the next page.
gb_capture_output_multi_page = False
# Will be True when the output thread is alive.
gb_output_thread_alive = False
# Captured output from the debugger is stored here.
gs_captured_output = ""
# Synchronization object used between output thread and main thread.
g_capture_output_event = threading.Event()
# Regex (initialized at runtime) to match the debugger prompt.
gre_prompt = ""
# Function (initialized at runtime) to match the debugger prompt.
g_find_prompt_function = None
# Function (initialized at runtime) to print the debugger prompt.
g_print_prompt_function = None
# List of regexes which match strings indicating debugger needs user input.
# (e.g. gdb requires user to press enter when displaying multi-page text)
gls_needs_user_input = []
# Will be True when the debugger needs user input.
gb_need_user_input = False
# The last few characters printed by the child. This is used for detecting
# when the prompt is printed.
gs_last_printed = ""

# Functions beginning with an underscore ('_') should not be used outside of
# this file!

class ThreadedOutput(threading.Thread):
    def run(self):
        global g_prompt_ready_event, gb_capture_output, gs_captured_output, \
               g_capture_output_event, gb_capture_output_til_prompt, \
               gb_hide_output, gn_max_need_input_length, gb_need_user_input, \
               gb_capture_output_multi_page, gs_last_printed
        # Used to detect when debugger needs additional user input
        last_printed_need_input = ""
        while 1:
            output = _get_child_output()
            if output != None:
                gs_last_printed = fredutil.last_n(gs_last_printed, output,
                                               GN_MAX_PROMPT_LENGTH)
                last_printed_need_input = \
                    fredutil.last_n(last_printed_need_input, output,
                                    gn_max_need_input_length)
                if gb_capture_output:
                    gs_captured_output += output
                    if gb_capture_output_multi_page:
                        if _match_needs_user_input(last_printed_need_input):
                            _send_child_input("\n")
                    if gb_capture_output_til_prompt:
                        if g_find_prompt_function(gs_last_printed):
                            gs_last_printed = ""
                            g_capture_output_event.set()
                    else:
                        gs_last_printed = ""
                        g_capture_output_event.set()
                if not gb_hide_output:
                    # Always remove prompt from output so we can print it:
                    output = re.sub(gre_prompt, '', output)
                    sys.stdout.write(output)
                    sys.stdout.flush()
            # Always keep these up-to-date:
            gb_need_user_input = _match_needs_user_input(last_printed_need_input)
            if g_find_prompt_function(gs_last_printed) or gb_need_user_input:
                g_prompt_ready_event.set()


def _start_output_thread():
    """Start the output thread in daemon mode.
    A thread in daemon mode will not be joined upon program exit."""
    global gb_output_thread_alive
    o = ThreadedOutput()
    o.daemon = True
    o.start()
    gb_output_thread_alive = True

def _reset_last_printed():
    """Reset the tracking of the debugger's last few printed characters."""
    global gs_last_printed
    gs_last_printed = ""    

def _send_child_input(input):
    """Write the given input string to the child process."""
    global gn_child_fd
    os.write(gn_child_fd, input)
        
def _get_child_output():
    """Read and return a string of output from the child process."""
    global gn_child_fd
    output = None
    try:
        l_ready = select.select([gn_child_fd], [], [], 1)
        if l_ready[0] == [gn_child_fd]:
            output = os.read(gn_child_fd, 1000)
    except:
        return None
    return output

def wait_for_prompt():
    """Block until the global g_prompt_ready_event has been set by the output
    thread."""
    global g_prompt_ready_event, gb_need_user_input
    while True:
        g_prompt_ready_event.wait()
        if gb_need_user_input:
            # Happens when, for example, gdb prints more than one screen,
            # and the user must press 'return' to continue printing.
            user_input = raw_input().strip()
            _send_child_input(user_input + '\n')
            gb_need_user_input = False
        else:
            break
    # Reset for next time
    g_prompt_ready_event.clear()
    
def _start_output_capture(wait_for_prompt):
    """Start recording output from child into global gs_captured_output.
    wait_for_prompt flag will cause all output until the next debugger prompt
    to be saved."""
    global gb_capture_output, gs_captured_output, g_capture_output_event, \
           gb_capture_output_til_prompt
    _reset_last_printed()
    gb_capture_output_til_prompt = wait_for_prompt
    gb_capture_output = True
    g_capture_output_event.clear()

def _wait_for_captured_output(b_wait_for_prompt, b_multi_page):
    """Wait until output capture is done, and return captured output.
    The actual output capture is done by the output thread, and placed into
    global gs_captured_output. This function resets that global string when
    finished."""
    global gb_capture_output, gs_captured_output, g_capture_output_event, \
           gb_capture_output_til_prompt, gb_capture_output_multi_page
    gb_capture_output_til_prompt = b_wait_for_prompt
    gb_capture_output_multi_page = b_multi_page
    g_capture_output_event.wait()
    output = gs_captured_output
    gs_captured_output = ""
    gb_capture_output = False
    return output

def get_child_response(s_input, hide=True, b_wait_for_prompt=False,
                       b_multi_page=True):
    """Sends requested input to child, and returns any response made.
    If hide flag is True (default), suppresses echoing from child.  If
    wait_for_prompt flag is True, collects output until the debugger prompt is
    ready."""
    global gb_hide_output
    global GB_FRED_DEMO, GS_FRED_DEMO_HIDE, GS_FRED_DEMO_UNHIDE_PREFIX
    global GB_FRED_DEMO_FROM_USER
    if GB_FRED_DEMO and s_input in GS_FRED_DEMO_HIDE and \
       not GB_FRED_DEMO_FROM_USER:
        hide = True
    if GB_FRED_DEMO and \
       len([x for x in GS_FRED_DEMO_UNHIDE_PREFIX if s_input.startswith(x)])>0:
        hide=False
    GB_FRED_DEMO_FROM_USER = False # reset back to default, which is False
    b_orig_hide_state = gb_hide_output
    gb_hide_output = hide
    _start_output_capture(b_wait_for_prompt)
    _send_child_input(s_input)
    response = _wait_for_captured_output(b_wait_for_prompt, b_multi_page)
    gb_hide_output = b_orig_hide_state
    return response

def _set_max_needs_input_length():
    """Sets correct value of gn_max_need_input_length."""
    global gn_max_need_input_length, gls_needs_user_input
    n_max = 0
    for item in gls_needs_user_input:
        if len(item) > n_max:
            n_max = len(item)
    gn_max_need_input_length = n_max

def _match_needs_user_input(s_str):
    """Return True if any regexes in gls_needs_user_input match 's_str'."""
    global gls_needs_user_input
    for item in gls_needs_user_input:
        if re.search(item, s_str) != None:
            return True
    return False

def _fred_completer(text, state):
    """Custom completer function called when the user presses TAB."""
    s_current_cmd = readline.get_line_buffer()
    # Write partial command+\t to debuggerso it can do the completion.
    result = get_child_response(s_current_cmd + '\t')
    # Erase what text we already have:
    result = result.replace(s_current_cmd, "")
    readline.insert_text(result)

def _spawn_child(argv):
    """Spawn a child process using the given command array."""
    global gn_child_pid, gn_child_fd, gb_output_thread_alive
    if not gb_output_thread_alive:
        _start_output_thread()
    fredutil.fred_debug("Starting child '%s'" % str(argv))
    (gn_child_pid, gn_child_fd) = pty.fork()
    if gn_child_pid == 0:
        sys.stderr = sys.stdout
        os.execvp(argv[0], argv)

def kill_child():
    """Kill the child process."""
    global gn_child_fd
    if gn_child_pid == -1:
      return
    fredutil.fred_debug("Killing child process pid %d" % gn_child_pid)
    signal_child(signal.SIGKILL)
    os.close(gn_child_fd)
    
def signal_child(signum):
    """Send the signal to the child process."""
    global gn_child_pid
    os.kill(gn_child_pid, signum)
    if signum == signal.SIGKILL:
        os.waitpid(gn_child_pid, 0)

def child_is_alive():
    """Return True if the child process is still alive; False if not."""
    try:
        signal(0)
    except:
        return False
    return True

def get_child_pid():
    """Return the current child pid."""
    global gn_child_pid
    return gn_child_pid

def get_command():
    """Get a command from the user using raw_input."""
    global g_print_prompt_function
    GB_FRED_DEMO_FROM_USER = True
    return raw_input(g_print_prompt_function()).strip()
    #return raw_input("!").strip()

def send_command_nonblocking(command):
    """Send a command to the child process, and do not wait for prompt."""
    _send_child_input(command+'\n')

def send_command(command):
    """Send a command to the child process and wait for the prompt."""
    global g_prompt_ready_event, gb_need_user_input
    _send_child_input(command+'\n')
    g_prompt_ready_event.clear()
    gb_need_user_input = False
    wait_for_prompt()
    
def reexec(argv):
    """Replace the current child process with the new given one."""
    if GB_FRED_DEMO:
        print "===================== RESTARTING gdb ====================="
    fredutil.fred_debug("Replacing current child with '%s'" % str(argv))
    _spawn_child(argv)

def setup(l_argv, b_spawn_child=True):
    """Perform any setup needed to do i/o with the child process."""
    _set_max_needs_input_length()
    # Enable tab completion (with our own 'completer' function)
    #readline.parse_and_bind('tab: complete')
    #readline.set_completer(_fred_completer)
    if b_spawn_child:
        _spawn_child(["dmtcp_checkpoint",
                      "--quiet",
                      "--with-module",
                      fredmanager.get_fredhijack_path()] +
                     l_argv)

def teardown():
    """Perform any cleanup associated with fredio module."""
    _reset_last_printed()
    kill_child()
