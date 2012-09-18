#ifndef SIS_H_
#define SIS_H

#define MAX_BUFFER_LENGTH 255
#define TRUE 1
#define FALSE 0

struct Course;
struct Enrollment;

// Representation of a Student node in a linked list.
// Contains student ID, student name, a student's course list, and a pointer to
// the next Student in the linked list.
struct Student {
  int sid;
  int numcourses;
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
  char depid[2];
  int cid;
  int numenrolled;
  int size;
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

// Initializes a student.
struct Student* initstudent();

// Initializes a course.
struct Course* initcourse();

// Initializes an enrollment.
struct Enrollment* initenrollment();

// Adds a student to the database.
void addstudent(struct Student** shead);

void addcourse(struct Course** chead);

void enrollstudent(struct Student** shead, struct Course** chead);

void dropstudent(struct Student** shead,
                 struct Course** chead,
                 int sid,
                 char depid[2],
                 int cid);

void removecourse(struct Student** shead, struct Course** chead);

struct Student* getstudent(int sid, struct Student** shead);

struct Course* getcourse(char depid[2], int cid, struct Course** chead);

int hasstudent(struct Course** course, int sid);

// Prints a student's information to the console.
void printstudent(struct Student** student);

void printcourse(struct Course** course);

void courselist(struct Course** chead);

void readfiles(struct Student** shead,
               struct Course** chead,
               char** coursefile,
               char** studentfile,
               char inputbuffer[]);

int comparestudent(struct Student** stu1, struct Student** stu2);
#endif
