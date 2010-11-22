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


import fredio

def special_command(command):
    """Return True if the given command needs special handling."""
    return command.startswith("fred-")

def main():
    """Program execution starts here."""
    cmd = ["gdb"]
    fredio.setup(cmd)
    fredio.start_output_thread()
    # Enter main input loop:
    while 1:
        fredio.wait_for_prompt()
        s = fredio.get_command()
        if special_command(s):
            print "Special command."
        else:
            fredio.send_command(s)

    
if __name__ == '__main__':
    main()
