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
'''

gn_child_pid = -1
gn_child_fd = -1
gs_prompt = "(gdb) "
gb_prompt_ready = False

def output_loop():
    class ThreadedOutput(threading.Thread):
        def run(self):
            global gb_prompt_ready
            while 1:
                output = get_child_output(1000)
                if output != None:
                    sys.stdout.write(output)
                    sys.stdout.flush()
                    gb_prompt_ready = output.endswith(gs_prompt)
    o = ThreadedOutput()
    o.daemon = True
    o.start()

def send_child_input(c):
    global gn_child_fd
    os.write(gn_child_fd, c)

def get_child_output(n):
    global gn_child_fd
    try:
        output = os.read(gn_child_fd, n)
    except:
        return None
    return output

def wait_for_prompt():
    global gb_prompt_ready
    while not gb_prompt_ready:
        pass
    gb_prompt_ready = False

def input_loop():
    while 1:
        wait_for_prompt()
        s = get_input()
        send_child_input(s+'\n')

def get_input():
    s = raw_input()
    s = s.strip()
    readline.insert_text('\b'*len(s))
    readline.redisplay()
    return s

def fred_completer(text, state):
    current_cmd = readline.get_line_buffer()
    sys.stdout.write('\b'*len(current_cmd))
    # Write partial command+\t to gdb so it can do the completion.
    os.write(child_fd, current_cmd)
    os.write(child_fd, '\t')

def spawn_child(argv):
    ''' Spawns a child process using the given command array. '''
    global gn_child_pid, gn_child_fd
    (gn_child_pid, gn_child_fd) = pty.fork()
    if gn_child_pid == 0:
        os.execvp(argv[0], argv)
    else:
        fcntl.fcntl(gn_child_fd, fcntl.F_SETFL, os.O_NONBLOCK)

def main():
    ''' Program execution starts here. '''
    # Enable tab completion (with our own 'completer' function)
    readline.parse_and_bind('tab: complete')
    readline.set_completer(fred_completer)
    cmd = ["gdb"]
    spawn_child(cmd)
    output_loop()
    input_loop()
    
if __name__ == '__main__':
    main()
