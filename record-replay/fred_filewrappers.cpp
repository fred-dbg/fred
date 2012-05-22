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

// TODO: Better way to do this. I think it was only a problem on dekaksi.
// Remove this, and see the compile error.
#define read _libc_read
#include <stdarg.h>
#include <stdlib.h>
#include <vector>
#include <list>
#include <string>
#include <fcntl.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/version.h>
#include <limits.h>
#include "constants.h"
#include "util.h"
#include  "jassert.h"
#include  "jfilesystem.h"

#include "fred_wrappers.h"
#include "synchronizationlogging.h"
#include <sys/mman.h>
#include <sys/syscall.h>
// TODO: hack to be able to compile this (fcntl wrapper).
#define open _libc_open
#define open64 _libc_open64
#define openat _libc_openat
#include <fcntl.h>
#undef open
#undef open64
#undef openat
#undef read

extern "C" int close ( int fd )
{
  BASIC_SYNC_WRAPPER(int, close, _real_close, fd);
}

extern "C" int fclose(FILE *fp)
{
  WRAPPER_HEADER(int, fclose, _real_fclose, fp);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_TYPED(int, fclose);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_fclose(fp);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int fchdir(int fd)
{
  BASIC_SYNC_WRAPPER(int, fchdir, _real_fchdir, fd);
}

extern "C" int open (const char *path, int flags, ... )
{
  mode_t mode = 0;
  // Handling the variable number of arguments
  if (flags & O_CREAT) {
    va_list arg;
    va_start (arg, flags);
    mode = va_arg (arg, int);
    va_end (arg);
  }
  BASIC_SYNC_WRAPPER(int, open, _real_open, path, flags, mode);
}

// FIXME: The 'fn64' version of functions is defined only when within
// __USE_LARGEFILE64 is #defined. The wrappers in this file need to consider
// this fact. The problem can occur, for example, when DMTCP is not compiled
// with __USE_LARGEFILE64 whereas the user-binary is. In that case the open64()
// call from user will come to DMTCP and DMTCP might fail to execute it
// properly.

// FIXME: Add the 'fn64' wrapper test cases to dmtcp test suite.
extern "C" int open64 (const char *path, int flags, ... )
{
  mode_t mode;
  // Handling the variable number of arguments
  if (flags & O_CREAT) {
    va_list arg;
    va_start (arg, flags);
    mode = va_arg (arg, int);
    va_end (arg);
  }
  BASIC_SYNC_WRAPPER(int, open64, _real_open64, path, flags, mode);
}

extern "C" FILE *fdopen(int fd, const char *mode)
{
  WRAPPER_HEADER(FILE*, fdopen, _real_fdopen, fd, mode);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(FILE*, fdopen);
    if (retval != NULL) {
      *retval = GET_FIELD(my_entry, fdopen, fdopen_retval);
    }
    WRAPPER_REPLAY_END(fdopen);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_fdopen(fd, mode);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    if (retval != NULL) {
      SET_FIELD2(my_entry, fdopen, fdopen_retval, *retval);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

#if 0
/* Until we fix the readdir() bug for tar, this is commented out.  If
   we don't comment this out (and fdopendir also), readdir() does not
   function properly in tar.  This is a "special case hack" for tar 1.26. */
// TODO: handle the variable argument here.
extern "C" int openat(int dirfd, const char *pathname, int flags, ...)
{
  BASIC_SYNC_WRAPPER(int, openat, _real_openat, dirfd, pathname, flags);
}
#endif

extern "C" DIR *opendir(const char *name)
{
  WRAPPER_HEADER(DIR*, opendir, _real_opendir, name);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_TYPED(DIR*, opendir);
    //TODO: May be we should restore data in *retval;
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_opendir(name);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

#if 0
/* Until we fix the readdir() bug for tar, this is commented out.  If
   we don't comment this out (and openat also), readdir() does not
   function properly in tar.  This is a "special case hack" for tar 1.26. */
extern "C" DIR *fdopendir(int fd)
{
  WRAPPER_HEADER(DIR*, fdopendir, _real_fdopendir, fd);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_TYPED(DIR*, fdopendir);
    //TODO: May be we should restore data in *retval;
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_fdopendir(fd);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}
#endif

extern "C" int closedir(DIR *dirp)
{
  WRAPPER_HEADER(int, closedir, _real_closedir, dirp);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY(closedir);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_closedir(dirp);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" ssize_t __getdelim(char **lineptr, size_t *n, int delim, FILE *stream)
{
  WRAPPER_HEADER(ssize_t, getdelim, _real_getdelim, lineptr, n, delim, stream);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(ssize_t, getdelim);
    if (retval != -1) {
      *lineptr = GET_FIELD(my_entry, getdelim, ret_lineptr);
      *n       = GET_FIELD(my_entry, getdelim, ret_n);
      WRAPPER_REPLAY_READ_FROM_READ_LOG(getdelim, *lineptr, *n);
    }
    WRAPPER_REPLAY_END(getdelim);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_getdelim(lineptr, n, delim, stream);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    if (retval != -1) {
      SET_FIELD2(my_entry, getdelim, ret_lineptr, *lineptr);
      SET_FIELD2(my_entry, getdelim, ret_n, *n);
      WRAPPER_LOG_WRITE_INTO_READ_LOG(getdelim, *lineptr, *n);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" ssize_t getdelim(char **lineptr, size_t *n, int delim, FILE *stream)
{
  dmtcp::ThreadInfo::setOkToLogNextFnc();
  return __getdelim(lineptr, n, delim, stream);
}

// WARNING:  Early versions of glibc (e.g. glibc 2.3) define this
//  function in stdio.h as inline.  This wrapper won't work in that case.
# if __GLIBC_PREREQ (2,4)
extern "C" ssize_t getline(char **lineptr, size_t *n, FILE *stream)
{
  WRAPPER_HEADER(ssize_t, getline, _real_getline, lineptr, n, stream);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(ssize_t, getline);
    if (retval != -1) {
      *lineptr = GET_FIELD(my_entry, getline, ret_lineptr);
      *n       = GET_FIELD(my_entry, getline, ret_n);
      WRAPPER_REPLAY_READ_FROM_READ_LOG(getline, *lineptr, *n);
    }
    WRAPPER_REPLAY_END(getline);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_getline(lineptr, n, stream);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    if (retval != -1) {
      SET_FIELD2(my_entry, getline, ret_lineptr, *lineptr);
      SET_FIELD2(my_entry, getline, ret_n, *n);
      WRAPPER_LOG_WRITE_INTO_READ_LOG(getline, *lineptr, *n);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}
# else
#  error getline() is already defined as inline in <stdio.h>.  Wrapper fails.
# endif

/* The list of strings: each string is a format, like for example %d or %lf.
 * This function deals with the following possible formats:
 * with or without whitespace delimited, eg: "%d%d" or "%d   %d".  */
static void parse_format (const char *format, dmtcp::list<dmtcp::string> *formats)
{
  int start = 0;
  size_t i;
  /* An argument format is delimited by expecting_start and expecting_end.
   * When expecting_start is true, that means we are about to begin a new
   * argument format. When expecting_end is true, we are expecting the
   * end of the argument format. expecting_start and expecting_end have
   * always opposite values. */
  bool expecting_start = true;
  bool expecting_end = false;
  char tmp[128];

  for ( i = 0; i < strlen(format); i++) {
    if (format[i] == '%') {
      if (expecting_end) {
        memset(tmp, 0, 128);
        memcpy(tmp, &format[start], i - start);
        formats->push_back(dmtcp::string(tmp));
        start = i;
      } else {
        start = i;
        expecting_end = true;
        expecting_start = false;
      }
      continue;
    }
    /* For formats like "%.2lf". */
    if (isdigit(format[i]) || format[i] == '.') continue;
    if (format[i] == ' ' || format[i] == '\t') {
      if (expecting_end) {
        expecting_end = false;
        expecting_start = true;
        memset(tmp, 0, 128);
        memcpy(tmp, &format[start], i - start);
        formats->push_back(dmtcp::string(tmp));
      }
      continue;
    }
  }
  /* This is for the last argument format in the list */
  if (!expecting_start && expecting_end) {
    memset(tmp, 0, 128);
    memcpy(tmp, &format[start], i - start);
    formats->push_back(dmtcp::string(tmp));
  }
}

/* For fscanf, for %5c like formats.
 * This function returns the number of characters read. */
static int get_how_many_characters (const char *str)
{
  /* The format has no integer conversion specifier, if the size of str is 2. */
  if (strlen(str) == 2) return 1;
  char tmp[512] = {'\0'};
  for (size_t i = 1; i < strlen(str) - 1; i++)
    tmp[i-1] = str[i];
  return atoi(tmp);
}

/* TODO: not all formats are mapped.
 * This function parses the given argument list and logs the values of the
 * arguments in the list to read_data_fd. Returns the number of bytes written. */
static int parse_va_list_and_log (va_list arg, const char *format)
{
  dmtcp::list<dmtcp::string> formats;
  parse_format (format, &formats);

  dmtcp::list<dmtcp::string>::iterator it;
  int bytes = 0;

  /* The list arg is made up of pointers to variables because the list arg
   * resulted as a call to fscanf. Thus we need to extract the address for
   * each argument and cast it to the corresponding type. */
  for (it = formats.begin(); it != formats.end(); it++) {
    /* Get next argument in the list. */
    long int *val = va_arg(arg, long int *);
    if (it->find("lf") != dmtcp::string::npos) {
      logReadData ((double *)val, sizeof(double));
      bytes += sizeof(double);
    }
    else if (it->find("d") != dmtcp::string::npos) {
      logReadData ((int *)val, sizeof(int));
      bytes += sizeof(int);
    }
    else if (it->find("c") != dmtcp::string::npos) {
      int nr_chars = get_how_many_characters(it->c_str());
      logReadData ((char *)val, nr_chars * sizeof(char));
      bytes += nr_chars * sizeof(char);
    }
    else if (it->find("s") != dmtcp::string::npos) {
      logReadData ((char *)val, strlen((char *)val)+ 1);
      bytes += strlen((char *)val) + 1;
    }
    else {
      JTRACE ("Format to add: ") (it->c_str());
      JASSERT (false).Text("format not added.");
    }
  }
  return bytes;
}

/* Parses the format string and reads into the given va_list of arguments.
  */
static void read_data_from_log_into_va_list (va_list arg, const char *format)
{
  dmtcp::list<dmtcp::string>::iterator it;
  dmtcp::list<dmtcp::string> formats;

  parse_format (format, &formats);
  /* The list arg is made up of pointers to variables because the list arg
   * resulted as a call to fscanf. Thus we need to extract the address for
   * each argument and cast it to the corresponding type. */
  for (it = formats.begin(); it != formats.end(); it++) {
    /* Get next argument in the list. */
    long int *val = va_arg(arg, long int *);
    if (it->find("lf") != dmtcp::string::npos) {
      _real_read(read_data_fd, (void *)val, sizeof(double));
    }
    else if (it->find("d") != dmtcp::string::npos) {
      _real_read(read_data_fd, (void *)val, sizeof(int));
    }
    else if (it->find("c") != dmtcp::string::npos) {
      int nr_chars = get_how_many_characters(it->c_str());
      _real_read(read_data_fd, (void *)val, nr_chars * sizeof(char));
    }
    else if (it->find("s") != dmtcp::string::npos) {
      bool terminate = false;
      int offset = 0;
      int i;
      char tmp[1024] = {'\0'};
      while (!terminate) {
        _real_read(read_data_fd, &tmp[offset], 128);
        for (i = 0; i < 128; i++) {
          if (tmp[i] == '\0') {
            terminate = true;
            break;
          }
        }
        if (!terminate) {
          offset += 128;
        }
      }
      /* We want to copy \0 at the end. */
      memcpy((void *)val, tmp, offset + i + 1);
      /* We want to be located one position to the right of \0. */
      _real_lseek(read_data_fd, i - 128 - 1, SEEK_CUR);
    }
    else {
      JASSERT (false).Text("format not added.");
    }
  }
}

/* fscanf seems to be #define'ed into this. */
extern "C" int __isoc99_fscanf (FILE *stream, const char *format, ...)
{
  va_list arg;
  va_start (arg, format);

  WRAPPER_HEADER(int, vfscanf, vfscanf, stream, format, arg);

  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(vfscanf);
    if (retval != EOF) {
      if (__builtin_expect(read_data_fd == -1, 0)) {
        read_data_fd = _real_open(RECORD_READ_DATA_LOG_PATH, O_RDONLY, 0);
      }
      JASSERT ( read_data_fd != -1 );
      lseek(read_data_fd, GET_FIELD(my_entry,vfscanf,data_offset), SEEK_SET);
      read_data_from_log_into_va_list (arg, format);
      va_end(arg);
    }
    WRAPPER_REPLAY_END(vfscanf);
  } else if (SYNC_IS_RECORD) {
    errno = 0;
    retval = vfscanf(stream, format, arg);
    int saved_errno = errno;
    va_end (arg);
    if (retval != EOF) {
      _real_pthread_mutex_lock(&read_data_mutex);
      SET_FIELD2(my_entry, vfscanf, data_offset, read_log_pos);
      va_start (arg, format);
      int bytes = parse_va_list_and_log(arg, format);
      va_end (arg);
      SET_FIELD(my_entry, vfscanf, bytes);
      _real_pthread_mutex_unlock(&read_data_mutex);
    }
    errno = saved_errno;
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

/* Here we borrow the data file used to store data returned from read() calls
   to store/replay the data for fgets() calls. */
extern "C" char *fgets(char *s, int size, FILE *stream)
{
  WRAPPER_HEADER(char *, fgets, _real_fgets, s, size, stream);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(char*, fgets);
    if (retval != NULL) {
      WRAPPER_REPLAY_READ_FROM_READ_LOG(fgets, s, size);
    }
    WRAPPER_REPLAY_END(fgets);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_fgets(s, size, stream);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    if (retval != NULL) {
      WRAPPER_LOG_WRITE_INTO_READ_LOG(fgets, s, size);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int ferror(FILE *stream)
{
  BASIC_SYNC_WRAPPER(int, ferror, _real_ferror, stream);
}

extern "C" int feof(FILE *stream)
{
  BASIC_SYNC_WRAPPER(int, feof, _real_feof, stream);
}

extern "C" int fileno(FILE *stream)
{
  BASIC_SYNC_WRAPPER(int, fileno, _real_fileno, stream);
}

static int _fprintf(FILE *stream, const char *format, va_list arg)
{
  return vfprintf(stream, format, arg);
}

/* TODO: I think fprintf() is an inline function, so we can't wrap it directly.
   fprintf() internally calls this function, which happens to be exported.
   So we wrap this and have it call vfprintf(). We should think carefully about
   this and determine whether it is an acceptable solution or not. */
extern "C" int __fprintf_chk (FILE *stream, int flag, const char *format, ...)
{
  va_list arg;
  va_start (arg, format);
  WRAPPER_HEADER(int, vfprintf, _fprintf, stream, format, arg);

  if (SYNC_IS_REPLAY) {
    // XXX: FIXME: why are we calling WRAPPER_REPLAY instead of
    // WRAPPER_REPLAY_START and _END?
    WRAPPER_REPLAY(vfprintf);
    /* If we're writing to stdout, we want to see the data to screen.
     * Thus execute the real system call. */
    // XXX We can't do this so easily. If we make the _real_printf call here,
    // it can call mmap() on replay at a different time as on record, since
    // the other FILE related syscalls are NOT made on replay.
    /*if (stream == stdout || stream == stderr) {
      retval = _fprintf(stream, format, arg);
      }*/
    retval = (int)(unsigned long)GET_COMMON(my_entry,
                                            retval);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _fprintf(stream, format, arg);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int fprintf (FILE *stream, const char *format, ...)
{
  va_list arg;
  va_start (arg, format);
  WRAPPER_HEADER(int, vfprintf, _fprintf, stream, format, arg);

  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(vfprintf);
    /* If we're writing to stdout, we want to see the data to screen.
     * Thus execute the real system call. */
    // XXX We can't do this so easily. If we make the _real_printf call here,
    // it can call mmap() on replay at a different time as on record, since
    // the other FILE related syscalls are NOT made on replay.
    /*if (stream == stdout || stream == stderr) {
      retval = _fprintf(stream, format, arg);
      }*/
    retval = (int)(unsigned long)GET_COMMON(my_entry, retval);
    WRAPPER_REPLAY_END(vfprintf);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _fprintf(stream, format, arg);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int fseek(FILE *stream, long offset, int whence)
{
  WRAPPER_HEADER(int, fseek, _real_fseek, stream, offset, whence);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_TYPED(int, fseek);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_fseek(stream, offset, whence);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int _IO_getc(FILE *stream)
{
  WRAPPER_HEADER(int, getc, _real_getc, stream);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_TYPED(int, getc);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_getc(stream);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int __uflow(FILE *stream)
{
  JASSERT(isProcessGDB()). Text("Not Implemented: *_unlocked fstream functions");
  dmtcp::ThreadInfo::setOkToLogNextFnc();
  return _IO_getc(stream);
}

extern "C" int fgetc(FILE *stream)
{
  WRAPPER_HEADER(int, fgetc, _real_fgetc, stream);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_TYPED(int, fgetc);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_fgetc(stream);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int ungetc(int c, FILE *stream)
{
  BASIC_SYNC_WRAPPER(int, ungetc, _real_ungetc, c, stream);
}

extern "C" char *getcwd(char *buf, size_t size)
{
  WRAPPER_HEADER(char *, getcwd, _real_getcwd, buf, size);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(char*, getcwd);
    if (retval != NULL) {
      /* retval will be a pointer to whatever buffer was used. If the
         user provided a NULL buffer, _real_getcwd allocated one on
         RECORD, but the optional event handler allocated it on REPLAY
         before we arrive here. Memory accurate replay allows us to
         depend on 'retval' pointing to the allocated buffer by the
         optional event handler. If the user provided a buffer, retval
         points to it. */
      WRAPPER_REPLAY_READ_FROM_READ_LOG(getcwd, retval, size);
    }
    WRAPPER_REPLAY_END(getcwd);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_getcwd(buf, size);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    if (retval != NULL) {
      /* retval will be a pointer to whatever buffer was used. If the
         user provided a NULL buffer, _real_getcwd will allocate one
         and retval points to it. If the user provided a buffer,
         retval points to it. */
      JASSERT(size > 0).Text("Unimplemented.");
      WRAPPER_LOG_WRITE_INTO_READ_LOG(getcwd, retval, size);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int fputs(const char *s, FILE *stream)
{
  WRAPPER_HEADER(int, fputs, _real_fputs, s, stream);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_TYPED(int, fputs);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_fputs(s, stream);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int fputc(int c, FILE *stream)
{
  WRAPPER_HEADER(int, fputc, _real_fputc, c, stream);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_TYPED(int, fputc);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_fputc(c, stream);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int _IO_putc(int c, FILE *stream)
{
  BASIC_SYNC_WRAPPER(int, putc, _real_putc, c, stream);
}

// WARNING:  Early versions of glibc (e.g. glibc 2.3) define this
//  function in stdio.h as inline.  This wrapper won't work in that case.
# if __GLIBC_PREREQ (2,4)
extern "C" int putchar(int c)
{
  return _IO_putc(c, stdout);
}
# else
#  error getline() is already defined as inline in <stdio.h>.  Wrapper fails.
# endif

extern "C" size_t fwrite(const void *ptr, size_t size, size_t nmemb,
    FILE *stream)
{
  WRAPPER_HEADER(size_t, fwrite, _real_fwrite, ptr, size, nmemb, stream);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_TYPED(size_t, fwrite);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_fwrite(ptr, size, nmemb, stream);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  WRAPPER_HEADER(size_t, fread, _real_fread, ptr, size, nmemb, stream);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(size_t, fread);
    if (retval != 0) {
      // fread() returns the number of items (NOT bytes) read.
      WRAPPER_REPLAY_READ_FROM_READ_LOG(fread, ptr, retval*size);
    }
    WRAPPER_REPLAY_END(fread);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_fread(ptr, size, nmemb, stream);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    if (retval != 0) {
      // fread() returns the number of items (NOT bytes) read.
      WRAPPER_LOG_WRITE_INTO_READ_LOG(fread, ptr, retval*size);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" void rewind(FILE *stream)
{
  BASIC_SYNC_WRAPPER_VOID(rewind, _real_rewind, stream);
}

extern "C" int truncate(const char *path, off_t length)
{
  BASIC_SYNC_WRAPPER(int, truncate, _real_truncate, path, length);
}

extern "C" int ftruncate(int fd, off_t length)
{
  BASIC_SYNC_WRAPPER(int, ftruncate, _real_ftruncate, fd, length);
}

extern "C" int truncate64(const char *path, off64_t length)
{
  BASIC_SYNC_WRAPPER(int, truncate64, _real_truncate64, path, length);
}

extern "C" int ftruncate64(int fd, off64_t length)
{
  BASIC_SYNC_WRAPPER(int, ftruncate64, _real_ftruncate64, fd, length);
}

extern "C" long ftell(FILE *stream)
{
  BASIC_SYNC_WRAPPER(long, ftell, _real_ftell, stream);
}

extern "C" int fgetpos(FILE *stream, fpos_t *pos)
{
  dmtcp::ThreadInfo::setOkToLogNextFnc();
  long res = ftell(stream);
  if (res != -1) {
    pos->__pos = (off_t) res;
  }
  return res;
}

extern "C" int fsetpos(FILE *stream, const fpos_t *pos)
{
  dmtcp::ThreadInfo::setOkToLogNextFnc();
  return fseek(stream, (long) pos->__pos, SEEK_SET);
}

extern "C" int fgetpos64(FILE *stream, fpos64_t *pos)
{
  dmtcp::ThreadInfo::setOkToLogNextFnc();
  long res = ftell(stream);
  if (res != -1) {
    pos->__pos = (off64_t) res;
  }
  return res;
}

extern "C" int fsetpos64(FILE *stream, const fpos64_t *pos)
{
  dmtcp::ThreadInfo::setOkToLogNextFnc();
  return fseek(stream, (long)pos->__pos, SEEK_SET);
}

extern "C" FILE *fopen (const char* path, const char* mode)
{
  WRAPPER_HEADER(FILE *, fopen, _real_fopen, path, mode);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(FILE*, fopen);
    if (retval != NULL) {
      *retval = GET_FIELD(my_entry, fopen, fopen_retval);
    }
    WRAPPER_REPLAY_END(fopen);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_fopen(path, mode);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    if (retval != NULL) {
      SET_FIELD2(my_entry, fopen, fopen_retval, *retval);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" FILE *fopen64 (const char* path, const char* mode)
{
  WRAPPER_HEADER(FILE *, fopen64, _real_fopen64, path, mode);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(FILE*, fopen64);
    if (retval != NULL) {
      *retval = GET_FIELD(my_entry, fopen64, fopen64_retval);
    }
    WRAPPER_REPLAY_END(fopen64);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_fopen64(path, mode);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    if (retval != NULL) {
      SET_FIELD2(my_entry, fopen64, fopen64_retval, *retval);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" FILE *freopen(const char* path, const char* mode, FILE *stream)
{
  WRAPPER_HEADER(FILE *, freopen, _real_freopen, path, mode, stream);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(FILE *, freopen);
    if (retval != NULL) {
      *retval = GET_FIELD(my_entry, freopen, freopen_retval);
    }
    WRAPPER_REPLAY_END(freopen);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_freopen(path, mode, stream);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    if (retval != NULL) {
      SET_FIELD2(my_entry, freopen, freopen_retval, *retval);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" FILE *tmpfile()
{
  WRAPPER_HEADER(FILE *, tmpfile, _real_tmpfile);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(FILE*, tmpfile);
    if (retval != NULL) {
      *retval = GET_FIELD(my_entry, tmpfile, tmpfile_retval);
    }
    WRAPPER_REPLAY_END(tmpfile);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_tmpfile();
    dmtcp::ThreadInfo::unsetOptionalEvent();
    if (retval != NULL) {
      SET_FIELD2(my_entry, tmpfile, tmpfile_retval, *retval);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int chmod(const char *path, mode_t mode)
{
  BASIC_SYNC_WRAPPER(int, chmod, _real_chmod, path, mode);
}

extern "C" int chown(const char *path, uid_t owner, gid_t group)
{
  BASIC_SYNC_WRAPPER(int, chown, _real_chown, path, owner, group);
}

#define _XSTAT_COMMON_SYNC_WRAPPER(name, ...)                               \
  do {                                                                      \
    if (SYNC_IS_REPLAY) {                                                   \
      WRAPPER_REPLAY_START(name);                                           \
      int saved_errno = GET_COMMON(my_entry, my_errno);                     \
      if (retval == 0 && buf != NULL) {                                     \
        *buf = GET_FIELD(my_entry, name, ret_buf);                          \
      }                                                                     \
      WRAPPER_REPLAY_END(name);                                             \
      if (saved_errno != 0) {                                               \
        errno = saved_errno;                                                \
      }                                                                     \
    } else if (SYNC_IS_RECORD) {                                            \
      retval = _real_ ## name(__VA_ARGS__);                                 \
      if (retval != -1 && buf != NULL) {                                    \
        SET_FIELD2(my_entry, name, ret_buf, *buf);                          \
      }                                                                     \
      WRAPPER_LOG_WRITE_ENTRY(my_entry);                                    \
    }                                                                       \
  }  while(0)

extern "C"
int __xstat(int vers, const char *path, struct stat *buf)
{
  WRAPPER_HEADER(int, xstat, _real_xstat, vers, path, buf);
  _XSTAT_COMMON_SYNC_WRAPPER(xstat, vers, path, buf);
  return retval;
}

extern "C"
int __xstat64(int vers, const char *path, struct stat64 *buf)
{
  WRAPPER_HEADER(int, xstat64, _real_xstat64, vers, path, buf);
  _XSTAT_COMMON_SYNC_WRAPPER(xstat64, vers, path, buf);
  return retval;
}

extern "C"
int __fxstat(int vers, int fd, struct stat *buf)
{
  WRAPPER_HEADER(int, fxstat, _real_fxstat, vers, fd, buf);
  _XSTAT_COMMON_SYNC_WRAPPER(fxstat, vers, fd, buf);
  return retval;
}

extern "C"
int __fxstat64(int vers, int fd, struct stat64 *buf)
{
  WRAPPER_HEADER(int, fxstat64, _real_fxstat64, vers, fd, buf);
  _XSTAT_COMMON_SYNC_WRAPPER(fxstat64, vers, fd, buf);
  return retval;
}

extern "C"
int __lxstat(int vers, const char *path, struct stat *buf)
{
  WRAPPER_HEADER(int, lxstat, _real_lxstat, vers, path, buf);
  _XSTAT_COMMON_SYNC_WRAPPER(lxstat, vers, path, buf);
  return retval;
}

extern "C"
int __lxstat64(int vers, const char *path, struct stat64 *buf)
{
  WRAPPER_HEADER(int, lxstat64, _real_lxstat64, vers, path, buf);
  _XSTAT_COMMON_SYNC_WRAPPER(lxstat64, vers, path, buf);
  return retval;
}

#if __GLIBC_PREREQ(2,5)
# define READLINK_RET_TYPE ssize_t
#else
# define READLINK_RET_TYPE int
#endif

extern "C" READLINK_RET_TYPE readlink(const char *path, char *buf,
                                      size_t bufsiz)
{
  WRAPPER_HEADER(READLINK_RET_TYPE, readlink, _real_readlink,
                 path, buf, bufsiz);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(READLINK_RET_TYPE, readlink);
    if (retval > 0 && buf != NULL) {
      WRAPPER_REPLAY_READ_FROM_READ_LOG(readlink, buf, retval);
    }
    WRAPPER_REPLAY_END(readlink);
  } else if (SYNC_IS_RECORD) {
    retval = _real_readlink(path, buf, bufsiz);
    if (retval > 0 && buf != NULL) {
      WRAPPER_LOG_WRITE_INTO_READ_LOG(readlink, buf, retval);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" char *realpath(const char *path, char *resolved_path)
{
  WRAPPER_HEADER(char *, realpath, _real_realpath, path, resolved_path);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(char*, realpath);
    if (retval != NULL) {
      int len = GET_FIELD(my_entry, realpath, len);
      WRAPPER_REPLAY_READ_FROM_READ_LOG(realpath, retval, len);
      // NULL-terminate as per man page.
      *(resolved_path + len) = '\0';
    }
    WRAPPER_REPLAY_END(realpath);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_realpath(path, resolved_path);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    if (retval != NULL) {
      int len = strlen(retval);
      SET_FIELD(my_entry, realpath, len);
      WRAPPER_LOG_WRITE_INTO_READ_LOG(realpath, retval, len);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int select(int nfds, fd_set *readfds, fd_set *writefds,
                      fd_set *exceptfds, struct timeval *timeout)
{
  WRAPPER_HEADER(int, select, _real_select, nfds, readfds, writefds, exceptfds, timeout);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(select);
    if (retval != -1) {
      copyFdSet(&GET_FIELD(my_entry, select, ret_readfds), readfds);
      copyFdSet(&GET_FIELD(my_entry, select, ret_writefds), writefds);
    }
    WRAPPER_REPLAY_END(select);
  } else if (SYNC_IS_RECORD) {
    retval = _real_select(nfds, readfds, writefds, exceptfds, timeout);
    int saved_errno = errno;
    if (retval != -1) {
      // Note that we're logging the *changed* fd set, so on replay we can
      // just read that from the log, load it into user's location and return.
      copyFdSet(readfds, &GET_FIELD(my_entry, select, ret_readfds));
      copyFdSet(writefds, &GET_FIELD(my_entry, select, ret_writefds));
    }
    errno = saved_errno;
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int ppoll(struct pollfd *fds, nfds_t nfds,
                     const struct timespec *timeout_ts,
                     const sigset_t *sigmask)
{
  WRAPPER_HEADER(int, ppoll, _real_ppoll, fds, nfds, timeout_ts, sigmask)
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(ppoll);
    if (retval > 0 && fds != NULL) {
      WRAPPER_REPLAY_READ_FROM_READ_LOG(ppoll, (void*)fds,
                                        nfds * sizeof(struct pollfd));
    }
    WRAPPER_REPLAY_END(ppoll);
  } else if (SYNC_IS_RECORD) {
    retval = _real_ppoll(fds, nfds, timeout_ts, sigmask);
    int saved_errno = errno;
    if (retval > 0 && fds != NULL) {
      WRAPPER_LOG_WRITE_INTO_READ_LOG(ppoll, (void*)fds,
                                      nfds * sizeof(struct pollfd));
    }
    errno = saved_errno;
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int poll(struct pollfd *fds, nfds_t nfds, int time)
{
  struct timespec time_ts;
  struct timespec *timeout_ts;

  if (time <= -1) {
    timeout_ts = NULL;
  } else {
    timeout_ts = &time_ts;
    timeout_ts->tv_sec = time/1000;
    timeout_ts->tv_nsec = (time % 1000) * 1000000;
  }
  dmtcp::ThreadInfo::setOkToLogNextFnc();
  return ppoll(fds, nfds, timeout_ts, NULL);
}

extern "C" ssize_t read(int fd, void *buf, size_t count)
{
  if (dmtcp_is_protected_fd(fd)) {
    return _real_read(fd, buf, count);
  }

  WRAPPER_HEADER(ssize_t, read, _real_read, fd, buf, count);

  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(ssize_t, read);
    // NOTE: We never actually call the user's _real_read. We don't
    // need to. We wait for the next event in the log that is the
    // READ_data_event, read from the read data log, and return the
    // corresponding value.
    if (retval > 0) {
      WRAPPER_REPLAY_READ_FROM_READ_LOG(read, buf, retval);
    }
    WRAPPER_REPLAY_END(read);
  } else if (SYNC_IS_RECORD) {
    // Note we don't call readAll here. It should be the responsibility of
    // the user code to handle EINTR if needed.
    retval = _real_read(fd, buf, count);
    if (retval > 0) {
      WRAPPER_LOG_WRITE_INTO_READ_LOG(read, buf, retval);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" ssize_t write(int fd, const void *buf, size_t count)
{
  if (dmtcp_is_protected_fd(fd)) {
    return _real_write(fd, buf, count);
  }
  BASIC_SYNC_WRAPPER(ssize_t, write, _real_write, fd, buf, count);
}

extern "C" ssize_t pread(int fd, void *buf, size_t count, off_t offset)
{
  if (dmtcp_is_protected_fd(fd)) {
    return _real_pread(fd, buf, count, offset);
  }
  WRAPPER_HEADER(ssize_t, pread, _real_pread, fd, buf, count, offset);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(ssize_t, pread);
    if (retval > 0) {
      WRAPPER_REPLAY_READ_FROM_READ_LOG(pread, buf, retval);
    }
    WRAPPER_REPLAY_END(pread);
  } else if (SYNC_IS_RECORD) {
    retval = _real_pread(fd, buf, count, offset);
    if (retval > 0) {
      WRAPPER_LOG_WRITE_INTO_READ_LOG(pread, buf, retval);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset)
{
  if (dmtcp_is_protected_fd(fd)) {
    return _real_pwrite(fd, buf, count, offset);
  }
  BASIC_SYNC_WRAPPER(ssize_t, pwrite, _real_pwrite, fd, buf, count, offset);
}

extern "C" ssize_t readv(int fd, const struct iovec *iov, int iovcnt)
{
  if (dmtcp_is_protected_fd(fd)) {
    return _real_readv(fd, iov, iovcnt);
  }

  WRAPPER_HEADER(ssize_t, readv, _real_readv, fd, iov, iovcnt);

  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(ssize_t, readv);
    if (retval > 0) {
      WRAPPER_REPLAY_READ_VECTOR_FROM_READ_LOG(readv, iov, iovcnt);
    }
    WRAPPER_REPLAY_END(readv);
  } else if (SYNC_IS_RECORD) {
    retval = _real_readv(fd, iov, iovcnt);
    if (retval > 0) {
      WRAPPER_LOG_WRITE_VECTOR_INTO_READ_LOG(readv, iov, iovcnt, retval);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
{
  BASIC_SYNC_WRAPPER(ssize_t, writev, _real_writev, fd, iov, iovcnt);
}

extern "C" ssize_t preadv(int fd, const struct iovec *iov, int iovcnt,
                          off_t offset)
{
  if (dmtcp_is_protected_fd(fd)) {
    return _real_preadv(fd, iov, iovcnt, offset);
  }

  WRAPPER_HEADER_RAW(ssize_t, preadv, _real_preadv, fd, iov, iovcnt, offset);
  ssize_t retval;
  log_entry_t my_entry = create_preadv_entry(my_clone_id, preadv_event, fd, iov, iovcnt, offset);

  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(ssize_t, preadv);
    if (retval > 0) {
      WRAPPER_REPLAY_READ_VECTOR_FROM_READ_LOG(preadv, iov, iovcnt);
    }
    WRAPPER_REPLAY_END(preadv);
  } else if (SYNC_IS_RECORD) {
    retval = _real_preadv(fd, iov, iovcnt, offset);
    if (retval > 0) {
      WRAPPER_LOG_WRITE_VECTOR_INTO_READ_LOG(preadv, iov, iovcnt, retval);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" ssize_t pwritev(int fd, const struct iovec *iov, int iovcnt,
                           off_t offset)
{
  BASIC_SYNC_WRAPPER(ssize_t, pwritev, _real_pwritev, fd, iov, iovcnt, offset);
}

extern "C" int access(const char *pathname, int mode)
{
  BASIC_SYNC_WRAPPER(int, access, _real_access, pathname, mode);
}

extern "C" int dup(int oldfd)
{
  BASIC_SYNC_WRAPPER(int, dup, _real_dup, oldfd);
}

extern "C" int dup2(int oldfd, int newfd)
{
  BASIC_SYNC_WRAPPER(int, dup2, _real_dup2, oldfd, newfd);
}

extern "C" int dup3(int oldfd, int newfd, int flags)
{
  BASIC_SYNC_WRAPPER(int, dup3, _real_dup3, oldfd, newfd, flags);
}

extern "C" off_t lseek(int fd, off_t offset, int whence)
{
  BASIC_SYNC_WRAPPER(off_t, lseek, _real_lseek, fd, offset, whence);
}

// FIXME: Add proper wrapper for lseek64 and llseek
extern "C" off64_t lseek64(int fd, off64_t offset, int whence)
{
  BASIC_SYNC_WRAPPER(off64_t, lseek64, _real_lseek64, fd, offset, whence);
}

extern "C" loff_t llseek(int fd, loff_t offset, int whence)
{
  BASIC_SYNC_WRAPPER(loff_t, llseek, _real_llseek, fd, offset, whence);
}

extern "C" int unlink(const char *pathname)
{
  BASIC_SYNC_WRAPPER(int, unlink, _real_unlink, pathname);
}

extern "C" int fdatasync(int fd)
{
  BASIC_SYNC_WRAPPER(int, fdatasync, _real_fdatasync, fd);
}

extern "C" int fsync(int fd)
{
  BASIC_SYNC_WRAPPER(int, fsync, _real_fsync, fd);
}

extern "C" int link(const char *oldpath, const char *newpath)
{
  BASIC_SYNC_WRAPPER(int, link, _real_link, oldpath, newpath);
}

extern "C" int symlink(const char *oldpath, const char *newpath)
{
  BASIC_SYNC_WRAPPER(int, symlink, _real_symlink, oldpath, newpath);
}

extern "C" int rename(const char *oldpath, const char *newpath)
{
  BASIC_SYNC_WRAPPER(int, rename, _real_rename, oldpath, newpath);
}

extern "C" int rmdir(const char *pathname)
{
  BASIC_SYNC_WRAPPER(int, rmdir, _real_rmdir, pathname);
}

extern "C" int mkdir(const char *pathname, mode_t mode)
{
  BASIC_SYNC_WRAPPER(int, mkdir, _real_mkdir, pathname, mode);
}

extern "C" struct dirent * /*__attribute__ ((optimize(0)))*/ readdir(DIR *dirp)
{
  WRAPPER_HEADER(struct dirent*, readdir, _real_readdir, dirp);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(struct dirent*, readdir);
    if (retval != NULL) {
      *retval = GET_FIELD(my_entry, readdir, readdir_retval);
    }
    WRAPPER_REPLAY_END(readdir);
  } else if (SYNC_IS_RECORD) {
    retval = _real_readdir(dirp);
    if (retval != NULL) {
      JASSERT(retval->d_reclen < 256);
      SET_FIELD2(my_entry, readdir, readdir_retval, *retval);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int readdir_r(DIR *dirp, struct dirent *entry,
                         struct dirent **result)
{
  void *return_addr = GET_RETURN_ADDRESS();
  if (!shouldSynchronize(return_addr) || isProcessGDB()) {
    return _real_readdir_r(dirp, entry, result);
  }
  int retval;
  log_entry_t my_entry = create_readdir_r_entry(my_clone_id,
      readdir_r_event, dirp, entry, result);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(readdir_r);
    if (retval == 0 && entry != NULL) {
      *entry = GET_FIELD(my_entry, readdir_r, ret_entry);
    }
    if (retval == 0 && result != NULL) {
      *result = GET_FIELD(my_entry, readdir_r, ret_result);
    }
    if (retval != 0) {
      *result = NULL;
    }
    WRAPPER_REPLAY_END(readdir_r);
  } else if (SYNC_IS_RECORD) {
    retval = _real_readdir_r(dirp, entry, result);
    if (retval == 0 && entry != NULL) {
      SET_FIELD2(my_entry, readdir_r, ret_entry, *entry);
    }
    if (retval == 0 && result != NULL) {
      SET_FIELD2(my_entry, readdir_r, ret_result, *result);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int mkstemp(char *temp)
{
  BASIC_SYNC_WRAPPER(int, mkstemp, _real_mkstemp, temp);
}

extern "C" int fflush(FILE *stream)
{
  WRAPPER_HEADER(int, fflush, _real_fflush, stream);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY(fflush);
    /* If the stream is stdout, we want to see the data to screen.
     * Thus execute the real system call. */
    // XXX We can't do this so easily. If we make the _real_fflush call here,
    // it can call mmap() on replay at a different time as on record, since
    // the other FILE related syscalls are NOT made on replay.
    /*if (stream == stdout || stream == stderr) {
      retval = _real_fflush(stream);
      }*/
  } else if (SYNC_IS_RECORD) {
    retval = _real_fflush(stream);
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" void setbuf(FILE *stream, char *buf)
{
  dmtcp::ThreadInfo::setOkToLogNextFnc();
  setvbuf(stream, buf, buf ? _IOFBF : _IONBF, BUFSIZ);
}

extern "C" void setbuffer(FILE *stream, char *buf, size_t size)
{
  dmtcp::ThreadInfo::setOkToLogNextFnc();
  setvbuf(stream, buf, buf ? _IOFBF : _IONBF, size);
}

extern "C" void setlinebuf(FILE *stream)
{
  dmtcp::ThreadInfo::setOkToLogNextFnc();
  setvbuf(stream, (char*) NULL, _IOLBF, 0);
}

extern "C" int setvbuf(FILE *stream, char *buf, int mode, size_t size)
{
  void *return_addr = GET_RETURN_ADDRESS();
  if (!shouldSynchronize(return_addr) || isProcessGDB()) {
    return _real_setvbuf(stream, buf, mode, size);
  }
  int retval;
  log_entry_t my_entry = create_setvbuf_entry(my_clone_id,
      setvbuf_event, stream, buf, mode, size);

  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_TYPED(int, setvbuf);
  } else if (SYNC_IS_RECORD) {
    WRAPPER_LOG(_real_setvbuf, stream, buf, mode, size);
  }
  return retval;
}

extern "C" int fcntl(int fd, int cmd, ...)
{
  va_list ap;
  void *arg = NULL;
  va_start(ap, cmd);
  arg = va_arg(ap, void*);
  va_end(ap);

  WRAPPER_HEADER(int, fcntl, _real_fcntl, fd, cmd, arg);

  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(fcntl);
    if (cmd == F_GETLK && retval != -1 && arg != NULL) {
      *(struct flock*)arg = GET_FIELD(my_entry, fcntl, ret_flock);
    }
    WRAPPER_REPLAY_END(fcntl);
  } else if (SYNC_IS_RECORD) {
    retval = _real_fcntl(fd, cmd, arg);
    if (cmd == F_GETLK && retval != -1 && arg != NULL) {
      SET_FIELD2(my_entry, fcntl, ret_flock, *(struct flock*)arg);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }

  return retval;
}


// FIXME: Ask Ana to write this wrapper.
#if 0
#ifdef PID_VIRTUALIZATION
int send_sigwinch;

void ioctl_helper(log_entry_t &my_entry, int &retval, int d, int request,
                  void *arg) {
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(ioctl);
    switch (request) {
      case SIOCGIFCONF: {
        *((struct ifconf *)arg) = GET_FIELD(my_entry, ioctl, ifconf_val);
        struct ifconf *i = (struct ifconf *)arg;
        WRAPPER_REPLAY_READ_FROM_READ_LOG(ioctl, i->ifc_buf, i->ifc_len);
        break;
      }
      case TIOCGWINSZ: {
        *((struct winsize *)arg) = GET_FIELD(my_entry, ioctl, win_val);
        break;
      }
      default:
        break;
    }
    WRAPPER_REPLAY_END(ioctl);
  } else if (SYNC_IS_RECORD) {
    retval = _real_ioctl(d, request, arg);
    switch (request) {
      case SIOCGIFCONF: {
        SET_FIELD2(my_entry, ioctl, ifconf_val, *((struct ifconf *)arg));
        struct ifconf *i = (struct ifconf *)arg;
        WRAPPER_LOG_WRITE_INTO_READ_LOG(ioctl, i->ifc_buf, i->ifc_len);
        break;
      }
      case TIOCGWINSZ: {
        SET_FIELD2(my_entry, ioctl, win_val, *((struct winsize *)arg));
        break;
      }
      default: {
        break;
      }
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
}
#endif
#endif

extern "C" int ioctl(int d,  unsigned long int request, ...)
{
  va_list ap;
  va_start(ap, request);
  void *arg = va_arg(ap, void*);
  va_end(ap);

  WRAPPER_HEADER(int, ioctl, _real_ioctl, d, request, arg);

  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(ioctl);
    switch (request) {
      case SIOCGIFCONF: {
        *((struct ifconf *)arg) = GET_FIELD(my_entry, ioctl, ifconf_val);
        struct ifconf *i = (struct ifconf *)arg;
        WRAPPER_REPLAY_READ_FROM_READ_LOG(ioctl, i->ifc_buf, i->ifc_len);
        break;
      }
      case TIOCGWINSZ: {
        *((struct winsize *)arg) = GET_FIELD(my_entry, ioctl, win_val);
        break;
      }
      case FIONREAD: {
        *(int*) arg = GET_FIELD(my_entry, ioctl, fionread_val);
        break;
      }
      default:
        break;
    }
    WRAPPER_REPLAY_END(ioctl);
  } else if (SYNC_IS_RECORD) {
    retval = _real_ioctl(d, request, arg);
    switch (request) {
      case SIOCGIFCONF: {
        SET_FIELD2(my_entry, ioctl, ifconf_val, *((struct ifconf *)arg));
        struct ifconf *i = (struct ifconf *)arg;
        WRAPPER_LOG_WRITE_INTO_READ_LOG(ioctl, i->ifc_buf, i->ifc_len);
        break;
      }
      case TIOCGWINSZ: {
        SET_FIELD2(my_entry, ioctl, win_val, *((struct winsize *)arg));
        break;
      }
      case FIONREAD: {
        SET_FIELD2(my_entry, ioctl, win_val, *((struct winsize *)arg));
        break;
      }
      default: {
        break;
      }
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }

  return retval;
}

extern "C" pid_t wait (__WAIT_STATUS stat_loc)
{
  dmtcp::ThreadInfo::setOkToLogNextFnc();
  return wait4(-1, stat_loc, 0, NULL);
}

extern "C" pid_t waitpid(pid_t pid, int *status, int options)
{
  dmtcp::ThreadInfo::setOkToLogNextFnc();
  return wait4(pid, status, options, NULL);
}

extern "C" int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options)
{
  WRAPPER_HEADER(int, waitid, _real_waitid, idtype, id, infop, options);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(int, waitid);
    if (retval != -1 && infop != NULL) {
      *infop = GET_FIELD(my_entry, waitid, ret_infop);
    }
    WRAPPER_REPLAY_END(waitid);
  } else if (SYNC_IS_RECORD) {
    retval = _real_waitid (idtype, id, infop, options);
    if (retval != -1 && infop != NULL) {
      SET_FIELD2(my_entry, waitid, ret_infop, *infop);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" pid_t wait3(__WAIT_STATUS status, int options, struct rusage *rusage)
{
  dmtcp::ThreadInfo::setOkToLogNextFnc();
  return wait4 (-1, status, options, rusage);
}

extern "C" pid_t wait4(pid_t pid, __WAIT_STATUS status, int options,
                       struct rusage *rusage)
{
  WRAPPER_HEADER(pid_t, wait4, _real_wait4, pid, status, options, rusage);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(pid_t, wait4);
    if (retval != -1 && status != NULL) {
      *(int*)status = (int) (unsigned long) GET_FIELD(my_entry, wait4, ret_status);
    }
    if (retval != -1 && rusage != NULL) {
      *rusage = GET_FIELD(my_entry, wait4, ret_rusage);
    }
    WRAPPER_REPLAY_END(wait4);
  } else if (SYNC_IS_RECORD) {
    pid_t retval = _real_wait4(pid, status, options, rusage);
    if (retval != -1 && status != NULL) {
      SET_FIELD2(my_entry, wait4, ret_status, (void*) (unsigned long) *(int*)status);
    }
    if (retval != -1 && rusage != NULL) {
      SET_FIELD2(my_entry, wait4, ret_rusage, *rusage);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int eventfd(unsigned int initval, int flags)
{
  errno = ENODEV;
  return -1;
}

extern "C" void flockfile(FILE *filehandle)
{
  BASIC_SYNC_WRAPPER_VOID(flockfile, _real_flockfile, filehandle);
}

extern "C" int ftrylockfile(FILE *filehandle)
{
  BASIC_SYNC_WRAPPER(int, ftrylockfile, _real_ftrylockfile, filehandle);
}

extern "C" void funlockfile(FILE *filehandle)
{
  BASIC_SYNC_WRAPPER_VOID(funlockfile, _real_funlockfile, filehandle);
}
