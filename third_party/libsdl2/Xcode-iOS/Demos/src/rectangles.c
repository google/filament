/*
 *  rectangles.c
 *  written by Holmes Futrell
 *  use however you want
*/

#include "SDL.h"
#include <time.h>
#include "common.h"

void
render(SDL_Renderer *renderer)
{
    Uint8 r, g, b;
    int renderW;
    int renderH;

    SDL_RenderGetLogicalSize(renderer, &renderW, &renderH);

    /*  Come up with a random rectangle */
    SDL_Rect rect;
    rect.w = randomInt(64, 128);
    rect.h = randomInt(64, 128);
    rect.x = randomInt(0, renderW);
    rect.y = randomInt(0, renderH);

    /* Come up with a random color */
    r = randomInt(50, 255);
    g = randomInt(50, 255);
    b = randomInt(50, 255);

    /*  Fill the rectangle in the color */
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderFillRect(renderer, &rect);

    /* update screen */
    SDL_RenderPresent(renderer);
}

int
main(int argc, char *argv[])
{

    SDL_Window *window;
    SDL_Renderer *renderer;
    int done;
    SDL_Event event;
    int windowW;
    int windowH;

    /* initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fatalError("Could not initialize SDL");
    }

    /* seed random number generator */
    srand(time(NULL));

    /* create window and renderer */
    window = SDL_CreateWindow(NULL, 0, 0, 320, 480, SDL_WINDOW_ALLOW_HIGHDPI);
    if (window == 0) {
        fatalError("Could not initialize Window");
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fatalError("Could not create renderer");
    }

    SDL_GetWindowSize(window, &windowW, &windowH);
    SDL_RenderSetLogicalSize(renderer, windowW, windowH);

    /* Fill screen with black */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    /* Enter render loop, waiting for user to quit */
    done = 0;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done = 1;
            }
        }
        render(renderer);
        SDL_Delay(1);
    }

    /* shutdown SDL */
    SDL_Quit();

    return 0;
}
