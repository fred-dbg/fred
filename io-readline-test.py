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
import select
import sys
import threading
import time

'''
First pass at defining a practical naming scheme.

Prefix     |    Meaning
-----------+------------
g_         |   global
n_         |   numeric
s_         |   string
l_         |   list
b_         |   boolean
f_         |   Python file object

All other non-basic types do not require a prefix other than scope-related.
For example, a mutex or thread object does not have a specific prefix.
However, the name of the variable should be descriptive enough to tell what
type it is meant to be.

Constants should be in all caps, including the prefix.
'''

gn_child_pid = -1
gf_child = None
gs_prompt = "(gdb) "
gb_prompt_ready = False
gb_hide_output = False
gb_capture_output = False
gs_captured_output = ""
g_capture_output_event = threading.Event()
g_output_thread = None

gf_child_stdin = None
gf_child_stdout = None

def last_n(s, source, n):
    '''
    Returns the last n characters of the concatenation of s+source.  This is
    used by the output loop to keep track of the last n characters read from
    the child.
    
    Examples/Tests:
    assert(last_n("abc", "efghijklmno", 5) == "klmno")
    assert(last_n("abcde", "fgh", 5) == "defgh")
    assert(last_n("abcde", "fghijkl", 5) == "hijkl")
    assert(last_n("abcdefghi", "wxyz", 5) == "iwxyz")
    '''
    return (s+source)[-n:]

assert(last_n("abc", "efghijklmno", 5) == "klmno")
assert(last_n("abcde", "fgh", 5) == "defgh")
assert(last_n("abcde", "fghijkl", 5) == "hijkl")
assert(last_n("abcdefghi", "wxyz", 5) == "iwxyz")

class ThreadedOutput(threading.Thread):
    def run(self):
        global gb_prompt_ready, g_output_lock, gb_capture_output, \
               gs_captured_output, g_capture_output_event, gb_hide_output
        # Last printed will be the last 'n' characters printed from child,
        # where n == len(gs_prompt). This is so we can know when the debugger
        # prompt has been printed to screen.
        last_printed = ""
        while 1:
            output = get_child_output()
            if output != None:
                if not gb_hide_output:
                    sys.stdout.write(output)
                    sys.stdout.flush()
                if gb_capture_output:
                    gs_captured_output += output
                    g_capture_output_event.set()
                last_printed = last_n(last_printed, output, len(gs_prompt))
            # Always keep this up-to-date:
            gb_prompt_ready = last_printed == gs_prompt

def start_output_thread():
    global g_output_thread
    g_output_thread = ThreadedOutput()
    g_output_thread.daemon = True
    g_output_thread.start()

def send_child_input(input):
    global gf_child
    os.write(gf_child.fileno(), input)
        
def get_child_output():
    global gf_child
    n = 1000
    # Don't use gf_child.read(n) here because it seems that will block until
    # it reads exactly n bytes.
    output = os.read(gf_child.fileno(), n)
    return output

def wait_for_prompt():
    global gb_prompt_ready, gf_child
    while not gb_prompt_ready:
        pass
    # Reset for next time
    gb_prompt_ready = False

def input_loop():
    while 1:
        wait_for_prompt()
        s = get_input()
        send_child_input(s+'\n')

def get_input():
    s = raw_input()
    s = s.strip()
    return s

def start_output_capture():
    global gb_capture_output, gs_captured_output, g_capture_output_event
    gb_capture_output = True
    g_capture_output_event.clear()

def wait_for_captured_output():
    global gb_capture_output, gs_captured_output, g_capture_output_event
    g_capture_output_event.wait()
    output = gs_captured_output
    gs_captured_output = ""
    gb_capture_output = False
    return output

def get_child_response(input, hide=True):
    ''' Sends requested input to child, and returns any response made.
    If hide flag is True (default), suppresses echoing from child.'''
    global gb_hide_output
    orig_hide_state = gb_hide_output
    gb_hide_output = hide
    # The output thread should be blocked on an os.read() call right now.
    start_output_capture()
    send_child_input(input)
    response = wait_for_captured_output()
    gb_hide_output = orig_hide_state
    return response

def fred_completer(text, state):
    current_cmd = readline.get_line_buffer()
    # Write partial command+\t to gdb so it can do the completion.
    result = get_child_response(current_cmd + '\t')
    result = result.replace(current_cmd, "")
    readline.insert_text(result)

def spawn_child(argv):
    ''' Spawns a child process using the given command array. '''
    global gn_child_pid, gf_child
    (gn_child_pid, child_fd) = pty.fork()
    if gn_child_pid == 0:
        os.execvp(argv[0], argv)
    else:
        gf_child = os.fdopen(child_fd, "wr", 0)
        #fcntl.fcntl(child_fd, fcntl.F_SETFL, os.O_NONBLOCK)

def main():
    ''' Program execution starts here. '''
    # Enable tab completion (with our own 'completer' function)
    readline.parse_and_bind('tab: complete')
    readline.set_completer(fred_completer)
    cmd = ["gdb"]
    spawn_child(cmd)
    start_output_thread()
    input_loop()
    
if __name__ == '__main__':
    main()
