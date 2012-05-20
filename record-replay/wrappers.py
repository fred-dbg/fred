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
import re
import string

copyrightHdr = """ \
/****************************************************************************
 * Copyright (C) 2009, 2010, 2011, 2012 by Kapil Arya, Gene Cooperman,      *
 *                                     Tyler Denniston, and Ana-Maria Visan *
 * {kapil,gene,tyler,amvisan}@ccs.neu.edu                                   *
 *                                                                          *
 * This file is part of FReD.                                               *
 *                                                                          *
 * FReD is free software: you can redistribute it and/or modify             *
 * it under the terms of the GNU General Public License as published by     *
 * the Free Software Foundation, either version 3 of the License, or        *
 * (at your option) any later version.                                      *
 *                                                                          *
 * FReD is distributed in the hope that it will be useful,                  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 * GNU General Public License for more details.                             *
 *                                                                          *
 * You should have received a copy of the GNU General Public License        *
 * along with FReD.  If not, see <http://www.gnu.org/licenses/>.            *
 ****************************************************************************/

 """

miscWrappers = [
  ('void', 'empty', []),
  ('int', 'accept', [('int', 'sockfd'),
                     ('struct sockaddr*', 'addr'),
                     ('socklen_t*', 'addrlen')]),
  ('int', 'accept4', [('int', 'sockfd'),
                      ('struct sockaddr*', 'addr'),
                      ('socklen_t*', 'addrlen'),
                      ('int', 'flags')]),
  ('int', 'access', [('const char*', 'pathname'),
                     ('int', 'mode')]),
  ('int', 'bind', [('int', 'sockfd'),
                   ('const struct sockaddr*', 'my_addr'),
                   ('socklen_t', 'addrlen')]),
  ('void*', 'calloc', [('size_t', 'nmemb'),
                       ('size_t', 'size')]),
  ('int', 'chmod', [('const char*', 'path'),
                    ('mode_t', 'mode')]),
  ('int', 'chown', [('const char*', 'path'),
                    ('uid_t', 'owner'),
                    ('gid_t', 'group')]),
  ('int', 'close', [('int', 'fd')]),
  ('int', 'connect', [('int', 'sockfd'),
                      ('const struct sockaddr*', 'serv_addr'),
                      ('socklen_t', 'addrlen')]),
  ('int', 'dup', [('int', 'oldfd')]),
  ('int', 'dup2', [('int', 'oldfd'),
                   ('int', 'newfd')]),
  ('int', 'dup3', [('int', 'oldfd'),
                   ('int', 'newfd'),
                   ('int', 'flags')]),
  ('int', 'fcntl', [('int', 'fd'),
                    ('int', 'cmd'),
                    ('void*', 'arg')]),
  ('int', 'fchdir', [('int', 'fd')]),
  ('int', 'fdatasync', [('int', 'fd')]),
  ('char*', 'getcwd', [('char*', 'buf'),
                       ('size_t', 'size')]),
  ('int', 'gettimeofday', [('struct timeval*', 'tv'),
                           ('struct timezone*', 'tz')]),
  ('int', 'getpeername', [('int', 'sockfd'),
                          ('struct sockaddr*', 'addr'),
                          ('socklen_t*', 'addrlen')]),
  ('int', 'getsockname', [('int', 'sockfd'),
                          ('struct sockaddr*', 'addr'),
                          ('socklen_t*', 'addrlen')]),
  ('int', 'link', [('const char*', 'oldpath'),
                   ('const char*', 'newpath')]),
  ('int', 'symlink', [('const char*', 'oldpath'),
                      ('const char*', 'newpath')]),
  ('int', 'listen', [('int', 'sockfd'),
                     ('int', 'backlog')]),
  ('struct tm*', 'localtime_r', [('const time_t*', 'timep'),
                                 ('struct tm*', 'result')]),
  ('int', 'utime', [('const char*', 'filename'),
                    ('const struct utimbuf*', 'times')]),
  ('int', 'utimes', [('const char*', 'filename'),
                     ('const struct timeval*', 'times')]),
  ('int', 'lutimes', [('const char*', 'filename'),
                      ('const struct timeval', 'tv[2]')]),
  ('int', 'futimes', [('int', 'fd'),
                      ('const struct timeval', 'tv[2]')]),
  ('int', 'clock_getres', [('clockid_t', 'clk_id'),
                           ('struct timespec*', 'res')]),
  ('int', 'clock_gettime', [('clockid_t', 'clk_id'),
                            ('struct timespec*', 'tp')]),
  ('int', 'clock_settime', [('clockid_t', 'clk_id'),
                            ('const struct timespec*', 'tp')]),
  ('off_t', 'lseek', [('int', 'fd'),
                      ('off_t', 'offset'),
                      ('int', 'whence')]),
  ('off64_t', 'lseek64', [('int', 'fd'),
                          ('off64_t', 'offset'),
                          ('int', 'whence')]),
  ('loff_t', 'llseek', [('int', 'fd'),
                        ('loff_t', 'offset'),
                        ('int', 'whence')]),
  ('void*', 'malloc', [('size_t', 'size')]),

  ('void', 'free', [('void*', 'ptr')]),

  ('int', 'mkdir', [('const char*', 'pathname'),
                    ('mode_t', 'mode')]),
  ('int', 'mkstemp', [('char*', 'temp')]),
  ('void*', 'mmap', [('void*', 'addr'),
                     ('size_t', 'length'),
                     ('int', 'prot'),
                     ('int', 'flags'),
                     ('int', 'fd'),
                     ('off_t', 'offset')]),
  ('void*', 'mmap64', [('void*', 'addr'),
                       ('size_t', 'length'),
                       ('int', 'prot'),
                       ('int', 'flags'),
                       ('int', 'fd'),
                       ('off64_t', 'offset')]),
  ('int', 'munmap', [('void*', 'addr'),
                     ('size_t', 'length')]),
  ('void*', 'mremap', [('void*', 'old_address'),
                       ('size_t', 'old_size'),
                       ('size_t', 'new_size'),
                       ('int', 'flags'),
                       ('void*', 'new_address')]),
  ('int', 'open', [('const char*', 'pathname'),
                   ('int', 'flags'),
                   ('mode_t', 'mode')]),
  ('int', 'open64', [('const char*', 'pathname'),
                     ('int', 'flags'),
                     ('mode_t', 'mode')]),
  ('int', 'openat', [('int', 'dirfd'),
                     ('const char*', 'pathname'),
                     ('int', 'flags')]),
  ('ssize_t', 'pread', [('int', 'fd'),
                        ('void*', 'buf'),
                        ('size_t', 'count'),
                        ('off_t', 'offset')]),
  ('ssize_t', 'preadv', [('int', 'fd'),
                         ('const struct iovec*', 'iov'),
                         ('int', 'iovcnt'),
                         ('off_t', 'offset')]),
  ('ssize_t', 'pwrite', [('int', 'fd'),
                         ('const void*', 'buf'),
                         ('size_t', 'count'),
                         ('off_t', 'offset')]),
  ('ssize_t', 'pwritev', [('int', 'fd'),
                          ('const struct iovec*', 'iov'),
                          ('int', 'iovcnt'),
                          ('off_t', 'offset')]),
  ('int', 'pthread_rwlock_unlock', [('pthread_rwlock_t*', 'rwlock')]),
  ('int', 'pthread_rwlock_rdlock', [('pthread_rwlock_t*', 'rwlock')]),
  ('int', 'pthread_rwlock_wrlock', [('pthread_rwlock_t*', 'rwlock')]),
  ('int', 'pthread_create', [('pthread_t*', 'thread'),
                             ('const pthread_attr_t*', 'attr'),
                             ('pthread_start_routine_t', 'start_routine'),
                             ('void*', 'arg')]),
  ('int', 'pthread_detach', [('pthread_t', 'thread')]),
  ('void', 'pthread_exit', [('void*', 'value_ptr')]),
  ('int', 'pthread_join', [('pthread_t', 'thread'),
                           ('void**', 'value_ptr')]),
  ('int', 'pthread_kill', [('pthread_t', 'thread'),
                           ('int', 'sig')]),
  ('int', 'pthread_mutex_lock', [('pthread_mutex_t*', 'mutex')]),
  ('int', 'pthread_mutex_trylock', [('pthread_mutex_t*', 'mutex')]),
  ('int', 'pthread_mutex_unlock', [('pthread_mutex_t*', 'mutex')]),
  ('int', 'rand', []),
  ('ssize_t', 'read', [('int', 'fd'),
                       ('void*', 'buf'),
                       ('size_t', 'count')]),
  ('ssize_t', 'readv', [('int', 'fd'),
                        ('const struct iovec*', 'iov'),
                        ('int', 'iovcnt')]),
  ('ssize_t', 'readlink', [('const char*', 'path'),
                           ('char*', 'buf'),
                           ('size_t', 'bufsiz')]),
  ('char*', 'realpath', [('const char*', 'path'),
                         ('char*', 'resolved_path')]),
  ('void*', 'realloc', [('void*', 'ptr'),
                        ('size_t', 'size')]),
  ('int', 'rename', [('const char*', 'oldpath'),
                     ('const char*', 'newpath')]),
  ('int', 'rmdir', [('const char*', 'pathname')]),
  ('int', 'select', [('int', 'nfds'),
                     ('fd_set*', 'readfds'),
                     ('fd_set*', 'writefds'),
                     ('fd_set*', 'exceptfds'),
                     ('struct timeval*', 'timeout')]),
  ('int', 'ppoll', [('struct pollfd*', 'fds'),
                    ('nfds_t', 'nfds'),
                    ('const struct timespec*', 'timeout_ts'),
                    ('const sigset_t*', 'sigmask')]),
  ('int', 'setsockopt', [('int', 's'),
                         ('int', 'level'),
                         ('int', 'optname'),
                         ('const void*', 'optval'),
                         ('socklen_t', 'optlen')]),
  ('int', 'getsockopt', [('int', 's'),
                         ('int', 'level'),
                         ('int', 'optname'),
                         ('void*', 'optval'),
                         ('socklen_t*', 'optlen')]),
  ('int', 'ioctl', [('int', 'd'),
                    ('int', 'request'),
                    ('void*', 'arg')]),
  ('int', 'sigwait', [('const sigset_t*', 'set'),
                      ('int*', 'sig')]),
  ('void', 'srand', [('unsigned', 'int', 'seed')]),
  ('int', 'socket', [('int', 'domain'),
                     ('int', 'type'),
                     ('int', 'protocol')]),
  ('int', 'socketpair', [('int', 'd'),
                         ('int', 'type'),
                         ('int', 'protocol'),
                         ('int', 'sv[2]')]),
  ('time_t', 'time', [('time_t*', 'tloc')]),
  ('int', 'truncate', [('const char*', 'path'),
                       ('off_t', 'length')]),
  ('int', 'ftruncate', [('int', 'fd'),
                        ('off_t', 'length')]),
  ('int', 'truncate64', [('const char*', 'path'),
                         ('off64_t', 'length')]),
  ('int', 'ftruncate64', [('int', 'fd'),
                          ('off64_t', 'length')]),
  ('int', 'unlink', [('const char*', 'pathname')]),
  ('ssize_t', 'write', [('int', 'fd'),
                        ('const void*', 'buf'),
                        ('size_t', 'count')]),
  ('ssize_t', 'writev', [('int', 'fd'),
                         ('const struct iovec*', 'iov'),
                         ('int', 'iovcnt')]),
  ('int', 'epoll_create', [('int', 'size')]),
  ('int', 'epoll_create1', [('int', 'flags')]),
  ('int', 'epoll_ctl', [('int', 'epfd'),
                        ('int', 'op'),
                        ('int', 'fd'),
                        ('struct epoll_event*', 'e')]),
  ('int', 'epoll_wait', [('int', 'epfd'),
                         ('struct epoll_event*', 'events'),
                         ('int', 'maxevents'),
                         ('int', 'timeout')]),
  ('int', 'getpwnam_r', [('const char*', 'name'),
                         ('struct passwd*', 'pwd'),
                         ('char*', 'buf'),
                         ('size_t', 'buflen'),
                         ('struct passwd**', 'result')]),
  ('int', 'getpwuid_r', [('uid_t', 'uid'),
                         ('struct passwd*', 'pwd'),
                         ('char*', 'buf'),
                         ('size_t', 'buflen'),
                         ('struct passwd**', 'result')]),
  ('int', 'getgrnam_r', [('const char*', 'name'),
                         ('struct group*', 'grp'),
                         ('char*', 'buf'),
                         ('size_t', 'buflen'),
                         ('struct group**', 'result')]),
  ('int', 'getgrgid_r', [('gid_t', 'gid'),
                         ('struct group*', 'grp'),
                         ('char*', 'buf'),
                         ('size_t', 'buflen'),
                         ('struct group**', 'result')]),
  ('int', 'getaddrinfo', [('const char*', 'node'),
                          ('const char*', 'service'),
                          ('const struct addrinfo*', 'hints'),
                          ('struct addrinfo**', 'res')]),
  ('void', 'freeaddrinfo', [('struct addrinfo*', 'res')]),
  ('int', 'getnameinfo', [('const struct sockaddr*', 'sa'),
                          ('socklen_t', 'salen'),
                          ('char*', 'host'),
                          ('size_t', 'hostlen'),
                          ('char*', 'serv'),
                          ('size_t', 'servlen'),
                          ('int', 'flags')]),
  ('ssize_t', 'sendto', [('int', 'sockfd'),
                         ('const void*', 'buf'),
                         ('size_t', 'len'),
                         ('int', 'flags'),
                         ('const struct sockaddr*', 'dest_addr'),
                         ('socklen_t', 'addrlen')]),
  ('ssize_t', 'sendmsg', [('int', 'sockfd'),
                          ('const struct msghdr*', 'msg'),
                          ('int', 'flags')]),
  ('ssize_t', 'recvfrom', [('int', 'sockfd'),
                           ('void*', 'buf'),
                           ('size_t', 'len'),
                           ('int', 'flags'),
                           ('struct sockaddr*', 'src_addr'),
                           ('socklen_t*', 'addrlen')]),
  ('ssize_t', 'recvmsg', [('int', 'sockfd'),
                          ('struct msghdr*', 'msg'),
                          ('int', 'flags')]),
  ('int', 'waitid', [('idtype_t', 'idtype'),
                     ('id_t', 'id'),
                     ('siginfo_t*', 'infop'),
                     ('int', 'options')]),
  ('pid_t', 'wait4', [('pid_t', 'pid'),
                      ('__WAIT_STATUS', 'status'),
                      ('int', 'options'),
                      ('struct rusage*', 'rusage')]),
  ('int', 'sigaction', [('int', 'signum'),
                        ('const struct sigaction*', 'act'),
                        ('struct sigaction*', 'oldact')]),
  ('sighandler_t', 'signal', [('int', 'signum'),
                              ('sighandler_t', 'handler')]),
  ('sighandler_t', 'sigset', [('int', 'sig'),
                              ('sighandler_t', 'disp')]),
]

