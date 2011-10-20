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

"""
This file should be executable from the command line to run several
integration and unit tests on FReD.
"""
from optparse import OptionParser
from random import randint
import os
import sys
import traceback

import fredapp
import fred.fredutil
import fred.dmtcpmanager
import fred.fredmanager
import fred.fredio

GS_PASSED_STRING = "Passed"
GS_FAILED_STRING = "Failed"
# XXX this path shouldn't be hardcoded.
GS_TEST_PROGRAMS_DIRECTORY = "fredtest/test-programs"

# Used for storing variable values between runs.
gd_stored_variables = {}
g_debugger = None
gs_dmtcp_port = ""
gb_fred_debug = False
gd_tests = {}
gn_coordinator_port = -1

"""
This file should be executable from the command line to run several
integration and unit tests on FReD.

To add a new test:

1) Define the test function. Follow the other functions
   (e.g. gdb_reverse_next()) as a template.
2) Add the new test function to the gd_tests dictionary in the
   initialize_tests() function.
3) Add call to test under run_integration_tests().
"""


def start_session(l_cmd):
    """Start the given command line as a fred session."""
    global g_debugger
    g_debugger = fredapp.fred_setup_as_module(l_cmd, gs_dmtcp_port,
                                              gb_fred_debug)

def end_session():
    """End the current debugger session."""
    global g_debugger
    fred.dmtcpmanager.kill_peers()
    fred.fredmanager.destroy()
    fred.fredutil.fred_teardown()
    g_debugger.destroy()
    g_debugger = None

def execute_commands(l_cmds):
    """Execute the given list of commands as if they were a source file."""
    fredapp.source_from_list(l_cmds)

def evaluate_variable(s_name):
    """Evaluate given variable in debugger and return value."""
    global g_debugger
    return g_debugger.evaluate_expression(s_name)

def store_variable(s_name):
    """Evaluate given variable in debugger and store value.
    For example, if the test is for gdb, and the debugged program (a.out)
    defines some variable 'my_var', then store_variable("my_var") will execute
    gdb command "print my_var" and store the value of my_var in the
    gd_stored_variables dictionary under the key "my_var"."""
    global gd_stored_variables
    gd_stored_variables[s_name] = evaluate_variable(s_name)

def check_stored_variable(s_name):
    """Evaluate given variable in debugger and check against stored value."""
    global gd_stored_variables
    return gd_stored_variables[s_name] == evaluate_variable(s_name)

def check_variable(s_name, s_value):
    """Evaluate given variable in debugger and check against given value."""
    return s_value == str(evaluate_variable(s_name))

def print_test_name(s_name):
    print "%-40s | " % s_name,
    sys.stdout.flush()

def gdb_record_replay(n_count=1):
    """Run a test on deterministic record/replay on pthread-test example."""
    global GS_TEST_PROGRAMS_DIRECTORY
    l_cmd = ["gdb", GS_TEST_PROGRAMS_DIRECTORY + "/pthread-test"]
    for i in range(0, n_count):
        print_test_name("gdb record/replay %d" % i)
        start_session(l_cmd)
        execute_commands(["b main", "b print_solution", "r", "fred-ckpt", "c"])
        store_variable("solution")
        execute_commands(["fred-restart", "c"])
        if check_stored_variable("solution"):
            print GS_PASSED_STRING
        else:
            print GS_FAILED_STRING
        end_session()

def gdb_record_replay_pthread_cond(n_count=1):
    """Run a test on deterministic record/replay on pthread-cond-var example."""
    global GS_TEST_PROGRAMS_DIRECTORY
    l_cmd = ["gdb", GS_TEST_PROGRAMS_DIRECTORY + "/pthread-cond-var"]
    for i in range(0, n_count):
        print_test_name("gdb record/replay pthread_cond %d" % i)
        start_session(l_cmd)
        execute_commands(["b main", "b print_solution", "r", "fred-ckpt", "c"])
        store_variable("solution")
        execute_commands(["fred-restart", "c"])
        if check_stored_variable("solution"):
            print GS_PASSED_STRING
        else:
            print GS_FAILED_STRING
        end_session()

