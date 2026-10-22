/*
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

/* Program to test hotplugging of audio devices */

#include "SDL_config.h"

#include <stdio.h>
#include <stdlib.h>

#if HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include "SDL.h"

static SDL_AudioSpec spec;
static Uint8 *sound = NULL;     /* Pointer to wave data */
static Uint32 soundlen = 0;     /* Length of wave data */

static int posindex = 0;
static Uint32 positions[64];

/* Call this instead of exit(), so we can clean up SDL: atexit() is evil. */
static void
quit(int rc)
{
    SDL_Quit();
    exit(rc);
}

void SDLCALL
fillerup(void *_pos, Uint8 * stream, int len)
{
    Uint32 pos = *((Uint32 *) _pos);
    Uint8 *waveptr;
    int waveleft;

    /* Set up the pointers */
    waveptr = sound + pos;
    waveleft = soundlen - pos;

    /* Go! */
    while (waveleft <= len) {
        SDL_memcpy(stream, waveptr, waveleft);
        stream += waveleft;
        len -= waveleft;
        waveptr = sound;
        waveleft = soundlen;
        pos = 0;
    }
    SDL_memcpy(stream, waveptr, len);
    pos += len;
    *((Uint32 *) _pos) = pos;
}

static int done = 0;
void
poked(int sig)
{
    done = 1;
}

static const char*
devtypestr(int iscapture)
{
    return iscapture ? "capture" : "output";
}

static void
iteration()
{
    SDL_Event e;
    SDL_AudioDeviceID dev;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            done = 1;
        } else if (e.type == SDL_KEYUP) {
            if (e.key.keysym.sym == SDLK_ESCAPE)
                done = 1;
        } else if (e.type == SDL_AUDIODEVICEADDED) {
            int index = e.adevice.which;
            int iscapture = e.adevice.iscapture;
            const char *name = SDL_GetAudioDeviceName(index, iscapture);
            if (name != NULL)
                SDL_Log("New %s audio device at index %u: %s\n", devtypestr(iscapture), (unsigned int) index, name);
            else {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Got new %s device at index %u, but failed to get the name: %s\n",
                    devtypestr(iscapture), (unsigned int) index, SDL_GetError());
                continue;
            }
            if (!iscapture) {
                positions[posindex] = 0;
                spec.userdata = &positions[posindex++];
                spec.callback = fillerup;
                dev = SDL_OpenAudioDevice(name, 0, &spec, NULL, 0);
                if (!dev) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open '%s': %s\n", name, SDL_GetError());
                } else {
                    SDL_Log("Opened '%s' as %u\n", name, (unsigned int) dev);
                    SDL_PauseAudioDevice(dev, 0);
                }
            }
        } else if (e.type == SDL_AUDIODEVICEREMOVED) {
            dev = (SDL_AudioDeviceID) e.adevice.which;
            SDL_Log("%s device %u removed.\n", devtypestr(e.adevice.iscapture), (unsigned int) dev);
            SDL_CloseAudioDevice(dev);
        }
    }
}

#ifdef __EMSCRIPTEN__
void
loop()
{
    if(done)
        emscripten_cancel_main_loop();
    else
        iteration();
}
#endif

int
main(int argc, char *argv[])
{
    int i;
    char filename[4096];

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    /* Load the SDL library */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s\n", SDL_GetError());
        return (1);
    }

    /* Some targets (Mac CoreAudio) need an event queue for audio hotplug, so make and immediately hide a window. */
    SDL_MinimizeWindow(SDL_CreateWindow("testaudiohotplug", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0));

    if (argc > 1) {
        SDL_strlcpy(filename, argv[1], sizeof(filename));
    } else {
        SDL_strlcpy(filename, "sample.wav", sizeof(filename));
    }
    /* Load the wave file into memory */
    if (SDL_LoadWAV(filename, &spec, &sound, &soundlen) == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load %s: %s\n", filename, SDL_GetError());
        quit(1);
    }

#if HAVE_SIGNAL_H
    /* Set the signals */
#ifdef SIGHUP
    signal(SIGHUP, poked);
#endif
    signal(SIGINT, poked);
#ifdef SIGQUIT
    signal(SIGQUIT, poked);
#endif
    signal(SIGTERM, poked);
#endif /* HAVE_SIGNAL_H */

    /* Show the list of available drivers */
    SDL_Log("Available audio drivers:");
    for (i = 0; i < SDL_GetNumAudioDrivers(); ++i) {
        SDL_Log("%i: %s", i, SDL_GetAudioDriver(i));
    }

    SDL_Log("Select a driver with the SDL_AUDIODRIVER environment variable.\n");
    SDL_Log("Using audio driver: %s\n", SDL_GetCurrentAudioDriver());

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loop, 0, 1);
#else
    while (!done) {
        SDL_Delay(100);
        iteration();
    }
#endif

    /* Clean up on signal */
    /* Quit audio first, then free WAV. This prevents access violations in the audio threads. */
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    SDL_FreeWAV(sound);
    SDL_Quit();
    return (0);
}

/* vi: set ts=4 sw=4 expandtab: */
