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

#ifndef _GNU_SOURCE
# define _GNU_SOURCE /* for sake of mremap */
#endif
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
#include <sys/mman.h>
#include <sys/syscall.h>
#include <malloc.h>
#include <execinfo.h>
#include "constants.h"
#include "fred_wrappers.h"
#include "util.h"
#include  "jassert.h"
#include  "jfilesystem.h"

#include "fred_wrappers.h"
#include "synchronizationlogging.h"
#include "log.h"
#include "threadinfo.h"

static pthread_mutex_t allocation_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mmap_lock = PTHREAD_MUTEX_INITIALIZER;

char progname[200] = {0};

#if 0 // USED only for debug purposes.
static int initHook = 0;
static void my_init_hooks (void);
static void *my_malloc_hook (size_t, const void *);
static void *my_realloc_hook (void *, size_t, const void *);
static void *my_memalign_hook (size_t, size_t, const void *);
static void my_free_hook (void*, const void *);
static void *(*old_malloc_hook) (size_t, const void *);
static void *(*old_realloc_hook) (void *, size_t, const void *);
static void *(*old_memalign_hook) (size_t, size_t, const void *);
static void  (*old_free_hook) (void*, const void *);
//static void *_wrapped_malloc(size_t size);
//static void _wrapped_free(void *ptr);
/* Override initializing hook from the C library. */
//void (*__malloc_initialize_hook) (void) = my_init_hooks;

static pthread_mutex_t hook_lock = PTHREAD_MUTEX_INITIALIZER;

static void my_init_hooks(void)
{
  strncpy(progname, jalib::Filesystem::GetProgramName().c_str(), 200);
  /* Save old hook functions (from libc) and set them to our own hooks. */
  _real_pthread_mutex_lock(&hook_lock);
  if (!initHook) {
    old_malloc_hook = __malloc_hook;
    old_realloc_hook = __realloc_hook;
    old_memalign_hook = __memalign_hook;
    old_free_hook = __free_hook;
    __malloc_hook = my_malloc_hook;
    __realloc_hook = my_realloc_hook;
    __memalign_hook = my_memalign_hook;
    __free_hook = my_free_hook;
    initHook = 1;
  }
  _real_pthread_mutex_unlock(&hook_lock);
}

static void *my_malloc_hook (size_t size, const void *caller)
{
  _real_pthread_mutex_lock(&hook_lock);
  void *result;
  /* Restore all old hooks */
  __malloc_hook = old_malloc_hook;
  __realloc_hook = old_realloc_hook;
  __memalign_hook = old_memalign_hook;
  __free_hook = old_free_hook;
  result = _real_malloc (size);
  /* Save underlying hooks */
  old_malloc_hook = __malloc_hook;
  old_free_hook = __free_hook;
  if (log_all_allocs && strcmp(progname, "gdb") != 0) {
    static int tyler_pid = _real_getpid();
    printf ("<%d> malloc (%u) returns %p\n", tyler_pid, (unsigned int) size, result);
  }
  /*static int tyler_pid = _real_getpid();
  void *buffer[10];
  int nptrs;
  // NB: In order to use backtrace, you must disable log_all_allocs.
  // AND remove the locks around !log_all_allocs real_malloc in malloc wrapper
  nptrs = backtrace (buffer, 10);
  backtrace_symbols_fd ( buffer, nptrs, 1);
  printf ("<%d> malloc (%u) returns %p\n", tyler_pid, (unsigned int) size, result);*/
  /* Restore our own hooks */
  __malloc_hook = my_malloc_hook;
  __realloc_hook = my_realloc_hook;
  __memalign_hook = my_memalign_hook;
  __free_hook = my_free_hook;
  _real_pthread_mutex_unlock(&hook_lock);
  return result;
}

static void *my_realloc_hook (void *ptr, size_t size, const void *caller)
{
  _real_pthread_mutex_lock(&hook_lock);
  void *result;
  /* Restore all old hooks */
  __malloc_hook = old_malloc_hook;
  __realloc_hook = old_realloc_hook;
  __memalign_hook = old_memalign_hook;
  __free_hook = old_free_hook;
  result = _real_realloc (ptr, size);
  /* Save underlying hooks */
  old_malloc_hook = __malloc_hook;
  old_free_hook = __free_hook;
  if (log_all_allocs && strcmp(progname, "gdb") != 0) {
    static int tyler_pid = _real_getpid();
    printf ("<%d> realloc (%p,%u) returns %p\n", tyler_pid, ptr, (unsigned int) size, result);
  }
  __malloc_hook = my_malloc_hook;
  __realloc_hook = my_realloc_hook;
  __memalign_hook = my_memalign_hook;
  __free_hook = my_free_hook;
  _real_pthread_mutex_unlock(&hook_lock);
  return result;

}

