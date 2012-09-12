#include "allocate.h"
#include <stdio.h>

int main(int argc, char** argv) {
  if (argc < 3 || argc > 4) {
    perror("usage: project1 [-v] course_file student_file");
    return 1;
  }
  return 0;
}
