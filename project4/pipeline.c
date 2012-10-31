#include <stdio.h>

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

int main(int argc, char** argv) {
  if (argc == 1) {
    return 0;
  }

  if (!validateInput(argc - 1, &argv[1])) {
    return -1;
  }
}
