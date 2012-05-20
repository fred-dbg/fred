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
// Needed for 'off64_t' and 'struct stat64'
//#define _GNU_SOURCE

#include <dirent.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <map>
#include "constants.h"
#include "jalib.h"
#include "dmtcpalloc.h"

//namespace dmtcp = std;
//using namespace dmtcp;
#include "synchronizationlogging.h"
#include "log.h"
#include "autogen/fred_read_log.h"

void print_log_entry_common(int idx, log_entry_t *entry) {
  printf("%2d: clone_id=%ld, [%-20.20s]: ",
         idx, GET_COMMON_PTR(entry, clone_id), log_event_str[entry->header.event]);

  switch ((long) (unsigned long) GET_COMMON_PTR(entry, retval)) {
    case 0:
      printf("ret=  0      , "); break;
    case -1:
      printf("ret= -1      , "); break;
    default:
      printf("ret=%p, ", GET_COMMON_PTR(entry, retval)); break;
  }
  printf("offset=%2ld, errno=%d, isOpt=%d",
         GET_COMMON_PTR(entry, log_offset), GET_COMMON_PTR(entry, my_errno));
}

void rewriteLog(char *log_path)
{
  dmtcp::SynchronizationLog log;
  /* Only need enough room for the metadata. */
  log.initialize(log_path, LOG_OFFSET_FROM_START);
  size_t logSize = log.getDataSize();
  log.destroy(SYNC_IS_RECORD);
  sync_logging_branch = SYNC_REPLAY;
  log.initialize(log_path, logSize + LOG_OFFSET_FROM_START + 1);
  printf("Metadata: dataSize=%zu, numEntries=%zu\n",
         log.getDataSize(), log.numEntries());
  log_entry_t entry = EMPTY_LOG_ENTRY;
  for (size_t i = 0; i < log.numEntries(); i++) {
    if (log.isEndOfLog()) {
      printf("Error reading log file. numEntries: %zu, entriesRead: %zu\n",
             log.numEntries(), i);
      exit(1);
    }
    entry = log.getCurrentEntry();
    printEntry(i, &entry);
    log.advanceToNextEntry();
  }
}


void initializeJalib()
{
  jalib::JalibFuncPtrs jalibFuncPtrs;

#define INIT_JALIB_FPTR(name) jalibFuncPtrs.name = name;

  jalibFuncPtrs.dmtcp_get_tmpdir = dmtcp_get_tmpdir;
  jalibFuncPtrs.dmtcp_get_uniquepid_str = dmtcp_get_uniquepid_str;
  jalibFuncPtrs.writeAll = dmtcp::Util::writeAll;
  jalibFuncPtrs.readAll = dmtcp::Util::readAll;

  INIT_JALIB_FPTR(open);
  INIT_JALIB_FPTR(fopen);
  INIT_JALIB_FPTR(close);
  INIT_JALIB_FPTR(fclose);

  INIT_JALIB_FPTR(syscall);
  INIT_JALIB_FPTR(mmap);
  INIT_JALIB_FPTR(munmap);

  INIT_JALIB_FPTR(read);
  INIT_JALIB_FPTR(write);
  INIT_JALIB_FPTR(select);

  INIT_JALIB_FPTR(socket);
  INIT_JALIB_FPTR(connect);
  INIT_JALIB_FPTR(bind);
  INIT_JALIB_FPTR(listen);
  INIT_JALIB_FPTR(accept);

  INIT_JALIB_FPTR(pthread_mutex_lock);
  INIT_JALIB_FPTR(pthread_mutex_trylock);
  INIT_JALIB_FPTR(pthread_mutex_unlock);

  jalib_init(jalibFuncPtrs, STDERR_FILENO, -1, 99);
  JASSERT_INIT("");
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "USAGE: %s /path/to/sync-log\n", argv[0]);
    return 1;
  }
  initializeJalib();
  rewriteLog(argv[1]);
  return 0;
}
