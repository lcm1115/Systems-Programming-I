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

int main( void ) {
        char seed[13];
	c_puts( "Hello, world!\n" );

        c_puts("Team:\nLiam Morris\n");
        c_puts("Enter twelve-character seed: ");
        c_gets(seed, 12);
        
        unsigned int vals[3];

        for (int i = 0; i < 3; ++i) {
          vals[i] = 0;
          for (int j = 0; j < 4; j++) {
            vals[i] |= seed[i * 4 + j] << (8 * j);
          }
        }

        unsigned int sd = vals[0] ^ vals[1] ^ vals[2];

        PutSeed(sd);

	return( 0 );
}
