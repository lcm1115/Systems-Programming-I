#ifndef SIS_H_
#define SIS_H

#define MAX_BUFFER_LENGTH 255
#define TRUE 1
#define FALSE 0

struct Course;

// Representation of a Student node in a linked list.
// Contains student ID, student name, a student's course list, and a pointer to
// the next Student in the linked list.
struct Student {
  int sid;
  char* first;
  char* middle;
  char* last;
  struct Course* clist;
  struct Student* next;
};

// Representation of a Course node in a linked list.
// Contains department ID, course ID, size, a course's student list, and a
// pointer to the next Course in the linked list.
struct Course {
  char depid[2];
  int cid;
  int size;
  struct Student* slist;
  struct Course* next;
};

// Copies a string from one buffer to another.
void strcopy(char** dest, char** src);

// Initializes a student.
struct Student* initstudent();

// Initializes a course.
struct Course* initcourse();

// Adds a student to the database.
void addstu(struct Student** shead);

void addcourse(struct Course** chead);

// Checks if a student ID already exists.
// Returns 1 if it exists, 0 otherwise.
int checksid(int sid, struct Student** shead);

// Checks if a course ID already exists.
// Returns 1 if it exists, 0 otherwise.
int checkcid(char depid[2], int cid, struct Course** chead);

// Prints a student's information to the console.
void printstu(struct Student** student);

void printcourse(struct Course** course);

void stulist(struct Student** shead);

void courselist(struct Course** chead);
#endif
