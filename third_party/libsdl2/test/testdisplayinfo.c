/*
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

/* Program to test querying of display info */

#include "SDL.h"

#include <stdio.h>
#include <stdlib.h>

static void
print_mode(const char *prefix, const SDL_DisplayMode *mode)
{
    if (!mode)
        return;

    SDL_Log("%s: fmt=%s w=%d h=%d refresh=%d\n",
            prefix, SDL_GetPixelFormatName(mode->format),
            mode->w, mode->h, mode->refresh_rate);
}

int
main(int argc, char *argv[])
{
    SDL_DisplayMode mode;
    int num_displays, dpy;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    /* Load the SDL library */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Log("Using video target '%s'.\n", SDL_GetCurrentVideoDriver());
    num_displays = SDL_GetNumVideoDisplays();

    SDL_Log("See %d displays.\n", num_displays);

    for (dpy = 0; dpy < num_displays; dpy++) {
        const int num_modes = SDL_GetNumDisplayModes(dpy);
        SDL_Rect rect = { 0, 0, 0, 0 };
        float ddpi, hdpi, vdpi;
        int m;

        SDL_GetDisplayBounds(dpy, &rect);
        SDL_Log("%d: \"%s\" (%dx%d, (%d, %d)), %d modes.\n", dpy, SDL_GetDisplayName(dpy), rect.w, rect.h, rect.x, rect.y, num_modes);

        if (SDL_GetDisplayDPI(dpy, &ddpi, &hdpi, &vdpi) == -1) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "    DPI: failed to query (%s)\n", SDL_GetError());
        } else {
            SDL_Log("    DPI: ddpi=%f; hdpi=%f; vdpi=%f\n", ddpi, hdpi, vdpi);
        }

        if (SDL_GetCurrentDisplayMode(dpy, &mode) == -1) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "    CURRENT: failed to query (%s)\n", SDL_GetError());
        } else {
            print_mode("CURRENT", &mode);
        }

        if (SDL_GetDesktopDisplayMode(dpy, &mode) == -1) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "    DESKTOP: failed to query (%s)\n", SDL_GetError());
        } else {
            print_mode("DESKTOP", &mode);
        }

        for (m = 0; m < num_modes; m++) {
            if (SDL_GetDisplayMode(dpy, m, &mode) == -1) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "    MODE %d: failed to query (%s)\n", m, SDL_GetError());
            } else {
                char prefix[64];
                SDL_snprintf(prefix, sizeof (prefix), "    MODE %d", m);
                print_mode(prefix, &mode);
            }
        }

        SDL_Log("\n");
    }

    SDL_Quit();
    return 0;
}

/* vi: set ts=4 sw=4 expandtab: */

