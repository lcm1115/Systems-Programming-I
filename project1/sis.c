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
  char* input = allocate(MAX_BUFFER_LENGTH);
  char* temp;
  if (argc < 3 || argc > 4 || (argc == 4 && strcmp(argv[1], "-v"))) {
    fprintf(stderr, "usage: project1 [-v] course_file student_file\n");
    return 1;
  }
  if (argc == 4) {
    verbose = TRUE;
  }
  readfiles(&shead, &chead, &argv[argc - 2], &argv[argc - 1], input);
  while (fgets(input, MAX_BUFFER_LENGTH, stdin)) {
    input[strlen(input) - 1] = 0;
    temp = strtok(input, " \t");
    if (!strcmp(temp, "student")) {
      addstudent(&shead);
    } else if (!strcmp(temp, "course")) {
      addcourse(&chead);
    } else if (!strcmp(temp, "add")) {
      enrollstudent(&shead, &chead);
    } else if (!strcmp(temp, "drop")) {
      int sid = atof(strtok(NULL, " \t"));
      char* temp = strtok(NULL, " \t");
      char tempid[3];
      int cid;

      tempid[0] = temp[0];
      tempid[1] = temp[1];
      tempid[2] = 0;
      temp += 2;
      cid = atof(temp);

      dropstudent(&shead, &chead, sid, tempid, cid);
    } else if (!strcmp(temp, "remove")) {
      removecourse(&shead, &chead);
    }
  }
  struct Student* curstu = shead;
  struct Course* curcourse = chead;
  struct Enrollment* cur;
  while (curstu) {
    printf("\n");
    printf("Student ");
    printstudent(&curstu);
    if (curstu->numcourses <= 0) {
      printf(" is taking no courses");
    } else {
      printf(" is taking %d courses:\n  ", curstu->numcourses);
      cur = curstu->clist;
      while (cur) {
        printf(" %s%03d", cur->course->depid, cur->course->cid);
        cur = cur->nextcourse;
      }
    }
    curstu = curstu->next;
    printf("\n");
  }
  while (curcourse) {
    printf("\n");
    printf("Course %s%03d (limit %d) ",
           curcourse->depid,
           curcourse->cid,
           curcourse->size);
    if (curcourse->numenrolled <= 0) {
      printf("is empty\n");
    } else {
      printf("contains %d students:\n", curcourse->numenrolled);
      cur = curcourse->slist;
      while (cur) {
        printf("   ");
        printstudent(&cur->student);
        printf("\n");
        cur = cur->nextstudent;
      }
    }
    curcourse = curcourse->next;
  }
  clearmem(&shead, &chead);
  unallocate(input);
  return 0;
}

void strcopy(char** dest, char** src) {
  *dest = allocate(strlen(*src) + 1);
  strcpy(*dest, *src);
}

struct Student* initstudent() {
  struct Student* newstudent = allocate(sizeof(struct Student));
  newstudent->numcourses = 0;
  newstudent->first = 0;
  newstudent->middle = 0;
  newstudent->last = 0;
  newstudent->clist = 0;
  newstudent->next = 0;
  return newstudent;
}

struct Course* initcourse() {
  struct Course* newcourse = allocate(sizeof(struct Course));
  newcourse->numenrolled = 0;
  newcourse->depid[0] = 0;
  newcourse->depid[1] = 0;
  newcourse->depid[2] = 0;
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

  temp = strtok(NULL, " \t");
  sid = atof(temp);
  if (getstudent(sid, shead)) {
    fprintf(stderr, "%d already exists\n", sid);
    return;
  }

  new = initstudent();
  new->sid = sid;

  temp = strtok(NULL, " \t");
  strcopy(&(new->last), &temp);
  temp = strtok(NULL, " \t");
  if (temp != 0) {
    strcopy(&(new->first), &temp);
  }
  temp = strtok(NULL, " \t");
  if (temp != 0) {
    strcopy(&(new->middle), &temp);
  }

  if (*shead == NULL) {
    *shead = new;
  } else if (comparestudent(&new, shead) < 0) {
    new->next = *shead;
    *shead = new;
  } else {
    cur = *shead;
    while (cur->next && comparestudent(&cur->next, &new) < 0) {
      cur = cur->next;
    }
    if (cur->next) {
      new->next = cur->next;
    }
    cur->next = new;
  }

