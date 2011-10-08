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
import pdb
import os
import shutil
import signal
import sys

from fred import dmtcpmanager
from fred import fredmanager
from fred import fredio
from fred import freddebugger
from fred import fredutil
from fred.algorithms import reverse_watch
from fred.algorithms import reverse_next
from fred.algorithms import reverse_step
from fred.algorithms import reverse_finish
from fred.algorithms import reverse_continue
from fred.algorithms import undo

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

Please make it a goal to document every public function with a docstring. It
only takes a few seconds, and saves a lot of time for future programmers. For
docstring conventions, you can see PEP-257.
'''


######################## Global Constants #####################################
GS_FRED_VERSION="Version: %prog 0.99-r277 (Nov. 12 2010)"
GS_FRED_USAGE="USAGE: %prog [options] xdb [ARGS] a.out [A.OUT ARGS]\n" + \
               "Replace `xdb' with the name of the target debugger"
GS_FRED_COMMAND_PREFIX="fred-"
GS_FRED_TMPDIR = "/tmp/fred.%s" % os.environ['USER']
GS_DMTCP_TMPDIR = GS_FRED_TMPDIR + "/dmtcp_tmpdir"
######################## End Global Constants #################################

######################## Global Variables #####################################
# The global ReversibleDebugger instance.
g_debugger = None
g_source_script = None
gs_resume_dir_path = None
######################## End Global Variables #################################

def fred_command_help():
   print """FReD commands:
(all optional 'count' arguments default to 1)
  fred-undo [n]:         Undo last n debugger commands.
  fred-reverse-next [n], fred-rn [n]:  Reverse-next n times.
  fred-reverse-step [n], fred-rs [n]:  Reverse-step n times.
  fred-reverse-finish, fred-rf:        Reverse execute until function exited.
  fred-reverse-continue, fred-rc:      Reverse execute to previous breakpoint.
  fred-checkpoint, fred-ckpt: Request a new checkpoint to be made.
  fred-restart:               Restart from last checkpoint.
  fred-reverse-watch <EXPR>, fred-rw <EXPR>:
                              Reverse execute until expression EXPR changes.
  fred-source <FILE>:         Read commands from source file.
  fred-list:                  List the available checkpoint files.
  fred-help:                  Display this help message.
  fred-history:               Display your command history up to this point.
  fred-debug:                 (*Experts only) Drop into a pdb prompt for FReD.
"""
   sys.stdout.flush()

def is_quit_command(s_command):
    """Return True if s_command is a debugger 'quit' command."""
    return s_command in ["q", "quit", "exit"]

def is_fred_command(s_command):
    """Return True if the given command needs special handling."""
    global GS_FRED_COMMAND_PREFIX
    return s_command.startswith(GS_FRED_COMMAND_PREFIX) or \
           is_quit_command(s_command)

def handle_fred_command(s_command):
    """Performs handling of 'special' (non-debugger) commands."""
    global g_debugger, GS_FRED_COMMAND_PREFIX
    s_command = s_command.replace(GS_FRED_COMMAND_PREFIX, "")
    (s_command_name, sep, s_command_args) = s_command.partition(' ')
    n_count = fredutil.to_int(s_command_args, 1)
    if is_quit_command(s_command_name):
        fredutil.fred_quit(0)
    elif s_command_name == "undo":
        undo.undo(g_debugger, n_count)
    elif s_command_name in ["reverse-next", "rn"]:
        # iF THIS DOESN'T WORK FOR YOU, CHANGE IT BACK TO reverse_next().
        reverse_next.NEW_reverse_next(g_debugger, n_count)
    elif s_command_name in ["reverse-step", "rs"]:
        # iF THIS DOESN'T WORK FOR YOU, CHANGE IT BACK TO reverse_step().
        reverse_step.NEW_reverse_step(g_debugger, n_count)
    elif s_command_name in ["reverse-finish", "rf"]:
        # iF THIS DOESN'T WORK FOR YOU, CHANGE IT BACK TO reverse_step().
        reverse_finish.NEW_reverse_finish(g_debugger, n_count)
    elif s_command_name in ["reverse-continue", "rc"]:
        reverse_continue.reverse_continue(g_debugger)
    elif s_command_name in ["checkpoint", "ckpt"]:
        g_debugger.do_checkpoint()
    elif s_command_name == "restart":
        # n_count defaults to 1 if no argument given (not appropriate here)
        n_index = fredutil.to_int(s_command_name, 0)
        g_debugger.do_restart(n_index, b_clear_history=True)
    elif s_command_name in ["reverse-watch", "rw"]:
        reverse_watch.reverse_watch_with_log_support(g_debugger, s_command_args)
        #reverse_watch.reverse_watch(g_debugger, s_command_args)
    elif s_command_name == "source":
        source_from_file(s_command_args)
    elif s_command_name == "list":
        print g_debugger.list_checkpoints()
    elif s_command_name == "help":
        fred_command_help()
    elif s_command_name == "history":
        print g_debugger.history()
    elif s_command_name == "debug":
        pdb.set_trace()
    else:
        fredutil.fred_error("Unknown FReD command '%s'" % s_command_name)

def dispatch_command(s_command):
    """Given a user command, dispatches and executes it in the right way."""
    fredutil.fred_timer_start(s_command)
    # TODO: Currently we do not log fred commands. Do we need to?
    if is_fred_command(s_command):
        handle_fred_command(s_command)
    else:
        # XXX: Figure out a more elegant way to do this. We can't set the
        # inferior pid until we know the inferior is alive, so we keep trying
        # to update it with every command issued until it succeeds.
        if fredmanager.get_pid() == -1:
            n_inf_pid = fredutil.get_inferior_pid(fredio.get_child_pid())
            fredmanager.set_pid(n_inf_pid)

        fredio.send_command(s_command)
        g_debugger.log_command(s_command)
    fredutil.fred_timer_stop(s_command)

def source_from_file(s_filename):
    """Execute commands from given file."""
    fredutil.fred_debug("Start sourcing from file '%s'" % s_filename)
    f = fredutil.open_file(s_filename)
    if f == None:
        return
    for s_line in f:
        s_line = s_line.strip()
        if s_line == "":
           continue
        dispatch_command(s_line)
    f.close()
    fredutil.fred_debug("Finished sourcing from file '%s'" % s_filename)

def source_from_list(ls_cmds):
    """Execute commands from given list."""
    for s_cmd in ls_cmds:
        s_cmd = s_cmd.strip()
        dispatch_command(s_cmd)


def parse_program_args():
    """Initialize command line options, and parse them.
    Return the user's inferior to execute as a list."""
    global GS_FRED_USAGE, g_source_script, gs_resume_dir_path
    parser = OptionParser(usage=GS_FRED_USAGE, version=GS_FRED_VERSION)
    parser.disable_interspersed_args()
    # Note that '-h' and '--help' are supported automatically.
    default_port = os.getenv("DMTCP_PORT") or 7779
    parser.add_option("-p", "--port", dest="dmtcp_port", default=default_port,
                      help="Use PORT for DMTCP port number. (default %default)",
                      metavar="PORT")
    parser.add_option("-x", "--source", dest="source_script",
                      help="Execute batch file FILE", metavar="FILE")
    parser.add_option("--enable-debug", dest="debug", default=False,
                      action="store_true",
                      help="Enable FReD debugging messages.")
    parser.add_option("--fred-demo", dest="fred_demo", default=False,
                      action="store_true",
                      help="Enable FReD demo mode.")
    parser.add_option("--resume", dest="resume_dir",
                      help="Resume session from directory DIR containing "
                      "FReD support files: checkpoint images, "
                      "synchronization logs, etc.", metavar="DIR")
    (options, l_args) = parser.parse_args()
    # 'l_args' is the 'gdb ARGS ./a.out' list
    if len(l_args) == 0 and options.resume_dir == None:
        parser.print_help()
        fredutil.fred_quit(1)
    if options.source_script != None:
        # Source script executed from main_io_loop().
        g_source_script = options.source_script
    if options.fred_demo:
        fredio.GB_FRED_DEMO = True
    if options.resume_dir != None:
        # Resume session from given directory.
        gs_resume_dir_path = options.resume_dir
    setup_environment_variables(str(options.dmtcp_port), options.debug)
    return l_args

def setup_debugger(s_debugger_name):
    """Initialize global ReversibleDebugger instance g_debugger."""
    global g_debugger, gs_resume_dir_path
    if s_debugger_name == "gdb":
        fredutil.fred_debug("Using gdb personality.")
        from fred.personality.personalityGdb import PersonalityGdb
        g_debugger = freddebugger.ReversibleDebugger(PersonalityGdb())
        del PersonalityGdb
    elif s_debugger_name == "pdb":
        fredutil.fred_debug("Using python personality.")
        from fred.personality.personalityPython import PersonalityPython
        g_debugger = freddebugger.ReversibleDebugger(PersonalityPython())
        del PersonalityPython
    elif s_debugger_name == "perl":
        fredutil.fred_debug("Using perl personality.")
        from fred.personality.personalityPerl import PersonalityPerl
        g_debugger = freddebugger.ReversibleDebugger(PersonalityPerl())
        del PersonalityPerl
    elif s_debugger_name.endswith("MATLAB"):
        fredutil.fred_debug("Using matlab personality.")
        from fred.personality.personalityMatlab import PersonalityMatlab
        g_debugger = freddebugger.ReversibleDebugger(PersonalityMatlab())
        del PersonalityMatlab
    else:
        fredutil.fred_fatal("Unimplemented debugger '%s'" % s_debugger_name)

def setup_environment_variables(s_dmtcp_port="7779", b_debug=False):
    """Set up the given environment variables.
    If any environment variables are already set, the existing values
    take precedence. This is so the user can set up environment
    variables for DMTCP/FReD in their .bashrc, for example."""
    fredutil.GB_DEBUG = b_debug
    fredutil.set_env_var_if_unset("DMTCP_PORT", s_dmtcp_port)
    fredutil.set_env_var_if_unset("DMTCP_TMPDIR", GS_DMTCP_TMPDIR)
    fredutil.set_env_var_if_unset("DMTCP_CHECKPOINT_DIR", GS_DMTCP_TMPDIR)
    fredutil.set_env_var_if_unset("DMTCP_GZIP", '0')
    fredutil.set_env_var_if_unset("DMTCP_QUIET", '2')
    # Create the DMTCP tmpdir if it doesn't exist:
    if not os.path.exists(GS_DMTCP_TMPDIR):
       os.makedirs(GS_DMTCP_TMPDIR, 0755)

def setup_fredio(l_cmd, b_spawn_child=True):
    """Set up I/O handling."""
    global g_debugger
    fredio.g_find_prompt_function  = g_debugger.get_find_prompt_function()
    fredio.g_print_prompt_function = g_debugger.get_prompt_string_function()
    fredio.gre_prompt              = g_debugger.get_prompt_regex()
    fredio.gls_needs_user_input    = g_debugger.get_ls_needs_input()
    fredio.setup(l_cmd, b_spawn_child)
    
def interactive_debugger_setup():
    """Perform any debugger setup that requires a debugger prompt."""
    global g_debugger, g_source_script
    if g_debugger.personality_name() == "gdb":
        # Special case for gdb: record name of executable.
        g_debugger._p.set_inferior_name()
    # If the user gave a source script file, execute it now.
    if g_source_script != None:
        source_from_file(g_source_script)

def fred_setup():
    """Perform any setup needed by FReD before entering an I/O loop."""
    global g_debugger, gs_resume_dir_path, GS_FRED_TMPDIR
    # Parse command line args and set up environment.
    l_cmd = parse_program_args()
    # Set up the FReD global debugger
    setup_debugger(l_cmd[0])
    # Set up I/O handling
    # (only spawn if we are not resuming:)
    b_resume = gs_resume_dir_path != None
    if not b_resume:
        cleanup_fred_files()
        setup_fredio(l_cmd, True)
    else:
        setup_fredio(l_cmd, False)
        dmtcpmanager.resume(GS_FRED_TMPDIR, gs_resume_dir_path)
        g_debugger.setup_from_resume()
        # XXX: Right now this is a hack to get the virtualized pid of the
        # inferior. We should make this more robust. Ideally we could have
        # support from DMTCP (via dmtcp_command) to tell us the inferior
        # virtualized pid.
        fredutil.fred_assert(g_debugger.personality_name() == "gdb")
        n_inf_pid = int(g_debugger.evaluate_expression("getpid()"))
        fredmanager.set_pid(n_inf_pid)

def fred_setup_as_module(l_cmd, s_dmtcp_port, b_debug):
    """Perform setup for FReD when being used as a module, return g_debugger.
    For example, fredtest.py uses FReD as a module."""
    global g_debugger
    cleanup_fred_files()
    setup_environment_variables(s_dmtcp_port, b_debug)
    setup_debugger(l_cmd[0])
    setup_fredio(l_cmd, True)
    # Since modules won't use the main_io_loop, we perform the debugger setup
    # requiring a debugger prompt here.
    fredio.wait_for_prompt()
    interactive_debugger_setup()
    return g_debugger

def cleanup_fred_files():
    """Remove FReD temporary directory and contents."""
    global GS_FRED_TMPDIR
    if not os.path.exists(GS_FRED_TMPDIR):
       return
    fredutil.fred_info("Removing previous temporary directory '%s'" % \
                       GS_FRED_TMPDIR)
    fredutil.fred_assert(GS_FRED_TMPDIR.find("/tmp") != -1)
    shutil.rmtree(GS_FRED_TMPDIR, ignore_errors=True)

def verify_critical_files_present():
    """Check for DMTCP binaries, fredhijack.so, and other required files.
    If any are not found, exit with a fatal error and appropriate message."""
    if not dmtcpmanager.is_dmtcp_in_path():
        fredutil.fred_fatal("No DMTCP binaries available in your PATH.\n")
    if not fredmanager.is_fredhijack_found():
        fredutil.fred_fatal("No fredhijack.so library found in %s.\n"
                            "Please edit fredmanager.py and change "
                            "GS_FREDHIJACK_PATH to point to the directory "
                            "containing fredhijack.so."%
                            GS_FREDHIJACK_PATH)

def main_io_loop(b_skip_prompt=False):
    """Main I/O loop to get and handle user commands."""
    global g_source_script
    if not b_skip_prompt:
       # This is true typically on resume. gdb doesn't print the prompt when
       # resuming from a checkpoint, so we don't wait for it here.
       fredio.wait_for_prompt()
    interactive_debugger_setup()
    s_last_command = ""
    while True:
        try:
            # Get one user command (blocking):
            s_command = fredio.get_command()
            # Special case: user entered '\n' => execute last command again.
            if s_command == '':
                s_command = s_last_command
            dispatch_command(s_command)
            s_last_command = s_command
        except KeyboardInterrupt:
            fredio.signal_child(signal.SIGINT)

def main():
    """Program execution starts here."""
    global gs_resume_dir_path
    # Don't do anything if we can't find DMTCP.
    verify_critical_files_present()
    fred_setup()
    # Main input/output loop
    # skip the prompt waiting if we are resuming:
    b_skip_prompt = gs_resume_dir_path != None
    main_io_loop(b_skip_prompt)
    # If we get here, quit.
    fredutil.fred_quit(0)
    
if __name__ == '__main__':
    main()
