#include <stdint.h>
#include "bdos.h"
#include "bios.h"

int putchar(int ch)
{
    bios_conout(ch);
    return ch;
}

unsigned char bdos(unsigned char fn, unsigned int DE) __naked
{
	fn;
	DE;

	__asm
	
	bdos = #5
	
        //
        // IX and IY are borked by CP/NET client!!! Preserve for SDCC!!!
        push    ix
        push    iy

        ld      c, a
	call	bdos				// returns w/ result in HL
  
        pop     iy
        pop     ix
  
        ret
	
	__endasm;
}
