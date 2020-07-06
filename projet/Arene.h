#include "Graphics.h"
#include <ncurses.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <time.h>

void init_arene(Robot r1,Robot r2,Robot r3,Robot r4);

void maj_robot(Robot r,Position end,Robot r1,Robot r2,Robot r3,Robot r4);

void end_screen();

void maj_missile(Robot r1,Robot r2,Robot r3,Robot r4,Missile m);
