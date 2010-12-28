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

from optparse import OptionParser
import os
import signal
import sys

from fred import fredio
from fred import freddebugger
from fred import fredutil

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
-----------+----------------------
g?_        |   global
-----------+----------------------
b_         |   boolean
d_         |   dictionary
f_         |   Python file object
l_         |   list
n_         |   numeric
s_         |   string

The ? after g_ is used to indicate the presence of an additional type
indicator.  For example a global integer should be name like:
  gn_a_global_number
To indicate global scope, and numeric type.

Whereever possible, local variables should also use a type prefix. Local scope
does not have or need a specific prefix.

All other non-basic types do not require a prefix other than scope-related.
For example, a mutex or thread object does not have a specific prefix.
However, the name of the variable should be descriptive enough to tell what
type it is meant to be.

Please make it a goal to document every putlic function with a docstring. It
only takes a few seconds, and saves a lot of time for future programmers. For
docstring conventions, you can see PEP-257.
'''


######################## Global Constants #####################################
GS_FRED_VERSION="Version: %prog 0.99-r277 (Nov. 12 2010)"
GS_FRED_USAGE="USAGE: %prog [options] xdb [ARGS] a.out [A.OUT ARGS]\n" + \
               "Replace `xdb' with the name of the target debugger"
GS_FRED_COMMAND_PREFIX="fred-"
######################## End Global Constants #################################

######################## Global Variables #####################################
# The global ReversibleDebugger instance.
g_debugger = None
######################## End Global Variables #################################

def fred_command_help():
   print """FReD commands:
(all optional 'count' arguments default to 1)
  fred-undo [n]:         Undo last n debugger commands.
  fred-reverse-next [n], fred-rn [n]:  Reverse-next n times.
  fred-reverse-step [n], fred-rs [n]:  Reverse-step n times.
  fred-checkpoint, fred-ckpt: Request a new checkpoint to be made.
  fred-restart:               Restart from last checkpoint.
  fred-reverse-watch <EXPR>, fred-rw <EXPR>:
                              Reverse execute until expression EXPR changes.
  fred-source <FILE>:         Read commands from source file.
  fred-list:                  List the available checkpoint files.
  fred-help:                  Display this help message.
  fred-history:               Display your command history up to this point.
"""
   sys.stdout.flush()

def is_quit_command(s_command):
    """Return True if s_command is a debugger 'quit' command."""
    return s_command in ["q", "quit"]

def handle_fred_command(s_command):
    """Performs handling of 'special' (non-debugger) commands."""
    global g_debugger, GS_FRED_COMMAND_PREFIX
    s_command = s_command.replace(GS_FRED_COMMAND_PREFIX, "")
    (s_command_name, sep, s_command_args) = s_command.partition(' ')
    n_count = fredutil.to_int(s_command_args)
    if is_quit_command(s_command_name):
        fred_quit(0)
    elif s_command_name == "undo":
        g_debugger.undo(n_count)
    elif s_command_name in ["reverse-next", "rn"]:
        g_debugger.reverse_next(n_count)
    elif s_command_name in ["reverse-step", "rs"]:
        g_debugger.reverse_step(n_count)
    elif s_command_name in ["checkpoint", "ckpt"]:
        fredutil.fred_error("Checkpoint is unimplemented.")
    elif s_command_name == "restart":
        fredutil.fred_error("Restart is unimplemented.")
    elif s_command_name in ["reverse-watch", "rw"]:
        g_debugger.reverse_watch(s_command_args)
    elif s_command_name == "source":
        g_debugger.source(s_command_args)
    elif s_command_name == "list":
        print g_debugger.list_checkpoints()
    elif s_command_name == "help":
        fred_command_help()
    elif s_command_name == "history":
        print g_debugger.history()
    else:
        fredutil.fred_error("Unhandled FReD command '%s'" % s_command_name)

def is_fred_command(s_command):
    """Return True if the given command needs special handling."""
    global GS_FRED_COMMAND_PREFIX
    return s_command.startswith(GS_FRED_COMMAND_PREFIX) or \
           is_quit_command(s_command)

def set_up_debugger(s_debugger_name):
    """Initialize global ReversibleDebugger instance g_debugger.
    Return the personality's find prompt function to pass to fredio."""
    global g_debugger
    if s_debugger_name == "gdb":
        from fred.personalityGdb import PersonalityGdb
        g_debugger = freddebugger.ReversibleDebugger(PersonalityGdb())
        del PersonalityGdb
    else:
        fredutil.fred_error("Unimplemented debugger '%s'" % s_debugger_name)
        fred_quit(1)
    return g_debugger.get_prompt_str_function()   

def parse_program_args():
    """Initialize command line options, and parse them.
    Return the user's inferior to execute as a list."""
    global GS_FRED_USAGE
    parser = OptionParser(usage=GS_FRED_USAGE, version=GS_FRED_VERSION)
    parser.disable_interspersed_args()
    # Note that '-h' and '--help' are supported automatically.
    parser.add_option("-p", "--port", dest="dmtcp_port", default=7779,
                      help="Use PORT for DMTCP port number. (default %default)",
                      metavar="PORT")
    parser.add_option("-x", "--source", dest="source_script",
                      help="Execute batch file FILE", metavar="FILE")
    (options, l_args) = parser.parse_args()
    # 'l_args' is the 'gdb ARGS ./a.out' list
    if len(l_args) == 0:
        parser.print_help()
        fred_quit(1)
    if options.source_script != None:
        assert False, "Sourcing from file unimplemented."
    os.environ['DMTCP_PORT'] = str(options.dmtcp_port)
    return l_args

def main():
    """Program execution starts here."""
    global g_debugger
    # Parse arguments
    l_cmd = parse_program_args()
    # Set up the FReD global debugger
    find_prompt_fnc = set_up_debugger(l_cmd[0])
    # Set up I/O handling (with appropriate find_prompt())
    fredio.setup(find_prompt_fnc, l_cmd)

    wait_for_prompt = True
    # Enter main input loop:
    while 1:
        try:
            if wait_for_prompt:
                fredio.wait_for_prompt()
            else:
                # Reset
                wait_for_prompt = True
            # Get one user command (blocking):
            s_command = fredio.get_command()
            if is_fred_command(s_command):
                handle_fred_command(s_command)
                # Skip wait_for_prompt() next iteration:
                wait_for_prompt = False
                g_debugger.prompt()
            else:
                fredio.send_command(s_command)
                g_debugger.log_command(s_command)
        except KeyboardInterrupt:
            fredio.signal(signal.SIGINT)
    fred_quit(0)

def fred_quit(exit_code):
    """Performs any necessary cleanup and quits FReD."""
    exit(exit_code)
    
if __name__ == '__main__':
    main()
