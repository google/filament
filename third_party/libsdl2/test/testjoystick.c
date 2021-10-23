/*
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

/* Simple program to test the SDL joystick routines */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#ifndef SDL_JOYSTICK_DISABLED

#ifdef __IPHONEOS__
#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   480
#else
#define SCREEN_WIDTH    640
#define SCREEN_HEIGHT   480
#endif

static SDL_Window *window = NULL;
static SDL_Renderer *screen = NULL;
static SDL_Joystick *joystick = NULL;
static SDL_bool done = SDL_FALSE;

static void
PrintJoystick(SDL_Joystick *joystick)
{
    const char *type;
    char guid[64];

    SDL_assert(SDL_JoystickFromInstanceID(SDL_JoystickInstanceID(joystick)) == joystick);
    SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(joystick), guid, sizeof (guid));
    switch (SDL_JoystickGetType(joystick)) {
    case SDL_JOYSTICK_TYPE_GAMECONTROLLER:
        type = "Game Controller";
        break;
    case SDL_JOYSTICK_TYPE_WHEEL:
        type = "Wheel";
        break;
    case SDL_JOYSTICK_TYPE_ARCADE_STICK:
        type = "Arcade Stick";
        break;
    case SDL_JOYSTICK_TYPE_FLIGHT_STICK:
        type = "Flight Stick";
        break;
    case SDL_JOYSTICK_TYPE_DANCE_PAD:
        type = "Dance Pad";
        break;
    case SDL_JOYSTICK_TYPE_GUITAR:
        type = "Guitar";
        break;
    case SDL_JOYSTICK_TYPE_DRUM_KIT:
        type = "Drum Kit";
        break;
    case SDL_JOYSTICK_TYPE_ARCADE_PAD:
        type = "Arcade Pad";
        break;
    case SDL_JOYSTICK_TYPE_THROTTLE:
        type = "Throttle";
        break;
    default:
        type = "Unknown";
        break;
    }
    SDL_Log("Joystick\n");
    SDL_Log("       name: %s\n", SDL_JoystickName(joystick));
    SDL_Log("       type: %s\n", type);
    SDL_Log("       axes: %d\n", SDL_JoystickNumAxes(joystick));
    SDL_Log("      balls: %d\n", SDL_JoystickNumBalls(joystick));
    SDL_Log("       hats: %d\n", SDL_JoystickNumHats(joystick));
    SDL_Log("    buttons: %d\n", SDL_JoystickNumButtons(joystick));
    SDL_Log("instance id: %d\n", SDL_JoystickInstanceID(joystick));
    SDL_Log("       guid: %s\n", guid);
    SDL_Log("    VID/PID: 0x%.4x/0x%.4x\n", SDL_JoystickGetVendor(joystick), SDL_JoystickGetProduct(joystick));
}

static void
DrawRect(SDL_Renderer *r, const int x, const int y, const int w, const int h)
{
    SDL_Rect area;
    area.x = x;
    area.y = y;
    area.w = w;
    area.h = h;
    SDL_RenderFillRect(r, &area);
}

