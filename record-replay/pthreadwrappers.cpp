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

#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include "constants.h"
#include  "jassert.h"
#include  "jfilesystem.h"
#include "synchronizationlogging.h"
#include "log.h"
#include "fred_wrappers.h"
#include "threadinfo.h"

static int signal_thread_alive = 0;
static volatile int thread_create_destroy = 0;
static pthread_mutex_t create_destroy_guard = PTHREAD_MUTEX_INITIALIZER;

static inline void memfence() {  asm volatile ("mfence" ::: "memory"); }
struct create_arg
{
  void *(*fn)(void *);
  void *thread_arg;
  void *userStack;
  pthread_attr_t attr;
  int   userDetachState;
};

static int internal_pthread_mutex_lock(pthread_mutex_t *);
static int internal_pthread_mutex_unlock(pthread_mutex_t *);

static void *start_wrapper(void *arg)
{
  dmtcp::ThreadInfo::postPthreadCreate();
  JASSERT(my_clone_id != -1);
  /*
   This start function calls the user's start function. We need this so that we
   gain control immediately after the user's start function terminates, but
   before control goes back to libpthread. Libpthread will do some cleanup
   involving a free() call and some low level locks. Since we can't control the
   low level locks, we must implement our own lock: thread_transition_mutex.
  */
  void *retval;
  struct create_arg *createArg = (struct create_arg *)arg;
  void *(*user_fnc) (void *) = createArg->fn;
  void *thread_arg = createArg->thread_arg;
  dmtcp::ThreadInfo::updateState(pthread_self(), createArg->attr,
                                 createArg->userStack,
                                 createArg->userDetachState);
  JALLOC_HELPER_FREE(arg);

  retval = (*user_fnc)(thread_arg);
  JTRACE ( "User start function over." );
  dmtcp::ThreadInfo::reapThisThread();
  return retval;
}

/*
   Create a thread stack via mmap() if one is not specified in the user
   attributes.

   Parameters:
   attr_out - (output) The final attributes caller should use.
   user_attr - User provided attributes; defer to these.
   size - If non-0, force new stack to this size.
*/
static void setupThreadStack(pthread_attr_t *attr_out,
                             const pthread_attr_t *user_attr, size_t size)
{
  size_t stack_size;
  void *stack_addr;
  int userStack = 0;
  // If the user's attributes have specified a stack size, use that.
  if (user_attr != NULL) {
    pthread_attr_getstack(user_attr, &stack_addr, &stack_size);
    if (stack_size != 0) {
      userStack = 1;
    }
    if (stack_addr != NULL) {
      // User provided stack, Copy the user's attributes:
      JASSERT(stack_size != 0);
      *attr_out = *user_attr;
      JASSERT(false);
      return;
    }
  }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
  int mmap_flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK;
#else
  int mmap_flags = MAP_PRIVATE | MAP_ANONYMOUS;
#endif
  size_t mmap_size;
  if (userStack) {
    mmap_size = stack_size;
  } else if (size == 0) {
    /* It's possible the we got killed in the middle of pthread_create. If that
     * happens, this assert is likely to trigger on REPLAY, as the correct
     * stack size is recorded only after the _real_pthread_create returns.
     JASSERT(!SYNC_IS_REPLAY);
    */

    // Also figure out the default stack size for NPTL threads using the
    // architecture-specific limits defined in nptl/sysdeps/ARCH/pthreaddef.h
    struct rlimit rl;
    JASSERT(0 == getrlimit(RLIMIT_STACK, &rl));
#ifdef __x86_64__
    size_t arch_default_stack_size = 32*1024*1024;
#else
    size_t arch_default_stack_size = 2*1024*1024;
#endif
    mmap_size = (rl.rlim_cur == RLIM_INFINITY) ? arch_default_stack_size : rl.rlim_cur;
  } else {
    mmap_size = size;
  }

  // mmap() wrapper handles forcing it to the same place on replay.
  void *s = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, mmap_flags, -1, 0);
  if (s == MAP_FAILED)  {
    JTRACE ( "Failed to map thread stack." ) ( mmap_size )
      ( strerror(errno) ) (global_log.currentEntryIndex());
    JASSERT ( false );
  }

  pthread_attr_setstack(attr_out, s, mmap_size);
}

