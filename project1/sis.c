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
  readfiles(&shead, &chead, &argv[argc - 2], &argv[argc - 1], input);
  printf("sis> ");
  fflush(stdout);
  while ((nread = read(STDIN_FILENO, input, MAX_BUFFER_LENGTH))) {
    input[nread - 1] = '\0';
    printf("input: %s\n", input);
    temp = strtok(input, " ");
    if (!strcmp(temp, "student")) {
      addstudent(&shead);
    } else if (!strcmp(temp, "course")) {
      addcourse(&chead);
    } else if (!strcmp(temp, "add")) {
      enrollstudent(&shead, &chead);
    } else if (!strcmp(temp, "drop")) {
    } else if (!strcmp(temp, "remove")) {
    }
    printf("sis> ");
    fflush(stdout);
  }
  printf("\nStudent List:\n");
  fflush(stdout);
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

struct Enrollment* initenrollment() {
  struct Enrollment* newenrollment = allocate(sizeof(struct Enrollment));
  newenrollment->student = 0;
  newenrollment->course = 0;
  newenrollment->nextstudent = 0;
  newenrollment->nextcourse = 0;
  return newenrollment;
}

void addstudent(struct Student** shead) {
  struct Student* new;
  struct Student* cur;
  int sid;
  char* temp;

  temp = strtok(NULL, " ");
  sid = atof(temp);
  if (getstudent(sid, shead)) {
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
    printstudent(&new);
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
  if (getcourse(tempid, cid, chead)) {
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
  } else if (strcmp(new->depid, (*chead)->depid) < 0 ||
             (!(strcmp(new->depid, (*chead)->depid)) &&
               new->cid < (*chead)->cid)) {
    new->next = *chead;
    *chead = new;
  } else {
    cur = *chead;
    while (cur->next && strcmp(cur->next->depid, new->depid) < 0) {
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

  if (verbose) printf("%s%03d opened with limit %d\n",
                       new->depid,
                       new->cid,
                       new->size);
}

void enrollstudent(struct Student** shead, struct Course** chead) {
  int sid = atof(strtok(NULL, " "));
  struct Student* student = getstudent(sid, shead);
  struct Course* course;
  struct Enrollment* enrollment;
  char* temp = strtok(NULL, " ");
  char tempid[2];
  int cid;

  tempid[0] = temp[0];
  tempid[1] = temp[1];
  temp += 2;
  cid = atof(temp);

  course = getcourse(tempid, cid, chead);

  if (!student) {
    char error[20];
    sprintf(error, "%05d does not exist", sid);
    perror(error);
  } else if (!course) {
    char error[20];
    sprintf(error, "%s%03d does not exist", tempid, cid);
    perror(error);
  } else if (student->numcourses == 5) {
    char error[25];
    sprintf(error, "%05d has a full schedule", sid);
    perror(error);
  } else if (course->numenrolled == course->size) {
    char error[13];
    sprintf(error, "%s%03d is full", tempid, cid);
    perror(error);
  } else if (hasstudent(&course, sid)) {
    char error[31];
    sprintf(error, "%05d already enrolled in %s%03d", sid, tempid, cid);
    perror(error);
  } else {
    struct Enrollment* enrollment = initenrollment();
    enrollment->student = student;
    enrollment->course = course;
    // Add course to student schedule
    if (student->clist == NULL) {
      student->clist = enrollment;
    } else if (strcmp(enrollment->course->depid, student->clist->course->depid) < 0 ||
               (!(strcmp(enrollment->course->depid, student->clist->course->depid)) &&
                 enrollment->course->cid < student->clist->course->cid)) {
      enrollment->nextcourse = student->clist;
      student->clist = enrollment;
    } else {
      struct Enrollment* cur = student->clist;
      while (cur->nextcourse &&
        strcmp(cur->nextcourse->course->depid, enrollment->course->depid) < 0) {
        cur = cur->nextcourse;
      }
      if (!cur->nextcourse) cur->nextcourse = enrollment;
      else {
        while (cur->nextcourse &&
          !strcmp(enrollment->course->depid, cur->nextcourse->course->depid) &&
          cur->nextcourse->course->cid < enrollment->course->cid) {
          cur = cur->nextcourse;
        }
        if (cur->nextcourse) {
          enrollment->nextcourse = cur->nextcourse;
        }
        cur->nextcourse = enrollment;
      }
    }
    // Add student to course list
    if (course->slist == NULL) {
      course->slist = enrollment;
    } else if (enrollment->student->sid < course->slist->student->sid) {
      enrollment->nextstudent = course->slist;
      course->slist = enrollment;
    } else {
      struct Enrollment* cur = course->slist;
      while (cur->nextstudent &&
        cur->nextstudent->student->sid < enrollment->student->sid) {
        cur = cur->nextstudent;
      }
      if (cur->nextstudent) {
        enrollment->nextstudent = cur->nextstudent;
      }
      cur->nextstudent = enrollment;
    }
    student->numcourses++;
    course->numenrolled++;
    if (verbose) {
      printf("%05d added to %s%d\n",
             enrollment->student->sid,
             enrollment->course->depid,
             enrollment->course->cid);
    }
  }
}

struct Student* getstudent(int sid, struct Student** shead) {
  struct Student* cur = *shead;
  while (cur && cur->sid != sid) {
    cur = cur->next;
  }
  return cur;
}

struct Course* getcourse(char depid[2], int cid, struct Course** chead) {
  struct Course* cur = *chead;
  while (cur && (strcmp(depid, cur->depid) || cid != cur->cid)) {
    cur = cur->next;
  }
  return cur;
}

int hasstudent(struct Course** course, int sid) {
  struct Enrollment* cur = (*course)->slist;
  while (cur && cur->student->sid != sid) {
    cur = cur->nextstudent;
  }
  if (cur) {
    return TRUE;
  } else {
    return FALSE;
  }
}

void printstudent(struct Student** student) {
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
    printstudent(&cur);
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

void readfiles(struct Student** shead,
               struct Course** chead,
               char** coursefile,
               char** studentfile,
               char inputbuffer[]) {
  FILE* fp = fopen(*coursefile, "r");

  while (fgets(inputbuffer, MAX_BUFFER_LENGTH, fp) != NULL) {
    strtok(inputbuffer, " ");
    addcourse(chead);
  }

  fclose(fp);
  fopen(*studentfile, "r");

  while (fgets(inputbuffer, MAX_BUFFER_LENGTH, fp) != NULL) {
    strtok(inputbuffer, " ");
    addstudent(shead);
  }

  fclose(fp);
}
