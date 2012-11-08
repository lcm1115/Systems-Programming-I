extern char** command;

extern int curIndex;

extern int prevIndex;

extern int numArgs;

extern int numProcesses;

extern int fd[2];

extern int read_fd;

extern int write_fd;

char* strdup(char* str);

void freeCommand();

int validateInput(int argc, char** argv);

int nextSpecial(int startIndex, char** argv);

void getNextCommand(int startIndex, int endIndex, char** argv);

void executeNext(int argc, char** argv);
