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
#include "../../SDL_internal.h"

#if SDL_AUDIO_DRIVER_HAIKU

/* Allow access to the audio stream on Haiku */

#include <SoundPlayer.h>
#include <signal.h>

#include "../../main/haiku/SDL_BeApp.h"

extern "C"
{

#include "SDL_audio.h"
#include "../SDL_audio_c.h"
#include "../SDL_sysaudio.h"
#include "SDL_haikuaudio.h"

}


/* !!! FIXME: have the callback call the higher level to avoid code dupe. */
/* The Haiku callback for handling the audio buffer */
static void
FillSound(void *device, void *stream, size_t len,
          const media_raw_audio_format & format)
{
    SDL_AudioDevice *audio = (SDL_AudioDevice *) device;
    SDL_AudioCallback callback = audio->callbackspec.callback;

    /* Only do something if audio is enabled */
    if (!SDL_AtomicGet(&audio->enabled) || SDL_AtomicGet(&audio->paused)) {
        if (audio->stream) {
            SDL_AudioStreamClear(audio->stream);
        }
        SDL_memset(stream, audio->spec.silence, len);
        return;
    }

    SDL_assert(audio->spec.size == len);

    if (audio->stream == NULL) {  /* no conversion necessary. */
        SDL_LockMutex(audio->mixer_lock);
        callback(audio->callbackspec.userdata, (Uint8 *) stream, len);
        SDL_UnlockMutex(audio->mixer_lock);
    } else {  /* streaming/converting */
        const int stream_len = audio->callbackspec.size;
        const int ilen = (int) len;
        while (SDL_AudioStreamAvailable(audio->stream) < ilen) {
            callback(audio->callbackspec.userdata, audio->work_buffer, stream_len);
            if (SDL_AudioStreamPut(audio->stream, audio->work_buffer, stream_len) == -1) {
                SDL_AudioStreamClear(audio->stream);
                SDL_AtomicSet(&audio->enabled, 0);
                break;
            }
        }

        const int got = SDL_AudioStreamGet(audio->stream, stream, ilen);
        SDL_assert((got < 0) || (got == ilen));
        if (got != ilen) {
            SDL_memset(stream, audio->spec.silence, len);
        }
    }
}

static void
HAIKUAUDIO_CloseDevice(_THIS)
{
    if (_this->hidden->audio_obj) {
        _this->hidden->audio_obj->Stop();
        delete _this->hidden->audio_obj;
    }
    delete _this->hidden;
}


static const int sig_list[] = {
    SIGHUP, SIGINT, SIGQUIT, SIGPIPE, SIGALRM, SIGTERM, SIGWINCH, 0
};

static inline void
MaskSignals(sigset_t * omask)
{
    sigset_t mask;
    int i;

    sigemptyset(&mask);
    for (i = 0; sig_list[i]; ++i) {
        sigaddset(&mask, sig_list[i]);
    }
    sigprocmask(SIG_BLOCK, &mask, omask);
}

static inline void
UnmaskSignals(sigset_t * omask)
{
    sigprocmask(SIG_SETMASK, omask, NULL);
}


static int
HAIKUAUDIO_OpenDevice(_THIS, void *handle, const char *devname, int iscapture)
{
    int valid_datatype = 0;
    media_raw_audio_format format;
    SDL_AudioFormat test_format = SDL_FirstAudioFormat(_this->spec.format);

    /* Initialize all variables that we clean on shutdown */
    _this->hidden = new SDL_PrivateAudioData;
    if (_this->hidden == NULL) {
        return SDL_OutOfMemory();
    }
    SDL_zerop(_this->hidden);

    /* Parse the audio format and fill the Be raw audio format */
    SDL_zero(format);
    format.byte_order = B_MEDIA_LITTLE_ENDIAN;
    format.frame_rate = (float) _this->spec.freq;
    format.channel_count = _this->spec.channels;        /* !!! FIXME: support > 2? */
    while ((!valid_datatype) && (test_format)) {
        valid_datatype = 1;
        _this->spec.format = test_format;
        switch (test_format) {
        case AUDIO_S8:
            format.format = media_raw_audio_format::B_AUDIO_CHAR;
            break;

        case AUDIO_U8:
            format.format = media_raw_audio_format::B_AUDIO_UCHAR;
            break;

        case AUDIO_S16LSB:
            format.format = media_raw_audio_format::B_AUDIO_SHORT;
            break;

        case AUDIO_S16MSB:
            format.format = media_raw_audio_format::B_AUDIO_SHORT;
            format.byte_order = B_MEDIA_BIG_ENDIAN;
            break;

        case AUDIO_S32LSB:
            format.format = media_raw_audio_format::B_AUDIO_INT;
            break;

        case AUDIO_S32MSB:
            format.format = media_raw_audio_format::B_AUDIO_INT;
            format.byte_order = B_MEDIA_BIG_ENDIAN;
            break;

        case AUDIO_F32LSB:
            format.format = media_raw_audio_format::B_AUDIO_FLOAT;
            break;

        case AUDIO_F32MSB:
            format.format = media_raw_audio_format::B_AUDIO_FLOAT;
            format.byte_order = B_MEDIA_BIG_ENDIAN;
            break;

        default:
            valid_datatype = 0;
            test_format = SDL_NextAudioFormat();
            break;
        }
    }

    if (!valid_datatype) {      /* shouldn't happen, but just in case... */
        return SDL_SetError("Unsupported audio format");
    }

    /* Calculate the final parameters for this audio specification */
    SDL_CalculateAudioSpec(&_this->spec);

    format.buffer_size = _this->spec.size;

    /* Subscribe to the audio stream (creates a new thread) */
    sigset_t omask;
    MaskSignals(&omask);
    _this->hidden->audio_obj = new BSoundPlayer(&format, "SDL Audio",
                                                FillSound, NULL, _this);
    UnmaskSignals(&omask);

    if (_this->hidden->audio_obj->Start() == B_NO_ERROR) {
        _this->hidden->audio_obj->SetHasData(true);
    } else {
        return SDL_SetError("Unable to start Be audio");
    }

    /* We're running! */
    return 0;
}

static void
HAIKUAUDIO_Deinitialize(void)
{
    SDL_QuitBeApp();
}

static int
HAIKUAUDIO_Init(SDL_AudioDriverImpl * impl)
{
    /* Initialize the Be Application, if it's not already started */
    if (SDL_InitBeApp() < 0) {
        return 0;
    }

    /* Set the function pointers */
    impl->OpenDevice = HAIKUAUDIO_OpenDevice;
    impl->CloseDevice = HAIKUAUDIO_CloseDevice;
    impl->Deinitialize = HAIKUAUDIO_Deinitialize;
    impl->ProvidesOwnCallbackThread = 1;
    impl->OnlyHasDefaultOutputDevice = 1;

    return 1;   /* this audio target is available. */
}

extern "C"
{
    extern AudioBootStrap HAIKUAUDIO_bootstrap;
}
AudioBootStrap HAIKUAUDIO_bootstrap = {
    "haiku", "Haiku BSoundPlayer", HAIKUAUDIO_Init, 0
};

#endif /* SDL_AUDIO_DRIVER_HAIKU */

/* vi: set ts=4 sw=4 expandtab: */