void
loop(void *arg)
{
    SDL_Event event;
    int i;

    /* blank screen, set up for drawing this frame. */
    SDL_SetRenderDrawColor(screen, 0x0, 0x0, 0x0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(screen);

    while (SDL_PollEvent(&event)) {
        switch (event.type) {

        case SDL_JOYDEVICEADDED:
            SDL_Log("Joystick device %d added.\n", (int) event.jdevice.which);
            if (!joystick) {
                joystick = SDL_JoystickOpen(event.jdevice.which);
                if (joystick) {
                    PrintJoystick(joystick);
                } else {
                    SDL_Log("Couldn't open joystick: %s\n", SDL_GetError());
                }
            }
            break;

        case SDL_JOYDEVICEREMOVED:
            SDL_Log("Joystick device %d removed.\n", (int) event.jdevice.which);
            if (event.jdevice.which == SDL_JoystickInstanceID(joystick)) {
                SDL_JoystickClose(joystick);
                joystick = SDL_JoystickOpen(0);
            }
            break;

        case SDL_JOYAXISMOTION:
            SDL_Log("Joystick %d axis %d value: %d\n",
                   event.jaxis.which,
                   event.jaxis.axis, event.jaxis.value);
            break;
        case SDL_JOYHATMOTION:
            SDL_Log("Joystick %d hat %d value:",
                   event.jhat.which, event.jhat.hat);
            if (event.jhat.value == SDL_HAT_CENTERED)
                SDL_Log(" centered");
            if (event.jhat.value & SDL_HAT_UP)
                SDL_Log(" up");
            if (event.jhat.value & SDL_HAT_RIGHT)
                SDL_Log(" right");
            if (event.jhat.value & SDL_HAT_DOWN)
                SDL_Log(" down");
            if (event.jhat.value & SDL_HAT_LEFT)
                SDL_Log(" left");
            SDL_Log("\n");
            break;
        case SDL_JOYBALLMOTION:
            SDL_Log("Joystick %d ball %d delta: (%d,%d)\n",
                   event.jball.which,
                   event.jball.ball, event.jball.xrel, event.jball.yrel);
            break;
        case SDL_JOYBUTTONDOWN:
            SDL_Log("Joystick %d button %d down\n",
                   event.jbutton.which, event.jbutton.button);
            /* First button triggers a 0.5 second full strength rumble */
            if (event.jbutton.button == 0) {
                SDL_JoystickRumble(joystick, 0xFFFF, 0xFFFF, 500);
            }
            break;
        case SDL_JOYBUTTONUP:
            SDL_Log("Joystick %d button %d up\n",
                   event.jbutton.which, event.jbutton.button);
            break;
        case SDL_KEYDOWN:
            /* Press the L key to lag for 3 seconds, to see what happens
                when SDL doesn't service the event loop quickly. */
            if (event.key.keysym.sym == SDLK_l) {
                SDL_Log("Lagging for 3 seconds...\n");
                SDL_Delay(3000);
                break;
            }

            if ((event.key.keysym.sym != SDLK_ESCAPE) &&
                (event.key.keysym.sym != SDLK_AC_BACK)) {
                break;
            }
            /* Fall through to signal quit */
        case SDL_FINGERDOWN:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_QUIT:
            done = SDL_TRUE;
            break;
        default:
            break;
        }
    }

    if (joystick) {

        /* Update visual joystick state */
        SDL_SetRenderDrawColor(screen, 0x00, 0xFF, 0x00, SDL_ALPHA_OPAQUE);
        for (i = 0; i < SDL_JoystickNumButtons(joystick); ++i) {
            if (SDL_JoystickGetButton(joystick, i) == SDL_PRESSED) {
                DrawRect(screen, (i%20) * 34, SCREEN_HEIGHT - 68 + (i/20) * 34, 32, 32);
            }
        }

        SDL_SetRenderDrawColor(screen, 0xFF, 0x00, 0x00, SDL_ALPHA_OPAQUE);
        for (i = 0; i < SDL_JoystickNumAxes(joystick); ++i) {
            /* Draw the X/Y axis */
            int x, y;
            x = (((int) SDL_JoystickGetAxis(joystick, i)) + 32768);
            x *= SCREEN_WIDTH;
            x /= 65535;
            if (x < 0) {
                x = 0;
            } else if (x > (SCREEN_WIDTH - 16)) {
                x = SCREEN_WIDTH - 16;
            }
            ++i;
            if (i < SDL_JoystickNumAxes(joystick)) {
                y = (((int) SDL_JoystickGetAxis(joystick, i)) + 32768);
            } else {
                y = 32768;
            }
            y *= SCREEN_HEIGHT;
            y /= 65535;
            if (y < 0) {
                y = 0;
            } else if (y > (SCREEN_HEIGHT - 16)) {
                y = SCREEN_HEIGHT - 16;
            }

            DrawRect(screen, x, y, 16, 16);
        }

        SDL_SetRenderDrawColor(screen, 0x00, 0x00, 0xFF, SDL_ALPHA_OPAQUE);
        for (i = 0; i < SDL_JoystickNumHats(joystick); ++i) {
            /* Derive the new position */
            int x = SCREEN_WIDTH/2;
            int y = SCREEN_HEIGHT/2;
            const Uint8 hat_pos = SDL_JoystickGetHat(joystick, i);

            if (hat_pos & SDL_HAT_UP) {
                y = 0;
            } else if (hat_pos & SDL_HAT_DOWN) {
                y = SCREEN_HEIGHT-8;
            }

            if (hat_pos & SDL_HAT_LEFT) {
                x = 0;
            } else if (hat_pos & SDL_HAT_RIGHT) {
                x = SCREEN_WIDTH-8;
            }

            DrawRect(screen, x, y, 8, 8);
        }
    }

    SDL_RenderPresent(screen);

#ifdef __EMSCRIPTEN__
    if (done) {
        emscripten_cancel_main_loop();
    }
#endif
}

int
main(int argc, char *argv[])
{
    SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    /* Initialize SDL (Note: video is required to start event loop) */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    /* Create a window to display joystick axis position */
    window = SDL_CreateWindow("Joystick Test", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
                              SCREEN_HEIGHT, 0);
    if (window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s\n", SDL_GetError());
        return SDL_FALSE;
    }

    screen = SDL_CreateRenderer(window, -1, 0);
    if (screen == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        return SDL_FALSE;
    }

    SDL_SetRenderDrawColor(screen, 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(screen);
    SDL_RenderPresent(screen);

    /* Loop, getting joystick events! */
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(loop, NULL, 0, 1);
#else
    while (!done) {
        loop(NULL);
    }
#endif

    SDL_DestroyRenderer(screen);
    SDL_DestroyWindow(window);

    SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

    return 0;
}

#else

int
main(int argc, char *argv[])
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL compiled without Joystick support.\n");
    return 1;
}

#endif

/* vi: set ts=4 sw=4 expandtab: */
