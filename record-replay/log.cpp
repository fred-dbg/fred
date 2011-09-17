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

/*
 * TODO:
 *
 * 1. How is my_clone_id handled in case of exec()? How do we make sure that we
 *    still write to the same log assigned for this thread ?
 * 2. Get rid of all XXX_return events.
 *
 */

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "constants.h"
#include "log.h"
#include "synchronizationlogging.h"
#include "fred_wrappers.h"
#include "util.h"
#include "jassert.h"

void dmtcp::SynchronizationLog::initialize(const char *path, size_t size)
{
  bool mapWithNoReserveFlag = SYNC_IS_RECORD;

  JASSERT(_startAddr == NULL);
  JASSERT(_log == NULL);
  JASSERT(_index == 0);
  JASSERT(_size == NULL);
  JASSERT(_dataSize == NULL);
  JASSERT(_entryIndex == 0);
  JASSERT(_numEntries == NULL);

  /* map_in calls init_common if appropriate. */
  map_in(path, size, mapWithNoReserveFlag);

  JTRACE ("Initialized global synchronization log path to" )
    (_path) ((long)_startAddr) (*_size) (mapWithNoReserveFlag);
}

void dmtcp::SynchronizationLog::init_common(size_t size)
{
  JASSERT(sizeof (LogMetadata) < LOG_OFFSET_FROM_START) (sizeof(LogMetadata));

  LogMetadata *metadata = (LogMetadata *) _startAddr;

  _numEntries = &(metadata->numEntries);
  _dataSize = &(metadata->dataSize);
  _size = &(metadata->size);
  _recordedStartAddr = &(metadata->recordedStartAddr);

  *_size = size;
  _log = _startAddr + LOG_OFFSET_FROM_START;

  if (SYNC_IS_RECORD) {
    JASSERT(_startAddr != NULL && _startAddr != MAP_FAILED);
    JTRACE("RECORD; filling in _recordedStartAddr.") ((long)_startAddr);
    *_recordedStartAddr = _startAddr;
  }
}

void dmtcp::SynchronizationLog::destroy()
{
  if (_startAddr != NULL) {
    unmap();
  }
  _startAddr = _log = NULL;
  _path = "";
  _size = NULL;
  _index = 0;
  _entryIndex = 0;
  _dataSize = NULL;
  _numEntries = NULL;
}

void dmtcp::SynchronizationLog::unmap()
{
  if (_startAddr == NULL) {
    return;
  }
  // Save the size in case we want to remap after this unmap:
  _savedSize = *_size;
  JASSERT(_real_munmap(_startAddr, *_size) == 0) (JASSERT_ERRNO) (*_size) (_startAddr);
}

void dmtcp::SynchronizationLog::map_in(const char *path, size_t size,
                                       bool mapWithNoReserveFlag)
{
#if 0
  bool created = false;
  struct stat buf;
  if (stat(path, &buf) == -1 && errno == ENOENT) {
    created = true;
    /* Make sure to clear old state, if this is not the first checkpoint.
       This case can happen (not first checkpoint, but create the log file)
       if log files have been moved or deleted. */
    _startAddr = NULL;
    destroy();
  }
#endif
  int fd = _real_open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  JASSERT(path==NULL || fd != -1);
  if (SYNC_IS_RECORD) {
    JASSERT(fd == -1 || _real_lseek(fd, size, SEEK_SET) == (off_t)size);
    if (fd != -1) Util::writeAll(fd, "", 1);
  }
  // FIXME: Instead of MAP_NORESERVE, we may also choose to back it with
  // /dev/null which would also _not_ allocate pages until needed.
  int mmapProt = PROT_READ | PROT_WRITE;
  int mmapFlags;
  void *mmapAddr = NULL;

  if (fd != -1) {
    mmapFlags = MAP_SHARED;
  } else {
    mmapFlags = MAP_PRIVATE | MAP_ANONYMOUS;
  }
  if (mapWithNoReserveFlag) {
    mmapFlags |= MAP_NORESERVE;
  }
  if (SYNC_IS_REPLAY) {
    void *tempAddr = _real_mmap(NULL, LOG_OFFSET_FROM_START, mmapProt,
                                mmapFlags, fd, 0);
    JASSERT(tempAddr != MAP_FAILED);
    LogMetadata *tempMetadata = (LogMetadata *) tempAddr;
    mmapAddr = tempMetadata->recordedStartAddr;
    JASSERT(mmapAddr != NULL);
    mmapFlags |= MAP_FIXED;
    _real_munmap(tempAddr, LOG_OFFSET_FROM_START);
  }

  SET_IN_MMAP_WRAPPER();
  /* _startAddr may not be null if this is not the first checkpoint. */
  if (_startAddr == NULL) {
    _startAddr = (char*) _real_mmap(mmapAddr, size, mmapProt, mmapFlags, fd, 0);
  } else {
    // XXX: Is there any bad interaction between mmapAddr being set above,
    // and this branch?
    void *retval = (char*) _real_mmap(_startAddr, size, mmapProt,
                                      mmapFlags | MAP_FIXED, fd, 0);
    JASSERT ( retval == (void *)_startAddr );
  }
  UNSET_IN_MMAP_WRAPPER();
  JASSERT(_startAddr != MAP_FAILED) (JASSERT_ERRNO);
  _real_close(fd);
  _path = path == NULL ? "" : path;
#if 0
  if (created || _size == NULL) {
    /* We either had to create the file, or this is the first checkpoint. */
    init_common(size);
  }
#else
  init_common(size);
#endif
}

