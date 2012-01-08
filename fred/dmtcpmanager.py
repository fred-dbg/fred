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
import fredmanager

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
    return gn_index_suffix

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

def remove_stale_ptrace_files():
    """Until DMTCP/ptrace cleans up its own files, we must clean up
    stale files explicitly."""
    ls_ptrace_files_to_remove = [ "ptrace_shared.txt",
                                  "ptrace_setoptions.txt",
                                  "ptrace_ckpthreads.txt",
                                  "new_ptrace_shared.txt",
                                  "ckpt_leader_file.txt" ]
    for f in ls_ptrace_files_to_remove:
        s_path = os.path.join(os.environ["DMTCP_TMPDIR"], f)
        if os.path.exists(s_path):
            os.remove(s_path)

def create_master_branch(s_name):
    """Create the master branch."""
    fredutil.fred_assert(not branch_exists(s_name))
    # Copy current tmpdir to correct path.
    relocate_dmtcp_tmpdir(s_name)
    load_dmtcp_tmpdir(s_name)
    
def create_branch(s_name):
    """Create and switch to a new branch. This call is blocking."""
    global gn_index_suffix
    
    if branch_exists(s_name):
        fredutil.fred_error("Branch named '%s' already exists.")
        return    
    fredutil.fred_debug("Creating branch '%s'" % s_name)

    # CREATE the branch: Take the branch base checkpoint. When it
    # finishes, record/replay will have already reopened the log
    # files, or created new ones.
    n_branched_index = gn_index_suffix
    checkpoint()

    # Copy the whole dmtcp_tmpdir to the new branch location and
    # symlink to the new location.
    relocate_dmtcp_tmpdir(s_name)
    load_dmtcp_tmpdir(s_name)

    # SWITCH to the branch: Remove all checkpoint images in the new
    # branch except for the most recently created one (the branch base
    # checkpoint). Rename that base checkpoint to index 0.
    remove_checkpoints_except_index(n_branched_index)
    rename_index_to_base(n_branched_index)

    # Reset checkpoint indexing past the base checkpoint.
    reset_checkpoint_indexing()
    
    # Restart from the base checkpoint. This is required so the logs
    # will be closed and reopened in the new branch.
    restart(0)

def switch_branch(s_name):
    """Switch to the specified branch."""
    if not branch_exists(s_name):
        fredutil.fred_error("Branch '%s' does not exist." % s_name)
    # Kill peers: force log flush.
    kill_peers()

    # Symlink dmtcp_tmpdir to branch tmpdir.
    load_dmtcp_tmpdir(s_name)

    reset_checkpoint_indexing()
    
    # Restart from branch base checkpoint
    restart(0)

def branch_exists(s_name):
    """Return True if the given branch name exists on disk."""
    return os.path.exists(get_dmtcp_tmpdir_path(s_name))

def checkpoint():
    """Perform a blocking checkpoint request and rename the checkpoint files."""
    global gn_index_suffix

    remove_stale_ptrace_files()
    
    s_checkpoint_re = "ckpt_.+\.dmtcp\..*"
    l_ckpts_before = [os.path.join(os.environ["DMTCP_TMPDIR"], x) \
                      for x in os.listdir(os.environ["DMTCP_TMPDIR"]) \
                      if re.search(s_checkpoint_re, x) != None]
    #fredutil.fred_debug("List ckpts before: %s" % str(l_ckpts_before))
    # Request the checkpoint.
    n_peers = get_num_peers()
    cmdstr = ["dmtcp_command", "--quiet", "bc"]
    fredutil.execute_shell_command_and_wait(cmdstr)
    fredutil.fred_debug("After blocking checkpoint command.")

    l_new_ckpts = []
    # There is what seems to be a DMTCP bug: the blocking checkpoint
    # can actually return before the checkpoints are written. It is
    # rare.
    while len(l_new_ckpts) < n_peers:
        l_ckpts_after = [os.path.join(os.environ["DMTCP_TMPDIR"], x) \
                         for x in os.listdir(os.environ["DMTCP_TMPDIR"]) \
                         if x.startswith("ckpt_") and x.endswith("dmtcp")]
        #fredutil.fred_debug("List ckpts after: %s" % str(l_ckpts_after))
        l_new_ckpts = [x for x in l_ckpts_after if x not in l_ckpts_before]
        time.sleep(0.001)
    for f in l_new_ckpts:
        fredutil.fred_debug("Renaming ckpt file from '%s' to '%s.%d'" %
                            (f, f, gn_index_suffix))
        os.rename(f, "%s.%d" % (f, gn_index_suffix))
    gn_index_suffix += 1

def restart(n_index):
    """Restart from the given index."""
    kill_peers()
    fredio.kill_child()
    fredmanager.kill_inferior()
    
    # Wait until the peers are really gone
    while get_num_peers() != 0:
        time.sleep(0.01)

    remove_stale_ptrace_files()
    
    l_ckpt_files = [os.path.join(os.environ["DMTCP_TMPDIR"], x) \
                    for x in os.listdir(os.environ["DMTCP_TMPDIR"]) \
                    if x.endswith(".dmtcp.%d" % n_index)]
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

    # Due to what is arguably a bug in DMTCP, checkpoint files must
    # end in "*.dmtcp" in order for DMTCP to restart from them. So we
    # symlink to conform to that pattern before restarting.
    l_symlinks = []
    for f in l_ckpt_files:
        s_new_path = os.path.join(os.environ["DMTCP_TMPDIR"],
                                  re.search("(ckpt_.*\.dmtcp)\..*", f).group(1))
        if os.path.exists(s_new_path):
            os.remove(s_new_path)
        os.symlink(f, s_new_path)
        l_symlinks.append(s_new_path)
    fredutil.fred_debug("Restarting checkpoint files: %s" % str(l_symlinks))
    cmdstr = ["dmtcp_restart"]
    map(cmdstr.append, l_symlinks)
    fredio.reexec(cmdstr)
    # Wait until every peer has finished resuming:
    while get_num_peers() < len(l_symlinks) or not is_running():
        time.sleep(0.01)

