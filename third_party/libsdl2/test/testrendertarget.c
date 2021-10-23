/*
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/
/* Simple program:  Move N sprites around on the screen as fast as possible */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include "SDL_test_common.h"


static SDLTest_CommonState *state;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *background;
    SDL_Texture *sprite;
    SDL_Rect sprite_rect;
    int scale_direction;
} DrawState;

DrawState *drawstates;
int done;
SDL_bool test_composite = SDL_FALSE;

/* Call this instead of exit(), so we can clean up SDL: atexit() is evil. */
static void
quit(int rc)
{
    SDLTest_CommonQuit(state);
    exit(rc);
}

SDL_Texture *
LoadTexture(SDL_Renderer *renderer, char *file, SDL_bool transparent)
{
    SDL_Surface *temp;
    SDL_Texture *texture;

    /* Load the sprite image */
    temp = SDL_LoadBMP(file);
    if (temp == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load %s: %s", file, SDL_GetError());
        return NULL;
    }

    /* Set transparent pixel as the pixel at (0,0) */
    if (transparent) {
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
    }

    /* Create textures from the image */
    texture = SDL_CreateTextureFromSurface(renderer, temp);
    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture: %s\n", SDL_GetError());
        SDL_FreeSurface(temp);
        return NULL;
    }
    SDL_FreeSurface(temp);

    /* We're ready to roll. :) */
    return texture;
}

