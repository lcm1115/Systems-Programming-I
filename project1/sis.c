#include "allocate.h"
#include "sis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int verbose;
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
  if (argc == 4) {
    verbose = TRUE;
  }
  printf("sis> ");
  fflush(stdout);
  while ((nread = read(STDIN_FILENO, input, MAX_BUFFER_LENGTH))) {
    input[nread - 1] = '\0';
    temp = strtok(input, " ");
    if (!strcmp(temp, "student")) {
      addstu(&shead);
    } else if (!strcmp(temp, "course")) {
      addcourse(&chead);
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
  temp = strtok(NULL, " ");
  if (temp != 0) {
    strcopy(&(new->first), &temp);
  }
  temp = strtok(NULL, " ");
  if (temp != 0) {
    strcopy(&(new->middle), &temp);
  }

  if (verbose) {
    printf("new ");
    printstu(&new);
  }
}

void addcourse(struct Course** chead) {
  struct Course* new;
  struct Course* cur;
  char* temp;
  char tempid[2];
  int cid;
  temp = strtok(NULL, " ");
  tempid[0] = temp[0];
  tempid[1] = temp[1];
  temp += 2;
  cid = atof(temp);
  if (checkcid(tempid, cid, chead)) {
    perror("cid already exists");
    return;
  }

  new = allocate(sizeof(struct Course));
  new->depid[0] = tempid[0];
  new->depid[1] = tempid[1];
  new->cid = cid;
  new->size = atof(strtok(NULL, " "));
  
  if (*chead == NULL) {
    *chead = new;
  } else if (strcmp(new->depid, (*chead)->depid) <= 0 &&
             new->cid < (*chead)->cid) {
    new->next = *chead;
    *chead = new;
  } else {
    cur = *chead;
    while (cur->next && strcmp(cur->next->depid, new->depid) <= 0) {
      cur = cur->next;
    }
    if (!cur->next) cur->next = new;
    else {
      while (cur->next && !strcmp(new->depid, cur->next->depid) &&
             cur->next->cid < new->cid) {
        cur = cur->next;
      }
      if (cur->next) {
        new->next = cur->next;
      }
      cur->next = new;
    }
  }

  //printcourse(&new);
  printf("Courses\n-------\n");
  courselist(chead);
}

int checksid(int sid, struct Student** shead) {
  struct Student* cur = *shead;
  while (cur) {
    if (cur->sid == sid) return 1;
    cur = cur->next;
  }
  return 0;
}

int checkcid(char depid[2], int cid, struct Course** chead) {
  struct Course* cur = *chead;
  while (cur) {
    if (!strcmp(depid, cur->depid) && cid == cur->cid) return 1;
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

void printcourse(struct Course** course) {
  printf("%s%03d %d\n", (*course)->depid, (*course)->cid, (*course)->size);
}

void stulist(struct Student** shead) {
  struct Student* cur = *shead;
  while (cur) {
    printstu(&cur);
    cur = cur->next;
  }
}

void courselist(struct Course** chead) {
  struct Course* cur = *chead;
  while (cur) {
    printcourse(&cur);
    cur = cur->next;
  }
}
