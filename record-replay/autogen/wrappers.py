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

import pdb
import re
import string
import textwrap

# Information on adding new wrappers/wrapper-groups:
#
# "wrapperGroups" := <list of wrapper-group-tuples>
# wrapper-group-tuple := (<wrapper-group>, <group-name>)
# wrapper-group := <list of wrapper-tuples>
# wrapper-tuple := ('<type>', '<name>', <argument-list> [, <opt-wrapper-info>]*)
# argument-list := <list of argument-tuples>
# argument-tuple := ('<type>', '<name>' [,<argument-flag>]*)
# argument-flag := '__save_retval' | '__no_save'
# opt-wrapper-info := ('opt', <optional-flags>) | ('extra', <extra-fields-for-struct>)
# optional-flags := 'decl_data_offset' | 'decl_retval'
# extra-fields-for-struct := '<type> <name>'
#
# Meaning of flags:
#  '__save_retval' : decl a ret_XXX field which saves the return value of this arg
#  '__no_save' : do not include this arg in log entry struct
#  'decl_data_offset' : add a 'off_t data_offset' field to the log entry struct
#  'decl_retval' : add a '<ret-type> <name__retval' field to the log entry struct

# Wrappers that don't require any special treatment. Most new wrappers should
# fit in this group
miscWrappers = [
  ('void', 'empty', []),
  ('int', 'accept', [('int', 'sockfd'),
                     ('struct sockaddr*', 'addr', '__save_retval'),
                     ('socklen_t*', 'addrlen', '__save_retval')]),
  ('int', 'accept4', [('int', 'sockfd'),
                      ('struct sockaddr*', 'addr', '__save_retval'),
                      ('socklen_t*', 'addrlen', '__save_retval'),
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
  # FIXME: fcntl needs more values (use readlog?)
  ('int', 'fcntl', [('int', 'fd'),
                    ('int', 'cmd'),
                    ('void*', 'arg')],
                   ('extra', 'struct flock ret_flock'),
                   ('opt', 'decl_fred_xxx_fn')),
  ('int', 'fchdir', [('int', 'fd')]),
  ('int', 'fdatasync', [('int', 'fd')]),
  ('char*', 'getcwd', [('char*', 'buf'),
                       ('size_t', 'size')],
                      ('opt', 'decl_data_offset')),
  ('int', 'gettimeofday', [('struct timeval*', 'tv', '__save_retval'),
                           ('struct timezone*', 'tz', '__save_retval')],
                          ('opt', 'decl_fred_xxx_fn')),
  ('int', 'getpeername', [('int', 'sockfd'),
                          ('struct sockaddr*', 'addr', '__save_retval'),
                          ('socklen_t*', 'addrlen', '__save_retval')]),
  ('int', 'getsockname', [('int', 'sockfd'),
                          ('struct sockaddr*', 'addr', '__save_retval'),
                          ('socklen_t*', 'addrlen', '__save_retval')]),
  ('int', 'link', [('const char*', 'oldpath'),
                   ('const char*', 'newpath')]),
  ('int', 'symlink', [('const char*', 'oldpath'),
                      ('const char*', 'newpath')]),
  ('int', 'listen', [('int', 'sockfd'),
                     ('int', 'backlog')]),
  ('struct tm*', 'localtime_r', [('const time_t*', 'timep'),
                                 ('struct tm*', 'result', '__save_retval')]),
  ('int', 'utime', [('const char*', 'filename'),
                    ('const struct utimbuf*', 'times')]),
  ('int', 'utimes', [('const char*', 'filename'),
                     ('const struct timeval*', 'times')]),
  ('int', 'lutimes', [('const char*', 'filename'),
                      ('const struct timeval*', 'tv')]),
  ('int', 'futimes', [('int', 'fd'),
                      ('const struct timeval*', 'tv')]),
  ('int', 'clock_getres', [('clockid_t', 'clk_id'),
                           ('struct timespec*', 'res', '__save_retval')]),
  ('int', 'clock_gettime', [('clockid_t', 'clk_id'),
                            ('struct timespec*', 'tp', '__save_retval')]),
  ('int', 'clock_settime', [('clockid_t', 'clk_id'),
                            ('const struct timespec*', 'tp')]),
  ('off_t', 'lseek', [('int', 'fd'),
                      ('off_t', 'offset'),
                      ('int', 'whence')],
                      ('opt', 'decl_fred_xxx_fn')),
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
                     ('off_t', 'offset')],
                    ('opt', 'decl_data_offset'),
                    ('opt', 'decl_fred_xxx_fn')),
  ('void*', 'mmap64', [('void*', 'addr'),
                       ('size_t', 'length'),
                       ('int', 'prot'),
                       ('int', 'flags'),
                       ('int', 'fd'),
                       ('off64_t', 'offset')],
                      ('opt', 'decl_data_offset')),
  ('int', 'munmap', [('void*', 'addr'),
                     ('size_t', 'length')],
                     ('opt', 'decl_fred_xxx_fn')),
  ('void*', 'mremap', [('void*', 'old_address'),
                       ('size_t', 'old_size'),
                       ('size_t', 'new_size'),
                       ('int', 'flags'),
                       ('void*', 'new_address')],
                       ('opt', 'decl_fred_xxx_fn')),
  ('int', 'open', [('const char*', 'pathname'),
                   ('int', 'flags'),
                   ('mode_t', 'mode')],
                  ('opt', 'decl_fred_xxx_fn')),
  ('int', 'open64', [('const char*', 'pathname'),
                     ('int', 'flags'),
                     ('mode_t', 'mode')]),
  ('int', 'openat', [('int', 'dirfd'),
                     ('const char*', 'pathname'),
                     ('int', 'flags')]),
  ('ssize_t', 'pread', [('int', 'fd'),
                        ('void*', 'buf'),
                        ('size_t', 'count'),
                        ('off_t', 'offset')],
                       ('opt', 'decl_data_offset')),
  ('ssize_t', 'preadv', [('int', 'fd'),
                         ('const struct iovec*', 'iov'),
                         ('int', 'iovcnt'),
                         ('off_t', 'offset')],
                        ('opt', 'decl_data_offset')),
  ('ssize_t', 'pwrite', [('int', 'fd'),
                         ('const void*', 'buf'),
                         ('size_t', 'count'),
                         ('off_t', 'offset')]),
  ('ssize_t', 'pwritev', [('int', 'fd'),
                          ('const struct iovec*', 'iov'),
                          ('int', 'iovcnt'),
                          ('off_t', 'offset')]),
  ('int', 'pthread_rwlock_unlock', [('pthread_rwlock_t*', 'rwlock', '__save_retval')]),
  ('int', 'pthread_rwlock_rdlock', [('pthread_rwlock_t*', 'rwlock', '__save_retval')]),
  ('int', 'pthread_rwlock_wrlock', [('pthread_rwlock_t*', 'rwlock', '__save_retval')]),
  ('int', 'pthread_create', [('pthread_t*', 'thread'),
                             ('const pthread_attr_t*', 'attr'),
                             ('pthread_start_routine_t', 'start_routine'),
                             ('void*', 'arg')],
                            ('extra', 'void *stack_addr'),
                            ('extra', 'size_t stack_size')),
  ('int', 'pthread_detach', [('pthread_t', 'thread')]),
  ('void', 'pthread_exit', [('void*', 'value_ptr')]),
  ('int', 'pthread_join', [('pthread_t', 'thread'),
                           ('void**', 'value_ptr')]),
  ('int', 'pthread_kill', [('pthread_t', 'thread'),
                           ('int', 'sig')]),
  ('int', 'pthread_mutex_lock', [('pthread_mutex_t*', 'mutex', '__save_retval')]),
  ('int', 'pthread_mutex_trylock', [('pthread_mutex_t*', 'mutex', '__save_retval')]),
  ('int', 'pthread_mutex_unlock', [('pthread_mutex_t*', 'mutex', '__save_retval')]),
  ('int', 'rand', []),
  ('ssize_t', 'read', [('int', 'fd'),
                       ('void*', 'buf'),
                       ('size_t', 'count')],
                      ('opt', 'decl_data_offset'),
                      ('opt', 'decl_fred_xxx_fn')),
  ('ssize_t', 'readv', [('int', 'fd'),
                        ('const struct iovec*', 'iov'),
                        ('int', 'iovcnt')],
                       ('opt', 'decl_data_offset')),
  ('ssize_t', 'readlink', [('const char*', 'path'),
                           ('char*', 'buf'),
                           ('size_t', 'bufsiz')],
                          ('opt', 'decl_data_offset')),
  ('char*', 'realpath', [('const char*', 'path'),
                         ('char*', 'resolved_path')],
                        ('opt', 'decl_data_offset'),
                        ('extra', 'size_t len')),
  ('void*', 'realloc', [('void*', 'ptr'),
                        ('size_t', 'size')]),
  ('int', 'rename', [('const char*', 'oldpath'),
                     ('const char*', 'newpath')]),
  ('int', 'rmdir', [('const char*', 'pathname')]),
  ('int', 'select', [('int', 'nfds'),
                     ('fd_set*', 'readfds', '__save_retval'),
                     ('fd_set*', 'writefds', '__save_retval'),
                     ('fd_set*', 'exceptfds'),
                     ('struct timeval*', 'timeout')]),
  ('int', 'ppoll', [('struct pollfd*', 'fds'),
                    ('nfds_t', 'nfds'),
                    ('const struct timespec*', 'timeout_ts'),
                    ('const sigset_t*', 'sigmask')],
                   ('opt', 'decl_data_offset')),
  ('int', 'setsockopt', [('int', 's'),
                         ('int', 'level'),
                         ('int', 'optname'),
                         ('const void*', 'optval'),
                         ('socklen_t', 'optlen')]),
  ('int', 'getsockopt', [('int', 's'),
                         ('int', 'level'),
                         ('int', 'optname'),
                         ('void*', 'optval'),
                         ('socklen_t*', 'optlen', '__save_retval')],
                        ('opt', 'decl_data_offset')),
  ('int', 'ioctl', [('int', 'd'),
                    ('int', 'request'),
                    ('void*', 'arg')],
                   ('opt', 'decl_data_offset'),
                   ('extra', 'struct winsize win_val'),
                   ('extra', 'struct ifconf ifconf_val'),
                   ('extra', 'int fionread_val')),
  ('int', 'sigwait', [('const sigset_t*', 'set'),
                      ('int*', 'sig', '__save_retval')]),
  ('void', 'srand', [('unsigned int', 'seed')]),
  ('int', 'socket', [('int', 'domain'),
                     ('int', 'type'),
                     ('int', 'protocol')]),
  # FIXME: ret_sv??
  ('int', 'socketpair', [('int', 'd'),
                         ('int', 'type'),
                         ('int', 'protocol'),
                         ('int*', 'sv')],
                        ('extra', 'int ret_sv[2]')),
  ('time_t', 'time', [('time_t*', 'tloc', '__save_retval')]),
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
                        ('size_t', 'count')],
                        ('opt', 'decl_fred_xxx_fn')),
  ('ssize_t', 'writev', [('int', 'fd'),
                         ('const struct iovec*', 'iov'),
                         ('int', 'iovcnt')]),
  ('int', 'epoll_create', [('int', 'size')]),
  ('int', 'epoll_create1', [('int', 'flags')]),
  ('int', 'epoll_ctl', [('int', 'epfd'),
                        ('int', 'op'),
                        ('int', 'fd'),
                        ('struct epoll_event*', 'ep')]),
  ('int', 'epoll_wait', [('int', 'epfd'),
                         ('struct epoll_event*', 'events'),
                         ('int', 'maxevents'),
                         ('int', 'timeout')],
                        ('opt', 'decl_data_offset')),
  ('int', 'getpwnam_r', [('const char*', 'name'),
                         ('struct passwd*', 'pwd', '__save_retval'),
                         ('char*', 'buf'),
                         ('size_t', 'buflen'),
                         ('struct passwd**', 'result', '__save_retval')],
                        ('opt', 'decl_data_offset')),
  ('int', 'getpwuid_r', [('uid_t', 'uid'),
                         ('struct passwd*', 'pwd', '__save_retval'),
                         ('char*', 'buf'),
                         ('size_t', 'buflen'),
                         ('struct passwd**', 'result', '__save_retval')],
                        ('opt', 'decl_data_offset')),
  ('int', 'getgrnam_r', [('const char*', 'name'),
                         ('struct group*', 'grp', '__save_retval'),
                         ('char*', 'buf'),
                         ('size_t', 'buflen'),
                         ('struct group**', 'result', '__save_retval')],
                        ('opt', 'decl_data_offset')),
  ('int', 'getgrgid_r', [('gid_t', 'gid'),
                         ('struct group*', 'grp', '__save_retval'),
                         ('char*', 'buf'),
                         ('size_t', 'buflen'),
                         ('struct group**', 'result', '__save_retval')],
                        ('opt', 'decl_data_offset')),
  ('int', 'getaddrinfo', [('const char*', 'node'),
                          ('const char*', 'service'),
                          ('const struct addrinfo*', 'hints'),
                          ('struct addrinfo**', 'res', '__save_retval')],
                        ('opt', 'decl_data_offset'),
                        ('extra', 'int num_results')),
  ('void', 'freeaddrinfo', [('struct addrinfo*', 'res')]),
  ('int', 'getnameinfo', [('const struct sockaddr*', 'sa'),
                          ('socklen_t', 'salen'),
                          ('char*', 'host'),
                          ('size_t', 'hostlen'),
                          ('char*', 'serv'),
                          ('size_t', 'servlen'),
                          ('int', 'flags')],
                        ('opt', 'decl_data_offset'),
                        ('extra', 'char ret_host[NI_MAXHOST]'),
                        ('extra', 'char ret_serv[NI_MAXSERV]')),
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
                           ('struct sockaddr*', 'src_addr', '__save_retval'),
                           ('socklen_t*', 'addrlen', '__save_retval')],
                          ('opt', 'decl_data_offset')),
  ('ssize_t', 'recvmsg', [('int', 'sockfd'),
                          ('struct msghdr*', 'msg', '__save_retval'),
                          ('int', 'flags')],
                         ('opt', 'decl_data_offset'),
                         ('extra', 'off_t control_buf_offset')),
  ('int', 'waitid', [('idtype_t', 'idtype'),
                     ('id_t', 'id'),
                     ('siginfo_t*', 'infop', '__save_retval'),
                     ('int', 'options')]),
  ('pid_t', 'wait4', [('pid_t', 'pid'),
                      ('__WAIT_STATUS', 'status', '__save_retval'),
                      ('int', 'options'),
                      ('struct rusage*', 'rusage', '__save_retval')]),
  ('int', 'sigaction', [('int', 'signum'),
                        ('const struct sigaction*', 'act'),
                        ('struct sigaction*', 'oldact')]),
  ('sighandler_t', 'signal', [('int', 'signum'),
                              ('sighandler_t', 'handler')]),
  ('sighandler_t', 'sigset', [('int', 'sig'),
                              ('sighandler_t', 'disp')]),
]

