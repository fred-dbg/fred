#!/usr/bin/python
from optparse import OptionParser
import os
import signal
import sys

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


import fredio
import freddebugger

GS_FRED_USAGE="USAGE: %prog [options] xdb [ARGS] a.out [A.OUT-ARGS]\n" + \
               "Replace `xdb' with the name of the target debugger"
GS_FRED_VERSION="Version: %prog 0.99-r277 (Nov. 12 2010)"

def is_quit_command(command):
    return command in ["q", "quit"]

def handle_special_command(command):
    """Performs handling of 'special' (non-debugger) commands."""
    if is_quit_command(command):
        fred_quit(0)

def is_special_command(command):
    """Return True if the given command needs special handling."""
    return command.startswith("fred-") or is_quit_command(command)

def parse_program_args():
    global GS_FRED_USAGE
    parser = OptionParser(usage=GS_FRED_USAGE, version=GS_FRED_VERSION)
    parser.disable_interspersed_args()
    # Note that '-h' and '--help' are supported automatically.
    parser.add_option("-p", "--port", dest="dmtcp_port", default=7779,
                      help="Use PORT for DMTCP port number. (default %default)",
                      metavar="PORT")
    parser.add_option("-x", "--source", dest="source_script",
                      help="Execute batch file FILE", metavar="FILE")
    (options, args) = parser.parse_args()
    if options.source_script != None:
        print "Unimplemented."
    os.environ['DMTCP_PORT'] = str(options.dmtcp_port)
    # args is the 'gdb ARGS ./a.out' list
    if len(args) == 0:
        parser.print_help()
        fred_quit(1)
    return args

def main():
    """Program execution starts here."""
    cmd = parse_program_args()
    fredio.setup(cmd)
    # Enter main input loop:
    while 1:
        try:
            fredio.wait_for_prompt()
            s = fredio.get_command()
            if is_special_command(s):
                handle_special_command(s)
            else:
                fredio.send_command(s)
        except KeyboardInterrupt:
            fredio.signal(signal.SIGINT)
    fred_quit(0)

def fred_quit(exit_code):
    """Performs any necessary cleanup and quits FReD."""
    exit(exit_code)
    
if __name__ == '__main__':
    main()
