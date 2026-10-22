/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

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

#if SDL_AUDIO_DRIVER_NETBSD

/*
 * Driver for native NetBSD audio(4).
 * vedge@vedge.com.ar.
 */

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/audioio.h>

#include "SDL_timer.h"
#include "SDL_audio.h"
#include "../../core/unix/SDL_poll.h"
#include "../SDL_audio_c.h"
#include "../SDL_audiodev_c.h"
#include "SDL_netbsdaudio.h"

/* Use timer for synchronization */
/* #define USE_TIMER_SYNC */

/* #define DEBUG_AUDIO */
/* #define DEBUG_AUDIO_STREAM */


static void
NETBSDAUDIO_DetectDevices(void)
{
    SDL_EnumUnixAudioDevices(0, NULL);
}


static void
NETBSDAUDIO_Status(_THIS)
{
#ifdef DEBUG_AUDIO
    /* *INDENT-OFF* */
    audio_info_t info;
    const audio_prinfo *prinfo;

    if (ioctl(this->hidden->audio_fd, AUDIO_GETINFO, &info) < 0) {
        fprintf(stderr, "AUDIO_GETINFO failed.\n");
        return;
    }

    prinfo = this->iscapture ? &info.play : &info.record;

    fprintf(stderr, "\n"
            "[%s info]\n"
            "buffer size	:   %d bytes\n"
            "sample rate	:   %i Hz\n"
            "channels	:   %i\n"
            "precision	:   %i-bit\n"
            "encoding	:   0x%x\n"
            "seek		:   %i\n"
            "sample count	:   %i\n"
            "EOF count	:   %i\n"
            "paused		:   %s\n"
            "error occured	:   %s\n"
            "waiting		:   %s\n"
            "active		:   %s\n"
            "",
            this->iscapture ? "record" : "play",
            prinfo->buffer_size,
            prinfo->sample_rate,
            prinfo->channels,
            prinfo->precision,
            prinfo->encoding,
            prinfo->seek,
            prinfo->samples,
            prinfo->eof,
            prinfo->pause ? "yes" : "no",
            prinfo->error ? "yes" : "no",
            prinfo->waiting ? "yes" : "no",
            prinfo->active ? "yes" : "no");

    fprintf(stderr, "\n"
            "[audio info]\n"
            "monitor_gain	:   %i\n"
            "hw block size	:   %d bytes\n"
            "hi watermark	:   %i\n"
            "lo watermark	:   %i\n"
            "audio mode	:   %s\n"
            "",
            info.monitor_gain,
            info.blocksize,
            info.hiwat, info.lowat,
            (info.mode == AUMODE_PLAY) ? "PLAY"
            : (info.mode = AUMODE_RECORD) ? "RECORD"
            : (info.mode == AUMODE_PLAY_ALL ? "PLAY_ALL" : "?"));
    /* *INDENT-ON* */
#endif /* DEBUG_AUDIO */
}


/* This function waits until it is possible to write a full sound buffer */
static void
NETBSDAUDIO_WaitDevice(_THIS)
{
#ifndef USE_BLOCKING_WRITES     /* Not necessary when using blocking writes */
    /* See if we need to use timed audio synchronization */
    if (this->hidden->frame_ticks) {
        /* Use timer for general audio synchronization */
        Sint32 ticks;

        ticks = ((Sint32) (this->hidden->next_frame - SDL_GetTicks())) - FUDGE_TICKS;
        if (ticks > 0) {
            SDL_Delay(ticks);
        }
    } else {
        /* Use SDL_IOReady() for audio synchronization */
#ifdef DEBUG_AUDIO
        fprintf(stderr, "Waiting for audio to get ready\n");
#endif
        if (SDL_IOReady(this->hidden->audio_fd, SDL_TRUE, 10 * 1000)
            <= 0) {
            const char *message =
                "Audio timeout - buggy audio driver? (disabled)";
            /* In general we should never print to the screen,
               but in this case we have no other way of letting
               the user know what happened.
             */
            fprintf(stderr, "SDL: %s\n", message);
            SDL_OpenedAudioDeviceDisconnected(this);
            /* Don't try to close - may hang */
            this->hidden->audio_fd = -1;
#ifdef DEBUG_AUDIO
            fprintf(stderr, "Done disabling audio\n");
#endif
        }
#ifdef DEBUG_AUDIO
        fprintf(stderr, "Ready!\n");
#endif
    }
#endif /* !USE_BLOCKING_WRITES */
}

