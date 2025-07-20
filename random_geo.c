#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "videoram.h"
#include "bdos.h"

void spiral(void)
{
    int x = SCREEN_WIDTH / 2;
    int y = SCREEN_HEIGHT / 2;
    int x1 = 0, y1 = 0;

    while (x1 < x && y1 < y)
    {
        line(x - x1, y - y1, x + x1, y - y1);
        line(x + x1, y - y1, x + x1, y + y1);
        line(x + x1, y + y1, x - x1 - 1, y + y1);
        x1 += 1;
        y1 += 1;

    }
}

void random_lines(void)
{
    int x1, x2;
    int y1, y2;
    
    for (int i = 0; i < 200; i++)
    {
        x1 = rand() % SCREEN_WIDTH;
        y1 = rand() % SCREEN_HEIGHT;
        x2 = rand() % SCREEN_WIDTH;
        y2 = rand() % SCREEN_HEIGHT;
        
        line(x1, y1, x2, y2);
    }
}

#include <stdarg.h>

/*
 * dbg_printf() works like printf(), but prints to the CP/M punch device.
 * This allows debug prints in a graphics application without disturbing graphics.
 *
 * FIXME: Should probably go elsewhere.
 */
void dbg_printf(char *msg, ...)
{
    char out[200];
    va_list args;

    va_start(args, msg);
    vsprintf(out, msg, args);
    va_end(args);

    for (int i = 0; i < strlen(out); i++)
        bdos(4, (unsigned int) out[i]);
}

void random_circles(void)
{
    int x, y, r;
    
    for (int i = 0; i < 200; i++)
    {
        r = rand() % 200;
        x = rand() % SCREEN_WIDTH;
        y = rand() % SCREEN_HEIGHT;

        //dbg_print("ellipse(%d, %d, %d, %d);\r\n", x, y, r, r / 2);
        circle(x, y, r);
    }
}

void random_ellipses(void)
{
    int x, y, rx, ry;

    for (int i = 0; i < 200; i++)
    {
        rx = rand() % 200;
        ry = rand() % 200;
        x = rand() % SCREEN_WIDTH;
        y = rand() % SCREEN_HEIGHT;

        ellipse(x, y, rx, ry);
    }
}
