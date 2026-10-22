/*
 *  touch.c
 *  written by Holmes Futrell
 *  use however you want
 */

#include "SDL.h"
#include <math.h>
#include "common.h"

#define BRUSH_SIZE 32           /* width and height of the brush */
#define PIXELS_PER_ITERATION 5  /* number of pixels between brush blots when forming a line */

static SDL_Texture *brush = 0;       /* texture for the brush */

/*
    draws a line from (startx, starty) to (startx + dx, starty + dy)
    this is accomplished by drawing several blots spaced PIXELS_PER_ITERATION apart
*/
void
drawLine(SDL_Renderer *renderer, float startx, float starty, float dx, float dy)
{

    float distance = sqrt(dx * dx + dy * dy);   /* length of line segment (pythagoras) */
    int iterations = distance / PIXELS_PER_ITERATION + 1;       /* number of brush sprites to draw for the line */
    float dx_prime = dx / iterations;   /* x-shift per iteration */
    float dy_prime = dy / iterations;   /* y-shift per iteration */
    SDL_Rect dstRect;           /* rect to draw brush sprite into */
    float x;
    float y;
    int i;

    dstRect.w = BRUSH_SIZE;
    dstRect.h = BRUSH_SIZE;

    /* setup x and y for the location of the first sprite */
    x = startx - BRUSH_SIZE / 2.0f;
    y = starty - BRUSH_SIZE / 2.0f;

    /* draw a series of blots to form the line */
    for (i = 0; i < iterations; i++) {
        dstRect.x = x;
        dstRect.y = y;
        /* shift x and y for next sprite location */
        x += dx_prime;
        y += dy_prime;
        /* draw brush blot */
        SDL_RenderCopy(renderer, brush, NULL, &dstRect);
    }
}

/*
    loads the brush texture
*/
void
initializeTexture(SDL_Renderer *renderer)
{
    SDL_Surface *bmp_surface;
    bmp_surface = SDL_LoadBMP("stroke.bmp");
    if (bmp_surface == NULL) {
        fatalError("could not load stroke.bmp");
    }
    brush =
        SDL_CreateTextureFromSurface(renderer, bmp_surface);
    SDL_FreeSurface(bmp_surface);
    if (brush == 0) {
        fatalError("could not create brush texture");
    }
    /* additive blending -- laying strokes on top of eachother makes them brighter */
    SDL_SetTextureBlendMode(brush, SDL_BLENDMODE_ADD);
    /* set brush color (red) */
    SDL_SetTextureColorMod(brush, 255, 100, 100);
}

int
main(int argc, char *argv[])
{

    int x, y, dx, dy;           /* mouse location          */
    Uint8 state;                /* mouse (touch) state */
    SDL_Event event;
    SDL_Window *window;         /* main window */
    SDL_Renderer *renderer;
    int done;                   /* does user want to quit? */
    int w, h;

    /* initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fatalError("Could not initialize SDL");
    }

    /* create main window and renderer */
    window = SDL_CreateWindow(NULL, 0, 0, 320, 480, SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALLOW_HIGHDPI);
    renderer = SDL_CreateRenderer(window, 0, 0);

    SDL_GetWindowSize(window, &w, &h);
    SDL_RenderSetLogicalSize(renderer, w, h);

    /* load brush texture */
    initializeTexture(renderer);

    /* fill canvass initially with all black */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    done = 0;
    while (!done && SDL_WaitEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            done = 1;
            break;
        case SDL_MOUSEMOTION:
            state = SDL_GetMouseState(&x, &y);  /* get its location */
            SDL_GetRelativeMouseState(&dx, &dy);        /* find how much the mouse moved */
            if (state & SDL_BUTTON_LMASK) {     /* is the mouse (touch) down? */
                drawLine(renderer, x - dx, y - dy, dx, dy);       /* draw line segment */
                SDL_RenderPresent(renderer);
            }
            break;
        }
    }

    /* cleanup */
    SDL_DestroyTexture(brush);
    SDL_Quit();

    return 0;
}