def gdb_record_replay_time(n_count=1):
    """Run a test on deterministic record/replay on time.c example."""
    global GS_TEST_PROGRAMS_DIRECTORY
    l_cmd = ["gdb", GS_TEST_PROGRAMS_DIRECTORY + "/time"]
    for i in range(0, n_count):
        print_test_name("gdb record/replay time %d" % i)
        start_session(l_cmd)
        execute_commands(["b main", "r", "fred-ckpt", "c"])
        execute_commands(["fred-restart", "c"])
        print GS_PASSED_STRING
        end_session()

def gdb_syscall_tester(n_count=1):
    """Run a test on deterministic record/replay on syscall-tester example."""
    global GS_TEST_PROGRAMS_DIRECTORY
    l_cmd = ["gdb", GS_TEST_PROGRAMS_DIRECTORY + "/syscall-tester"]
    for i in range(0, n_count):
        print_test_name("gdb syscall-tester %d" % i)
        start_session(l_cmd)
        execute_commands(["b main", "b 4619", "r", "fred-ckpt", "c"])
        store_variable("whole_test")
        execute_commands(["fred-restart", "c"])
        if check_stored_variable("whole_test"):
            print GS_PASSED_STRING
        else:
            print GS_FAILED_STRING
        end_session()

def gdb_reverse_watch(n_count=1):
    """Run a reverse-watch test on test_list linked list example."""
    global GS_TEST_PROGRAMS_DIRECTORY
    l_cmd = ["gdb", GS_TEST_PROGRAMS_DIRECTORY + "/test-list"]
    for i in range(0, n_count):
        print_test_name("gdb reverse watch %d" % i)
        start_session(l_cmd)
        execute_commands(["b main", "r", "fred-ckpt", "b 29",
                          "c", "fred-rw list_len(head) < 10"])
        if check_variable("list_len(head)", "9"):
            execute_commands(["n"])
            if check_variable("list_len(head)", "10"):
                print GS_PASSED_STRING
            else:
                print GS_FAILED_STRING
        else:
            print GS_FAILED_STRING
        end_session()

def gdb_reverse_watch_mt(n_count=1):
    """Run a multithreaded reverse-watch test on pthread_test example."""
    global GS_TEST_PROGRAMS_DIRECTORY
    l_cmd = ["gdb", GS_TEST_PROGRAMS_DIRECTORY + "/pthread-test"]
    for i in range(0, n_count):
        print_test_name("gdb reverse watch (MT) %d" % i)
        start_session(l_cmd)
        execute_commands(["b main", "r", "fred-ckpt", "b print_solution",
                          "c", "fred-rw solution < 100"])
        if int(evaluate_variable("solution")) < 100:
            execute_commands(["s"])
            if int(evaluate_variable("solution")) >= 100:
                print GS_PASSED_STRING
            else:
                print GS_FAILED_STRING
        else:
            print GS_FAILED_STRING
        end_session()

def gdb_reverse_watch_mt_priv(n_count=1):
    """Run a multithreaded reverse-watch test on
    pthread-test-thread-private, which contains multiple threads, but
    the expression is on a piece of data modified only by one thread
    (the main thread).."""
    global GS_TEST_PROGRAMS_DIRECTORY
    l_cmd = ["gdb", GS_TEST_PROGRAMS_DIRECTORY + "/pthread-test-thread-private"]
    for i in range(0, n_count):
        print_test_name("gdb reverse watch (MT/priv) %d" % i)
        start_session(l_cmd)
        execute_commands(["b main", "r", "fred-ckpt", "b print_solution",
                          "c", "fred-rw main_thread_private_data < 3"])
        if check_variable("main_thread_private_data", "2"):
            execute_commands(["s"])
            if check_variable("main_thread_private_data", "3"):
                print GS_PASSED_STRING
            else:
                print GS_FAILED_STRING
        else:
            print GS_FAILED_STRING
        end_session()

