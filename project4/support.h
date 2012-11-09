/*
** File:        support.h
** Author:      Liam Morris (lcm1115)
** Description: This file contains the function prototypes for support functions
**              for the pipeline, such as validating input and getting a command
**              from command line arguments.
*/

// Global Variables
// Current command being processed.
extern char** command;

// Current index in argv when searching for commands.
extern int curIndex;

// Previous value of curIndex when searching for commands.
extern int prevIndex;

// Number of arguments in the current command.
extern int numArgs;

// Number of processes remaining to execute.
extern int numProcesses;

// File descriptors for I/O redirection with pipes.
extern int fd[2];

// File descriptors for file I/O.
extern int read_fd;
extern int write_fd;

// Implementation of strdup.
// Arguments:
//   str - string to be copied
// Returns:
//   A new cstring containing whose content is the same as str.
char* strdup(char* str);

// Frees all cstrings in a given command buffer.
void freeCommand();

// Validates input and ensures that command line input is legal.
int validateInput(int argc, char** argv);

// Gets the index of the next special character in argv starting from a given
// start index and working backwards.
// Arguments:
//   startIndex - index to start from when searching for next special character.
//   argv - command line arguments
// Returns:
//   An integer representing the index of the next special character, or -1 if
//   no special character is found.
int nextSpecial(int startIndex, char** argv);

// Get's the next command and command arguments between two indices of argv.
// Arguments:
//   startIndex - index to start searching in argument array
//   endIndex - index to end searching in argument array
//   argv - command line arguments
void getNextCommand(int startIndex, int endIndex, char** argv);

// Execute next command on the pipeline.
// Arguments:
//   argc - number of command line arguments given
//   argv - command line arguments
void executeNext(int argc, char** argv);
