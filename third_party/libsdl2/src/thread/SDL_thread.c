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

/* System independent thread management routines for SDL */

#include "SDL_assert.h"
#include "SDL_thread.h"
#include "SDL_thread_c.h"
#include "SDL_systhread.h"
#include "SDL_hints.h"
#include "../SDL_error_c.h"


SDL_TLSID
SDL_TLSCreate()
{
    static SDL_atomic_t SDL_tls_id;
    return SDL_AtomicIncRef(&SDL_tls_id)+1;
}

void *
SDL_TLSGet(SDL_TLSID id)
{
    SDL_TLSData *storage;

    storage = SDL_SYS_GetTLSData();
    if (!storage || id == 0 || id > storage->limit) {
        return NULL;
    }
    return storage->array[id-1].data;
}

int
SDL_TLSSet(SDL_TLSID id, const void *value, void (SDLCALL *destructor)(void *))
{
    SDL_TLSData *storage;

    if (id == 0) {
        return SDL_InvalidParamError("id");
    }

    storage = SDL_SYS_GetTLSData();
    if (!storage || (id > storage->limit)) {
        unsigned int i, oldlimit, newlimit;

        oldlimit = storage ? storage->limit : 0;
        newlimit = (id + TLS_ALLOC_CHUNKSIZE);
        storage = (SDL_TLSData *)SDL_realloc(storage, sizeof(*storage)+(newlimit-1)*sizeof(storage->array[0]));
        if (!storage) {
            return SDL_OutOfMemory();
        }
        storage->limit = newlimit;
        for (i = oldlimit; i < newlimit; ++i) {
            storage->array[i].data = NULL;
            storage->array[i].destructor = NULL;
        }
        if (SDL_SYS_SetTLSData(storage) != 0) {
            return -1;
        }
    }

    storage->array[id-1].data = SDL_const_cast(void*, value);
    storage->array[id-1].destructor = destructor;
    return 0;
}

static void
SDL_TLSCleanup()
{
    SDL_TLSData *storage;

    storage = SDL_SYS_GetTLSData();
    if (storage) {
        unsigned int i;
        for (i = 0; i < storage->limit; ++i) {
            if (storage->array[i].destructor) {
                storage->array[i].destructor(storage->array[i].data);
            }
        }
        SDL_SYS_SetTLSData(NULL);
        SDL_free(storage);
    }
}


/* This is a generic implementation of thread-local storage which doesn't
   require additional OS support.

   It is not especially efficient and doesn't clean up thread-local storage
   as threads exit.  If there is a real OS that doesn't support thread-local
   storage this implementation should be improved to be production quality.
*/

typedef struct SDL_TLSEntry {
    SDL_threadID thread;
    SDL_TLSData *storage;
    struct SDL_TLSEntry *next;
} SDL_TLSEntry;

static SDL_mutex *SDL_generic_TLS_mutex;
static SDL_TLSEntry *SDL_generic_TLS;


SDL_TLSData *
SDL_Generic_GetTLSData(void)
{
    SDL_threadID thread = SDL_ThreadID();
    SDL_TLSEntry *entry;
    SDL_TLSData *storage = NULL;

#if !SDL_THREADS_DISABLED
    if (!SDL_generic_TLS_mutex) {
        static SDL_SpinLock tls_lock;
        SDL_AtomicLock(&tls_lock);
        if (!SDL_generic_TLS_mutex) {
            SDL_mutex *mutex = SDL_CreateMutex();
            SDL_MemoryBarrierRelease();
            SDL_generic_TLS_mutex = mutex;
            if (!SDL_generic_TLS_mutex) {
                SDL_AtomicUnlock(&tls_lock);
                return NULL;
            }
        }
        SDL_AtomicUnlock(&tls_lock);
    }
#endif /* SDL_THREADS_DISABLED */

    SDL_MemoryBarrierAcquire();
    SDL_LockMutex(SDL_generic_TLS_mutex);
    for (entry = SDL_generic_TLS; entry; entry = entry->next) {
        if (entry->thread == thread) {
            storage = entry->storage;
            break;
        }
    }
#if !SDL_THREADS_DISABLED
    SDL_UnlockMutex(SDL_generic_TLS_mutex);
#endif

    return storage;
}

int
SDL_Generic_SetTLSData(SDL_TLSData *storage)
{
    SDL_threadID thread = SDL_ThreadID();
    SDL_TLSEntry *prev, *entry;

    /* SDL_Generic_GetTLSData() is always called first, so we can assume SDL_generic_TLS_mutex */
    SDL_LockMutex(SDL_generic_TLS_mutex);
    prev = NULL;
    for (entry = SDL_generic_TLS; entry; entry = entry->next) {
        if (entry->thread == thread) {
            if (storage) {
                entry->storage = storage;
            } else {
                if (prev) {
                    prev->next = entry->next;
                } else {
                    SDL_generic_TLS = entry->next;
                }
                SDL_free(entry);
            }
            break;
        }
        prev = entry;
    }
    if (!entry) {
        entry = (SDL_TLSEntry *)SDL_malloc(sizeof(*entry));
        if (entry) {
            entry->thread = thread;
            entry->storage = storage;
            entry->next = SDL_generic_TLS;
            SDL_generic_TLS = entry;
        }
    }
    SDL_UnlockMutex(SDL_generic_TLS_mutex);

    if (!entry) {
        return SDL_OutOfMemory();
    }
    return 0;
}

