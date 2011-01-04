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
DMTCP_MANAGER_ROOT = "/tmp/fred.%s/.fred-dmtcp-manager/" % os.environ["USER"]
DMTCP_PORT   = 0
TTYFD        = sys.stdin.fileno()  # stdin by default
# ---------- End of Global constants 

# ---------- Global variables
childInTransition           = False # Child in irregular state
ckptListFilePaths           = [] # List of checkpoint list files
openFilesListPaths          = []
synchronizationLogListFiles = []
# Path to the checkpoint index file (contains total # of ckpt files)
ckptIndexFilePath           = ""
# Counter variable to append to ckpt files
ckptCounter                = -1
# The current checkpoint index
currentCkptIndex           = -1
# The global number of checkpoints
numCheckpoints             = 0
# ---------- End of Global variables

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
                fredutil.fred_error("ERROR: Can't parse NUM_PEERS= output "
                                    "from coordinator")
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

    if len(argv) < 2: # Default case, no arguments
        fredutil.fred_fatal("Wrong number of arguments to invoke "
                            "DMTCP Manager.")
    
    #spawnChild(argv)

    # Define the "first" program to be the first argument to dmtcp_checkpoint
    # that doesn't begin with a hyphen.
    for arg in argv[1:]:
        if arg.startswith('-'):
            continue
        else:
            first = arg
            break
    # Get the basename if there are / present:
    if first.find('/') >= 0:
        # path to file, split on slashes and take the text after the last slash
        exp = '.*/(.+)$'
        firstName = re.search(exp, first).group(1)
    else:
        firstName = first
    dprint("Got firstName to be: %s" % firstName)
    initializeFiles(firstName)
    childInTransition = False
    return 0

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
    
def initializeFiles(firstName):
    ''' Initalizes housekeeping files and directories. '''
    global DMTCP_MANAGER_ROOT, ckptIndexFilePath, numCheckpoints

    dprint("Initializing files based on firstName: %s" % firstName)

    # Create DMTCP_MANAGER_ROOT if it doesn't exist
    if not fileExists(DMTCP_MANAGER_ROOT):
        dprint("%s doesn't exist; creating it now." % DMTCP_MANAGER_ROOT)
        os.makedirs(DMTCP_MANAGER_ROOT, 0755)

    # Initialize checkpoint index file path: file which contains latest index
    # of checkpoint.  We only care about the first program name because any
    # parent/child relationships (i.e. multiple program names)
    # have checkpoint indices that are the same
    dprint("firstName is: %s" % firstName)
    dprint("Initializing ckptIndexFilePath to: %s" % DMTCP_MANAGER_ROOT +
           firstName)
    ckptIndexFilePath = DMTCP_MANAGER_ROOT + firstName

    # Based on that, load the file (if it exists) and get the value inside it
    if fileExists(ckptIndexFilePath):
        f = open(ckptIndexFilePath, 'r')
        num = int(f.read()) if (f != None) else 0
        f.close()
    else:
        # It doesn't exist, initialize num to 0
        num = 0
    numCheckpoints = num

def readLineNumber(n, file):
    ''' Returns nth line from file 'file'. NB: If used in conjunction with a
        checkpoint index number, the caller should add 1 to n.
        Checkpoint index 0 == line number 1. '''
    dprint("Reading line %d from file %s" % (n, file))
    f = open(file)
    line = ""
    for i in range(0, n):
        line = f.readline()
    f.close()
    return line.rstrip()

def availableCkpts():
    ''' Returns the list of available checkpoint indices. '''
    global ckptListFilePaths
    if len(ckptListFilePaths) > 0:
        ckptFile = ckptListFilePaths[0]
    else:
        dprint("No checkpoint list files available.")
        return
    ckpt_list = []
    dprint("Opening list file: %s" % ckptFile)
    f = open(ckptFile)
    for line in f:
        line = line.rstrip()
        ckpt_list.append(line)
    dprint("Done loading ckpt list.")
    return ckpt_list    

