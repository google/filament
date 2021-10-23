/*
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/
#include <stdio.h>
#include "SDL.h"

/* !!! FIXME: move this to the test framework */

static void log_locales(void)
{
    SDL_Locale *locales = SDL_GetPreferredLocales();
    if (locales == NULL) {
        SDL_Log("Couldn't determine locales: %s", SDL_GetError());
    } else {
        SDL_Locale *l;
        unsigned int total = 0;
        SDL_Log("Locales, in order of preference:");
        for (l = locales; l->language; l++) {
            const char *c = l->country;
            SDL_Log(" - %s%s%s", l->language, c ? "_" : "", c ? c : "");
            total++;
        }
        SDL_Log("%u locales seen.", total);
        SDL_free(locales);
    }
}

int main(int argc, char **argv)
{
    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    /* Print locales and languages */
    if (SDL_Init(SDL_INIT_VIDEO) != -1) {
        log_locales();

        if ((argc == 2) && (SDL_strcmp(argv[1], "--listen") == 0)) {
            SDL_bool keep_going = SDL_TRUE;
            while (keep_going) {
                SDL_Event e;
                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_QUIT) {
                        keep_going = SDL_FALSE;
                    } else if (e.type == SDL_LOCALECHANGED) {
                        SDL_Log("Saw SDL_LOCALECHANGED event!");
                        log_locales();
                    }
                }
                SDL_Delay(10);
            }
        }

        SDL_Quit();
    }

   return 0;
}

/* vi: set ts=4 sw=4 expandtab: */