fstreamWrappers = [

  ('FILE*', 'fopen', [('const char*', 'path'),
                      ('const char*', 'mode')]),

  ('FILE*', 'fopen64', [('const char*', 'path'),
                        ('const char*', 'mode')]),

  ('FILE*', 'freopen', [('const char*', 'path'),
                        ('const char*', 'mode'),
                        ('FILE*', 'stream')]),
  ('int', 'fclose', [('FILE*', 'fp')]),

  ('FILE*', 'fdopen', [('int', 'fd'),
                       ('const char*', 'mode')]),

  ('char*', 'fgets', [('char*', 's'),
                      ('int', 'size'),
                      ('FILE*', 'stream')]),

  ('int', 'ferror', [('FILE*', 'stream')]),

  ('int', 'feof', [('FILE*', 'stream')]),

  ('int', 'fileno', [('FILE*', 'stream')]),

  ('int', 'fflush', [('FILE*', 'stream')]),

  ('int', 'setvbuf', [('FILE*', 'stream'),
                      ('char*', 'buf'),
                      ('int', 'mode'),
                      ('size_t', 'size')]),

  ('int', 'fseek', [('FILE*', 'stream'),
                    ('long', 'offset'),
                    ('int', 'whence')]),

  ('int', 'fputs', [('const char*', 's'),
                    ('FILE*', 'stream')]),

  ('int', 'fputc', [('int', 'c'),
                    ('FILE*', 'stream')]),

  ('int', 'fsync', [('int', 'fd')]),

  ('long', 'ftell', [('FILE*', 'stream')]),

  ('size_t', 'fwrite', [('const void*', 'ptr'),
                        ('size_t', 'size'),
                        ('size_t', 'nmemb'),
                        ('FILE*', 'stream')]),

  ('size_t', 'fread', [('void*', 'ptr'),
                       ('size_t', 'size'),
                       ('size_t', 'nmemb'),
                       ('FILE*', 'stream')]),

  ('int', 'getc', [('FILE*', 'stream')]),

  ('int', 'fgetc', [('FILE*', 'stream')]),

  ('int', 'ungetc', [('int', 'c'),
                     ('FILE*', 'stream')]),

  ('ssize_t', 'getline', [('char**', 'lineptr'),
                          ('size_t*', 'n'),
                          ('FILE*', 'stream')]),

  ('ssize_t', 'getdelim', [('char**', 'lineptr'),
                           ('size_t*', 'n'),
                           ('int', 'delim'),
                           ('FILE*', 'stream')]),

  ('int', 'putc', [('int', 'c'),
                   ('FILE*', 'stream')]),

  ('void', 'rewind', [('FILE*', 'stream')]),

  ('FILE*', 'tmpfile', []),

  ('void', 'flockfile', [('FILE*', 'filehandle')]),

  ('int', 'ftrylockfile', [('FILE*', 'filehandle')]),

  ('void', 'funlockfile', [('FILE*', 'filehandle')]),

  ('int', 'closedir', [('DIR*', 'dirp')]),

  ('DIR*', 'opendir', [('const char*', 'name')]),

  ('DIR*', 'fdopendir', [('int', 'fd')]),

  ('struct dirent*', 'readdir', [('DIR*', 'dirp')]),

  ('int', 'readdir_r', [('DIR*', 'dirp'),
                        ('struct dirent*', 'entry'),
                        ('struct dirent**', 'result')]),

]