static void *my_memalign_hook (size_t boundary, size_t size, const void *caller)
{
  _real_pthread_mutex_lock(&hook_lock);
  void *result;
  /* Restore all old hooks */
  __malloc_hook = old_malloc_hook;
  __realloc_hook = old_realloc_hook;
  __memalign_hook = old_memalign_hook;
  __free_hook = old_free_hook;
  result = _real_libc_memalign (boundary, size);
  /* Save underlying hooks */
  old_malloc_hook = __malloc_hook;
  old_free_hook = __free_hook;
  if (log_all_allocs && strcmp(progname, "gdb") != 0) {
    static int tyler_pid = _real_getpid();
    printf ("<%d> memalign (%u,%u) returns %p\n", tyler_pid,
        (unsigned int)boundary, (unsigned int) size, result);
  }
  __malloc_hook = my_malloc_hook;
  __realloc_hook = my_realloc_hook;
  __memalign_hook = my_memalign_hook;
  __free_hook = my_free_hook;
  _real_pthread_mutex_unlock(&hook_lock);
  return result;

}

static void my_free_hook (void *ptr, const void *caller)
{
  _real_pthread_mutex_lock(&hook_lock);
  /* Restore all old hooks */
  __malloc_hook = old_malloc_hook;
  __realloc_hook = old_realloc_hook;
  __memalign_hook = old_memalign_hook;
  __free_hook = old_free_hook;
  _real_free (ptr);
  /* Save underlying hooks */
  old_malloc_hook = __malloc_hook;
  old_free_hook = __free_hook;
  /* printf might call free, so protect it too. */
  if (log_all_allocs) {
    static int tyler_pid = _real_getpid();
    printf ("<%d> freed pointer %p\n", tyler_pid, ptr);
  }
  /* Restore our own hooks */
  __malloc_hook = my_malloc_hook;
  __realloc_hook = my_realloc_hook;
  __memalign_hook = my_memalign_hook;
  __free_hook = my_free_hook;
  _real_pthread_mutex_unlock(&hook_lock);
}
#endif

#define MMAP_WRAPPER_HEADER_TYPED(type, name, ...)                      \
  void *return_addr = GET_RETURN_ADDRESS();                             \
  if ((!shouldSynchronize(return_addr) && !log_all_allocs) ||           \
      isProcessGDB()) {                                                 \
    type retval = (type) _real_##name(__VA_ARGS__);                     \
    return retval;                                                      \
  }                                                                     \
  log_entry_t my_entry = create_ ## name ## _entry(my_clone_id,         \
      name ## _event,                                                   \
      __VA_ARGS__);                                                     \
  type retval;

#define MMAP_WRAPPER_REPLAY_START(name)                                 \
  WRAPPER_REPLAY_START_TYPED(void*, name);                              \
  _real_pthread_mutex_lock(&mmap_lock);

#define MMAP_WRAPPER_REPLAY_END(name)                                   \
  _real_pthread_mutex_unlock(&mmap_lock);                               \
  WRAPPER_REPLAY_END(name);

#define MALLOC_FAMILY_WRAPPER_HEADER_TYPED(ret_type, name, ...)         \
  void *return_addr = GET_RETURN_ADDRESS();                             \
  if ((!shouldSynchronize(return_addr) && !log_all_allocs) ||           \
      isProcessGDB()) {                                                 \
    ret_type retval = _real_ ## name (__VA_ARGS__);                     \
    return retval;                                                      \
  }                                                                     \
  log_entry_t my_entry = create_ ## name ## _entry(my_clone_id,         \
                                                   name ## _event,      \
                                                   __VA_ARGS__);        \
  ret_type retval;

#define MALLOC_FAMILY_WRAPPER_HEADER(name, ...)                         \
  MALLOC_FAMILY_WRAPPER_HEADER_TYPED(void*, name, __VA_ARGS__)

