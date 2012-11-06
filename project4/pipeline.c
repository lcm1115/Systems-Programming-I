#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char** command;
int curIndex;
int prevIndex;
int numCommands = 0;
int numProcesses = 1;
int fd[2];

char* strdup(char* str) {
  char* new = (char*) malloc(strlen(str) + 1);
  new[strlen(str)] = 0;
  strcpy(new, str);

  return new;
}

void freeCommand() {
  for (int i = 0; i < numCommands; ++i) {
    free(command[i]);
  }
  free(command);
}

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
        } else if (i == argc - 1 || argv[i+1][0] == '|') {
          fprintf(stderr, "Missing redirect filename\n");
          return 0;
        } else if (i <= argc - 3 && argv[i+2][0] != '<' &&
                   argv[i+2][0] != '>' && argv[i+2][0] != '|') {
          fprintf(stderr, "%s: Illegal argument after redirect\n", argv[i+2]);
          return 0;
        } else if (output) {
          fprintf(stderr, "%s: Illegal output redirect\n", argv[i+1]);
          return 0;
        }
        output = 1;
        break;
      case '<':
        if (i == 0 || argv[i-1][0] == '|') {
          fprintf(stderr, "Missing command name\n");
          return 0;
        } else if (i == argc - 1 || argv[i+1][0] == '|') {
          fprintf(stderr, "Missing redirect filename\n");
          return 0;
        } else if (i <= argc - 3 && argv[i+2][0] != '>' &&
                   argv[i+2][0] != '<' && argv[i+2][0] != '|') {
          fprintf(stderr, "%s: Illegal argument after redirect\n", argv[i+2]);
          return 0;
        } else if (input) {
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
  freeCommand();
  numCommands = endIndex - startIndex + 1;
  command = (char**) malloc(sizeof(char*) * numCommands + 1);
  command[numCommands] = 0;

  for (int i = 0; i < numCommands; ++i) {
    command[i] = strdup(argv[i + startIndex]);
  }
}

void executeNext(int argc, char** argv) {
  --numProcesses;
  pipe(fd);
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

int main(int argc, char** argv) {
  if (!validateInput(argc - 1, &argv[1])) {
    return -1;
  }

  for (int i = 0; argv[i] != 0; ++i) {
    if (!strcmp(argv[i], "|")) {
      ++numProcesses;
    }
  }

  curIndex = argc - 1;
  prevIndex = curIndex;
  curIndex = nextSpecial(curIndex, argv);
  if (curIndex == -1) curIndex = 0;
  getNextCommand(curIndex + 1, prevIndex, argv);

  if (numProcesses > 1) {
    executeNext(argc, argv);
  } else {
    execvp(command[0], command);
  }

  freeCommand();
}
