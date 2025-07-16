#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "videoram.h"
#include "characters.h"

#ifndef NOASM
#define NAKED __naked
#else
#define NAKED
#endif

// Look up table: transforms 4 bits to 8 bits by duplicating each bit.
// Ex.: 1001 -> 11000011
static unsigned char double_bits_full[16] = {
    0, 3, 12, 15, 48, 51, 60, 63, 192, 195, 204, 207, 240, 243, 252, 255
};

// Look up table: transforms 4 bits to 8 bits by inserting a bit 0 between each
// bit.
// Ex.: 1111 -> 10101010
static unsigned char double_bits_half[16] = {
    0, 2, 8, 10, 32, 34, 40, 42, 128, 130, 136, 138, 160, 162, 168, 170
};

// The video structure contains global variables for this library.
static struct {
    unsigned int *roller;       // 0: Roller RAM address
    unsigned int *line_starts;  // 2: Line start offsets
    unsigned char *screen;      // 4: Screen memory address
    unsigned char row;          // 6: Row where the next character will be
                                //    printed
    unsigned char col;          // 7: Column where the next character will be
                                //    printed
    unsigned char *address;     // 8: Screen address where the next character
                                //    will be printed
    unsigned char *font;        // 10: Address of the character font
    unsigned char font_size;    // 12: Size at which the next character will be
                                //     printed
    unsigned char *brightness;  // 13: Brightness for double width character
} video = { 0 };

// Function type for a print function
typedef void PRINT_FUNCTION(const unsigned char *string);

// Declare the four print functions
void print_normal_size(const unsigned char *string);
void print_double_width(const unsigned char *string);
void print_double_height(const unsigned char *string);
void print_double_size(const unsigned char *string);

// Pointers to the print functions
PRINT_FUNCTION *prints[] = {
    print_normal_size,
    print_double_width,
    print_double_height,
    print_double_size
};

// Set the character size.
// The available values are SIZE_NORMAL, SIZE_DOUBLE_WIDTH, SIZE_DOUBLE_HEIGHT,
// and SIZE_DOUBLE.
void set_size(unsigned char size) {
    video.font_size = size;
}

// Set brightness for double width characters.
// The available values are BRIGHTNESS_FULL and BRIGHTNESS_HALF.
void set_brightness(unsigned char brightness) {
    if(brightness == BRIGHTNESS_HALF) {
        video.brightness = double_bits_half;
    } else {
        video.brightness = double_bits_full;
    }
}

char roller_ram[ROLLER_SIZE + 3 * 512 + SCREEN_SIZE];

void alloc_screen_memory(void) {
    // The roller RAM address must be a multiple of 512.
    video.roller = (unsigned int *) (((size_t) (roller_ram + ROLLER_SIZE)) & 0xFE00);
    // Beware: these are int (16-bit) arrays, so it's 256 now (and not 512)
    video.line_starts = (unsigned int *) video.roller + 256;

    // Video memory directly follows the roller RAM.
    video.screen = (unsigned char *)(video.line_starts + 256);
}

// Initialize the roller RAM to point at our own screen memory
void init_roller_ram(void) {
    unsigned char line;
    unsigned char row;
    unsigned int index;
    unsigned int address;
    unsigned int inbank;

    // The roller RAM has one entry for each screen line
    index = 0;
    address = (size_t) video.screen;

    // There are 32 rows on a PCW screen
    for(row = 0; row < 32; row++) {
        // Each row groups 8 screen lines
        for(line = 0; line < 8; line++) {
            // Determines which RAM bank will hold the line
            inbank = address & (BANK_SIZE - 1);

            video.line_starts[index] = address;
            video.roller[index++] = (((address >> 14) + BASE_BANK) << 13)
                                  + ((inbank >> 1) & 0xFFF8)
                                  + (inbank & 7);

            address++;
        }

        address += 720-8;
    }
}

/*
 * SDCC allows to treat I/O ports as variables and generates IN/OUT instructions
 * when assigning or reading to this variable
 */
__sfr __at(SET_ROLLER_ADDRESS) roller_port;

