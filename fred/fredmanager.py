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

import fredutil
import dmtcpmanager

import os
import re
import signal

GS_FREDHIJACK_NAME = "fredhijack.so"
GS_FREDHIJACK_PATH = ""

g_child_subprocess = None
gn_pid = -1
gn_initial_inf_pid = -1

def set_inferior_pid(n_pid):
    """Set the pid of the inferior process."""
    global gn_pid
    fredutil.fred_debug("Setting inferior pid to %d." % n_pid)
    gn_pid = n_pid

def get_inferior_pid():
    """Return the pid of the inferior process."""
    global gn_pid
    return gn_pid

def set_initial_inferior_pid(n_pid):
    """Set the initial (i.e. virtualized) pid of the inferior."""
    global gn_initial_inf_pid
    fredutil.fred_debug("Setting initial inferior pid to %d." % n_pid)
    gn_initial_inf_pid = n_pid

def get_initial_inferior_pid():
    """Return the initial (i.e. virtualized) pid of the inferior."""
    global gn_initial_inf_pid
    return gn_initial_inf_pid
    
def kill_inferior():
    """Kill the inferior process, if it exists."""
    n_pid = get_inferior_pid()
    if n_pid == -1:
        return
    try:
        os.kill(n_pid, signal.SIGKILL)
        os.waitpid(n_pid, 0)
    except OSError:
        pass

def is_fredhijack_found():
    """Return True if fredhijack.so library is in a known location."""
    return os.path.exists(get_fredhijack_path())

def get_fredhijack_path():
    """Return the path to fredhijack.so."""
    global GS_FREDHIJACK_PATH, GS_FREDHIJACK_NAME
    return os.path.join(GS_FREDHIJACK_PATH, GS_FREDHIJACK_NAME)

def set_fredhijack_path(s_path):
    """Sets the path to fredhijack.so."""
    global GS_FREDHIJACK_PATH
    GS_FREDHIJACK_PATH = s_path
    
def _execute_fred_command(s_cmd, s_arg=None):
    """Execute the given fred_command command and return its output."""
    global g_child_subprocess
    fredutil.fred_assert(s_cmd in ["status", "info", "break", "continue"])
    l_cmd = ["%s/fred_command" % GS_FREDHIJACK_PATH]
    fredutil.fred_assert(get_initial_inferior_pid() != -1)
    s_path = "%s/fred-shm.%d" % (os.environ["DMTCP_TMPDIR"],
                                 get_initial_inferior_pid())
    l_cmd.append("--%s" % s_cmd)
    if s_arg != None:
        l_cmd.append(s_arg)
    l_cmd.append(s_path)
    fredutil.fred_debug("Executing fred_command: %s" % l_cmd)
    if s_cmd == "break":
        fredutil.fred_assert(g_child_subprocess == None)
        g_child_subprocess = fredutil.execute_background_shell_command(l_cmd)
        s_output = ""
    else:
        s_output = fredutil.execute_shell_command(l_cmd)
    return s_output

def destroy():
    """Perform any cleanup associated with the fred manager."""
    g_child_subprocess = None
    set_inferior_pid(-1)
    set_initial_inferior_pid(-1)

def set_fred_breakpoint(n_index):
    """Set a FReD internal breakpoint on entry index n_index."""
    _execute_fred_command("break", str(n_index))

def wait_on_fred_breakpoint():
    """Blocking wait until a FReD internal breakpoint is hit."""
    global g_child_subprocess
    fredutil.fred_assert(g_child_subprocess != None)
    g_child_subprocess.communicate()
    g_child_subprocess = None

def send_fred_continue():
    """Send FReD internal continue command."""
    _execute_fred_command("continue")

def get_current_thread():
    """Return the clone id of the current entry's thread."""
    s_clone_id_re = "Current clone id = (\d+)"
    s_output = _execute_fred_command("status")
    m = re.search(s_clone_id_re, s_output)
    if m != None:
        fredutil.fred_debug("Current clone id is: %d" % int(m.group(1)))
        return int(m.group(1))
    else:
        return None

def get_current_entry_index():
    """Return the index of the current entry."""
    s_index_re = "Current entry index = (\d+)"
    s_output = _execute_fred_command("status")
    m = re.search(s_index_re, s_output)
    if m != None:
        fredutil.fred_debug("Current entry index is: %d" % int(m.group(1)))
        return int(m.group(1))
    else:
        return None

def get_total_entries():
    """Return the total number of log entries."""
    s_total_entries_re = "Total number of log entries = (\d+)"
    s_output = _execute_fred_command("info")
    m = re.search(s_total_entries_re, s_output)
    if m != None:
        fredutil.fred_debug("Total entries are: %d" % int(m.group(1)))
        return int(m.group(1))
    else:
        return None

def get_total_threads():
    """Return the total number of log threads."""
    s_total_threads_re = "Total number of threads = (\d+)"
    s_output = _execute_fred_command("info")
    m = re.search(s_total_threads_re, s_output)
    if m != None:
        fredutil.fred_debug("Total threads are: %d" % int(m.group(1)))
        return int(m.group(1))
    else:
        return None

def current_fred_state():
    """Return a FredState instance representing the current FReD state."""
    state = FredState()
    state.set_total_entries(get_total_entries())
    state.set_total_threads(get_total_threads())
    state.set_current_entry(get_current_entry_index())
    state.set_current_thread(get_current_thread())
    return state

class FredState:
    def __init__(self):
        self.n_total_entries = -1
        self.n_total_threads = -1
        self.n_current_entry = -1
        self.n_current_thread = -1

    def total_entries(self):
        return self.n_total_entries

    def total_threads(self):
        return self.n_total_threads

    def current_entry(self):
        return self.n_current_entry

    def current_thread(self):
        return self.n_current_thread

    def set_total_entries(self, n_entries):
        self.n_total_entries = n_entries

    def set_total_threads(self, n_threads):
        self.n_total_threads = n_threads

    def set_current_entry(self, n_entry):
        self.n_current_entry = n_entry

    def set_current_thread(self, n_clone_id):
        self.n_current_thread = n_clone_id