# Wrappers that operate on file/dir streams.
fstreamWrappers = [

  ('FILE*', 'fopen', [('const char*', 'path'),
                      ('const char*', 'mode')],
                     ('opt', 'decl_retval')),

  ('FILE*', 'fopen64', [('const char*', 'path'),
                        ('const char*', 'mode')],
                       ('opt', 'decl_retval')),

  ('FILE*', 'freopen', [('const char*', 'path'),
                        ('const char*', 'mode'),
                        ('FILE*', 'stream')],
                       ('opt', 'decl_retval')),
  ('int', 'fclose', [('FILE*', 'fp')]),

  ('FILE*', 'fdopen', [('int', 'fd'),
                       ('const char*', 'mode')],
                      ('opt', 'decl_retval')),

  ('char*', 'fgets', [('char*', 's'),
                      ('int', 'size'),
                      ('FILE*', 'stream')],
                     ('opt', 'decl_data_offset')),

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

  ('int', 'fgetpos', [('FILE*', 'stream'),
                      ('fpos_t*', 'pos')]),

  ('int', 'fgetpos64', [('FILE*', 'stream'),
                        ('fpos64_t*', 'pos')]),

  ('int', 'fsetpos', [('FILE*', 'stream'),
                      ('const fpos_t*', 'pos')]),

  ('int', 'fsetpos64', [('FILE*', 'stream'),
                        ('const fpos64_t*', 'pos')]),

  ('size_t', 'fwrite', [('const void*', 'ptr'),
                        ('size_t', 'size'),
                        ('size_t', 'nmemb'),
                        ('FILE*', 'stream')]),

  ('size_t', 'fread', [('void*', 'ptr'),
                       ('size_t', 'size'),
                       ('size_t', 'nmemb'),
                       ('FILE*', 'stream')],
                      ('opt', 'decl_data_offset')),

  ('int', 'getc', [('FILE*', 'stream')]),

  ('int', 'fgetc', [('FILE*', 'stream')]),

  ('int', 'ungetc', [('int', 'c'),
                     ('FILE*', 'stream')]),

  ('ssize_t', 'getline', [('char**', 'lineptr', '__save_retval'),
                          ('size_t*', 'n', '__save_retval'),
                          ('FILE*', 'stream')],
                         ('opt', 'decl_data_offset')),

  ('ssize_t', 'getdelim', [('char**', 'lineptr', '__save_retval'),
                           ('size_t*', 'n', '__save_retval'),
                           ('int', 'delim'),
                           ('FILE*', 'stream')],
                          ('opt', 'decl_data_offset')),

  ('int', 'putc', [('int', 'c'),
                   ('FILE*', 'stream')]),

  ('void', 'rewind', [('FILE*', 'stream')]),

  ('FILE*', 'tmpfile', [], ('opt', 'decl_retval')),

  ('void', 'flockfile', [('FILE*', 'filehandle')]),

  ('int', 'ftrylockfile', [('FILE*', 'filehandle')]),

  ('void', 'funlockfile', [('FILE*', 'filehandle')]),

  ('int', 'closedir', [('DIR*', 'dirp')]),

  ('DIR*', 'opendir', [('const char*', 'name')]),

  ('DIR*', 'fdopendir', [('int', 'fd')]),

  ('struct dirent*', 'readdir', [('DIR*', 'dirp')], ('opt', 'decl_retval')),

  ('int', 'readdir_r', [('DIR*', 'dirp'),
                        ('struct dirent*', 'entry', '__save_retval'),
                        ('struct dirent**', 'result', '__save_retval')]),

]

