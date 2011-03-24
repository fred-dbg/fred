#!/usr/bin/python

import fredio
import fredutil

#****************************************************************************
#*   Copyright (C) 2010 by Kapil Arya, Gene Cooperman, Tyler Denniston,     *
#*     Artem Polyakov, Praveen S. Solanki, and Ana Maria Visan              *
#*   kapil@ccs.neu.edu, gene@ccs.neu.edu, tyler@ccs.neu.edu,                *
#*   artpol84@gmail.com, psolanki@ccs.neu.edu, amvisan@ccs.neu.edu          *
#*                                                                          *
#*  FReD is free software: you can redistribute it and/or                   *
#*  modify it under the terms of the GNU General Public License as          *
#*  published by the Free Software Foundation, either version 3 of the      *
#*  License, or (at your option) any later version.                         *
#*                                                                          *
#*  FReD is distributed in the hope that it will be useful,      	    *
#*  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
#*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
#*  GNU General Public License for more details.                            *
#*                                                                          *
#*  You should have received a copy of the GNU  General Public              *
#*  License along with FReD.  If not, see                                   *
#*  <http://www.gnu.org/licenses/>.                                         *
#****************************************************************************

import os
import shutil
import sys
import subprocess
import string
import time
import re
import pdb

# Set this to True (or anything non-null) for lots of debugging messages.
try:
    DEBUG = os.environ['DMTCP_MANAGER_DEBUG']
except KeyError:
    DEBUG = None
# Set to True to perform housekeeping for 'synchronization-log*' files
# created by DMTCP.
SYNCHRONIZATION_LOG_HANDLING = True
SYNC_LOG_BASENAME = "synchronization-"

# ---------- Global constants
DMTCP_MANAGER_ROOT = "/tmp/fred.%s/fred-dmtcp-manager/" % os.environ["USER"]
DMTCP_PORT   = 0
TTYFD        = sys.stdin.fileno()  # stdin by default
# ---------- End of Global constants 

# ---------- Global variables
childInTransition           = False # Child in irregular state
# List of different basenames for checkpoint files. Example:
# ["ckpt_gdb_1cb82bdb80f31c4-7852-4d7d1d4c.dmtcp",
#  "ckpt_mysqld_1cb82bdb80f31c4-7861-4d7d1d4c.dmtcp"]
checkpoint_basenames          = []
# The same thing but the ckpt_files_* directories:
checkpoint_files_basenames    = []
# The same but synchronization-log* files:
synchronization_log_basenames = []
# Counter variable to append to ckpt files
ckptCounter                = -1
# The current checkpoint index
currentCkptIndex           = -1
# The global number of checkpoints
numCheckpoints             = 0
# ---------- End of Global variables

def initialize_global_variables():
    global childInTransition, ckptCounter, currentCkptIndex, numCheckpoints
    childInTransition           = False # Child in irregular state
    # Counter variable to append to ckpt files
    ckptCounter                = -1
    # The current checkpoint index
    currentCkptIndex           = -1
    # The global number of checkpoints
    numCheckpoints             = 0