def ckptList(index):
    ''' Return the list of checkpoint files associated with a given index. '''
    global ckptListFilePaths
    ckptFileList = []
    for item in ckptListFilePaths:
        dprint("Opening ckpt list file: %s" % item)
        ckpt = DMTCP_MANAGER_ROOT + readLineNumber(index+1, item)
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
          - synchronization-log-* files, if SYNCHRONIZATION_LOG_HANDLING
        by creating symbolic links from the DMTCP_MANAGER_ROOT directory to where
        DMTCP expects to find the files. '''
    global currentCkptIndex
    ckptDir = os.getenv('DMTCP_CHECKPOINT_DIR', '.')
    # Remove old 'ckpt_files_*' symlinks:
    removeFilePattern(ckptDir + "/ckpt_files_")
    # Create new 'ckpt_files_*' symlinks:
    for item in openFilesListPaths:
        dprint("Opening open files list file: %s" % item)
        filesDir = readLineNumber(index+1, item)
        # Files dir with no '.x' suffix:
        filesDirNoSuffix = re.search('(.+)\.[0-9]+', filesDir).group(1)
        dprint("Symlinking %s to %s/%s." % (DMTCP_MANAGER_ROOT + filesDir, \
                                            ckptDir, \
                                            filesDirNoSuffix))
        os.symlink(DMTCP_MANAGER_ROOT + filesDir, ckptDir + "/" + filesDirNoSuffix)
    if SYNCHRONIZATION_LOG_HANDLING:
        for item in synchronizationLogListFiles:
            dprint("Opening synchronization log list file: %s" % item)
            logfile = readLineNumber(index+1, item)
            dprint("Got log name: %s" % logfile)
            logfileNoSuffix = re.search('(.+)\.[0-9]+', logfile).group(1)
            full_path = os.environ["DMTCP_TMPDIR"] + "/" + logfileNoSuffix
            if os.path.islink(full_path):
                # It is a link; we can safely just delete it.
                os.remove(full_path)
            else:
                # It is not a link. This means DMTCP previously patched the log
                # and created a new file. We should replace the old log we had
                # under this index with the new version so we don't lose the
                # patched version.
                oldLog = DMTCP_MANAGER_ROOT + "/" + logfileNoSuffix + \
                         "." + str(currentCkptIndex)
                dprint("Replacing old log %s in favor of new (patched) log %s" \
                       % (oldLog, full_path))
                os.remove(oldLog)
                os.rename(full_path, oldLog)
            # Create a new symlink for the target index's sync log:
            dprint("Symlinking %s to %s." % (DMTCP_MANAGER_ROOT + logfile, full_path))
            os.symlink(DMTCP_MANAGER_ROOT + logfile, full_path)

def restart_last_ckpt():
    ''' Restart from the most recent checkpoint. '''
    global numCheckpoints
    dprint("Restoring most recent ckpt image: %d." % (numCheckpoints-1))
    restart_ckpt(numCheckpoints-1)

def restart_ckpt(index):
    ''' Restart from the ckpt file(s) referenced by the given index. '''
    global ckptListFilePaths, openFilesListPaths, DMTCP_MANAGER_ROOT
    global childInTransition, ckptCounter, currentCkptIndex
    dprint("Going to restart index %d" % index)
    childInTransition = True
    # Kill the currently connected peers:
    #killChild() # XXX: hack: killPeers() doesn't do the job for Matlab.
    DMTCPManager.killPeers()
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
    # Wait until every peer has finished resuming:
    while DMTCPManager.getNumPeers() < numPeers:
        time.sleep(0.1)
    currentCkptIndex = index
    childInTransition = False

def getProgramNameFromCkpt(name):
    ''' Returns the program name from the given checkpoint filename. '''
    # TODO: consolidate into one regular expression
    
    #ckpt_simulate_gdb_30a1a199-20161-4a7f1436.dmtcp.1
    s = re.search('ckpt_(.+)_[0-9a-z]+-[0-9a-z]+-[0-9a-z]+\.dmtcp.*', name)
    if s != None:
        return s.group(1)
    else:
        # Try with unique counter format
        #ckpt_simulate_gdb_30a1a199-20161-4a7f1436_0001.dmtcp.1
        s = re.search('ckpt_(.+)_[0-9a-z]+-[0-9a-z]+-[0-9a-z]+_[0-9]+\.dmtcp',
                      name)
        return s.group(1)

def getProgramNameFromFilesDir(name):
    ''' Returns the program name from the given "ckpt_files_*"
        directory name. '''
    # ckpt_files_mysqld_5f6d41e6-29643-4c7e6ad6
    s = re.search('ckpt_files_(.+)_[0-9a-z]+-[0-9a-z]+-[0-9a-z]+', name)
    return s.group(1)

def updateHousekeeping(filename):
    ''' Updates the required housekeeping files to include the filename. '''
    global DMTCP_MANAGER_ROOT, ckptCounter, ckptListFilePaths, ckptIndexFilePath

    if fileExists(filename):
        # Add counter and move to DMTCP_MANAGER_ROOT
        dprint("Renaming \"%s\" to \"%s.%d\"" % (filename,
               DMTCP_MANAGER_ROOT + filename, ckptCounter))
        os.rename(filename, "%s.%d" % (DMTCP_MANAGER_ROOT+filename, ckptCounter))
    else:
        dprint("File %s does not exist; skipping." % filename)
        return
    progname = getProgramNameFromCkpt(filename)
    dprint("Got program name from checkpoint file: %s" % progname)
    listfilepath = DMTCP_MANAGER_ROOT + progname + "_list"
    # Want to see if it exists first. If not, will add it to
    # ckptListFilePaths
    if not listfilepath in ckptListFilePaths:
        dprint("Didn't find checkpoint list file. Adding new entry: %s"
               % listfilepath)
        ckptListFilePaths.append(listfilepath)

    f = open(listfilepath, "a") # Append to end
    f.write("%s.%d\n" % (filename, ckptCounter))
    f.close()
    f = open(ckptIndexFilePath, "w")  # Overwrite
    f.write(str(numCheckpoints))
    f.close()
    dprint("Finished housekeeping for %s" % filename)

def updateOpenFilesHousekeeping(filename):
    ''' Updates the required housekeeping files to include the filename. '''
    global DMTCP_MANAGER_ROOT, ckptCounter, openFilesListPaths

    if fileExists(filename):
        # Add counter and move to DMTCP_MANAGER_ROOT
        dprint("Renaming \"%s\" to \"%s.%d\"" % (filename,
               DMTCP_MANAGER_ROOT + filename, ckptCounter))
        os.rename(filename, "%s.%d" % (DMTCP_MANAGER_ROOT+filename, ckptCounter))
    else:
        dprint("File %s does not exist; skipping." % filename)
        return
    progname = getProgramNameFromFilesDir(filename)
    dprint("Got program name from open files directory: %s" % progname)
    listfilepath = DMTCP_MANAGER_ROOT + progname + "_files_list"
    # Want to see if it exists first. If not, will add it to
    # openFilesListPaths
    if not listfilepath in openFilesListPaths:
        dprint("Didn't find checkpoint list file. Adding new entry: %s"
               % listfilepath)
        openFilesListPaths.append(listfilepath)

    f = open(listfilepath, "a") # Append to end
    f.write("%s.%d\n" % (filename, ckptCounter))
    f.close()
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
    global synchronizationLogListFiles
    if not SYNCHRONIZATION_LOG_HANDLING:
        return
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
        os.rename(full_filename,
                  "%s.%d" % (DMTCP_MANAGER_ROOT + filename, ckptCounter))
    # Leave a symlink for DMTCP to write to
    dprint("Symlinking %s.%d to %s" % \
           (DMTCP_MANAGER_ROOT + filename, ckptCounter, full_filename))
    os.symlink("%s.%d" % (DMTCP_MANAGER_ROOT + filename, ckptCounter),
               full_filename)
    listfilepath = DMTCP_MANAGER_ROOT + filename + "_list"
    if not listfilepath in synchronizationLogListFiles:
        dprint("Didn't find sync log list file. Adding new entry: %s"
               % listfilepath)
        synchronizationLogListFiles.append(listfilepath)
    f = open(listfilepath, "a") # Append to end
    f.write("%s.%d\n" % (filename, ckptCounter))
    f.close()

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
        updateOpenFilesHousekeeping(diff)
    if SYNCHRONIZATION_LOG_HANDLING:
        listSyncLogs = \
            [x for x in executeShellCommand(["ls", os.environ["DMTCP_TMPDIR"]]).split('\n') \
             if x.startswith(SYNC_LOG_BASENAME)]
        for log in listSyncLogs:
            updateSyncHousekeeping(log)

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
