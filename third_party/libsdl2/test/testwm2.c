/*
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

#include <stdlib.h>
#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include "SDL_test_common.h"

static SDLTest_CommonState *state;
int done;

static const char *cursorNames[] = {
        "arrow",
        "ibeam",
        "wait",
        "crosshair",
        "waitarrow",
        "sizeNWSE",
        "sizeNESW",
        "sizeWE",
        "sizeNS",
        "sizeALL",
        "NO",
        "hand",
};
int system_cursor = -1;
SDL_Cursor *cursor = NULL;

/* Call this instead of exit(), so we can clean up SDL: atexit() is evil. */
static void
quit(int rc)
{
    SDLTest_CommonQuit(state);
    exit(rc);
}

void
loop()
{
    int i;
    SDL_Event event;
        /* Check for events */
        while (SDL_PollEvent(&event)) {
            SDLTest_CommonEvent(state, &event, &done);

            if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    SDL_Window *window = SDL_GetWindowFromID(event.window.windowID);
                    if (window) {
                        SDL_Log("Window %d resized to %dx%d\n",
                            event.window.windowID,
                            event.window.data1,
                            event.window.data2);
                    }
                }
                if (event.window.event == SDL_WINDOWEVENT_MOVED) {
                    SDL_Window *window = SDL_GetWindowFromID(event.window.windowID);
                    if (window) {
                        SDL_Log("Window %d moved to %d,%d (display %s)\n",
                            event.window.windowID,
                            event.window.data1,
                            event.window.data2,
                            SDL_GetDisplayName(SDL_GetWindowDisplayIndex(window)));
                    }
                }
            }
            if (event.type == SDL_KEYUP) {
                SDL_bool updateCursor = SDL_FALSE;

                if (event.key.keysym.sym == SDLK_LEFT) {
                    --system_cursor;
                    if (system_cursor < 0) {
                        system_cursor = SDL_NUM_SYSTEM_CURSORS - 1;
                    }
                    updateCursor = SDL_TRUE;
                } else if (event.key.keysym.sym == SDLK_RIGHT) {
                    ++system_cursor;
                    if (system_cursor >= SDL_NUM_SYSTEM_CURSORS) {
                        system_cursor = 0;
                    }
                    updateCursor = SDL_TRUE;
                }
                if (updateCursor) {
                    SDL_Log("Changing cursor to \"%s\"", cursorNames[system_cursor]);
                    SDL_FreeCursor(cursor);
                    cursor = SDL_CreateSystemCursor((SDL_SystemCursor)system_cursor);
                    SDL_SetCursor(cursor);
                }
            }
        }

        for (i = 0; i < state->num_windows; ++i) {
            SDL_Renderer *renderer = state->renderers[i];
            SDL_RenderClear(renderer);
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
    int i;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    SDL_assert(SDL_arraysize(cursorNames) == SDL_NUM_SYSTEM_CURSORS);

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
        }
        if (consumed < 0) {
            SDL_Log("Usage: %s %s\n", argv[0], SDLTest_CommonUsage(state));
            quit(1);
        }
        i += consumed;
    }
    if (!SDLTest_CommonInit(state)) {
        quit(2);
    }

    for (i = 0; i < state->num_windows; ++i) {
        SDL_Renderer *renderer = state->renderers[i];
        SDL_SetRenderDrawColor(renderer, 0xA0, 0xA0, 0xA0, 0xFF);
        SDL_RenderClear(renderer);
    }
 
    /* Main render loop */
    done = 0;
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loop, 0, 1);
#else
    while (!done) {
        loop();
    }
#endif
    SDL_FreeCursor(cursor);

    quit(0);
    /* keep the compiler happy ... */
    return(0);
}

/* vi: set ts=4 sw=4 expandtab: */
