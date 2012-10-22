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

int main( void ) {
        char seed[12];
        c_puts("Team:\nLiam Morris\n");
        c_puts("Enter twelve-character seed: ");
        for (int i = 0; i < 12; ++i) {
          unsigned int ch = c_getchar();
          if (ch == 10 || ch == 15) {
            --i;
            c_putchar(7);
          } else {
            seed[i] = ch;
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
        int total_clocks = 0;
        while (1) {
          int clocks = 0;
          c_puts("\nPress the enter key when you are ready\n");
          unsigned int input = 0;
          while (input != 10 && input != 15) {
            input = c_getchar();
            c_putchar(10);
          }
          int ch = Random() * 25 + 'a';
          c_putchar(7);
          c_putchar(ch);
          do {
            input = c_getchar();
            c_putchar(10);
          } while (input != ch);
          total_clocks += clocks;
          c_puts("Another try?\n");
          do {
            input = c_getchar();
            c_putchar(10);
            if (input != 'y' && input != 'n') {
              c_putchar(7);
            }
          } while (input != 'y' && input != 'n');
          if (input == 'n') {
            break;
          }
        }
	return( 0 );
}
