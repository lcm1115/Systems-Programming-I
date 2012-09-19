#ifndef PROJECT1_H_
#define PROJECT1_H

#include "allocate.h"
#include <stdint.h>

#define MAX_BUFFER_LENGTH 255

struct Course;
struct Enrollment;

// Representation of a Student node in a linked list.
// Contains student ID, student name, a student's course list, and a pointer to
// the next Student in the linked list.
struct Student {
  int sid;
  char* first;
  char* middle;
  char* last;
  struct Enrollment* clist;
  struct Student* next;
};

// Representation of a Course node in a linked list.
// Contains department ID, course ID, size, a course's student list, and a
// pointer to the next Course in the linked list.
struct Course {
  char depid[3];
  short cid;
  uint8_t size;
  struct Enrollment* slist;
  struct Course* next;
};

struct Enrollment {
  struct Student* student;
  struct Course* course;
  struct Enrollment* nextstudent;
  struct Enrollment* nextcourse;
};

// Copies a string from one buffer to another.
void strcopy(char** dest, char** src);

// Adds a student to the database.
void addstudent();

void addcourse();

void enrollstudent();

void dropstudent(int sid, char depid[2], short cid);

void removecourse();

struct Student* getstudent(int sid);

struct Course* getcourse(char depid[2], short cid);

int hasstudent(struct Course** course, int sid);

void printstudent(struct Student** student);

void readfiles(char** coursefile, char** studentfile);

int comparestudent(struct Student** stu1, struct Student** stu2);

void clearmem();

uint8_t numenrolled(struct Course** course);

uint8_t numcourses(struct Student** student);
#endif
