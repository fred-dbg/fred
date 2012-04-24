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

/* Thread locals: */
LIB_PRIVATE __thread clone_id_t my_clone_id = -1;

static volatile clone_id_t global_clone_counter = 0;
static pthread_mutex_t cloneIdLock = PTHREAD_MUTEX_INITIALIZER;

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
  }
}

void dmtcp::ThreadInfo::registerThread(clone_id_t id)
{
  if (id == -1) {
    id = globalCloneCounter();
  }
  JASSERT(cloneIdTbl != NULL);
  ThreadLocalData *thrData = &(*cloneIdTbl)[id];
  thrData->init(id);
}

void dmtcp::ThreadInfo::initThread()
{
  /* Assigning my_clone_id should be the very first thing.*/
  if (my_clone_id == -1) {
    my_clone_id = get_next_clone_id();
  }
  JTRACE ( "Thread start initialization." ) ( my_clone_id );

  JASSERT(cloneIdTbl != NULL);
  if (cloneIdTbl->find(my_clone_id) == cloneIdTbl->end()) {
    registerThread(my_clone_id);
  }
  ThreadLocalData *thrData = &(*cloneIdTbl)[my_clone_id];
  thrData->update(my_clone_id, pthread_self());

  if (SYNC_IS_RECORD) {
    global_log.incrementNumberThreads();
  }

  JTRACE ( "Thread Initialized" ) ( my_clone_id ) ( pthread_self() );
}

void dmtcp::ThreadInfo::destroyThread(pthread_t pth)
{
  JASSERT(pthreadIdTbl != NULL);

  if (pthreadIdTbl->find(pth) != pthreadIdTbl->end()) {
    clone_id_t id  = (*pthreadIdTbl)[pth];
    (*cloneIdTbl)[id].destroy();
    cloneIdTbl->erase(id);
    pthreadIdTbl->erase(pth);
  }
}

void dmtcp::ThreadInfo::resetOnFork()
{
  if (cloneIdTbl == NULL) {
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
}

dmtcp::ThreadLocalData* dmtcp::ThreadInfo::getThreadLocalData(pthread_t id)
{
  JASSERT(pthreadIdTbl->find(id) != pthreadIdTbl->end()) (id);
  return getThreadLocalData((*pthreadIdTbl)[id]);
}

dmtcp::ThreadLocalData* dmtcp::ThreadInfo::getThreadLocalData(clone_id_t id)
{
  if (cloneIdTbl->find(id) == cloneIdTbl->end()) {
    JNOTE("BLAH BLAH");
    sleep(100);
  }
  JASSERT(cloneIdTbl->find(id) != cloneIdTbl->end()) (id);
  return &(*cloneIdTbl)[id];
}

void dmtcp::ThreadInfo::postSuspend()
{
  // Remove the threads which aren't alive anymore.
  PthreadIdTblIt it;
  dmtcp::vector<clone_id_t> stale_clone_ids;
  for (it = pthreadIdTbl->begin(); it != pthreadIdTbl->end(); it++) {
    if (_real_pthread_kill(it->second, 0) != 0) {
      stale_clone_ids.push_back(it->first);
    }
  }
  for (size_t i = 0; i < stale_clone_ids.size(); i++) {
    dmtcp::ThreadLocalData *thrInfo = &(*cloneIdTbl)[stale_clone_ids[i]];
    cloneIdTbl->erase(stale_clone_ids[i]);
    pthreadIdTbl->erase(stale_clone_ids[i]);
  }
}

void dmtcp::ThreadInfo::postRestartResume()
{
  CloneIdTblIt it;
  for (it = cloneIdTbl->begin(); it != cloneIdTbl->end(); it++) {
    clone_id_t id = it->first;
    dmtcp::ThreadLocalData *thrInfo = getThreadLocalData(id);
    sem_init(&thrInfo->sem, 0, 0);
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
  struct timespec ts;
  struct timespec ts_ms = {0, 1 * 1000 * 1000};
  JASSERT(cloneIdTbl->find(my_clone_id) != cloneIdTbl->end()) (my_clone_id);
  dmtcp::ThreadLocalData *thrInfo = getThreadLocalData(my_clone_id);
  sem_t& sem = thrInfo->sem;
  do {
    _real_clock_gettime(CLOCK_REALTIME, &ts);
    TIMESPEC_ADD(&ts, &ts_ms, &ts);
  } while (sem_timedwait(&sem, &ts) != 0);
  JTRACE("WAIT REturned") (my_clone_id);

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
  if (my_clone_id == -1) initThread();
  dmtcp::ThreadLocalData *thrInfo = getThreadLocalData(my_clone_id);
  thrInfo->isOptionalEvent++;
}
void dmtcp::ThreadInfo::unsetOptionalEvent()
{
  if (cloneIdTbl == NULL) {
    return;
  }
  if (my_clone_id == -1) initThread();
  dmtcp::ThreadLocalData *thrInfo = getThreadLocalData(my_clone_id);
  thrInfo->isOptionalEvent--;
}
bool dmtcp::ThreadInfo::isOptionalEvent()
{
  if (cloneIdTbl == NULL) {
    return false;
  }
  if (my_clone_id == -1) initThread();
  dmtcp::ThreadLocalData *thrInfo = getThreadLocalData(my_clone_id);
  return thrInfo->isOptionalEvent != 0;
}
void dmtcp::ThreadInfo::setOkToLogNextFnc()
{
  if (cloneIdTbl == NULL) {
    return;
  }
  if (my_clone_id == -1) initThread();
  dmtcp::ThreadLocalData *thrInfo = getThreadLocalData(my_clone_id);
  thrInfo->isOkToLogNextFnc = true;
}
void dmtcp::ThreadInfo::unsetOkToLogNextFnc()
{
  if (cloneIdTbl == NULL) {
    return;
  }
  if (my_clone_id == -1) initThread();
  dmtcp::ThreadLocalData *thrInfo = getThreadLocalData(my_clone_id);
  thrInfo->isOkToLogNextFnc = false;
}
bool dmtcp::ThreadInfo::isOkToLogNextFnc()
{
  if (cloneIdTbl == NULL) {
    return false;
  }
  if (my_clone_id == -1) initThread();
  dmtcp::ThreadLocalData *thrInfo = getThreadLocalData(my_clone_id);
  return thrInfo->isOkToLogNextFnc != 0;
}
