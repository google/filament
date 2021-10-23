/*
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

/*  Usage:
 *  Spacebar to begin recording a gesture on all touches.
 *  s to save all touches into "./gestureSave"
 *  l to load all touches from "./gestureSave"
 */

#include "SDL.h"
#include <stdlib.h> /* for exit() */

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include "SDL_test.h"
#include "SDL_test_common.h"

#define WIDTH 640
#define HEIGHT 480
#define BPP 4

/* MUST BE A POWER OF 2! */
#define EVENT_BUF_SIZE 256

#define VERBOSE 0

static SDLTest_CommonState *state;
static SDL_Event events[EVENT_BUF_SIZE];
static int eventWrite;
static int colors[7] = {0xFF,0xFF00,0xFF0000,0xFFFF00,0x00FFFF,0xFF00FF,0xFFFFFF};
static int quitting = 0;

typedef struct
{
    float x, y;
} Point;

typedef struct
{
    float ang, r;
    Point p;
} Knob;

static Knob knob = { 0.0f, 0.1f, { 0.0f, 0.0f } };


static void
setpix(SDL_Surface *screen, float _x, float _y, unsigned int col)
{
    Uint32 *pixmem32;
    Uint32 colour;
    Uint8 r, g, b;
    const int x = (int)_x;
    const int y = (int)_y;
    float a;

    if ( (x < 0) || (x >= screen->w) || (y < 0) || (y >= screen->h) ) {
        return;
    }

    pixmem32 = (Uint32 *) screen->pixels + y * screen->pitch / BPP + x;

    SDL_memcpy(&colour, pixmem32, screen->format->BytesPerPixel);

    SDL_GetRGB(colour,screen->format,&r,&g,&b);

    /* r = 0;g = 0; b = 0; */
    a = (float) ((col >> 24) & 0xFF);
    if (a == 0) {
        a = 0xFF; /* Hack, to make things easier. */
    }

    a = (a == 0.0f) ? 1 : (a / 255.0f);
    r = (Uint8) (r * (1 - a) + ((col >> 16) & 0xFF) * a);
    g = (Uint8) (g * (1 - a) + ((col >> 8) & 0xFF) * a);
    b = (Uint8) (b * (1 - a) + ((col >> 0) & 0xFF) * a);
    colour = SDL_MapRGB(screen->format, r, g, b);

    *pixmem32 = colour;
}

#if 0 /* unused */
static void
drawLine(SDL_Surface *screen, float x0, float y0, float x1, float y1, unsigned int col)
{
    float t;
    for (t = 0; t < 1; t += (float) (1.0f / SDL_max(SDL_fabs(x0 - x1), SDL_fabs(y0 - y1)))) {
        setpix(screen, x1 + t * (x0 - x1), y1 + t * (y0 - y1), col);
    }
}
#endif

static void
drawCircle(SDL_Surface *screen, float x, float y, float r, unsigned int c)
{
    float tx,ty, xr;
    for (ty = (float) -SDL_fabs(r); ty <= (float) SDL_fabs((int) r); ty++) {
        xr = (float) SDL_sqrt(r * r - ty * ty);
        if (r > 0) { /* r > 0 ==> filled circle */
            for(tx = -xr + 0.5f; tx <= xr - 0.5f; tx++) {
                setpix(screen, x + tx, y + ty, c);
            }
        } else {
            setpix(screen, x - xr + 0.5f, y + ty, c);
            setpix(screen, x + xr - 0.5f, y + ty, c);
        }
    }
}

static void
drawKnob(SDL_Surface *screen, const Knob *k)
{
    drawCircle(screen, k->p.x * screen->w, k->p.y * screen->h, k->r * screen->w, 0xFFFFFF);
    drawCircle(screen, (k->p.x + k->r / 2 * SDL_cosf(k->ang)) * screen->w,
               (k->p.y + k->r / 2 * SDL_sinf(k->ang)) * screen->h, k->r / 4 * screen->w, 0);
}

