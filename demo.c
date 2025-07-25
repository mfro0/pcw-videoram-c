#include <stdio.h>
#include <stdlib.h>
#include "bdos.h"

#include "videoram.h"
#include "random_geo.h"

// Line that will receive characters
static unsigned char cline[] = "                                ";

int getchar(void)
{
    return bdos(1, 0);
}

int main(void) {
    unsigned char i;
    unsigned char j;

    init_video_ram();

    // Print the title frame
    set_size(SIZE_DOUBLE_WIDTH);
    locate(3, 0);
    print((unsigned char *) "\226\232\232\232\232\232\232\232\234");
    set_size(SIZE_DOUBLE);
    locate(3, 1);
    print((unsigned char *) "\225       \225");
    set_size(SIZE_DOUBLE_WIDTH);
    locate(3, 3);
    print((unsigned char *) "\223\232\232\232\232\232\232\232\231");

    // Print the title
    set_size(SIZE_DOUBLE_HEIGHT);
    locate(5, 1);
    print((unsigned char *) "VideoRAM demo!");

    // Print explanations in french
    set_size(SIZE_NORMAL);
    locate(0, 5);
    print((unsigned char *) "Le jeu de caract}res de");
    locate(0, 6);
    print((unsigned char *) "l'Amstrad  PCW  utilise");
    locate(0, 7);
    print((unsigned char *) "la norme ASCII  {tendue");
    locate(0, 8);
    print((unsigned char *) "@ 256  caract}res  mais");
    locate(0, 9);
    print((unsigned char *) "certains sont remplac{s");
    locate(0, 10);
    print((unsigned char *) "par    des   caract}res");
    locate(0, 11);
    print((unsigned char *) "accentu{s.");

    locate(0, 13);
    print((unsigned char *) "Le  jeu  de  caract}res");
    locate(0, 14);
    print((unsigned char *) "est  orient{ traitement");
    locate(0, 15);
    print((unsigned char *) "de texte.");

    frame(0, 5*8, 23*8, 16*8);

    // Print every character (except character 0)
    set_size(SIZE_DOUBLE);
    for(j = 0; j < 16; j++) {
        for(i = 0; i < 16; i++) {
            cline[i * 2] = (j << 4) + i;
        }

        if(cline[0] == '\0') cline[0] = ' ';

        locate(90-65, j * 2);
        print(cline);
    }

    for(i = 0; i < 16; i++) {
        vertical_line(i * 32 + 24 * 8, 0, 255);
        horizontal_line(24 * 8, 703, i * 16);
    }

    vertical_line(704, 0, 255);
    horizontal_line(24 * 8, 703, 255);

#ifdef NOT_USED
    // Draw vertical lines
    for(int x = 10; x < 710; x++) {
        vertical_line(x, 10, 245);
    }

    // Draw horizontal lines
    for(int y = 20; y < 100; y++) {
        horizontal_line(500-y, 500+y, y);
    }

#endif /* NOT_USED */

    for(int x = 1; x <= 120; x++) {
        frame(x * 2, x, 720 - x * 2, 255 - x);
    }

    getchar();
    
    clear_screen();
    random_lines();
    getchar();

    clear_screen();
    spiral();
    getchar();

    clear_screen();
    random_circles();
    getchar();

    clear_screen();
    random_ellipses();
    // Wait for a key before returning to CP/M
    getchar();
    
    // Restore standard screen settings
    restore_video_ram();

    return 0;
}