def gdb_reverse_watch_no_log(n_count=1):
    """Run a single threaded reverse-watch test on test-list-no-malloc
    which contains 0 threads and 0 log entries.."""
    global GS_TEST_PROGRAMS_DIRECTORY
    l_cmd = ["gdb", GS_TEST_PROGRAMS_DIRECTORY + "/test-list-no-malloc"]
    for i in range(0, n_count):
        print_test_name("gdb reverse watch (no log) %d" % i)
        start_session(l_cmd)
        execute_commands(["b main", "r", "fred-ckpt", "b finished",
                          "c", "fred-rw list_len() < 10"])
        if check_variable("list_len()", "9"):
            execute_commands(["n"])
            if check_variable("list_len()", "10"):
                print GS_PASSED_STRING
            else:
                print GS_FAILED_STRING
        else:
            print GS_FAILED_STRING
        end_session()

def gdb_reverse_next(n_count=1):
    """Run a reverse-next test on pthread-test."""
    global GS_TEST_PROGRAMS_DIRECTORY
    l_cmd = ["gdb", GS_TEST_PROGRAMS_DIRECTORY + "/pthread-test"]
    for i in range(0, n_count):
        print_test_name("gdb reverse next %d" % i)
        start_session(l_cmd)
        execute_commands(["b main", "r", "fred-ckpt", "b 85",
                          "c", "fred-reverse-next"])
        current_backtrace_frame = g_debugger.current_position()
        if current_backtrace_frame.line() == 84:
            print GS_PASSED_STRING
        else:
            print GS_FAILED_STRING
        end_session()

def gdb_reverse_step(n_count=1):
    """Run a reverse-step test on pthread-test."""
    global GS_TEST_PROGRAMS_DIRECTORY
    l_cmd = ["gdb", GS_TEST_PROGRAMS_DIRECTORY + "/pthread-test"]
    for i in range(0, n_count):
        print_test_name("gdb reverse step %d" % i)
        start_session(l_cmd)
        execute_commands(["b main", "r", "fred-ckpt", "b print_solution",
                          "c", "n", "n", "fred-reverse-step"])
        current_backtrace_frame = g_debugger.current_position()
        if current_backtrace_frame.line() == 63:
            print GS_PASSED_STRING
        else:
            print GS_FAILED_STRING
        end_session()

def gdb_reverse_continue(n_count=1):
    """Run a reverse-continue test on pthread-test."""
    global GS_TEST_PROGRAMS_DIRECTORY
    l_cmd = ["gdb", GS_TEST_PROGRAMS_DIRECTORY + "/pthread-test"]
    for i in range(0, n_count):
        print_test_name("gdb reverse continue %d" % i)
        start_session(l_cmd)
        execute_commands(["b main", "r", "fred-ckpt", "b print_solution",
                          "b 84", "c", "c", "c", "fred-rc"])
        current_backtrace_frame = g_debugger.current_position()
        if g_debugger.at_breakpoint() and current_backtrace_frame.line() == 84:
            print GS_PASSED_STRING
        else:
            print GS_FAILED_STRING
        end_session()

def gdb_reverse_finish(n_count=1):
    """Run a reverse-finish test on test-list."""
    global GS_TEST_PROGRAMS_DIRECTORY
    l_cmd = ["gdb", GS_TEST_PROGRAMS_DIRECTORY + "/test-list"]
    for i in range(0, n_count):
        print_test_name("gdb reverse finish %d" % i)
        start_session(l_cmd)
        execute_commands(["b main", "r", "fred-ckpt", "step",
                          "fred-reverse-finish"])
        current_backtrace_frame = g_debugger.current_position()
        if current_backtrace_frame.line() == 21:
            print GS_PASSED_STRING
        else:
            print GS_FAILED_STRING
        end_session()

def run_integration_tests():
    """Run all available integration tests."""
    gdb_record_replay()
    gdb_record_replay_pthread_cond()
    gdb_record_replay_time()
    gdb_syscall_tester()
    gdb_reverse_watch()
    gdb_reverse_next()
    gdb_reverse_step()
    gdb_reverse_continue()
    gdb_reverse_finish()
    gdb_reverse_watch_mt()
    gdb_reverse_watch_mt_priv()
    gdb_reverse_watch_no_log()
    
