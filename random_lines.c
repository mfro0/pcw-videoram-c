#include <stdlib.h>
#include "videoram.h"

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
    
    for (int i = 0; i < 1000; i++)
    {
        r = (rand() % SCREEN_HEIGHT) / 8;
        x = (rand() % SCREEN_WIDTH - 2 * r) + r;
        y = (rand() % SCREEN_HEIGHT - 2 * r) + r;
        
        circle(x, y, r);
    }
}