# pthread_cond_* wrappers require special treatment as libc and libpthread both have different (and non-compatible) implementations.
pthreadCondWrappers = [
  ('int', 'pthread_cond_broadcast', [('pthread_cond_t*', 'cond', '__save_retval')]),
  ('int', 'pthread_cond_signal', [('pthread_cond_t*', 'cond', '__save_retval')]),
  ('int', 'pthread_cond_wait', [('pthread_cond_t*', 'cond', '__save_retval'),
                                ('pthread_mutex_t*', 'mutex', '__save_retval')]),
  ('int', 'pthread_cond_timedwait', [('pthread_cond_t*', 'cond', '__save_retval'),
                                     ('pthread_mutex_t*', 'mutex', '__save_retval'),
                                     ('const struct timespec*', 'abstime')]),
  ('int', 'pthread_cond_destroy', [('pthread_cond_t*', 'cond', '__save_retval')]),
]


# stat/lstat/fstat functions are MACRO wrappers around the underlying xstat/lxstat/fxstat functions. libc defines them with a leading '__'.
xstatWrappers = [
  ('int', 'fxstat', [('int', 'vers'),
                     ('int', 'fd'),
                     ('struct stat*', 'buf', '__save_retval')],
                     ('opt', 'decl_fred_xxx_fn')),
  ('int', 'fxstat64', [('int', 'vers'),
                       ('int', 'fd'),
                       ('struct stat64*', 'buf', '__save_retval')]),
  ('int', 'lxstat', [('int', 'vers'),
                     ('const char*', 'path'),
                     ('struct stat*', 'buf', '__save_retval')],
                     ('opt', 'decl_fred_xxx_fn')),
  ('int', 'lxstat64', [('int', 'vers'),
                       ('const char*', 'path'),
                       ('struct stat64*', 'buf', '__save_retval')]),
  ('int', 'xstat', [('int', 'vers'),
                    ('const char*', 'path'),
                    ('struct stat*', 'buf', '__save_retval')],
                    ('opt', 'decl_fred_xxx_fn')),
  ('int', 'xstat64', [('int', 'vers'),
                      ('const char*', 'path'),
                      ('struct stat64*', 'buf', '__save_retval')]),
  ('void*', 'libc_memalign', [('size_t', 'boundary'),
                              ('size_t', 'size')]),
]


