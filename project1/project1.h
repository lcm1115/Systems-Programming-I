/*
** File: project1.h
** Author: Liam Morris - lcm1115
** Description: Declares functions, constants, and structures for a basic course
**              and student database.
*/

#ifndef PROJECT1_H_
#define PROJECT1_H

#include <allocate.h>
#include <stdint.h>

// Maximum buffer length, in bytes, to be used for input buffer.
#define MAX_BUFFER_LENGTH 255

struct Course;
struct Enrollment;

// Representation of a Student node in a linked list.
// Contains student ID, student name, a student's course list, and a pointer to
// the next Student in the linked list.
//
// Fields:
//     sid - Student ID
//     first - Student's first name
//     middle - Student's middle name
//     last - Student's last name (or just 'name' if the student only has
//            one name)
//     clist - Student's schedule (pointer to linked list of Enrollments)
//     next - next Student in the Student linked list
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
//
// Fields:
//     depid - 2 character representation of a department ID
//     cid - Course ID
//     size - maximum size of the course roster
//     slist - Course roster (pointer to linked list of Enrollments)
//     next - next Course in the Course linked list
struct Course {
  char depid[3];
  short cid;
  uint8_t size;
  struct Enrollment* slist;
  struct Course* next;
};

// Representation of an "Enrollment" in a linked list.
// When a student registers for a course, an Enrollment is initialized with the
// course and the student, as well as any pointers to the next courses or
// students in the enrollment linked list.
//
// Fields:
//     student - pointer to Student associated with this Enrollment
//     course - pointer to Course associated with this Enrollment
//     nextstudent - pointer to next Student in Enrollment linked list
//     nextcourse - pointer to next Course in Enrollment linked list
struct Enrollment {
  struct Student* student;
  struct Course* course;
  struct Enrollment* nextstudent;
  struct Enrollment* nextcourse;
};

// Copies a string from one buffer to another.
// Allocates memory for destination buffer before copying.
//
// Arguments:
//     dest - destination buffer in which to copy data
//     src - source buffer from which to copy data
void strcopy(char** dest, char** src);

// Adds a student to the database.
void addstudent();

// Adds a course to the database.
void addcourse();

// Enrolls a student in a course.
void enrollstudent();

// Drops a student from a course.
//
// Arguments:
//     sid - Student ID to drop from course
//     depid - department ID of course that is being dropped
//     cid - course ID of course that is being dropped
void dropstudent(int sid, char depid[2], short cid);

// Removes a course from a database and removes it from all students' schedules
// that were enrolled in it.
void removecourse();

// Returns a pointer to a student with a specificed ID.
// Returns NULL if student does not exist.
//
// Arguments:
//     sid - Student ID of Student to look for
struct Student* getstudent(int sid);

// Returns a pointer to a course with a specified department and course ID.
// Returns NULL if course does not exist.
//
// Arguments:
//     depid - department ID of course to look for
//     cid - course ID of course to look for
struct Course* getcourse(char depid[2], short cid);

// Determines whether a student (with a specified ID) is enrolled in a course.
// Returns 1 if the student is found, 0 otherwise.
//
// Arguments:
//     course - pointer to Course
//     sid - Student ID to check
uint8_t hasstudent(struct Course** course, int sid);

// Prints out a student to the console in one of the following formats:
//     sid (name)
//     sid (last first)
//     sid (last first middle)
//
// Arguments:
//     student - pointer to Student to print
void printstudent(struct Student** student);

// Compares two students for equality.
// Student fields are compared in the order last > first > middle > sid.
// If stu1 is less than stu2, -1 is returned. If stu2 is less than stu1, 1 is
// returned.
//
// Arguments:
//     stu1 - pointer to first Student being compared
//     stu2 - pointer to second Student being compared
int comparestudent(struct Student** stu1, struct Student** stu2);

// Deallocates all Student, Course, and Enrollment instances.
void clearmem();

// Returns the number of students enrolled in a course.
//
// Arguments:
//     course - pointer to Course
uint8_t numenrolled(struct Course** course);

// Returns the number of courses a student is enrolled in.
//
// Arguments:
//     student - pointer to Student
uint8_t numcourses(struct Student** student);
#endif