class DMTCPManager:
    ''' This class manages all interaction with DMTCP (through "dmtcp_command"
        commands)'''

    @staticmethod
    def checkPath():
        ''' Checks to see if DMTCP binaries are in the user`s path. '''
        for path in os.environ['PATH'].split(os.pathsep):
            exe_file = os.path.join(path, 'dmtcp_command')
            if os.path.exists(exe_file) and os.access(exe_file, os.X_OK):
                return True
        return False

    @staticmethod
    def getNumPeers():
        ''' Returns NUM_PEERS from "dmtcp_command s" as an int '''
        cmd = [ 'dmtcp_command', "--port", str(DMTCP_PORT), 's' ]
        output = executeShellCommand(cmd)
        if output != None:
            exp = '^NUM_PEERS=(\d+)'
            m = re.search(exp, output, re.MULTILINE)
            if m != None:
                numPeers = m.group(1)
                dprint("Got NUM_PEERS from DMTCP: " + numPeers)
                return int(numPeers)
            else:
                if output == "":
                    fredutil.fred_error("Output was NULL string")
                return 0  # Heuristically guessing 0 peers, could be a problem
        else:
            fredutil.fred_error("ERROR: Can't get NUM_PEERS. "
                                "Did the coordinator die?")
            return 0

    @staticmethod
    def isRunning():
        ''' Returns true if dmtcp_command reports RUNNING as "yes". '''
        cmd = ["dmtcp_command", "--quiet", "--port", str(DMTCP_PORT), "s"]
        output = executeShellCommand(cmd)
        if output != None:
            running = re.search('RUNNING=(\w+)', output, re.MULTILINE).group(1)
            return running == 'yes'
        else:
            fredutil.fred_error("ERROR: Can't get RUNNING."
                                "Did the coordinator die?")
            return False
        
    @staticmethod
    def killPeers():
        ''' Sends "k" command to DMTCP. '''
        cmd = ["dmtcp_command", "--quiet", "--port", str(DMTCP_PORT), "k"]
        dprint("Sending command '%s'" % ' '.join(cmd))
        #executeShellCommand(['dmtcp_command','k'])
        if fredio.GB_FRED_DEMO:
	    print "===================== KILLING gdb ====================="
        pid = os.fork()
        if pid == 0:
            sys.stderr = sys.stdout
            os.execvp('dmtcp_command', ['dmtcp_command', '--quiet', 'k'])
        else:
            os.waitpid(pid, 0)

    @staticmethod
    def restartWithCheckpointFiles(list):
        ''' Runs dmtcp_restart with the given list of ckpt files. '''
        cmdstr = ["dmtcp_restart", "--quiet", "--port", str(DMTCP_PORT)]
        for f in list:
            cmdstr.append(f)
        dprint("Executing restart command: %s" % str(cmdstr))
        if fredio.GB_FRED_DEMO:
	    print "===================== RESTARTING gdb ====================="
        fredio.reexec(cmdstr)

    @staticmethod
    def checkpoint():
        ''' Performs a blocking checkpoint request. '''
        cmdstr = ["dmtcp_command", "--quiet", "--port", str(DMTCP_PORT), "bc"]
        executeShellCommandAndWait(cmdstr)
        dprint("After blocking checkpoint command.")

def start(argv, dmtcp_port):
    '''Initializes and starts the child.'''
    global childInTransition, DMTCP_PORT
    dprint("Starting from start().")
    childInTransition = True
    dprint("Using port %d for DMTCP." % dmtcp_port)
    DMTCP_PORT = dmtcp_port
    # Use DMTCP_GZIP=0 : fast checkpoint/restart, but larger checkpoint files
    os.environ["DMTCP_GZIP"] = '0'
    os.environ["DMTCP_QUIET"] = '2'
    # Don't do anything if we can't find DMTCP.
    if not DMTCPManager.checkPath():
        fredutil.fred_fatal("No DMTCP binaries available in your PATH.\n")
    initializeFiles()
    childInTransition = False
    return 0

