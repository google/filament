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

#if SDL_AUDIO_DRIVER_WASAPI

#include "../../core/windows/SDL_windows.h"
#include "SDL_audio.h"
#include "SDL_timer.h"
#include "../SDL_audio_c.h"
#include "../SDL_sysaudio.h"

#define COBJMACROS
#include <mmdeviceapi.h>
#include <audioclient.h>

#include "SDL_wasapi.h"

/* This constant isn't available on MinGW-w64 */
#ifndef AUDCLNT_STREAMFLAGS_RATEADJUST
#define AUDCLNT_STREAMFLAGS_RATEADJUST  0x00100000
#endif

/* these increment as default devices change. Opened default devices pick up changes in their threads. */
SDL_atomic_t WASAPI_DefaultPlaybackGeneration;
SDL_atomic_t WASAPI_DefaultCaptureGeneration;

/* This is a list of device id strings we have inflight, so we have consistent pointers to the same device. */
typedef struct DevIdList
{
    WCHAR *str;
    struct DevIdList *next;
} DevIdList;

static DevIdList *deviceid_list = NULL;

/* Some GUIDs we need to know without linking to libraries that aren't available before Vista. */
static const IID SDL_IID_IAudioRenderClient = { 0xf294acfc, 0x3146, 0x4483,{ 0xa7, 0xbf, 0xad, 0xdc, 0xa7, 0xc2, 0x60, 0xe2 } };
static const IID SDL_IID_IAudioCaptureClient = { 0xc8adbd64, 0xe71e, 0x48a0,{ 0xa4, 0xde, 0x18, 0x5c, 0x39, 0x5c, 0xd3, 0x17 } };
static const GUID SDL_KSDATAFORMAT_SUBTYPE_PCM = { 0x00000001, 0x0000, 0x0010,{ 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };
static const GUID SDL_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = { 0x00000003, 0x0000, 0x0010,{ 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };


void
WASAPI_RemoveDevice(const SDL_bool iscapture, LPCWSTR devid)
{
    DevIdList *i;
    DevIdList *next;
    DevIdList *prev = NULL;
    for (i = deviceid_list; i; i = next) {
        next = i->next;
        if (SDL_wcscmp(i->str, devid) == 0) {
            if (prev) {
                prev->next = next;
            } else {
                deviceid_list = next;
            }
            SDL_RemoveAudioDevice(iscapture, i->str);
            SDL_free(i->str);
            SDL_free(i);
        }
        prev = i;
    }
}

static SDL_AudioFormat
WaveFormatToSDLFormat(WAVEFORMATEX *waveformat)
{
    if ((waveformat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) && (waveformat->wBitsPerSample == 32)) {
        return AUDIO_F32SYS;
    } else if ((waveformat->wFormatTag == WAVE_FORMAT_PCM) && (waveformat->wBitsPerSample == 16)) {
        return AUDIO_S16SYS;
    } else if ((waveformat->wFormatTag == WAVE_FORMAT_PCM) && (waveformat->wBitsPerSample == 32)) {
        return AUDIO_S32SYS;
    } else if (waveformat->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        const WAVEFORMATEXTENSIBLE *ext = (const WAVEFORMATEXTENSIBLE *) waveformat;
        if ((SDL_memcmp(&ext->SubFormat, &SDL_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, sizeof (GUID)) == 0) && (waveformat->wBitsPerSample == 32)) {
            return AUDIO_F32SYS;
        } else if ((SDL_memcmp(&ext->SubFormat, &SDL_KSDATAFORMAT_SUBTYPE_PCM, sizeof (GUID)) == 0) && (waveformat->wBitsPerSample == 16)) {
            return AUDIO_S16SYS;
        } else if ((SDL_memcmp(&ext->SubFormat, &SDL_KSDATAFORMAT_SUBTYPE_PCM, sizeof (GUID)) == 0) && (waveformat->wBitsPerSample == 32)) {
            return AUDIO_S32SYS;
        }
    }
    return 0;
}

void
WASAPI_AddDevice(const SDL_bool iscapture, const char *devname, WAVEFORMATEXTENSIBLE *fmt, LPCWSTR devid)
{
    DevIdList *devidlist;
    SDL_AudioSpec spec;

    /* You can have multiple endpoints on a device that are mutually exclusive ("Speakers" vs "Line Out" or whatever).
       In a perfect world, things that are unplugged won't be in this collection. The only gotcha is probably for
       phones and tablets, where you might have an internal speaker and a headphone jack and expect both to be
       available and switch automatically. (!!! FIXME...?) */

    /* see if we already have this one. */
    for (devidlist = deviceid_list; devidlist; devidlist = devidlist->next) {
        if (SDL_wcscmp(devidlist->str, devid) == 0) {
            return;  /* we already have this. */
        }
    }

    devidlist = (DevIdList *) SDL_malloc(sizeof (*devidlist));
    if (!devidlist) {
        return;  /* oh well. */
    }

    devid = SDL_wcsdup(devid);
    if (!devid) {
        SDL_free(devidlist);
        return;  /* oh well. */
    }

    devidlist->str = (WCHAR *) devid;
    devidlist->next = deviceid_list;
    deviceid_list = devidlist;

    SDL_zero(spec);
    spec.channels = (Uint8)fmt->Format.nChannels;
    spec.freq = fmt->Format.nSamplesPerSec;
    spec.format = WaveFormatToSDLFormat((WAVEFORMATEX *) fmt);
    SDL_AddAudioDevice(iscapture, devname, &spec, (void *) devid);
}

static void
WASAPI_DetectDevices(void)
{
    WASAPI_EnumerateEndpoints();
}

static SDL_INLINE SDL_bool
WasapiFailed(_THIS, const HRESULT err)
{
    if (err == S_OK) {
        return SDL_FALSE;
    }

    if (err == AUDCLNT_E_DEVICE_INVALIDATED) {
        this->hidden->device_lost = SDL_TRUE;
    } else if (SDL_AtomicGet(&this->enabled)) {
        IAudioClient_Stop(this->hidden->client);
        SDL_OpenedAudioDeviceDisconnected(this);
        SDL_assert(!SDL_AtomicGet(&this->enabled));
    }

    return SDL_TRUE;
}

static int
UpdateAudioStream(_THIS, const SDL_AudioSpec *oldspec)
{
    /* Since WASAPI requires us to handle all audio conversion, and our
       device format might have changed, we might have to add/remove/change
       the audio stream that the higher level uses to convert data, so
       SDL keeps firing the callback as if nothing happened here. */

    if ( (this->callbackspec.channels == this->spec.channels) &&
         (this->callbackspec.format == this->spec.format) &&
         (this->callbackspec.freq == this->spec.freq) &&
         (this->callbackspec.samples == this->spec.samples) ) {
        /* no need to buffer/convert in an AudioStream! */
        SDL_FreeAudioStream(this->stream);
        this->stream = NULL;
    } else if ( (oldspec->channels == this->spec.channels) &&
         (oldspec->format == this->spec.format) &&
         (oldspec->freq == this->spec.freq) ) {
        /* The existing audio stream is okay to keep using. */
    } else {
        /* replace the audiostream for new format */
        SDL_FreeAudioStream(this->stream);
        if (this->iscapture) {
            this->stream = SDL_NewAudioStream(this->spec.format,
                                this->spec.channels, this->spec.freq,
                                this->callbackspec.format,
                                this->callbackspec.channels,
                                this->callbackspec.freq);
        } else {
            this->stream = SDL_NewAudioStream(this->callbackspec.format,
                                this->callbackspec.channels,
                                this->callbackspec.freq, this->spec.format,
                                this->spec.channels, this->spec.freq);
        }

        if (!this->stream) {
            return -1;
        }
    }

    /* make sure our scratch buffer can cover the new device spec. */
    if (this->spec.size > this->work_buffer_len) {
        Uint8 *ptr = (Uint8 *) SDL_realloc(this->work_buffer, this->spec.size);
        if (ptr == NULL) {
            return SDL_OutOfMemory();
        }
        this->work_buffer = ptr;
        this->work_buffer_len = this->spec.size;
    }

    return 0;
}


static void ReleaseWasapiDevice(_THIS);

static SDL_bool
RecoverWasapiDevice(_THIS)
{
    ReleaseWasapiDevice(this);  /* dump the lost device's handles. */

    if (this->hidden->default_device_generation) {
        this->hidden->default_device_generation = SDL_AtomicGet(this->iscapture ?  &WASAPI_DefaultCaptureGeneration : &WASAPI_DefaultPlaybackGeneration);
    }

    /* this can fail for lots of reasons, but the most likely is we had a
       non-default device that was disconnected, so we can't recover. Default
       devices try to reinitialize whatever the new default is, so it's more
       likely to carry on here, but this handles a non-default device that
       simply had its format changed in the Windows Control Panel. */
    if (WASAPI_ActivateDevice(this, SDL_TRUE) == -1) {
        SDL_OpenedAudioDeviceDisconnected(this);
        return SDL_FALSE;
    }

    this->hidden->device_lost = SDL_FALSE;

    return SDL_TRUE;  /* okay, carry on with new device details! */
}

static SDL_bool
RecoverWasapiIfLost(_THIS)
{
    const int generation = this->hidden->default_device_generation;
    SDL_bool lost = this->hidden->device_lost;

    if (!SDL_AtomicGet(&this->enabled)) {
        return SDL_FALSE;  /* already failed. */
    }

    if (!this->hidden->client) {
        return SDL_TRUE;  /* still waiting for activation. */
    }

    if (!lost && (generation > 0)) { /* is a default device? */
        const int newgen = SDL_AtomicGet(this->iscapture ? &WASAPI_DefaultCaptureGeneration : &WASAPI_DefaultPlaybackGeneration);
        if (generation != newgen) {  /* the desired default device was changed, jump over to it. */
            lost = SDL_TRUE;
        }
    }

    return lost ? RecoverWasapiDevice(this) : SDL_TRUE;
}

static Uint8 *
WASAPI_GetDeviceBuf(_THIS)
{
    /* get an endpoint buffer from WASAPI. */
    BYTE *buffer = NULL;

    while (RecoverWasapiIfLost(this) && this->hidden->render) {
        if (!WasapiFailed(this, IAudioRenderClient_GetBuffer(this->hidden->render, this->spec.samples, &buffer))) {
            return (Uint8 *) buffer;
        }
        SDL_assert(buffer == NULL);
    }

    return (Uint8 *) buffer;
}

static void
WASAPI_PlayDevice(_THIS)
{
    if (this->hidden->render != NULL) {  /* definitely activated? */
        /* WasapiFailed() will mark the device for reacquisition or removal elsewhere. */
        WasapiFailed(this, IAudioRenderClient_ReleaseBuffer(this->hidden->render, this->spec.samples, 0));
    }
}

static void
WASAPI_WaitDevice(_THIS)
{
    while (RecoverWasapiIfLost(this) && this->hidden->client && this->hidden->event) {
        DWORD waitResult = WaitForSingleObjectEx(this->hidden->event, 200, FALSE);
        if (waitResult == WAIT_OBJECT_0) {
            const UINT32 maxpadding = this->spec.samples;
            UINT32 padding = 0;
            if (!WasapiFailed(this, IAudioClient_GetCurrentPadding(this->hidden->client, &padding))) {
                /*SDL_Log("WASAPI EVENT! padding=%u maxpadding=%u", (unsigned int)padding, (unsigned int)maxpadding);*/
                if (this->iscapture) {
                    if (padding > 0) {
                        break;
                    }
                } else {
                    if (padding <= maxpadding) {
                        break;
                    }
                }
            }
        } else if (waitResult != WAIT_TIMEOUT) {
            /*SDL_Log("WASAPI FAILED EVENT!");*/
            IAudioClient_Stop(this->hidden->client);
            SDL_OpenedAudioDeviceDisconnected(this);
        }
    }
}

static int
WASAPI_CaptureFromDevice(_THIS, void *buffer, int buflen)
{
    SDL_AudioStream *stream = this->hidden->capturestream;
    const int avail = SDL_AudioStreamAvailable(stream);
    if (avail > 0) {
        const int cpy = SDL_min(buflen, avail);
        SDL_AudioStreamGet(stream, buffer, cpy);
        return cpy;
    }

    while (RecoverWasapiIfLost(this)) {
        HRESULT ret;
        BYTE *ptr = NULL;
        UINT32 frames = 0;
        DWORD flags = 0;

        /* uhoh, client isn't activated yet, just return silence. */
        if (!this->hidden->capture) {
            /* Delay so we run at about the speed that audio would be arriving. */
            SDL_Delay(((this->spec.samples * 1000) / this->spec.freq));
            SDL_memset(buffer, this->spec.silence, buflen);
            return buflen;
        }

        ret = IAudioCaptureClient_GetBuffer(this->hidden->capture, &ptr, &frames, &flags, NULL, NULL);
        if (ret != AUDCLNT_S_BUFFER_EMPTY) {
            WasapiFailed(this, ret); /* mark device lost/failed if necessary. */
        }

        if ((ret == AUDCLNT_S_BUFFER_EMPTY) || !frames) {
            WASAPI_WaitDevice(this);
        } else if (ret == S_OK) {
            const int total = ((int) frames) * this->hidden->framesize;
            const int cpy = SDL_min(buflen, total);
            const int leftover = total - cpy;
            const SDL_bool silent = (flags & AUDCLNT_BUFFERFLAGS_SILENT) ? SDL_TRUE : SDL_FALSE;

            if (silent) {
                SDL_memset(buffer, this->spec.silence, cpy);
            } else {
                SDL_memcpy(buffer, ptr, cpy);
            }
            
            if (leftover > 0) {
                ptr += cpy;
                if (silent) {
                    SDL_memset(ptr, this->spec.silence, leftover);  /* I guess this is safe? */
                }

                if (SDL_AudioStreamPut(stream, ptr, leftover) == -1) {
                    return -1;  /* uhoh, out of memory, etc. Kill device.  :( */
                }
            }

            ret = IAudioCaptureClient_ReleaseBuffer(this->hidden->capture, frames);
            WasapiFailed(this, ret); /* mark device lost/failed if necessary. */

            return cpy;
        }
    }

    return -1;  /* unrecoverable error. */
}

static void
WASAPI_FlushCapture(_THIS)
{
    BYTE *ptr = NULL;
    UINT32 frames = 0;
    DWORD flags = 0;

    if (!this->hidden->capture) {
        return;  /* not activated yet? */
    }

    /* just read until we stop getting packets, throwing them away. */
    while (SDL_TRUE) {
        const HRESULT ret = IAudioCaptureClient_GetBuffer(this->hidden->capture, &ptr, &frames, &flags, NULL, NULL);
        if (ret == AUDCLNT_S_BUFFER_EMPTY) {
            break;  /* no more buffered data; we're done. */
        } else if (WasapiFailed(this, ret)) {
            break;  /* failed for some other reason, abort. */
        } else if (WasapiFailed(this, IAudioCaptureClient_ReleaseBuffer(this->hidden->capture, frames))) {
            break;  /* something broke. */
        }
    }
    SDL_AudioStreamClear(this->hidden->capturestream);
}

static void
ReleaseWasapiDevice(_THIS)
{
    if (this->hidden->client) {
        IAudioClient_Stop(this->hidden->client);
        IAudioClient_SetEventHandle(this->hidden->client, NULL);
        IAudioClient_Release(this->hidden->client);
        this->hidden->client = NULL;
    }

    if (this->hidden->render) {
        IAudioRenderClient_Release(this->hidden->render);
        this->hidden->render = NULL;
    }

    if (this->hidden->capture) {
        IAudioCaptureClient_Release(this->hidden->capture);
        this->hidden->capture = NULL;
    }

    if (this->hidden->waveformat) {
        CoTaskMemFree(this->hidden->waveformat);
        this->hidden->waveformat = NULL;
    }

    if (this->hidden->capturestream) {
        SDL_FreeAudioStream(this->hidden->capturestream);
        this->hidden->capturestream = NULL;
    }

    if (this->hidden->activation_handler) {
        WASAPI_PlatformDeleteActivationHandler(this->hidden->activation_handler);
        this->hidden->activation_handler = NULL;
    }

    if (this->hidden->event) {
        CloseHandle(this->hidden->event);
        this->hidden->event = NULL;
    }
}

static void
WASAPI_CloseDevice(_THIS)
{
    WASAPI_UnrefDevice(this);
}

void
WASAPI_RefDevice(_THIS)
{
    SDL_AtomicIncRef(&this->hidden->refcount);
}

void
WASAPI_UnrefDevice(_THIS)
{
    if (!SDL_AtomicDecRef(&this->hidden->refcount)) {
        return;
    }

    /* actual closing happens here. */

    /* don't touch this->hidden->task in here; it has to be reverted from
       our callback thread. We do that in WASAPI_ThreadDeinit().
       (likewise for this->hidden->coinitialized). */
    ReleaseWasapiDevice(this);
    SDL_free(this->hidden->devid);
    SDL_free(this->hidden);
}

/* This is called once a device is activated, possibly asynchronously. */
int
WASAPI_PrepDevice(_THIS, const SDL_bool updatestream)
{
    /* !!! FIXME: we could request an exclusive mode stream, which is lower latency;
       !!!  it will write into the kernel's audio buffer directly instead of
       !!!  shared memory that a user-mode mixer then writes to the kernel with
       !!!  everything else. Doing this means any other sound using this device will
       !!!  stop playing, including the user's MP3 player and system notification
       !!!  sounds. You'd probably need to release the device when the app isn't in
       !!!  the foreground, to be a good citizen of the system. It's doable, but it's
       !!!  more work and causes some annoyances, and I don't know what the latency
       !!!  wins actually look like. Maybe add a hint to force exclusive mode at
       !!!  some point. To be sure, defaulting to shared mode is the right thing to
       !!!  do in any case. */
    const SDL_AudioSpec oldspec = this->spec;
    const AUDCLNT_SHAREMODE sharemode = AUDCLNT_SHAREMODE_SHARED;
    UINT32 bufsize = 0;  /* this is in sample frames, not samples, not bytes. */
    REFERENCE_TIME default_period = 0;
    IAudioClient *client = this->hidden->client;
    IAudioRenderClient *render = NULL;
    IAudioCaptureClient *capture = NULL;
    WAVEFORMATEX *waveformat = NULL;
    SDL_AudioFormat test_format = SDL_FirstAudioFormat(this->spec.format);
    SDL_AudioFormat wasapi_format = 0;
    SDL_bool valid_format = SDL_FALSE;
    HRESULT ret = S_OK;
    DWORD streamflags = 0;

    SDL_assert(client != NULL);

#ifdef __WINRT__  /* CreateEventEx() arrived in Vista, so we need an #ifdef for XP. */
    this->hidden->event = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);
#else
    this->hidden->event = CreateEventW(NULL, 0, 0, NULL);
#endif

    if (this->hidden->event == NULL) {
        return WIN_SetError("WASAPI can't create an event handle");
    }

    ret = IAudioClient_GetMixFormat(client, &waveformat);
    if (FAILED(ret)) {
        return WIN_SetErrorFromHRESULT("WASAPI can't determine mix format", ret);
    }

    SDL_assert(waveformat != NULL);
    this->hidden->waveformat = waveformat;

    this->spec.channels = (Uint8) waveformat->nChannels;

    /* Make sure we have a valid format that we can convert to whatever WASAPI wants. */
    wasapi_format = WaveFormatToSDLFormat(waveformat);

    while ((!valid_format) && (test_format)) {
        if (test_format == wasapi_format) {
            this->spec.format = test_format;
            valid_format = SDL_TRUE;
            break;
        }
        test_format = SDL_NextAudioFormat();
    }

    if (!valid_format) {
        return SDL_SetError("WASAPI: Unsupported audio format");
    }

    ret = IAudioClient_GetDevicePeriod(client, &default_period, NULL);
    if (FAILED(ret)) {
        return WIN_SetErrorFromHRESULT("WASAPI can't determine minimum device period", ret);
    }

#if 1  /* we're getting reports that WASAPI's resampler introduces distortions, so it's disabled for now. --ryan. */
    this->spec.freq = waveformat->nSamplesPerSec;  /* force sampling rate so our resampler kicks in, if necessary. */
#else
    /* favor WASAPI's resampler over our own, in Win7+. */
    if (this->spec.freq != waveformat->nSamplesPerSec) {
        /* RATEADJUST only works with output devices in share mode, and is available in Win7 and later.*/
        if (WIN_IsWindows7OrGreater() && !this->iscapture && (sharemode == AUDCLNT_SHAREMODE_SHARED)) {
            streamflags |= AUDCLNT_STREAMFLAGS_RATEADJUST;
            waveformat->nSamplesPerSec = this->spec.freq;
            waveformat->nAvgBytesPerSec = waveformat->nSamplesPerSec * waveformat->nChannels * (waveformat->wBitsPerSample / 8);
        } else {
            this->spec.freq = waveformat->nSamplesPerSec;  /* force sampling rate so our resampler kicks in. */
        }
    }
#endif

    streamflags |= AUDCLNT_STREAMFLAGS_EVENTCALLBACK;
    ret = IAudioClient_Initialize(client, sharemode, streamflags, 0, 0, waveformat, NULL);
    if (FAILED(ret)) {
        return WIN_SetErrorFromHRESULT("WASAPI can't initialize audio client", ret);
    }

    ret = IAudioClient_SetEventHandle(client, this->hidden->event);
    if (FAILED(ret)) {
        return WIN_SetErrorFromHRESULT("WASAPI can't set event handle", ret);
    }

    ret = IAudioClient_GetBufferSize(client, &bufsize);
    if (FAILED(ret)) {
        return WIN_SetErrorFromHRESULT("WASAPI can't determine buffer size", ret);
    }

    /* Match the callback size to the period size to cut down on the number of
       interrupts waited for in each call to WaitDevice */
    {
        const float period_millis = default_period / 10000.0f;
        const float period_frames = period_millis * this->spec.freq / 1000.0f;
        this->spec.samples = (Uint16)SDL_ceilf(period_frames);
    }

    /* Update the fragment size as size in bytes */
    SDL_CalculateAudioSpec(&this->spec);

    this->hidden->framesize = (SDL_AUDIO_BITSIZE(this->spec.format) / 8) * this->spec.channels;

    if (this->iscapture) {
        this->hidden->capturestream = SDL_NewAudioStream(this->spec.format, this->spec.channels, this->spec.freq, this->spec.format, this->spec.channels, this->spec.freq);
        if (!this->hidden->capturestream) {
            return -1;  /* already set SDL_Error */
        }

        ret = IAudioClient_GetService(client, &SDL_IID_IAudioCaptureClient, (void**) &capture);
        if (FAILED(ret)) {
            return WIN_SetErrorFromHRESULT("WASAPI can't get capture client service", ret);
        }

        SDL_assert(capture != NULL);
        this->hidden->capture = capture;
        ret = IAudioClient_Start(client);
        if (FAILED(ret)) {
            return WIN_SetErrorFromHRESULT("WASAPI can't start capture", ret);
        }

        WASAPI_FlushCapture(this);  /* MSDN says you should flush capture endpoint right after startup. */
    } else {
        ret = IAudioClient_GetService(client, &SDL_IID_IAudioRenderClient, (void**) &render);
        if (FAILED(ret)) {
            return WIN_SetErrorFromHRESULT("WASAPI can't get render client service", ret);
        }

        SDL_assert(render != NULL);
        this->hidden->render = render;
        ret = IAudioClient_Start(client);
        if (FAILED(ret)) {
            return WIN_SetErrorFromHRESULT("WASAPI can't start playback", ret);
        }
    }

    if (updatestream) {
        if (UpdateAudioStream(this, &oldspec) == -1) {
            return -1;
        }
    }

    return 0;  /* good to go. */
}


static int
WASAPI_OpenDevice(_THIS, void *handle, const char *devname, int iscapture)
{
    LPCWSTR devid = (LPCWSTR) handle;

    /* Initialize all variables that we clean on shutdown */
    this->hidden = (struct SDL_PrivateAudioData *)
        SDL_malloc((sizeof *this->hidden));
    if (this->hidden == NULL) {
        return SDL_OutOfMemory();
    }
    SDL_zerop(this->hidden);

    WASAPI_RefDevice(this);   /* so CloseDevice() will unref to zero. */

    if (!devid) {  /* is default device? */
        this->hidden->default_device_generation = SDL_AtomicGet(iscapture ? &WASAPI_DefaultCaptureGeneration : &WASAPI_DefaultPlaybackGeneration);
    } else {
        this->hidden->devid = SDL_wcsdup(devid);
        if (!this->hidden->devid) {
            return SDL_OutOfMemory();
        }
    }

    if (WASAPI_ActivateDevice(this, SDL_FALSE) == -1) {
        return -1;  /* already set error. */
    }

    /* Ready, but waiting for async device activation.
       Until activation is successful, we will report silence from capture
       devices and ignore data on playback devices.
       Also, since we don't know the _actual_ device format until after
       activation, we let the app have whatever it asks for. We set up
       an SDL_AudioStream to convert, if necessary, once the activation
       completes. */

    return 0;
}

static void
WASAPI_ThreadInit(_THIS)
{
    WASAPI_PlatformThreadInit(this);
}

static void
WASAPI_ThreadDeinit(_THIS)
{
    WASAPI_PlatformThreadDeinit(this);
}

void
WASAPI_BeginLoopIteration(_THIS)
{
	/* no-op. */
}

static void
WASAPI_Deinitialize(void)
{
    DevIdList *devidlist;
    DevIdList *next;

    WASAPI_PlatformDeinit();

    for (devidlist = deviceid_list; devidlist; devidlist = next) {
        next = devidlist->next;
        SDL_free(devidlist->str);
        SDL_free(devidlist);
    }
    deviceid_list = NULL;
}

static int
WASAPI_Init(SDL_AudioDriverImpl * impl)
{
    SDL_AtomicSet(&WASAPI_DefaultPlaybackGeneration, 1);
    SDL_AtomicSet(&WASAPI_DefaultCaptureGeneration, 1);

    if (WASAPI_PlatformInit() == -1) {
        return 0;
    }

    /* Set the function pointers */
    impl->DetectDevices = WASAPI_DetectDevices;
    impl->ThreadInit = WASAPI_ThreadInit;
    impl->ThreadDeinit = WASAPI_ThreadDeinit;
    impl->BeginLoopIteration = WASAPI_BeginLoopIteration;
    impl->OpenDevice = WASAPI_OpenDevice;
    impl->PlayDevice = WASAPI_PlayDevice;
    impl->WaitDevice = WASAPI_WaitDevice;
    impl->GetDeviceBuf = WASAPI_GetDeviceBuf;
    impl->CaptureFromDevice = WASAPI_CaptureFromDevice;
    impl->FlushCapture = WASAPI_FlushCapture;
    impl->CloseDevice = WASAPI_CloseDevice;
    impl->Deinitialize = WASAPI_Deinitialize;
    impl->HasCaptureSupport = 1;

    return 1;   /* this audio target is available. */
}

AudioBootStrap WASAPI_bootstrap = {
    "wasapi", "WASAPI", WASAPI_Init, 0
};

#endif  /* SDL_AUDIO_DRIVER_WASAPI */

/* vi: set ts=4 sw=4 expandtab: */