/* Routine to get the thread-specific error variable */
SDL_error *
SDL_GetErrBuf(void)
{
    static SDL_SpinLock tls_lock;
    static SDL_bool tls_being_created;
    static SDL_TLSID tls_errbuf;
    static SDL_error SDL_global_errbuf;
    const SDL_error *ALLOCATION_IN_PROGRESS = (SDL_error *)-1;
    SDL_error *errbuf;

    /* tls_being_created is there simply to prevent recursion if SDL_TLSCreate() fails.
       It also means it's possible for another thread to also use SDL_global_errbuf,
       but that's very unlikely and hopefully won't cause issues.
     */
    if (!tls_errbuf && !tls_being_created) {
        SDL_AtomicLock(&tls_lock);
        if (!tls_errbuf) {
            SDL_TLSID slot;
            tls_being_created = SDL_TRUE;
            slot = SDL_TLSCreate();
            tls_being_created = SDL_FALSE;
            SDL_MemoryBarrierRelease();
            tls_errbuf = slot;
        }
        SDL_AtomicUnlock(&tls_lock);
    }
    if (!tls_errbuf) {
        return &SDL_global_errbuf;
    }

    SDL_MemoryBarrierAcquire();
    errbuf = (SDL_error *)SDL_TLSGet(tls_errbuf);
    if (errbuf == ALLOCATION_IN_PROGRESS) {
        return &SDL_global_errbuf;
    }
    if (!errbuf) {
        /* Mark that we're in the middle of allocating our buffer */
        SDL_TLSSet(tls_errbuf, ALLOCATION_IN_PROGRESS, NULL);
        errbuf = (SDL_error *)SDL_malloc(sizeof(*errbuf));
        if (!errbuf) {
            SDL_TLSSet(tls_errbuf, NULL, NULL);
            return &SDL_global_errbuf;
        }
        SDL_zerop(errbuf);
        SDL_TLSSet(tls_errbuf, errbuf, SDL_free);
    }
    return errbuf;
}


/* Arguments and callback to setup and run the user thread function */
typedef struct
{
    int (SDLCALL * func) (void *);
    void *data;
    SDL_Thread *info;
    SDL_sem *wait;
} thread_args;

void
SDL_RunThread(void *data)
{
    thread_args *args = (thread_args *) data;
    int (SDLCALL * userfunc) (void *) = args->func;
    void *userdata = args->data;
    SDL_Thread *thread = args->info;
    int *statusloc = &thread->status;

    /* Perform any system-dependent setup - this function may not fail */
    SDL_SYS_SetupThread(thread->name);

    /* Get the thread id */
    thread->threadid = SDL_ThreadID();

    /* Wake up the parent thread */
    SDL_SemPost(args->wait);

    /* Run the function */
    *statusloc = userfunc(userdata);

    /* Clean up thread-local storage */
    SDL_TLSCleanup();

    /* Mark us as ready to be joined (or detached) */
    if (!SDL_AtomicCAS(&thread->state, SDL_THREAD_STATE_ALIVE, SDL_THREAD_STATE_ZOMBIE)) {
        /* Clean up if something already detached us. */
        if (SDL_AtomicCAS(&thread->state, SDL_THREAD_STATE_DETACHED, SDL_THREAD_STATE_CLEANED)) {
            if (thread->name) {
                SDL_free(thread->name);
            }
            SDL_free(thread);
        }
    }
}

#ifdef SDL_CreateThread
#undef SDL_CreateThread
#endif
#if SDL_DYNAMIC_API
#define SDL_CreateThread SDL_CreateThread_REAL
#endif

#ifdef SDL_PASSED_BEGINTHREAD_ENDTHREAD
static SDL_Thread *
SDL_CreateThreadWithStackSize(int (SDLCALL * fn) (void *),
                 const char *name, const size_t stacksize, void *data,
                 pfnSDL_CurrentBeginThread pfnBeginThread,
                 pfnSDL_CurrentEndThread pfnEndThread)
#else
static SDL_Thread *
SDL_CreateThreadWithStackSize(int (SDLCALL * fn) (void *),
                const char *name, const size_t stacksize, void *data)