static void
DrawScreen(SDL_Window *window)
{
    SDL_Surface *screen = SDL_GetWindowSurface(window);
    int i;

    if (!screen) {
        return;
    }

    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 75, 75, 75));

    /* draw Touch History */
    for (i = eventWrite; i < eventWrite + EVENT_BUF_SIZE; ++i) {
        const SDL_Event *event = &events[i & (EVENT_BUF_SIZE - 1)];
        const float age = (float)(i - eventWrite) / EVENT_BUF_SIZE;
        float x, y;
        unsigned int c, col;

        if ( (event->type == SDL_FINGERMOTION) ||
             (event->type == SDL_FINGERDOWN) ||
             (event->type == SDL_FINGERUP) ) {
            x = event->tfinger.x;
            y = event->tfinger.y;

            /* draw the touch: */
            c = colors[event->tfinger.fingerId % 7];
            col = ((unsigned int) (c * (0.1f + 0.85f))) | (unsigned int) (0xFF * age) << 24;

            if (event->type == SDL_FINGERMOTION) {
                drawCircle(screen, x * screen->w, y * screen->h, 5, col);
            } else if (event->type == SDL_FINGERDOWN) {
                drawCircle(screen, x * screen->w, y * screen->h, -10, col);
            }
        }
    }

    if (knob.p.x > 0) {
        drawKnob(screen, &knob);
    }

    SDL_UpdateWindowSurface(window);
}

static void
loop(void)
{
    SDL_Event event;
    SDL_RWops *stream;
    int i;

    while (SDL_PollEvent(&event)) {
        SDLTest_CommonEvent(state, &event, &quitting);

        /* Record _all_ events */
        events[eventWrite & (EVENT_BUF_SIZE-1)] = event;
        eventWrite++;

        switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_i: {
                        for (i = 0; i < SDL_GetNumTouchDevices(); ++i) {
                            const SDL_TouchID id = SDL_GetTouchDevice(i);
                            SDL_Log("Fingers Down on device %"SDL_PRIs64": %d", id, SDL_GetNumTouchFingers(id));
                        }
                        break;
                    }

                    case SDLK_SPACE:
                        SDL_RecordGesture(-1);
                        break;

                    case SDLK_s:
                        stream = SDL_RWFromFile("gestureSave", "w");
                        SDL_Log("Wrote %i templates", SDL_SaveAllDollarTemplates(stream));
                        SDL_RWclose(stream);
                        break;

                    case SDLK_l:
                        stream = SDL_RWFromFile("gestureSave", "r");
                        SDL_Log("Loaded: %i", SDL_LoadDollarTemplates(-1, stream));
                        SDL_RWclose(stream);
                        break;
                }
                break;

#if VERBOSE
            case SDL_FINGERMOTION:
                SDL_Log("Finger: %"SDL_PRIs64",x: %f, y: %f",event.tfinger.fingerId,
                        event.tfinger.x,event.tfinger.y);
                break;

            case SDL_FINGERDOWN:
                SDL_Log("Finger: %"SDL_PRIs64" down - x: %f, y: %f",
                        event.tfinger.fingerId,event.tfinger.x,event.tfinger.y);
                break;

            case SDL_FINGERUP:
                SDL_Log("Finger: %"SDL_PRIs64" up - x: %f, y: %f",
                        event.tfinger.fingerId,event.tfinger.x,event.tfinger.y);
                break;
#endif

            case SDL_MULTIGESTURE:
#if VERBOSE
                SDL_Log("Multi Gesture: x = %f, y = %f, dAng = %f, dR = %f",
                        event.mgesture.x, event.mgesture.y,
                        event.mgesture.dTheta, event.mgesture.dDist);
                SDL_Log("MG: numDownTouch = %i",event.mgesture.numFingers);
#endif

                knob.p.x = event.mgesture.x;
                knob.p.y = event.mgesture.y;
                knob.ang += event.mgesture.dTheta;
                knob.r += event.mgesture.dDist;
                break;

            case SDL_DOLLARGESTURE:
                SDL_Log("Gesture %"SDL_PRIs64" performed, error: %f",
                        event.dgesture.gestureId, event.dgesture.error);
                break;

            case SDL_DOLLARRECORD:
                SDL_Log("Recorded gesture: %"SDL_PRIs64"",event.dgesture.gestureId);
                break;
        }
    }

    for (i = 0; i < state->num_windows; ++i) {
        if (state->windows[i]) {
            DrawScreen(state->windows[i]);
        }
    }

#ifdef __EMSCRIPTEN__
    if (quitting) {
        emscripten_cancel_main_loop();
    }
#endif
}

int main(int argc, char* argv[])
{
    state = SDLTest_CommonCreateState(argv, SDL_INIT_VIDEO);
    if (!state) {
        return 1;
    }

    state->window_title = "Gesture Test";
    state->window_w = WIDTH;
    state->window_h = HEIGHT;
    state->skip_renderer = SDL_TRUE;

    if (!SDLTest_CommonDefaultArgs(state, argc, argv) || !SDLTest_CommonInit(state)) {
        SDLTest_CommonQuit(state);
        return 1;
    }

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loop, 0, 1);
#else
    while (!quitting) {
        loop();
    }
#endif

    SDLTest_CommonQuit(state);
    return 0;
}

