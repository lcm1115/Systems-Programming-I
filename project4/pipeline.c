#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "support.h"
#include <unistd.h>

// Global Variables

// Current command being processed.
char** command;

// Current index in argv when searching for commands.
int curIndex;

// Previous value of curIndex when searching for commands.
int prevIndex;

// Number of arguments in the current command.
int numArgs = 0;

// Number of processes remaining to execute.
int numProcesses = 1;

// File descriptors for I/O redirection with pipes.
int fd[2];

// File descriptors for file I/O.
int read_fd = -1;
int write_fd = -1;

int main(int argc, char** argv) {
  if (argc == 1) {
    return 0;
  }

  // Check input for validity.
  if (!validateInput(argc - 1, &argv[1])) {
    return -1;
  }

  // Count number of processes on the pipeline.
  for (int i = 0; argv[i] != 0; ++i) {
    if (!strcmp(argv[i], "|")) {
      ++numProcesses;
    }
  }

  // Open files as needed for I/O redirection.
  int bound = argc - 1;
  for (int i = 0; i < bound; ++i) {
    // Find output redirect if it exists and open the pipe.
    if (!strcmp(argv[i], ">")) {
      write_fd = open(argv[i+1], O_CREAT | O_WRONLY);
      if (write_fd < 0) {
        perror(argv[i+1]);
        exit(-1);
      }
      dup2(write_fd, 1);
      close(write_fd);
      if (argc == bound + 1) argc = i;
    }
    // Find input redirect if it exists and get the file descriptor for it.
    if (!strcmp(argv[i], "<")) {
      read_fd = open(argv[i+1], O_RDONLY);
      if (read_fd < 0) {
        perror(argv[i+1]);
        exit(-1);
      }
      if (argc == bound + 1) argc = i;
    }
  }

  // Get first command to execute.
  curIndex = argc - 1;
  prevIndex = curIndex;
  curIndex = nextSpecial(curIndex, argv);
  if (curIndex == -1) curIndex = 0;
  getNextCommand(curIndex + 1, prevIndex, argv);

  // If there are multiple processes, enter function to execute all processes.
  if (numProcesses > 1) {
    executeNext(argc, argv);
  }
  // Otherwise, redirect stdin if needed and execute the command.
  else {
    if (read_fd != -1) {
      dup2(read_fd, 0);
      close(read_fd);
    }
    execvp(command[0], command);
  }

  freeCommand();
}