def resume(argv, dmtcp_port, s_resume_dir):
    '''Initializes and starts the child, resuming session from given path,
    and restarting from latest checkpoint in resume directory.'''
    global SYNC_LOG_BASENAME, checkpoint_basenames, checkpoint_files_basenames,\
           synchronization_log_basenames, ckptCounter, numCheckpoints
    if not os.path.exists(s_resume_dir):
        fredutil.fred_fatal("Cannot resume: %s doesn't exist." % s_resume_dir)
    start(argv, dmtcp_port)
    # Build lists of relevant files:
    list_ckpt_files = []
    list_ckpt_file_files = []
    list_sync_files = []
    for item in executeShellCommand(["ls", s_resume_dir]).split("\n"):
        item = os.path.basename(item)
        if item.find(".dmtcp.") != -1:
            list_ckpt_files.append(item)
            continue
        elif item.startswith("ckpt_files"):
            list_ckpt_file_files.append(item)
            continue
        elif item.startswith(SYNC_LOG_BASENAME):
            list_sync_files.append(item)
    ckptCounter = -1
    # Initialize number of checkpoint files, and basenames.
    fredutil.fred_info("Copying files for session resume. This could take a "
                       "few minutes if the checkpoint images are large.")
    for item in list_ckpt_files:
        full_src_path = os.path.join(s_resume_dir, item)
        executeShellCommandAndWait(["cp", full_src_path, DMTCP_MANAGER_ROOT])
        # Keep track of the maximum '.x' suffix found:
        i = int(item[item.rfind(".")+1:])
        if i > ckptCounter:
            ckptCounter = i
        item = item[:item.rfind(".")]
        if item not in checkpoint_basenames:
            checkpoint_basenames.append(item)
    numCheckpoints = ckptCounter + 1 # the .x suffixes count from 0.
    ckptCounter += 1
    for item in list_ckpt_file_files:
        full_src_path = os.path.join(s_resume_dir, item)
        executeShellCommandAndWait(["cp", full_src_path, DMTCP_MANAGER_ROOT])
        item = item[:item.rfind(".")]
        if item not in checkpoint_files_basenames:
            checkpoint_files_basenames.append(item)
    for item in list_sync_files:
        full_src_path = os.path.join(s_resume_dir, item)
        executeShellCommandAndWait(["cp", full_src_path, DMTCP_MANAGER_ROOT])
        item = item[:item.rfind(".")]
        if item not in synchronization_log_basenames:
            synchronization_log_basenames.append(item)
    # Restart from the latest checkpoint, and go:
    fredutil.fred_info("Resuming session from most recent checkpoint.")
    restart_last_ckpt()


def remove_manager_root():
    """Remove manager root directory and contents."""
    fredutil.fred_debug("Removing manager directory '%s'" % \
                        DMTCP_MANAGER_ROOT)
    # Safety feature: assert that the directory contains "/tmp", just in case.
    fredutil.fred_assert(DMTCP_MANAGER_ROOT.find("/tmp") != -1)
    shutil.rmtree(DMTCP_MANAGER_ROOT, ignore_errors = True)

def manager_quit():
    ''' Used when manager is in module mode to kill the child, and perform any
    cleanup. '''
    global childInTransition, DMTCP_MANAGER_ROOT
    fredutil.fred_debug("DMTCP Manager exiting.")
    childInTransition = True
    DMTCPManager.killPeers()
    remove_manager_root()
    # Needed for fredtest.py multiple sessions:
    initialize_global_variables()
    
def initializeFiles():
    ''' Initalizes housekeeping files and directories. '''
    global DMTCP_MANAGER_ROOT, numCheckpoints
    # Create DMTCP_MANAGER_ROOT if it doesn't exist
    if not fileExists(DMTCP_MANAGER_ROOT):
        dprint("%s doesn't exist; creating it now." % DMTCP_MANAGER_ROOT)
        os.makedirs(DMTCP_MANAGER_ROOT, 0755)
    numCheckpoints = 0

def availableCkpts():
    ''' Returns the list of available checkpoints. '''
    global numCheckpoints, checkpoint_basenames
    if len(checkpoint_basenames) > 0:
        ckptFile = checkpoint_basenames[0]
    else:
        dprint("No checkpoint list files available.")
        return
    ckpt_list = []
    for i in range(0, numCheckpoints):
        ckpt_list.append(ckptFile + ".%d" % i)
    dprint("Done loading ckpt list.")
    return ckpt_list    

def ckptList(index):
    ''' Return the list of checkpoint files associated with a given index. '''
    global checkpoint_basenames
    ckptFileList = []
    for item in checkpoint_basenames:
        ckpt = DMTCP_MANAGER_ROOT + item + ".%d" % index
        ckptFileList.append(ckpt)
    return ckptFileList

