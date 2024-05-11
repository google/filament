/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../SDL_internal.h"
#include "SDL_power.h"
#include "SDL_syspower.h"

/*
 * Returns SDL_TRUE if we have a definitive answer.
 * SDL_FALSE to try next implementation.
 */
typedef SDL_bool
    (*SDL_GetPowerInfo_Impl) (SDL_PowerState * state, int *seconds,
                              int *percent);

#ifndef SDL_POWER_DISABLED
#ifdef SDL_POWER_HARDWIRED
/* This is for things that _never_ have a battery */
static SDL_bool
SDL_GetPowerInfo_Hardwired(SDL_PowerState * state, int *seconds, int *percent)
{
    *seconds = -1;
    *percent = -1;
    *state = SDL_POWERSTATE_NO_BATTERY;
    return SDL_TRUE;
}
#endif
#endif


static SDL_GetPowerInfo_Impl implementations[] = {
#ifndef SDL_POWER_DISABLED
#ifdef SDL_POWER_LINUX          /* in order of preference. More than could work. */
    SDL_GetPowerInfo_Linux_org_freedesktop_upower,
    SDL_GetPowerInfo_Linux_sys_class_power_supply,
    SDL_GetPowerInfo_Linux_proc_acpi,
    SDL_GetPowerInfo_Linux_proc_apm,
#endif
#ifdef SDL_POWER_WINDOWS        /* handles Win32, Win64, PocketPC. */
    SDL_GetPowerInfo_Windows,
#endif
#ifdef SDL_POWER_UIKIT          /* handles iPhone/iPad/etc */
    SDL_GetPowerInfo_UIKit,
#endif
#ifdef SDL_POWER_MACOSX         /* handles Mac OS X, Darwin. */
    SDL_GetPowerInfo_MacOSX,
#endif
#ifdef SDL_POWER_HAIKU          /* with BeOS euc.jp apm driver. Does this work on Haiku? */
    SDL_GetPowerInfo_Haiku,
#endif
#ifdef SDL_POWER_ANDROID        /* handles Android. */
    SDL_GetPowerInfo_Android,
#endif
#ifdef SDL_POWER_PSP        /* handles PSP. */
    SDL_GetPowerInfo_PSP,
#endif
#ifdef SDL_POWER_WINRT          /* handles WinRT */
    SDL_GetPowerInfo_WinRT,
#endif
#ifdef SDL_POWER_EMSCRIPTEN     /* handles Emscripten */
    SDL_GetPowerInfo_Emscripten,
#endif

#ifdef SDL_POWER_HARDWIRED
    SDL_GetPowerInfo_Hardwired,
#endif
#endif
};

SDL_PowerState
SDL_GetPowerInfo(int *seconds, int *percent)
{
    const int total = sizeof(implementations) / sizeof(implementations[0]);
    int _seconds, _percent;
    SDL_PowerState retval = SDL_POWERSTATE_UNKNOWN;
    int i;

    /* Make these never NULL for platform-specific implementations. */
    if (seconds == NULL) {
        seconds = &_seconds;
    }

    if (percent == NULL) {
        percent = &_percent;
    }

    for (i = 0; i < total; i++) {
        if (implementations[i](&retval, seconds, percent)) {
            return retval;
        }
    }

    /* nothing was definitive. */
    *seconds = -1;
    *percent = -1;
    return SDL_POWERSTATE_UNKNOWN;
}

/* vi: set ts=4 sw=4 expandtab: */
