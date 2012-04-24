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

#ifndef THREAD_INFO_H
#define THREAD_INFO_H

#include <pthread.h>
#include "constants.h"
#include "dmtcpalloc.h"
#include "util.h"
#include "dmtcpplugin.h"
#include "jassert.h"
#include "jfilesystem.h"
#include "fred_wrappers.h"
#include "synchronizationlogging.h"

namespace dmtcp {
  struct ThreadLocalData {
    void init(clone_id_t _id) {
      id = _id;
      pthreadId = -1;
      mmap_no_sync = 0;
      in_mmap_wrapper = 0;
      isOptionalEvent = 0;
      isOkToLogNextFnc = 0;
      sem_init(&sem, 0 , 0);
    }

    void update(clone_id_t _id, pthread_t _pth) {
      JASSERT(id == _id) (id) (_id);
      pthreadId = _pth;
    }

    void destroy() {
      sem_destroy(&sem);
    }

    clone_id_t id;
    pthread_t pthreadId;

    int mmap_no_sync;
    int in_mmap_wrapper;
    unsigned char isOptionalEvent;
    bool isOkToLogNextFnc;
    // Semaphore used for waiting-for-turn and waking up threads according to
    // log entry.
    sem_t sem;
  };

  namespace ThreadInfo {

    void init();
    void registerThread(clone_id_t id = -1);
    void destroyThread(pthread_t pth);
    void initThread();
    void resetOnFork();

    ThreadLocalData* getThreadLocalData(pthread_t pth);
    ThreadLocalData* getThreadLocalData(clone_id_t id);
    pthread_t cloneIdToPthreadId(clone_id_t clone_id);

    void postSuspend();
    void postRestartResume();

    void wakeUpThread(clone_id_t id);
    void waitForTurn();

    void setOptionalEvent();
    void unsetOptionalEvent();
    bool isOptionalEvent();

    void setOkToLogNextFnc();
    void unsetOkToLogNextFnc();
    bool isOkToLogNextFnc();

    extern dmtcp::map<clone_id_t, ThreadLocalData> *cloneIdTbl;
    extern dmtcp::map<pthread_t,  clone_id_t> *pthreadIdTbl;
    typedef dmtcp::map<clone_id_t, ThreadLocalData>::iterator CloneIdTblIt;
    typedef dmtcp::map<pthread_t,  clone_id_t>::iterator PthreadIdTblIt;
  };
};

#endif