def removeFilePattern(path):
    ''' Removes files starting with the given string. To remove files relative
        to the current directory, call:
          removeFilePattern("some_prefix_")
        Or, to remove absolute paths:
          removeFilePattern("/absolute/path/some_prefix_") '''
    directory = os.path.dirname(path)
    pattern = os.path.basename(path)
    if directory == '':
        directory = '.'
    dprint("Getting list of files starting with '%s' in dir '%s'" % \
           (pattern, directory))
    # Get the list of files in 'directory' which start with 'pattern':
    files = [x for x in executeShellCommand(["ls", directory]).split('\n')\
             if x.startswith(pattern)]
    for f in files:
        dprint("Removing: %s" % directory + "/" + f)
        os.remove(directory + "/" + f)

def setupCheckpointSupportFiles(index):
    ''' Performs setup for support files associated with the given ckpt index:
          - ckpt_files_* directories, if any
          - synchronization-log-* files, by creating symbolic links from the
            DMTCP_MANAGER_ROOT directory to where DMTCP expects to find the
            files. '''
    global checkpoint_basenames, checkpoint_files_basenames, \
           synchronization_log_basenames
    ckptDir = os.getenv('DMTCP_CHECKPOINT_DIR', '.')
    # Remove old 'ckpt_files_*' symlinks:
    removeFilePattern(ckptDir + "/ckpt_files_")
    # Create new 'ckpt_files_*' symlinks:
    for item in checkpoint_files_basenames:
        filesDir = item + ".%d" % index
        # Files dir with no '.x' suffix:
        filesDirNoSuffix = item
        dprint("Symlinking %s to %s/%s." % (DMTCP_MANAGER_ROOT + filesDir, \
                                            ckptDir, \
                                            filesDirNoSuffix))
        os.symlink(DMTCP_MANAGER_ROOT + filesDir, ckptDir + "/" + filesDirNoSuffix)

    for item in synchronization_log_basenames:
        logfile = item + ".%d" % index
        logfileNoSuffix = item
        full_path = os.environ["DMTCP_TMPDIR"] + "/" + logfileNoSuffix
        if not os.path.exists(full_path):
            # If it doesn't exist, this is probably a resume. Just ignore it.
            fredutil.fred_debug("path %s does not exist. Ignoring." % \
                                    full_path)
        elif os.path.islink(full_path):
            # It is a link; we can safely just delete it.
            os.remove(full_path)
        else:
            # It is not a link. This means DMTCP previously patched the log
            # and created a new file. We should replace the old log we had
            # under this index with the new version so we don't lose the
            # patched version.
            oldLog = "%s/%s.%d" % \
                (DMTCP_MANAGER_ROOT, logfileNoSuffix, index)
            dprint("Replacing old log %s with new (patched) log %s" % \
                       (oldLog, full_path))
            os.remove(oldLog)
            shutil.move(full_path, oldLog)
        # Create a new symlink for the target index's sync log:
        dprint("Symlinking %s to %s." % \
                   (DMTCP_MANAGER_ROOT + logfile, full_path))
        os.symlink(DMTCP_MANAGER_ROOT + logfile, full_path)

def restart_last_ckpt():
    ''' Restart from the most recent checkpoint. '''
    global numCheckpoints
    dprint("Restoring most recent ckpt image: %d." % (numCheckpoints-1))
    restart_ckpt(numCheckpoints-1)

