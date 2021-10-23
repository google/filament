/*
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

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

#include "SDL.h"

static SDL_TLSID tls;
static int alive = 0;
static int testprio = 0;

/* Call this instead of exit(), so we can clean up SDL: atexit() is evil. */
static void
quit(int rc)
{
    SDL_Quit();
    exit(rc);
}

static const char *
getprioritystr(SDL_ThreadPriority priority)
{
    switch(priority)
    {
    case SDL_THREAD_PRIORITY_LOW: return "SDL_THREAD_PRIORITY_LOW";
    case SDL_THREAD_PRIORITY_NORMAL: return "SDL_THREAD_PRIORITY_NORMAL";
    case SDL_THREAD_PRIORITY_HIGH: return "SDL_THREAD_PRIORITY_HIGH";
    case SDL_THREAD_PRIORITY_TIME_CRITICAL: return "SDL_THREAD_PRIORITY_TIME_CRITICAL";
    }

    return "???";
}

int SDLCALL
ThreadFunc(void *data)
{
    SDL_ThreadPriority prio = SDL_THREAD_PRIORITY_NORMAL;

    SDL_TLSSet(tls, "baby thread", NULL);
    SDL_Log("Started thread %s: My thread id is %lu, thread data = %s\n",
           (char *) data, SDL_ThreadID(), (const char *)SDL_TLSGet(tls));
    while (alive) {
        SDL_Log("Thread '%s' is alive!\n", (char *) data);

        if (testprio) {
            SDL_Log("SDL_SetThreadPriority(%s):%d\n", getprioritystr(prio), SDL_SetThreadPriority(prio));
            if (++prio > SDL_THREAD_PRIORITY_TIME_CRITICAL)
                prio = SDL_THREAD_PRIORITY_LOW;
        }

        SDL_Delay(1 * 1000);
    }
    SDL_Log("Thread '%s' exiting!\n", (char *) data);
    return (0);
}

static void
killed(int sig)
{
    SDL_Log("Killed with SIGTERM, waiting 5 seconds to exit\n");
    SDL_Delay(5 * 1000);
    alive = 0;
    quit(0);
}

int
main(int argc, char *argv[])
{
    int arg = 1;
    SDL_Thread *thread;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    /* Load the SDL library */
    if (SDL_Init(0) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s\n", SDL_GetError());
        return (1);
    }

    while (argv[arg] && *argv[arg] == '-') {
        if (SDL_strcmp(argv[arg], "--prio") == 0) {
            testprio = 1;
        }
        ++arg;
    }

    tls = SDL_TLSCreate();
    SDL_assert(tls);
    SDL_TLSSet(tls, "main thread", NULL);
    SDL_Log("Main thread data initially: %s\n", (const char *)SDL_TLSGet(tls));

    alive = 1;
    thread = SDL_CreateThread(ThreadFunc, "One", "#1");
    if (thread == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create thread: %s\n", SDL_GetError());
        quit(1);
    }
    SDL_Delay(5 * 1000);
    SDL_Log("Waiting for thread #1\n");
    alive = 0;
    SDL_WaitThread(thread, NULL);

    SDL_Log("Main thread data finally: %s\n", (const char *)SDL_TLSGet(tls));

    alive = 1;
    signal(SIGTERM, killed);
    thread = SDL_CreateThread(ThreadFunc, "Two", "#2");
    if (thread == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create thread: %s\n", SDL_GetError());
        quit(1);
    }
    raise(SIGTERM);

    SDL_Quit();                 /* Never reached */
    return (0);                 /* Never reached */
}
