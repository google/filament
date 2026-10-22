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

#if defined(__WIN32__) || defined(__WINRT__)
#include "../core/windows/SDL_windows.h"
#endif

#include "SDL_atomic.h"
#include "SDL_mutex.h"
#include "SDL_timer.h"

#if !defined(HAVE_GCC_ATOMICS) && defined(__SOLARIS__)
#include <atomic.h>
#endif

#if defined(__WATCOMC__) && defined(__386__)
SDL_COMPILE_TIME_ASSERT(locksize, 4==sizeof(SDL_SpinLock));
extern _inline int _SDL_xchg_watcom(volatile int *a, int v);
#pragma aux _SDL_xchg_watcom = \
  "xchg [ecx], eax" \
  parm [ecx] [eax] \
  value [eax] \
  modify exact [eax];
#endif /* __WATCOMC__ && __386__ */

/* This function is where all the magic happens... */
SDL_bool
SDL_AtomicTryLock(SDL_SpinLock *lock)
{
#if SDL_ATOMIC_DISABLED
    /* Terrible terrible damage */
    static SDL_mutex *_spinlock_mutex;

    if (!_spinlock_mutex) {
        /* Race condition on first lock... */
        _spinlock_mutex = SDL_CreateMutex();
    }
    SDL_LockMutex(_spinlock_mutex);
    if (*lock == 0) {
        *lock = 1;
        SDL_UnlockMutex(_spinlock_mutex);
        return SDL_TRUE;
    } else {
        SDL_UnlockMutex(_spinlock_mutex);
        return SDL_FALSE;
    }

#elif defined(_MSC_VER)
    SDL_COMPILE_TIME_ASSERT(locksize, sizeof(*lock) == sizeof(long));
    return (InterlockedExchange((long*)lock, 1) == 0);

#elif defined(__WATCOMC__) && defined(__386__)
    return _SDL_xchg_watcom(lock, 1) == 0;

#elif HAVE_GCC_ATOMICS || HAVE_GCC_SYNC_LOCK_TEST_AND_SET
    return (__sync_lock_test_and_set(lock, 1) == 0);

#elif defined(__GNUC__) && defined(__arm__) && \
        (defined(__ARM_ARCH_4__) || defined(__ARM_ARCH_4T__) || \
         defined(__ARM_ARCH_5__) || defined(__ARM_ARCH_5TE__) || \
         defined(__ARM_ARCH_5TEJ__))
    int result;
    __asm__ __volatile__ (
        "swp %0, %1, [%2]\n"
        : "=&r,&r" (result) : "r,0" (1), "r,r" (lock) : "memory");
    return (result == 0);

#elif defined(__GNUC__) && defined(__arm__)
    int result;
    __asm__ __volatile__ (
        "ldrex %0, [%2]\nteq   %0, #0\nstrexeq %0, %1, [%2]"
        : "=&r" (result) : "r" (1), "r" (lock) : "cc", "memory");
    return (result == 0);

#elif defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
    int result;
    __asm__ __volatile__(
        "lock ; xchgl %0, (%1)\n"
        : "=r" (result) : "r" (lock), "0" (1) : "cc", "memory");
    return (result == 0);

#elif defined(__MACOSX__) || defined(__IPHONEOS__)
    /* Maybe used for PowerPC, but the Intel asm or gcc atomics are favored. */
    return OSAtomicCompareAndSwap32Barrier(0, 1, lock);

#elif defined(__SOLARIS__) && defined(_LP64)
    /* Used for Solaris with non-gcc compilers. */
    return (SDL_bool) ((int) atomic_cas_64((volatile uint64_t*)lock, 0, 1) == 0);

#elif defined(__SOLARIS__) && !defined(_LP64)
    /* Used for Solaris with non-gcc compilers. */
    return (SDL_bool) ((int) atomic_cas_32((volatile uint32_t*)lock, 0, 1) == 0);

#else
#error Please implement for your platform.
    return SDL_FALSE;
#endif
}

void
SDL_AtomicLock(SDL_SpinLock *lock)
{
    /* FIXME: Should we have an eventual timeout? */
    while (!SDL_AtomicTryLock(lock)) {
        SDL_Delay(0);
    }
}

void
SDL_AtomicUnlock(SDL_SpinLock *lock)
{
#if defined(_MSC_VER)
    _ReadWriteBarrier();
    *lock = 0;

#elif defined(__WATCOMC__) && defined(__386__)
    SDL_CompilerBarrier ();
    *lock = 0;

#elif HAVE_GCC_ATOMICS || HAVE_GCC_SYNC_LOCK_TEST_AND_SET
    __sync_lock_release(lock);

#elif defined(__SOLARIS__)
    /* Used for Solaris when not using gcc. */
    *lock = 0;
    membar_producer();

#else
    *lock = 0;
#endif
}

/* vi: set ts=4 sw=4 expandtab: */
