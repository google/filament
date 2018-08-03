/*
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

/* Simple test of the SDL threading code */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "SDL.h"

#define NUMTHREADS 10

static SDL_atomic_t time_for_threads_to_die[NUMTHREADS];

/* Call this instead of exit(), so we can clean up SDL: atexit() is evil. */
static void
quit(int rc)
{
    SDL_Quit();
    exit(rc);
}

int SDLCALL
SubThreadFunc(void *data)
{
    while (!*(int volatile *) data) {
        ;                       /* SDL_Delay(10); *//* do nothing */
    }
    return 0;
}

int SDLCALL
ThreadFunc(void *data)
{
    SDL_Thread *sub_threads[NUMTHREADS];
    int flags[NUMTHREADS];
    int i;
    int tid = (int) (uintptr_t) data;

    SDL_Log("Creating Thread %d\n", tid);

    for (i = 0; i < NUMTHREADS; i++) {
        char name[64];
        SDL_snprintf(name, sizeof (name), "Child%d_%d", tid, i);
        flags[i] = 0;
        sub_threads[i] = SDL_CreateThread(SubThreadFunc, name, &flags[i]);
    }

    SDL_Log("Thread '%d' waiting for signal\n", tid);
    while (SDL_AtomicGet(&time_for_threads_to_die[tid]) != 1) {
        ;                       /* do nothing */
    }

    SDL_Log("Thread '%d' sending signals to subthreads\n", tid);
    for (i = 0; i < NUMTHREADS; i++) {
        flags[i] = 1;
        SDL_WaitThread(sub_threads[i], NULL);
    }

    SDL_Log("Thread '%d' exiting!\n", tid);

    return 0;
}

int
main(int argc, char *argv[])
{
    SDL_Thread *threads[NUMTHREADS];
    int i;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    /* Load the SDL library */
    if (SDL_Init(0) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s\n", SDL_GetError());
        return (1);
    }

    signal(SIGSEGV, SIG_DFL);
    for (i = 0; i < NUMTHREADS; i++) {
        char name[64];
        SDL_snprintf(name, sizeof (name), "Parent%d", i);
        SDL_AtomicSet(&time_for_threads_to_die[i], 0);
        threads[i] = SDL_CreateThread(ThreadFunc, name, (void*) (uintptr_t) i);

        if (threads[i] == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create thread: %s\n", SDL_GetError());
            quit(1);
        }
    }

    for (i = 0; i < NUMTHREADS; i++) {
        SDL_AtomicSet(&time_for_threads_to_die[i], 1);
    }

    for (i = 0; i < NUMTHREADS; i++) {
        SDL_WaitThread(threads[i], NULL);
    }
    SDL_Quit();
    return (0);
}