void dmtcp::SynchronizationLog::map_in()
{
  char path_copy[RECORD_LOG_PATH_MAX] = {'\0'};
  strncpy(path_copy, _path.c_str(), RECORD_LOG_PATH_MAX);
  /* We don't want to pass a pointer to _path, because that could be reset
     as part of a subsequent call to destroy(). */
  map_in(path_copy, _savedSize, false);
}

/* Truncate the log to the current position. */
void dmtcp::SynchronizationLog::truncate()
{
  JTRACE ( "Truncating log to current position." )
    ( _path ) ( _entryIndex ) ( getIndex() );
  memset(&_log[getIndex()], 0, getDataSize() - getIndex());
  // Note that currently _size is constant, so we don't modify it here.
  *_numEntries = _entryIndex;
  setDataSize(getIndex());
}

int dmtcp::SynchronizationLog::advanceToNextEntry()
{
  log_entry_t temp_entry = EMPTY_LOG_ENTRY;
  int entrySize = getEntryAtOffset(temp_entry, getIndex());
  if (entrySize != 0) {
    atomicIncrementIndex(entrySize);
    atomicIncrementEntryIndex();
  }
  return entrySize;
}

int dmtcp::SynchronizationLog::getCurrentEntry(log_entry_t& entry)
{
  int entrySize = getEntryAtOffset(entry, getIndex());
  return entrySize;  
}

// Reads the entry from log and returns the length of entry
int dmtcp::SynchronizationLog::getEntryAtOffset(log_entry_t& entry, size_t index)
{
  size_t currentDataSize = getDataSize();
  if (index == currentDataSize || getEntryHeaderAtOffset(entry, index) == 0) {
    entry = EMPTY_LOG_ENTRY;
    return 0;
  }

  size_t event_size;
  JASSERT(GET_COMMON(entry, event) > 0);
  GET_EVENT_SIZE(GET_COMMON(entry, event), event_size);

  if (index + log_event_common_size + event_size > currentDataSize) {
    JASSERT ((index + log_event_common_size + event_size) <= currentDataSize)
      (index) (log_event_common_size) (event_size) (currentDataSize);
  }

#if 1
  READ_ENTRY_FROM_LOG(&_log[index + log_event_common_size], entry);
#else
  void *ptr;
  GET_EVENT_DATA_PTR(entry, ptr);
  memcpy(ptr, &_log[index + log_event_common_size], event_size);
#endif

  return log_event_common_size + event_size;
}

void dmtcp::SynchronizationLog::appendEntry(log_entry_t& entry)
{
  int eventSize = -1;
  log_off_t offset;

  GET_EVENT_SIZE(GET_COMMON(entry, event), eventSize);
  JASSERT( eventSize > 0 );
  eventSize += log_event_common_size;
  offset = atomicIncrementOffset(eventSize);  
  __sync_fetch_and_add(_numEntries, 1);
  SET_COMMON2(entry, log_offset, offset);

  JASSERT(eventSize == writeEntryAtOffset(entry, offset));
}

