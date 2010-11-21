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
import subprocess
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

GB_USE_SUBPROCESS = False

gn_child_pid = -1
gf_child = None
gs_prompt = "(gdb) "
gb_prompt_ready = False
g_output_thread = None
g_output_lock = threading.Lock()

GS_CHILD_STDIN_PATH = "/tmp/fred.child.stdin"
GS_CHILD_STDOUT_PATH = "/tmp/fred.child.stdout"
gf_child_stdin = None
gf_child_stdout = None
g_child_p = None

class ThreadedOutput(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)            
        self.should_exit = False

    def run(self):
        global gb_prompt_ready, g_output_lock
        while 1:
            g_output_lock.acquire()
            if self.should_exit:
                g_output_lock.release()
                break
            output = get_child_output()
            if output != None:
                sys.stdout.write(output)
                sys.stdout.flush()
                # Always keep this up-to-date:
                gb_prompt_ready = output.endswith(gs_prompt)
            g_output_lock.release()
    
def stop_output_thread():
    global g_output_thread, gf_child
    gf_child.flush()
    g_output_thread.should_exit = True
    g_output_thread.join()

def start_output_thread():
    global g_output_thread
    g_output_thread = ThreadedOutput()
    g_output_thread.daemon = True
    g_output_thread.start()

def send_child_input(input):
    global gf_child, GB_USE_SUBPROCESS, gf_child_stdin
    if GB_USE_SUBPROCESS:
        g_child_stdin.write(input)
    else:
        gf_child.write(input)
        gf_child.flush()
        
def get_child_output():
    global gf_child, GB_USE_SUBPROCESS, gf_child_stdout
    if GB_USE_SUBPROCESS:
        output = gf_child_stdout.read(100)
        return output
    else:
        n = 1000
        try:
            output = gf_child.read(n)
        except:
            return None
        return output

def wait_for_prompt():
    global gb_prompt_ready
    while not gb_prompt_ready:
        pass
    # Reset for next time
    gb_prompt_ready = False

def input_loop():
    wait_for_prompt()
    res = get_child_response("bt\n")
    sys.stdout.write("$"+res+"$")
    sys.stdout.flush()
    while 1:
        wait_for_prompt()
        s = get_input()
        send_child_input(s+'\n')

def get_input():
    s = raw_input()
    s = s.strip()
    return s

def get_child_response(input):
    ''' Sends requested input to child, and returns any response made. '''
    global g_output_lock
    g_output_lock.acquire()
    send_child_input(input)
    response = ''
    output_chunk = get_child_output()
    while output_chunk != None:
        response += output_chunk
        output_chunk = get_child_output()
    g_output_lock.release()
    return response

def fred_completer(text, state):
    current_cmd = readline.get_line_buffer()
    sys.stdout.write('\b'*len(current_cmd))
    # Write partial command+\t to gdb so it can do the completion.
    result = get_child_response(current_cmd + '\t')
    result = result.replace(current_cmd, "")
    readline.insert_text(result)

def spawn_child(argv):
    ''' Spawns a child process using the given command array. '''
    global gn_child_pid, gf_child, GB_USE_SUBPROCESS, g_child_p
    if GB_USE_SUBPROCESS:
        g_child_p = subprocess.Popen(argv, stdin=gf_child_stdin,
                                     stdout=gf_child_stdout,
                                     stderr=gf_child_stdout, close_fds=True)
    else:
        (gn_child_pid, child_fd) = pty.fork()
        if gn_child_pid == 0:
            os.execvp(argv[0], argv)
        else:
            gf_child = os.fdopen(child_fd, "wr", 0)
            fcntl.fcntl(child_fd, fcntl.F_SETFL, os.O_NONBLOCK)

def main():
    ''' Program execution starts here. '''
    global gf_child_stdin, gf_child_stdout
    try:
        os.unlink(GS_CHILD_STDIN_PATH)
        os.unlink(GS_CHILD_STDOUT_PATH)
    except OSError:
        pass
    gf_child_stdout = os.mkfifo(GS_CHILD_STDOUT_PATH)
    gf_child_stdin = os.mkfifo(GS_CHILD_STDIN_PATH)
    # Enable tab completion (with our own 'completer' function)
    readline.parse_and_bind('tab: complete')
    readline.set_completer(fred_completer)
    cmd = ["gdb"]
    spawn_child(cmd)
    start_output_thread()
    input_loop()
    
if __name__ == '__main__':
    main()
