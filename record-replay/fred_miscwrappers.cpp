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

#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "fred_wrappers.h"
#include "dmtcpplugin.h"

extern "C"
int shmget(key_t key, size_t size, int shmflg)
{
  errno = ENOMEM;
  return -1;
}

#define SYSCALL_GET_ARG(type,arg) type arg = va_arg(ap, type)

#define SYSCALL_GET_ARGS_2(type1,arg1,type2,arg2)                       \
  SYSCALL_GET_ARG(type1,arg1);                                          \
  SYSCALL_GET_ARG(type2,arg2)

#define SYSCALL_GET_ARGS_3(type1,arg1,type2,arg2,type3,arg3)            \
  SYSCALL_GET_ARGS_2(type1,arg1,type2,arg2);                            \
  SYSCALL_GET_ARG(type3,arg3)

#define SYSCALL_GET_ARGS_4(type1,arg1,type2,arg2,type3,arg3,type4,arg4) \
  SYSCALL_GET_ARGS_3(type1,arg1,type2,arg2,type3,arg3);                 \
  SYSCALL_GET_ARG(type4,arg4)

#define SYSCALL_GET_ARGS_5(type1,arg1,type2,arg2,type3,arg3,type4,arg4, \
                           type5,arg5)                                  \
  SYSCALL_GET_ARGS_4(type1,arg1,type2,arg2,type3,arg3,type4,arg4);      \
  SYSCALL_GET_ARG(type5,arg5)

#define SYSCALL_GET_ARGS_6(type1,arg1,type2,arg2,type3,arg3,type4,arg4, \
                           type5,arg5,type6,arg6)                        \
  SYSCALL_GET_ARGS_5(type1,arg1,type2,arg2,type3,arg3,type4,arg4,       \
                     type5,arg5);                                       \
  SYSCALL_GET_ARG(type6,arg6)

#define SYSCALL_GET_ARGS_7(type1,arg1,type2,arg2,type3,arg3,type4,arg4, \
                           type5,arg5,type6,arg6,type7,arg7)             \
  SYSCALL_GET_ARGS_6(type1,arg1,type2,arg2,type3,arg3,type4,arg4,       \
                     type5,arg5,type6,arg6);                             \
  SYSCALL_GET_ARG(type7,arg7)


/* Comments by Gene:
 * Here, syscall is the wrapper, and the call to syscall would be _real_syscall
 * We would add a special case for SYS_gettid, while all others default as below
 * It depends on the idea that arguments are stored in registers, whose
 *  natural size is:  sizeof(void*)
 * So, we pass six arguments to syscall, and it will ignore any extra arguments
 * I believe that all Linux system calls have no more than 7 args.
 * clone() is an example of one with 7 arguments.
 * If we discover system calls for which the 7 args strategy doesn't work,
 *  we can special case them.
 *
 * XXX: DO NOT USE JTRACE/JNOTE/JASSERT in this function; even better, do not
 *      use any STL here.  (--Kapil)
 */

extern "C" long int syscall(long int sys_num, ... )
{
  long int ret;
  va_list ap;

  va_start(ap, sys_num);

  switch ( sys_num ) {

    case SYS_mmap:
    {
      SYSCALL_GET_ARGS_6(void*, addr, size_t, length, int, prot, int, flags,
                         int, fd, off_t, offset);
      ret = (long int) fred_mmap(addr, length, prot, flags, fd, offset);
      break;
    }

    case SYS_mremap:
    {
      SYSCALL_GET_ARGS_5(void*, old_addr, size_t, old_size, size_t, new_size,
                         int, flags, void*, new_addr);
      ret = (long int) fred_mremap(old_addr, old_size, new_size, flags, new_addr);
      break;
    }

    case SYS_munmap:
    {
      SYSCALL_GET_ARGS_2(void*, addr, size_t, length);
      ret = fred_munmap(addr, length);
      break;
    }

    default:
    {
      SYSCALL_GET_ARGS_7(void*, arg1, void*, arg2, void*, arg3, void*, arg4,
                         void*, arg5, void*, arg6, void*, arg7);
      ret = _real_syscall(sys_num, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
      break;
    }
  }
  va_end(ap);
  return ret;
}