# printf and scanf wrappers require special treatment
printfScanfWrappers = [
  ('int', 'vfprintf', [('FILE*', 'stream'),
                      ('const char*', 'format'),
                      ('va_list', 'ap', '__no_save')]),
  ('int', 'vfscanf', [('FILE*', 'stream'),
                     ('const char*', 'format'),
                     ('va_list', 'ap', '__no_save')],
                    ('opt', 'decl_data_offset'),
                    ('extra', 'int bytes')),
]

# These wrappers are used internally by fred. They do not have any
# libc/libpthread implementation.
nonSyscallWrappers = [
  ('void', 'exec_barrier', []),
  ('void', 'signal_handler', [('int', 'sig'),
                              ('siginfo_t*', 'info'),
                              ('void*', 'data')]),
  ('void', 'user', []),
]

# syscall wrappers requires special treatment.
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

# All of our code should only use wrapperGroups. There should be no need to use
# any wrapper group directly.
wrapperGroups = [(miscWrappers, 'misc'),
                 (fstreamWrappers, 'fstream'),
                 (pthreadCondWrappers, 'pthreadCond'),
                 (xstatWrappers, 'xstat'),
                 (printfScanfWrappers, 'printfScanf'),
                 (nonSyscallWrappers, 'noSyscall'),
                 (syscallWrapper, 'syscall'),
                ]

