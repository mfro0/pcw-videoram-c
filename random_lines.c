#include <stdlib.h>
#include <stdio.h>
#include "videoram.h"

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
    
    for (int i = 0; i < 100; i++)
    {
        x1 = rand() % SCREEN_WIDTH;
        y1 = rand() % SCREEN_HEIGHT;
        x2 = rand() % SCREEN_WIDTH;
        y2 = rand() % SCREEN_HEIGHT;
        
        line(x1, y1, x2, y2);
    }
}

void random_circles(void)
{
    int x, y, r;
    
    for (int i = 0; i < 500; i++)
    {
        r = rand() % 50;
        x = rand() % SCREEN_WIDTH;
        y = rand() % SCREEN_HEIGHT;
        
        ellipse(x, y, r, r / 2);
    }
}