// Sets the roller RAM address
void set_roller_ram_address(void) {
    unsigned int bank;
    unsigned int address;

    // Determines which RAM bank will hold the roller RAM
    bank = BASE_BANK + ((size_t) video.roller >> 14);

    // Determines the address of the roller RAM in this bank
    address = (unsigned int)video.roller & (BANK_SIZE - 1);

    roller_port = (bank * 32) + (address >> 9);

    //outp(SET_ROLLER_ADDRESS, (bank * 32) + (address >> 9));
}

// Change back the roller RAM to its standard address
void restore_video_ram(void) {
    //outp(SET_ROLLER_ADDRESS, 0x5B);
    roller_port = 0x5b;
}

// Clear the screen
void clear_screen(void) {
    memset(video.screen, 0, SCREEN_SIZE);
}

// Sets the position of the next character to be printed.
// col=[0..89], row=[0..31]
void locate(unsigned char col, unsigned char row) {
    video.row = row;
    video.col = col;
    video.address = video.screen + row * 720 + col * 8;
}

// Update the cursor position after each printed character.
void advance_cursor(void) NAKED {
#ifdef NOASM
    if(video.font_size & 1) {
        video.col += 2;
        video.address += 16;
    } else {
        video.col++;
        video.address += 8;
    }

    if(video.col < 90) return;

    video.col = 0;

    if(video.font_size & 2) {
        video.row += 2;
        video.address += 720;
    } else {
        video.row++;
    }

    if(video.row < 32) return;

    video.row = 0;
    video.address = video.screen;
#else
/*
    unsigned int *roller;       // 0: Roller RAM address
    unsigned int *line_starts;  // 2: Line start offsets
    unsigned char *screen;      // 4: Screen memory address
    unsigned char row;          // 6: Row where the next character will be
                                //    printed
    unsigned char col;          // 7: Column where the next character will be
                                //    printed
    unsigned char *address;     // 8: Screen address where the next character
                                //    will be printed
    unsigned char *font;        // 10: Address of the character font
    unsigned char font_size;    // 12: Size at which the next character will be
                                //     printed
    unsigned char *brightness;  // 13: Brightness for double width character
*/
__asm
    ; if(video.font_size & 1) {
    ld a, (_video+12)
	rrca
	jp	nc, ac_simple_width

.ac_double_width:
    ; video.col += 2;
	ld	a,(_video+7)
    add #2
    ld (_video+7), a

    ; video.address += 16;
    ld hl, (_video+8)
    ex de, hl
    ld hl, #16
    add hl, de
    ld (_video+8), hl
	jp	same_line

ac_simple_width:
    ; video.col++;
	ld	hl,#_video+7
	inc	(hl)

    ; video.address += 8;
	ld	hl, (_video+8)
    ex de, hl
    ld hl, #8
    add hl, de
    ld (_video+8), hl

same_line:
    ; if(video.col < 90) return;
    ld a, (_video+7)
    cp #90
    ret c

.next_line:
    ; video.col = 0;
    xor a
    ld (_video+7), a

    ; if(video.font_size & 2) {
	ld a, (_video+12)
	and	#2
	jp z, simple_height

.double_height:
    ; video.row += 2;
    ld a, (#_video+6)
    add #2
    ld (#_video+6), a

    ; video.address += 720;
    ld hl, (#_video+8)
    ld de, #720
    add hl, de
    ld (#_video+8), hl
	jp	same_page

simple_height:
    ; video.row++;
	ld	hl, #_video+6
	inc	(hl)

same_page:
    ; if(video.row < 32) return;
	ld	a,(#_video+6)
    cp #32
    ret c
.next_page:
    ; video.row = 0;
    xor a
    ld (#_video+6), a

    ; video.address = video.screen;
	ld	de, #_video+8
	ld	hl, #_video+4
    ldi
    ldi
	ret

__endasm;
#endif /* NOASM */
}

// Main print function which uses the dedicated print function given the current
// settings.
void print(const unsigned char *string) {
    prints[video.font_size](string);
}

// Print normal size characters. This uses memcpy in order to draw characters
// at the maximum speed.
void print_normal_size(const unsigned char *string) NAKED {
#ifdef NOASM

    for(; *string != '\0'; string++) {
        memcpy(video.address, &video.font[*string * 8], 8);
        advance_cursor();
    }

#else

    string;

__asm
    ; IX = stack frame
    ; string +4
    ; i -1, character_drawing -3, left -4, right -5, offset -7
    push ix
    ld ix, #0
    add ix, sp

    ld c, 4(ix)
    ld b, 5(ix)

.forloop_pns:
    ; for(; *string != '\0'; string++) {
    ld a, (bc)
    or a
    jp z, endloop_pns

    ; &video.font[*string * 8]
    ld l, a
    ld h, #0
    add hl, hl
    add hl, hl
    add hl, hl
    ld de, (_video+10)
    add hl, de

    push bc
    ld de, (_video+8)
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi

    call _advance_cursor
    pop bc

    inc bc
    jp .forloop_pns

endloop_pns:
    pop ix
    ret
__endasm;
#endif /* NOASM */
}

// Print double width characters.
void print_double_width(const unsigned char *string) {
    unsigned char i;
    unsigned char *character_drawing;

    for(; *string != '\0'; string++) {
        character_drawing = &video.font[*string * 8];
        for(i = 0; i != 8; i++) {
            // Left part
            video.address[i] = video.brightness[*character_drawing >> 4];

            // Right part
            video.address[i+8] = video.brightness[*character_drawing & 15];

            character_drawing++;
        }
        advance_cursor();
    }
}


// Look up table to accelerate character drawing when using double height.
const unsigned int dh_offset[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 720, 721, 722, 723, 724, 725, 726, 727
};

// Print double height characters.
void print_double_height(const unsigned char *string) {
    unsigned char i;
    unsigned char *character_drawing;

    for(; *string != '\0'; string++) {
        character_drawing = &video.font[*string * 8];
        for(i = 0; i != 16; i+= 2) {
            // The same line is printed twice
            video.address[dh_offset[i]] = *character_drawing;
            video.address[dh_offset[i+1]] = *character_drawing;

            character_drawing++;
        }

        advance_cursor();
    }
}

// Print double size characters.
void print_double_size(const unsigned char *string) NAKED {
#ifdef NOASM
    unsigned char i;
    unsigned char *character_drawing;
    unsigned char left;
    unsigned char right;
    unsigned int offset;

    for(; *string != '\0'; string++) {
        character_drawing = &video.font[*string * 8];
        for(i = 0; i != 16; i+= 2) {
            left = video.brightness[*character_drawing >> 4];
            right = video.brightness[*character_drawing & 15];
            offset = dh_offset[i];

            // Upper left part
            video.address[offset] = left;

            // Upper right part
            video.address[offset+8] = right;

            // Bottom left part
            video.address[offset+1] = left;

            // Bottom right part
            video.address[offset+9] = right;

            character_drawing++;
        }

        advance_cursor();
    }
#else
    string;

__asm
    ; IX = stack frame
    ; string +4
    ; i -1, character_drawing -3, left -4, right -5, offset -7
    push ix
    ld ix, #0
    add ix, sp

    push bc
    push bc
    push bc
    dec sp

    ld l, 4(ix)
    ld h, 5(ix)

forloop_pds:
    ; for(; *string != '\0'; string++) {
    ld a,(hl)
    or a
    jp z, endloop_pds

    inc hl
    push hl

    ; *string * 8
    ld l, a
    ld h, #0
    add hl, hl
    add hl, hl
    add hl, hl

    ; character_drawing = &video.font[*string * 8];
    ex de, hl
    ld hl, (_video+10)
    add hl, de
    ex de, hl ; de = &video.font[*string * 8]

    ld a, #0
    ; for(i = 0; i != 16; i+= 2) {
forloop_pds_i:
    cp #16
    jp c, forloop_pds_i_inside
    push de
    call _advance_cursor
    pop de
    pop hl
    jp forloop_pds

forloop_pds_i_inside:
    push af

    ; offset = dh_offset[i];
    ld hl, #_dh_offset
    add a
    ld c, a
    ld b, #0
    add hl, bc
    ld c, (hl)
    inc hl
    ld b, (hl)
    push bc
    pop iy
    ld bc, (#_video+8)
    add iy, bc ; iy = &video.address[offset]

    ; *character_drawing >> 4
    ld a, (de)
    rra
    rra
    rra
    rra
    and #0x0f

    ; left = video.brightness[*character_drawing >> 4];
    ld hl, (#_video+13)
    add l
    ld l, a
    jr nc, .+3
    inc h
    ld a, (hl)

    ; video.address[offset] = left; // a = left, iy = offset
    ; video.address[offset+1] = left;
    ld 0(iy), a
    ld 1(iy), a

    ; *character_drawing & 0x0f
    ld a, (de)
    and #0x0f

    ; right = video.brightness[*character_drawing & 15];
    ld hl, (_video+13)
    add l
    ld l, a
    jr nc, .+3
    inc h
    ld a, (hl)

    ; video.address[offset+8] = right; // a = right
    ; video.address[offset+9] = right;
    ld 8(iy), a
    ld 9(iy), a

    pop af
    add a, #2
    inc de
    jp forloop_pds_i

endloop_pds:
    inc sp
    pop bc
    pop bc
    pop bc
    pop ix
    ret
__endasm;
#endif /* NOASM */
}

// Defines which font to use when printing characters on the screen.
void set_font(unsigned char *font) {
    video.font = font;
}

void set_pixel(int x, int y)
{
    unsigned char vertical_masks[] = { 128, 64, 32, 16, 8, 4, 2, 1 };
    unsigned char *address;
    unsigned char mask;
    int offset;
    unsigned int *line_start;

    mask = vertical_masks[(unsigned char) x & 7];

    offset = x & 0xfff8;

    line_start = &video.line_starts[y];

    address = (unsigned char *)((size_t) *line_start) + offset;
    *address |= mask;
}

int sign(int x)
{
    return x > 0 ? +1 : x < 0 ? -1 : 0;
}

/*
 * Bresenham's circle algorithm
 */

static void circle_points(int xc, int yc, int x, int y)
{
    set_pixel(xc + x, yc + y);
    set_pixel(xc - x, yc + y);
    set_pixel(xc + x, yc - y);
    set_pixel(xc - x, yc - y);
    set_pixel(xc + y, yc + x);
    set_pixel(xc - y, yc - x);
    set_pixel(xc + y, yc - x);
    set_pixel(xc - y, yc - x);
}

void circle(int xc, int yc, int r)
{
    int x = 0, y = r;
    int d = 3 - 2 * r;
    circle_points(xc, yc, x, y);
    while (y >= x)
    {
        /* check for decision parameter and correspondingly update d, y */
        if (d > 0)
        {
            y--;
            d = d + 4 * (x - y) + 10;
        }
        else
            d = d + 4 * x + 6;
        
        /* increment x after updating decision parameter */
        x++;
        
        /* draw the circle using the new coordinates */
        circle_points(xc, yc, x, y);
    }
}
/*
 * Bresenham-algorithm: draw line
 *
 * input:
 *    xstart, ystart        coordinates of starting point
 *    xend, yend            coordinates of end point
 */
void line(int xstart, int ystart, int xend, int yend)
{
    int x, y, t, dx, dy;
    int incx, incy, pdx, pdy, ddx, ddy;
    int deltaslowdirection, deltafastdirection, err;

    /* calculate distance in both directions */
    dx = xend - xstart;
    dy = yend - ystart;

    /* determine sign of increment */
    incx = sign(dx);
    incy = sign(dy);
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;

    /* determine larger distance */
    if (dx > dy)
    {
        /* x is faster distance */
        pdx = incx; pdy = 0;    /* pd. ist Parallelschritt */
        ddx = incx; ddy = incy; /* dd. ist Diagonalschritt */
        deltaslowdirection = dy;   deltafastdirection = dx;
    }
    else
    {
        /* y is faster distance */
        pdx = 0;    pdy = incy; /* pd. ist Parallelschritt */
        ddx = incx; ddy = incy; /* dd. ist Diagonalschritt */
        deltaslowdirection = dx;   deltafastdirection = dy;
    }

    /* Initialisierungen vor Schleifenbeginn */
    x = xstart;
    y = ystart;
    err = deltafastdirection / 2;
    set_pixel(x, y);

    /* Pixel berechnen */
    for (t = 0; t < deltafastdirection; ++t) /* t zählt die Pixel, deltafastdirection ist Anzahl der Schritte */
    {
        /* Aktualisierung Fehlerterm */
        err -= deltaslowdirection;
        if (err < 0)
        {
            /* Fehlerterm wieder positiv (>= 0) machen */
            err += deltafastdirection;
            /* Schritt in langsame Richtung, Diagonalschritt */
            x += ddx;
            y += ddy;
        }
        else
        {
            /* Schritt in schnelle Richtung, Parallelschritt */
            x += pdx;
            y += pdy;
        }
        set_pixel(x, y);
    }
}

void vertical_line(unsigned int x, unsigned char y1, unsigned char y2) NAKED {
    unsigned char mask;
    unsigned int y;
    unsigned int offset;
    unsigned char *address;
    unsigned int *line_start;
    unsigned char vertical_masks[] = { 128, 64, 32, 16, 8, 4, 2, 1 };

#ifdef NOASM

    mask = vertical_masks[(unsigned char) x & 7];
    offset = x & 0xfff8;

    line_start = &video.line_starts[y1];
    for(y = y1; y < y2 + 1; y++) {
        address = (unsigned char *)((size_t) *line_start) + offset;
        *address |= mask;
        line_start++;
    }
#else
    x;
    y1;
    y2;
__asm
    ; x +8, y1 +6, y2 +4
    ; mask -1, y -2, offset -4, line_start -6
    ; IX = stack frame
    push ix
    ld ix, #0
    add ix, sp

    ; Reserve 6 bytes on the stack
    push bc
    push bc
    push bc

    ; mask = vertical_masks[(unsigned char)x & 7];
    ld a, 8(ix) ; x
    and #7
    ld e, a
    ld d, #0
    ld hl, #_vertical_masks
    add hl,de
    ld a,(hl)

    ld -1(ix), a ; mask

    ; offset = x & 0xfff8;
    ld a, 8(ix) ; x
    and #0xf8
    ld e, a
    ld d, 9(ix)

    ld -4(ix), e ; offset
    ld -3(ix), d

    ; line_start = &video.line_starts[y1];
    ld e, 6(ix) ; y1
    ld d, #0

    ld    hl, (_video+1+1)
    add hl, de
    add hl, de

    ex (sp), hl

    ; for(y = y1; y != y2 + 1; y++) {
    ld a, 4(ix) ; y2
    sub a, 6(ix) ; y1

    or a
    jp z,endfor

    pop de
    push de

.forloop:
    ld l, e
    ld h, d

    ; address = (unsigned char *)(*line_start) + offset;
    ld c,(hl)
    inc hl
    ld b,(hl) ; bc = *line_start

    ld l, -4(ix) ; hl=offset
    ld h, -3(ix)

    add hl, bc ; hl = *line_start + offset

    ; *address |= mask;
    ld c, -1(ix) ; mask

    ld b, a
    ld a, (hl)
    or c
    ld (hl), a
    ld a, b

    ; line_start++;
    inc de
    inc de

    dec a
    jp nz, .forloop

endfor:

    pop bc
    pop bc
    pop bc
    pop ix
    ret
__endasm;
#endif /* NOASM */
}

unsigned char horz_start_masks[] = { 255, 127, 63, 31, 15, 7, 3, 1 };
unsigned char horz_end_masks[] = { 128, 192, 224, 240, 248, 252, 254, 255 };

void horizontal_line(unsigned int x1, unsigned int x2, unsigned char y) {
    unsigned char start_mask;
    unsigned char end_mask;
    unsigned char pixel_count;
    unsigned char i;
    unsigned char *address;

    start_mask = horz_start_masks[(unsigned char)x1 & 7];
    end_mask = horz_end_masks[(unsigned char)x2 & 7];

    address = (unsigned char *)((size_t) video.line_starts[y]) + (x1 & 0xfff8);
    if ((x1 & 0xfff8) == (x2 & 0xfff8)) {
        *address |= start_mask & end_mask;
        return;
    }

    *address |= start_mask;
    address += 8;

    pixel_count = (((x2 & 0xfff8) - (x1 & 0xfff8)) >> 3) - 1;
    for(i = 0; i != pixel_count; i++) {
        *address = 255;
        address += 8;
    }

    *address |= end_mask;
}

void frame(unsigned int tx, unsigned char ty, unsigned int bx, unsigned char by) {
    vertical_line(tx, ty, by);
    vertical_line(bx, ty, by);
    horizontal_line(tx, bx, ty);
    horizontal_line(tx, bx, by);
}

// Initializes everything!
void init_video_ram(void) {
    alloc_screen_memory();
    locate(0, 0);
    set_size(SIZE_NORMAL);
    set_brightness(BRIGHTNESS_FULL);
    set_font(stdfont);
    init_roller_ram();
    clear_screen();
    set_roller_ram_address();
}