#REACH_RECORD_REPLAY_WRAPPER_2('MACRO')]),
pthreadCondWrappers = [
  ('int', 'pthread_cond_broadcast', [('pthread_cond_t*', 'cond')]),
  ('int', 'pthread_cond_signal', [('pthread_cond_t*', 'cond')]),
  ('int', 'pthread_cond_wait', [('pthread_cond_t*', 'cond'),
                                ('pthread_mutex_t*', 'mutex')]),
  ('int', 'pthread_cond_timedwait', [('pthread_cond_t*', 'cond'),
                                     ('pthread_mutex_t*', 'mutex'),
                                     ('const struct timespec*', 'abstime')]),
  ('int', 'pthread_cond_destroy', [('pthread_cond_t*', 'cond')]),
]


#REACH_RECORD_REPLAY_WRAPPER_3('MACRO')]),
xstatWrappers = [
  ('int', 'fxstat', [('int', 'vers'),
                     ('int', 'fd'),
                     ('struct stat*', 'buf')]),
  ('int', 'fxstat64', [('int', 'vers'),
                       ('int', 'fd'),
                       ('struct stat64*', 'buf')]),
  ('int', 'lxstat', [('int', 'vers'),
                     ('const char*', 'path'),
                     ('struct stat*', 'buf')]),
  ('int', 'lxstat64', [('int', 'vers'),
                       ('const char*', 'path'),
                       ('struct stat64*', 'buf')]),
  ('int', 'xstat', [('int', 'vers'),
                    ('const char*', 'path'),
                    ('struct stat*', 'buf')]),
  ('int', 'xstat64', [('int', 'vers'),
                      ('const char*', 'path'),
                      ('struct stat64*', 'buf')]),
  ('void*', 'libc_memalign', [('size_t', 'boundary'),
                              ('size_t', 'size')]),
]


