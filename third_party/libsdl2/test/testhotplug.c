/*
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

/* Simple program to test the SDL joystick hotplugging */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL.h"

#if !defined SDL_JOYSTICK_DISABLED && !defined SDL_HAPTIC_DISABLED

int
main(int argc, char *argv[])
{
    SDL_Joystick *joystick = NULL;
    SDL_Haptic *haptic = NULL;
    SDL_JoystickID instance = -1;
    SDL_bool keepGoing = SDL_TRUE;
    int i;
    SDL_bool enable_haptic = SDL_TRUE;
    Uint32 init_subsystems = SDL_INIT_VIDEO | SDL_INIT_JOYSTICK;
    
    for (i = 1; i < argc; ++i) {
        if (SDL_strcasecmp(argv[i], "--nohaptic") == 0) {
            enable_haptic = SDL_FALSE;
        }
    }

    if(enable_haptic) {
        init_subsystems |= SDL_INIT_HAPTIC;
    }
    
    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

    /* Initialize SDL (Note: video is required to start event loop) */
    if (SDL_Init(init_subsystems) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    /*
    //SDL_CreateWindow("Dummy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 128, 128, 0);
    */

    SDL_Log("There are %d joysticks at startup\n", SDL_NumJoysticks());
    if (enable_haptic)
        SDL_Log("There are %d haptic devices at startup\n", SDL_NumHaptics());

    while(keepGoing)
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    keepGoing = SDL_FALSE;
                    break;
                case SDL_JOYDEVICEADDED:
                    if (joystick != NULL)
                    {
                        SDL_Log("Only one joystick supported by this test\n");
                    }
                    else
                    {
                        joystick = SDL_JoystickOpen(event.jdevice.which);
                        instance = SDL_JoystickInstanceID(joystick);
                        SDL_Log("Joy Added  : %d : %s\n", event.jdevice.which, SDL_JoystickName(joystick));
                        if (enable_haptic)
                        {
                            if (SDL_JoystickIsHaptic(joystick))
                            {
                                haptic = SDL_HapticOpenFromJoystick(joystick);
                                if (haptic)
                                {
                                    SDL_Log("Joy Haptic Opened\n");
                                    if (SDL_HapticRumbleInit( haptic ) != 0)
                                    {
                                        SDL_Log("Could not init Rumble!: %s\n", SDL_GetError());
                                        SDL_HapticClose(haptic);
                                        haptic = NULL;
                                    }
                                } else {
                                    SDL_Log("Joy haptic open FAILED!: %s\n", SDL_GetError());
                                }
                            }
                            else
                            {
                                SDL_Log("No haptic found\n");
                            }
                        }
                    }
                    break;
                case SDL_JOYDEVICEREMOVED:
                    if (instance == event.jdevice.which)
                    {
                        SDL_Log("Joy Removed: %d\n", event.jdevice.which);
                        instance = -1;
                        if(enable_haptic && haptic)
                        {
                            SDL_HapticClose(haptic);
                            haptic = NULL;
                        }
                        SDL_JoystickClose(joystick);
                        joystick = NULL;
                    } else {
                        SDL_Log("Unknown joystick diconnected\n");
                    }
                    break;
                case SDL_JOYAXISMOTION:
/*
//                    SDL_Log("Axis Move: %d\n", event.jaxis.axis);
*/
                    if (enable_haptic)
                        SDL_HapticRumblePlay(haptic, 0.25, 250);
                    break;
                case SDL_JOYBUTTONDOWN:
                    SDL_Log("Button Press: %d\n", event.jbutton.button);
                    if(enable_haptic && haptic)
                    {
                        SDL_HapticRumblePlay(haptic, 0.25, 250);
                    }
                    if (event.jbutton.button == 0) {
                        SDL_Log("Exiting due to button press of button 0\n");
                        keepGoing = SDL_FALSE;
                    }
                    break;
                case SDL_JOYBUTTONUP:
                    SDL_Log("Button Release: %d\n", event.jbutton.button);
                    break;
            }
        }
    }

    SDL_Quit();

    return 0;
}
#else

int
main(int argc, char *argv[])
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL compiled without Joystick and haptic support.\n");
    return 1;
}

#endif
