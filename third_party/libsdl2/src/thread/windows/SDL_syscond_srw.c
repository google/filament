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

#include "SDL_hints.h"
#include "SDL_thread.h"

#include "../generic/SDL_syscond_c.h"
#include "SDL_sysmutex_c.h"

typedef SDL_cond * (*pfnSDL_CreateCond)(void);
typedef void (*pfnSDL_DestroyCond)(SDL_cond *);
typedef int (*pfnSDL_CondSignal)(SDL_cond *);
typedef int (*pfnSDL_CondBroadcast)(SDL_cond *);
typedef int (*pfnSDL_CondWait)(SDL_cond *, SDL_mutex *);
typedef int (*pfnSDL_CondWaitTimeout)(SDL_cond *, SDL_mutex *, Uint32);

typedef struct SDL_cond_impl_t
{
    pfnSDL_CreateCond       Create;
    pfnSDL_DestroyCond      Destroy;
    pfnSDL_CondSignal       Signal;
    pfnSDL_CondBroadcast    Broadcast;
    pfnSDL_CondWait         Wait;
    pfnSDL_CondWaitTimeout  WaitTimeout;
} SDL_cond_impl_t;

/* Implementation will be chosen at runtime based on available Kernel features */
static SDL_cond_impl_t SDL_cond_impl_active = {0};


/**
 * Native Windows Condition Variable (SRW Locks)
 */

#ifndef CONDITION_VARIABLE_INIT
#define CONDITION_VARIABLE_INIT {0}
typedef struct CONDITION_VARIABLE {
    PVOID Ptr;
} CONDITION_VARIABLE, *PCONDITION_VARIABLE;
#endif

#if __WINRT__
#define pWakeConditionVariable WakeConditionVariable
#define pWakeAllConditionVariable WakeAllConditionVariable
#define pSleepConditionVariableSRW SleepConditionVariableSRW
#else
typedef VOID(WINAPI *pfnWakeConditionVariable)(PCONDITION_VARIABLE);
typedef VOID(WINAPI *pfnWakeAllConditionVariable)(PCONDITION_VARIABLE);
typedef BOOL(WINAPI *pfnSleepConditionVariableSRW)(PCONDITION_VARIABLE, PSRWLOCK, DWORD, ULONG);

static pfnWakeConditionVariable pWakeConditionVariable = NULL;
static pfnWakeAllConditionVariable pWakeAllConditionVariable = NULL;
static pfnSleepConditionVariableSRW pSleepConditionVariableSRW = NULL;
#endif

typedef struct SDL_cond_srw
{
    CONDITION_VARIABLE cond;
} SDL_cond_srw;


static SDL_cond *
SDL_CreateCond_srw(void)
{
    SDL_cond_srw *cond;

    /* Relies on CONDITION_VARIABLE_INIT == 0. */
    cond = (SDL_cond_srw *) SDL_calloc(1, sizeof(*cond));
    if (!cond) {
        SDL_OutOfMemory();
    }

    return (SDL_cond *)cond;
}

static void
SDL_DestroyCond_srw(SDL_cond * cond)
{
    if (cond) {
        /* There are no kernel allocated resources */
        SDL_free(cond);
    }
}

static int
SDL_CondSignal_srw(SDL_cond * _cond)
{
    SDL_cond_srw *cond = (SDL_cond_srw *)_cond;
    if (!cond) {
        return SDL_SetError("Passed a NULL condition variable");
    }

    pWakeConditionVariable(&cond->cond);

    return 0;
}

static int
SDL_CondBroadcast_srw(SDL_cond * _cond)
{
    SDL_cond_srw *cond = (SDL_cond_srw *)_cond;
    if (!cond) {
        return SDL_SetError("Passed a NULL condition variable");
    }

    pWakeAllConditionVariable(&cond->cond);

    return 0;
}

static int
SDL_CondWaitTimeout_srw(SDL_cond * _cond, SDL_mutex * _mutex, Uint32 ms)
{
    SDL_cond_srw *cond = (SDL_cond_srw *)_cond;
    SDL_mutex_srw *mutex = (SDL_mutex_srw *)_mutex;
    DWORD timeout;
    int ret;

    if (!cond) {
        return SDL_SetError("Passed a NULL condition variable");
    }
    if (!mutex) {
        return SDL_SetError("Passed a NULL mutex");
    }

    if (mutex->count != 1 || mutex->owner != GetCurrentThreadId()) {
        return SDL_SetError("Passed mutex is not locked or locked recursively");
    }

    if (ms == SDL_MUTEX_MAXWAIT) {
        timeout = INFINITE;
    } else {
        timeout = (DWORD) ms;
    }

    /* The mutex must be updated to the released state */
    mutex->count = 0;
    mutex->owner = 0;

    if (pSleepConditionVariableSRW(&cond->cond, &mutex->srw, timeout, 0) == FALSE) {
        if (GetLastError() == ERROR_TIMEOUT) {
            ret = SDL_MUTEX_TIMEDOUT;
        } else {
            ret = SDL_SetError("SleepConditionVariableSRW() failed");
        }
    } else {
        ret = 0;
    }

    /* The mutex is owned by us again, regardless of status of the wait */
    SDL_assert(mutex->count == 0 && mutex->owner == 0);
    mutex->count = 1;
    mutex->owner = GetCurrentThreadId();

    return ret;
}

