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

using namespace dmtcp;

dmtcp::map<clone_id_t, dmtcp::ThreadLocalData> *dmtcp::ThreadInfo::cloneIdTbl = NULL;
dmtcp::map<pthread_t,  clone_id_t> *dmtcp::ThreadInfo::pthreadIdTbl = NULL;


void dmtcp::ThreadInfo::init()
{
  if (cloneIdTbl == NULL) {
    cloneIdTbl = new dmtcp::map<clone_id_t, ThreadLocalData>;
    pthreadIdTbl = new dmtcp::map<pthread_t, clone_id_t>;
    cloneIdTbl->clear();
    pthreadIdTbl->clear();
  }
}

void dmtcp::ThreadInfo::initThread(clone_id_t id)
{
  JASSERT(cloneIdTbl != NULL);
  ThreadLocalData *thrData = &(*cloneIdTbl)[id];
  thrData->init(id);
}

void dmtcp::ThreadInfo::updateThread(clone_id_t id, pthread_t pth)
{
  JASSERT(cloneIdTbl != NULL);
  if (cloneIdTbl->find(id) == cloneIdTbl->end()) {
    initThread(id);
  }
  ThreadLocalData *thrData = &(*cloneIdTbl)[id];
  thrData->update(id, pth);
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
  dmtcp::ThreadLocalData *thrInfo = getThreadLocalData(id);
  JNOTE("Posting") (id) (my_clone_id);
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

void dmtcp::ThreadInfo::setInMmapWrapper()
{
  if (cloneIdTbl == NULL) {
    return;
  }
  if (my_clone_id == -1) initialize_thread();
  dmtcp::ThreadLocalData *thrInfo = getThreadLocalData(my_clone_id);
  thrInfo->in_mmap_wrapper = 1;
}

void dmtcp::ThreadInfo::unsetInMmapWrapper()
{
  if (cloneIdTbl == NULL) {
    return;
  }
  if (my_clone_id == -1) initialize_thread();
  dmtcp::ThreadLocalData *thrInfo = getThreadLocalData(my_clone_id);
  thrInfo->in_mmap_wrapper = 0;
}

bool dmtcp::ThreadInfo::isInMmapWrapper()
{
  if (cloneIdTbl == NULL) {
    return false;
  }
  if (my_clone_id == -1) initialize_thread();
  dmtcp::ThreadLocalData *thrInfo = getThreadLocalData(my_clone_id);
  thrInfo->in_mmap_wrapper == 1;
}

void dmtcp::ThreadInfo::setOptionalEvent()
{
  if (cloneIdTbl == NULL) {
    return;
  }
  if (my_clone_id == -1) initialize_thread();
  dmtcp::ThreadLocalData *thrInfo = getThreadLocalData(my_clone_id);
  thrInfo->isOptionalEvent++;
}
void dmtcp::ThreadInfo::unsetOptionalEvent()
{
  if (cloneIdTbl == NULL) {
    return;
  }
  if (my_clone_id == -1) initialize_thread();
  dmtcp::ThreadLocalData *thrInfo = getThreadLocalData(my_clone_id);
  thrInfo->isOptionalEvent--;
}
bool dmtcp::ThreadInfo::isOptionalEvent()
{
  if (cloneIdTbl == NULL) {
    return false;
  }
  if (my_clone_id == -1) initialize_thread();
  dmtcp::ThreadLocalData *thrInfo = getThreadLocalData(my_clone_id);
  return thrInfo->isOptionalEvent != 0;
}
void dmtcp::ThreadInfo::setOkToLogNextFnc()
{
  if (cloneIdTbl == NULL) {
    return;
  }
  if (my_clone_id == -1) initialize_thread();
  dmtcp::ThreadLocalData *thrInfo = getThreadLocalData(my_clone_id);
  thrInfo->isOkToLogNextFnc = true;
}
void dmtcp::ThreadInfo::unsetOkToLogNextFnc()
{
  if (cloneIdTbl == NULL) {
    return;
  }
  if (my_clone_id == -1) initialize_thread();
  dmtcp::ThreadLocalData *thrInfo = getThreadLocalData(my_clone_id);
  thrInfo->isOkToLogNextFnc = false;
}
bool dmtcp::ThreadInfo::isOkToLogNextFnc()
{
  if (cloneIdTbl == NULL) {
    return false;
  }
  if (my_clone_id == -1) initialize_thread();
  dmtcp::ThreadLocalData *thrInfo = getThreadLocalData(my_clone_id);
  return thrInfo->isOkToLogNextFnc != 0;
}
