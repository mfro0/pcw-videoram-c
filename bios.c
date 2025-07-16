#include <stdint.h>
#include "bios.h"

void bios_wboot(void) __naked
{
    __asm
    __endasm;
}

unsigned char bios_constat(void) __naked
{
    __asm

    FUNCTION = 2                        ; BIOS console status

    push        hl
    push        de
    ld          hl,(0x1)                ; BIOS entry point - contains a jump to function(1)
    ld          de,#3 * FUNCTION - 3
    add         hl,de
    ld          (#.+4),hl
    call        0
    pop         de
    pop         hl
    ret

    __endasm;
    /* function result in A */
}

unsigned char bios_conin(void) __naked
{
    __asm
    
    FUNCTION = 3                        ; BIOS console input

    push        hl
    push        de
    ld          hl,(0x1)
    ld          de,#3 * FUNCTION - 3
    add         hl,de
    ld          (#. + 4),hl
    call        0
    pop         de
    pop         hl
    ret

    __endasm;
}


void bios_conout(int c) __naked
{
    c;

    __asm

    FUNCTION = 4
    push        hl
    push        de
    push        bc
    ld          iy,#2
    add         iy,sp
    ld          c,(iy)

    ld          hl,(0x1)
    ld          de,#3 * FUNCTION - 3
    add         hl,de
    ld          (#. + 4),hl
    call        0
    pop         bc
    pop         de
    pop         hl
    ret

    __endasm;
}

unsigned int bios(unsigned char fn) __naked
{
    fn;

    __asm

    version = 12
    arg = 6
    SELDSK = 9
    SECTRN = 16
    DEVTBL = 20
    DRVTBL = 22
    MOVE = 25
    USERF = 30

    ld          c,#version
    // call        cpm                  FIXME
    cp          #0x30
    jr          nc,0030$

    ld          hl,#00021$
    push        hl                      // set return address
    
    ld          l,arg + 0(ix)
    dec         l
    ld          e,l
    ld          h,#0
    ld          d,#0
    add         hl,hl
    add         hl,de
    ld          de,(1)
    add         hl,de
    push        hl
    ld          c,arg + 2(ix)
    ld          b,arg + 3(ix)
    ld          e,arg + 4(ix)
    ld          d,arg + 5(ix)
    ret                                 // do bios call
00021$:
    ; check for 16-bit or 8-bit result. For CP/M 2.2 the only functions which return 16-bit results are
    ; SELDSK (9) and SECTRN (16).
    ; This code is shared with CP/M 4 and that operating system has a few more calls which may return a
    ; 16-bit result.
0022$:          // exit22
    ld          c,a
    ld          a,arg + 0(ix)
    cp          #SELDSK
    jr          z,0023$
    cp          #SECTRN
    jr          z,0023$
    cp          #DEVTBL
    jr          z,0023$
    cp          #DRVTBL
    jr          z,0023$
    cp          #MOVE
    jr          z,0023$
    cp          #USERF
    jr          z,0023$
    ld          l,c
    ld          h,#0
0023$:
    jp          0024$                  // C return

0030$:
    // BIOS call for CP/M 3.x
    push        iy
    ld          iy,#biospb
    ld          e,arg + 0(ix)
    ld          0(iy),e
    ld          hl,#rutable
    ld          d,#0
    srl         e
    push        af
    add         hl,de
    pop         af
    ld          a,(hl)
    jr          nc,0031$               // skip if index was even
    rrca                                // if index was odd, move high nybble down
    rrca
    rrca
    rrca

0031$:
    push        ix
    pop         hl
    ld          de,#arg+2
    add         hl,de
    ld          e,(hl)
    inc         hl
    rra
    jr          nc,0034$
    ld          1(iy),e
    ld          e,(hl)
    inc         hl
    ld          d,(hl)
    inc         hl
0034$:
    ld          b,#3
0035$:
    inc         iy
    inc         iy
    rra
    jr          nc,0036$
    ld          0(iy),e
    ld          1(iy),d
    ld          e,(hl)
    inc         hl
    ld          d,(hl)
    inc         hl
0036$:
    djnz        0035$

    pop         iy
    ld          de,#biospb
    ld          c,#50
    // call        cpm                          // FIXME!
    jr          0021$

0024$:
    ret                         // needs to be verified!
                                //
    .area       data
    .ds         8

rutable:
    .db         0x0
    .db         0x0
    .db         0x22
    .db         0x02
    .db         0x60
    .db         0x22
    .db         0x02
    .db         0x02
    .db         0x06
    .db         0x00
    .db         0x20
    .db         0x20
    .db         0xe0
    .db         0x12
    .db         0x21
    .db         0xff
    .db         0x0f

biospb:
    .ds         8

    .area       text


    __endasm;
}
