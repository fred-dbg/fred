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

#ifndef CONSTANTS_H
#define CONSTANTS_H

//#ifdef HAVE_CONFIG_H
//# include "config.h"
//#endif
#include "linux/version.h"
#include "config.h"

#define LIBC_FILENAME "libc.so.6"
#define LIBPTHREAD_FILENAME "libpthread.so.0"

#define ENABLE_MALLOC_WRAPPER
#define ENV_VAR_LOG_REPLAY "DMTCP_LOG_REPLAY"

#define MAX_LOG_LENGTH ((size_t)250 * 1024 * 1024)
#define INVALID_LOG_OFFSET (~0U)
#endif
