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

