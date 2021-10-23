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

#if SDL_AUDIO_DRIVER_ALSA

#ifndef SDL_ALSA_NON_BLOCKING
#define SDL_ALSA_NON_BLOCKING 0
#endif

/* Allow access to a raw mixing buffer */

#include <sys/types.h>
#include <signal.h>             /* For kill() */
#include <string.h>

#include "SDL_timer.h"
#include "SDL_audio.h"
#include "../SDL_audio_c.h"
#include "SDL_alsa_audio.h"

#ifdef SDL_AUDIO_DRIVER_ALSA_DYNAMIC
#include "SDL_loadso.h"
#endif

static int (*ALSA_snd_pcm_open)
  (snd_pcm_t **, const char *, snd_pcm_stream_t, int);
static int (*ALSA_snd_pcm_close) (snd_pcm_t * pcm);
static snd_pcm_sframes_t (*ALSA_snd_pcm_writei)
  (snd_pcm_t *, const void *, snd_pcm_uframes_t);
static snd_pcm_sframes_t (*ALSA_snd_pcm_readi)
  (snd_pcm_t *, void *, snd_pcm_uframes_t);
static int (*ALSA_snd_pcm_recover) (snd_pcm_t *, int, int);
static int (*ALSA_snd_pcm_prepare) (snd_pcm_t *);
static int (*ALSA_snd_pcm_drain) (snd_pcm_t *);
static const char *(*ALSA_snd_strerror) (int);
static size_t(*ALSA_snd_pcm_hw_params_sizeof) (void);
static size_t(*ALSA_snd_pcm_sw_params_sizeof) (void);
static void (*ALSA_snd_pcm_hw_params_copy)
  (snd_pcm_hw_params_t *, const snd_pcm_hw_params_t *);
static int (*ALSA_snd_pcm_hw_params_any) (snd_pcm_t *, snd_pcm_hw_params_t *);
static int (*ALSA_snd_pcm_hw_params_set_access)
  (snd_pcm_t *, snd_pcm_hw_params_t *, snd_pcm_access_t);
static int (*ALSA_snd_pcm_hw_params_set_format)
  (snd_pcm_t *, snd_pcm_hw_params_t *, snd_pcm_format_t);
static int (*ALSA_snd_pcm_hw_params_set_channels)
  (snd_pcm_t *, snd_pcm_hw_params_t *, unsigned int);
static int (*ALSA_snd_pcm_hw_params_get_channels)
  (const snd_pcm_hw_params_t *, unsigned int *);
static int (*ALSA_snd_pcm_hw_params_set_rate_near)
  (snd_pcm_t *, snd_pcm_hw_params_t *, unsigned int *, int *);
static int (*ALSA_snd_pcm_hw_params_set_period_size_near)
  (snd_pcm_t *, snd_pcm_hw_params_t *, snd_pcm_uframes_t *, int *);
static int (*ALSA_snd_pcm_hw_params_get_period_size)
  (const snd_pcm_hw_params_t *, snd_pcm_uframes_t *, int *);
static int (*ALSA_snd_pcm_hw_params_set_periods_min)
  (snd_pcm_t *, snd_pcm_hw_params_t *, unsigned int *, int *);
static int (*ALSA_snd_pcm_hw_params_set_periods_first)
  (snd_pcm_t *, snd_pcm_hw_params_t *, unsigned int *, int *);
static int (*ALSA_snd_pcm_hw_params_get_periods)
  (const snd_pcm_hw_params_t *, unsigned int *, int *);
static int (*ALSA_snd_pcm_hw_params_set_buffer_size_near)
  (snd_pcm_t *pcm, snd_pcm_hw_params_t *, snd_pcm_uframes_t *);
static int (*ALSA_snd_pcm_hw_params_get_buffer_size)
  (const snd_pcm_hw_params_t *, snd_pcm_uframes_t *);
static int (*ALSA_snd_pcm_hw_params) (snd_pcm_t *, snd_pcm_hw_params_t *);
static int (*ALSA_snd_pcm_sw_params_current) (snd_pcm_t *,
                                              snd_pcm_sw_params_t *);
static int (*ALSA_snd_pcm_sw_params_set_start_threshold)
  (snd_pcm_t *, snd_pcm_sw_params_t *, snd_pcm_uframes_t);
static int (*ALSA_snd_pcm_sw_params) (snd_pcm_t *, snd_pcm_sw_params_t *);
static int (*ALSA_snd_pcm_nonblock) (snd_pcm_t *, int);
static int (*ALSA_snd_pcm_wait)(snd_pcm_t *, int);
static int (*ALSA_snd_pcm_sw_params_set_avail_min)
  (snd_pcm_t *, snd_pcm_sw_params_t *, snd_pcm_uframes_t);
