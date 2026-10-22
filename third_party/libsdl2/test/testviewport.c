/*
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/
/* Simple program:  Check viewports */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include "SDL_test.h"
#include "SDL_test_common.h"


static SDLTest_CommonState *state;

SDL_Rect viewport;
int done, j;
SDL_bool use_target = SDL_FALSE;
#ifdef __EMSCRIPTEN__
Uint32 wait_start;
#endif

/* Call this instead of exit(), so we can clean up SDL: atexit() is evil. */
static void
quit(int rc)
{
    SDLTest_CommonQuit(state);
    exit(rc);
}

void
DrawOnViewport(SDL_Renderer * renderer, SDL_Rect viewport)
{    
    SDL_Rect rect;

    /* Set the viewport */
    SDL_RenderSetViewport(renderer, &viewport);
    
    /* Draw a gray background */
    SDL_SetRenderDrawColor(renderer, 0x80, 0x80, 0x80, 0xFF);
    SDL_RenderClear(renderer);

    /* Test inside points */
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xF, 0xFF);
    SDL_RenderDrawPoint(renderer, viewport.h/2 + 10, viewport.w/2);
    SDL_RenderDrawPoint(renderer, viewport.h/2 - 10, viewport.w/2);
    SDL_RenderDrawPoint(renderer, viewport.h/2     , viewport.w/2 - 10);
    SDL_RenderDrawPoint(renderer, viewport.h/2     , viewport.w/2 + 10);

    /* Test horizontal and vertical lines */
    SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
    SDL_RenderDrawLine(renderer, 1, 0, viewport.w-2, 0);
    SDL_RenderDrawLine(renderer, 1, viewport.h-1, viewport.w-2, viewport.h-1);
    SDL_RenderDrawLine(renderer, 0, 1, 0, viewport.h-2);
    SDL_RenderDrawLine(renderer, viewport.w-1, 1, viewport.w-1, viewport.h-2);

    /* Test diagonal lines */
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0xFF);
    SDL_RenderDrawLine(renderer, 0, 0,
                       viewport.w-1, viewport.h-1);
    SDL_RenderDrawLine(renderer, viewport.w-1, 0,
                       0, viewport.h-1);                      

    /* Test outside points */
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xF, 0xFF);
    SDL_RenderDrawPoint(renderer, viewport.h/2 + viewport.h, viewport.w/2);
    SDL_RenderDrawPoint(renderer, viewport.h/2 - viewport.h, viewport.w/2);
    SDL_RenderDrawPoint(renderer, viewport.h/2     , viewport.w/2 - viewport.w);
    SDL_RenderDrawPoint(renderer, viewport.h/2     , viewport.w/2 + viewport.w);

    /* Add a box at the top */
    rect.w = 8;
    rect.h = 8;
    rect.x = (viewport.w - rect.w) / 2;
    rect.y = 0;
    SDL_RenderFillRect(renderer, &rect);
}

void
loop()
{
#ifdef __EMSCRIPTEN__
    /* Avoid using delays */
    if(SDL_GetTicks() - wait_start < 1000)
        return;
    wait_start = SDL_GetTicks();
#endif
    SDL_Event event;
    int i;
    /* Check for events */
    while (SDL_PollEvent(&event)) {
        SDLTest_CommonEvent(state, &event, &done);
    }

    /* Move a viewport box in steps around the screen */
    viewport.x = j * 100;
    viewport.y = viewport.x;
    viewport.w = 100 + j * 50;
    viewport.h = 100 + j * 50;
    j = (j + 1) % 4;
    SDL_Log("Current Viewport x=%i y=%i w=%i h=%i", viewport.x, viewport.y, viewport.w, viewport.h);

    for (i = 0; i < state->num_windows; ++i) {
        if (state->windows[i] == NULL)
            continue;

        /* Draw using viewport */
        DrawOnViewport(state->renderers[i], viewport);

        /* Update the screen! */
        if (use_target) {
            SDL_SetRenderTarget(state->renderers[i], NULL);
            SDL_RenderCopy(state->renderers[i], state->targets[i], NULL, NULL);
            SDL_RenderPresent(state->renderers[i]);
            SDL_SetRenderTarget(state->renderers[i], state->targets[i]);
        } else {
            SDL_RenderPresent(state->renderers[i]);
        }
    }

#ifdef __EMSCRIPTEN__
    if (done) {
        emscripten_cancel_main_loop();
    }
#endif
}

int
main(int argc, char *argv[])
{
    int i;
    Uint32 then, now, frames;

    /* Initialize test framework */
    state = SDLTest_CommonCreateState(argv, SDL_INIT_VIDEO);
    if (!state) {
        return 1;
    }

    for (i = 1; i < argc;) {
        int consumed;

        consumed = SDLTest_CommonArg(state, i);
        if (consumed == 0) {
            consumed = -1;
            if (SDL_strcasecmp(argv[i], "--target") == 0) {
                use_target = SDL_TRUE;
                consumed = 1;
            }
        }
        if (consumed < 0) {
            SDL_Log("Usage: %s %s [--target]\n",
                    argv[0], SDLTest_CommonUsage(state));
            quit(1);
        }
        i += consumed;
    }
    if (!SDLTest_CommonInit(state)) {
        quit(2);
    }

    if (use_target) {
        int w, h;

        for (i = 0; i < state->num_windows; ++i) {
            SDL_GetWindowSize(state->windows[i], &w, &h);
            state->targets[i] = SDL_CreateTexture(state->renderers[i], SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
            SDL_SetRenderTarget(state->renderers[i], state->targets[i]);
        }
    }

    for (i = 0; i < state->num_windows; ++i) {
        SDL_Renderer *renderer = state->renderers[i];
        SDL_SetRenderDrawColor(renderer, 0xA0, 0xA0, 0xA0, 0xFF);
        SDL_RenderClear(renderer);
    }

    /* Main render loop */
    frames = 0;
    then = SDL_GetTicks();
    done = 0;
    j = 0;

#ifdef __EMSCRIPTEN__
    wait_start = SDL_GetTicks();
    emscripten_set_main_loop(loop, 0, 1);
#else
    while (!done) {
        ++frames;
        loop();
        SDL_Delay(1000);
    }
#endif

    /* Print out some timing information */
    now = SDL_GetTicks();
    if (now > then) {
        double fps = ((double) frames * 1000) / (now - then);
        SDL_Log("%2.2f frames per second\n", fps);
    }
    quit(0);
    return 0;
}

/* vi: set ts=4 sw=4 expandtab: */
