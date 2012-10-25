/*
** File:	main.c
**
** Author:	Liam Morris
**
** Description:	Main program for executing a reaction timer program on an ADDS
**              terminal. This file contains the logic for the actual flow of
**              the program and relies on functions contained in the ISR for
**              sending and receiving data to and from the terminal.
*/
#include "c_io.h"
#include "isr.h"
#include "rngs.h"
#include "startup.h"
#include "support.h"
#include "uart.h"
#include "x86arch.h"

// Boolean that determines if a new character needs to be processed in the
// program (character was just read from keyboard).
int newchar = 0;

// Number of clock cycles that have occurred.
int tick_count = 0;

// Number of rounds that have passed.
int num_rounds = 0;

// Array that holds int values so that numbers may be printed to the terminal.
int nums[256];

// Current character in the buffer (buffer size of 1)
char c = 0;

char* next_output_ch = 0;

char BEL = 0x7;

int main( void ) {
  // Initialize terminal.
  init();

  // Install ISR for clock cycle and serial port.
  __install_isr(INT_VEC_TIMER, clock_isr);
  __install_isr(INT_VEC_SERIAL_PORT_1, serial_isr);

  // Enable interrupts
  __asm("sti");

  // Enable terminal.
  __outb(UA4_IER, 0xf);

  // Clear the screen.
  __outb(UA4_TXD, 0x1a);
  c_puts("Cleared Screen\n");

  // Print basic information to the terminal.
  char seed[12];
  serial_write("Team:\n\rLiam Morris\n\r");
  serial_write("Clock is ticking at the default rate\n\r");
  serial_write("Enter twelve-character seed: ");

  // Read in seed from terminal.
  for (int i = 0; i < 12; ++i) {
    newchar = 0;
    while (!newchar);
    // If new line is entered, send BEL to terminal.
    if (c == 10 || c == 15) {
      --i;
      serial_write(&BEL);
    }
    // Otherwise, store character in buffer.
    else {
      seed[i] = c;
      serial_write(&c);
    }
  }
  c_printf("Seed: %s\n", seed);
  
  unsigned int vals[3];

  // For each 4 byte segment, calculate it's value in little endian order.
  for (int i = 0; i < 3; ++i) {
    vals[i] = 0;
    for (int j = 0; j < 4; j++) {
      vals[i] |= seed[i * 4 + j] << (8 * j);
    }
  }

  // XOR the values together to get the seed, then set the seed.
  unsigned int sd = vals[0] ^ vals[1] ^ vals[2];
  PutSeed(sd);

  unsigned int total_clocks = 0;

  // Start the program!
  while (1) {
    ++num_rounds;
    serial_write("\n\rPress the enter key when you are ready\n\r");
    c = 0;

    // Wait for enter key.
    while (c != 10 && c != 15);

    // Get random character [a-z], print it to the terminal, and reset tick
    // counter to 0.
    int ch = Random() * 25 + 'a';
    serial_write(&ch);
    tick_count = 0;

    // Wait for correct character to be entered.
    // Each time an incorrect character is entered, send BEL to terminal.
    while (c != ch) {
      serial_write(&BEL);
      newchar = 0;
      while (!newchar);
    }

    // Store current tick count in a variable (just in case it increments again
    // before we're done using it!), then add it to total ticks
    int ticks = tick_count;
    total_clocks += ticks;
    double average = (double) total_clocks / (double) num_rounds;
    c_printf("\nTicks: %d Total Clocks: %d Num Rounds: %d\n",
              ticks, total_clocks, num_rounds);

    // Print timing statistics.
    serial_write("\n\rCurrent:  ");
    serial_putd(ticks);
    serial_write("   Average:  ");
    serial_putx(average);
    serial_write(" (");
    serial_putd(num_rounds);
    serial_write(" rounds)");

    // Prompt for another round.
    serial_write("\n\rAnother try?\n\r");
    c = 0;
    while (1) {
      newchar = 0;
      while (!newchar);
      // If 'y' or 'n' is entered, break out of the loop.
      if (c == 'y' || c == 'n') {
        break;
      }
      // For all other characters, send a BEL to the terminal.
      else {
        serial_write(&BEL);
      }
    }
    // If user enters 'n', exit program.
    if (c == 'n') {
      serial_write(":(");
      __delay(1);
      break;
    }
  }
  return (0);
}
