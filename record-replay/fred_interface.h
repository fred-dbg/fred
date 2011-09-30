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

#ifndef _FRED_INTERFACE_H
#define _FRED_INTERFACE_H

#include "synchronizationlogging.h"

typedef struct {
  clone_id_t current_clone_id;
  size_t current_log_entry_index;
  size_t total_entries;
  unsigned breakpoint_at_index;
} fred_interface_info_t;

#define FRED_INTERFACE_SHM_SIZE sizeof(fred_interface_info_t)
#define FRED_INTERFACE_SHM_FILE_FMT "%s/fred-shm.%d"

/* Indicates no breakpoint is currently set. */
#define FRED_INTERFACE_NO_BP -1
/* Indicates that the set breakpoint has been hit. */
#define FRED_INTERFACE_BP_HIT -2

#endif
