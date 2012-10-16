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

#include "threadinfo.h"
#include "log.h"

#define GLOBAL_CLONE_COUNTER_INIT 1

using namespace dmtcp;

dmtcp::map<clone_id_t, dmtcp::ThreadLocalData> *dmtcp::ThreadInfo::cloneIdTbl = NULL;

dmtcp::vector<clone_id_t> *dmtcp::ThreadInfo::threadsToBeReaped = NULL;
static sem_t threadReaperSem;
static sem_t threadCreationSem;
static int threadReaperSemValue = 0;
static int threadCreationSemValue = 1;

static __thread dmtcp::ThreadLocalData *_myThreadInfo = NULL;

static dmtcp::ThreadLocalData *myThreadInfo() {
  if (_myThreadInfo == NULL) {
    dmtcp::ThreadInfo::initThread();
  }
  return _myThreadInfo;
}
/* Thread locals: */
LIB_PRIVATE __thread clone_id_t my_clone_id = -1;

static volatile clone_id_t global_clone_counter = 0;
static pthread_mutex_t cloneIdLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t _tblLock = PTHREAD_MUTEX_INITIALIZER;

static void _lockTbl() {
  JASSERT(_real_pthread_mutex_lock(&_tblLock) == 0);
}

static void _unlockTbl() {
  JASSERT(_real_pthread_mutex_unlock(&_tblLock) == 0);
}

static clone_id_t get_next_clone_id()
{
  clone_id_t id;
  _real_pthread_mutex_lock(&cloneIdLock);
  id = global_clone_counter;
  global_clone_counter++;
  _real_pthread_mutex_unlock(&cloneIdLock);
  return id;
}

static clone_id_t globalCloneCounter()
{
  clone_id_t id;
  _real_pthread_mutex_lock(&cloneIdLock);
  id = global_clone_counter;
  _real_pthread_mutex_unlock(&cloneIdLock);
  return id;
}

void dmtcp::ThreadInfo::init()
{
  /* This is called only on exec(). We reset the global clone counter for this
     process, assign the first thread (this one) clone_id 1, and increment the
     counter. */
  JTRACE ( "resetting global clone counter." );
  global_clone_counter = GLOBAL_CLONE_COUNTER_INIT;

  if (cloneIdTbl == NULL) {
    cloneIdTbl = new dmtcp::map<clone_id_t, ThreadLocalData>;
    threadsToBeReaped = new dmtcp::vector<clone_id_t>;
    cloneIdTbl->clear();
    threadsToBeReaped->clear();
    sem_init(&threadReaperSem, 0, threadReaperSemValue);
    sem_init(&threadCreationSem, 0, threadCreationSemValue);
    pthread_mutex_t ll = PTHREAD_MUTEX_INITIALIZER;
    _tblLock = ll;
  }
  registerThread(global_clone_counter);
}

void dmtcp::ThreadInfo::prePthreadCreate()
{
  int ret;
  do {
    ret = sem_wait(&threadCreationSem);
  } while (ret != 0);

  dmtcp::ThreadInfo::registerThread(globalCloneCounter());
}

void dmtcp::ThreadInfo::postPthreadCreate()
{
  JASSERT(sem_post(&threadCreationSem) == 0);
}

void dmtcp::ThreadInfo::registerThread(clone_id_t id)
{
  JASSERT(id != -1);
  _lockTbl();
  JASSERT(cloneIdTbl != NULL);
  if (cloneIdTbl->find(my_clone_id) == cloneIdTbl->end()) {
    ThreadLocalData thrInfo;
    (*cloneIdTbl)[id] = thrInfo;
  }
  _unlockTbl();
}

void dmtcp::ThreadInfo::initThread()
{
  /* Assigning my_clone_id should be the very first thing.*/
  if (my_clone_id == -1) {
    my_clone_id = get_next_clone_id();
    JASSERT(_myThreadInfo == NULL);
  } else {
    JASSERT(my_clone_id == 1) (my_clone_id);
  }
  JTRACE ( "Thread start initialization." ) ( my_clone_id );

  JASSERT(cloneIdTbl != NULL);

  registerThread(my_clone_id);
  _myThreadInfo = &(*cloneIdTbl)[my_clone_id];
  _myThreadInfo->init(my_clone_id, pthread_self());

  if (SYNC_IS_RECORD) {
    global_log.incrementNumberThreads();
  }

  JTRACE ( "Thread Initialized" ) ( my_clone_id ) ( pthread_self() );
}