/* Disable the pthread_CREATE_DETACHED flag if present in the given
   attributes. Returns the modified attributes. */
static void disableDetachState(pthread_attr_t *attr)
{
  // The opposite and only alternative to PTHREAD_CREATE_DETACHED is
  // PTHREAD_CREATE_JOINABLE.
  pthread_attr_setdetachstate(attr, PTHREAD_CREATE_JOINABLE);
}

/* Begin wrapper code */

/* Performs the _real version with log and replay. Does NOT check
   shouldSynchronize() and shouldn't be called directly unless you know what
   you're doing. */
static int internal_pthread_mutex_lock(pthread_mutex_t *mutex)
{
  int retval = 0;
  log_entry_t my_entry = create_pthread_mutex_lock_entry(my_clone_id,
                                                         pthread_mutex_lock_event,
                                                         mutex);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY(pthread_mutex_lock);
  } else if (SYNC_IS_RECORD) {
    retval = _real_pthread_mutex_lock(mutex);
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

/* Performs the _real version with log and replay. Does NOT check
   shouldSynchronize() and shouldn't be called directly unless you know what
   you're doing. */
static int internal_pthread_mutex_unlock(pthread_mutex_t *mutex)
{
  int retval = 0;
  log_entry_t my_entry = create_pthread_mutex_unlock_entry(my_clone_id,
                                                           pthread_mutex_unlock_event,
                                                           mutex);

  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY(pthread_mutex_unlock);
  } else if (SYNC_IS_RECORD) {
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
    retval = _real_pthread_mutex_unlock(mutex);
    WRAPPER_LOG_UPDATE_ENTRY(my_entry);
  }
  return retval;
}

/* Performs the _real version with log and replay. Does NOT check
   shouldSynchronize() and shouldn't be called directly unless you know what
   you're doing. */
static int internal_pthread_cond_signal(pthread_cond_t *cond)
{
  int retval = 0;
  log_entry_t my_entry = create_pthread_cond_signal_entry(my_clone_id,
                                                          pthread_cond_signal_event,
                                                          cond);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY(pthread_cond_signal);
  } else  if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_pthread_cond_signal(cond);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

/* Performs the _real version with log and replay. Does NOT check
   shouldSynchronize() and shouldn't be called directly unless you know what
   you're doing. */
