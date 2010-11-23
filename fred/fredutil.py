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

import re
import sys

# Legal characters for a file path
gs_file_path_re = "[/a-zA-Z0-9_\-\.]+"

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

def fred_error(message):
    """Print an error message to the screen with a fred-specific prefix."""
    sys.stdout.write("FReD: %s\n" % message)
    sys.stdout.flush()

def getRE(str, idx=0):
    """Return a regular expression string matching the given string with a
    specified number of characters required.
    Examples:
    getRE("testing") => "^t(e(s(t(i(n(g)?)?)?)?)?)?\b"
    getRE("testing", 4) => "^testi(n(g)?)?\b"
    """
    # TODO: The second example above makes it seem like the requirement
    # number isn't properly respected.
    if len(str) < 1:
        return str
    return '^' + str[:idx] + getREWork(str[idx:]) + r'\b'

def getREWork(str):
    """Helper function for getRE(): wraps the string in recursive (x(...)?)?
    layers."""
    if len(str) < 1:
        return str
    if len(str) == 1:
        return str
    return str[0] + '(' + getREWork(str[1:]) + ')?'
