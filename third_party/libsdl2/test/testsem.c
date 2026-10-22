/*
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

/* Simple test of the SDL semaphore code */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "SDL.h"

#define NUM_THREADS 10

static SDL_sem *sem;
int alive = 1;

int SDLCALL
ThreadFunc(void *data)
{
    int threadnum = (int) (uintptr_t) data;
    while (alive) {
        SDL_SemWait(sem);
        SDL_Log("Thread number %d has got the semaphore (value = %d)!\n",
                threadnum, SDL_SemValue(sem));
        SDL_Delay(200);
        SDL_SemPost(sem);
        SDL_Log("Thread number %d has released the semaphore (value = %d)!\n",
                threadnum, SDL_SemValue(sem));
        SDL_Delay(1);           /* For the scheduler */
    }
    SDL_Log("Thread number %d exiting.\n", threadnum);
    return 0;
}

static void
killed(int sig)
{
    alive = 0;
}

static void
TestWaitTimeout(void)
{
    Uint32 start_ticks;
    Uint32 end_ticks;
    Uint32 duration;
    int retval;

    sem = SDL_CreateSemaphore(0);
    SDL_Log("Waiting 2 seconds on semaphore\n");

    start_ticks = SDL_GetTicks();
    retval = SDL_SemWaitTimeout(sem, 2000);
    end_ticks = SDL_GetTicks();

    duration = end_ticks - start_ticks;

    /* Accept a little offset in the effective wait */
    if (duration > 1900 && duration < 2050)
        SDL_Log("Wait done.\n");
    else
        SDL_Log("Wait took %d milliseconds\n", duration);

    /* Check to make sure the return value indicates timed out */
    if (retval != SDL_MUTEX_TIMEDOUT)
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_SemWaitTimeout returned: %d; expected: %d\n", retval, SDL_MUTEX_TIMEDOUT);
}

int
main(int argc, char **argv)
{
    SDL_Thread *threads[NUM_THREADS];
    uintptr_t i;
    int init_sem;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    if (argc < 2) {
        SDL_Log("Usage: %s init_value\n", argv[0]);
        return (1);
    }

    /* Load the SDL library */
    if (SDL_Init(0) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s\n", SDL_GetError());
        return (1);
    }
    signal(SIGTERM, killed);
    signal(SIGINT, killed);

    init_sem = atoi(argv[1]);
    sem = SDL_CreateSemaphore(init_sem);

    SDL_Log("Running %d threads, semaphore value = %d\n", NUM_THREADS,
           init_sem);
    /* Create all the threads */
    for (i = 0; i < NUM_THREADS; ++i) {
        char name[64];
        SDL_snprintf(name, sizeof (name), "Thread%u", (unsigned int) i);
        threads[i] = SDL_CreateThread(ThreadFunc, name, (void *) i);
    }

    /* Wait 10 seconds */
    SDL_Delay(10 * 1000);

    /* Wait for all threads to finish */
    SDL_Log("Waiting for threads to finish\n");
    alive = 0;
    for (i = 0; i < NUM_THREADS; ++i) {
        SDL_WaitThread(threads[i], NULL);
    }
    SDL_Log("Finished waiting for threads\n");

    SDL_DestroySemaphore(sem);

    TestWaitTimeout();

    SDL_Quit();
    return (0);
}
