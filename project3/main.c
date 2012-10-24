/*
** File:	main.c
**
** Author:	Liam Morris
**
** Description:	Dummy main program
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
extern char c = 0;

// ISR handling for clock cycle.
// Increments tick_count on each interrupt.
void clock_isr(int vector, int code) {
  ++tick_count;

  // Send end of interrupt byte.
  __outb(PIC_MASTER_CMD_PORT, PIC_EOI);
}

// Writes a character to the terminal.
// Arguments:
//   ch - character to write to the terminal
void serial_putchar(char ch) {
  // If character is a new line, replace it with a carriage return.
  if (ch == '\n') {
    serial_putchar('\r');
  }

  // Wait until ready to transmit.
  while ((__inb(UA4_LSR) & UA4_LSR_TXRDY) == 0);

  // Send byte to terminal.
  __outb(UA4_TXD, ch);
}

// Writes a string to the terminal.
// Arguments:
//   buffer - null-terminated string to be written to the terminal
void serial_puts(char* buffer) {
  char ch;
  // Write each character in the buffer until null is reached.
  while ((ch = *buffer++) != '\0') {
    serial_putchar(ch);
  }
}

// Reads a character from the terminal.
// Returns:
//   ch - byte that was read from the terminal
int serial_getchar(void) {
  int ch;

  // Strip parity bit if it exists.
  ch = __inb(UA4_RXD) & 0x7f;

  // If the character is a carriage return, set it to a new line.
  if (ch == '\r') {
    ch = '\n';
  }

  return ch;
}

// Writes an integer to the terminal.
// Arguments:
//   d - the int to be written
void serial_putd(int d) {
  int i = 0;
  while (d > 0) {
    // Get current digit to write, then store it in array
    int n = d % 10;
    d /= 10;
    nums[i] = n;
    ++i;
  }
  --i;

  // Iterate across array backwards and write each value to the terminal.
  while (i >= 0) {
    serial_putchar('0' + nums[i]);
    --i;
  }
}

// Writes a double to the terminal.
// Arguments:
//   x - the double to be written
void serial_putx(double x) {
  // Get the int portion of the double and write it to the terminal.
  int d = (int) x;
  serial_putd(d);

  // We need a decimal point!
  serial_putchar('.');

  // Take the decimal value and print it to the terminal.
  int r = (int) (x * 10) % 10;
  serial_putd(r);
}

// ISR for the terminal.
// Reads in bytes and handles all interrupts as needed.
void serial_isr(int vector, int code) {
  int br = 0;
  // Loop until there is no interrupt in the terminal.
  while (!br) {
    // Get the interrupt ID
    switch (__inb(UA4_EIR) & UA4_EIR_INT_PRI_MASK) {
      // Reset the status of the terminal for each interrupt that we are not
      // currently handling, otherwise read in a char and flip the global flag
      // so that the program knows to process that char.
      case UA4_EIR_LINE_STATUS:
        __inb(UA4_LSR);
        break;
      case UA4_EIR_RX_HIGH:
        c = serial_getchar();
        newchar = 1;
        break;
      case UA5_EIR_RX_FIFO_TO:
        c = serial_getchar();
        newchar = 1;
        break;
      case UA4_EIR_TX_LOW:
        __inb(UA4_EIR);
        break;
      case UA4_EIR_MODEM_STATUS:
        __inb(UA4_MSR);
        break;
      case UA4_EIR_NO_INT:
        br = 1;
        break;
    }
  }

  // Send end of interrupt byte.
  __outb(PIC_MASTER_CMD_PORT, PIC_EOI);
}

// Initialize the terminal as specified in the project description.
void init(void) {
  // Sends the following to FIFO Control Register:
  //   null byte
  //   enable
  //   enable and receiver soft reset
  //   enable, receiver soft reset, and transmitter soft reset
  __outb(UA4_FCR, 0);
  __outb(UA4_FCR, UA5_FCR_FIFO_EN);
  __outb(UA4_FCR, UA5_FCR_FIFO_EN | UA5_FCR_RXSR);
  __outb(UA4_FCR, UA5_FCR_FIFO_EN | UA5_FCR_RXSR | UA5_FCR_TXSR);

  // Sends a null byte to the Interrupt Enable Register
  __outb(UA4_IER, 0);

  // Sets the baud rate of bank 1 to 9600 bps
  __outb(UA4_LCR, UA4_LCR_BANK1);
  __outb(UA4_LBGD_L, BAUD_LOW_BYTE(BAUD_9600));
  __outb(UA4_LBGD_H, BAUD_HIGH_BYTE(BAUD_9600));

  // Sets character format in bank 0
  __outb(UA4_LCR, (UA4_LCR_BANK0 | UA4_LCR_BITS_8 | UA4_LCR_NO_PARITY |
                   UA4_LCR_1_STOP_BIT));

  // Sends Interrupt Signal Enable, Data Terminal Ready, and Ready to Send to
  // Model Control Register.
  __outb(UA4_MCR, (UA4_MCR_ISEN | UA4_MCR_DTR | UA4_MCR_RTS));
}

int main( void ) {
  // Initialize terminal.
  init();

  // Install ISR for clock cycle and serial port.
  __install_isr(INT_VEC_TIMER, clock_isr);
  __install_isr(INT_VEC_SERIAL_PORT_1, serial_isr);

  // Enable terminal.
  __outb(UA4_IER, 0xf);

  // Clear the screen.
  serial_putchar(0x1a);

  // Enable interrupts.
  __asm("sti");

  // Print basic information to the terminal.
  char seed[12];
  serial_puts("Team:\nLiam Morris\n");
  serial_puts("Clock is ticking at the default rate\n");
  serial_puts("Enter twelve-character seed: ");

  // Read in seed from terminal.
  for (int i = 0; i < 12; ++i) {
    newchar = 0;
    c_puts("Waiting for new char\n");
    while (!newchar);
    c_printf("Got new char: %c\n", c);
    // If new line is entered, send BEL to terminal.
    if (c == 10 || c == 15) {
      --i;
      serial_putchar(7);
    }
    // Otherwise, store character in buffer.
    else {
      seed[i] = c;
      serial_putchar(c);
    }
  }
  
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
    serial_puts("\nPress the enter key when you are ready\n");
    c = 0;

    // Wait for enter key.
    while (c != 10 && c != 15);

    // Get random character [a-z], print it to the terminal, and reset tick
    // counter to 0.
    int ch = Random() * 25 + 'a';
    serial_putchar(ch);
    tick_count = 0;

    // Wait for correct character to be entered.
    // Each time an incorrect character is entered, send BEL to terminal.
    while (c != ch) {
      serial_putchar(7);
      newchar = 0;
      while (!newchar);
    }

    // Store current tick count in a variable (just in case it increments again
    // before we're done using it!), then add it to total ticks
    int ticks = tick_count;
    total_clocks += ticks;
    double average = (double) total_clocks / (double) num_rounds;
    c_printf("\nTicks: %d Total Clocks: %d Num Rounds: %d",
              ticks, total_clocks, num_rounds);

    // Print timing statistics.
    serial_puts("\nCurrent:  ");
    serial_putd(ticks);
    serial_puts("   Average:  ");
    serial_putx(average);
    serial_puts(" (");
    serial_putd(num_rounds);
    serial_puts(" rounds)");

    // Prompt for another round.
    serial_puts("\nAnother try?\n");
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
        serial_putchar(7);
      }
    }
    // If user enters 'n', exit program.
    if (c == 'n') {
      break;
    }
  }
  return (0);
}