#REACH_RECORD_REPLAY_WRAPPER_4('MACRO')]),
printfScanfWrappers = [
  ('int', 'fprintf', [('FILE*', 'stream'),
                      ('const char*', 'format'),
                      ('va_list', 'ap')]),
  ('int', 'fscanf', [('FILE*', 'stream'),
                     ('const char*', 'format'),
                     ('va_list', 'ap')]),
]

noSyscallWrappers = [
  #REACH_RECORD_REPLAY_WRAPPER_5('MACRO')]),
  ('void', 'exec_barrier', []),
  ('void', 'signal_handler', [('int', 'sig'),
                              ('siginfo_t*', 'info'),
                              ('void*', 'data')]),
  ('void', 'user', []),
]

#REACH_NON_RECORD_REPLAY_WRAPPER('MACRO')]),
syscallWrapper = [
  ('long int', 'syscall', [('int', 'num'),
                           ('void*', 'a1'),
                           ('void*', 'a2'),
                           ('void*', 'a3'),
                           ('void*', 'a4'),
                           ('void*', 'a5'),
                           ('void*', 'a6'),
                           ('void*', 'a7')]),
]

wrapperGroups = [miscWrappers,
                 fstreamWrappers,
                 pthreadCondWrappers,
                 xstatWrappers,
                 printfScanfWrappers,
                 noSyscallWrappers,
                 syscallWrapper
                ]

