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

#ifdef SDL_TIMER_WINDOWS

#include "../../core/windows/SDL_windows.h"
#include <mmsystem.h>

#include "SDL_timer.h"
#include "SDL_hints.h"


/* The first (low-resolution) ticks value of the application */
static DWORD start = 0;
static BOOL ticks_started = FALSE; 

/* Store if a high-resolution performance counter exists on the system */
static BOOL hires_timer_available;
/* The first high-resolution ticks value of the application */
static LARGE_INTEGER hires_start_ticks;
/* The number of ticks per second of the high-resolution performance counter */
static LARGE_INTEGER hires_ticks_per_second;

static void
SDL_SetSystemTimerResolution(const UINT uPeriod)
{
#ifndef __WINRT__
    static UINT timer_period = 0;

    if (uPeriod != timer_period) {
        if (timer_period) {
            timeEndPeriod(timer_period);
        }

        timer_period = uPeriod;

        if (timer_period) {
            timeBeginPeriod(timer_period);
        }
    }
#endif
}

static void SDLCALL
SDL_TimerResolutionChanged(void *userdata, const char *name, const char *oldValue, const char *hint)
{
    UINT uPeriod;

    /* Unless the hint says otherwise, let's have good sleep precision */
    if (hint && *hint) {
        uPeriod = SDL_atoi(hint);
    } else {
        uPeriod = 1;
    }
    if (uPeriod || oldValue != hint) {
        SDL_SetSystemTimerResolution(uPeriod);
    }
}

void
SDL_TicksInit(void)
{
    if (ticks_started) {
        return;
    }
    ticks_started = SDL_TRUE;

    /* if we didn't set a precision, set it high. This affects lots of things
       on Windows besides the SDL timers, like audio callbacks, etc. */
    SDL_AddHintCallback(SDL_HINT_TIMER_RESOLUTION,
                        SDL_TimerResolutionChanged, NULL);

    /* Set first ticks value */
    /* QueryPerformanceCounter has had problems in the past, but lots of games
       use it, so we'll rely on it here.
     */
    if (QueryPerformanceFrequency(&hires_ticks_per_second) == TRUE) {
        hires_timer_available = TRUE;
        QueryPerformanceCounter(&hires_start_ticks);
    } else {
        hires_timer_available = FALSE;
#ifndef __WINRT__
        start = timeGetTime();
#endif /* __WINRT__ */
    }
}

void
SDL_TicksQuit(void)
{
    SDL_DelHintCallback(SDL_HINT_TIMER_RESOLUTION,
                        SDL_TimerResolutionChanged, NULL);

    SDL_SetSystemTimerResolution(0);  /* always release our timer resolution request. */

    start = 0;
    ticks_started = SDL_FALSE;
}

Uint32
SDL_GetTicks(void)
{
    DWORD now = 0;
    LARGE_INTEGER hires_now;

    if (!ticks_started) {
        SDL_TicksInit();
    }

    if (hires_timer_available) {
        QueryPerformanceCounter(&hires_now);

        hires_now.QuadPart -= hires_start_ticks.QuadPart;
        hires_now.QuadPart *= 1000;
        hires_now.QuadPart /= hires_ticks_per_second.QuadPart;

        return (DWORD) hires_now.QuadPart;
    } else {
#ifndef __WINRT__
        now = timeGetTime();
#endif /* __WINRT__ */
    }

    return (now - start);
}

Uint64
SDL_GetPerformanceCounter(void)
{
    LARGE_INTEGER counter;

    if (!QueryPerformanceCounter(&counter)) {
        return SDL_GetTicks();
    }
    return counter.QuadPart;
}

Uint64
SDL_GetPerformanceFrequency(void)
{
    LARGE_INTEGER frequency;

    if (!QueryPerformanceFrequency(&frequency)) {
        return 1000;
    }
    return frequency.QuadPart;
}

void
SDL_Delay(Uint32 ms)
{
    /* Sleep() is not publicly available to apps in early versions of WinRT.
     *
     * Visual C++ 2013 Update 4 re-introduced Sleep() for Windows 8.1 and
     * Windows Phone 8.1.
     *
     * Use the compiler version to determine availability.
     *
     * NOTE #1: _MSC_FULL_VER == 180030723 for Visual C++ 2013 Update 3.
     * NOTE #2: Visual C++ 2013, when compiling for Windows 8.0 and
     *    Windows Phone 8.0, uses the Visual C++ 2012 compiler to build
     *    apps and libraries.
     */
#if defined(__WINRT__) && defined(_MSC_FULL_VER) && (_MSC_FULL_VER <= 180030723)
    static HANDLE mutex = 0;
    if (!mutex) {
        mutex = CreateEventEx(0, 0, 0, EVENT_ALL_ACCESS);
    }
    WaitForSingleObjectEx(mutex, ms, FALSE);
#else
    if (!ticks_started) {
        SDL_TicksInit();
    }

    Sleep(ms);
#endif
}

#endif /* SDL_TIMER_WINDOWS */

/* vi: set ts=4 sw=4 expandtab: */