class RetType:
    """ Information about return type"""

    def __init__(self, rtype):
        self.ret_type_str = rtype.strip()

    def __str__(self):
        return self.ret_type_str

    def format_str(self):
        format_map = {'int' : '%d',
                      'uid_t' : '%d',
                      'gid_t' : '%d',
                      'pid_t' : '%d',
                      'off_t' : '%d',
                      'loff_t' : '%lu',
                      'off64_t' : '%l',
                      'unsigned' : '%u',
                      'unsigned int' : '%u',
                      'long' : '%ld',
                      'long int' : '%ld',
                      'unsigned long' : '%ul',
                      'unsigned long int' : '%ul',
                      'long long' : '%ll',
                      'long long int' : '%ll',
                      'unsigned long long' : '%ull',
                      'unsigned long long int' : '%ull',
                      'void*' : '%p',
                      'const void*' : '%p',
                      'char*' : '%p',
                      'const char*' : '%p',
                      'size_t' : '%zu',
                      'ssize_t' : '%z',
                      'socklen_t' : '%u',
                      'clockid_t' : '%u',
                      'mode_t' : '%u',
                      'idtype_t' : '%u',
                      'id_t' : '%u',
                      'mode_t' : '%u',
                      '__WAIT_STATUS' : '%d',
                      'pthread_start_routine_t' : '%p',
                      'sighandler_t' : '%p',
                      'nfds_t' : '%p',
                      'pthread_t' : '%p'
                      }
        fmt = format_map.get(self.ret_type_str)
        if fmt == None:
            if self.ret_type_str[-1] != '*':
                print self.ret_type_str
            fmt = format_map.get('void*')
        return fmt

    def deref_type(self):
        if self.ret_type_str[-1] == '*':
            return self.ret_type_str[:-1]
        if self.ret_type_str == '__WAIT_STATUS':
            return 'void*'

        print 'Error: Given type can not be dereferenced: ' + self.ret_type_str
        raise
        return None

                      
