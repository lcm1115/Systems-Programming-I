/*
** File:        support.c
** Author:      Liam Morris (lcm1115)
** Description: This file implements the functions described in support.h.
**              This provides functionality to pipeline.c for parsing
**              command line arguments and executing commands.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "support.h"
#include <unistd.h>

// Implementation of strdup.
// Arguments:
//   str - string to be copied
// Returns:
//   A new cstring containing whose content is the same as str.
char* strdup(char* str) {
  char* new = (char*) malloc(strlen(str) + 1);
  if (!new) {
    return 0;
  }
  new[strlen(str)] = 0;
  strcpy(new, str);

  return new;
}

// Frees all cstrings in a given command buffer.
void freeCommand() {
  for (int i = 0; i < numArgs; ++i) {
    free(command[i]);
  }
  free(command);
}

// Validates input and ensures that command line input is legal.
int validateInput(int argc, char** argv) {
  int output = 0;
  int input = 0;
  for (int i = 0; i < argc; ++i) {
    switch (argv[i][0]) {
      // Check for illegal null command for pipe.
      case '|':
        if (i == 0 || i == argc - 1 ||
            argv[i-1][0] == '|' || argv[i+1][0] == '|') {
          fprintf(stderr, "Illegal null command\n");
          return 0;
        }
        break;
      // Check cases for output redirect.
      case '>':
        // Make sure some sort of output is happening.
        if (i == 0 || argv[i-1][0] == '|') {
          fprintf(stderr, "Missing command name\n");
          return 0;
        }
        // Make sure that there is a filename to write to!
        else if (i == argc - 1 || !strcmp(argv[i+1], "|") ||
                   !strcmp(argv[i+1], "<") || !strcmp(argv[i+1], ">")) {
          fprintf(stderr, "Missing redirect filename\n");
          return 0;
        }
       // Make sure that only one filename is listed.
        else if (i <= argc - 3 && strcmp(argv[i+2], "<") &&
                   strcmp(argv[i+2], ">") && strcmp(argv[i+2], "|")) {
          fprintf(stderr, "%s: Illegal argument after redirect\n", argv[i+2]);
          return 0;
        }
        // Make sure that the output is one of the last two commands.
        else if ((i <= argc - 3 && !strcmp(argv[i+2], "|")) || output) {
          fprintf(stderr, "%s: Illegal output redirect\n", argv[i+1]);
          return 0;
        }
        output = 1;
        break;
      // Check cases for input redirect.
      case '<':
        // Make sure some sort of input is happening.
        if (i == 0 || argv[i-1][0] == '|') {
          fprintf(stderr, "Missing command name\n");
          return 0;
        }
        // Make sure that there is a filename to read from!
        else if (i == argc - 1 || !strcmp(argv[i+1], "|") ||
                   !strcmp(argv[i+1], "<") || !strcmp(argv[i+1], ">")) {
          fprintf(stderr, "Missing redirect filename\n");
          return 0;
        }
        // Make sure that only one filename is listed.
        else if (i <= argc - 3 && strcmp(argv[i+2], ">") &&
                   strcmp(argv[i+2], "<") && strcmp(argv[i+2], "|")) {
          fprintf(stderr, "%s: Illegal argument after redirect\n", argv[i+2]);
          return 0;
        }
        // Make sure that the input is one of the last two commands.
        else if ((i <= argc - 3 && !strcmp(argv[i+2], "|")) || input) {
          fprintf(stderr, "%s: Illegal input redirect\n", argv[i+1]);
          return 0;
        }
        input = 1;
        break;
    }
  }
}

// Gets the index of the next special character in argv starting from a given
// start index and working backwards.
// Arguments:
//   startIndex - index to start from when searching for next special character.
//   argv - command line arguments
// Returns:
//   An integer representing the index of the next special character, or -1 if
//   no special character is found.
int nextSpecial(int startIndex, char** argv) {
  for (int i = startIndex; i > 0; --i) {
    if (strlen(argv[i]) == 1 && (argv[i][0] == '|' ||
        argv[i][0] == '>' || argv[i][0] == '<')) {
        return i;
    }
  }

  return -1;
}

// Get's the next command and command arguments between two indices of argv.
// Arguments:
//   startIndex - index to start searching in argument array
//   endIndex - index to end searching in argument array
//   argv - command line arguments
void getNextCommand(int startIndex, int endIndex, char** argv) {
  // Free old command buffer.
  freeCommand();

  // Allocate space for each argument.
  numArgs = endIndex - startIndex + 1;
  command = (char**) malloc(sizeof(char*) * numArgs + 1);
  command[numArgs] = 0;
  
  // Copy each argument into the command buffer.
  for (int i = 0; i < numArgs; ++i) {
    command[i] = strdup(argv[i + startIndex]);
    if (!command[i]) {
      fprintf(stderr, "Memory error\n");
    }
  }
}

// Execute next command on the pipeline.
// Arguments:
//   argc - number of command line arguments given
//   argv - command line arguments
void executeNext(int argc, char** argv) {
  --numProcesses;
  // Create pipe and make sure it is valid.
  if (pipe(fd) < 0) {
    perror("pipe");
    exit(-1);
  }

  // Fork a child process.
  pid_t pid = fork();
  switch (pid) {
    // Exit on error.
    case -1:
      perror("fork");
      exit(-1);

    // Handle child process.
    case 0:
      // Retrieve command to execute.
      --curIndex;
      prevIndex = curIndex;
      curIndex = nextSpecial(curIndex, argv);
      if (curIndex == -1) curIndex = 0;
      getNextCommand(curIndex + 1, prevIndex, argv);

      // If this is the first command on the pipeline and input is received from
      // a file, dup the file descriptor onto stdin.
      if (read_fd != -1 && numProcesses <= 1) {
        dup2(read_fd, 0);
        close(read_fd);
      }

      // Redirect pipe to stdout.
      close(fd[0]);
      dup2(fd[1], 1);
      close(fd[1]);

      // If there is another process to execute, call the function again.
      if (numProcesses > 1) {
        executeNext(argc, argv);
      }
      execvp(command[0], command);
      perror(command[0]);
      break;

    // Handle parent process.
    default:
      // Redirect read side of pipe to stdin.
      close(fd[1]);
      dup2(fd[0], 0);
      close(fd[0]);
      execvp(command[0], command);
      perror(command[0]);
      break;
  }
}
