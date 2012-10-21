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
dmtcp::map<pthread_t,  clone_id_t> *dmtcp::ThreadInfo::pthreadIdTbl = NULL;

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
    pthreadIdTbl = new dmtcp::map<pthread_t, clone_id_t>;
    cloneIdTbl->clear();
    pthreadIdTbl->clear();
    pthread_mutex_t ll = PTHREAD_MUTEX_INITIALIZER;
    _tblLock = ll;
  }
}

void dmtcp::ThreadInfo::registerThread(clone_id_t id, pthread_t pth)
{
  if (id == -1) {
    id = globalCloneCounter();
  } else {
    JASSERT(pth == -1);
    pth = pthread_self();
  }
  _lockTbl();
  JASSERT(cloneIdTbl != NULL);
  ThreadLocalData *thrData = &(*cloneIdTbl)[id];
  (*pthreadIdTbl)[pth] = id;
  thrData->init(id);
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

  _lockTbl();
  JASSERT(cloneIdTbl != NULL);
  if (cloneIdTbl->find(my_clone_id) == cloneIdTbl->end()) {
    JASSERT(pthreadIdTbl->find(pthread_self()) == pthreadIdTbl->end())
      (pthread_self());
    _unlockTbl();
    registerThread(my_clone_id);
    _lockTbl();
  }
  JASSERT(pthreadIdTbl->find(pthread_self()) != pthreadIdTbl->end())
    (pthread_self());
  _myThreadInfo = &(*cloneIdTbl)[my_clone_id];
  _myThreadInfo->update(my_clone_id, pthread_self());
  _unlockTbl();

  if (SYNC_IS_RECORD) {
    global_log.incrementNumberThreads();
  }

  JTRACE ( "Thread Initialized" ) ( my_clone_id ) ( pthread_self() );
}

void dmtcp::ThreadInfo::destroyThread(pthread_t pth)
{
  _lockTbl();
  JASSERT(pthreadIdTbl != NULL);

  if (pthreadIdTbl->find(pth) != pthreadIdTbl->end()) {
    clone_id_t id  = (*pthreadIdTbl)[pth];
    (*cloneIdTbl)[id].destroy();
    cloneIdTbl->erase(id);
    pthreadIdTbl->erase(pth);
  }
  _unlockTbl();
}

void dmtcp::ThreadInfo::resetOnFork()
{
  if (cloneIdTbl != NULL) {
    cloneIdTbl->clear();
    pthreadIdTbl->clear();
  }

  // This is called only on fork() by the new child process. We reset the
  // global clone counter for this process, assign the first thread (this one)
  // clone_id 1, and increment the counter.
  JTRACE ( "resetting global counter in new process." );
  global_clone_counter = GLOBAL_CLONE_COUNTER_INIT;
  my_clone_id = get_next_clone_id();
  initThread();
  pthread_mutex_t ll = PTHREAD_MUTEX_INITIALIZER;
  _tblLock = ll;
}

dmtcp::ThreadLocalData* dmtcp::ThreadInfo::getThreadLocalData(pthread_t pth)
{
  clone_id_t id;
  _lockTbl();
  JASSERT(pthreadIdTbl->find(pth) != pthreadIdTbl->end()) (pth);
  id = (*pthreadIdTbl)[pth];
  _unlockTbl();
  return getThreadLocalData(id);
}

dmtcp::ThreadLocalData* dmtcp::ThreadInfo::getThreadLocalData(clone_id_t id,
                                                              bool initialize)
{
  if (my_clone_id == -1 && initialize) {
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
  PthreadIdTblIt it;
  dmtcp::vector<clone_id_t> stale_clone_ids;
  JASSERT(pthreadIdTbl->size() != 0);
  for (it = pthreadIdTbl->begin(); it != pthreadIdTbl->end(); it++) {
    if (_real_pthread_kill(it->first, 0) != 0) {
      stale_clone_ids.push_back(it->second);
    } else {
      dmtcp::ThreadLocalData *thrInfo = &(*cloneIdTbl)[it->second];
      JASSERT(sem_getvalue(&thrInfo->sem, &thrInfo->semValue) == 0);
      thrInfo->destroy();
    }
  }
  for (size_t i = 0; i < stale_clone_ids.size(); i++) {
    dmtcp::ThreadLocalData *thrInfo = &(*cloneIdTbl)[stale_clone_ids[i]];
    cloneIdTbl->erase(stale_clone_ids[i]);
    pthreadIdTbl->erase(stale_clone_ids[i]);
  }
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
}

void dmtcp::ThreadInfo::postRestartResume()
{
  // No need to acquire table-lock as only checkpoint-thread is executing at
  // the moment.
  dmtcp::ThreadLocalData *thrInfo;
  CloneIdTblIt it;
  for (it = cloneIdTbl->begin(); it != cloneIdTbl->end(); it++) {
    clone_id_t id = it->first;
    thrInfo = getThreadLocalData(id);
    sem_init(&thrInfo->sem, 0, thrInfo->semValue);
  }

  log_entry_t temp_entry = global_log.getCurrentEntry();
  thrInfo = getThreadLocalData(temp_entry.cloneId());
  if (thrInfo->semValue == 0) {
    //JASSERT (global_log.currentEntryIndex() == 0);
    dmtcp::ThreadInfo::wakeUpThread(temp_entry.cloneId());
  }
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