class ArgInfo:
    """Information about each wrapper argument"""

    def __init__(self, atuple):
        if len(atuple) < 2:
            print 'Error: Invalid Argument format'
            return
        self._ret_type = RetType(atuple[0])
        self._name = atuple[1]
        self._save_retval = '__save_retval' in atuple[2:]
        self._dont_save = '__no_save' in atuple[2:]

    def __str__(self):
        return self._name

    def arg_decl(self):
        return str(self._ret_type) + ' ' + self._name

    def name(self):
        return self._name

    def save_retval(self):
        return self._save_retval

    def dont_save(self):
        return self._dont_save

    def ret_type(self):
        return self._ret_type

    def deref_type(self):
        return self._ret_type.deref_type()


class WrapperInfo:
    """Complete information about a wrapper function"""

    def __init__(self, wtuple, groupName):
        if len(wtuple) < 3:
            print "Error: Invalid format"
            print wtuple
            return
        self.ret_type = RetType(wtuple[0])
        self._name = wtuple[1]
        self.args = []
        self.decl_data_offset = False
        self.decl_retval = False
        self.extra_fields_for_log_entry_struct = []
        self.groupName = groupName

        for arg in wtuple[2]:
            self.args += [ArgInfo(arg)]

        for info in wtuple[3:]:
            if len(info) < 2:
                print 'Error: Invalid optional information format'
                break
            if info[0] == 'opt':
                if info[1] == 'decl_data_offset':
                    self.decl_data_offset = True
                if info[1] == 'decl_retval':
                    self.decl_retval = True
            if info[0] == 'extra':
                self.extra_fields_for_log_entry_struct += [info[1]]

    def __str__(self):
        return self._name

    def name(self):
        return self._name

    def enum(self):
        return self._name + "_event"

    def groupName(self):
        return self.groupName

    def get_arg_signature(self):
        arg_sign = []
        for arg in self.args:
            arg_sign += [arg.arg_decl()]
        return string.join(arg_sign, ', ')

    def get_real_XXX_decl(self):
        res = []
        if self._name == 'syscall':
            return 'long int _real_syscall(long int sys_num, ...);'
        return '%s _real_%s(%s);' % (self.ret_type, self._name,
                                     self.get_arg_signature())
    def get_fred_XXX_decl(self):
        res = []
        if self._name == 'syscall':
            return 'long int _real_syscall(long int sys_num, ...);'
        return '%s _real_%s(%s);' % (self.ret_type, self._name,
                                     self.get_arg_signature())

    def get_create_entry_fn(self):
        sign = 'log_entry_t create_%s_entry(' % (self._name)
        slen = len(sign)
        sign += 'clone_id_t clone_id, event_code_t event'
        if len(self.args) > 0:
            sign += ',\n%s%s' % (' ' * slen, self.get_arg_signature())
        sign += ')'

        body =  '\n{\n'
        body += '  log_entry_t e = EMPTY_LOG_ENTRY;\n'
        body += '  setupCommonFields(&e, clone_id, event);\n'

        for arg in self.args:
            if not arg.dont_save():
                body += '  SET_FIELD(e, %s, %s);\n' % (self._name, arg.name())

        body += '  return e;\n}\n'
        return (sign, body)

    def get_turn_check_p_fn(self):
        sign = 'int %s_turn_check(log_entry_t *e1, log_entry_t *e2)' % (self._name)
        body = '\n{\n  return base_turn_check(e1,e2)'
        for arg in self.args:
            body += '\n    && '
            body += 'ARE_FIELDS_EQUAL_PTR (e1, e2, %s, %s)'  % (self._name, arg.name())
        body += ';\n}\n'
        return (sign, body)
    
    def get_struct_def(self):
        ret = 'typedef struct {\n'
        for arg in self.args:
            ret += '  %s;\n' % (arg.arg_decl())
            if arg.save_retval():
                ret += '  %s %s;\n' % (arg.deref_type(), 'ret_' + arg.name())
        if self.decl_data_offset:
            ret += '  off_t data_offset;\n'
        if self.decl_retval:
            ret += '  %s %s_retval;\n' % (self.ret_type.deref_type(), self._name)
        for extra in self.extra_fields_for_log_entry_struct:
            ret += '  %s;\n' % (extra)
    
        ret += '} log_event_%s_t;\n' % (self._name)
        return ret

    def get_print_entry_fn(self):
        sign =  'void print_log_entry_%s' % (self._name)
        sign += '(int idx, log_entry_t *entry)'
        body =  ' {\n'
        body += '  printf("'
        fmt_str = ''
        arg_list = ''
        for arg in self.args:
            if arg.dont_save() == False:
                fmt_str += ' %s=%s' % (arg.name(), arg.ret_type().format_str())
                arg_list += ',\n         GET_FIELD_PTR(entry, %s, %s)' \
                         % (self._name, arg.name())
        body = '  {\n  printf("%s\\n" %s);\n}\n' % (fmt_str, arg_list)
        return (sign, body)

