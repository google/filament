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

#include "SDL_test.h"
#include "SDL_test_common.h"

#define NUM_SPRITES    100
#define MAX_SPEED     1

static SDLTest_CommonState *state;
static int num_sprites;
static SDL_Texture **sprites;
static SDL_bool cycle_color;
static SDL_bool cycle_alpha;
static int cycle_direction = 1;
static int current_alpha = 0;
static int current_color = 0;
static SDL_Rect *positions;
static SDL_Rect *velocities;
static int sprite_w, sprite_h;
static SDL_BlendMode blendMode = SDL_BLENDMODE_BLEND;
static Uint32 next_fps_check, frames;
static const Uint32 fps_check_delay = 5000;

/* Number of iterations to move sprites - used for visual tests. */
/* -1: infinite random moves (default); >=0: enables N deterministic moves */
static int iterations = -1;

int done;

/* Call this instead of exit(), so we can clean up SDL: atexit() is evil. */
static void
quit(int rc)
{
    SDL_free(sprites);
    SDL_free(positions);
    SDL_free(velocities);
    SDLTest_CommonQuit(state);
    exit(rc);
}

int
LoadSprite(const char *file)
{
    int i;
    SDL_Surface *temp;

    /* Load the sprite image */
    temp = SDL_LoadBMP(file);
    if (temp == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load %s: %s", file, SDL_GetError());
        return (-1);
    }
    sprite_w = temp->w;
    sprite_h = temp->h;

    /* Set transparent pixel as the pixel at (0,0) */
    if (temp->format->palette) {
        SDL_SetColorKey(temp, 1, *(Uint8 *) temp->pixels);
    } else {
        switch (temp->format->BitsPerPixel) {
        case 15:
            SDL_SetColorKey(temp, 1, (*(Uint16 *) temp->pixels) & 0x00007FFF);
            break;
        case 16:
            SDL_SetColorKey(temp, 1, *(Uint16 *) temp->pixels);
            break;
        case 24:
            SDL_SetColorKey(temp, 1, (*(Uint32 *) temp->pixels) & 0x00FFFFFF);
            break;
        case 32:
            SDL_SetColorKey(temp, 1, *(Uint32 *) temp->pixels);
            break;
        }
    }

    /* Create textures from the image */
    for (i = 0; i < state->num_windows; ++i) {
        SDL_Renderer *renderer = state->renderers[i];
        sprites[i] = SDL_CreateTextureFromSurface(renderer, temp);
        if (!sprites[i]) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture: %s\n", SDL_GetError());
            SDL_FreeSurface(temp);
            return (-1);
        }
        if (SDL_SetTextureBlendMode(sprites[i], blendMode) < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't set blend mode: %s\n", SDL_GetError());
            SDL_FreeSurface(temp);
            SDL_DestroyTexture(sprites[i]);
            return (-1);
        }
    }
    SDL_FreeSurface(temp);

    /* We're ready to roll. :) */
    return (0);
}

