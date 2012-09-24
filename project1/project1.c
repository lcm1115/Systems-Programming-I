/* 
** File: project1.c
** Author: Liam Morris - lcm1115
** Description: Implements a basic course and student database. Allows for
**              adding new courses, adding new students, registering students
**              for courses, dropping students from courses, and removing
**              courses from the system.
**
**              Usage:
**                project1 [-v] courses.file students.file
**
**              If the flag -v is set, verbose mode is enabled which prints out
**              messages when certain commands finish.
*/
#include <allocate.h>
#include "project1.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Verbosity flag
uint8_t verbose;

// Heads of student and course linked lists.
struct Student* shead = 0;
struct Course* chead = 0;

// Input buffer
char* input;

int main(int argc, char** argv) {
  input = allocate(MAX_BUFFER_LENGTH);
  if (!input) {
    printf("Memory allocation failed!\n");
    clearmem();
    exit(-1);
  }
  char* temp;
  // Checks that flags are valid.
  if (argc < 3 || argc > 4 || (argc == 4 && strcmp(argv[1], "-v"))) {
    fprintf(stderr, "usage: project1 [-v] course_file student_file\n");
    return 1;
  }
  // Set verbosity if necessary.
  if (argc == 4) {
    verbose = 1;
  }
  temp = argv[argc - 2];

  // Read in course and student files
  FILE* fp;
READ:
  fp = fopen(temp, "r");
  if (!fp) {
    perror(temp);
    clearmem();
    exit(-1);
  }

  // Read each line and process call the corresponding function.
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
  // If student file hasn't been read yet, read it.
  if (strcmp(temp, argv[argc - 1])) {
    temp = argv[argc - 1];
    goto READ;
  }

  // Read commands until no input is entered.
  while (fgets(input, MAX_BUFFER_LENGTH, stdin)) {
    // Strip \n from end of the command.
    input[strlen(input) - 1] = 0;

    // Call appropriate function depending on which command was issued.
    temp = strtok(input, " \t");
    if (!strcmp(temp, "student")) {
      addstudent();
    } else if (!strcmp(temp, "course")) {
      addcourse();
    } else if (!strcmp(temp, "add")) {
      enrollstudent();
    } else if (!strcmp(temp, "drop")) {
      // Parse student and course ID information and then call dropstudent.
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

  // After all input has been processed, print out information about the
  // students and courses.
  struct Student* curs = shead;
  struct Course* curc = chead;
  struct Enrollment* cur;

  // Iterate across students list
  while (curs) {
    printf("\n");
    printf("Student ");
    printstudent(&curs);

    // If a student is taking courses, print them.
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

  // Iterate across course lists.
  while (curc) {
    printf("\n");
    printf("Course %s%03d (limit %d) ",
           curc->depid,
           curc->cid,
           curc->size);

    // If students are enrolled in the course, print them.
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

  // Unallocate all memory that was allocated.
  clearmem();
}

// Unallocates all dynamically allocated memory
void clearmem() {
  struct Student* curs = shead;
  struct Student* prevs;
  struct Enrollment* cure;
  struct Enrollment* preve;
  // Iterate across the student list and delete each student.
  // Additionally, delete all enrollments.
  while (curs) {
    prevs = curs;
    curs = curs->next;
    cure = prevs->clist;
    while (cure) {
      preve = cure;
      cure = cure->nextcourse;
      unallocate(preve);
    }

    // Unallocate all dynamically allocated memory from the student, then
    // delete the actual student.
    unallocate(prevs->first);
    unallocate(prevs->middle);
    unallocate(prevs->last);
    unallocate(prevs);
  }

  // Iterate across the course and delete each course.
  struct Course* curc = chead;
  struct Course* prevc;
  while (curc) {
    prevc = curc;
    curc = curc->next;
    unallocate(prevc);
  }
  unallocate(input);
}

// Allocates memory for a destination buffer, then copies the contents of a
// source buffer into it.
void strcopy(char** dest, char** src) {
  // Allocate strlen + 1 bytes for the new buffer (to account for NULL-byte)
  *dest = allocate(strlen(*src) + 1);
  if (!dest) {
    printf("Memory allocation failed!\n");
    clearmem();
    exit(-1);
  }

  // Copy contents
  strcpy(*dest, *src);
}

// Adds a new student to the database.
void addstudent() {
  struct Student* new;
  struct Student* cur;
  int sid;
  char* temp;

  // Extract sid from input buffer and make sure that student doesn't already
  // exist.
  temp = strtok(NULL, " \t");
  sid = atof(temp);
  if (getstudent(sid)) {
    fprintf(stderr, "%d already exists\n", sid);
    return;
  }

  // Allocate memory for the new student and initialize all fields.
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

  // Extract all names possible from input.
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

  // If student linked list is empty, make student the new head.
  if (shead == NULL) {
    shead = new;
  }
  // If the student is "less than" the head, make it the new head.
  else if (comparestudent(&new, &shead) < 0) {
    new->next = shead;
    shead = new;
  }
  // Otherwise, place the student where it belongs in the list.
  else {
    cur = shead;
    // Iterate until the end of the list is found or the current position is
    // the proper position for the new student.
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

// Adds a new course to the database.
void addcourse() {
  struct Course* new;
  struct Course* cur;
  char* temp;
  char tempid[3];
  short cid;

  // Parse course ID information and make sure it doesn't already exist.
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

  // Allocate new course and initialize all fields.
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

  // If course list is currently empty, make the new course the head.
  if (chead == NULL) {
    chead = new;
  }
  // If the course is "less than" the current head, make it the new head.
  else if (strcmp(new->depid, chead->depid) < 0 ||
             (!(strcmp(new->depid, chead->depid)) &&
               new->cid < chead->cid)) {
    new->next = chead;
    chead = new;
  }
  // Otherwise, insert the course in its proper position.
  else {
    cur = chead;
    // Find approximate position by department ID.
    while (cur->next && strcmp(cur->next->depid, new->depid) < 0) {
      cur = cur->next;
    }
    // If at the end of the list, put new course on the end of the list.
    if (!cur->next) cur->next = new;
    // Otherwise, find exact position by course ID within department ID.
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

// Enrolls a student in a course.
void enrollstudent() {
  // Get pointer to student.
  int sid = atof(strtok(NULL, " \t"));
  struct Student* student = getstudent(sid);
  struct Enrollment* cur;
  struct Course* course;

  // Get pointer to course.
  char* temp = strtok(NULL, " \t");
  char tempid[3];
  short cid;

  tempid[0] = temp[0];
  tempid[1] = temp[1];
  tempid[2] = 0;
  temp += 2;
  cid = atof(temp);

  course = getcourse(tempid, cid);

  // Check that:
  //     Student exists
  //     Course exists
  //     Student has room in schedule
  //     Course has room in roster
  //     Student isn't already enrolled in course.
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
    // Allocate enrollment and initialize all fields.
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
    // If student's schedule is empty, make this course head of the list.
    if (student->clist == NULL) {
      student->clist = enrollment;
    }
    // If first entry in student's schedule is greater than new enrollment,
    // make the new enrollment the first entry.
    else if (strcmp(enrollment->course->depid, student->clist->course->depid) < 0 ||
               (!(strcmp(enrollment->course->depid, student->clist->course->depid)) &&
                 enrollment->course->cid < student->clist->course->cid)) {
      enrollment->nextcourse = student->clist;
      student->clist = enrollment;
    }
    // Otherwise insert course in proper location in schedule.
    else {
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
    // If course roster is empty, make new enrollment the first entry.
    if (course->slist == NULL) {
      course->slist = enrollment;
    }
    // If first student in roster comes after new enrollment's student
    // alphabetically, make the new enrollment the new first entry.
    else if (comparestudent(&enrollment->student, &course->slist->student) < 0) {
      enrollment->nextstudent = course->slist;
      course->slist = enrollment;
    }
    // Otherwise insert the new enrollment in to the roster in the proper
    // position (alphabetically by student).
    else {
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

// Drops a student from a course.
void dropstudent(int sid,
                 char depid[2],
                 short cid) {
  // Get pointers to student and course.
  struct Student* student = getstudent(sid);
  struct Course* course = getcourse(depid, cid);
  struct Enrollment* todelete;

  // Check that:
  //     Student exists
  //     Course exists
  //     Student is enrolled in the course
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
    // Find enrollment listing in course's roster and remove it from the linked
    // list.
    struct Enrollment* cur = course->slist;
    if (!cur->nextstudent || cur->student->sid == student->sid) {
      course->slist = cur->nextstudent;
    } else {
      while (cur->nextstudent->student->sid != student->sid) {
        cur = cur->nextstudent;
      }
      cur->nextstudent = cur->nextstudent->nextstudent;
    }
    // Find enrollment listing in student's schedule and remove it from the
    // linked list.
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
    // Delete enrollment.
    unallocate(todelete);
  }
}

// Drops all students from a course and removes it from the database.
void removecourse() {
  struct Course* course;
  struct Enrollment* enrollment;

  // Get department and course ID and get a pointer to the course.
  char* temp = strtok(NULL, " \t");
  char tempid[3];
  short cid;

  tempid[0] = temp[0];
  tempid[1] = temp[1];
  tempid[2] = 0;
  temp += 2;
  cid = atof(temp);

  course = getcourse(tempid, cid);

  // Make sure course exists.
  if (!course) {
    printf("%s%03d does not exist\n", tempid, cid);
    return;
  }

  enrollment = course->slist;

  // Disable verbosity while dropping students.
  uint8_t v = verbose;
  verbose = 0;
  while (enrollment != NULL && enrollment->student != NULL) {
    // Drop all students in the course's roster.
    dropstudent(enrollment->student->sid,
                course->depid,
                course->cid);
    enrollment = enrollment->nextstudent;
  }
  // Remove course from course list.
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
  // Delete course.
  unallocate(course);
}

// Returns a pointer to a Student with a specified ID
struct Student* getstudent(int sid) {
  struct Student* cur = shead;
  // Iterate through linked list until student ID is found.
  while (cur && cur->sid != sid) {
    cur = cur->next;
  }
  return cur;
}

// Returns a pointer to a Course with a specified department ID and course ID
struct Course* getcourse(char depid[2], short cid) {
  struct Course* cur = chead;
  // Iterate through linked list until department ID and course ID is found.
  while (cur && (strcmp(depid, cur->depid) || cid != cur->cid)) {
    cur = cur->next;
  }
  return cur;
}

// Returns whether or not a Student is specified in a Course
uint8_t hasstudent(struct Course** course, int sid) {
  // Iterate through roster until student with specified student ID is found.
  // Return 1 if it is found, return 0 otherwise.
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

// Prints a student ID and name(s) in the following format:
// SID (Last [First [Middle]])
void printstudent(struct Student** student) {
  printf("%05d (%s", (*student)->sid, (*student)->last);
  if ((*student)->first != 0) printf(" %s", (*student)->first);
  if ((*student)->middle != 0) printf(" %s", (*student)->middle);
  printf(")");
}

// Compares tow students for equality.
int comparestudent(struct Student** stu1, struct Student** stu2) {
  // Compare last names.
  if (strcmp((*stu1)->last, (*stu2)->last) < 0) {
    return -1;
  } else if (strcmp((*stu1)->last, (*stu2)->last) > 0) {
    return 1;
  } else {
    // Compare first names.
    if (strcmp((*stu1)->first, (*stu2)->first) < 0) {
      return -1;
    } else if (strcmp((*stu1)->first, (*stu2)->first) > 0) {
      return 1;
    } else {
      // Compare middle names.
      if (strcmp((*stu1)->middle, (*stu2)->middle) < 0) {
        return -1;
      } else if (strcmp((*stu1)->middle, (*stu2)->middle) > 0) {
        return 1;
      } else {
        // Compare student ID.
        return (*stu1)->sid - (*stu2)->sid;
      }
    }
  }
}

// Returns the number of Students enrolled in a Course
uint8_t numenrolled(struct Course** course) {
  // Iterate through linked list and count how many students are looked at.
  struct Enrollment* cur = (*course)->slist;
  uint8_t numenrolled = 0;
  while (cur) {
    cur = cur->nextstudent;
    ++numenrolled;
  }
  return numenrolled;
}

// Returns the number of Courses a Student is enrolled in.
uint8_t numcourses(struct Student** student) {
  // Iterate through linked list and count how many courses are looked at.
  struct Enrollment* cur = (*student)->clist;
  uint8_t numcourses = 0;
  while (cur) {
    cur = cur->nextcourse;
    ++numcourses;
  }
  return numcourses;
}