void dmtcp::ThreadInfo::destroyThread(pthread_t pth)
{
  ThreadLocalData *thrInfo = getThreadLocalData(pth);
  thrInfo->destroy();
  cloneIdTbl->erase(thrInfo->id);
}

void dmtcp::ThreadInfo::updateState(pthread_t  pth,
                                    pthread_attr_t attr,
                                    void      *userStack,
                                    int        userDetachState)
{
  ThreadLocalData *thrInfo = getThreadLocalData(pth);
  thrInfo->userStack = userStack;
  thrInfo->userDetachState = userDetachState;

  /* Do not call pthread_getattr_np from ThreadInfo
   * pthread_getattr_np calls several syscalls from within libc which can not
   * be replayed correctly (such as feof_unlocked).
   */
  pthread_attr_getstack(&attr, &thrInfo->stackAddr, &thrInfo->stackSize);
}

void dmtcp::ThreadInfo::resetOnFork()
{
  if (cloneIdTbl != NULL) {
    cloneIdTbl->clear();
    threadsToBeReaped->clear();
    sem_t tsem;
    threadReaperSem = tsem;
    sem_init(&threadReaperSem, 0, threadReaperSemValue);
    threadCreationSem = tsem;
    sem_init(&threadCreationSem, 0, threadCreationSemValue);
  }

  // This is called only on fork() by the new child process. We reset the
  // global clone counter for this process, assign the first thread (this one)
  // clone_id 1, and increment the counter.
  JTRACE ( "resetting global counter in new process." );
  global_clone_counter = GLOBAL_CLONE_COUNTER_INIT;
  my_clone_id = get_next_clone_id();
  _myThreadInfo = NULL;
  registerThread(my_clone_id);
  initThread();
  pthread_mutex_t ll = PTHREAD_MUTEX_INITIALIZER;
  _tblLock = ll;
}

dmtcp::ThreadLocalData* dmtcp::ThreadInfo::getThreadLocalData(pthread_t pth)
{
  dmtcp::ThreadLocalData *thrInfo = NULL;
  _lockTbl();
  CloneIdTblIt it;
  for (it = cloneIdTbl->begin(); it != cloneIdTbl->end(); it++) {
    thrInfo = &it->second;
    if (pthread_equal(pth, thrInfo->pthreadId)) {
      break;
    }
  }
  JASSERT(it != cloneIdTbl->end());
  _unlockTbl();
  return thrInfo;
}

dmtcp::ThreadLocalData* dmtcp::ThreadInfo::getThreadLocalData(clone_id_t id)
{
  if (my_clone_id == -1) {
    initThread();
    id = my_clone_id;
  }
  ThreadLocalData *thrInfo;

  _lockTbl();
  JASSERT(cloneIdTbl->find(id) != cloneIdTbl->end()) (id);
  thrInfo = &(*cloneIdTbl)[id];
  _unlockTbl();
  return thrInfo;
}

void dmtcp::ThreadInfo::postSuspend()
{
  // No need to acquire table-lock as only checkpoint-thread is executing at
  // the moment.
  // Remove the threads which aren't alive anymore.
  dmtcp::vector<clone_id_t> stale_clone_ids;
  JASSERT(cloneIdTbl->size() != 0);
  CloneIdTblIt it;
  for (it = cloneIdTbl->begin(); it != cloneIdTbl->end(); it++) {
    ThreadLocalData *thrInfo = &it->second;
    if (_real_pthread_kill(thrInfo->pthreadId, 0) != 0) {
      stale_clone_ids.push_back(it->first);
    } else {
      JASSERT(sem_getvalue(&thrInfo->sem, &thrInfo->semValue) == 0);
      thrInfo->destroy();
    }
  }
  for (size_t i = 0; i < stale_clone_ids.size(); i++) {
    dmtcp::ThreadLocalData *thrInfo = &(*cloneIdTbl)[stale_clone_ids[i]];
    cloneIdTbl->erase(stale_clone_ids[i]);
  }

  sem_getvalue(&threadReaperSem, &threadReaperSemValue);
  sem_getvalue(&threadCreationSem, &threadCreationSemValue);
  sem_destroy(&threadReaperSem);
  sem_destroy(&threadCreationSem);
}

