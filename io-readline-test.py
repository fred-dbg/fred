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
STYLE CONVENTIONS
-----------------

We should follow the style conventions in PEP-8, with some additions:
  <http://www.python.org/dev/peps/pep-0008/>

The most obvious ones are:

 - No "camelCase" style should be used. Use the lower_case_with_underscore
   style for functions and variables.
 - Max line length 79 characters.
 - Indentation is 4 spaces (NOT literal tab (\t) characters)
 - Avoid extraneous whitespace such as:
     Yes: spam(ham[1], {eggs: 2})
     No:  spam( ham[ 1 ], { eggs: 2 } )

In addition, all global variables should have a prefix to indicate their
scope and type. The following prefixes will be used:

Prefix     |    Meaning
-----------+------------
g?_        |   global
n_         |   numeric
s_         |   string
l_         |   list
b_         |   boolean
f_         |   Python file object

The ? after g_ is used to indicate the presence of an additional type
indicator.  For example a global integer should be name like:
  gn_a_global_number
To indicate global scope, and numeric type.

All other non-basic types do not require a prefix other than scope-related.
For example, a mutex or thread object does not have a specific prefix.
However, the name of the variable should be descriptive enough to tell what
type it is meant to be.

Please make it a goal to document every putlic function with a docstring. It
only takes a few seconds, and saves a lot of time for future programmers. For
docstring conventions, you can see PEP-257.
'''

gn_child_pid = -1
gn_child_fd = None
gs_prompt = "(gdb) "
gb_prompt_ready = False
gb_hide_output = False
gb_capture_output = False
gb_capture_output_til_prompt = False
gs_captured_output = ""
g_capture_output_event = threading.Event()

def last_n(s, source, n):
    """ Return the last n characters of the concatenation of s+source.
    This is used by the output loop to keep track of the last n characters read
    from the child.
    
    Examples/Tests:
    assert(last_n("abc", "efghijklmno", 5) == "klmno")
    assert(last_n("abcde", "fgh", 5) == "defgh")
    assert(last_n("abcde", "fghijkl", 5) == "hijkl")
    assert(last_n("abcdefghi", "wxyz", 5) == "iwxyz")
    """
    return (s+source)[-n:]

assert(last_n("abc", "efghijklmno", 5) == "klmno")
assert(last_n("abcde", "fgh", 5) == "defgh")
assert(last_n("abcde", "fghijkl", 5) == "hijkl")
assert(last_n("abcdefghi", "wxyz", 5) == "iwxyz")

class ThreadedOutput(threading.Thread):
    def run(self):
        global gb_prompt_ready, gb_capture_output, gs_captured_output, \
               g_capture_output_event, gb_capture_output_til_prompt, \
               gb_hide_output
        # Last printed will be the last 'n' characters printed from child,
        # where n == len(gs_prompt). This is so we can know when the debugger
        # prompt has been printed to screen.
        last_printed = ""
        while 1:
            output = get_child_output()
            if output != None:
                last_printed = last_n(last_printed, output, len(gs_prompt))
                if not gb_hide_output:
                    sys.stdout.write(output)
                    sys.stdout.flush()
                if gb_capture_output:
                    gs_captured_output += output
                    if gb_capture_output_til_prompt:
                        if last_printed == gs_prompt:
                            g_capture_output_event.set()
                    else:
                        g_capture_output_event.set()
            # Always keep this up-to-date:
            gb_prompt_ready = last_printed == gs_prompt

def start_output_thread():
    """Start the output thread in daemon mode.
    A thread in daemon mode will not be joined upon program exit."""
    o = ThreadedOutput()
    o.daemon = True
    o.start()

def send_child_input(input):
    """Write the given input string to the child process."""
    global gn_child_fd
    os.write(gn_child_fd, input)
        
def get_child_output():
    """Read and return a string of output from the child process."""
    global gn_child_fd
    output = os.read(gn_child_fd, 1000)
    return output

def wait_for_prompt():
    """Spin until the global gb_prompt_ready flag has been set to True.
    gb_prompt_ready is set by the output thread."""
    global gb_prompt_ready
    while not gb_prompt_ready:
        pass
    # Reset for next time
    gb_prompt_ready = False

def start_output_capture(wait_for_prompt):
    """Start recording output from child into global gs_captured_output.
    wait_for_prompt flag will cause all output until the next debugger prompt
    to be saved."""
    global gb_capture_output, gs_captured_output, g_capture_output_event, \
           gb_capture_output_til_prompt
    gb_capture_output_til_prompt = wait_for_prompt
    gb_capture_output = True
    g_capture_output_event.clear()

def wait_for_captured_output(wait_for_prompt):
    """Wait until output capture is done, and return captured output.
    The actual output capture is done by the output thread, and placed into
    global gs_captured_output. This function resets that global string when
    finished."""
    global gb_capture_output, gs_captured_output, g_capture_output_event, \
           gb_capture_output_til_prompt
    gb_capture_output_til_prompt = wait_for_prompt
    g_capture_output_event.wait()
    output = gs_captured_output
    gs_captured_output = ""
    gb_capture_output = False
    return output

def get_child_response(input, hide=True, wait_for_prompt=False):
    """Sends requested input to child, and returns any response made.
    If hide flag is True (default), suppresses echoing from child.  If
    wait_for_prompt flag is True, collects output until the debugger prompt is
    ready."""
    global gb_hide_output
    orig_hide_state = gb_hide_output
    gb_hide_output = hide
    start_output_capture(wait_for_prompt)
    send_child_input(input)
    response = wait_for_captured_output(wait_for_prompt)
    gb_hide_output = orig_hide_state
    return response

def fred_completer(text, state):
    """Custom completer function called when the user presses TAB."""
    current_cmd = readline.get_line_buffer()
    # Write partial command+\t to debuggerso it can do the completion.
    result = get_child_response(current_cmd + '\t')
    # Erase what text we already have:
    result = result.replace(current_cmd, "")
    readline.insert_text(result)

def spawn_child(argv):
    """Spawns a child process using the given command array."""
    global gn_child_pid, gn_child_fd
    (gn_child_pid, gn_child_fd) = pty.fork()
    if gn_child_pid == 0:
        os.execvp(argv[0], argv)

def main():
    """Program execution starts here."""
    # Enable tab completion (with our own 'completer' function)
    readline.parse_and_bind('tab: complete')
    readline.set_completer(fred_completer)
    cmd = ["gdb"]
    spawn_child(cmd)
    start_output_thread()
    # Enter main input loop:
    while 1:
        wait_for_prompt()
        s = raw_input().strip()
        send_child_input(s+'\n')
    
if __name__ == '__main__':
    main()