SDL_bool
DrawComposite(DrawState *s)
{
    SDL_Rect viewport, R;
    SDL_Texture *target;

    static SDL_bool blend_tested = SDL_FALSE;
    if (!blend_tested) {
        SDL_Texture *A, *B;
        Uint32 P;

        A = SDL_CreateTexture(s->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, 1, 1);
        SDL_SetTextureBlendMode(A, SDL_BLENDMODE_BLEND);

        B = SDL_CreateTexture(s->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, 1, 1);
        SDL_SetTextureBlendMode(B, SDL_BLENDMODE_BLEND);

        SDL_SetRenderTarget(s->renderer, A);
        SDL_SetRenderDrawColor(s->renderer, 0x00, 0x00, 0x00, 0x80);
        SDL_RenderFillRect(s->renderer, NULL);

        SDL_SetRenderTarget(s->renderer, B);
        SDL_SetRenderDrawColor(s->renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderFillRect(s->renderer, NULL);
        SDL_RenderCopy(s->renderer, A, NULL, NULL);
        SDL_RenderReadPixels(s->renderer, NULL, SDL_PIXELFORMAT_ARGB8888, &P, sizeof(P));

        SDL_Log("Blended pixel: 0x%8.8X\n", P);

        SDL_DestroyTexture(A);
        SDL_DestroyTexture(B);
        blend_tested = SDL_TRUE;
    }

    SDL_RenderGetViewport(s->renderer, &viewport);

    target = SDL_CreateTexture(s->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, viewport.w, viewport.h);
    SDL_SetTextureBlendMode(target, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(s->renderer, target);

    /* Draw the background.
       This is solid black so when the sprite is copied to it, any per-pixel alpha will be blended through.
     */
    SDL_SetRenderDrawColor(s->renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderFillRect(s->renderer, NULL);

    /* Scale and draw the sprite */
    s->sprite_rect.w += s->scale_direction;
    s->sprite_rect.h += s->scale_direction;
    if (s->scale_direction > 0) {
        if (s->sprite_rect.w >= viewport.w || s->sprite_rect.h >= viewport.h) {
            s->scale_direction = -1;
        }
    } else {
        if (s->sprite_rect.w <= 1 || s->sprite_rect.h <= 1) {
            s->scale_direction = 1;
        }
    }
    s->sprite_rect.x = (viewport.w - s->sprite_rect.w) / 2;
    s->sprite_rect.y = (viewport.h - s->sprite_rect.h) / 2;

    SDL_RenderCopy(s->renderer, s->sprite, NULL, &s->sprite_rect);

    SDL_SetRenderTarget(s->renderer, NULL);
    SDL_RenderCopy(s->renderer, s->background, NULL, NULL);

    SDL_SetRenderDrawBlendMode(s->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(s->renderer, 0xff, 0x00, 0x00, 0x80);
    R.x = 0;
    R.y = 0;
    R.w = 100;
    R.h = 100;
    SDL_RenderFillRect(s->renderer, &R);
    SDL_SetRenderDrawBlendMode(s->renderer, SDL_BLENDMODE_NONE);

    SDL_RenderCopy(s->renderer, target, NULL, NULL);
    SDL_DestroyTexture(target);

    /* Update the screen! */
    SDL_RenderPresent(s->renderer);
    return SDL_TRUE;
}

SDL_bool
Draw(DrawState *s)
{
    SDL_Rect viewport;
    SDL_Texture *target;

    SDL_RenderGetViewport(s->renderer, &viewport);

    target = SDL_CreateTexture(s->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, viewport.w, viewport.h);
    if (!target) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create render target texture: %s\n", SDL_GetError());
        return SDL_FALSE;
    }
    SDL_SetRenderTarget(s->renderer, target);

    /* Draw the background */
    SDL_RenderCopy(s->renderer, s->background, NULL, NULL);

    /* Scale and draw the sprite */
    s->sprite_rect.w += s->scale_direction;
    s->sprite_rect.h += s->scale_direction;
    if (s->scale_direction > 0) {
        if (s->sprite_rect.w >= viewport.w || s->sprite_rect.h >= viewport.h) {
            s->scale_direction = -1;
        }
    } else {
        if (s->sprite_rect.w <= 1 || s->sprite_rect.h <= 1) {
            s->scale_direction = 1;
        }
    }
    s->sprite_rect.x = (viewport.w - s->sprite_rect.w) / 2;
    s->sprite_rect.y = (viewport.h - s->sprite_rect.h) / 2;

    SDL_RenderCopy(s->renderer, s->sprite, NULL, &s->sprite_rect);

    SDL_SetRenderTarget(s->renderer, NULL);
    SDL_RenderCopy(s->renderer, target, NULL, NULL);
    SDL_DestroyTexture(target);

    /* Update the screen! */
    SDL_RenderPresent(s->renderer);
    return SDL_TRUE;
}

void
loop()
{
    int i;
    SDL_Event event;

    /* Check for events */
    while (SDL_PollEvent(&event)) {
        SDLTest_CommonEvent(state, &event, &done);
    }
    for (i = 0; i < state->num_windows; ++i) {
        if (state->windows[i] == NULL)
            continue;
        if (test_composite) {
            if (!DrawComposite(&drawstates[i])) done = 1;
        } else {
            if (!Draw(&drawstates[i])) done = 1;
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
    int frames;
    Uint32 then, now;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

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
            if (SDL_strcasecmp(argv[i], "--composite") == 0) {
                test_composite = SDL_TRUE;
                consumed = 1;
            }
        }
        if (consumed < 0) {
            static const char *options[] = { "[--composite]", NULL };
            SDLTest_CommonLogUsage(state, argv[0], options);
            quit(1);
        }
        i += consumed;
    }
    if (!SDLTest_CommonInit(state)) {
        quit(2);
    }

    drawstates = SDL_stack_alloc(DrawState, state->num_windows);
    for (i = 0; i < state->num_windows; ++i) {
        DrawState *drawstate = &drawstates[i];

        drawstate->window = state->windows[i];
        drawstate->renderer = state->renderers[i];
        if (test_composite) {
            drawstate->sprite = LoadTexture(drawstate->renderer, "icon-alpha.bmp", SDL_TRUE);
        } else {
            drawstate->sprite = LoadTexture(drawstate->renderer, "icon.bmp", SDL_TRUE);
        }
        drawstate->background = LoadTexture(drawstate->renderer, "sample.bmp", SDL_FALSE);
        if (!drawstate->sprite || !drawstate->background) {
            quit(2);
        }
        SDL_QueryTexture(drawstate->sprite, NULL, NULL,
                         &drawstate->sprite_rect.w, &drawstate->sprite_rect.h);
        drawstate->scale_direction = 1;
    }

    /* Main render loop */
    frames = 0;
    then = SDL_GetTicks();
    done = 0;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loop, 0, 1);
#else
    while (!done) {
        ++frames;
        loop();
    }
#endif

    /* Print out some timing information */
    now = SDL_GetTicks();
    if (now > then) {
        double fps = ((double) frames * 1000) / (now - then);
        SDL_Log("%2.2f frames per second\n", fps);
    }

    SDL_stack_free(drawstates);

    quit(0);
    return 0;
}

/* vi: set ts=4 sw=4 expandtab: */
