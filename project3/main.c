/*
** SCCS ID:	@(#)main.c	1.3	03/15/05
**
** File:	main.c
**
** Author:	K. Reek
**
** Contributor:	Warren R. Carithers
**
** Description:	Dummy main program
*/
#include "c_io.h"
#include "rngs.h"
#include "startup.h"
#include "support.h"
#include "uart.h"
#include "x86arch.h"

int newchar = 0;
int tick_count = 0;
int num_rounds;
int nums[256];
char c = 0;

void clock_isr(int vector, int code) {
  ++tick_count;
  __outb(PIC_MASTER_CMD_PORT, PIC_EOI);
}

void serial_putchar(char ch) {
  if (ch == '\n') {
    serial_putchar('\r');
  }

  while ((__inb(UA4_LSR) & UA4_LSR_TXRDY) == 0);

  __outb(UA4_TXD, ch);
}

void serial_puts(char* buffer) {
  char ch;
  while ((ch = *buffer++) != '\0') {
    serial_putchar(ch);
  }
}

int serial_getchar(void) {
  int ch;

  ch = __inb(UA4_RXD) & 0x7f;
  if (ch == '\r') {
    ch = '\n';
  }
  c = ch;
  return ch;
}

void serial_putd(int d) {
  int i = 0;
  while (d > 0) {
    int n = d % 10;
    d /= 10;
    nums[i] = n;
    ++i;
  }
  --i;
  while (i >= 0) {
    serial_putchar('0' + nums[i]);
    --i;
  }
}

void serial_putx(double x) {
  int d = (int) x;
  serial_putd(d);
  serial_putchar('.');
  int r = (int) (x * 10) % 10;
  serial_putd(r);
}

void serial_isr(int vector, int code) {
  int br = 0;
  while (!br) {
    switch (__inb(UA4_EIR) & UA4_EIR_INT_PRI_MASK) {
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
  __outb(PIC_MASTER_CMD_PORT, PIC_EOI);
}

void init(void) {
  __outb(UA4_FCR, 0);
  __outb(UA4_FCR, UA5_FCR_FIFO_EN);
  __outb(UA4_FCR, UA5_FCR_FIFO_EN | UA5_FCR_RXSR);
  __outb(UA4_FCR, UA5_FCR_FIFO_EN | UA5_FCR_RXSR | UA5_FCR_TXSR);

  __outb(UA4_IER, 0);

  __outb(UA4_LCR, UA4_LCR_BANK1);
  __outb(UA4_LBGD_L, BAUD_LOW_BYTE(BAUD_9600));
  __outb(UA4_LBGD_H, BAUD_HIGH_BYTE(BAUD_9600));

  __outb(UA4_LCR, (UA4_LCR_BANK0 | UA4_LCR_BITS_8 | UA4_LCR_NO_PARITY |
                   UA4_LCR_1_STOP_BIT));

  __outb(UA4_MCR, (UA4_MCR_ISEN | UA4_MCR_DTR | UA4_MCR_RTS));
}

int main( void ) {
  init();
  __install_isr(INT_VEC_TIMER, clock_isr);
  __install_isr(INT_VEC_SERIAL_PORT_1, serial_isr);
  __outb(UA4_IER, 0xf);
  serial_putchar(0x1a);
  __asm("sti");
  char seed[12];
  serial_puts("Team:\nLiam Morris\n");
  serial_puts("Clock is ticking at the default rate\n");
  serial_puts("Enter twelve-character seed: ");
  for (int i = 0; i < 12; ++i) {
    newchar = 0;
    c_puts("Waiting for new char\n");
    while (!newchar);
    c_printf("Got new char: %c\n", c);
    if (c == 10 || c == 15) {
      --i;
      serial_putchar(7);
    } else {
      seed[i] = c;
      serial_putchar(c);
    }
  }
  
  unsigned int vals[3];

  for (int i = 0; i < 3; ++i) {
    vals[i] = 0;
    for (int j = 0; j < 4; j++) {
      vals[i] |= seed[i * 4 + j] << (8 * j);
    }
  }

  unsigned int sd = vals[0] ^ vals[1] ^ vals[2];

  PutSeed(sd);
  unsigned int total_clocks = 0;
  while (1) {
    ++num_rounds;
    serial_puts("\nPress the enter key when you are ready\n");
    c = 0;
    while (c != 10 && c != 15);
    int ch = Random() * 25 + 'a';
    serial_putchar(ch);
    tick_count = 0;
    while (c != ch) {
      serial_putchar(7);
      newchar = 0;
      while (!newchar);
    }
    int ticks = tick_count;
    total_clocks += ticks;
    double average = (double) total_clocks / (double) num_rounds;
    c_printf("\nTicks: %d Total Clocks: %d Num Rounds: %d",
              ticks, total_clocks, num_rounds);
    serial_puts("\nCurrent:  ");
    serial_putd(ticks);
    serial_puts("   Average:  ");
    serial_putx(average);
    serial_puts(" (");
    serial_putd(num_rounds);
    serial_puts(" rounds)");
    serial_puts("\nAnother try?\n");
    c = 0;
    while (1) {
      newchar = 0;
      while (!newchar);
      if (c == 'y' || c == 'n') {
        break;
      } else {
        serial_putchar(7);
      }
    }
    if (c == 'n') {
      break;
    }
  }
  return (0);
}
