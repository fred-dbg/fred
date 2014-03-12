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
#include <fcntl.h>

static int isPassthroughStream(FILE *stream)
{
  return isPassthroughFd(_real_fileno(stream));
}

#ifdef ENABLE_MEM_ACCURACY
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
    WRAPPER_LOG_WRITE_ENTRY(fopen);
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
    WRAPPER_LOG_WRITE_ENTRY(fopen64);
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
    WRAPPER_LOG_WRITE_ENTRY(freopen);
  }
  return retval;
}
#endif

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
    WRAPPER_LOG_WRITE_ENTRY(tmpfile);
  }
  return retval;
}

extern "C" void flockfile(FILE *stream)
{
  BASIC_SYNC_WRAPPER_VOID(flockfile, _real_flockfile, stream);
}

extern "C" int ftrylockfile(FILE *stream)
{
  BASIC_SYNC_WRAPPER(int, ftrylockfile, _real_ftrylockfile, stream);
}

extern "C" void funlockfile(FILE *stream)
{
  BASIC_SYNC_WRAPPER_VOID(funlockfile, _real_funlockfile, stream);
}

#undef WRAPPER_HEADER_RAW
#define WRAPPER_HEADER_RAW(ret_type, name, real_func, ...)              \
  void *return_addr = GET_RETURN_ADDRESS();                             \
  do {                                                                  \
    if (!shouldSynchronize(return_addr) || isProcessGDB() ||            \
        isPassthroughStream(stream)) {            \
      return real_func(__VA_ARGS__);                                    \
    }                                                                   \
  } while(0)

#ifdef ENABLE_MEM_ACCURACY
extern "C" int fclose(FILE *stream)
{
  WRAPPER_HEADER(int, fclose, _real_fclose, stream);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_TYPED(int, fclose);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_fclose(stream);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    WRAPPER_LOG_WRITE_ENTRY(fclose);
  }
  return retval;
}
#endif

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
    WRAPPER_LOG_WRITE_ENTRY(fdopendir);
  }
  return retval;
}
#endif

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
    WRAPPER_LOG_WRITE_ENTRY(getdelim);
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
    WRAPPER_LOG_WRITE_ENTRY(getline);
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
    WRAPPER_LOG_WRITE_ENTRY(vfscanf);
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
    WRAPPER_LOG_WRITE_ENTRY(fgets);
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
      }
    retval = (int) RETVAL(my_entry, vfprintf);
      */
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _fprintf(stream, format, arg);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    WRAPPER_LOG_WRITE_ENTRY(vfprintf);
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
      }
      retval = (int) RETVAL(my_entry, vfprintf);
    */
    WRAPPER_REPLAY_END(vfprintf);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _fprintf(stream, format, arg);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    WRAPPER_LOG_WRITE_ENTRY(vfprintf);
  }
  return retval;
}

extern "C" int printf (const char *format, ...)
{
  va_list arg;
  va_start (arg, format);
  FILE *stream = stdout;
  WRAPPER_HEADER(int, vfprintf, _fprintf, stream, format, arg);

  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY(vfprintf);
    /* If we're writing to stdout, we want to see the data to screen.
     * Thus execute the real system call. */
    // XXX We can't do this so easily. If we make the _real_printf call here,
    // it can call mmap() on replay at a different time as on record, since
    // the other FILE related syscalls are NOT made on replay.
    /*if (stream == stdout || stream == stderr) {
      retval = _fprintf(stream, format, arg);
      }
      retval = (int) RETVAL(my_entry, vfprintf);
    */
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _fprintf(stream, format, arg);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    WRAPPER_LOG_WRITE_ENTRY(vfprintf);
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
    WRAPPER_LOG_WRITE_ENTRY(fseek);
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
    WRAPPER_LOG_WRITE_ENTRY(getc);
  }
  return retval;
}

extern "C" int __uflow2(FILE *stream)
{
  //if (isPassthroughStream(stream)) {
  //  return _IO_getc(stream);
  //}
  JWARNING(isProcessGDB()). Text("Not Implemented: *_unlocked fstream functions");
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
    WRAPPER_LOG_WRITE_ENTRY(fgetc);
  }
  return retval;
}

extern "C" int ungetc(int c, FILE *stream)
{
  BASIC_SYNC_WRAPPER(int, ungetc, _real_ungetc, c, stream);
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
    WRAPPER_LOG_WRITE_ENTRY(fputs);
  }
  return retval;
}

extern "C" int _IO_puts(const char *s)
{
  dmtcp::ThreadInfo::setOkToLogNextFnc();
  return fputs(s, stdout);
}

extern "C" int puts(const char *s)
{
  dmtcp::ThreadInfo::setOkToLogNextFnc();
  return fputs(s, stdout);
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
    WRAPPER_LOG_WRITE_ENTRY(fputc);
  }
  return retval;
}

extern "C" int _IO_putc(int c, FILE *stream)
{
  WRAPPER_HEADER(int, putc, _real_putc, c, stream);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_TYPED(int, putc);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_putc(c, stream);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    WRAPPER_LOG_WRITE_ENTRY(putc);
  }
  return retval;
}

// WARNING:  Early versions of glibc (e.g. glibc 2.3) define this
//  function in stdio.h as inline.  This wrapper won't work in that case.
# if __GLIBC_PREREQ (2,4)
extern "C" int putchar(int c)
{
  dmtcp::ThreadInfo::setOkToLogNextFnc();
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
    WRAPPER_LOG_WRITE_ENTRY(fwrite);
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
    WRAPPER_LOG_WRITE_ENTRY(fread);
  }
  return retval;
}

extern "C" void rewind(FILE *stream)
{
  BASIC_SYNC_WRAPPER_VOID(rewind, _real_rewind, stream);
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
    WRAPPER_LOG_WRITE_ENTRY(fflush);
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
    WRAPPER_LOG(setvbuf, _real_setvbuf, stream, buf, mode, size);
  }
  return retval;
}

