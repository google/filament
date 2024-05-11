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
#include "../../SDL_internal.h"

#if SDL_THREAD_WINDOWS

/* Mutex functions using the Win32 API */

#include "../../core/windows/SDL_windows.h"

#include "SDL_mutex.h"


struct SDL_mutex
{
    CRITICAL_SECTION cs;
};

/* Create a mutex */
SDL_mutex *
SDL_CreateMutex(void)
{
    SDL_mutex *mutex;

    /* Allocate mutex memory */
    mutex = (SDL_mutex *) SDL_malloc(sizeof(*mutex));
    if (mutex) {
        /* Initialize */
        /* On SMP systems, a non-zero spin count generally helps performance */
#if __WINRT__
        InitializeCriticalSectionEx(&mutex->cs, 2000, 0);
#else
        InitializeCriticalSectionAndSpinCount(&mutex->cs, 2000);
#endif
    } else {
        SDL_OutOfMemory();
    }
    return (mutex);
}

/* Free the mutex */
void
SDL_DestroyMutex(SDL_mutex * mutex)
{
    if (mutex) {
        DeleteCriticalSection(&mutex->cs);
        SDL_free(mutex);
    }
}

/* Lock the mutex */
int
SDL_LockMutex(SDL_mutex * mutex)
{
    if (mutex == NULL) {
        return SDL_SetError("Passed a NULL mutex");
    }

    EnterCriticalSection(&mutex->cs);
    return (0);
}

/* TryLock the mutex */
int
SDL_TryLockMutex(SDL_mutex * mutex)
{
    int retval = 0;
    if (mutex == NULL) {
        return SDL_SetError("Passed a NULL mutex");
    }

    if (TryEnterCriticalSection(&mutex->cs) == 0) {
        retval = SDL_MUTEX_TIMEDOUT;
    }
    return retval;
}

/* Unlock the mutex */
int
SDL_UnlockMutex(SDL_mutex * mutex)
{
    if (mutex == NULL) {
        return SDL_SetError("Passed a NULL mutex");
    }

    LeaveCriticalSection(&mutex->cs);
    return (0);
}

#endif /* SDL_THREAD_WINDOWS */

/* vi: set ts=4 sw=4 expandtab: */
