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
#include "../SDL_internal.h"

#include "SDL_hints.h"

/* General quit handling code for SDL */

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#include "SDL_events.h"
#include "SDL_events_c.h"

#if defined(HAVE_SIGNAL_H) || defined(HAVE_SIGACTION)
#define HAVE_SIGNAL_SUPPORT 1
#endif

#ifdef HAVE_SIGNAL_SUPPORT
static SDL_bool disable_signals = SDL_FALSE;
static SDL_bool send_quit_pending = SDL_FALSE;

#ifdef SDL_BACKGROUNDING_SIGNAL
static SDL_bool send_backgrounding_pending = SDL_FALSE;
#endif

#ifdef SDL_FOREGROUNDING_SIGNAL
static SDL_bool send_foregrounding_pending = SDL_FALSE;
#endif

static void
SDL_HandleSIG(int sig)
{
    /* Reset the signal handler */
    signal(sig, SDL_HandleSIG);

    /* Send a quit event next time the event loop pumps. */
    /* We can't send it in signal handler; malloc() might be interrupted! */
    if ((sig == SIGINT) || (sig == SIGTERM)) {
        send_quit_pending = SDL_TRUE;
    }

    #ifdef SDL_BACKGROUNDING_SIGNAL
    else if (sig == SDL_BACKGROUNDING_SIGNAL) {
        send_backgrounding_pending = SDL_TRUE;
    }
    #endif

    #ifdef SDL_FOREGROUNDING_SIGNAL
    else if (sig == SDL_FOREGROUNDING_SIGNAL) {
        send_foregrounding_pending = SDL_TRUE;
    }
    #endif
}

static void
SDL_EventSignal_Init(const int sig)
{
#ifdef HAVE_SIGACTION
    struct sigaction action;

    sigaction(sig, NULL, &action);
#ifdef HAVE_SA_SIGACTION
    if ( action.sa_handler == SIG_DFL && (void (*)(int))action.sa_sigaction == SIG_DFL ) {
#else
    if ( action.sa_handler == SIG_DFL ) {
#endif
        action.sa_handler = SDL_HandleSIG;
        sigaction(sig, &action, NULL);
    }
#elif HAVE_SIGNAL_H
    void (*ohandler) (int) = signal(sig, SDL_HandleSIG);
    if (ohandler != SIG_DFL) {
        signal(sig, ohandler);
    }
#endif
}

static void
SDL_EventSignal_Quit(const int sig)
{
#ifdef HAVE_SIGACTION
    struct sigaction action;
    sigaction(sig, NULL, &action);
    if ( action.sa_handler == SDL_HandleSIG ) {
        action.sa_handler = SIG_DFL;
        sigaction(sig, &action, NULL);
    }
#elif HAVE_SIGNAL_H
    void (*ohandler) (int) = signal(sig, SIG_DFL);
    if (ohandler != SDL_HandleSIG) {
        signal(sig, ohandler);
    }
#endif /* HAVE_SIGNAL_H */
}

/* Public functions */
static int
SDL_QuitInit_Internal(void)
{
    /* Both SIGINT and SIGTERM are translated into quit interrupts */
    /* and SDL can be built to simulate iOS/Android semantics with arbitrary signals. */
    SDL_EventSignal_Init(SIGINT);
    SDL_EventSignal_Init(SIGTERM);

    #ifdef SDL_BACKGROUNDING_SIGNAL
    SDL_EventSignal_Init(SDL_BACKGROUNDING_SIGNAL);
    #endif

    #ifdef SDL_FOREGROUNDING_SIGNAL
    SDL_EventSignal_Init(SDL_FOREGROUNDING_SIGNAL);
    #endif

    /* That's it! */
    return 0;
}

static void
SDL_QuitQuit_Internal(void)
{
    SDL_EventSignal_Quit(SIGINT);
    SDL_EventSignal_Quit(SIGTERM);

    #ifdef SDL_BACKGROUNDING_SIGNAL
    SDL_EventSignal_Quit(SDL_BACKGROUNDING_SIGNAL);
    #endif

    #ifdef SDL_FOREGROUNDING_SIGNAL
    SDL_EventSignal_Quit(SDL_FOREGROUNDING_SIGNAL);
    #endif
}
#endif

int
SDL_QuitInit(void)
{
#ifdef HAVE_SIGNAL_SUPPORT
    if (!SDL_GetHintBoolean(SDL_HINT_NO_SIGNAL_HANDLERS, SDL_FALSE)) {
        return SDL_QuitInit_Internal();
    }
#endif
    return 0;
}

void
SDL_QuitQuit(void)
{
#ifdef HAVE_SIGNAL_SUPPORT
    if (!disable_signals) {
        SDL_QuitQuit_Internal();
    }
#endif
}

void
SDL_SendPendingSignalEvents(void)
{
#ifdef HAVE_SIGNAL_SUPPORT
    if (send_quit_pending) {
        SDL_SendQuit();
        SDL_assert(!send_quit_pending);
    }

    #ifdef SDL_BACKGROUNDING_SIGNAL
    if (send_backgrounding_pending) {
        send_backgrounding_pending = SDL_FALSE;
        SDL_OnApplicationWillResignActive();
    }
    #endif

    #ifdef SDL_FOREGROUNDING_SIGNAL
    if (send_foregrounding_pending) {
        send_foregrounding_pending = SDL_FALSE;
        SDL_OnApplicationDidBecomeActive();
    }
    #endif
#endif
}

/* This function returns 1 if it's okay to close the application window */
int
SDL_SendQuit(void)
{
#ifdef HAVE_SIGNAL_SUPPORT
    send_quit_pending = SDL_FALSE;
#endif
    return SDL_SendAppEvent(SDL_QUIT);
}

/* vi: set ts=4 sw=4 expandtab: */
