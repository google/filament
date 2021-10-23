/*
  Copyright (C) 2013 Apoorv Upreti <apoorvupreti@gmail.com>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/
/* Quits, hangs or crashes based on the command line options passed. */

#include <SDL.h>
#include <SDL_test.h>

static SDLTest_CommonState *state;
static int exit_code;
static SDL_bool hang;
static SDL_bool crash;

int
main(int argc, char** argv)
{
    int i, done;
    SDL_Event event;

    state = SDLTest_CommonCreateState(argv, SDL_INIT_VIDEO);
    if(!state)
        return 1;

    state->window_flags |= SDL_WINDOW_RESIZABLE;
    
    exit_code = 0;
    hang = SDL_FALSE;
    crash = SDL_FALSE;

    for(i = 1; i < argc; )
    {
        int consumed;
        consumed = SDLTest_CommonArg(state, i);
        if(consumed == 0)
        {
            consumed = -1;
            if(SDL_strcasecmp(argv[i], "--exit-code") == 0)
            {
                if(argv[i + 1])
                {
                    exit_code = SDL_atoi(argv[i + 1]);
                    consumed = 2;
                }
            }
            else if(SDL_strcasecmp(argv[i], "--hang") == 0)
            {
                hang = SDL_TRUE;
                consumed = 1;
            }
            else if(SDL_strcasecmp(argv[i], "--crash") == 0)
            {
                crash = SDL_TRUE;
                consumed = 1;
            }
        }

        if(consumed < 0)
        {
            static const char *options = { "[--exit-code N]", "[--crash]", "[--hang]", NULL };
            SDLTest_CommonLogUsage(state, argv[0], options);
            SDLTest_CommonQuit(state);
            return 1;
        }
        i += consumed;
    }

    if(!SDLTest_CommonInit(state))
    {
        SDLTest_CommonQuit(state);
        return 1;
    }

    /* infinite loop to hang the process */
    while(hang)
        SDL_Delay(10);

    /* dereference NULL pointer to crash process */
    if(crash)
    {
        int* p = NULL;
        *p = 5;
    }

    /* event loop */
    done = 0;
    while(!done)
    {
        while(SDL_PollEvent(&event))
            SDLTest_CommonEvent(state, &event, &done);
        SDL_Delay(10);
    }

    return exit_code;
}
