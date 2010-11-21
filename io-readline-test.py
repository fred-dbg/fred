#!/usr/bin/python
import fcntl
import os
import pty
import readline
import sys
import threading

child_pid = -1
child_fd = -1
prompt_length = len("(gdb) ")

def output_loop():
    class ThreadedOutput(threading.Thread):
        def run(self):
            while 1:
                output = get_child_output(1000)
                if output != None:
                    sys.stdout.write(output)
                    sys.stdout.flush()
    o = ThreadedOutput()
    o.daemon = True
    o.start()

def send_child_input(c):
    global child_fd
    os.write(child_fd, c)

def get_child_output(n):
    global child_fd
    try:
        output = os.read(child_fd, n)
    except:
        return None
    return output

def input_loop():
    while 1:
        s = get_input()
        send_child_input(s+'\n')

def get_input():
    padding = ' '*prompt_length
    s = raw_input(padding)
    s = s.strip()
    return s

def fred_completer(text, state):
    current_cmd = readline.get_line_buffer()
    sys.stdout.write('\b'*len(current_cmd))
    # Write partial command+\t to gdb so it can do the completion.
    os.write(child_fd, current_cmd)
    os.write(child_fd, '\t')

def spawn_child(argv):
    ''' Spawns a child process using the given command array. '''
    global child_pid, child_fd
    (child_pid, child_fd) = pty.fork()
    if child_pid == 0:
        os.execvp(argv[0], argv)
    else:
        fcntl.fcntl(child_fd, fcntl.F_SETFL, os.O_NONBLOCK)

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