static int internal_pthread_cond_wait(pthread_cond_t *cond,
                                      pthread_mutex_t *mutex)
{
  /*
   * pthread_cond_wait() internally consists of three events:
   *   pthread_mutex_unlock()
   *   block on cond-var cond
   *   pthread_mutex_lock()
   * Ideally we would like to log the pthread_mutex_{lock,unlock} calls, but
   * since libpthread is making internal calls, we are unable to do so.
   *
   * Due to our inablility to force threads to acquire the mutex during REPLAY,
   * on broadcast calls, all threads blocked on condition variable wake up and
   * start to executing inside the critical section without acquiring the
   * mutex. This is * totally undesired and incorrect.
   *
   * The solution is to make threads wait until the thread executing inside the
   * critical section has left the critical section (This is done either by and
   * explicit call to pthread_mutex_unlock() or a call to pthread_cond_wait()
   * which involves unlocking the mutex).
   *
   * In order to do so, we introduce a _fake_ pthread_mutex_unlock() call (The
   * call is fake in the sense that _real_XXX version is never called during
   * record or replay). This extra log event makes sure that the other threads
   * have to wait until the thread in critical section is able to process this
   * event, hence making sure that only one thread is executing in the critical
   * section during REPLAY.
   */
  FAKE_BASIC_SYNC_WRAPPER(int, pthread_mutex_unlock, mutex);

  int retval = 0;
  log_entry_t my_entry = create_pthread_cond_wait_entry(my_clone_id,
                                                        pthread_cond_wait_event,
                                                        cond, mutex);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY(pthread_cond_wait);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_pthread_cond_wait(cond, mutex);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

/* Performs the _real version with log and replay. Does NOT check
   shouldSynchronize() and shouldn't be called directly unless you know what
   you're doing. */
static int internal_pthread_create(pthread_t *thread,
    const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg)
{
  int retval = 0, was_detached = 0;
  pthread_attr_t the_attr;
  size_t stack_size;
  void *stack_addr;
  struct create_arg *createArg =
    (struct create_arg*) JALLOC_HELPER_MALLOC(sizeof(*createArg));
  createArg->fn = start_routine;
  createArg->thread_arg = arg;
  if (attr != NULL) {
    pthread_attr_getstack(attr, &createArg->userStack, &stack_size);
    pthread_attr_getdetachstate(attr, &createArg->userDetachState);
  } else {
    createArg->userStack = NULL;
    createArg->userDetachState = PTHREAD_CREATE_JOINABLE;
  }

  log_entry_t my_entry = create_pthread_create_entry(my_clone_id,
                                                     pthread_create_event,
                                                     thread, attr,
                                                     start_routine, arg);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(pthread_create);
    stack_addr = (void *)GET_FIELD(my_entry, pthread_create, stack_addr);
    stack_size = GET_FIELD(my_entry, pthread_create, stack_size);
    dmtcp::ThreadInfo::prePthreadCreate();
    WRAPPER_REPLAY_END(pthread_create);

    // Register a new thread with ThreadInfo.
    pthread_t newPthreadId = GET_FIELD(my_entry, pthread_create, ret_thread);
    // Set up thread stacks to how they were at record time.
    pthread_attr_init(&the_attr);

    setupThreadStack(&the_attr, attr, stack_size);
    // Never let the user create a detached thread:
    disableDetachState(&the_attr);
    createArg->attr = the_attr;
    retval = _real_pthread_create(thread, &the_attr,
                                  start_wrapper, (void *)createArg);
    pthread_attr_destroy(&the_attr);

  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::prePthreadCreate();
    WRAPPER_LOG_WRITE_ENTRY(my_entry);

    pthread_attr_init(&the_attr);
    // Possibly create a thread stack if the user has not provided one:
    setupThreadStack(&the_attr, attr, 0);
    // Never let the user create a detached thread:
    disableDetachState(&the_attr);

    createArg->attr = the_attr;
    retval = _real_pthread_create(thread, &the_attr,
                                  start_wrapper, (void *)createArg);
    my_entry.setRetval((void*)(unsigned long)retval);
    my_entry.setSavedErrno(errno);

    // Log whatever stack we ended up using:
    pthread_attr_getstack(&the_attr, &stack_addr, &stack_size);
    pthread_attr_destroy(&the_attr);
    SET_FIELD(my_entry, pthread_create, stack_addr);
    SET_FIELD(my_entry, pthread_create, stack_size);
    SET_FIELD2(my_entry, pthread_create, ret_thread, *thread);

    // Log annotation on the fly.
    WRAPPER_LOG_UPDATE_ENTRY(my_entry);
  }

  if (retval != 0) {
    dmtcp::ThreadInfo::postPthreadCreate();
  }
  return retval;
}

extern "C" int pthread_mutex_lock(pthread_mutex_t *mutex)
{
  WRAPPER_HEADER_RAW(int, pthread_mutex_lock, _real_pthread_mutex_lock,
                     mutex);

  /* NOTE: Don't call JTRACE (or anything that calls JTRACE) before
    this point. */
  int retval = internal_pthread_mutex_lock(mutex);
  return retval;
}

extern "C" int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
  WRAPPER_HEADER(int, pthread_mutex_trylock, _real_pthread_mutex_trylock,
                 mutex);
  /* NOTE: Don't call JTRACE (or anything that calls JTRACE) before
    this point. */
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY(pthread_mutex_trylock);
  } else if (SYNC_IS_RECORD) {
    retval = _real_pthread_mutex_trylock(mutex);
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
  WRAPPER_HEADER_RAW(int, pthread_mutex_unlock, _real_pthread_mutex_unlock,
                     mutex);
  /* NOTE: Don't call JTRACE (or anything that calls JTRACE) before
    this point. */
  int retval = internal_pthread_mutex_unlock(mutex);
  return retval;
}

