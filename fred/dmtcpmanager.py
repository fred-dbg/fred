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
import fredutil

import os
import pdb
import re
import shutil
import subprocess
import sys
import time

gn_index_suffix = 0

def is_dmtcp_in_path():
    """Check to see if DMTCP binaries are in the user's path."""
    for path in os.environ['PATH'].split(os.pathsep):
        exe_file = os.path.join(path, 'dmtcp_command')
        if os.path.exists(exe_file) and os.access(exe_file, os.X_OK):
            return True
    return False

def start_coordinator(n_port):
    """Start a coordinator on given port. Return False on error."""
    status = fredutil.execute_shell_command_and_wait(["dmtcp_coordinator",
                                                      "--background",
                                                      "-p", str(n_port)])
    return status == 0
    
def kill_coordinator(n_port):
    """Kills the coordinator on given port."""
    try:
        fredutil.execute_shell_command_and_wait(["dmtcp_command",
                                                 "--quiet", "-p",
                                                 str(n_port), "q"])
    except subprocess.CalledProcessError:
        pass
    except:
        raise

def get_num_checkpoints():
    """Return the number of checkpoints DMTCP manager has made."""
    global gn_index_suffix
    # Indexing starts from zero, so add one.
    return gn_index_suffix + 1

def get_num_peers():
    """Return NUM_PEERS from 'dmtcp_command s' as an integer."""
    cmd = [ 'dmtcp_command', 's' ]
    output = fredutil.execute_shell_command(cmd)
    if output != None:
        exp = '^NUM_PEERS=(\d+)'
        m = re.search(exp, output, re.MULTILINE)
        if m != None:
            n_peers = m.group(1)
            return int(n_peers)
        else:
            if output == "":
                fredutil.fred_error("Output was NULL string")
            return 0  # Heuristically guessing 0 peers, could be a problem
    else:
        fredutil.fred_error("ERROR: Can't get NUM_PEERS. "
                            "Did the coordinator die?")
        return 0

def is_running():
    """Return True if dmtcp_command reports RUNNING as 'yes'."""
    cmd = ["dmtcp_command", "s"]
    output = fredutil.execute_shell_command(cmd)
    if output != None:
        m = re.search('RUNNING=(\w+)', output, re.MULTILINE)
        if m != None:
            running = m.group(1)
            return running == 'yes'
        else:
            return False
    else:
        fredutil.fred_error("ERROR: Can't get RUNNING. "
                            "Did the coordinator die?")
        return False

def kill_peers():
    """Send 'k' command to coordinator."""
    cmd = ["dmtcp_command", "k"]
    fredutil.fred_debug("Sending command '%s'" % ' '.join(cmd))
    if fredio.GB_FRED_DEMO:
        print "===================== KILLING gdb ====================="
    pid = os.fork()
    if pid == 0:
        sys.stderr = sys.stdout
        os.execvp('dmtcp_command', ['dmtcp_command', '--quiet', 'k'])
    else:
        os.waitpid(pid, 0)

def checkpoint():
    """Perform a blocking checkpoint request."""
    global gn_index_suffix
    # Create an indexed temp directory (.X). If this is the first checkpoint,
    # move the existent directory instead of creating one.
    s_path = os.environ["DMTCP_TMPDIR"] + "." + str(gn_index_suffix)
    if gn_index_suffix == 0: # first checkpoint
        shutil.move(os.environ["DMTCP_TMPDIR"], s_path)
    else:
        fredutil.fred_debug("Creating new indexed temp directory: %s" % s_path)
        os.makedirs(s_path, 0755)
    gn_index_suffix += 1
    # Link the main DMTCP temp directory to that indexed one.
    if os.path.exists(os.environ["DMTCP_TMPDIR"]):
        os.remove(os.environ["DMTCP_TMPDIR"])
    os.symlink(s_path, os.environ["DMTCP_TMPDIR"])
    # Remove any checkpoint files or synchronization files from the tmpdir.
    # We should have only ONE set of ckpt files/sync files per tmpdir.
    l_files = [os.path.join(os.environ["DMTCP_TMPDIR"], x) \
               for x in os.listdir(os.environ["DMTCP_TMPDIR"]) \
               if x.endswith(".dmtcp") or x.startswith("synchronization-")]
    map(os.remove, l_files)
    # Request the checkpoint.
    cmdstr = ["dmtcp_command", "--quiet", "bc"]
    fredutil.execute_shell_command_and_wait(cmdstr)
    fredutil.fred_debug("After blocking checkpoint command.")

def restart(n_index):
    """Restart from the given index."""
    kill_peers()
    fredio.kill_child()
    # Wait until the peers are really gone
    while get_num_peers() != 0:
        time.sleep(0.01)
    # Remove the main temp link.
    os.remove(os.environ["DMTCP_TMPDIR"])
    # Re-link to the desired index.
    os.symlink(os.environ["DMTCP_TMPDIR"] + "." + str(n_index),
               os.environ["DMTCP_TMPDIR"])
    l_ckpt_files = [os.path.join(os.environ["DMTCP_TMPDIR"], x) \
                    for x in os.listdir(os.environ["DMTCP_TMPDIR"]) \
                    if x.endswith(".dmtcp")]
    if (len(l_ckpt_files) > 2):
        # XXX: I think this is a Python bug.... sometimes even when there are
        # physically only two checkpoint files on disk, l_ckpt_files will
        # contain 4, with the two unique ones being duplicated:
        # (Pdb) p l_ckpt_files
        #['/tmp/fred.tyler/dmtcp_tmpdir/ckpt_test_list_X-3089-4db5c59e.dmtcp',
        #'/tmp/fred.tyler/dmtcp_tmpdir/ckpt_gdb_X-3081-4db5c59c.dmtcp',
        #'/tmp/fred.tyler/dmtcp_tmpdir/ckpt_test_list_X-3089-4db5c59e.dmtcp',
        #'/tmp/fred.tyler/dmtcp_tmpdir/ckpt_gdb_X-3081-4db5c59c.dmtcp']
        # I have replaced the hostname with X for readability.
        l_ckpt_files = list(set(l_ckpt_files))
    fredutil.fred_debug("Restarting checkpoint files: %s" % str(l_ckpt_files))
    cmdstr = ["dmtcp_restart"]
    map(cmdstr.append, l_ckpt_files)
    fredio.reexec(cmdstr)
    # Wait until every peer has finished resuming:
    while get_num_peers() < len(l_ckpt_files) or not is_running():
        time.sleep(0.01)

def erase_checkpoints(n_begin, n_end):
    pass

def resume(s_fred_tmpdir, s_resume_dir):
    """Set up tmpdir structure from a given path."""
    global gn_index_suffix
    if not os.path.exists(s_resume_dir):
        fredutil.fred_fatal("Cannot resume: bad path %s." % s_resume_dir)
    fredutil.fred_info("Removing existing FReD temp dir at %s." %
                       s_fred_tmpdir)
    shutil.rmtree(s_fred_tmpdir, ignore_errors = True)
    fredutil.fred_info("Copying files for session resume. This could take a "
                       "few minutes if the checkpoint images are large.")
    shutil.copytree(s_resume_dir,
                    os.environ["DMTCP_TMPDIR"] + "." + str(gn_index_suffix))
    os.symlink(os.environ["DMTCP_TMPDIR"] + "." + str(gn_index_suffix),
               os.environ["DMTCP_TMPDIR"])
    fredutil.fred_info("Resuming session.")
    restart(gn_index_suffix)

def manager_teardown():
    global gn_index_suffix
    gn_index_suffix = 0