static int (*ALSA_snd_pcm_reset)(snd_pcm_t *);
static int (*ALSA_snd_device_name_hint) (int, const char *, void ***);
static char* (*ALSA_snd_device_name_get_hint) (const void *, const char *);
static int (*ALSA_snd_device_name_free_hint) (void **);
static snd_pcm_sframes_t (*ALSA_snd_pcm_avail)(snd_pcm_t *);
#ifdef SND_CHMAP_API_VERSION
static snd_pcm_chmap_t* (*ALSA_snd_pcm_get_chmap) (snd_pcm_t *);
static int (*ALSA_snd_pcm_chmap_print) (const snd_pcm_chmap_t *map, size_t maxlen, char *buf);
#endif

#ifdef SDL_AUDIO_DRIVER_ALSA_DYNAMIC
#define snd_pcm_hw_params_sizeof ALSA_snd_pcm_hw_params_sizeof
#define snd_pcm_sw_params_sizeof ALSA_snd_pcm_sw_params_sizeof

static const char *alsa_library = SDL_AUDIO_DRIVER_ALSA_DYNAMIC;
static void *alsa_handle = NULL;

static int
load_alsa_sym(const char *fn, void **addr)
{
    *addr = SDL_LoadFunction(alsa_handle, fn);
    if (*addr == NULL) {
        /* Don't call SDL_SetError(): SDL_LoadFunction already did. */
        return 0;
    }

    return 1;
}

