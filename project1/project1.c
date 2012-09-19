#include "allocate.h"
#include "project1.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

uint8_t verbose;
struct Student* shead = 0;
struct Course* chead = 0;
char* input;
int main(int argc, char** argv) {
  input = allocate(MAX_BUFFER_LENGTH);
  if (!input) {
    printf("Memory allocation failed!\n");
    clearmem();
    exit(-1);
  }
  char* temp;
  if (argc < 3 || argc > 4 || (argc == 4 && strcmp(argv[1], "-v"))) {
    fprintf(stderr, "usage: project1 [-v] course_file student_file\n");
    return 1;
  }
  if (argc == 4) {
    verbose = 1;
  }
  temp = argv[argc - 2];

  FILE* fp;
READ:
  fp = fopen(temp, "r");
  if (!fp) {
    perror(temp);
    clearmem();
    exit(-1);
  }

  while (fgets(input, MAX_BUFFER_LENGTH, fp) != NULL) {
    input[strlen(input) - 1] = 0;
    char* test = strtok(input, " \t");
    if (!strcmp(temp, argv[argc - 2])) {
      addcourse();
    } else {
      addstudent();
    }
  }

  fclose(fp);
  if (strcmp(temp, argv[argc - 1])) {
    temp = argv[argc - 1];
    goto READ;
  }

  while (fgets(input, MAX_BUFFER_LENGTH, stdin)) {
    input[strlen(input) - 1] = 0;
    temp = strtok(input, " \t");
    if (!strcmp(temp, "student")) {
      addstudent();
    } else if (!strcmp(temp, "course")) {
      addcourse();
    } else if (!strcmp(temp, "add")) {
      enrollstudent();
    } else if (!strcmp(temp, "drop")) {
      int sid = atof(strtok(NULL, " \t"));
      char* temp = strtok(NULL, " \t");
      char tempid[3];
      short cid;

      tempid[0] = temp[0];
      tempid[1] = temp[1];
      tempid[2] = 0;
      temp += 2;
      cid = atof(temp);

      dropstudent(sid, tempid, cid);
    } else if (!strcmp(temp, "remove")) {
      removecourse();
    }
  }
  struct Student* curs = shead;
  struct Course* curc = chead;
  struct Enrollment* cur;
  while (curs) {
    printf("\n");
    printf("Student ");
    printstudent(&curs);
    if (numcourses(&curs) <= 0) {
      printf(" is taking no courses");
    } else {
      printf(" is taking %d courses:\n  ", numcourses(&curs));
      cur = curs->clist;
      while (cur) {
        printf(" %s%03d", cur->course->depid, cur->course->cid);
        cur = cur->nextcourse;
      }
    }
    curs = curs->next;
    printf("\n");
  }
  while (curc) {
    printf("\n");
    printf("Course %s%03d (limit %d) ",
           curc->depid,
           curc->cid,
           curc->size);
    if (numenrolled(&curc) <= 0) {
      printf("is empty\n");
    } else {
      printf("contains %d students:\n", numenrolled(&curc));
      cur = curc->slist;
      while (cur) {
        printf("   ");
        printstudent(&cur->student);
        printf("\n");
        cur = cur->nextstudent;
      }
    }
    curc = curc->next;
  }
  clearmem();
}

void clearmem() {
  struct Student* curs = shead;
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

  struct Course* curc = chead;
  struct Course* prevc;
  while (curc) {
    prevc = curc;
    curc = curc->next;
    unallocate(prevc);
  }
  unallocate(input);
}

void strcopy(char** dest, char** src) {
  *dest = allocate(strlen(*src) + 1);
  if (!dest) {
    printf("Memory allocation failed!\n");
    clearmem();
    exit(-1);
  }
  strcpy(*dest, *src);
}

