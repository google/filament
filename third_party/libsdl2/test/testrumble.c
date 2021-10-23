/*
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/
/*
Copyright (c) 2011, Edgar Simo Serra
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Simple Directmedia Layer (SDL) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * includes
 */
#include <stdlib.h>
#include <string.h>             /* strstr */
#include <ctype.h>              /* isdigit */

#include "SDL.h"

#ifndef SDL_HAPTIC_DISABLED

static SDL_Haptic *haptic;


/**
 * @brief The entry point of this force feedback demo.
 * @param[in] argc Number of arguments.
 * @param[in] argv Array of argc arguments.
 */
int
main(int argc, char **argv)
{
    int i;
    char *name;
    int index;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    name = NULL;
    index = -1;
    if (argc > 1) {
        size_t l;
        name = argv[1];
        if ((strcmp(name, "--help") == 0) || (strcmp(name, "-h") == 0)) {
            SDL_Log("USAGE: %s [device]\n"
                   "If device is a two-digit number it'll use it as an index, otherwise\n"
                   "it'll use it as if it were part of the device's name.\n",
                   argv[0]);
            return 0;
        }

        l = SDL_strlen(name);
        if ((l < 3) && SDL_isdigit(name[0]) && ((l == 1) || SDL_isdigit(name[1]))) {
            index = SDL_atoi(name);
            name = NULL;
        }
    }

    /* Initialize the force feedbackness */
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK |
             SDL_INIT_HAPTIC);
    SDL_Log("%d Haptic devices detected.\n", SDL_NumHaptics());
    if (SDL_NumHaptics() > 0) {
        /* We'll just use index or the first force feedback device found */
        if (name == NULL) {
            i = (index != -1) ? index : 0;
        }
        /* Try to find matching device */
        else {
            for (i = 0; i < SDL_NumHaptics(); i++) {
                if (strstr(SDL_HapticName(i), name) != NULL)
                    break;
            }

            if (i >= SDL_NumHaptics()) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to find device matching '%s', aborting.\n",
                       name);
                return 1;
            }
        }

        haptic = SDL_HapticOpen(i);
        if (haptic == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create the haptic device: %s\n",
                   SDL_GetError());
            return 1;
        }
        SDL_Log("Device: %s\n", SDL_HapticName(i));
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No Haptic devices found!\n");
        return 1;
    }

    /* We only want force feedback errors. */
    SDL_ClearError();

    if (SDL_HapticRumbleSupported(haptic) == SDL_FALSE) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Rumble not supported!\n");
        return 1;
    }
    if (SDL_HapticRumbleInit(haptic) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize rumble: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Log("Playing 2 second rumble at 0.5 magnitude.\n");
    if (SDL_HapticRumblePlay(haptic, 0.5, 5000) != 0) {
       SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to play rumble: %s\n", SDL_GetError() );
       return 1;
    }
    SDL_Delay(2000);
    SDL_Log("Stopping rumble.\n");
    SDL_HapticRumbleStop(haptic);
    SDL_Delay(2000);
    SDL_Log("Playing 2 second rumble at 0.3 magnitude.\n");
    if (SDL_HapticRumblePlay(haptic, 0.3f, 5000) != 0) {
       SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to play rumble: %s\n", SDL_GetError() );
       return 1;
    }
    SDL_Delay(2000);

    /* Quit */
    if (haptic != NULL)
        SDL_HapticClose(haptic);
    SDL_Quit();

    return 0;
}

#else

int
main(int argc, char *argv[])
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL compiled without Haptic support.\n");
    return 1;
}

#endif
