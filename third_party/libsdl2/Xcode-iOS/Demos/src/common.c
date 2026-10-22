/*
 *  common.c
 *  written by Holmes Futrell
 *  use however you want
 */

#include "common.h"
#include "SDL.h"
#include <stdlib.h>

/*
    Produces a random int x, min <= x <= max
    following a uniform distribution
*/
int
randomInt(int min, int max)
{
    return min + rand() % (max - min + 1);
}

/*
    Produces a random float x, min <= x <= max
    following a uniform distribution
 */
float
randomFloat(float min, float max)
{
    return rand() / (float) RAND_MAX *(max - min) + min;
}

void
fatalError(const char *string)
{
    printf("%s: %s\n", string, SDL_GetError());
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, string, SDL_GetError(), NULL);
    exit(1);
}

static Uint64 prevTime = 0;

double
updateDeltaTime(void)
{
    Uint64 curTime;
    double deltaTime;

    if (prevTime == 0) {
        prevTime = SDL_GetPerformanceCounter();
    }

    curTime = SDL_GetPerformanceCounter();
    deltaTime = (double) (curTime - prevTime) / (double) SDL_GetPerformanceFrequency();
    prevTime = curTime;

    return deltaTime;
}
