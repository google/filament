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

#if SDL_TIMER_OS2

#include "SDL_timer.h"
#include "../../core/os2/SDL_os2.h"

#define INCL_DOSERRORS
#define INCL_DOSMISC
#define INCL_DOSPROFILE
#define INCL_DOSSEMAPHORES
#define INCL_DOSDATETIME
#define INCL_DOSPROCESS
#define INCL_DOSEXCEPTIONS
#include <os2.h>

/* No need to switch priorities in SDL_Delay() for OS/2 versions > Warp3 fp 42, */
/*#define _SWITCH_PRIORITY*/

typedef unsigned long long  ULLONG;

static ULONG    ulTmrFreq = 0;
static ULLONG   ullTmrStart;

void
SDL_TicksInit(void)
{
    ULONG   ulRC;

    ulRC = DosTmrQueryFreq(&ulTmrFreq);
    if (ulRC != NO_ERROR) {
        debug_os2("DosTmrQueryFreq() failed, rc = %u", ulRC);
    } else {
        ulRC = DosTmrQueryTime((PQWORD)&ullTmrStart);
        if (ulRC == NO_ERROR)
            return;
        debug_os2("DosTmrQueryTime() failed, rc = %u", ulRC);
    }

    ulTmrFreq = 0; /* Error - use DosQuerySysInfo() for timer. */
    DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT, (PULONG)&ullTmrStart, sizeof(ULONG));
}

void
SDL_TicksQuit(void)
{
}

Uint32
SDL_GetTicks(void)
{
    ULONG   ulResult;
    ULLONG  ullTmrNow;

    if (ulTmrFreq == 0) /* Was not initialized. */
        SDL_TicksInit();

    if (ulTmrFreq != 0) {
        DosTmrQueryTime((PQWORD)&ullTmrNow);
        ulResult = (ullTmrNow - ullTmrStart) * 1000 / ulTmrFreq;
    } else {
        DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT, (PULONG)&ullTmrNow, sizeof(ULONG));
        ulResult = (ULONG)ullTmrNow - (ULONG)ullTmrStart;
    }

    return ulResult;
}

Uint64
SDL_GetPerformanceCounter(void)
{
    QWORD   qwTmrNow;

    if (ulTmrFreq == 0 || (DosTmrQueryTime(&qwTmrNow) != NO_ERROR))
        return SDL_GetTicks();

    return *((Uint64 *)&qwTmrNow);
}

Uint64
SDL_GetPerformanceFrequency(void)
{
    return (ulTmrFreq == 0)? 1000 : (Uint64)ulTmrFreq;
}

void
SDL_Delay(Uint32 ms)
{
    HTIMER  hTimer = NULLHANDLE;
    ULONG   ulRC;
#ifdef _SWITCH_PRIORITY
    PPIB    pib;
    PTIB    tib;
    BOOL    fSetPriority = ms < 50;
    ULONG   ulSavePriority;
    ULONG   ulNesting;
#endif
    HEV     hevTimer;

    if (ms == 0) {
      DosSleep(0);
      return;
    }

    ulRC = DosCreateEventSem(NULL, &hevTimer, DC_SEM_SHARED, FALSE);
    if (ulRC != NO_ERROR) {
        debug_os2("DosAsyncTimer() failed, rc = %u", ulRC);
        DosSleep(ms);
        return;
    }

#ifdef _SWITCH_PRIORITY
    if (fSetPriority) {
        if (DosGetInfoBlocks(&tib, &pib) != NO_ERROR)
            fSetPriority = FALSE;
        else {
            ulSavePriority = tib->tib_ptib2->tib2_ulpri;
            if (((ulSavePriority & 0xFF00) == 0x0300) || /* already have high pr. */
                  (DosEnterMustComplete( &ulNesting) != NO_ERROR))
                fSetPriority = FALSE;
            else {
                DosSetPriority(PRTYS_THREAD, PRTYC_TIMECRITICAL, 0, 0);
            }
        }
    }
#endif

    DosResetEventSem(hevTimer, &ulRC);
    ulRC = DosAsyncTimer(ms, (HSEM)hevTimer, &hTimer);

#ifdef _SWITCH_PRIORITY
    if (fSetPriority) {
        if (DosSetPriority(PRTYS_THREAD, (ulSavePriority >> 8) & 0xFF, 0, 0) == NO_ERROR)
            DosSetPriority(PRTYS_THREAD, 0, ulSavePriority & 0xFF, 0);
        DosExitMustComplete(&ulNesting);
    }
#endif

    if (ulRC != NO_ERROR) {
        debug_os2("DosAsyncTimer() failed, rc = %u", ulRC);
    } else {
        DosWaitEventSem(hevTimer, SEM_INDEFINITE_WAIT);
    }

    if (ulRC != NO_ERROR)
        DosSleep(ms);

    DosCloseEventSem(hevTimer);
}

#endif /* SDL_TIMER_OS2 */

/* vi: set ts=4 sw=4 expandtab: */
