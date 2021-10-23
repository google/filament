/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

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
#include "../../SDL_internal.h"

#ifndef SDL_POWER_DISABLED
#if SDL_POWER_WINDOWS

#include "../../core/windows/SDL_windows.h"

#include "SDL_power.h"

SDL_bool
SDL_GetPowerInfo_Windows(SDL_PowerState * state, int *seconds, int *percent)
{
    SYSTEM_POWER_STATUS status;
    SDL_bool need_details = SDL_FALSE;

    /* This API should exist back to Win95. */
    if (!GetSystemPowerStatus(&status))
    {
        /* !!! FIXME: push GetLastError() into SDL_GetError() */
        *state = SDL_POWERSTATE_UNKNOWN;
    } else if (status.BatteryFlag == 0xFF) {    /* unknown state */
        *state = SDL_POWERSTATE_UNKNOWN;
    } else if (status.BatteryFlag & (1 << 7)) { /* no battery */
        *state = SDL_POWERSTATE_NO_BATTERY;
    } else if (status.BatteryFlag & (1 << 3)) { /* charging */
        *state = SDL_POWERSTATE_CHARGING;
        need_details = SDL_TRUE;
    } else if (status.ACLineStatus == 1) {
        *state = SDL_POWERSTATE_CHARGED;        /* on AC, not charging. */
        need_details = SDL_TRUE;
    } else {
        *state = SDL_POWERSTATE_ON_BATTERY;     /* not on AC. */
        need_details = SDL_TRUE;
    }

    *percent = -1;
    *seconds = -1;
    if (need_details) {
        const int pct = (int) status.BatteryLifePercent;
        const int secs = (int) status.BatteryLifeTime;

        if (pct != 255) {       /* 255 == unknown */
            *percent = (pct > 100) ? 100 : pct; /* clamp between 0%, 100% */
        }
        if (secs != 0xFFFFFFFF) {       /* ((DWORD)-1) == unknown */
            *seconds = secs;
        }
    }

    return SDL_TRUE;            /* always the definitive answer on Windows. */
}

#endif /* SDL_POWER_WINDOWS */
#endif /* SDL_POWER_DISABLED */

/* vi: set ts=4 sw=4 expandtab: */