void dmtcp::SynchronizationLog::updateEntry(const log_entry_t& entry)
{
  // only allow it for pthread_create and malloc calls
  JASSERT(GET_COMMON(entry, event) == pthread_create_event ||
	  GET_COMMON(entry, event) == pthread_rwlock_unlock_event ||
	  GET_COMMON(entry, event) == pthread_mutex_unlock_event ||
	  GET_COMMON(entry, event) == malloc_event ||
	  GET_COMMON(entry, event) == libc_memalign_event ||
	  GET_COMMON(entry, event) == calloc_event ||
	  GET_COMMON(entry, event) == realloc_event);

#ifdef DEBUG
  log_entry_t old_entry = EMPTY_LOG_ENTRY;
  JASSERT(getEntryAtOffset(old_entry, GET_COMMON(entry, log_offset)) != 0);

  // Only allow replacing events of the same type. Allowing it for differing
  // types (which means differing sizes) would take more work.
  JASSERT(GET_COMMON(entry, event) == GET_COMMON(old_entry, event));

  if (GET_COMMON(entry, event) == pthread_create_event) {
    JASSERT(GET_COMMON(entry, log_offset) == GET_COMMON(old_entry, log_offset) &&
	    IS_EQUAL_FIELD(entry, old_entry, pthread_create, thread) &&
	    IS_EQUAL_FIELD(entry, old_entry, pthread_create, thread) &&
	    IS_EQUAL_FIELD(entry, old_entry, pthread_create, start_routine) &&
	    IS_EQUAL_FIELD(entry, old_entry, pthread_create, attr) &&
	    IS_EQUAL_FIELD(entry, old_entry, pthread_create, arg));
  } else if (GET_COMMON(entry, event) == malloc_event) {
    JASSERT(GET_COMMON(entry, log_offset) == GET_COMMON(old_entry, log_offset) &&
	    IS_EQUAL_FIELD(entry, old_entry, malloc, size));
  }
#endif

  writeEntryAtOffset(entry, GET_COMMON(entry, log_offset));
}

/* Move appropriate markers to the end, so that we enter "append" mode. */
void dmtcp::SynchronizationLog::moveMarkersToEnd()
{
  _index = getDataSize();
  _entryIndex = *_numEntries;
}

int dmtcp::SynchronizationLog::writeEntryAtOffset(const log_entry_t& entry,
                                                  size_t index)
{
  if (__builtin_expect(_startAddr == 0, 0)) {
    JASSERT(false);
  }

  int event_size = -1;
  GET_EVENT_SIZE(GET_COMMON(entry, event), event_size);
  JASSERT( event_size > 0 );

  JASSERT ((LOG_OFFSET_FROM_START + index +
            log_event_common_size + event_size) < *_size)
    ( *_size ) .Text ("Log size too small. Please increase MAX_LOG_LENGTH"
                      " in synchronizationlogging.h");

  writeEntryHeaderAtOffset(entry, index);

#if 1
  WRITE_ENTRY_TO_LOG(&_log[index + log_event_common_size], entry);
#else
  void *ptr;
  GET_EVENT_DATA_PTR(entry, ptr);
  memcpy(&_log[index + log_event_common_size], ptr, event_size);
#endif

  return log_event_common_size + event_size;
}

