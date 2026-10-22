/*
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

/* Test the thread and mutex locking functions
   Also exercises the system's signal/thread interaction
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h> /* for atexit() */

#include "SDL.h"

static SDL_mutex *mutex = NULL;
static SDL_threadID mainthread;
static SDL_Thread *threads[6];
static SDL_atomic_t doterminate;

/*
 * SDL_Quit() shouldn't be used with atexit() directly because
 *  calling conventions may differ...
 */
static void
SDL_Quit_Wrapper(void)
{
    SDL_Quit();
}

void
printid(void)
{
    SDL_Log("Process %lu:  exiting\n", SDL_ThreadID());
}

void
terminate(int sig)
{
    signal(SIGINT, terminate);
    SDL_AtomicSet(&doterminate, 1);
}

void
closemutex(int sig)
{
    SDL_threadID id = SDL_ThreadID();
    int i;
    SDL_Log("Process %lu:  Cleaning up...\n", id == mainthread ? 0 : id);
    SDL_AtomicSet(&doterminate, 1);
    for (i = 0; i < 6; ++i)
        SDL_WaitThread(threads[i], NULL);
    SDL_DestroyMutex(mutex);
    exit(sig);
}

int SDLCALL
Run(void *data)
{
    if (SDL_ThreadID() == mainthread)
        signal(SIGTERM, closemutex);
    while (!SDL_AtomicGet(&doterminate)) {
        SDL_Log("Process %lu ready to work\n", SDL_ThreadID());
        if (SDL_LockMutex(mutex) < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't lock mutex: %s", SDL_GetError());
            exit(1);
        }
        SDL_Log("Process %lu, working!\n", SDL_ThreadID());
        SDL_Delay(1 * 1000);
        SDL_Log("Process %lu, done!\n", SDL_ThreadID());
        if (SDL_UnlockMutex(mutex) < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't unlock mutex: %s", SDL_GetError());
            exit(1);
        }
        /* If this sleep isn't done, then threads may starve */
        SDL_Delay(10);
    }
    if (SDL_ThreadID() == mainthread && SDL_AtomicGet(&doterminate)) {
        SDL_Log("Process %lu:  raising SIGTERM\n", SDL_ThreadID());
        raise(SIGTERM);
    }
    return (0);
}

int
main(int argc, char *argv[])
{
    int i;
    int maxproc = 6;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    /* Load the SDL library */
    if (SDL_Init(0) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit_Wrapper);

    SDL_AtomicSet(&doterminate, 0);

    if ((mutex = SDL_CreateMutex()) == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create mutex: %s\n", SDL_GetError());
        exit(1);
    }

    mainthread = SDL_ThreadID();
    SDL_Log("Main thread: %lu\n", mainthread);
    atexit(printid);
    for (i = 0; i < maxproc; ++i) {
        char name[64];
        SDL_snprintf(name, sizeof (name), "Worker%d", i);
        if ((threads[i] = SDL_CreateThread(Run, name, NULL)) == NULL)
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create thread!\n");
    }
    signal(SIGINT, terminate);
    Run(NULL);

    return (0);                 /* Never reached */
}