/* cast funcs to char* first, to please GCC's strict aliasing rules. */
#define SDL_ALSA_SYM(x) \
    if (!load_alsa_sym(#x, (void **) (char *) &ALSA_##x)) return -1
#else
#define SDL_ALSA_SYM(x) ALSA_##x = x
#endif

static int
load_alsa_syms(void)
{
    SDL_ALSA_SYM(snd_pcm_open);
    SDL_ALSA_SYM(snd_pcm_close);
    SDL_ALSA_SYM(snd_pcm_writei);
    SDL_ALSA_SYM(snd_pcm_readi);
    SDL_ALSA_SYM(snd_pcm_recover);
    SDL_ALSA_SYM(snd_pcm_prepare);
    SDL_ALSA_SYM(snd_pcm_drain);
    SDL_ALSA_SYM(snd_strerror);
    SDL_ALSA_SYM(snd_pcm_hw_params_sizeof);
    SDL_ALSA_SYM(snd_pcm_sw_params_sizeof);
    SDL_ALSA_SYM(snd_pcm_hw_params_copy);
    SDL_ALSA_SYM(snd_pcm_hw_params_any);
    SDL_ALSA_SYM(snd_pcm_hw_params_set_access);
    SDL_ALSA_SYM(snd_pcm_hw_params_set_format);
    SDL_ALSA_SYM(snd_pcm_hw_params_set_channels);
    SDL_ALSA_SYM(snd_pcm_hw_params_get_channels);
    SDL_ALSA_SYM(snd_pcm_hw_params_set_rate_near);
    SDL_ALSA_SYM(snd_pcm_hw_params_set_period_size_near);
    SDL_ALSA_SYM(snd_pcm_hw_params_get_period_size);
    SDL_ALSA_SYM(snd_pcm_hw_params_set_periods_min);
    SDL_ALSA_SYM(snd_pcm_hw_params_set_periods_first);
    SDL_ALSA_SYM(snd_pcm_hw_params_get_periods);
    SDL_ALSA_SYM(snd_pcm_hw_params_set_buffer_size_near);
    SDL_ALSA_SYM(snd_pcm_hw_params_get_buffer_size);
    SDL_ALSA_SYM(snd_pcm_hw_params);
    SDL_ALSA_SYM(snd_pcm_sw_params_current);
    SDL_ALSA_SYM(snd_pcm_sw_params_set_start_threshold);
    SDL_ALSA_SYM(snd_pcm_sw_params);
    SDL_ALSA_SYM(snd_pcm_nonblock);
    SDL_ALSA_SYM(snd_pcm_wait);
    SDL_ALSA_SYM(snd_pcm_sw_params_set_avail_min);
    SDL_ALSA_SYM(snd_pcm_reset);
    SDL_ALSA_SYM(snd_device_name_hint);
    SDL_ALSA_SYM(snd_device_name_get_hint);
    SDL_ALSA_SYM(snd_device_name_free_hint);
    SDL_ALSA_SYM(snd_pcm_avail);
#ifdef SND_CHMAP_API_VERSION
    SDL_ALSA_SYM(snd_pcm_get_chmap);
    SDL_ALSA_SYM(snd_pcm_chmap_print);
#endif

    return 0;
}

#undef SDL_ALSA_SYM

#ifdef SDL_AUDIO_DRIVER_ALSA_DYNAMIC

static void
UnloadALSALibrary(void)
{
    if (alsa_handle != NULL) {
        SDL_UnloadObject(alsa_handle);
        alsa_handle = NULL;
    }
}

static int
LoadALSALibrary(void)
{
    int retval = 0;
    if (alsa_handle == NULL) {
        alsa_handle = SDL_LoadObject(alsa_library);
        if (alsa_handle == NULL) {
            retval = -1;
            /* Don't call SDL_SetError(): SDL_LoadObject already did. */
        } else {
            retval = load_alsa_syms();
            if (retval < 0) {
                UnloadALSALibrary();
            }
        }
    }
    return retval;
}

#else

static void
UnloadALSALibrary(void)
{
}

static int
LoadALSALibrary(void)
{
    load_alsa_syms();
    return 0;
}

#endif /* SDL_AUDIO_DRIVER_ALSA_DYNAMIC */

static const char *
get_audio_device(void *handle, const int channels)
{
    const char *device;

    if (handle != NULL) {
        return (const char *) handle;
    }

    /* !!! FIXME: we also check "SDL_AUDIO_DEVICE_NAME" at the higher level. */
    device = SDL_getenv("AUDIODEV");    /* Is there a standard variable name? */
    if (device != NULL) {
        return device;
    }

    if (channels == 6) {
        return "plug:surround51";
    } else if (channels == 4) {
        return "plug:surround40";
    }

    return "default";
}


/* This function waits until it is possible to write a full sound buffer */
static void
ALSA_WaitDevice(_THIS)
{
#if SDL_ALSA_NON_BLOCKING
    const snd_pcm_sframes_t needed = (snd_pcm_sframes_t) this->spec.samples;
    while (SDL_AtomicGet(&this->enabled)) {
        const snd_pcm_sframes_t rc = ALSA_snd_pcm_avail(this->hidden->pcm_handle);
        if ((rc < 0) && (rc != -EAGAIN)) {
            /* Hmm, not much we can do - abort */
            fprintf(stderr, "ALSA snd_pcm_avail failed (unrecoverable): %s\n",
                        ALSA_snd_strerror(rc));
            SDL_OpenedAudioDeviceDisconnected(this);
            return;
        } else if (rc < needed) {
            const Uint32 delay = ((needed - (SDL_max(rc, 0))) * 1000) / this->spec.freq;
            SDL_Delay(SDL_max(delay, 10));
        } else {
            break;  /* ready to go! */
        }
    }
#endif
}


/* !!! FIXME: is there a channel swizzler in alsalib instead? */
/*
 * http://bugzilla.libsdl.org/show_bug.cgi?id=110
 * "For Linux ALSA, this is FL-FR-RL-RR-C-LFE
 *  and for Windows DirectX [and CoreAudio], this is FL-FR-C-LFE-RL-RR"
 */
#define SWIZ6(T, buf, numframes) \
    T *ptr = (T *) buf; \
    Uint32 i; \
    for (i = 0; i < numframes; i++, ptr += 6) { \
        T tmp; \
        tmp = ptr[2]; ptr[2] = ptr[4]; ptr[4] = tmp; \
        tmp = ptr[3]; ptr[3] = ptr[5]; ptr[5] = tmp; \
    }

static void
swizzle_alsa_channels_6_64bit(void *buffer, Uint32 bufferlen)
{
    SWIZ6(Uint64, buffer, bufferlen);
}

static void
swizzle_alsa_channels_6_32bit(void *buffer, Uint32 bufferlen)
{
    SWIZ6(Uint32, buffer, bufferlen);
}

static void
swizzle_alsa_channels_6_16bit(void *buffer, Uint32 bufferlen)
{
    SWIZ6(Uint16, buffer, bufferlen);
}

static void
swizzle_alsa_channels_6_8bit(void *buffer, Uint32 bufferlen)
{
    SWIZ6(Uint8, buffer, bufferlen);
}

#undef SWIZ6


/*
 * Called right before feeding this->hidden->mixbuf to the hardware. Swizzle
 *  channels from Windows/Mac order to the format alsalib will want.
 */
static void
swizzle_alsa_channels(_THIS, void *buffer, Uint32 bufferlen)
{
    if (this->spec.channels == 6) {
        switch (SDL_AUDIO_BITSIZE(this->spec.format)) {
            case 8: swizzle_alsa_channels_6_8bit(buffer, bufferlen); break;
            case 16: swizzle_alsa_channels_6_16bit(buffer, bufferlen); break;
            case 32: swizzle_alsa_channels_6_32bit(buffer, bufferlen); break;
            case 64: swizzle_alsa_channels_6_64bit(buffer, bufferlen); break;
            default: SDL_assert(!"unhandled bitsize"); break;
        }
    }

    /* !!! FIXME: update this for 7.1 if needed, later. */
}

#ifdef SND_CHMAP_API_VERSION
/* Some devices have the right channel map, no swizzling necessary */
static void
no_swizzle(_THIS, void *buffer, Uint32 bufferlen)
{
}
#endif /* SND_CHMAP_API_VERSION */


static void
ALSA_PlayDevice(_THIS)
{
    const Uint8 *sample_buf = (const Uint8 *) this->hidden->mixbuf;
    const int frame_size = ((SDL_AUDIO_BITSIZE(this->spec.format)) / 8) *
                                this->spec.channels;
    snd_pcm_uframes_t frames_left = ((snd_pcm_uframes_t) this->spec.samples);

    this->hidden->swizzle_func(this, this->hidden->mixbuf, frames_left);

    while ( frames_left > 0 && SDL_AtomicGet(&this->enabled) ) {
        int status = ALSA_snd_pcm_writei(this->hidden->pcm_handle,
                                         sample_buf, frames_left);

        if (status < 0) {
            if (status == -EAGAIN) {
                /* Apparently snd_pcm_recover() doesn't handle this case -
                   does it assume snd_pcm_wait() above? */
                SDL_Delay(1);
                continue;
            }
            status = ALSA_snd_pcm_recover(this->hidden->pcm_handle, status, 0);
            if (status < 0) {
                /* Hmm, not much we can do - abort */
                fprintf(stderr, "ALSA write failed (unrecoverable): %s\n",
                        ALSA_snd_strerror(status));
                SDL_OpenedAudioDeviceDisconnected(this);
                return;
            }
            continue;
        }
        else if (status == 0) {
            /* No frames were written (no available space in pcm device).
               Allow other threads to catch up. */
            Uint32 delay = (frames_left / 2 * 1000) / this->spec.freq;
            SDL_Delay(delay);
        }

        sample_buf += status * frame_size;
        frames_left -= status;
    }
}

static Uint8 *
ALSA_GetDeviceBuf(_THIS)
{
    return (this->hidden->mixbuf);
}

static int
ALSA_CaptureFromDevice(_THIS, void *buffer, int buflen)
{
    Uint8 *sample_buf = (Uint8 *) buffer;
    const int frame_size = ((SDL_AUDIO_BITSIZE(this->spec.format)) / 8) *
                                this->spec.channels;
    const int total_frames = buflen / frame_size;
    snd_pcm_uframes_t frames_left = total_frames;
    snd_pcm_uframes_t wait_time = frame_size / 2;

    SDL_assert((buflen % frame_size) == 0);

    while ( frames_left > 0 && SDL_AtomicGet(&this->enabled) ) {
        int status;

        status = ALSA_snd_pcm_readi(this->hidden->pcm_handle,
                                        sample_buf, frames_left);

        if (status == -EAGAIN) {
            ALSA_snd_pcm_wait(this->hidden->pcm_handle, wait_time);
            status = 0;
        }
        else if (status < 0) {
            /*printf("ALSA: capture error %d\n", status);*/
            status = ALSA_snd_pcm_recover(this->hidden->pcm_handle, status, 0);
            if (status < 0) {
                /* Hmm, not much we can do - abort */
                fprintf(stderr, "ALSA read failed (unrecoverable): %s\n",
                        ALSA_snd_strerror(status));
                return -1;
            }
            continue;
        }

        /*printf("ALSA: captured %d bytes\n", status * frame_size);*/
        sample_buf += status * frame_size;
        frames_left -= status;
    }

    this->hidden->swizzle_func(this, buffer, total_frames - frames_left);

    return (total_frames - frames_left) * frame_size;
}

static void
ALSA_FlushCapture(_THIS)
{
    ALSA_snd_pcm_reset(this->hidden->pcm_handle);
}

static void
ALSA_CloseDevice(_THIS)
{
    if (this->hidden->pcm_handle) {
        /* Wait for the submitted audio to drain
           ALSA_snd_pcm_drop() can hang, so don't use that.
         */
        Uint32 delay = ((this->spec.samples * 1000) / this->spec.freq) * 2;
        SDL_Delay(delay);

        ALSA_snd_pcm_close(this->hidden->pcm_handle);
    }
    SDL_free(this->hidden->mixbuf);
    SDL_free(this->hidden);
}

static int
ALSA_set_buffer_size(_THIS, snd_pcm_hw_params_t *params)
{
    int status;
    snd_pcm_hw_params_t *hwparams;
    snd_pcm_uframes_t persize;
    unsigned int periods;

    /* Copy the hardware parameters for this setup */
    snd_pcm_hw_params_alloca(&hwparams);
    ALSA_snd_pcm_hw_params_copy(hwparams, params);

    /* Attempt to match the period size to the requested buffer size */
    persize = this->spec.samples;
    status = ALSA_snd_pcm_hw_params_set_period_size_near(
                this->hidden->pcm_handle, hwparams, &persize, NULL);
    if ( status < 0 ) {
        return(-1);
    }

    /* Need to at least double buffer */
    periods = 2;
    status = ALSA_snd_pcm_hw_params_set_periods_min(
                this->hidden->pcm_handle, hwparams, &periods, NULL);
    if ( status < 0 ) {
        return(-1);
    }

    status = ALSA_snd_pcm_hw_params_set_periods_first(
                this->hidden->pcm_handle, hwparams, &periods, NULL);
    if ( status < 0 ) {
        return(-1);
    }

    /* "set" the hardware with the desired parameters */
    status = ALSA_snd_pcm_hw_params(this->hidden->pcm_handle, hwparams);
    if ( status < 0 ) {
        return(-1);
    }

    this->spec.samples = persize;

    /* This is useful for debugging */
    if ( SDL_getenv("SDL_AUDIO_ALSA_DEBUG") ) {
        snd_pcm_uframes_t bufsize;

        ALSA_snd_pcm_hw_params_get_buffer_size(hwparams, &bufsize);

        fprintf(stderr,
            "ALSA: period size = %ld, periods = %u, buffer size = %lu\n",
            persize, periods, bufsize);
    }

    return(0);
}

static int
ALSA_OpenDevice(_THIS, void *handle, const char *devname, int iscapture)
{
    int status = 0;
    snd_pcm_t *pcm_handle = NULL;
    snd_pcm_hw_params_t *hwparams = NULL;
    snd_pcm_sw_params_t *swparams = NULL;
    snd_pcm_format_t format = 0;
    SDL_AudioFormat test_format = 0;
    unsigned int rate = 0;
    unsigned int channels = 0;
#ifdef SND_CHMAP_API_VERSION
    snd_pcm_chmap_t *chmap;
    char chmap_str[64];
#endif

    /* Initialize all variables that we clean on shutdown */
    this->hidden = (struct SDL_PrivateAudioData *)
        SDL_malloc((sizeof *this->hidden));
    if (this->hidden == NULL) {
        return SDL_OutOfMemory();
    }
    SDL_zerop(this->hidden);

    /* Open the audio device */
    /* Name of device should depend on # channels in spec */
    status = ALSA_snd_pcm_open(&pcm_handle,
                get_audio_device(handle, this->spec.channels),
                iscapture ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK,
                SND_PCM_NONBLOCK);

    if (status < 0) {
        return SDL_SetError("ALSA: Couldn't open audio device: %s",
                            ALSA_snd_strerror(status));
    }

    this->hidden->pcm_handle = pcm_handle;

    /* Figure out what the hardware is capable of */
    snd_pcm_hw_params_alloca(&hwparams);
    status = ALSA_snd_pcm_hw_params_any(pcm_handle, hwparams);
    if (status < 0) {
        return SDL_SetError("ALSA: Couldn't get hardware config: %s",
                            ALSA_snd_strerror(status));
    }

    /* SDL only uses interleaved sample output */
    status = ALSA_snd_pcm_hw_params_set_access(pcm_handle, hwparams,
                                               SND_PCM_ACCESS_RW_INTERLEAVED);
    if (status < 0) {
        return SDL_SetError("ALSA: Couldn't set interleaved access: %s",
                     ALSA_snd_strerror(status));
    }

    /* Try for a closest match on audio format */
    status = -1;
    for (test_format = SDL_FirstAudioFormat(this->spec.format);
         test_format && (status < 0);) {
        status = 0;             /* if we can't support a format, it'll become -1. */
        switch (test_format) {
        case AUDIO_U8:
            format = SND_PCM_FORMAT_U8;
            break;
        case AUDIO_S8:
            format = SND_PCM_FORMAT_S8;
            break;
        case AUDIO_S16LSB:
            format = SND_PCM_FORMAT_S16_LE;
            break;
        case AUDIO_S16MSB:
            format = SND_PCM_FORMAT_S16_BE;
            break;
        case AUDIO_U16LSB:
            format = SND_PCM_FORMAT_U16_LE;
            break;
        case AUDIO_U16MSB:
            format = SND_PCM_FORMAT_U16_BE;
            break;
        case AUDIO_S32LSB:
            format = SND_PCM_FORMAT_S32_LE;
            break;
        case AUDIO_S32MSB:
            format = SND_PCM_FORMAT_S32_BE;
            break;
        case AUDIO_F32LSB:
            format = SND_PCM_FORMAT_FLOAT_LE;
            break;
        case AUDIO_F32MSB:
            format = SND_PCM_FORMAT_FLOAT_BE;
            break;
        default:
            status = -1;
            break;
        }
        if (status >= 0) {
            status = ALSA_snd_pcm_hw_params_set_format(pcm_handle,
                                                       hwparams, format);
        }
        if (status < 0) {
            test_format = SDL_NextAudioFormat();
        }
    }
    if (status < 0) {
        return SDL_SetError("ALSA: Couldn't find any hardware audio formats");
    }
    this->spec.format = test_format;

    /* Validate number of channels and determine if swizzling is necessary
     * Assume original swizzling, until proven otherwise.
     */
    this->hidden->swizzle_func = swizzle_alsa_channels;
#ifdef SND_CHMAP_API_VERSION
    chmap = ALSA_snd_pcm_get_chmap(pcm_handle);
    if (chmap) {
        ALSA_snd_pcm_chmap_print(chmap, sizeof(chmap_str), chmap_str);
        if (SDL_strcmp("FL FR FC LFE RL RR", chmap_str) == 0 ||
            SDL_strcmp("FL FR FC LFE SL SR", chmap_str) == 0) {
            this->hidden->swizzle_func = no_swizzle;
        }
        free(chmap);
    }
#endif /* SND_CHMAP_API_VERSION */

    /* Set the number of channels */
    status = ALSA_snd_pcm_hw_params_set_channels(pcm_handle, hwparams,
                                                 this->spec.channels);
    channels = this->spec.channels;
    if (status < 0) {
        status = ALSA_snd_pcm_hw_params_get_channels(hwparams, &channels);
        if (status < 0) {
            return SDL_SetError("ALSA: Couldn't set audio channels");
        }
        this->spec.channels = channels;
    }

    /* Set the audio rate */
    rate = this->spec.freq;
    status = ALSA_snd_pcm_hw_params_set_rate_near(pcm_handle, hwparams,
                                                  &rate, NULL);
    if (status < 0) {
        return SDL_SetError("ALSA: Couldn't set audio frequency: %s",
                            ALSA_snd_strerror(status));
    }
    this->spec.freq = rate;

    /* Set the buffer size, in samples */
    status = ALSA_set_buffer_size(this, hwparams);
    if (status < 0) {
        return SDL_SetError("Couldn't set hardware audio parameters: %s", ALSA_snd_strerror(status));
    }

    /* Set the software parameters */
    snd_pcm_sw_params_alloca(&swparams);
    status = ALSA_snd_pcm_sw_params_current(pcm_handle, swparams);
    if (status < 0) {
        return SDL_SetError("ALSA: Couldn't get software config: %s",
                            ALSA_snd_strerror(status));
    }
    status = ALSA_snd_pcm_sw_params_set_avail_min(pcm_handle, swparams, this->spec.samples);
    if (status < 0) {
        return SDL_SetError("Couldn't set minimum available samples: %s",
                            ALSA_snd_strerror(status));
    }
    status =
        ALSA_snd_pcm_sw_params_set_start_threshold(pcm_handle, swparams, 1);
    if (status < 0) {
        return SDL_SetError("ALSA: Couldn't set start threshold: %s",
                            ALSA_snd_strerror(status));
    }
    status = ALSA_snd_pcm_sw_params(pcm_handle, swparams);
    if (status < 0) {
        return SDL_SetError("Couldn't set software audio parameters: %s",
                            ALSA_snd_strerror(status));
    }

    /* Calculate the final parameters for this audio specification */
    SDL_CalculateAudioSpec(&this->spec);

    /* Allocate mixing buffer */
    if (!iscapture) {
        this->hidden->mixlen = this->spec.size;
        this->hidden->mixbuf = (Uint8 *) SDL_malloc(this->hidden->mixlen);
        if (this->hidden->mixbuf == NULL) {
            return SDL_OutOfMemory();
        }
        SDL_memset(this->hidden->mixbuf, this->spec.silence, this->hidden->mixlen);
    }

    #if !SDL_ALSA_NON_BLOCKING
    if (!iscapture) {
        ALSA_snd_pcm_nonblock(pcm_handle, 0);
    }
    #endif

    /* We're ready to rock and roll. :-) */
    return 0;
}

typedef struct ALSA_Device
{
    char *name;
    SDL_bool iscapture;
    struct ALSA_Device *next;
} ALSA_Device;

static void
add_device(const int iscapture, const char *name, void *hint, ALSA_Device **pSeen)
{
    ALSA_Device *dev = SDL_malloc(sizeof (ALSA_Device));
    char *desc;
    char *handle = NULL;
    char *ptr;

    if (!dev) {
        return;
    }

    /* Not all alsa devices are enumerable via snd_device_name_get_hint
       (i.e. bluetooth devices).  Therefore if hint is passed in to this
       function as  NULL, assume name contains desc.
       Make sure not to free the storage associated with desc in this case */
    if (hint) {
        desc = ALSA_snd_device_name_get_hint(hint, "DESC");
        if (!desc) {
            SDL_free(dev);
            return;
        }
    } else {
        desc = (char *) name;
    }

    SDL_assert(name != NULL);

    /* some strings have newlines, like "HDA NVidia, HDMI 0\nHDMI Audio Output".
       just chop the extra lines off, this seems to get a reasonable device
       name without extra details. */
    if ((ptr = strchr(desc, '\n')) != NULL) {
        *ptr = '\0';
    }

    /*printf("ALSA: adding %s device '%s' (%s)\n", iscapture ? "capture" : "output", name, desc);*/

    handle = SDL_strdup(name);
    if (!handle) {
        if (hint) {
            free(desc);
        }
        SDL_free(dev);
        return;
    }

    /* Note that spec is NULL, because we are required to open the device before
     * acquiring the mix format, making this information inaccessible at
     * enumeration time
     */
    SDL_AddAudioDevice(iscapture, desc, NULL, handle);
    if (hint)
        free(desc);
    dev->name = handle;
    dev->iscapture = iscapture;
    dev->next = *pSeen;
    *pSeen = dev;
}


static SDL_atomic_t ALSA_hotplug_shutdown;
static SDL_Thread *ALSA_hotplug_thread;

static int SDLCALL
ALSA_HotplugThread(void *arg)
{
    SDL_sem *first_run_semaphore = (SDL_sem *) arg;
    ALSA_Device *devices = NULL;
    ALSA_Device *next;
    ALSA_Device *dev;
    Uint32 ticks;

    SDL_SetThreadPriority(SDL_THREAD_PRIORITY_LOW);

    while (!SDL_AtomicGet(&ALSA_hotplug_shutdown)) {
        void **hints = NULL;
        ALSA_Device *unseen;
        ALSA_Device *seen;
        ALSA_Device *prev;

        if (ALSA_snd_device_name_hint(-1, "pcm", &hints) == 0) {
            int i, j;
            const char *match = NULL;
            int bestmatch = 0xFFFF;
            size_t match_len = 0;
            int defaultdev = -1;
            static const char * const prefixes[] = {
                "hw:", "sysdefault:", "default:", NULL
            };

            unseen = devices;
            seen = NULL;
            /* Apparently there are several different ways that ALSA lists
               actual hardware. It could be prefixed with "hw:" or "default:"
               or "sysdefault:" and maybe others. Go through the list and see
               if we can find a preferred prefix for the system. */
            for (i = 0; hints[i]; i++) {
                char *name = ALSA_snd_device_name_get_hint(hints[i], "NAME");
                if (!name) {
                    continue;
                }

                /* full name, not a prefix */
                if ((defaultdev == -1) && (SDL_strcmp(name, "default") == 0)) {
                    defaultdev = i;
                }

                for (j = 0; prefixes[j]; j++) {
                    const char *prefix = prefixes[j];
                    const size_t prefixlen = SDL_strlen(prefix);
                    if (SDL_strncmp(name, prefix, prefixlen) == 0) {
                        if (j < bestmatch) {
                            bestmatch = j;
                            match = prefix;
                            match_len = prefixlen;
                        }
                    }
                }

                free(name);
            }

            /* look through the list of device names to find matches */
            for (i = 0; hints[i]; i++) {
                char *name;

                /* if we didn't find a device name prefix we like at all... */
                if ((!match) && (defaultdev != i)) {
                    continue;  /* ...skip anything that isn't the default device. */
                }

                name = ALSA_snd_device_name_get_hint(hints[i], "NAME");
                if (!name) {
                    continue;
                }

                /* only want physical hardware interfaces */
                if (!match || (SDL_strncmp(name, match, match_len) == 0)) {
                    char *ioid = ALSA_snd_device_name_get_hint(hints[i], "IOID");
                    const SDL_bool isoutput = (ioid == NULL) || (SDL_strcmp(ioid, "Output") == 0);
                    const SDL_bool isinput = (ioid == NULL) || (SDL_strcmp(ioid, "Input") == 0);
                    SDL_bool have_output = SDL_FALSE;
                    SDL_bool have_input = SDL_FALSE;

                    free(ioid);

                    if (!isoutput && !isinput) {
                        free(name);
                        continue;
                    }

                    prev = NULL;
                    for (dev = unseen; dev; dev = next) {
                        next = dev->next;
                        if ( (SDL_strcmp(dev->name, name) == 0) && (((isinput) && dev->iscapture) || ((isoutput) && !dev->iscapture)) ) {
                            if (prev) {
                                prev->next = next;
                            } else {
                                unseen = next;
                            }
                            dev->next = seen;
                            seen = dev;
                            if (isinput) have_input = SDL_TRUE;
                            if (isoutput) have_output = SDL_TRUE;
                        } else {
                            prev = dev;
                        }
                    }

                    if (isinput && !have_input) {
                        add_device(SDL_TRUE, name, hints[i], &seen);
                    }
                    if (isoutput && !have_output) {
                        add_device(SDL_FALSE, name, hints[i], &seen);
                    }
                }

                free(name);
            }

            ALSA_snd_device_name_free_hint(hints);

            devices = seen;   /* now we have a known-good list of attached devices. */

            /* report anything still in unseen as removed. */
            for (dev = unseen; dev; dev = next) {
                /*printf("ALSA: removing usb %s device '%s'\n", dev->iscapture ? "capture" : "output", dev->name);*/
                next = dev->next;
                SDL_RemoveAudioDevice(dev->iscapture, dev->name);
                SDL_free(dev->name);
                SDL_free(dev);
            }
        }

        /* On first run, tell ALSA_DetectDevices() that we have a complete device list so it can return. */
        if (first_run_semaphore) {
            SDL_SemPost(first_run_semaphore);
            first_run_semaphore = NULL;  /* let other thread clean it up. */
        }

        /* Block awhile before checking again, unless we're told to stop. */
        ticks = SDL_GetTicks() + 5000;
        while (!SDL_AtomicGet(&ALSA_hotplug_shutdown) && !SDL_TICKS_PASSED(SDL_GetTicks(), ticks)) {
            SDL_Delay(100);
        }
    }

    /* Shutting down! Clean up any data we've gathered. */
    for (dev = devices; dev; dev = next) {
        /*printf("ALSA: at shutdown, removing %s device '%s'\n", dev->iscapture ? "capture" : "output", dev->name);*/
        next = dev->next;
        SDL_free(dev->name);
        SDL_free(dev);
    }

    return 0;
}

static void
ALSA_DetectDevices(void)
{
    /* Start the device detection thread here, wait for an initial iteration to complete. */
    SDL_sem *semaphore = SDL_CreateSemaphore(0);
    if (!semaphore) {
        return;  /* oh well. */
    }

    SDL_AtomicSet(&ALSA_hotplug_shutdown, 0);

    ALSA_hotplug_thread = SDL_CreateThread(ALSA_HotplugThread, "SDLHotplugALSA", semaphore);
    if (ALSA_hotplug_thread) {
        SDL_SemWait(semaphore);  /* wait for the first iteration to finish. */
    }

    SDL_DestroySemaphore(semaphore);
}

static void
ALSA_Deinitialize(void)
{
    if (ALSA_hotplug_thread != NULL) {
        SDL_AtomicSet(&ALSA_hotplug_shutdown, 1);
        SDL_WaitThread(ALSA_hotplug_thread, NULL);
        ALSA_hotplug_thread = NULL;
    }

    UnloadALSALibrary();
}

static int
ALSA_Init(SDL_AudioDriverImpl * impl)
{
    if (LoadALSALibrary() < 0) {
        return 0;
    }

    /* Set the function pointers */
    impl->DetectDevices = ALSA_DetectDevices;
    impl->OpenDevice = ALSA_OpenDevice;
    impl->WaitDevice = ALSA_WaitDevice;
    impl->GetDeviceBuf = ALSA_GetDeviceBuf;
    impl->PlayDevice = ALSA_PlayDevice;
    impl->CloseDevice = ALSA_CloseDevice;
    impl->Deinitialize = ALSA_Deinitialize;
    impl->CaptureFromDevice = ALSA_CaptureFromDevice;
    impl->FlushCapture = ALSA_FlushCapture;

    impl->HasCaptureSupport = SDL_TRUE;

    return 1;   /* this audio target is available. */
}


AudioBootStrap ALSA_bootstrap = {
    "alsa", "ALSA PCM audio", ALSA_Init, 0
};

#endif /* SDL_AUDIO_DRIVER_ALSA */

/* vi: set ts=4 sw=4 expandtab: */
