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

#if SDL_THREAD_WINDOWS

#include "../../core/windows/SDL_windows.h"

#include "SDL_thread.h"
#include "../SDL_thread_c.h"

#if WINAPI_FAMILY_WINRT
#include <fibersapi.h>

#ifndef TLS_OUT_OF_INDEXES
#define TLS_OUT_OF_INDEXES  FLS_OUT_OF_INDEXES
#endif

#define TlsAlloc()  FlsAlloc(NULL)
#define TlsSetValue FlsSetValue
#define TlsGetValue FlsGetValue
#endif

static DWORD thread_local_storage = TLS_OUT_OF_INDEXES;
static SDL_bool generic_local_storage = SDL_FALSE;

SDL_TLSData *
SDL_SYS_GetTLSData(void)
{
    if (thread_local_storage == TLS_OUT_OF_INDEXES && !generic_local_storage) {
        static SDL_SpinLock lock;
        SDL_AtomicLock(&lock);
        if (thread_local_storage == TLS_OUT_OF_INDEXES && !generic_local_storage) {
            DWORD storage = TlsAlloc();
            if (storage != TLS_OUT_OF_INDEXES) {
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
    return (SDL_TLSData *)TlsGetValue(thread_local_storage);
}

int
SDL_SYS_SetTLSData(SDL_TLSData *data)
{
    if (generic_local_storage) {
        return SDL_Generic_SetTLSData(data);
    }
    if (!TlsSetValue(thread_local_storage, data)) {
        return SDL_SetError("TlsSetValue() failed");
    }
    return 0;
}

#endif /* SDL_THREAD_WINDOWS */

/* vi: set ts=4 sw=4 expandtab: */
