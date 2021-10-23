/*
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

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

static SDL_Rect viewport;
static int done, j;
static SDL_bool use_target = SDL_FALSE;
#ifdef __EMSCRIPTEN__
static Uint32 wait_start;
#endif
static SDL_Texture *sprite;
static int sprite_w, sprite_h;

/* Call this instead of exit(), so we can clean up SDL: atexit() is evil. */
static void
quit(int rc)
{
    SDLTest_CommonQuit(state);
    exit(rc);
}

int
LoadSprite(char *file, SDL_Renderer *renderer)
{
    SDL_Surface *temp;

    /* Load the sprite image */
    temp = SDL_LoadBMP(file);
    if (temp == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load %s: %s\n", file, SDL_GetError());
        return (-1);
    }
    sprite_w = temp->w;
    sprite_h = temp->h;

    /* Set transparent pixel as the pixel at (0,0) */
    if (temp->format->palette) {
        SDL_SetColorKey(temp, SDL_TRUE, *(Uint8 *) temp->pixels);
    } else {
        switch (temp->format->BitsPerPixel) {
        case 15:
            SDL_SetColorKey(temp, SDL_TRUE,
                            (*(Uint16 *) temp->pixels) & 0x00007FFF);
            break;
        case 16:
            SDL_SetColorKey(temp, SDL_TRUE, *(Uint16 *) temp->pixels);
            break;
        case 24:
            SDL_SetColorKey(temp, SDL_TRUE,
                            (*(Uint32 *) temp->pixels) & 0x00FFFFFF);
            break;
        case 32:
            SDL_SetColorKey(temp, SDL_TRUE, *(Uint32 *) temp->pixels);
            break;
        }
    }

    /* Create textures from the image */
    sprite = SDL_CreateTextureFromSurface(renderer, temp);
    if (!sprite) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture: %s\n", SDL_GetError());
        SDL_FreeSurface(temp);
        return (-1);
    }
    SDL_FreeSurface(temp);

    /* We're ready to roll. :) */
    return (0);
}

void
DrawOnViewport(SDL_Renderer * renderer)
{    
    SDL_Rect rect;

    /* Set the viewport */
    SDL_RenderSetViewport(renderer, &viewport);
    
    /* Draw a gray background */
    SDL_SetRenderDrawColor(renderer, 0x80, 0x80, 0x80, 0xFF);
    SDL_RenderClear(renderer);

    /* Test inside points */
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x00, 0xFF);
    SDL_RenderDrawPoint(renderer, viewport.h/2 + 20, viewport.w/2);
    SDL_RenderDrawPoint(renderer, viewport.h/2 - 20, viewport.w/2);
    SDL_RenderDrawPoint(renderer, viewport.h/2     , viewport.w/2 - 20);
    SDL_RenderDrawPoint(renderer, viewport.h/2     , viewport.w/2 + 20);

    /* Test horizontal and vertical lines */
    SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
    SDL_RenderDrawLine(renderer, 1, 0, viewport.w-2, 0);
    SDL_RenderDrawLine(renderer, 1, viewport.h-1, viewport.w-2, viewport.h-1);
    SDL_RenderDrawLine(renderer, 0, 1, 0, viewport.h-2);
    SDL_RenderDrawLine(renderer, viewport.w-1, 1, viewport.w-1, viewport.h-2);

    /* Test diagonal lines */
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0xFF);
    SDL_RenderDrawLine(renderer, 0, 0, viewport.w-1, viewport.h-1);
    SDL_RenderDrawLine(renderer, viewport.w-1, 0, 0, viewport.h-1);                      

    /* Test outside points */
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x00, 0xFF);
    SDL_RenderDrawPoint(renderer, viewport.h/2 + viewport.h, viewport.w/2);
    SDL_RenderDrawPoint(renderer, viewport.h/2 - viewport.h, viewport.w/2);
    SDL_RenderDrawPoint(renderer, viewport.h/2, viewport.w/2 - viewport.w);
    SDL_RenderDrawPoint(renderer, viewport.h/2, viewport.w/2 + viewport.w);

    /* Add a box at the top */
    rect.w = 8;
    rect.h = 8;
    rect.x = (viewport.w - rect.w) / 2;
    rect.y = 0;
    SDL_RenderFillRect(renderer, &rect);

    /* Add a clip rect and fill it with the sprite */
    SDL_QueryTexture(sprite, NULL, NULL, &rect.w, &rect.h);
    rect.x = (viewport.w - rect.w) / 2;
    rect.y = (viewport.h - rect.h) / 2;
    SDL_RenderSetClipRect(renderer, &rect);
    SDL_RenderCopy(renderer, sprite, NULL, &rect);
    SDL_RenderSetClipRect(renderer, NULL);
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
        DrawOnViewport(state->renderers[i]);

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
            static const char *options[] = { "[--target]", NULL };
            SDLTest_CommonLogUsage(state, argv[0], options);
            quit(1);
        }
        i += consumed;
    }
    if (!SDLTest_CommonInit(state)) {
        quit(2);
    }

    if (LoadSprite("icon.bmp", state->renderers[0]) < 0) {
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