  if (verbose) {
    printf("new student ");
    printstudent(&new);
    printf("\n");
  }
}

void addcourse(struct Course** chead) {
  struct Course* new;
  struct Course* cur;
  char* temp;
  char tempid[3];
  int cid;
  temp = strtok(NULL, " \t");
  tempid[0] = temp[0];
  tempid[1] = temp[1];
  tempid[2] = 0;
  temp += 2;
  cid = atof(temp);
  if (getcourse(tempid, cid, chead)) {
    fprintf(stderr, "%s%03d already exists\n", tempid, cid);
    return;
  }

  new = initcourse();
  new->depid[0] = tempid[0];
  new->depid[1] = tempid[1];
  if (strlen(new->depid) >= 3) {
    new->depid[2] = 0;
  }
  new->cid = cid;
  new->size = atof(strtok(NULL, " \t"));

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
  int sid = atof(strtok(NULL, " \t"));
  struct Student* student = getstudent(sid, shead);
  struct Enrollment* cur;
  struct Course* course;
  char* temp = strtok(NULL, " \t");
  char tempid[3];
  int cid;

  tempid[0] = temp[0];
  tempid[1] = temp[1];
  tempid[2] = 0;
  temp += 2;
  cid = atof(temp);

  course = getcourse(tempid, cid, chead);

  if (!student) {
    fprintf(stderr, "%05d does not exist\n", sid);
  } else if (!course) {
    fprintf(stderr, "%s%03d does not exist\n", tempid, cid);
  } else if (student->numcourses == 5) {
    fprintf(stderr, "%05d has a full schedule\n", sid);
  } else if (course->numenrolled == course->size) {
    fprintf(stderr, "%s%03d is full\n", tempid, cid);
  } else if (hasstudent(&course, sid)) {
    fprintf(stderr, "%05d already enrolled in %s%03d\n",
            sid,
            course->depid,
            course->cid);
  } else {
    struct Enrollment* enrollment = initenrollment();
    enrollment->student = student;
    enrollment->course = course;
    if (student->clist == NULL) {
      student->clist = enrollment;
    } else if (strcmp(enrollment->course->depid, student->clist->course->depid) < 0 ||
               (!(strcmp(enrollment->course->depid, student->clist->course->depid)) &&
                 enrollment->course->cid < student->clist->course->cid)) {
      enrollment->nextcourse = student->clist;
      student->clist = enrollment;
    } else {
      cur = student->clist;
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
    if (course->slist == NULL) {
      course->slist = enrollment;
    } else if (comparestudent(&enrollment->student, &course->slist->student) < 0) {
      enrollment->nextstudent = course->slist;
      course->slist = enrollment;
    } else {
      cur = course->slist;
      while (cur->nextstudent &&
        comparestudent(&cur->nextstudent->student, &enrollment->student) < 0) {
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
      printf("%05d added to %s%03d\n",
             enrollment->student->sid,
             enrollment->course->depid,
             enrollment->course->cid);
    }
  }
}

void dropstudent(struct Student** shead,
                 struct Course** chead,
                 int sid,
                 char depid[2],
                 int cid) {
  struct Student* student = getstudent(sid, shead);
  struct Course* course = getcourse(depid, cid, chead);
  struct Enrollment* todelete;

  if (!student) {
    fprintf(stderr, "%05d does not exist\n", sid);
  } else if (!course) {
    fprintf(stderr, "%s%03d does not exist\n", depid, cid);
  } else if (!hasstudent(&course, sid)) {
    fprintf(stderr, "%05d not enrolled in %s%03d\n",
            sid,
            course->depid,
            course->cid);
  } else {
    struct Enrollment* cur = course->slist;
    if (!cur->nextstudent || cur->student->sid == student->sid) {
      course->slist = cur->nextstudent;
    } else {
      while (cur->nextstudent->student->sid != student->sid) {
        cur = cur->nextstudent;
      }
      cur->nextstudent = cur->nextstudent->nextstudent;
    }
    cur = student->clist;
    if (!cur->nextcourse || cur->course == course) {
      todelete = student->clist;
      student->clist = cur->nextcourse;
    } else {
      while (cur->nextcourse->course->cid != course->cid ||
             strcmp(cur->nextcourse->course->depid, course->depid)) {
        cur = cur->nextcourse;
      }
      todelete = cur->nextcourse;
      cur->nextcourse = cur->nextcourse->nextcourse;
    }
    student->numcourses--;
    course->numenrolled--;
    if (verbose) {
      printf("%05d dropped from %s%03d\n",
             student->sid,
             course->depid,
             course->cid);
    }
    unallocate(todelete);
  }
}

void removecourse(struct Student** shead, struct Course** chead) {
  struct Course* course;
  struct Enrollment* enrollment;
  char* temp = strtok(NULL, " \t");
  char tempid[3];
  int cid;

  tempid[0] = temp[0];
  tempid[1] = temp[1];
  tempid[2] = 0;
  temp += 2;
  cid = atof(temp);

  course = getcourse(tempid, cid, chead);

  if (!course) {
    printf("%s%03d does not exist\n", tempid, cid);
    return;
  }

  enrollment = course->slist;
  int v = verbose;
  verbose = 0;
  while (enrollment != NULL && enrollment->student != NULL) {
    dropstudent(shead,
                chead,
                enrollment->student->sid,
                course->depid,
                course->cid);
    enrollment = enrollment->nextstudent;
  }
  if (!strcmp(course->depid, (*chead)->depid) &&
      course->cid == (*chead)->cid) {
    (*chead) = (*chead)->next;
  } else {
    struct Course* cur = *chead;
    while (strcmp(course->depid, cur->next->depid) ||
           course->cid != cur->next->cid) {
      cur = cur->next;
    }
    cur->next = cur->next->next;
  }
  verbose = v;
  if (verbose) {
    printf("%s%03d closed\n", course->depid, course->cid);
  }
  unallocate(course);
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
  printf("%05d (%s", (*student)->sid, (*student)->last);
  if ((*student)->first != 0) printf(" %s", (*student)->first);
  if ((*student)->middle != 0) printf(" %s", (*student)->middle);
  printf(")");
}

void printcourse(struct Course** course) {
  printf("%s%03d %d", (*course)->depid, (*course)->cid, (*course)->size);
}

void readfiles(struct Student** shead,
               struct Course** chead,
               char** coursefile,
               char** studentfile,
               char inputbuffer[]) {
  FILE* fp = fopen(*coursefile, "r");

  while (fgets(inputbuffer, MAX_BUFFER_LENGTH, fp) != NULL) {
    inputbuffer[strlen(inputbuffer) - 1] = 0;
    char* test = strtok(inputbuffer, " \t");
    addcourse(chead);
  }

  fclose(fp);
  fopen(*studentfile, "r");

  while (fgets(inputbuffer, MAX_BUFFER_LENGTH, fp) != NULL) {
    inputbuffer[strlen(inputbuffer) - 1] = 0;
    strtok(inputbuffer, " \t");
    addstudent(shead);
  }

  fclose(fp);
}

int comparestudent(struct Student** stu1, struct Student** stu2) {
  if (strcmp((*stu1)->last, (*stu2)->last) < 0) {
    return -1;
  } else if (strcmp((*stu1)->last, (*stu2)->last) > 0) {
    return 1;
  } else {
    if (strcmp((*stu1)->first, (*stu2)->first) < 0) {
      return -1;
    } else if (strcmp((*stu1)->first, (*stu2)->first) > 0) {
      return 1;
    } else {
      if (strcmp((*stu1)->middle, (*stu2)->middle) < 0) {
        return -1;
      } else if (strcmp((*stu1)->middle, (*stu2)->middle) > 0) {
        return 1;
      } else {
        return (*stu1)->sid - (*stu2)->sid;
      }
    }
  }
}

void clearmem(struct Student** shead, struct Course** chead) {
  struct Student* curs = *shead;
  struct Student* prevs;
  struct Enrollment* cure;
  struct Enrollment* preve;
  while (curs) {
    prevs = curs;
    curs = curs->next;
    cure = prevs->clist;
    while (cure) {
      preve = cure;
      cure = cure->nextcourse;
      unallocate(preve);
    }
    unallocate(prevs->first);
    unallocate(prevs->middle);
    unallocate(prevs->last);
    unallocate(prevs);
  }

  struct Course* curc = *chead;
  struct Course* prevc;
  while (curc) {
    prevc = curc;
    curc = curc->next;
    unallocate(prevc);
  }
}
