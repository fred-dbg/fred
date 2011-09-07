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

import pdb
import os
import re
import sys
import time

import fredio
import dmtcpmanager

# Legal characters for a file path
GS_FILE_PATH_RE = "[/a-zA-Z0-9_\-\.]+"
GB_DEBUG = False
# Set to true to enable timings display.
# Internal use only -- no command line switch.
GB_ENABLE_TIMINGS = True
gd_timers = {}

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

def fred_info(message):
    """Print an info message to the screen with a fred-specific prefix."""
    # TODO: This is hackish. Needed for testing framework to hide info messages
    # during integration tests.
    if not fredio.gb_hide_output:
        sys.stdout.write("FReD: %s\n" % message)
        sys.stdout.flush()

def fred_error(message):
    """Print an error message to the screen with a fred-specific prefix."""
    sys.stderr.write("FReD error: %s\n" % message)
    sys.stderr.flush()

def fred_warning(message):
    """Print a warning message to the screen with a fred-specific prefix."""
    sys.stderr.write("FReD warning: %s\n" % message)
    sys.stderr.flush()

def fred_fatal(message):
    """Kill FReD with given message."""
    sys.stderr.write("FReD fatal error: %s\n" % message)
    sys.stderr.flush()
    fred_quit(1)

def fred_debug(msg):
    """Print the given debug message."""
    global GB_DEBUG
    if GB_DEBUG:
        caller_name = os.path.basename(sys._getframe(1).f_code.co_filename)
        caller_lineno = sys._getframe(1).f_lineno
        # Prints name of file and line number of caller to fred_debug()
        print "[fred-debug] %s:%d - %s" % (caller_name, caller_lineno, msg)

def fred_assert(b_expr, msg="Assertion failed."):
    """Assert that b_expr is True. If not, print message and dump into pdb."""
    global GB_DEBUG
    if not b_expr:
        caller_name = os.path.basename(sys._getframe(1).f_code.co_filename)
        caller_lineno = sys._getframe(1).f_lineno
        fred_error(msg)
        fred_error("Assertion failed at %s:%d." % (caller_name, caller_lineno))
        if GB_DEBUG:
            pdb.set_trace()

def fred_timer_start(s_timer_name):
    """Start a timer with the given name."""
    global GB_ENABLE_TIMINGS, gd_timers
    if GB_ENABLE_TIMINGS:
	gd_timers[s_timer_name] = time.time()
        fred_debug("Starting timer '%s'." % s_timer_name)

def fred_timer_stop(s_timer_name):
    """Stop timer with the given name and report duration."""
    global GB_ENABLE_TIMINGS, gd_timers
    if GB_ENABLE_TIMINGS:
	n_stop_time = time.time()
        fred_info("'%s' took %.3f seconds." % \
		  (s_timer_name, n_stop_time - gd_timers[s_timer_name]))
        del gd_timers[s_timer_name]

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

def to_int(str, n_default=0):
    """Return the integer representation of the given string, or 'default'."""
    try:
        retval = int(str)
        return retval
    except ValueError:
        return n_default

def open_file(s_filename):
    """Open given filename and return file instance, or None on error."""
    f = None
    try:
        f = open(s_filename)
    except IOError as (errno, strerror):
        fred_error("Error opening source file '%s': %s" % \
                       (s_filename, strerror))
    return f

def fred_teardown():
    fred_debug("FReD exiting.")
    fredio.teardown()
    dmtcpmanager.manager_teardown()
    
def fred_quit(exit_code):
    """Perform any necessary cleanup and quits FReD."""
    fred_teardown()
    sys.exit(exit_code)

def set_env_var_if_unset(s_name, s_val):
    """Set the given environment variable if it is currently unset."""
    try:
        fred_debug("Trying to set env var '%s' to value '%s'." % \
                       (s_name, s_val))
        s_cur_val = os.environ[s_name]
        if len(s_cur_val) == 0:
            os.environ[s_name] = s_val
            return
    except KeyError:
        os.environ[s_name] = s_val
        return

def get_fredhijack_path():
    """Return the path to fredhijack.so."""
    return dmtcpmanager.get_fredhijack_path()
