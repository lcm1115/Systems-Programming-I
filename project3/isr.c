/*
** File:        isr.c
**
** Author:      Liam Morris
**
** Description: This program contains the code for the ISR for both the clock
**              and the serial receiver/transmitter. This allows for the main
**              program to interface with the serial device to read and write
**              data from and to the terminal.
*/

#include "c_io.h"
#include "startup.h"
#include "uart.h"
#include "x86arch.h"

// Current state of the transmitter interrupt.
int interrupt_state = 0;

// Output buffer for use in transmitting data.
char output_buffer[256];

// Next character to be transmitted to the terminal.
char *next_output_ch;

extern int newchar;
extern int tick_count;
extern char nums[256];
extern char c;

// Implementation of strcpy. Copies one string into another.
// Arguments:
//   src - buffer to copy data into
//   dest - buffer to copy data from
void strcpy(char* src, char* dest) {
  while (*dest != 0) {
    *src = *dest;
    ++src;
    ++dest;
  }
}

// Implementation of strcat. Concatenates two strings.
// Arguments:
//   src - buffer to concatenate data onto.
//   dest - buffer to read data from to concatenate
void strcat(char* src, char* dest) {
  int i = 0;
  char* ptr1 = src;
  char* ptr2 = dest;
  while (*ptr1 != 0) {
    ptr1++;
    i++;
  }
  while (ptr1 && ptr2 && *ptr2 != 0) {
    *ptr1 = *ptr2;
    ptr1++;
    ptr2++;
    i++;
  }
}

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
void serial_write(char* user_buffer) {
  __asm("cli");

  // Determine if currently transmitting.
  // If so, concatenate buffer onto output buffer.
  // Otherwise, copy buffer onto output buffer and write first character.
  if ((interrupt_state & UA4_IER_TX_INT_ENABLE) != 0) {
    strcat(output_buffer, user_buffer);
    __asm("sti");
  } else {
    char ch;

    __asm("sti");
    ch = *user_buffer;
    if (ch != 0) {
      strcpy(output_buffer, user_buffer + 1);
      next_output_ch = output_buffer;

      // Update state.
      interrupt_state |= UA4_IER_TX_INT_ENABLE;

      // Remove stopper! (Write first character)
      __outb(UA4_TXD, ch);
    }
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
    nums[i] = '0' + n;
    ++i;
  }
  nums[i] = 0;
  int digits = i;
  i--;

  // Swap order of digits.
  for (int j = 0; j < digits / 2; j++, i--) {
    nums[i] ^= nums[j];
    nums[j] ^= nums[i];
    nums[i] ^= nums[j];
  }
  // Iterate across array backwards and write each value to the terminal.
  serial_write(nums);
}

// Writes a double to the terminal.
// Arguments:
//   x - the double to be written
void serial_putx(double x) {
  // Get the int portion of the double and write it to the terminal.
  int d = (int) x;
  serial_putd(d);

  char* dec = ".";

  // We need a decimal point!
  serial_write(dec);
  // Take the decimal value and print it to the terminal.
  int r = (int) (x * 10) % 10;
  serial_putd(r);
}

// ISR for the terminal.
// Reads in bytes and handles all interrupts as needed.
void serial_isr(int vector, int code) {
  int br = 0;
  char nextch;
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
      // Get character from terminal, update globals accordingly.
      case UA4_EIR_RX_HIGH:
        c = serial_getchar();
        newchar = 1;
        c_printf("Received: %c\n", c);
        break;
      case UA5_EIR_RX_FIFO_TO:
        __inb(UA4_RXD);
        c_printf("ERROR: Shouldn't be in FIFO!\n");
        break;
      // Transmit data to terminal.
      case UA4_EIR_TX_LOW:
        // Get character to write.
        nextch = *next_output_ch;

        // If not null, write to terminal.
        // Otherwise, reset buffer and update interrupt state.
        if (nextch != 0) {
          next_output_ch++;
          __outb(UA4_TXD, nextch);
        } else {
          c_puts("Buffer Empty\n");
          interrupt_state &= ~UA4_IER_TX_INT_ENABLE;
          for (int i = 0; i < 256; ++i) {
            output_buffer[i] = 0;
          }
        }
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

