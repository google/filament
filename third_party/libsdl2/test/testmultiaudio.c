/*
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/
#include "SDL.h"

#include <stdio.h> /* for fflush() and stdout */

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

static SDL_AudioSpec spec;
static Uint8 *sound = NULL;     /* Pointer to wave data */
static Uint32 soundlen = 0;     /* Length of wave data */

typedef struct
{
    SDL_AudioDeviceID dev;
    int soundpos;
    SDL_atomic_t done;
} callback_data;

callback_data cbd[64];

void SDLCALL
play_through_once(void *arg, Uint8 * stream, int len)
{
    callback_data *cbdata = (callback_data *) arg;
    Uint8 *waveptr = sound + cbdata->soundpos;
    int waveleft = soundlen - cbdata->soundpos;
    int cpy = len;
    if (cpy > waveleft)
        cpy = waveleft;

    SDL_memcpy(stream, waveptr, cpy);
    len -= cpy;
    cbdata->soundpos += cpy;
    if (len > 0) {
        stream += cpy;
        SDL_memset(stream, spec.silence, len);
        SDL_AtomicSet(&cbdata->done, 1);
    }
}

void
loop()
{
    if (SDL_AtomicGet(&cbd[0].done)) {
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
#endif
        SDL_PauseAudioDevice(cbd[0].dev, 1);
        SDL_CloseAudioDevice(cbd[0].dev);
        SDL_FreeWAV(sound);
        SDL_Quit();
    }
}

static void
test_multi_audio(int devcount)
{
    int keep_going = 1;
    int i;
    
#ifdef __ANDROID__  
    SDL_Event event;
  
    /* Create a Window to get fully initialized event processing for testing pause on Android. */
    SDL_CreateWindow("testmultiaudio", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 320, 240, 0);
#endif

    if (devcount > 64) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Too many devices (%d), clamping to 64...\n",
                devcount);
        devcount = 64;
    }

    spec.callback = play_through_once;

    for (i = 0; i < devcount; i++) {
        const char *devname = SDL_GetAudioDeviceName(i, 0);
        SDL_Log("playing on device #%d: ('%s')...", i, devname);
        fflush(stdout);

        SDL_memset(&cbd[0], '\0', sizeof(callback_data));
        spec.userdata = &cbd[0];
        cbd[0].dev = SDL_OpenAudioDevice(devname, 0, &spec, NULL, 0);
        if (cbd[0].dev == 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Open device failed: %s\n", SDL_GetError());
        } else {
            SDL_PauseAudioDevice(cbd[0].dev, 0);
#ifdef __EMSCRIPTEN__
            emscripten_set_main_loop(loop, 0, 1);
#else
            while (!SDL_AtomicGet(&cbd[0].done)) {
                #ifdef __ANDROID__                
                /* Empty queue, some application events would prevent pause. */
                while (SDL_PollEvent(&event)){}
                #endif                
                SDL_Delay(100);
            }
            SDL_PauseAudioDevice(cbd[0].dev, 1);
#endif
            SDL_Log("done.\n");
            SDL_CloseAudioDevice(cbd[0].dev);
        }
    }

    SDL_memset(cbd, '\0', sizeof(cbd));

    SDL_Log("playing on all devices...\n");
    for (i = 0; i < devcount; i++) {
        const char *devname = SDL_GetAudioDeviceName(i, 0);
        spec.userdata = &cbd[i];
        cbd[i].dev = SDL_OpenAudioDevice(devname, 0, &spec, NULL, 0);
        if (cbd[i].dev == 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Open device %d failed: %s\n", i, SDL_GetError());
        }
    }

    for (i = 0; i < devcount; i++) {
        if (cbd[i].dev) {
            SDL_PauseAudioDevice(cbd[i].dev, 0);
        }
    }

    while (keep_going) {
        keep_going = 0;
        for (i = 0; i < devcount; i++) {
            if ((cbd[i].dev) && (!SDL_AtomicGet(&cbd[i].done))) {
                keep_going = 1;
            }
        }
        #ifdef __ANDROID__        
        /* Empty queue, some application events would prevent pause. */
        while (SDL_PollEvent(&event)){}
        #endif        

        SDL_Delay(100);
    }

#ifndef __EMSCRIPTEN__
    for (i = 0; i < devcount; i++) {
        if (cbd[i].dev) {
            SDL_PauseAudioDevice(cbd[i].dev, 1);
            SDL_CloseAudioDevice(cbd[i].dev);
        }
    }

    SDL_Log("All done!\n");
#endif
}


int
main(int argc, char **argv)
{
    int devcount = 0;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    /* Load the SDL library */
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s\n", SDL_GetError());
        return (1);
    }

    SDL_Log("Using audio driver: %s\n", SDL_GetCurrentAudioDriver());
    
    devcount = SDL_GetNumAudioDevices(0);
    if (devcount < 1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Don't see any specific audio devices!\n");
    } else {
        if (argv[1] == NULL) {
            argv[1] = "sample.wav";
        }

        /* Load the wave file into memory */
        if (SDL_LoadWAV(argv[1], &spec, &sound, &soundlen) == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load %s: %s\n", argv[1],
                    SDL_GetError());
        } else {
            test_multi_audio(devcount);
            SDL_FreeWAV(sound);
        }
    }

    SDL_Quit();
    return 0;
}