#endif
{
    SDL_Thread *thread;
    thread_args *args;
    int ret;

    /* Allocate memory for the thread info structure */
    thread = (SDL_Thread *) SDL_malloc(sizeof(*thread));
    if (thread == NULL) {
        SDL_OutOfMemory();
        return (NULL);
    }
    SDL_zerop(thread);
    thread->status = -1;
    SDL_AtomicSet(&thread->state, SDL_THREAD_STATE_ALIVE);

    /* Set up the arguments for the thread */
    if (name != NULL) {
        thread->name = SDL_strdup(name);
        if (thread->name == NULL) {
            SDL_OutOfMemory();
            SDL_free(thread);
            return (NULL);
        }
    }

    /* Set up the arguments for the thread */
    args = (thread_args *) SDL_malloc(sizeof(*args));
    if (args == NULL) {
        SDL_OutOfMemory();
        if (thread->name) {
            SDL_free(thread->name);
        }
        SDL_free(thread);
        return (NULL);
    }
    args->func = fn;
    args->data = data;
    args->info = thread;
    args->wait = SDL_CreateSemaphore(0);
    if (args->wait == NULL) {
        if (thread->name) {
            SDL_free(thread->name);
        }
        SDL_free(thread);
        SDL_free(args);
        return (NULL);
    }

    thread->stacksize = stacksize;

    /* Create the thread and go! */
#ifdef SDL_PASSED_BEGINTHREAD_ENDTHREAD
    ret = SDL_SYS_CreateThread(thread, args, pfnBeginThread, pfnEndThread);
#else
    ret = SDL_SYS_CreateThread(thread, args);
#endif
    if (ret >= 0) {
        /* Wait for the thread function to use arguments */
        SDL_SemWait(args->wait);
    } else {
        /* Oops, failed.  Gotta free everything */
        if (thread->name) {
            SDL_free(thread->name);
        }
        SDL_free(thread);
        thread = NULL;
    }
    SDL_DestroySemaphore(args->wait);
    SDL_free(args);

    /* Everything is running now */
    return (thread);
}

#ifdef SDL_PASSED_BEGINTHREAD_ENDTHREAD
DECLSPEC SDL_Thread *SDLCALL
SDL_CreateThread(int (SDLCALL * fn) (void *),
                 const char *name, void *data,
                 pfnSDL_CurrentBeginThread pfnBeginThread,
                 pfnSDL_CurrentEndThread pfnEndThread)
#else
DECLSPEC SDL_Thread *SDLCALL
SDL_CreateThread(int (SDLCALL * fn) (void *),
                 const char *name, void *data)
#endif
{
    /* !!! FIXME: in 2.1, just make stackhint part of the usual API. */
    const char *stackhint = SDL_GetHint(SDL_HINT_THREAD_STACK_SIZE);
    size_t stacksize = 0;

    /* If the SDL_HINT_THREAD_STACK_SIZE exists, use it */
    if (stackhint != NULL) {
        char *endp = NULL;
        const Sint64 hintval = SDL_strtoll(stackhint, &endp, 10);
        if ((*stackhint != '\0') && (*endp == '\0')) {  /* a valid number? */
            if (hintval > 0) {  /* reject bogus values. */
                stacksize = (size_t) hintval;
            }
        }
    }

#ifdef SDL_PASSED_BEGINTHREAD_ENDTHREAD
    return SDL_CreateThreadWithStackSize(fn, name, stacksize, data, pfnBeginThread, pfnEndThread);
#else
    return SDL_CreateThreadWithStackSize(fn, name, stacksize, data);
#endif
}

SDL_Thread *
SDL_CreateThreadInternal(int (SDLCALL * fn) (void *), const char *name,
                         const size_t stacksize, void *data) {
#ifdef SDL_PASSED_BEGINTHREAD_ENDTHREAD
    return SDL_CreateThreadWithStackSize(fn, name, stacksize, data, NULL, NULL);
#else
    return SDL_CreateThreadWithStackSize(fn, name, stacksize, data);
#endif
}

SDL_threadID
SDL_GetThreadID(SDL_Thread * thread)
{
    SDL_threadID id;

    if (thread) {
        id = thread->threadid;
    } else {
        id = SDL_ThreadID();
    }
    return id;
}

const char *
SDL_GetThreadName(SDL_Thread * thread)
{
    if (thread) {
        return thread->name;
    } else {
        return NULL;
    }
}

int
SDL_SetThreadPriority(SDL_ThreadPriority priority)
{
    return SDL_SYS_SetThreadPriority(priority);
}

void
SDL_WaitThread(SDL_Thread * thread, int *status)
{
    if (thread) {
        SDL_SYS_WaitThread(thread);
        if (status) {
            *status = thread->status;
        }
        if (thread->name) {
            SDL_free(thread->name);
        }
        SDL_free(thread);
    }
}

void
SDL_DetachThread(SDL_Thread * thread)
{
    if (!thread) {
        return;
    }

    /* Grab dibs if the state is alive+joinable. */
    if (SDL_AtomicCAS(&thread->state, SDL_THREAD_STATE_ALIVE, SDL_THREAD_STATE_DETACHED)) {
        SDL_SYS_DetachThread(thread);
    } else {
        /* all other states are pretty final, see where we landed. */
        const int thread_state = SDL_AtomicGet(&thread->state);
        if ((thread_state == SDL_THREAD_STATE_DETACHED) || (thread_state == SDL_THREAD_STATE_CLEANED)) {
            return;  /* already detached (you shouldn't call this twice!) */
        } else if (thread_state == SDL_THREAD_STATE_ZOMBIE) {
            SDL_WaitThread(thread, NULL);  /* already done, clean it up. */
        } else {
            SDL_assert(0 && "Unexpected thread state");
        }
    }
}

/* vi: set ts=4 sw=4 expandtab: */