size_t dmtcp::SynchronizationLog::getEntryHeaderAtOffset(log_entry_t& entry,
                                                      size_t index)
{
  size_t currentDataSize = getDataSize();
  JASSERT ((index + log_event_common_size) <= currentDataSize)
    (index) (currentDataSize);

#ifdef NO_LOG_ENTRY_TO_BUFFER
  memcpy(&entry.header, &_log[index], log_event_common_size);
#else
  char* buffer = &_log[index];

  memcpy(&GET_COMMON(entry, event), buffer, sizeof(GET_COMMON(entry, event)));
  buffer += sizeof(GET_COMMON(entry, event));
  memcpy(&GET_COMMON(entry, isOptional), buffer, sizeof(GET_COMMON(entry, isOptional)));
  buffer += sizeof(GET_COMMON(entry, isOptional));
  memcpy(&GET_COMMON(entry, log_offset), buffer, sizeof(GET_COMMON(entry, log_offset)));
  buffer += sizeof(GET_COMMON(entry, log_offset));
  memcpy(&GET_COMMON(entry, clone_id), buffer, sizeof(GET_COMMON(entry, clone_id)));
  buffer += sizeof(GET_COMMON(entry, clone_id));
  memcpy(&GET_COMMON(entry, my_errno), buffer, sizeof(GET_COMMON(entry, my_errno)));
  buffer += sizeof(GET_COMMON(entry, my_errno));
  memcpy(&GET_COMMON(entry, retval), buffer, sizeof(GET_COMMON(entry, retval)));
  buffer += sizeof(GET_COMMON(entry, retval));

  JASSERT((buffer - &_log[index]) == log_event_common_size)
    (index) (log_event_common_size) (buffer);
#endif

  if (GET_COMMON(entry, clone_id) == 0) {
    return 0;
  }
  return log_event_common_size;
}

void dmtcp::SynchronizationLog::writeEntryHeaderAtOffset(const log_entry_t& entry,
                                                        size_t index)
{
  
  JASSERT(GET_COMMON(entry, clone_id) > 0);

#ifdef NO_LOG_ENTRY_TO_BUFFER
  memcpy(&_log[index], &entry.header, log_event_common_size);
#else
  char* buffer = &_log[index];

  memcpy(buffer, &GET_COMMON(entry, event), sizeof(GET_COMMON(entry, event)));
  buffer += sizeof(GET_COMMON(entry, event));
  memcpy(buffer, &GET_COMMON(entry, isOptional), sizeof(GET_COMMON(entry, isOptional)));
  buffer += sizeof(GET_COMMON(entry, isOptional));
  memcpy(buffer, &GET_COMMON(entry, log_offset), sizeof(GET_COMMON(entry, log_offset)));
  buffer += sizeof(GET_COMMON(entry, log_offset));
  memcpy(buffer, &GET_COMMON(entry, clone_id), sizeof(GET_COMMON(entry, clone_id)));
  buffer += sizeof(GET_COMMON(entry, clone_id));
  memcpy(buffer, &GET_COMMON(entry, my_errno), sizeof(GET_COMMON(entry, my_errno)));
  buffer += sizeof(GET_COMMON(entry, my_errno));
  memcpy(buffer, &GET_COMMON(entry, retval), sizeof(GET_COMMON(entry, retval)));
  buffer += sizeof(GET_COMMON(entry, retval));

  JASSERT((buffer - &_log[index]) == log_event_common_size)
    (index) (log_event_common_size) (buffer);
#endif
}

size_t dmtcp::SynchronizationLog::getIndex()
{
  return __sync_fetch_and_add(&_index, 0);  
}

size_t dmtcp::SynchronizationLog::atomicIncrementIndex(size_t delta)
{
  return __sync_fetch_and_add(&_index, delta);
}

size_t dmtcp::SynchronizationLog::atomicIncrementEntryIndex()
{
  return __sync_fetch_and_add(&_entryIndex, 1);
}

/*
 * This function should be used instead of dereferencing _dataSize directly.
 * It adds error checking for NULL, and correct atomic functionality.
 */
size_t dmtcp::SynchronizationLog::getDataSize()
{
  if (_dataSize == NULL) {
    return 0;
  } else {
    return __sync_fetch_and_add(_dataSize, 0);
  }
}

/*
 * This function should be used instead of setting the value of
 * _dataSize directly.  It adds error checking for NULL, and correct
 * atomic functionality.
 */
void dmtcp::SynchronizationLog::setDataSize(log_off_t newVal)
{
  JASSERT(_dataSize != NULL);
  __sync_val_compare_and_swap(_dataSize, *_dataSize, newVal);
}

inline log_off_t
dmtcp::SynchronizationLog::atomicIncrementOffset(log_off_t delta)
{
  JASSERT(_dataSize != NULL);
  return __sync_fetch_and_add(_dataSize, delta);
}