def run_unit_tests():
    """Run all available unit tests."""
    pass

def run_tests(ls_test_list):
    """Run given list of tests, or all tests if None."""
    global gd_tests
    print "%-40s | %-15s" % ("Test name", "Result")
    print "-" * 41 + "+" + "-" * 15
    # TODO: This is hackish. Used to hide fred_info() messages.
    fred.fredio.gb_hide_output = True
    if ls_test_list == None:
        run_integration_tests()
        run_unit_tests()
    else:
        for t in ls_test_list:
            try:
                gd_tests[t]()
            except KeyError:
                # If you've added a new test and are seeing this error message,
                # you probably forgot to update gd_tests in initialize_tests().
                print "Unknown test '%s'. Skipping." % t
                continue

def parse_fredtest_args():
    """Parse command line args, and return list of tests to run.
    Then set up fredapp module accordingly."""
    global gs_dmtcp_port, gb_fred_debug, gn_coordinator_port
    parser = OptionParser()
    parser.disable_interspersed_args()
    # Note that '-h' and '--help' are supported automatically.
    parser.add_option("-p", "--port", dest="dmtcp_port",
                      help="Use PORT for DMTCP port number. If unspecified, "
                      "starts a background coordinator on a random port.",
                      metavar="PORT")
    parser.add_option("--enable-debug", dest="debug", default=False,
                      action="store_true",
                      help="Enable FReD debugging messages.")
    parser.add_option("-l", "--list-tests", dest="list_tests", default=False,
                      action="store_true",
                      help="List available tests and exit.")
    parser.add_option("-t", "--tests", dest="test_list",
                      help="Comma delimited list of tests to run.")
    (options, l_args) = parser.parse_args()
    if options.list_tests:
        list_tests()
        sys.exit(1)
    if options.dmtcp_port == None:
        n_new_port = randint(2000,10000)
        gs_dmtcp_port = str(n_new_port)
        status = fred.dmtcpmanager.start_coordinator(n_new_port)
        fred.fredutil.fred_assert(status)
        gn_coordinator_port = n_new_port
    else:
        gs_dmtcp_port = str(options.dmtcp_port)
    gb_fred_debug = options.debug
    return options.test_list.split(",") if options.test_list != None else None

def list_tests():
    """Displays a list of all available tests."""
    global gd_tests
    print "Available tests:"
    for k in sorted(gd_tests.keys()):
        print k

def initialize_tests():
    """Initializes the list of known tests.
    This must be called before running any tests."""
    global gd_tests
    # When you add a new test, update this map from test name -> test fnc.
    gd_tests = { "gdb-record-replay" : gdb_record_replay,
                 "gdb-record-replay-time" : gdb_record_replay_time,
                 "gdb-record-replay-pthread-cond" : 
                 gdb_record_replay_pthread_cond,
                 "gdb-syscall-tester" : gdb_syscall_tester,
                 "gdb-reverse-watch" : gdb_reverse_watch,
                 "gdb-reverse-watch-mt" : gdb_reverse_watch_mt,
                 "gdb-reverse-watch-mt-priv" : gdb_reverse_watch_mt_priv,
                 "gdb-reverse-watch-no-log" : gdb_reverse_watch_no_log,
                 "gdb-reverse-next"  : gdb_reverse_next,
                 "gdb-reverse-step"  : gdb_reverse_step,
                 "gdb-reverse-continue"  : gdb_reverse_continue,
                 "gdb-reverse-finish"  : gdb_reverse_finish }

def main():
    """Program execution starts here."""
    global gn_coordinator_port
    # Don't do anything if we can't find DMTCP.
    fredapp.verify_critical_files_present()
    
    initialize_tests()

    ls_test_list = parse_fredtest_args()
    try:
        run_tests(ls_test_list)
    except:
        traceback.print_exc()

    if fred.dmtcpmanager.is_running():
        fred.dmtcpmanager.kill_peers()
    if gn_coordinator_port != -1:
        fred.dmtcpmanager.kill_coordinator(gn_coordinator_port)

if __name__ == "__main__":
    main()
