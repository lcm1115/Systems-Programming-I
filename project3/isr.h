/*
** File:        isr.h
**
** Author:      Liam Morris
**
** Description: Declares functions for use in ISR. Refer to isr.c for specific
**              documentation regarding functionality.
*/

// Implementation of strcpy. Copies one string into another.
// Arguments:
//   src - buffer to copy data into
//   dest - buffer to copy data from
void strcpy(char* src, char* dest);

// Implementation of strcat. Concatenates two strings.
// Arguments:
//   src - buffer to concatenate data onto.
//   dest - buffer to read data from to concatenate
void strcat(char* src, char* dest);

// ISR handling for clock cycle.
// Increments tick_count on each interrupt.
void clock_isr(int vector, int code);

// Writes a buffer to the terminal.
// Arguments:
//   user_buffer - buffer to write to the terminal
void serial_write(char* user_buffer);

// Reads a character from the terminal.
// Returns:
//   ch - byte that was read from the terminal
int serial_getchar(void);

// Writes an integer to the terminal.
// Arguments:
//   d - the int to be written
void serial_putd(int d);

// Writes a double to the terminal.
// Arguments:
//   x - the double to be written
void serial_putx(double x);

// ISR for the terminal.
// Reads in bytes and handles all interrupts as needed.
void serial_isr(int vector, int code);

// Initialize the terminal as specified in the project description.
void init(void);