def restart_ckpt(index):
    ''' Restart from the ckpt file(s) referenced by the given index. '''
    global DMTCP_MANAGER_ROOT, childInTransition, currentCkptIndex
    dprint("Going to restart index %d" % index)
    childInTransition = True
    # Kill the currently connected peers:
    DMTCPManager.killPeers()
    fredio.kill_child() # XXX: hack: killPeers() doesn't do the job for Matlab.
    # Wait until the peers are really gone
    while DMTCPManager.getNumPeers() != 0:
        time.sleep(0.1)
    # Set up any support files associated with the checkpoint: ckpt_files_*, etc
    setupCheckpointSupportFiles(index)
    # The number of peers we wait on to restart is equal to the number of
    # checkpoint files we're using to restart.  Don't use
    # DMTCPManager.getNumPeers() because we may want a different number of
    # peers than what we have now. I.e. you have a gdb session whose traced
    # process died, and you want to restart.
    ckptFileList = ckptList(index)
    # Move checkpoint files to conform with the template: ckpt_*.dmtcp
    for i in range(0, len(ckptFileList)):
        ckpt = ckptFileList[i]
        # Remove .x suffix:
        new_name = ckpt[:string.rfind(ckpt, '.')]
        dprint("Symlinking %s to %s for restart." % (ckpt, new_name))
        # Delete file in case it exists from a previous restart:
        if os.path.exists(new_name):
            os.remove(new_name)
        os.symlink(ckpt, new_name)
        ckptFileList[i] = new_name
    numPeers = len(ckptFileList)
    DMTCPManager.restartWithCheckpointFiles(ckptFileList)
    dprint("Restart: waiting until %d peers." % numPeers)
    # Wait until every peer has finished resuming:
    while DMTCPManager.getNumPeers() < numPeers or not DMTCPManager.isRunning():
        time.sleep(0.1)
    dprint("Done waiting. peers: %d, running: %s" % \
           (DMTCPManager.getNumPeers(), str(DMTCPManager.isRunning())))
    currentCkptIndex = index
    childInTransition = False

def erase_checkpoints(n_start_idx, n_end_idx):
    """Erase the given interval of checkpoints (inclusive)."""
    pass

def updateHousekeeping(filename):
    ''' Updates the required housekeeping files to include the filename. '''
    global DMTCP_MANAGER_ROOT, ckptCounter, checkpoint_files_basenames, \
           checkpoint_basenames
    if fileExists(filename):
        # Add counter and move to DMTCP_MANAGER_ROOT
        dprint("Renaming \"%s\" to \"%s.%d\"" % (filename,
               DMTCP_MANAGER_ROOT + filename, ckptCounter))
        shutil.move(filename, "%s.%d" % (DMTCP_MANAGER_ROOT+filename, ckptCounter))
        # Add to basenames if it's not there already
        if filename.find("_files_") != -1:
            if filename not in checkpoint_files_basenames:
                checkpoint_files_basenames.append(filename)
        else:
            if filename not in checkpoint_basenames:
                checkpoint_basenames.append(filename)
    else:
        dprint("File %s does not exist; skipping." % filename)
        return
    dprint("Finished housekeeping for %s" % filename)

def updateSyncHousekeeping(filename):
    '''
    Perform housekeeping for the synchronization log files. On checkpoint, DMTCP
    creates the synchronization logs and starts writing to them. The sketch is:
      - take checkpoint A
      - (1) create log file A
      - record to log file A
      - take checkpoint B
      - (2) move log file A out of the way
      - create log file B
      - record to log file B
      ...
    Since DMTCP does not know anything about multiple checkpoint images or
    synchronization files, upon checkpoint the manager must move them out of
    the way. For steps 1, and 2 above, the manager does something special.
    
      (1) Upon checkpoint, we move the synchronization-log* files to
          DMTCP_MANAGER_ROOT. Then we symlink those files back to the original
          location, so that all modifications made by DMTCP go to the correct
          file.

      (2) Upon the 2nd checkpoint, we remove the symlinks created in step (1).
          Then we create new symlinks to point to the 2nd log files. DMTCP does
          not know about the new file, and so thinks it is writing to the same
          file.

    Some special consideration is needed for how DMTCP patches the log files.
    '''
    global synchronization_log_basenames
    dprint("Updating housekeeping information for synchronization log %s" % \
           filename)
    full_filename = os.environ["DMTCP_TMPDIR"] + "/" + filename
    dprint("Full filename for log is: %s" % full_filename)
    # Add counter and move to DMTCP_MANAGER_ROOT
    if os.path.islink(full_filename):
        # Remove the old link (step 2 in above comment)
        dprint("Removing old symlink %s." % full_filename)
        os.remove(full_filename)
    else:
        # Not a link; this must be the first checkpoint
        dprint("Renaming \"%s\" to \"%s.%d\"" % \
               (full_filename, DMTCP_MANAGER_ROOT + filename, ckptCounter))
        shutil.move(full_filename,
                  "%s.%d" % (DMTCP_MANAGER_ROOT + filename, ckptCounter))
    # Leave a symlink for DMTCP to write to
    dprint("Symlinking %s.%d to %s" % \
           (DMTCP_MANAGER_ROOT + filename, ckptCounter, full_filename))
    os.symlink("%s.%d" % (DMTCP_MANAGER_ROOT + filename, ckptCounter),
               full_filename)
    # Add to basenames if it's not there already
    if filename not in synchronization_log_basenames:
        synchronization_log_basenames.append(filename)

