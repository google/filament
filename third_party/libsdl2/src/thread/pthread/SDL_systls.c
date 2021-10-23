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
#include "SDL_thread.h"
#include "../SDL_systhread.h"
#include "../SDL_thread_c.h"

#include <pthread.h>


#define INVALID_PTHREAD_KEY ((pthread_key_t)-1)

static pthread_key_t thread_local_storage = INVALID_PTHREAD_KEY;
static SDL_bool generic_local_storage = SDL_FALSE;

SDL_TLSData *
SDL_SYS_GetTLSData(void)
{
    if (thread_local_storage == INVALID_PTHREAD_KEY && !generic_local_storage) {
        static SDL_SpinLock lock;
        SDL_AtomicLock(&lock);
        if (thread_local_storage == INVALID_PTHREAD_KEY && !generic_local_storage) {
            pthread_key_t storage;
            if (pthread_key_create(&storage, NULL) == 0) {
                SDL_MemoryBarrierRelease();
                thread_local_storage = storage;
            } else {
                generic_local_storage = SDL_TRUE;
            }
        }
        SDL_AtomicUnlock(&lock);
    }
    if (generic_local_storage) {
        return SDL_Generic_GetTLSData();
    }
    SDL_MemoryBarrierAcquire();
    return (SDL_TLSData *)pthread_getspecific(thread_local_storage);
}

int
SDL_SYS_SetTLSData(SDL_TLSData *data)
{
    if (generic_local_storage) {
        return SDL_Generic_SetTLSData(data);
    }
    if (pthread_setspecific(thread_local_storage, data) != 0) {
        return SDL_SetError("pthread_setspecific() failed");
    }
    return 0;
}

/* vi: set ts=4 sw=4 expandtab: */
