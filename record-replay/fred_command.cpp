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

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include "fred_interface.h"

typedef enum {
  FRED_COMMAND_INVALID,
  FRED_COMMAND_INFO,
  FRED_COMMAND_STATUS,
  FRED_COMMAND_BREAK,
  FRED_COMMAND_CONTINUE
} fred_command_type_t;

typedef struct {
  fred_command_type_t type;
  void *arg;
} fred_command_t;

#define EMPTY_FRED_COMMAND {FRED_COMMAND_INVALID, 0}

static const char *file_name = NULL;

static void print_usage(char *name)
{
  fprintf(stderr, "Usage: %s [OPTIONS] <FILE>\n", name);
  fprintf(stderr, " Send command to program running under FReD control.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, " Options:\n");
  fprintf(stderr,
          "  <FILE>         : Path to fred-shm file for the target process.\n");
  fprintf(stderr, "  -s, --status   : Displays current status information.\n");
  fprintf(stderr, "  -i, --info     : Displays global information.\n");
  fprintf(stderr,
          "  -b X, --break=X: Set a \"breakpoint\" on log entry index X.\n");
  fprintf(stderr, "  -c, --continue : Continue paused replay execution.\n");
}

static void handle_info_command(fred_interface_info_t *info)
{
  printf("Total number of log entries = %Zu\n", info->total_entries);
  printf("Total number of threads = %Zu\n", info->total_threads);
}

static void handle_status_command(fred_interface_info_t *info)
{
  printf("Current clone id = %ld\n", info->current_clone_id);
  printf("Current entry index = %Zu\n", info->current_log_entry_index);
}

static void handle_breakpoint_command(fred_interface_info_t *info,
                                      fred_command_t *cmd)
{
  long entry_num = (long)cmd->arg;
  info->breakpoint_at_index = entry_num;
  printf("Set breakpoint at entry number %ld.\n", entry_num);
  printf("Waiting until breakpoint is hit...\n");
  fflush(stdout);
  /* Wait until breakpoint is hit to exit. */
  while (1) {
    if (info->breakpoint_at_index == FRED_INTERFACE_BP_HIT) {
      break;
    }
    usleep(1);
  }
  printf("Breakpoint hit at entry %ld. Execution is paused.\n", entry_num);
}

static void handle_continue_command(fred_interface_info_t *info)
{
  info->breakpoint_at_index = FRED_INTERFACE_NO_BP;
  printf("Removed breakpoint and continuing execution.\n");
}

static void execute_command(fred_command_t *cmd)
{
  fred_interface_info_t *info = NULL;
  int fd = open(file_name, O_RDWR, 0);
  if (fd == -1) {
    perror("open");
    exit(1);
  }
  info = (fred_interface_info_t *)mmap(NULL, FRED_INTERFACE_SHM_SIZE,
                                       PROT_READ | PROT_WRITE,
                                       MAP_SHARED, fd, 0);
  if ((void *)info == (void *)MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  switch(cmd->type) {
  case FRED_COMMAND_INFO:
    handle_info_command(info);
    break;
  case FRED_COMMAND_STATUS:
    handle_status_command(info);
    break;
  case FRED_COMMAND_BREAK:
    handle_breakpoint_command(info, cmd);
    break;
  case FRED_COMMAND_CONTINUE:
    handle_continue_command(info);
    break;
  default:
    break;
  }

  munmap(info, FRED_INTERFACE_SHM_SIZE);
}

int main(int argc, char **argv)
{
  if (argc < 2) {
    print_usage(argv[0]);
    exit(1);
  }

  fred_command_t cmd = EMPTY_FRED_COMMAND;
  int opt, option_index;
  static struct option long_options[] =
    {
      {"status",    no_argument,       0, 's'},
      {"info",      no_argument,       0, 'i'},
      {"break",     required_argument, 0, 'b'},
      {"continue",  no_argument,       0, 'c'},
      {0, 0, 0, 0} // required (see man getopt)
    };

  while ((opt = getopt_long(argc, argv, "p:sib:c", long_options,
                            &option_index)) != -1) {
    switch (opt) {
    case 's':
      cmd.type = FRED_COMMAND_STATUS;
      break;
    case 'i':
      cmd.type = FRED_COMMAND_INFO;
      break;
    case 'b':
      cmd.type = FRED_COMMAND_BREAK;
      cmd.arg = (void *)strtol(optarg, NULL, 10);
      break;
    case 'c':
      cmd.type = FRED_COMMAND_CONTINUE;
      break;
    default:
      break;
    }
  }
  file_name = argv[argc-1];
  execute_command(&cmd);

  return 0;
}