def get_wrapper_enum(wrapper):
    return 'enum_' + wrapper[1]

def get_wrapper_enum_list():
    enumList=[]
    for wrapperGroup in wrapperGroups:
        enumList += map(get_wrapper_enum, wrapperGroup)
    return enumList


def get_arg_decl(a):
    return string.join(a)

def get_real_XXX_decl():
    res = []
    for wrapperGroup in wrapperGroups:
        for wrapper in wrapperGroup:
            argList = map(get_arg_decl, wrapper[2])
            argDecl = string.join(argList, ', ')
            res += ['%s _real_%s(%s);' % (wrapper[0], wrapper[1], argDecl)]
    return res

def get_real_XXX_addrs():
    res = []
    for wrapper in miscWrappers + fstreamWrappers:
        res += ['\t_real_func_addr[%s] = _real_dlsym(RTLD_NEXT, %s);'
                % (get_wrapper_enum(wrapper), wrapper[1])];
    for wrapper in pthreadCondWrappers:
        res += ['\t_real_func_addr[%s] = dlvsym(RTLD_NEXT, %s, "GLIBC_2.3.2");'
                % (get_wrapper_enum(wrapper), wrapper[1])];
    for wrapper in xstatWrappers:
        res += ['\t_real_func_addr[%s] = _real_dlsym(RTLD_NEXT, __%s);'
                % (get_wrapper_enum(wrapper), wrapper[1])];
    return res


