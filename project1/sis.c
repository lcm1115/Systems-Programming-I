#include "allocate.h"
#include "sis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv) {
  struct Student* shead = 0;
  struct Course* chead = 0;
  char input[MAX_BUFFER_LENGTH];
  char* temp;
  int nread;
  if (argc < 3 || argc > 4 || (argc == 4 && strcmp(argv[1], "-v"))) {
    perror("usage: project1 [-v] course_file student_file");
    return 1;
  }
  printf("sis> ");
  fflush(stdout);
  while ((nread = read(STDIN_FILENO, input, MAX_BUFFER_LENGTH))) {
    input[nread - 1] = '\0';
    temp = strtok(input, " ");
    if (!strcmp(temp, "student")) {
      addstu(&shead);
    }
    printf("sis> ");
    fflush(stdout);
  }
  printf("\nStudent List:\n");
  fflush(stdout);
  stulist(&shead);
  return 0;
}

void strcopy(char** dest, char** src) {
  *dest = allocate(strlen(*src) + 1);
  strcpy(*dest, *src);
}

struct Student* initstudent() {
  struct Student* newstudent = allocate(sizeof(struct Student));
  newstudent->first = 0;
  newstudent->middle = 0;
  newstudent->last = 0;
  newstudent->clist = 0;
  newstudent->next = 0;
  return newstudent;
}

struct Course* initcourse() {
  struct Course* newcourse = allocate(sizeof(struct Course));
  newcourse->slist = 0;
  newcourse->next = 0;
  return newcourse;
}

void addstu(struct Student** shead) {
  struct Student* new;
  struct Student* cur;
  int sid;
  char* temp;

  temp = strtok(NULL, " ");
  sid = atof(temp);
  if (checksid(sid, shead)) {
    perror("sid already exists");
    return;
  }

  new = allocate(sizeof(struct Student));
  new->sid = sid;

  if (*shead == NULL) {
    *shead = new;
  } else if (new->sid < (*shead)->sid) {
    printf("New head\n");
    new->next = *shead;
    *shead = new;
  } else {
    cur = *shead;
    while (cur->next && cur->next->sid < new->sid) {
      cur = cur->next;
    }
    if (cur->next) {
      new->next = cur->next;
    }
    cur->next = new;
  }

  temp = strtok(NULL, " ");
  strcopy(&(new->last), &temp);
  printf("Copied last: %s\n", new->last);
  temp = strtok(NULL, " ");
  if (temp != 0) {
    strcopy(&(new->first), &temp);
    printf("Copied first: %s\n", new->first);
  }
  temp = strtok(NULL, " ");
  if (temp != 0) {
    strcopy(&(new->middle), &temp);
    printf("Copied middle: %s\n", new->middle);
  }

  printstu(&new);
}

int checksid(int sid, struct Student** shead) {
  struct Student* cur = *shead;
  while (cur) {
    if (cur->sid == sid) return 1;
    cur = cur->next;
  }
  return 0;
}

void printstu(struct Student** student) {
  printf("%05d %s", (*student)->sid, (*student)->last);
  if ((*student)->first != 0) printf(" %s", (*student)->first);
  if ((*student)->middle != 0) printf(" %s", (*student)->middle);
  printf("\n");
}

void stulist(struct Student** shead) {
  struct Student* cur = *shead;
  while (cur) {
    printstu(&cur);
    cur = cur->next;
  }
}