def get_dmtcp_tmpdir_path(s_name):
    """Return the full path for DMTCP_TMPDIR with suffix s_name."""
    return "%s-%s" % (os.environ["DMTCP_TMPDIR"], s_name)

def relocate_dmtcp_tmpdir(s_name):
    """Copy the current DMTCP_TMPDIR to a new location with suffix s_name."""
    if not os.path.islink(os.environ["DMTCP_TMPDIR"]):
        # When creating the master branch, the tmpdir is a directory not a link
        # Just move it and return.
        fredutil.fred_debug("DMTCP_TMPDIR is a directory, not a link.")
        s_new_path = get_dmtcp_tmpdir_path(s_name)
        os.rename(os.environ["DMTCP_TMPDIR"], s_new_path)
        return
    
    s_current_path = os.path.normpath(os.readlink(os.environ["DMTCP_TMPDIR"]))
    if not os.path.isabs(s_current_path):
        # Resolve relative to absolute path.
        s_current_path = os.path.join(os.path.dirname(os.environ["DMTCP_TMPDIR"]),
                                      s_current_path)
    s_new_path = get_dmtcp_tmpdir_path(s_name)
    if os.path.exists(s_new_path):
        fredutil.fred_error("Requested new path '%s' already exists." %
                            s_new_path)
        return
    shutil.copytree(s_current_path, s_new_path)
    fredutil.fred_debug("Copied DMTCP_TMPDIR from '%s' to '%s'." %
                        (s_current_path, s_new_path))

def load_dmtcp_tmpdir(s_name):
    """Change the DMTCP_TMPDIR symlink to point at the given tmpdir name."""
    if os.path.exists(os.environ["DMTCP_TMPDIR"]):
        os.remove(os.environ["DMTCP_TMPDIR"])
    s_path = get_dmtcp_tmpdir_path(s_name)
    os.symlink(s_path, os.environ["DMTCP_TMPDIR"])
    fredutil.fred_debug("Symlinked DMTCP_TMPDIR to: %s" % s_path)

def remove_checkpoints_except_index(n_index):
    """Remove all checkpoint images in the current DMTCP_TMPDIR except
    the specified index."""
    s_path = os.environ["DMTCP_TMPDIR"]
    s_checkpoint_re = "ckpt_.+\.dmtcp.*"
    l_files = [os.path.join(os.environ["DMTCP_TMPDIR"], x) \
               for x in os.listdir(os.environ["DMTCP_TMPDIR"]) \
               if re.search(s_checkpoint_re, x) != None and \
                  not x.endswith(".%d" % n_index)]
    fredutil.fred_debug("Removing files: %s" % str(l_files))
    map(os.remove, l_files)

def rename_index_to_base(n_index):
    """Rename all checkpoint images of the given index to index 0 ("*.0")."""
    l_files = [os.path.join(os.environ["DMTCP_TMPDIR"], x) \
               for x in os.listdir(os.environ["DMTCP_TMPDIR"]) \
               if x.endswith(".%d" % n_index)]
    s_checkpoint_re = "(ckpt_.+\.dmtcp)\..*"
    for f in l_files:
        s_new_name = os.path.join(os.environ["DMTCP_TMPDIR"],
                                  "%s.0" % re.search(s_checkpoint_re,
                                                     f).group(1))
        fredutil.fred_debug("Renaming ckpt %s to base ckpt %s." %
                            (f, s_new_name))
        os.rename(f, s_new_name)

def reset_checkpoint_indexing():
    """Set gn_index_suffix to the appropriate value based on existent
    checkpoint files."""
    global gn_index_suffix
    s_checkpoint_re = "ckpt_.+\.dmtcp\.(.*)"
    l_files = [os.path.join(os.environ["DMTCP_TMPDIR"], x) \
               for x in os.listdir(os.environ["DMTCP_TMPDIR"]) \
               if re.search(s_checkpoint_re, x) != None]
    if len(l_files) == 0:
        gn_index_suffix = 0
    else:
        n_max_index = max([int(re.search(s_checkpoint_re, x).group(1)) \
                           for x in l_files])
        gn_index_suffix = n_max_index + 1
    fredutil.fred_debug("Reset ckpt index to %d" % gn_index_suffix)
    
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
                    os.environ["DMTCP_TMPDIR"] + "-MASTER")
    os.symlink(os.environ["DMTCP_TMPDIR"] + "-MASTER",
               os.environ["DMTCP_TMPDIR"])
    fredutil.fred_info("Resuming session.")
    reset_checkpoint_indexing()
    restart(0)
    

def manager_teardown():
    global gn_index_suffix
    gn_index_suffix = 0