copyrightHdr = """\
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

/* This file is auto-generated. Please modify wrappers.py and re-execute it
 * to make any changes to this file.
 */

"""

def gen_wrapper_util_cpp(allWrappers):
    header = textwrap.dedent("""\
        #include "wrapper_util.h"
        #include "synchronizationlogging.h"

        """)

    log_event_size_start = 'static size_t log_event_size[numTotalWrappers] = {\n'
    log_event_size_end = textwrap.dedent("""\

        };

        size_t getLogEventSize(const log_entry_t *entry) {
          return log_event_size[entry->header.event];
        }
        """)

    setup_common_fields = textwrap.dedent("""\
        static void setupCommonFields(log_entry_t *e, clone_id_t clone_id,
                                      event_code_t event)
        {
          // Zero out all fields:
          memset(&(e->header), 0, sizeof(e->header));
          SET_COMMON_PTR(e, clone_id);
          SET_COMMON_PTR(e, event);
          SET_COMMON_PTR2(e, log_offset, INVALID_LOG_OFFSET);
        }
        """)

    base_turn_check = textwrap.dedent("""\
        static int base_turn_check(log_entry_t *e1, log_entry_t *e2) {
          // Predicate function for a basic check -- event # and clone id.
          return GET_COMMON_PTR(e1,clone_id) == GET_COMMON_PTR(e2,clone_id) &&
                 GET_COMMON_PTR(e1,event) == GET_COMMON_PTR(e2,event);
        }
        """)

    event_size = []
    create_entry_fn = []
    turn_check_p_fn = []
    for winfo in allWrappers:
        event_size += ['  sizeof(log_event_%s_t),' % (winfo.name())]
        (create_entry_sign, create_entry_body) = winfo.get_create_entry_fn()
        create_entry_fn += [create_entry_sign + create_entry_body]
        (turn_check_p_sign, turn_check_p_body) = winfo.get_turn_check_p_fn()
        turn_check_p_fn += [turn_check_p_sign + turn_check_p_body]

    fd = open('wrapper_util.cpp', 'w')
    fd.write(copyrightHdr)
    fd.write(header)

    fd.write(log_event_size_start)
    fd.write(string.join(event_size, '\n'))
    fd.write(log_event_size_end)

    fd.write(setup_common_fields + '\n')
    fd.write(string.join(create_entry_fn, '\n'))
    fd.write(base_turn_check + '\n')
    fd.write(string.join(turn_check_p_fn, '\n'))

    fd.close()


def gen_fred_read_log_h(allWrappers):
    header = textwrap.dedent("""\
        #include "synchronizationlogging.h"

        void print_log_entry_common(int idx, log_entry_t *entry);

        """)

    print_entry_start = textwrap.dedent("""\
        void printEntry(int idx, log_entry_t *entry)
        {
          print_log_entry_common(idx, entry);
          switch (entry->header.event) {
          """)

    print_entry_end = '  }\n}\n'
    log_event_str = 'static const char *log_event_str[] = {\n'

    print_log_entry = []
    print_entry = []
    for winfo in allWrappers:
        (print_str_sign, print_str_body) = winfo.get_print_entry_fn()
        print_log_entry += [print_str_sign + print_str_body]

        print_entry += ['    case %s: print_log_entry_%s(idx, entry); break;\n' \
                        % (winfo.enum(), winfo.name())]

        log_event_str += '  "%s",\n' % (winfo.name())

    fd = open('fred_read_log.h', 'w')
    fd.write(copyrightHdr)
    fd.write(header)

    log_event_str += '};\n\n'
    fd.write(log_event_str)

    fd.write(string.join(print_log_entry, '\n'))

    fd.write('\n')
    fd.write(print_entry_start)
    fd.write(string.join(print_entry, '\n'))
    fd.write(print_entry_end)

    fd.close()

############################## 
# Generate fred_wrappers.h
############################## 

def gen_fred_wrappers_raw_h(allWrappers):
    header = textwrap.dedent("""\
        #ifndef FRED_WRAPPERS_H
        # error "Never use <fred_wrappers_raw.h> directly;" \\
                "include <fred_wrappers.h> instead."
        #endif

        #ifdef __cplusplus
        extern "C"
        {
        #endif

        """)

    enumList = []
    realDeclList = []
    for winfo in allWrappers:
        enumList += [winfo.enum()]
        realDeclList += [winfo.get_real_XXX_decl()]

    enumList += ['numTotalWrappers']
    out = '  typedef enum {\n    %s\n  } event_code_t;\n\n' \
                               % (string.join(enumList, ',\n    '))
    out += '  ' + string.join(realDeclList, ';\n  ')

    footer = '\n\n#ifdef __cplusplus\n}\n#endif'

    fd = open('fred_wrappers_raw.h', 'w')
    fd.write(copyrightHdr)
    fd.write(header)
    fd.write(out)
    fd.write(footer)
    fd.close()