void dmtcp::ThreadInfo::postCkptResume()
{
  // No need to acquire table-lock as only checkpoint-thread is executing at
  // the moment.
  CloneIdTblIt it;
  for (it = cloneIdTbl->begin(); it != cloneIdTbl->end(); it++) {
    dmtcp::ThreadLocalData *thrInfo = &(*cloneIdTbl)[it->first];
    sem_init(&thrInfo->sem, 0, thrInfo->semValue);
  }
  sem_init(&threadReaperSem, 0, threadReaperSemValue);
  sem_init(&threadCreationSem, 0, threadCreationSemValue);
}

void dmtcp::ThreadInfo::postRestartResume()
{
  // No need to acquire table-lock as only checkpoint-thread is executing at
  // the moment.
  dmtcp::ThreadLocalData *thrInfo;
  CloneIdTblIt it;
  for (it = cloneIdTbl->begin(); it != cloneIdTbl->end(); it++) {
    thrInfo = &it->second;
    sem_init(&thrInfo->sem, 0, thrInfo->semValue);
  }

  log_entry_t temp_entry = global_log.getCurrentEntry();
  thrInfo = getThreadLocalData(temp_entry.cloneId());
  if (thrInfo->semValue == 0) {
    //JASSERT (global_log.currentEntryIndex() == 0);
    dmtcp::ThreadInfo::wakeUpThread(temp_entry.cloneId());
  }
  sem_init(&threadReaperSem, 0, threadReaperSemValue);
  sem_init(&threadCreationSem, 0, threadCreationSemValue);
}

clone_id_t nextThread = -1;
void dmtcp::ThreadInfo::wakeUpThread(clone_id_t id)
{
  global_log.checkForBreakpoint();

  dmtcp::ThreadLocalData *thrInfo = getThreadLocalData(id);
  JTRACE("Posting") (id) (my_clone_id);
  nextThread = id;
  sem_post(&thrInfo->sem);
}

clone_id_t waitThread=-1;
void dmtcp::ThreadInfo::waitForTurn()
{
  int res;
  struct timespec ts;
  struct timespec ts_ms = {0, 1 * 1000 * 1000};
  JASSERT(myThreadInfo() != NULL);
  sem_t& sem = myThreadInfo()->sem;
  do {
    //DMTCP_PLUGIN_DISABLE_CKPT();
    _real_clock_gettime(CLOCK_REALTIME, &ts);
    TIMESPEC_ADD(&ts, &ts_ms, &ts);
    res = sem_timedwait(&sem, &ts);
    //DMTCP_PLUGIN_ENABLE_CKPT();
  } while (res != 0);
  JTRACE("sem_wait returned") (my_clone_id);

  waitThread = my_clone_id;
}

pthread_t dmtcp::ThreadInfo::cloneIdToPthreadId(clone_id_t clone_id)
{
  dmtcp::ThreadLocalData *tdata = getThreadLocalData(clone_id);
  return tdata->pthreadId;
}

void dmtcp::ThreadInfo::setOptionalEvent()
{
  if (cloneIdTbl == NULL) {
    return;
  }
  myThreadInfo()->isOptionalEvent++;
}
void dmtcp::ThreadInfo::unsetOptionalEvent()
{
  if (cloneIdTbl == NULL) {
    return;
  }
  myThreadInfo()->isOptionalEvent--;
}
bool dmtcp::ThreadInfo::isOptionalEvent()
{
  if (cloneIdTbl == NULL) {
    return false;
  }
  return myThreadInfo()->isOptionalEvent != 0;
}
void dmtcp::ThreadInfo::setOkToLogNextFnc()
{
  if (cloneIdTbl == NULL) {
    return;
  }
  myThreadInfo()->isOkToLogNextFnc = true;
}
void dmtcp::ThreadInfo::unsetOkToLogNextFnc()
{
  if (cloneIdTbl == NULL) {
    return;
  }
  myThreadInfo()->isOkToLogNextFnc = false;
}
bool dmtcp::ThreadInfo::isOkToLogNextFnc()
{
  if (cloneIdTbl == NULL) {
    return false;
  }
  return myThreadInfo()->isOkToLogNextFnc != 0;
}

