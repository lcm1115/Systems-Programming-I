#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
      case '|':
        if (i == 0 || i == argc - 1 ||
            argv[i-1][0] == '|' || argv[i+1][0] == '|') {
          fprintf(stderr, "Illegal null command\n");
          return 0;
        }
        break;
      case '>':
        if (i == 0 || argv[i-1][0] == '|') {
          fprintf(stderr, "Missing command name\n");
          return 0;
        } else if (i == argc - 1 || !strcmp(argv[i+1], "|") ||
                   !strcmp(argv[i+1], "<") || !strcmp(argv[i+1], ">")) {
          fprintf(stderr, "Missing redirect filename\n");
          return 0;
        } else if (i <= argc - 3 && strcmp(argv[i+2], "<") &&
                   strcmp(argv[i+2], ">") && strcmp(argv[i+2], "|")) {
          fprintf(stderr, "%s: Illegal argument after redirect\n", argv[i+2]);
          return 0;
        } else if ((i <= argc - 3 && !strcmp(argv[i+2], "|")) || output) {
          fprintf(stderr, "%s: Illegal output redirect\n", argv[i+1]);
          return 0;
        }
        output = 1;
        break;
      case '<':
        if (i == 0 || argv[i-1][0] == '|') {
          fprintf(stderr, "Missing command name\n");
          return 0;
        } else if (i == argc - 1 || !strcmp(argv[i+1], "|") ||
                   !strcmp(argv[i+1], "<") || !strcmp(argv[i+1], ">")) {
          fprintf(stderr, "Missing redirect filename\n");
          return 0;
        } else if (i <= argc - 3 && strcmp(argv[i+2], ">") &&
                   strcmp(argv[i+2], "<") && strcmp(argv[i+2], "|")) {
          fprintf(stderr, "%s: Illegal argument after redirect\n", argv[i+2]);
          return 0;
        } else if ((i <= argc - 3 && !strcmp(argv[i+2], "|")) || input) {
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

// Execute next command.
void executeNext(int argc, char** argv) {
  --numProcesses;
  if (pipe(fd) < 0) {
    perror("pipe");
  }
  int pid = fork();
  switch (pid) {
    case -1:
      perror("fork");
      exit(-1);

    case 0:
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
      close(fd[0]);
      dup2(fd[1], 1);
      close(fd[1]);
      if (numProcesses > 1) {
        executeNext(argc, argv);
      }
      execvp(command[0], command);
      perror(command[0]);
      break;

    default:
      close(fd[1]);
      dup2(fd[0], 0);
      close(fd[0]);
      execvp(command[0], command);
      perror(command[0]);
      break;
  }
}