def gen_syscallsreal_helper_c(allWrappers):
    header = textwrap.dedent("""\
        #include "fred_wrappers.h"

        void * _real_dlsym(void *handle, const char *symbol);
        extern LIB_PRIVATE void *_real_func_addr[];

        """)

    libc_func_addr = []
    for wInfo in allWrappers:
        if wInfo.groupName == 'pthreadCond':
            s = '_real_func_addr[%s] = dlvsym(RTLD_NEXT, "%s", "GLIBC_2.3.2");' \
                               % (wInfo.enum(), wInfo.name())
        elif wInfo.groupName == 'xstat':
            s = '_real_func_addr[%s] = _real_dlsym(RTLD_NEXT, "__%s");' \
                               % (wInfo.enum(), wInfo.name())
        else:
            s = '_real_func_addr[%s] = _real_dlsym(RTLD_NEXT, "%s");' \
                               % (wInfo.enum(), wInfo.name())
        libc_func_addr += [s]

    out = 'LIB_PRIVATE void fred_get_libc_func_addr() {\n  %s\n}' \
            % (string.join(libc_func_addr, '\n  '))

    fd = open('syscallsreal_helper.c', 'w')
    fd.write(copyrightHdr)
    fd.write(header)
    fd.write(out)
    fd.close()

def gen_wrapper_util_h(allWrappers):
    header = textwrap.dedent("""\
        #ifndef WRAPPER_UTIL_H
        #define WRAPPER_UTIL_H

        #include "fred_wrappers.h"

        #ifdef __cplusplus
        extern "C"
        {
        #endif

        """)

    log_entry_decl = textwrap.dedent("""\
        typedef struct {
          event_code_t event;
          unsigned char isOptional;
          log_off_t log_offset;
          clone_id_t clone_id;
          int my_errno;
          void* retval;
        } log_entry_header_t;

        typedef struct {
          // Shared among all events ("common area"):
          /* IMPORTANT: Adding new fields to the common area requires that you also
           * update the log_event_common_size definition. */
          log_entry_header_t header;

          union log_entry_data edata;
        } log_entry_t;

        """)

    footer = textwrap.dedent("""\
        size_t getLogEventSize(const log_entry_t *entry);

        #ifdef __cplusplus
        }
        #endif
        #endif""")

    log_entry_union_start = 'union log_entry_data {\n  '
    log_entry_union_end = '\n};\n'

    event_size = []
    log_entry_union = []
    struct_def = []
    turn_check_p = []
    create_entry = []
    for wInfo in allWrappers:
        event_size += ['static const int log_event_%s_size = sizeof(log_event_%s_t);' \
                       % (wInfo.name(), wInfo.name())]
        log_entry_union += ['log_event_%s_t log_event_%s;' \
                            % (wInfo.name(), wInfo.name())]
        turn_check_p += ['int %s_turn_check(log_entry_t *e1, log_entry_t *e2);' \
                         % (wInfo.name())]
        (create_entry_sign, create_entry_body) = wInfo.get_create_entry_fn()
        create_entry += [create_entry_sign + ';']
        struct_def += [wInfo.get_struct_def()]

    fd = open('wrapper_util.h', 'w')
    fd.write(copyrightHdr)
    fd.write(header)

    #fd.write(string.join(event_size, '\n'))
    #fd.write('\n\n')

    fd.write(string.join(struct_def, '\n'))
    fd.write('\n\n')

    fd.write(log_entry_union_start)
    fd.write(string.join(log_entry_union, '\n  '))
    fd.write(log_entry_union_end)

    fd.write('\n\n')
    fd.write(log_entry_decl)
    fd.write('\n\n')

    fd.write(string.join(turn_check_p, '\n'))
    fd.write('\n\n')
    fd.write(string.join(create_entry, '\n'))
    fd.write('\n')


    fd.write(footer)
    fd.close()


def main():
    """Start hook"""
    allWrappers = []
    for (wrapperGroup, groupName) in wrapperGroups:
        for wrapper in wrapperGroup:
            allWrappers += [WrapperInfo(wrapper, groupName)]
    gen_fred_read_log_h(allWrappers)
    gen_wrapper_util_cpp(allWrappers)
    gen_fred_wrappers_raw_h(allWrappers)
    gen_syscallsreal_helper_c(allWrappers)
    gen_wrapper_util_h(allWrappers)
    return

if __name__ == '__main__':
    main()
