#ifndef SIS_H_
#define SIS_H

#define MAX_BUFFER_LENGTH 255

struct Course;

struct Student {
  int sid;
  char* first, middle, last;
  Course* clist;
  Student* next;
};

struct Course {
  char depid[2];
  int cid;
  Student* slist;
  Course* next;
}

#endif