bool dmtcp::ThreadInfo::isUserJoinable(pthread_t pth)
{
  dmtcp::ThreadLocalData *thrInfo = getThreadLocalData(pth);
  return thrInfo->userDetachState == PTHREAD_CREATE_JOINABLE;
}

void dmtcp::ThreadInfo::reapThisThread()
{
  // Reap existing threads before quitting
  //reapThreads();

  /*
    Called from two places:
     - pthread_exit() wrapper
     - end of start_wrapper() (which calls user's start function).

    We call the internal_* versions here because we want them to be
    logged/replayed, but we want to skip the shouldSynchronize() function. That
    function will refuse to log/replay these because they are not coming from
    user code.
  */

  if (myThreadInfo()->userDetachState == PTHREAD_CREATE_DETACHED) {
    _lockTbl();
    threadsToBeReaped->push_back(my_clone_id);
    _unlockTbl();
    sem_post(&threadReaperSem);
  }
}

void dmtcp::ThreadInfo::reapThreads()
{
  while (sem_trywait(&threadReaperSem) == 0) {
    _lockTbl();
    JASSERT(threadsToBeReaped->size() >= 1);
    clone_id_t reapId = threadsToBeReaped->back();
    threadsToBeReaped->pop_back();
    _unlockTbl();

    ThreadLocalData *thrInfo = getThreadLocalData(reapId);
    JASSERT(thrInfo != NULL);

    int ret;
    if (SYNC_IS_NOOP) {
      ret = pthread_join(thrInfo->pthreadId, NULL);
      reapThread(thrInfo->pthreadId);
    } else if (SYNC_IS_REPLAY) {
      //ret = pthread_join(thrInfo->pthreadId, NULL);
    } else  if (SYNC_IS_RECORD) {
      setOptionalEvent();
      ret = pthread_join(thrInfo->pthreadId, NULL);
      reapThread(thrInfo->pthreadId);
      unsetOptionalEvent();
    }

#if 0
    if (ret != 0) {
      _lockTbl();
      threadsToBeReaped->push_back(reapId);
      _unlockTbl();
      sem_post(&threadReaperSem);
      break;
    }
#endif
  }
}

void dmtcp::ThreadInfo::reapThread(pthread_t pth)
{
  ThreadLocalData *thrInfo = getThreadLocalData(pth);
  JASSERT(thrInfo != NULL);
  if (thrInfo->userStack == NULL) {
    void *stackAddr = thrInfo->stackAddr;
    size_t stackSize = thrInfo->stackSize;
    dmtcp::ThreadInfo::destroyThread(pth);
    JASSERT(munmap(stackAddr, stackSize) == 0)
      (JASSERT_ERRNO) (pth) (stackAddr) (stackSize)
      .Text("Unable to munmap user thread stack.");
  } else {
    dmtcp::ThreadInfo::destroyThread(pth);
  }
}

void dmtcp::ThreadInfo::markDetached(pthread_t pth)
{
  bool flag = false;
  _lockTbl();
  dmtcp::ThreadLocalData *thrInfo;
  CloneIdTblIt it;
  for (it = cloneIdTbl->begin(); it != cloneIdTbl->end(); it++) {
    ThreadLocalData *thrInfo = &it->second;
    if (pthread_equal(pth, thrInfo->pthreadId)) {
      thrInfo->userDetachState = PTHREAD_CREATE_DETACHED;
      if (_real_pthread_kill(pth, 0) != 0) {
        threadsToBeReaped->push_back(thrInfo->id);
        flag = true;
      }
      // FIXME: What if the thread has died already.
    }
  }
  _unlockTbl();
  if (flag) {
    sem_post(&threadReaperSem);
  }
}
