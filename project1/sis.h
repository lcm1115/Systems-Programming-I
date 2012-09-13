#ifndef SIS_H_
#define SIS_H

#define MAX_BUFFER_LENGTH 255

struct Course;

struct Student {
  int sid;
  char* first;
  char* middle;
  char* last;
  struct Course* clist;
  struct Student* next;
};

struct Course {
  char depid[2];
  int cid;
  struct Student* slist;
  struct Course* next;
};

void strcopy(char** dest, char** src);

struct Student* initstudent();

struct Course* initcourse();

void addstu(struct Student** shead);

int checksid(int sid, struct Student** shead);

int checkcid(int cid, struct Course** chead);

void printstu(struct Student** student);

void stulist(struct Student** shead);
#endif