void
MoveSprites(SDL_Renderer * renderer, SDL_Texture * sprite)
{
    int i;
    SDL_Rect viewport, temp;
    SDL_Rect *position, *velocity;

    /* Query the sizes */
    SDL_RenderGetViewport(renderer, &viewport);

    /* Cycle the color and alpha, if desired */
    if (cycle_color) {
        current_color += cycle_direction;
        if (current_color < 0) {
            current_color = 0;
            cycle_direction = -cycle_direction;
        }
        if (current_color > 255) {
            current_color = 255;
            cycle_direction = -cycle_direction;
        }
        SDL_SetTextureColorMod(sprite, 255, (Uint8) current_color,
                               (Uint8) current_color);
    }
    if (cycle_alpha) {
        current_alpha += cycle_direction;
        if (current_alpha < 0) {
            current_alpha = 0;
            cycle_direction = -cycle_direction;
        }
        if (current_alpha > 255) {
            current_alpha = 255;
            cycle_direction = -cycle_direction;
        }
        SDL_SetTextureAlphaMod(sprite, (Uint8) current_alpha);
    }

    /* Draw a gray background */
    SDL_SetRenderDrawColor(renderer, 0xA0, 0xA0, 0xA0, 0xFF);
    SDL_RenderClear(renderer);

    /* Test points */
    SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
    SDL_RenderDrawPoint(renderer, 0, 0);
    SDL_RenderDrawPoint(renderer, viewport.w-1, 0);
    SDL_RenderDrawPoint(renderer, 0, viewport.h-1);
    SDL_RenderDrawPoint(renderer, viewport.w-1, viewport.h-1);

    /* Test horizontal and vertical lines */
    SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
    SDL_RenderDrawLine(renderer, 1, 0, viewport.w-2, 0);
    SDL_RenderDrawLine(renderer, 1, viewport.h-1, viewport.w-2, viewport.h-1);
    SDL_RenderDrawLine(renderer, 0, 1, 0, viewport.h-2);
    SDL_RenderDrawLine(renderer, viewport.w-1, 1, viewport.w-1, viewport.h-2);

    /* Test fill and copy */
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    temp.x = 1;
    temp.y = 1;
    temp.w = sprite_w;
    temp.h = sprite_h;
    SDL_RenderFillRect(renderer, &temp);
    SDL_RenderCopy(renderer, sprite, NULL, &temp);
    temp.x = viewport.w-sprite_w-1;
    temp.y = 1;
    temp.w = sprite_w;
    temp.h = sprite_h;
    SDL_RenderFillRect(renderer, &temp);
    SDL_RenderCopy(renderer, sprite, NULL, &temp);
    temp.x = 1;
    temp.y = viewport.h-sprite_h-1;
    temp.w = sprite_w;
    temp.h = sprite_h;
    SDL_RenderFillRect(renderer, &temp);
    SDL_RenderCopy(renderer, sprite, NULL, &temp);
    temp.x = viewport.w-sprite_w-1;
    temp.y = viewport.h-sprite_h-1;
    temp.w = sprite_w;
    temp.h = sprite_h;
    SDL_RenderFillRect(renderer, &temp);
    SDL_RenderCopy(renderer, sprite, NULL, &temp);

    /* Test diagonal lines */
    SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
    SDL_RenderDrawLine(renderer, sprite_w, sprite_h,
                       viewport.w-sprite_w-2, viewport.h-sprite_h-2);
    SDL_RenderDrawLine(renderer, viewport.w-sprite_w-2, sprite_h,
                       sprite_w, viewport.h-sprite_h-2);

    /* Conditionally move the sprites, bounce at the wall */
    if (iterations == -1 || iterations > 0) {
        for (i = 0; i < num_sprites; ++i) {
            position = &positions[i];
            velocity = &velocities[i];
            position->x += velocity->x;
            if ((position->x < 0) || (position->x >= (viewport.w - sprite_w))) {
                velocity->x = -velocity->x;
                position->x += velocity->x;
            }
            position->y += velocity->y;
            if ((position->y < 0) || (position->y >= (viewport.h - sprite_h))) {
                velocity->y = -velocity->y;
                position->y += velocity->y;
            }

        }
        
        /* Countdown sprite-move iterations and disable color changes at iteration end - used for visual tests. */
        if (iterations > 0) {
            iterations--;
            if (iterations == 0) {
                cycle_alpha = SDL_FALSE;
                cycle_color = SDL_FALSE;
            }
        }
    }

    /* Draw sprites */
    for (i = 0; i < num_sprites; ++i) {
        position = &positions[i];

        /* Blit the sprite onto the screen */
        SDL_RenderCopy(renderer, sprite, NULL, position);
    }

    /* Update the screen! */
    SDL_RenderPresent(renderer);
}

void
loop()
{
    Uint32 now;
    int i;
    SDL_Event event;

    /* Check for events */
    while (SDL_PollEvent(&event)) {
        SDLTest_CommonEvent(state, &event, &done);
    }
    for (i = 0; i < state->num_windows; ++i) {
        if (state->windows[i] == NULL)
            continue;
        MoveSprites(state->renderers[i], sprites[i]);
    }
#ifdef __EMSCRIPTEN__
    if (done) {
        emscripten_cancel_main_loop();
    }
#endif

    frames++;
    now = SDL_GetTicks();
    if (SDL_TICKS_PASSED(now, next_fps_check)) {
        /* Print out some timing information */
        const Uint32 then = next_fps_check - fps_check_delay;
        const double fps = ((double) frames * 1000) / (now - then);
        SDL_Log("%2.2f frames per second\n", fps);
        next_fps_check = now + fps_check_delay;
        frames = 0;
    }

}