enumList = get_wrapper_enum_list() + ['numTotalWrappers']
realDecl = get_real_XXX_decl()
realAddrs = get_real_XXX_addrs()


############################## 
# Generate fred_wrappers.h
############################## 

fred_wrappers_h_header = """
#ifndef FRED_WRAPPERS_H
#define FRED_WRAPPERS_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <utime.h>
#include <sys/time.h>
#include <stdarg.h>
#include <asm/ldt.h>
#include <stdio.h>
#include <thread_db.h>
#include <sys/procfs.h>
#include <syslog.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/epoll.h>
#include <dirent.h>
#include <unistd.h>
#include <poll.h>
#include <pwd.h>
#include <grp.h>
#include <netdb.h>

#include "constants.h"

#define LIB_PRIVATE __attribute__ ((visibility ("hidden")))

#ifdef __cplusplus
extern "C"
{
#endif

  extern int fred_wrappers_initializing;
  void _dmtcp_setup_trampolines();
  void initialize_wrappers();

#if __GLIBC_PREREQ(2,5)
# define READLINK_RET_TYPE ssize_t
#else
# define READLINK_RET_TYPE int
#endif

  void *fred_mmap(void *addr, size_t length, int prot, int flags,
                  int fd, off_t offset);
  void *fred_mremap(void *old_address, size_t old_size,
                    size_t new_size, int flags, ...);
  int fred_munmap(void *addr, size_t length);

"""

fred_wrappers_h_footer = """
#ifdef __cplusplus
}
#endif

#endif
"""

fred_wrappers_h_enum_str = '  typedef enum {\n    %s\n  } LibcWrapperOffset;\n\n' \
                           % (string.join(enumList, ',\n    '))

fred_wrappers_h = open('fred_wrappers2.h', 'w')
fred_wrappers_h.write(copyrightHdr)
fred_wrappers_h.write(fred_wrappers_h_header)
fred_wrappers_h.write(fred_wrappers_h_enum_str)
fred_wrappers_h.write('  ')
fred_wrappers_h.write(string.join(realDecl, '\n  '))
fred_wrappers_h.write(fred_wrappers_h_footer)