#define MALLOC_FAMILY_WRAPPER_REPLAY_START(name)                        \
  waitForAllocTurn(&my_entry, &name##_turn_check);

#define MALLOC_FAMILY_WRAPPER_REPLAY_END(name)                          \
  WRAPPER_REPLAY_END(name)

#define MALLOC_FAMILY_BASIC_SYNC_WRAPPER(ret_type, name, ...)           \
  MALLOC_FAMILY_WRAPPER_HEADER_TYPED(ret_type, name, __VA_ARGS__);      \
  do {                                                                  \
    if (SYNC_IS_REPLAY) {                                               \
      WRAPPER_REPLAY_START_TYPED(ret_type, name);                       \
      _real_pthread_mutex_lock(&allocation_lock);                       \
      WRAPPER_REPLAY_END(name);                                         \
      ret_type new_retval = _real_ ## name(__VA_ARGS__);                \
      _real_pthread_mutex_unlock(&allocation_lock);                     \
      JASSERT (new_retval == retval)                                    \
        ( retval ) ( new_retval ) (global_log.currentEntryIndex())      \
        .Text ( #name " returned wrong address on replay" );            \
    } else if (SYNC_IS_RECORD) {                                        \
      _real_pthread_mutex_lock(&allocation_lock);                       \
      WRAPPER_LOG_WRITE_ENTRY(my_entry);                                \
      dmtcp::ThreadInfo::setOptionalEvent();                            \
      retval = _real_ ## name(__VA_ARGS__);                             \
      dmtcp::ThreadInfo::unsetOptionalEvent();                          \
      my_entry.setRetval(retval);                                       \
      my_entry.setSavedErrno(errno);                                    \
      WRAPPER_LOG_UPDATE_ENTRY(my_entry);                               \
      _real_pthread_mutex_unlock(&allocation_lock);                     \
    }                                                                   \
  } while(0)


/* This buffer (wrapper_init_buf) is used to pass on to dlsym() while it is
 * initializing the dmtcp wrappers. See comments in syscallsreal.c for more
 * details.
 */
static char wrapper_init_buf[1024];
static bool mem_allocated_for_initializing_wrappers = false;

extern "C" void *calloc(size_t nmemb, size_t size)
{
  if (fred_wrappers_initializing) {
    JASSERT(!mem_allocated_for_initializing_wrappers);
    memset(wrapper_init_buf, 0, sizeof (wrapper_init_buf));
    //void *ret = JALLOC_HELPER_MALLOC ( nmemb * size );
    mem_allocated_for_initializing_wrappers = true;
    return (void*) wrapper_init_buf;
  }
  MALLOC_FAMILY_BASIC_SYNC_WRAPPER(void*, calloc, nmemb, size);
  return retval;
}

extern "C" void *malloc(size_t size)
{
  if (fred_wrappers_initializing) {
    return calloc(1, size);
  }
  MALLOC_FAMILY_BASIC_SYNC_WRAPPER(void*, malloc, size);
  return retval;
}

extern "C" void *__libc_memalign(size_t boundary, size_t size)
{
  JASSERT (my_clone_id != 0);
  MALLOC_FAMILY_BASIC_SYNC_WRAPPER(void*, libc_memalign, boundary, size);
  return retval;
}

extern "C" void *valloc(size_t size)
{
  return __libc_memalign(sysconf(_SC_PAGESIZE), size);
}

// FIXME:  Add wrapper for alloca(), posix_memalign(), etc.,

extern "C" void free(void *ptr)
{
  if (fred_wrappers_initializing) {
    JASSERT(mem_allocated_for_initializing_wrappers);
    JASSERT(ptr == wrapper_init_buf);
    return;
  }
  if (ptr == NULL) {
    return;
  }
  void *return_addr = GET_RETURN_ADDRESS();
  if ((!shouldSynchronize(return_addr) && !log_all_allocs) ||
      ptr == NULL || isProcessGDB()) {
    _real_pthread_mutex_lock(&allocation_lock);
    _real_free(ptr);
    _real_pthread_mutex_unlock(&allocation_lock);
    return;
  }

  log_entry_t my_entry = create_free_entry(my_clone_id, free_event, ptr);
  void *retval = NULL;

  if (SYNC_IS_REPLAY) {
    waitForTurn(&my_entry, &free_turn_check);
    _real_pthread_mutex_lock(&allocation_lock);
    WRAPPER_REPLAY_END(free);
    _real_free(ptr);
    _real_pthread_mutex_unlock(&allocation_lock);
  } else if (SYNC_IS_RECORD) {
    _real_pthread_mutex_lock(&allocation_lock);
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
    _real_free(ptr);
    _real_pthread_mutex_unlock(&allocation_lock);
  }
}

extern "C" void *realloc(void *ptr, size_t size)
{
  MALLOC_FAMILY_BASIC_SYNC_WRAPPER(void*, realloc, ptr, size);
  return retval;
}

static bool isPartOfAddrRangeAlreadyMmapped(void *addr, size_t length)
{
  int mapsFd = -1;
  dmtcp::Util::ProcMapsArea area;
  char *startAddr = (char*) addr;
  char *endAddr = startAddr + length;
  bool result = false;

  if ((mapsFd = _real_open("/proc/self/maps", O_RDONLY, S_IRUSR)) == -1) {
    perror("open");
    exit(1);
  }

  while (dmtcp::Util::readProcMapsLine(mapsFd, &area)) {
    if ((startAddr >= area.addr && startAddr < area.endAddr) ||
        (endAddr > area.addr && endAddr <= area.endAddr)) {
      result = (area.prot & PROT_EXEC) != 0;
      break;
    }
  }
  _real_close(mapsFd);
  return result;
}

/* mmap/mmap64
 * TODO: Remove the PROT_WRITE flag on REPLAY phase if it was not part of
 *       original flags.
 * FIXME: MAP_SHARED areas are restored as MAP_PRIVATE, check for correctness.
 */
extern "C" void *fred_mmap(void *addr, size_t length, int prot, int flags,
                           int fd, off_t offset)
{
  MMAP_WRAPPER_HEADER_TYPED(void*, mmap, addr, length, prot, flags, fd, offset);
  if (SYNC_IS_REPLAY) {
    bool mmap_read_from_readlog = false;
    MMAP_WRAPPER_REPLAY_START(mmap);
    //JWARNING ( addr == NULL ).Text("Unimplemented to have non-null addr.");
    addr = my_entry.retval();
    if (retval != MAP_FAILED && fd != -1 &&
        ((flags & MAP_PRIVATE) != 0 || (flags & MAP_SHARED) != 0)) {
      flags &= ~MAP_SHARED;
      flags |= MAP_PRIVATE;
      flags |= MAP_ANONYMOUS;
      fd = -1;
      offset = 0;
      //size_t page_size = sysconf(_SC_PAGESIZE);
      //size_t page_mask = ~(page_size - 1);
      //length = (length + page_size - 1) & page_mask ;
      mmap_read_from_readlog = true;
    }
    flags |= MAP_FIXED;
    JASSERT(!isPartOfAddrRangeAlreadyMmapped(addr, length));
    retval = (void*) _real_mmap(addr, length, prot | PROT_WRITE,
                                flags, fd, offset);
    JASSERT ( retval == my_entry.retval() ) (retval)
      (my_entry.retval()) (JASSERT_ERRNO);
    if (mmap_read_from_readlog) {
      WRAPPER_REPLAY_READ_FROM_READ_LOG(mmap, retval, length);
    }
    MMAP_WRAPPER_REPLAY_END(mmap);
  } else if (SYNC_IS_RECORD) {
    _real_pthread_mutex_lock(&mmap_lock);
    retval = (void*) _real_mmap(addr, length, prot,
                                   flags, fd, offset);
    if (retval != MAP_FAILED && fd != -1 &&
        ((flags & MAP_PRIVATE) != 0 || (flags & MAP_SHARED) != 0)) {
      WRAPPER_LOG_WRITE_INTO_READ_LOG(mmap, retval, length);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
    _real_pthread_mutex_unlock(&mmap_lock);
  }
  return retval;
}

extern "C" int fred_munmap(void *addr, size_t length)
{
  bool _use_syscall = false;
  MALLOC_FAMILY_WRAPPER_HEADER_TYPED(int, munmap, addr, length);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(munmap);
    _real_pthread_mutex_lock(&mmap_lock);
    retval = _real_munmap(addr, length);
    JASSERT (retval == (int)(unsigned long)my_entry.retval());
    _real_pthread_mutex_unlock(&mmap_lock);
    WRAPPER_REPLAY_END(munmap);
  } else if (SYNC_IS_RECORD) {
    _real_pthread_mutex_lock(&mmap_lock);
    retval = _real_munmap(addr, length);
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
    _real_pthread_mutex_unlock(&mmap_lock);
  }
  return retval;
}

// When exactly did the declaration of /usr/include/sys/mman.h change?
// (The extra parameter was created for the sake of MREMAP_FIXED.)
extern "C" void *fred_mremap(void *old_address, size_t old_size,
                             size_t new_size, int flags, ...)
{
  va_list ap;
  va_start( ap, flags );
  void *new_address = va_arg ( ap, void * );
  va_end ( ap );

  bool _use_syscall = false;
  MALLOC_FAMILY_WRAPPER_HEADER_TYPED(void*, mremap, old_address, old_size,
                                     new_size, flags, new_address);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(void *, mremap);
    _real_pthread_mutex_lock(&mmap_lock);
    void *addr = my_entry.retval();
    flags |= (MREMAP_MAYMOVE | MREMAP_FIXED);
    JASSERT(!isPartOfAddrRangeAlreadyMmapped(addr, new_size));
    retval = (void*) _real_mremap(old_address, old_size, new_size,
                                  flags, addr);
    JASSERT ( retval == my_entry.retval() );
    _real_pthread_mutex_unlock(&mmap_lock);
    WRAPPER_REPLAY_END(mremap);
  } else if (SYNC_IS_RECORD) {
    _real_pthread_mutex_lock(&mmap_lock);
    retval = (void*) _real_mremap(old_address, old_size, new_size,
                                  flags, new_address);
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
    _real_pthread_mutex_unlock(&mmap_lock);
  }
  return retval;
}

/*
extern "C" void *mmap2(void *addr, size_t length, int prot,
    int flags, int fd, off_t pgoffset)
{

}
*/