int
main(int argc, char *argv[])
{
    int i;
    Uint64 seed;
    const char *icon = "icon.bmp";

    /* Initialize parameters */
    num_sprites = NUM_SPRITES;

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
            if (SDL_strcasecmp(argv[i], "--blend") == 0) {
                if (argv[i + 1]) {
                    if (SDL_strcasecmp(argv[i + 1], "none") == 0) {
                        blendMode = SDL_BLENDMODE_NONE;
                        consumed = 2;
                    } else if (SDL_strcasecmp(argv[i + 1], "blend") == 0) {
                        blendMode = SDL_BLENDMODE_BLEND;
                        consumed = 2;
                    } else if (SDL_strcasecmp(argv[i + 1], "add") == 0) {
                        blendMode = SDL_BLENDMODE_ADD;
                        consumed = 2;
                    } else if (SDL_strcasecmp(argv[i + 1], "mod") == 0) {
                        blendMode = SDL_BLENDMODE_MOD;
                        consumed = 2;
                    } else if (SDL_strcasecmp(argv[i + 1], "sub") == 0) {
                        blendMode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_SRC_ALPHA, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_SUBTRACT, SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_SUBTRACT);
                        consumed = 2;
                    }
                }
            } else if (SDL_strcasecmp(argv[i], "--iterations") == 0) {
                if (argv[i + 1]) {
                    iterations = SDL_atoi(argv[i + 1]);
                    if (iterations < -1) iterations = -1;
                    consumed = 2;
                }
            } else if (SDL_strcasecmp(argv[i], "--cyclecolor") == 0) {
                cycle_color = SDL_TRUE;
                consumed = 1;
            } else if (SDL_strcasecmp(argv[i], "--cyclealpha") == 0) {
                cycle_alpha = SDL_TRUE;
                consumed = 1;
            } else if (SDL_isdigit(*argv[i])) {
                num_sprites = SDL_atoi(argv[i]);
                consumed = 1;
            } else if (argv[i][0] != '-') {
                icon = argv[i];
                consumed = 1;
            }
        }
        if (consumed < 0) {
            static const char *options[] = { "[--blend none|blend|add|mod]", "[--cyclecolor]", "[--cyclealpha]", "[--iterations N]", "[num_sprites]", "[icon.bmp]", NULL };
            SDLTest_CommonLogUsage(state, argv[0], options);
            quit(1);
        }
        i += consumed;
    }
    if (!SDLTest_CommonInit(state)) {
        quit(2);
    }

    /* Create the windows, initialize the renderers, and load the textures */
    sprites =
        (SDL_Texture **) SDL_malloc(state->num_windows * sizeof(*sprites));
    if (!sprites) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Out of memory!\n");
        quit(2);
    }
    for (i = 0; i < state->num_windows; ++i) {
        SDL_Renderer *renderer = state->renderers[i];
        SDL_SetRenderDrawColor(renderer, 0xA0, 0xA0, 0xA0, 0xFF);
        SDL_RenderClear(renderer);
    }
    if (LoadSprite(icon) < 0) {
        quit(2);
    }

    /* Allocate memory for the sprite info */
    positions = (SDL_Rect *) SDL_malloc(num_sprites * sizeof(SDL_Rect));
    velocities = (SDL_Rect *) SDL_malloc(num_sprites * sizeof(SDL_Rect));
    if (!positions || !velocities) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Out of memory!\n");
        quit(2);
    }

    /* Position sprites and set their velocities using the fuzzer */ 
    if (iterations >= 0) {
        /* Deterministic seed - used for visual tests */
        seed = (Uint64)iterations;
    } else {
        /* Pseudo-random seed generated from the time */
        seed = (Uint64)time(NULL);
    }
    SDLTest_FuzzerInit(seed);
    for (i = 0; i < num_sprites; ++i) {
        positions[i].x = SDLTest_RandomIntegerInRange(0, state->window_w - sprite_w);
        positions[i].y = SDLTest_RandomIntegerInRange(0, state->window_h - sprite_h);
        positions[i].w = sprite_w;
        positions[i].h = sprite_h;
        velocities[i].x = 0;
        velocities[i].y = 0;
        while (!velocities[i].x && !velocities[i].y) {
            velocities[i].x = SDLTest_RandomIntegerInRange(-MAX_SPEED, MAX_SPEED);
            velocities[i].y = SDLTest_RandomIntegerInRange(-MAX_SPEED, MAX_SPEED);
        }
    }

    /* Main render loop */
    frames = 0;
    next_fps_check = SDL_GetTicks() + fps_check_delay;
    done = 0;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loop, 0, 1);
#else
    while (!done) {
        loop();
    }
#endif

    quit(0);
    return 0;
}

/* vi: set ts=4 sw=4 expandtab: */
