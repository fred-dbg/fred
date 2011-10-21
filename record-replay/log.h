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

#ifndef LOG_H
#define LOG_H

#include "constants.h"
#include "jalloc.h"
#include "dmtcpalloc.h"
#include "jassert.h"
#include "synchronizationlogging.h"
#include "fred_interface.h"
#include <unistd.h>

#define DMTCP_PAGE_SIZE sysconf(_SC_PAGESIZE)

/* This offset is how far from the beginning of the mmapped region the actual
   log starts. I.e. the LogMetadata struct lives within the first
   LOG_OFFSET_FROM_START bytes of the log. Making this a full page so that
   later we can reuse some smaller mmapped region for the log, and simply
   recycle over the same memory region. For that, it is helpful to have a
   page-aligned offset for the start of the log contents. */

#define LOG_OFFSET_FROM_START DMTCP_PAGE_SIZE

namespace dmtcp
{
  typedef struct LogMetadata {
    size_t size;
    size_t dataSize;
    size_t numEntries;
    size_t numThreads;
    void * recordedStartAddr;
  } LogMetadata;

  class SynchronizationLog
  {
    public:
#ifdef JALIB_ALLOCATOR
      static void* operator new(size_t nbytes, void* p) { return p; }
      static void* operator new(size_t nbytes) { JALLOC_HELPER_NEW(nbytes); }
      static void  operator delete(void* p) { JALLOC_HELPER_DELETE(p); }
#endif
      SynchronizationLog()
        : _path ("")
        , _cloneId(-1)
        , _startAddr (NULL)
        , _recordedStartAddr (NULL)
        , _log (NULL)
        , _index (0)
        , _entryIndex (0)
        , _size (NULL)
        , _dataSize (NULL)
        , _numEntries (NULL)
        , _numThreads (NULL)
        , _sharedInterfaceInfo (NULL)
        , _entryOffsetMarker (0)
        , _entryIndexMarker (0)
      {}

      ~SynchronizationLog() {}

      void initialize(const char* path, size_t size = MAX_LOG_LENGTH);

    private:
      void init_common(size_t size);
      void init_shm();
      void destroy_shm();

    public:
      void   destroy(int mode);
      void   unmap();
      void   map_in(const char *path, size_t size,
		    bool mapWithNoReserveFlag);
      void   map_in();
      size_t currentEntryIndex() { return _entryIndex; }
      bool   empty() { return numEntries() == 0; }
      size_t getIndex();
      size_t getDataSize();
      void   setDataSize(log_off_t newVal);
      void   incrementNumberThreads();
      size_t numEntries() { return _numEntries == NULL ? 0 : *_numEntries; }
      void * getRecordedStartAddr() { return _recordedStartAddr == NULL ? NULL : *_recordedStartAddr; }
      bool   isMappedIn() { return _startAddr != NULL; }
      string getPath() { return _path; }
      void   mergeLogs(dmtcp::vector<clone_id_t> clone_ids);

      int    advanceToNextEntry();
      int    getCurrentEntry(log_entry_t& entry);
      void   appendEntry(log_entry_t& entry);
      void   updateEntry(const log_entry_t& entry);
      void   moveMarkersToEnd();

    private:
      void   resetIndex() { _index = 0; _entryIndex = 0; }
      void   resetMarkers()
      { resetIndex(); *_dataSize = 0; *_numEntries = 0; *_numThreads = 0; }

      int    writeEntryAtOffset(const log_entry_t& entry, size_t index);
      void   writeEntryHeaderAtOffset(const log_entry_t& entry, size_t index);
      size_t getEntryHeaderAtOffset(log_entry_t& entry, size_t index);
      int    getEntryAtOffset(log_entry_t& entry, size_t index);

      inline log_off_t atomicIncrementOffset(log_off_t delta);
      size_t atomicIncrementIndex(log_off_t delta);
      size_t atomicIncrementEntryIndex();

    private:
      string  _path;
      clone_id_t _cloneId;
      char   *_startAddr;
      char   *_log;
      size_t  _index;
      size_t  _entryIndex;
      size_t *_size;
      size_t _savedSize;   // Only used between checkpoints
      size_t *_dataSize;   // Must be modified atomically.
      size_t *_numEntries; // Must be modified atomically.
      size_t *_numThreads; // Must be modified atomically.
      void ** _recordedStartAddr;
      fred_interface_info_t *_sharedInterfaceInfo;
      size_t _entryOffsetMarker;
      size_t _entryIndexMarker;
  };

}
#endif