static int
SDL_CondWait_srw(SDL_cond * cond, SDL_mutex * mutex) {
    return SDL_CondWaitTimeout_srw(cond, mutex, SDL_MUTEX_MAXWAIT);
}

static const SDL_cond_impl_t SDL_cond_impl_srw =
{
    &SDL_CreateCond_srw,
    &SDL_DestroyCond_srw,
    &SDL_CondSignal_srw,
    &SDL_CondBroadcast_srw,
    &SDL_CondWait_srw,
    &SDL_CondWaitTimeout_srw,
};

/**
 * Generic Condition Variable implementation using SDL_mutex and SDL_sem
 */

static const SDL_cond_impl_t SDL_cond_impl_generic =
{
    &SDL_CreateCond_generic,
    &SDL_DestroyCond_generic,
    &SDL_CondSignal_generic,
    &SDL_CondBroadcast_generic,
    &SDL_CondWait_generic,
    &SDL_CondWaitTimeout_generic,
};


SDL_cond *
SDL_CreateCond(void)
{
    if (SDL_cond_impl_active.Create == NULL) {
        /* Default to generic implementation, works with all mutex implementations */
        const SDL_cond_impl_t * impl = &SDL_cond_impl_generic;

        if (SDL_mutex_impl_active.Type == SDL_MUTEX_INVALID) {
            /* The mutex implementation isn't decided yet, trigger it */
            SDL_mutex *mutex = SDL_CreateMutex();
            if (!mutex) {
                return NULL;
            }
            SDL_DestroyMutex(mutex);

            SDL_assert(SDL_mutex_impl_active.Type != SDL_MUTEX_INVALID);
        }

        /* It is required SRW Locks are used */
        if (SDL_mutex_impl_active.Type == SDL_MUTEX_SRW) {
#if __WINRT__
            /* Link statically on this platform */
            impl = &SDL_cond_impl_srw;
#else
            HMODULE kernel32 = GetModuleHandle(TEXT("kernel32.dll"));
            if (kernel32) {
                pWakeConditionVariable = (pfnWakeConditionVariable) GetProcAddress(kernel32, "WakeConditionVariable");
                pWakeAllConditionVariable = (pfnWakeAllConditionVariable) GetProcAddress(kernel32, "WakeAllConditionVariable");
                pSleepConditionVariableSRW = (pfnSleepConditionVariableSRW) GetProcAddress(kernel32, "SleepConditionVariableSRW");
                if (pWakeConditionVariable && pWakeAllConditionVariable && pSleepConditionVariableSRW) {
                    /* Use the Windows provided API */
                    impl = &SDL_cond_impl_srw;
                }
            }
            if (!(kernel32 && pWakeConditionVariable && pWakeAllConditionVariable && pSleepConditionVariableSRW)) {
                SDL_LogWarn(SDL_LOG_CATEGORY_SYSTEM, "Could not load required imports for SRW Condition Variables although SRW Locks are used!");
            }
#endif
        }

        SDL_memcpy(&SDL_cond_impl_active, impl, sizeof(SDL_cond_impl_active));
    }
    return SDL_cond_impl_active.Create();
}

void
SDL_DestroyCond(SDL_cond * cond)
{
    SDL_cond_impl_active.Destroy(cond);
}

int
SDL_CondSignal(SDL_cond * cond)
{
    return SDL_cond_impl_active.Signal(cond);
}

int
SDL_CondBroadcast(SDL_cond * cond)
{
    return SDL_cond_impl_active.Broadcast(cond);
}

int
SDL_CondWaitTimeout(SDL_cond * cond, SDL_mutex * mutex, Uint32 ms)
{
    return SDL_cond_impl_active.WaitTimeout(cond, mutex, ms);
}

int
SDL_CondWait(SDL_cond * cond, SDL_mutex * mutex)
{
    return SDL_cond_impl_active.Wait(cond, mutex);
}

/* vi: set ts=4 sw=4 expandtab: */