void addstudent() {
  struct Student* new;
  struct Student* cur;
  int sid;
  char* temp;

  temp = strtok(NULL, " \t");
  sid = atof(temp);
  if (getstudent(sid)) {
    fprintf(stderr, "%d already exists\n", sid);
    return;
  }

  new = allocate(sizeof(struct Student));
  if (!new) {
    printf("Memory allocation failed!\n");
    clearmem();
    exit(-1);
  }
  new->first = 0;
  new->middle = 0;
  new->last = 0;
  new->clist = 0;
  new->next = 0;
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

  if (shead == NULL) {
    shead = new;
  } else if (comparestudent(&new, &shead) < 0) {
    new->next = shead;
    shead = new;
  } else {
    cur = shead;
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

void addcourse() {
  struct Course* new;
  struct Course* cur;
  char* temp;
  char tempid[3];
  short cid;
  temp = strtok(NULL, " \t");
  tempid[0] = temp[0];
  tempid[1] = temp[1];
  tempid[2] = 0;
  temp += 2;
  cid = atof(temp);
  if (getcourse(tempid, cid)) {
    fprintf(stderr, "%s%03d already exists\n", tempid, cid);
    return;
  }

  new = allocate(sizeof(struct Course));
  if (!new) {
    printf("Memory allocation failed!\n");
    clearmem();
    exit(-1);
  }
  new->depid[0] = tempid[0];
  new->depid[1] = tempid[1];
  new->depid[2] = 0;
  new->slist = 0;
  new->next = 0;
  new->cid = cid;
  new->size = atof(strtok(NULL, " \t"));

  if (chead == NULL) {
    chead = new;
  } else if (strcmp(new->depid, chead->depid) < 0 ||
             (!(strcmp(new->depid, chead->depid)) &&
               new->cid < chead->cid)) {
    new->next = chead;
    chead = new;
  } else {
    cur = chead;
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

void enrollstudent() {
  int sid = atof(strtok(NULL, " \t"));
  struct Student* student = getstudent(sid);
  struct Enrollment* cur;
  struct Course* course;
  char* temp = strtok(NULL, " \t");
  char tempid[3];
  short cid;

  tempid[0] = temp[0];
  tempid[1] = temp[1];
  tempid[2] = 0;
  temp += 2;
  cid = atof(temp);

  course = getcourse(tempid, cid);

  if (!student) {
    fprintf(stderr, "%05d does not exist\n", sid);
  } else if (!course) {
    fprintf(stderr, "%s%03d does not exist\n", tempid, cid);
  } else if (numcourses(&student) == 5) {
    fprintf(stderr, "%05d has a full schedule\n", sid);
  } else if (numenrolled(&course) == course->size) {
    fprintf(stderr, "%s%03d is full\n", tempid, cid);
  } else if (hasstudent(&course, sid)) {
    fprintf(stderr, "%05d already enrolled in %s%03d\n",
            sid,
            course->depid,
            course->cid);
  } else {
    struct Enrollment* enrollment = allocate(sizeof(struct Enrollment));
    if (!enrollment) {
      printf("Memory allocation failed!\n");
      clearmem();
      exit(-1);
    }
    enrollment->nextstudent = 0;
    enrollment->nextcourse = 0;
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
    if (verbose) {
      printf("%05d added to %s%03d\n",
             enrollment->student->sid,
             enrollment->course->depid,
             enrollment->course->cid);
    }
  }
}

void dropstudent(int sid,
                 char depid[2],
                 short cid) {
  struct Student* student = getstudent(sid);
  struct Course* course = getcourse(depid, cid);
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
    if (verbose) {
      printf("%05d dropped from %s%03d\n",
             student->sid,
             course->depid,
             course->cid);
    }
    unallocate(todelete);
  }
}

void removecourse() {
  struct Course* course;
  struct Enrollment* enrollment;
  char* temp = strtok(NULL, " \t");
  char tempid[3];
  short cid;

  tempid[0] = temp[0];
  tempid[1] = temp[1];
  tempid[2] = 0;
  temp += 2;
  cid = atof(temp);

  course = getcourse(tempid, cid);

  if (!course) {
    printf("%s%03d does not exist\n", tempid, cid);
    return;
  }

  enrollment = course->slist;
  uint8_t v = verbose;
  verbose = 0;
  while (enrollment != NULL && enrollment->student != NULL) {
    dropstudent(enrollment->student->sid,
                course->depid,
                course->cid);
    enrollment = enrollment->nextstudent;
  }
  if (!strcmp(course->depid, chead->depid) &&
      course->cid == chead->cid) {
    chead = chead->next;
  } else {
    struct Course* cur = chead;
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

struct Student* getstudent(int sid) {
  struct Student* cur = shead;
  while (cur && cur->sid != sid) {
    cur = cur->next;
  }
  return cur;
}

struct Course* getcourse(char depid[2], short cid) {
  struct Course* cur = chead;
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
    return 1;
  } else {
    return 0;
  }
}

void printstudent(struct Student** student) {
  printf("%05d (%s", (*student)->sid, (*student)->last);
  if ((*student)->first != 0) printf(" %s", (*student)->first);
  if ((*student)->middle != 0) printf(" %s", (*student)->middle);
  printf(")");
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

uint8_t numenrolled(struct Course** course) {
  struct Enrollment* cur = (*course)->slist;
  uint8_t numenrolled = 0;
  while (cur) {
    cur = cur->nextstudent;
    ++numenrolled;
  }
  return numenrolled;
}

uint8_t numcourses(struct Student** student) {
  struct Enrollment* cur = (*student)->clist;
  uint8_t numcourses = 0;
  while (cur) {
    cur = cur->nextcourse;
    ++numcourses;
  }
  return numcourses;
}