def do_checkpoint():
    ''' Do the checkpointing, and associated housekeeping. '''
    global ckptCounter, numCheckpoints, currentCkptIndex
    ckptCounter += 1
    numCheckpoints += 1
    numPeers = DMTCPManager.getNumPeers()
    dprint("Number of peers getting checkpointed: %d." % numPeers)
    listDmtcpFilesBefore = \
        [x for x in executeShellCommand(["ls"]).split('\n') \
         if x.endswith(".dmtcp")]
    dprint("List of *.dmtcp files before: %s" % listDmtcpFilesBefore)
    listCkptFileDirsBefore = \
        [x for x in executeShellCommand(["ls"]).split('\n') \
         if x.startswith("ckpt_files")]
    dprint("List of ckpt_files* before: %s" % listCkptFileDirsBefore)
    dprint("Going to send blocking checkpoint command.")
    DMTCPManager.checkpoint()
    listDmtcpFilesAfter = [x for x in executeShellCommand(["ls"]).split('\n') \
                       if x.endswith(".dmtcp")]
    dprint("List of *.dmtcp files after: %s" % listDmtcpFilesAfter)
    dmtcpDiffs = listDiff(listDmtcpFilesBefore, listDmtcpFilesAfter)
    dprint("Differences are: %s" % dmtcpDiffs)
    listCkptFileDirsAfter = \
        [x for x in executeShellCommand(["ls"]).split('\n') \
         if x.startswith("ckpt_files")]
    dprint("List of ckpt_files* after: %s" % listCkptFileDirsAfter)
    fileDiffs = listDiff(listCkptFileDirsBefore, listCkptFileDirsAfter)
    dprint("Differences are: %s" % fileDiffs)
    for diff in dmtcpDiffs:
        updateHousekeeping(diff)
    for diff in fileDiffs:
        updateHousekeeping(diff)
    listSyncLogs = \
        [x for x in executeShellCommand(["ls", os.environ["DMTCP_TMPDIR"]]).split('\n') \
             if x.startswith(SYNC_LOG_BASENAME)]
    for diff in listSyncLogs:
        updateSyncHousekeeping(diff)
    currentCkptIndex = ckptCounter

def listDiff(one, two):
    ''' Effectively performs "list subtraction"--returns two - one.  That is,
        returns a list of all the elements in "two" that are not in "one".'''
    diffs = []
    for item in two:
        if item not in one:
            diffs.append(item)

    return diffs

def fileExists(filename):
    ''' Returns true if the given file exists. '''
    return os.path.exists(filename)

def executeShellCommandAndWait(cmd):
    """ Executes a shell command and calls waitpid() on it. That means that
        this is a blocking call. """
    rc = subprocess.check_call(cmd, stderr=subprocess.STDOUT)
    return rc

def executeShellCommand(cmd):
    ''' Executes a shell command and returns its output.
        Argument should be a list of the arguments.'''    
    dprint("Executing command string: %s" % cmd)
    # If we omit stdin arg and p.stdin.close(), then this doesn't work.  Why?
    p = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                         stderr=subprocess.STDOUT, close_fds=True)
    p.stdin.close()
    return p.stdout.read()

def dprint(msg):
    ''' Prints debugging messages to console if the DMTCP_MANAGER_DEBUG env
        variable is set.'''
    global DEBUG
    if DEBUG != None:
        caller_name = sys._getframe(1).f_code.co_name
        caller_lineno = sys._getframe(1).f_lineno
        # Prints name of function and line number making call to dprint()
        print "[mon-debug] %s():%d - %s" % (caller_name, caller_lineno, msg)