static void
NETBSDAUDIO_PlayDevice(_THIS)
{
    int written, p = 0;

    /* Write the audio data, checking for EAGAIN on broken audio drivers */
    do {
        written = write(this->hidden->audio_fd,
                        &this->hidden->mixbuf[p], this->hidden->mixlen - p);

        if (written > 0)
            p += written;
        if (written == -1 && errno != 0 && errno != EAGAIN && errno != EINTR) {
            /* Non recoverable error has occurred. It should be reported!!! */
            perror("audio");
            break;
        }

#ifdef DEBUG_AUDIO
        fprintf(stderr, "Wrote %d bytes of audio data\n", written);
#endif

        if (p < this->hidden->mixlen
            || ((written < 0) && ((errno == 0) || (errno == EAGAIN)))) {
            SDL_Delay(1);       /* Let a little CPU time go by */
        }
    } while (p < this->hidden->mixlen);

    /* If timer synchronization is enabled, set the next write frame */
    if (this->hidden->frame_ticks) {
        this->hidden->next_frame += this->hidden->frame_ticks;
    }

    /* If we couldn't write, assume fatal error for now */
    if (written < 0) {
        SDL_OpenedAudioDeviceDisconnected(this);
    }
}

static Uint8 *
NETBSDAUDIO_GetDeviceBuf(_THIS)
{
    return (this->hidden->mixbuf);
}


static int
NETBSDAUDIO_CaptureFromDevice(_THIS, void *_buffer, int buflen)
{
    Uint8 *buffer = (Uint8 *) _buffer;
    int br, p = 0;

    /* Capture the audio data, checking for EAGAIN on broken audio drivers */
    do {
        br = read(this->hidden->audio_fd, buffer + p, buflen - p);
        if (br > 0)
            p += br;
        if (br == -1 && errno != 0 && errno != EAGAIN && errno != EINTR) {
            /* Non recoverable error has occurred. It should be reported!!! */
            perror("audio");
            return p ? p : -1;
        }

#ifdef DEBUG_AUDIO
        fprintf(stderr, "Captured %d bytes of audio data\n", br);
#endif

        if (p < buflen
            || ((br < 0) && ((errno == 0) || (errno == EAGAIN)))) {
            SDL_Delay(1);       /* Let a little CPU time go by */
        }
    } while (p < buflen);
}

static void
NETBSDAUDIO_FlushCapture(_THIS)
{
    audio_info_t info;
    size_t remain;
    Uint8 buf[512];

    if (ioctl(this->hidden->audio_fd, AUDIO_GETINFO, &info) < 0) {
        return;  /* oh well. */
    }

    remain = (size_t) (info.record.samples * (SDL_AUDIO_BITSIZE(this->spec.format) / 8));
    while (remain > 0) {
        const size_t len = SDL_min(sizeof (buf), remain);
        const int br = read(this->hidden->audio_fd, buf, len);
        if (br <= 0) {
            return;  /* oh well. */
        }
        remain -= br;
    }
}

static void
NETBSDAUDIO_CloseDevice(_THIS)
{
    if (this->hidden->audio_fd >= 0) {
        close(this->hidden->audio_fd);
    }
    SDL_free(this->hidden->mixbuf);
    SDL_free(this->hidden);
}

