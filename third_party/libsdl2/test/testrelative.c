/*
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

/* Simple program:  Test relative mouse motion */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "SDL_test_common.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

static SDLTest_CommonState *state;
int i, done;
SDL_Rect rect;
SDL_Event event;

static void
DrawRects(SDL_Renderer * renderer, SDL_Rect * rect)
{
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, rect);
}

static void
loop(){
    /* Check for events */
    while (SDL_PollEvent(&event)) {
        SDLTest_CommonEvent(state, &event, &done);
        switch(event.type) {
        case SDL_MOUSEMOTION:
            {
                rect.x += event.motion.xrel;
                rect.y += event.motion.yrel;
            }
            break;
        }
    }
    for (i = 0; i < state->num_windows; ++i) {
        SDL_Rect viewport;
        SDL_Renderer *renderer = state->renderers[i];
        if (state->windows[i] == NULL)
            continue;
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(renderer);

        /* Wrap the cursor rectangle at the screen edges to keep it visible */
        SDL_RenderGetViewport(renderer, &viewport);
        if (rect.x < viewport.x) rect.x += viewport.w;
        if (rect.y < viewport.y) rect.y += viewport.h;
        if (rect.x > viewport.x + viewport.w) rect.x -= viewport.w;
        if (rect.y > viewport.y + viewport.h) rect.y -= viewport.h;

        DrawRects(renderer, &rect);

        SDL_RenderPresent(renderer);
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

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    /* Initialize test framework */
    state = SDLTest_CommonCreateState(argv, SDL_INIT_VIDEO);
    if (!state) {
        return 1;
    }
    for (i = 1; i < argc; ++i) {
        SDLTest_CommonArg(state, i);
    }
    if (!SDLTest_CommonInit(state)) {
        return 2;
    }

    /* Create the windows and initialize the renderers */
    for (i = 0; i < state->num_windows; ++i) {
        SDL_Renderer *renderer = state->renderers[i];
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColor(renderer, 0xA0, 0xA0, 0xA0, 0xFF);
        SDL_RenderClear(renderer);
    }

    srand((unsigned int)time(NULL));
    if(SDL_SetRelativeMouseMode(SDL_TRUE) < 0) {
        return 3;
    };

    rect.x = DEFAULT_WINDOW_WIDTH / 2;
    rect.y = DEFAULT_WINDOW_HEIGHT / 2;
    rect.w = 10;
    rect.h = 10;
    /* Main render loop */
    done = 0;
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loop, 0, 1);
#else
    while (!done) {
        loop();
        }
#endif
    SDLTest_CommonQuit(state);
    return 0;
}

/* vi: set ts=4 sw=4 expandtab: */
