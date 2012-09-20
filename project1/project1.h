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

  // Representation of course schedule for a student.
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

  // Representation of a student roster for a course.
  struct Enrollment* slist;
  struct Course* next;
};

// Representation of an "Enrollment" in a linked list.
// When a student registers for a course, an Enrollment is initialized with the
// course and the student, as well as any pointers to the next courses or
// students in the enrollment linked list.
struct Enrollment {
  struct Student* student;
  struct Course* course;
  struct Enrollment* nextstudent;
  struct Enrollment* nextcourse;
};

// Copies a string from one buffer to another.
// Allocates memory for destination buffer before copying.
void strcopy(char** dest, char** src);

// Adds a student to the database.
void addstudent();

// Adds a course to the database.
void addcourse();

// Enrolls a student in a course.
void enrollstudent();

// Drops a student from a course.
void dropstudent(int sid, char depid[2], short cid);

// Removes a course from a database and removes it from all students' schedules
// that were enrolled in it.
void removecourse();

// Returns a pointer to a student with a specificed ID.
// Returns NULL if student does not exist.
struct Student* getstudent(int sid);

// Returns a pointer to a course with a specified department and course ID.
// Returns NULL if course does not exist.
struct Course* getcourse(char depid[2], short cid);

// Determines whether a student (with a specified ID) is enrolled in a course.
// Returns 1 if the student is found, 0 otherwise.
uint8_t hasstudent(struct Course** course, int sid);

// Prints out a student to the console in one of the following formats:
//     sid (name)
//     sid (last first)
//     sid (last first middle)
void printstudent(struct Student** student);

// Compares two students for equality.
// Student fields are compared in the order last > first > middle > sid.
// If stu1 is less than stu2, -1 is returned. If stu2 is less than stu1, 1 is
// returned.
int comparestudent(struct Student** stu1, struct Student** stu2);

// Deallocates all Student, Course, and Enrollment instances.
void clearmem();

// Returns the number of students enrolled in a course.
uint8_t numenrolled(struct Course** course);

// Returns the number of courses a student is enrolled in.
uint8_t numcourses(struct Student** student);
#endif