extern "C" int pthread_cond_signal(pthread_cond_t *cond)
{
  WRAPPER_HEADER_RAW(int, pthread_cond_signal, _real_pthread_cond_signal,
                     cond);
  int retval = internal_pthread_cond_signal(cond);
  return retval;
}

extern "C" int pthread_cond_broadcast(pthread_cond_t *cond)
{
  WRAPPER_HEADER(int, pthread_cond_broadcast, _real_pthread_cond_broadcast,
                 cond);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY(pthread_cond_broadcast);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_pthread_cond_broadcast(cond);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
  WRAPPER_HEADER_RAW(int, pthread_cond_wait, _real_pthread_cond_wait,
                     cond, mutex);
  int retval = internal_pthread_cond_wait(cond, mutex);
  return retval;
}

extern "C" int pthread_cond_timedwait(pthread_cond_t *cond,
    pthread_mutex_t *mutex, const struct timespec *abstime)
{
  /* See the comments in internal_pthread_cond_wait() for the explanation of
   * the call to FAKE_BASIC_SYNC_WRAPPER()
   */
  FAKE_BASIC_SYNC_WRAPPER(int, pthread_mutex_unlock, mutex);

  WRAPPER_HEADER(int, pthread_cond_timedwait, _real_pthread_cond_timedwait,
                 cond, mutex, abstime);

  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY(pthread_cond_timedwait);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_pthread_cond_timedwait(cond, mutex, abstime);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int pthread_cond_destroy(pthread_cond_t *cond)
{
  WRAPPER_HEADER(int, pthread_cond_destroy, _real_pthread_cond_destroy, cond);

  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY(pthread_cond_destroy);
  } else if (SYNC_IS_RECORD) {
    retval = _real_pthread_cond_destroy(cond);
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
{
  WRAPPER_HEADER(int, pthread_rwlock_unlock, _real_pthread_rwlock_unlock,
                 rwlock);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY(pthread_rwlock_unlock);
  } else if (SYNC_IS_RECORD) {
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
    retval = _real_pthread_rwlock_unlock(rwlock);
    WRAPPER_LOG_UPDATE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
{
  WRAPPER_HEADER(int, pthread_rwlock_rdlock, _real_pthread_rwlock_rdlock,
                 rwlock);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY(pthread_rwlock_rdlock);
  } else if (SYNC_IS_RECORD) {
    retval = _real_pthread_rwlock_rdlock(rwlock);
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
{
  WRAPPER_HEADER(int, pthread_rwlock_wrlock, _real_pthread_rwlock_wrlock,
                 rwlock);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY(pthread_rwlock_wrlock);
  } else if (SYNC_IS_RECORD) {
    retval = _real_pthread_rwlock_wrlock(rwlock);
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

static void *dummy_start_wrapper(void *arg) { return NULL; }

static bool pthread_cancel_initialized = false;
static void initialize_pthread_cancel()
{
  /* libpthread defines pthread_cancel_init() which initializes some internal
   * data structures for future use. The function is called whenever a threads
   * is cancelled (pthread_exit(), return from start etc.). The function makes
   * internal calls to dlopen() etc. If pthread_cancel_init() is not called
   * prior to starting logging, it may cause log divergence as we do not have a
   * mechanism to capture and serialize calls to libc internal dlopen().
   *
   * The fix is to create a short lived thread which would eventually make a
   * call to pthread_cancel_init() while exiting thus solving the problem.
   * Notice that we do not want to create this thread before creating the
   * checkpoint thread as this could lead to races withing libmtcp.so. The
   * safest place is to create the thread after checkpoint thread is created
   * and before starting user main().
   */
  if (!pthread_cancel_initialized) {
    if (!isProcessGDB()) {
      pthread_t pth;
      JASSERT(_real_pthread_create(&pth, NULL, dummy_start_wrapper, NULL) == 0);
      pthread_cancel(pth);
      JASSERT(_real_pthread_join(pth, NULL) == 0);
      dmtcp::ThreadInfo::destroyThread(pth);
      pthread_cancel_initialized = true;
    }
  }
}

extern "C" int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
    void *(*start_routine)(void*), void *arg)
{
  /* Reap Existing Threads */
  dmtcp::ThreadInfo::reapThreads();
  int retval;

  /* Here I've split apart WRAPPER_HEADER_RAW because we need to create the
   * reaper thread even if the current mode is SYNC_IS_NOOP. */
  void *return_addr = GET_RETURN_ADDRESS();
  if (!dmtcp_is_running_state() ||
      !validAddress(return_addr) || isProcessGDB()) {
    retval =  _real_pthread_create(thread, attr, start_routine, arg);
  } else if (SYNC_IS_NOOP) {
    struct create_arg *createArg =
      (struct create_arg*) JALLOC_HELPER_MALLOC(sizeof(*createArg));
    createArg->fn = start_routine;
    createArg->thread_arg = arg;
    createArg->userStack = (void*) -1; // Make sure it is non-NULL
    if (attr != NULL) {
      createArg->attr = *attr;
      pthread_attr_getdetachstate(attr, &createArg->userDetachState);
    } else {
      createArg->userDetachState = PTHREAD_CREATE_JOINABLE;
    }
    dmtcp::ThreadInfo::prePthreadCreate();
    retval = _real_pthread_create(thread, attr, start_wrapper, createArg);
    if (retval != 0) {
      dmtcp::ThreadInfo::postPthreadCreate();
    }
  } else {
    retval = internal_pthread_create(thread, attr, start_routine, arg);
  }
  if (!pthread_cancel_initialized) {
    initialize_pthread_cancel();
  }
  return retval;
}

extern "C" void pthread_exit(void *value_ptr)
{
  if (SYNC_IS_NOOP) {
    dmtcp::ThreadInfo::reapThisThread();
  }
  WRAPPER_HEADER_NO_RETURN(pthread_exit, _real_pthread_exit, value_ptr);

  if (SYNC_IS_REPLAY) {
    waitForTurn(&my_entry, &pthread_exit_turn_check);
    getNextLogEntry();
    dmtcp::ThreadInfo::reapThisThread();
    _real_pthread_exit(value_ptr);
  } else  if (SYNC_IS_RECORD) {
    // Not restart; we should be logging.
    addNextLogEntry(my_entry);
    dmtcp::ThreadInfo::reapThisThread();
    _real_pthread_exit(value_ptr);
  }
  while(1); // to suppress compiler warning about 'noreturn' function returning
}

extern "C" int pthread_detach(pthread_t thread)
{
  int retval = 0;
  void *return_addr = GET_RETURN_ADDRESS();
  if (!shouldSynchronize(return_addr)) {
    int retval = _real_pthread_detach(thread);
    if (retval == 0) {
      dmtcp::ThreadInfo::markDetached(thread);
    }
    return retval;
  }

  log_entry_t my_entry = create_pthread_detach_entry(my_clone_id,
                                                     pthread_detach_event,
                                                     thread);

  //FIXME: We don't need to log/replay this event.
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY(pthread_detach);
  } else  if (SYNC_IS_RECORD) {
    retval = _real_pthread_detach(thread);
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }

  if (retval == 0) {
    dmtcp::ThreadInfo::markDetached(thread);
  }
  return retval;
}

static void *signal_thread(void *arg)
{
  size_t signal_sent_on = 0;
  log_entry_t temp_entry = EMPTY_LOG_ENTRY;
  while (1) {
    // Lock this so it doesn't change from underneath:
    temp_entry = global_log.getCurrentEntry();
    if (__builtin_expect(temp_entry.eventId() == signal_handler_event, 0)) {
      if (signal_sent_on != global_log.currentEntryIndex()) {
        // Only send one signal per sig_handler entry.
        signal_sent_on = global_log.currentEntryIndex();
        clone_id_t clone_id = temp_entry.cloneId();
        _real_pthread_kill(dmtcp::ThreadInfo::cloneIdToPthreadId(clone_id),
                           GET_FIELD(temp_entry, signal_handler, savedSig));
      }
    }
    usleep(20);
  }
  JASSERT(false) .Text("Unreachable");
  return NULL;
}

static void createSignalThread()
{
  pthread_t t;
  internal_pthread_create(&t, NULL, signal_thread, NULL);
}

extern "C" int pthread_kill(pthread_t thread, int sig)
{
  WRAPPER_HEADER(int, pthread_kill, _real_pthread_kill, thread, sig);

  if (__builtin_expect(signal_thread_alive == 0, 0)) {
    // Start the thread who will send signals (only on replay, but we need to
    // start it here so record has same behavior).
    signal_thread_alive = 1;
    createSignalThread();
  }
  if (SYNC_IS_REPLAY) {
    waitForTurn(&my_entry, &pthread_kill_turn_check);
    getNextLogEntry();
    // TODO: Do something better than always returning success.
    retval = 0;//_real_pthread_kill(thread, sig);
  } else  if (SYNC_IS_RECORD) {
    // Not restart; we should be logging.
    addNextLogEntry(my_entry);
    retval = _real_pthread_kill(thread, sig);
  }
  return retval;
}


extern "C" int pthread_join (pthread_t thread, void **value_ptr)
{
  /* Reap Existing Threads */
  //dmtcp::ThreadInfo::reapThreads();

  /* We change things up a bit here. Since we don't allow the user's
     pthread_join() to have an effect, we don't call the mtcp
     "delete_thread_on_pthread_join()" function here unless we decide not to
     synchronize this call to pthread_join().

     We DO need to call it from the thread reaper reapThread(), however, which
     is in pthreadwrappers.cpp. */
  //if (!dmtcp::ThreadInfo::isUserJoinable(thread)) {
    //return EINVAL;
  //}
  void *return_addr = GET_RETURN_ADDRESS();
  if (!shouldSynchronize(return_addr)) {
    int retval = _real_pthread_join(thread, value_ptr);
    if (retval == 0) {
      dmtcp::ThreadInfo::reapThread(thread);
    }
    return retval;
  }

  int retval = 0;
  log_entry_t my_entry = create_pthread_join_entry(my_clone_id,
      pthread_join_event, thread, value_ptr);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(pthread_join);
    int saved_retval = (long) my_entry.retval();
    WRAPPER_REPLAY_END(pthread_join);
    retval = _real_pthread_join(thread, value_ptr);
    JASSERT(retval == saved_retval);
    if (value_ptr != NULL) {
      JASSERT(*value_ptr == GET_FIELD(my_entry, pthread_join, ret_value_ptr));
    }
  } else if (SYNC_IS_RECORD) {
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
    retval = _real_pthread_join(thread, value_ptr);
    if (value_ptr != NULL) {
      SET_FIELD2(my_entry, pthread_join, ret_value_ptr, *value_ptr);
    }
    WRAPPER_LOG_UPDATE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" long int random()
{
  dmtcp::ThreadInfo::setOkToLogNextFnc();
  return rand();
}

extern "C" int rand()
{
  WRAPPER_HEADER(int, rand, _real_rand);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY(rand);
  } else if (SYNC_IS_RECORD) {
    retval = _real_rand();
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

// TODO: We can remove log/replay from srand() once we are confident that we
// have captured all random events used to provide the seed (e.g. time()).
extern "C" void srand(unsigned int seed)
{
  BASIC_SYNC_WRAPPER_VOID(srand, _real_srand, seed);
}
extern "C" void srandom(unsigned int seed)
{
  BASIC_SYNC_WRAPPER_VOID(srand, _real_srand, seed);
}

extern "C" int rand_r(unsigned int *seedp)
{
  dmtcp::ThreadInfo::setOkToLogNextFnc();
  return rand();
}

extern "C" int random_r(struct random_data *buf, int32_t *result)
{
  dmtcp::ThreadInfo::setOkToLogNextFnc();
  *result = rand();
  return 0;
}

extern "C" time_t time(time_t *tloc)
{
  WRAPPER_HEADER(time_t, time, _real_time, tloc);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(time);
    if (retval != (time_t) -1 && tloc != NULL) {
      *tloc = GET_FIELD(my_entry, time, ret_tloc);
    }
    WRAPPER_REPLAY_END(time);
  } else if (SYNC_IS_RECORD) {
    retval = _real_time(tloc);
    if (tloc != NULL) {
      SET_FIELD2(my_entry, time, ret_tloc, *tloc);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int gettimeofday(struct timeval *tv, struct timezone *tz)
{
  WRAPPER_HEADER(int, gettimeofday, _real_gettimeofday, tv, tz);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(gettimeofday);
    if (retval == 0 && tv != NULL) {
      *tv = GET_FIELD(my_entry, gettimeofday, ret_tv);
    }
    if (retval == 0 && tz != NULL) {
      *tz = GET_FIELD(my_entry, gettimeofday, ret_tz);
    }
    WRAPPER_REPLAY_END(gettimeofday);
  } else if (SYNC_IS_RECORD) {
    retval = _real_gettimeofday(tv, tz);
    if (retval == 0 && tv != NULL) {
      SET_FIELD2(my_entry, gettimeofday, ret_tv, *tv);
    }
    if (retval == 0 && tz != NULL) {
      SET_FIELD2(my_entry, gettimeofday, ret_tz, *tz);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int utime(const char *filename, const struct utimbuf *times)
{
  BASIC_SYNC_WRAPPER(int, utime, _real_utime, filename, times);
}

extern "C" int utimes(const char *filename, const struct timeval *times)
{
  BASIC_SYNC_WRAPPER(int, utimes, _real_utimes, filename, times);
}

extern "C" int futimes(int fd, const struct timeval tv[2])
{
  BASIC_SYNC_WRAPPER(int, futimes, _real_futimes, fd, tv);
}

extern "C" int lutimes(const char *filename, const struct timeval tv[2])
{
  BASIC_SYNC_WRAPPER(int, lutimes, _real_lutimes, filename, tv);
}

extern "C" struct tm *localtime(const time_t *timep)
{
  static struct tm result;
  dmtcp::ThreadInfo::setOkToLogNextFnc();
  return localtime_r(timep, &result);
}

extern "C" struct tm *localtime_r(const time_t *timep, struct tm *result)
{
  WRAPPER_HEADER(struct tm *, localtime_r, _real_localtime_r, timep, result);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START_TYPED(struct tm *, localtime_r);
    if (retval != NULL) {
      *retval = GET_FIELD(my_entry, localtime_r, ret_result);
      if (result != NULL) {
        *result = GET_FIELD(my_entry, localtime_r, ret_result);
      }
    }
    WRAPPER_REPLAY_END(localtime_r);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_localtime_r(timep, result);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    if (retval != NULL) {
      SET_FIELD2(my_entry, localtime_r, ret_result, *retval);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int clock_getres(clockid_t clk_id, struct timespec *res)
{
  WRAPPER_HEADER(int, clock_getres, _real_clock_getres, clk_id, res);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(clock_getres);
    if (retval == 0 && res != NULL) {
      *res = GET_FIELD(my_entry, clock_getres, ret_res);
    }
    WRAPPER_REPLAY_END(clock_getres);
  } else if (SYNC_IS_RECORD) {
    retval = _real_clock_getres(clk_id, res);
    if (retval == 0 && res != NULL) {
      SET_FIELD2(my_entry, clock_getres, ret_res, *res);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int clock_gettime(clockid_t clk_id, struct timespec *tp)
{
  WRAPPER_HEADER(int, clock_gettime, _real_clock_gettime, clk_id, tp);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(clock_gettime);
    if (retval == 0 && tp != NULL) {
      *tp = GET_FIELD(my_entry, clock_gettime, ret_tp);
    }
    WRAPPER_REPLAY_END(clock_gettime);
  } else if (SYNC_IS_RECORD) {
    retval = _real_clock_gettime(clk_id, tp);
    if (retval == 0 && tp != NULL) {
      SET_FIELD2(my_entry, clock_gettime, ret_tp, *tp);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}


extern "C" int clock_settime(clockid_t clk_id, const struct timespec *tp)
{
  BASIC_SYNC_WRAPPER(int, clock_settime, _real_clock_settime, clk_id, tp);
}

/* End wrapper code */
