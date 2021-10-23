/*
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

#include "SDL.h"

int main(int argc, char **argv)
{
    int total, i;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL_Init(SDL_INIT_VIDEO) failed: %s", SDL_GetError());
        return 1;
    }

    total = SDL_GetNumVideoDisplays();
    for (i = 0; i < total; i++) {
        SDL_Rect bounds = { -1,-1,-1,-1 }, usable = { -1,-1,-1,-1 };
        SDL_GetDisplayBounds(i, &bounds);
        SDL_GetDisplayUsableBounds(i, &usable);
        SDL_Log("Display #%d ('%s'): bounds={(%d,%d),%dx%d}, usable={(%d,%d),%dx%d}",
                i, SDL_GetDisplayName(i),
                bounds.x, bounds.y, bounds.w, bounds.h,
                usable.x, usable.y, usable.w, usable.h);
    }

    SDL_Quit();
    return 0;
}

/* vi: set ts=4 sw=4 expandtab: */