static int
NETBSDAUDIO_OpenDevice(_THIS, void *handle, const char *devname, int iscapture)
{
    const int flags = iscapture ? OPEN_FLAGS_INPUT : OPEN_FLAGS_OUTPUT;
    SDL_AudioFormat format = 0;
    audio_info_t info;
    audio_prinfo *prinfo = iscapture ? &info.play : &info.record;

    /* We don't care what the devname is...we'll try to open anything. */
    /*  ...but default to first name in the list... */
    if (devname == NULL) {
        devname = SDL_GetAudioDeviceName(0, iscapture);
        if (devname == NULL) {
            return SDL_SetError("No such audio device");
        }
    }

    /* Initialize all variables that we clean on shutdown */
    this->hidden = (struct SDL_PrivateAudioData *)
        SDL_malloc((sizeof *this->hidden));
    if (this->hidden == NULL) {
        return SDL_OutOfMemory();
    }
    SDL_zerop(this->hidden);

    /* Open the audio device */
    this->hidden->audio_fd = open(devname, flags, 0);
    if (this->hidden->audio_fd < 0) {
        return SDL_SetError("Couldn't open %s: %s", devname, strerror(errno));
    }

    AUDIO_INITINFO(&info);

    /* Calculate the final parameters for this audio specification */
    SDL_CalculateAudioSpec(&this->spec);

    /* Set to play mode */
    info.mode = iscapture ? AUMODE_RECORD : AUMODE_PLAY;
    if (ioctl(this->hidden->audio_fd, AUDIO_SETINFO, &info) < 0) {
        return SDL_SetError("Couldn't put device into play mode");
    }

    AUDIO_INITINFO(&info);
    for (format = SDL_FirstAudioFormat(this->spec.format);
         format; format = SDL_NextAudioFormat()) {
        switch (format) {
        case AUDIO_U8:
            prinfo->encoding = AUDIO_ENCODING_ULINEAR;
            prinfo->precision = 8;
            break;
        case AUDIO_S8:
            prinfo->encoding = AUDIO_ENCODING_SLINEAR;
            prinfo->precision = 8;
            break;
        case AUDIO_S16LSB:
            prinfo->encoding = AUDIO_ENCODING_SLINEAR_LE;
            prinfo->precision = 16;
            break;
        case AUDIO_S16MSB:
            prinfo->encoding = AUDIO_ENCODING_SLINEAR_BE;
            prinfo->precision = 16;
            break;
        case AUDIO_U16LSB:
            prinfo->encoding = AUDIO_ENCODING_ULINEAR_LE;
            prinfo->precision = 16;
            break;
        case AUDIO_U16MSB:
            prinfo->encoding = AUDIO_ENCODING_ULINEAR_BE;
            prinfo->precision = 16;
            break;
        default:
            continue;
        }

        if (ioctl(this->hidden->audio_fd, AUDIO_SETINFO, &info) == 0) {
            break;
        }
    }

    if (!format) {
        return SDL_SetError("No supported encoding for 0x%x", this->spec.format);
    }

    this->spec.format = format;

    AUDIO_INITINFO(&info);
    prinfo->channels = this->spec.channels;
    if (ioctl(this->hidden->audio_fd, AUDIO_SETINFO, &info) == -1) {
        this->spec.channels = 1;
    }
    AUDIO_INITINFO(&info);
    prinfo->sample_rate = this->spec.freq;
    info.blocksize = this->spec.size;
    info.hiwat = 5;
    info.lowat = 3;
    (void) ioctl(this->hidden->audio_fd, AUDIO_SETINFO, &info);
    (void) ioctl(this->hidden->audio_fd, AUDIO_GETINFO, &info);
    this->spec.freq = prinfo->sample_rate;

    if (!iscapture) {
        /* Allocate mixing buffer */
        this->hidden->mixlen = this->spec.size;
        this->hidden->mixbuf = (Uint8 *) SDL_malloc(this->hidden->mixlen);
        if (this->hidden->mixbuf == NULL) {
            return SDL_OutOfMemory();
        }
        SDL_memset(this->hidden->mixbuf, this->spec.silence, this->spec.size);
    }

    NETBSDAUDIO_Status(this);

    /* We're ready to rock and roll. :-) */
    return 0;
}

static int
NETBSDAUDIO_Init(SDL_AudioDriverImpl * impl)
{
    /* Set the function pointers */
    impl->DetectDevices = NETBSDAUDIO_DetectDevices;
    impl->OpenDevice = NETBSDAUDIO_OpenDevice;
    impl->PlayDevice = NETBSDAUDIO_PlayDevice;
    impl->WaitDevice = NETBSDAUDIO_WaitDevice;
    impl->GetDeviceBuf = NETBSDAUDIO_GetDeviceBuf;
    impl->CloseDevice = NETBSDAUDIO_CloseDevice;
    impl->CaptureFromDevice = NETBSDAUDIO_CaptureFromDevice;
    impl->FlushCapture = NETBSDAUDIO_FlushCapture;

    impl->HasCaptureSupport = SDL_TRUE;
    impl->AllowsArbitraryDeviceNames = 1;

    return 1;   /* this audio target is available. */
}


AudioBootStrap NETBSDAUDIO_bootstrap = {
    "netbsd", "NetBSD audio", NETBSDAUDIO_Init, 0
};

#endif /* SDL_AUDIO_DRIVER_NETBSD */

/* vi: set ts=4 sw=4 expandtab: */
