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

/*
  The PulseAudio target for SDL 1.3 is based on the 1.3 arts target, with
   the appropriate parts replaced with the 1.2 PulseAudio target code. This
   was the cleanest way to move it to 1.3. The 1.2 target was written by
   Stéphan Kochen: stephan .a.t. kochen.nl
*/
#include "../../SDL_internal.h"
#include "SDL_assert.h"

#if SDL_AUDIO_DRIVER_PULSEAUDIO

/* Allow access to a raw mixing buffer */

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#include <unistd.h>
#include <sys/types.h>
#include <pulse/pulseaudio.h>

#include "SDL_timer.h"
#include "SDL_audio.h"
#include "../SDL_audio_c.h"
#include "SDL_pulseaudio.h"
#include "SDL_loadso.h"
#include "../../thread/SDL_systhread.h"

#if (PA_API_VERSION < 12)
/** Return non-zero if the passed state is one of the connected states */
static SDL_INLINE int PA_CONTEXT_IS_GOOD(pa_context_state_t x) {
    return
        x == PA_CONTEXT_CONNECTING ||
        x == PA_CONTEXT_AUTHORIZING ||
        x == PA_CONTEXT_SETTING_NAME ||
        x == PA_CONTEXT_READY;
}
/** Return non-zero if the passed state is one of the connected states */
static SDL_INLINE int PA_STREAM_IS_GOOD(pa_stream_state_t x) {
    return
        x == PA_STREAM_CREATING ||
        x == PA_STREAM_READY;
}
#endif /* pulseaudio <= 0.9.10 */


static const char *(*PULSEAUDIO_pa_get_library_version) (void);
static pa_channel_map *(*PULSEAUDIO_pa_channel_map_init_auto) (
    pa_channel_map *, unsigned, pa_channel_map_def_t);
static const char * (*PULSEAUDIO_pa_strerror) (int);
static pa_mainloop * (*PULSEAUDIO_pa_mainloop_new) (void);
static pa_mainloop_api * (*PULSEAUDIO_pa_mainloop_get_api) (pa_mainloop *);
static int (*PULSEAUDIO_pa_mainloop_iterate) (pa_mainloop *, int, int *);
static int (*PULSEAUDIO_pa_mainloop_run) (pa_mainloop *, int *);
static void (*PULSEAUDIO_pa_mainloop_quit) (pa_mainloop *, int);
static void (*PULSEAUDIO_pa_mainloop_free) (pa_mainloop *);

static pa_operation_state_t (*PULSEAUDIO_pa_operation_get_state) (
    pa_operation *);
static void (*PULSEAUDIO_pa_operation_cancel) (pa_operation *);
static void (*PULSEAUDIO_pa_operation_unref) (pa_operation *);

static pa_context * (*PULSEAUDIO_pa_context_new) (pa_mainloop_api *,
    const char *);
static int (*PULSEAUDIO_pa_context_connect) (pa_context *, const char *,
    pa_context_flags_t, const pa_spawn_api *);
static pa_operation * (*PULSEAUDIO_pa_context_get_sink_info_list) (pa_context *, pa_sink_info_cb_t, void *);
static pa_operation * (*PULSEAUDIO_pa_context_get_source_info_list) (pa_context *, pa_source_info_cb_t, void *);
static pa_operation * (*PULSEAUDIO_pa_context_get_sink_info_by_index) (pa_context *, uint32_t, pa_sink_info_cb_t, void *);
static pa_operation * (*PULSEAUDIO_pa_context_get_source_info_by_index) (pa_context *, uint32_t, pa_source_info_cb_t, void *);
static pa_context_state_t (*PULSEAUDIO_pa_context_get_state) (pa_context *);
static pa_operation * (*PULSEAUDIO_pa_context_subscribe) (pa_context *, pa_subscription_mask_t, pa_context_success_cb_t, void *);
static void (*PULSEAUDIO_pa_context_set_subscribe_callback) (pa_context *, pa_context_subscribe_cb_t, void *);
static void (*PULSEAUDIO_pa_context_disconnect) (pa_context *);
static void (*PULSEAUDIO_pa_context_unref) (pa_context *);

static pa_stream * (*PULSEAUDIO_pa_stream_new) (pa_context *, const char *,
    const pa_sample_spec *, const pa_channel_map *);
static int (*PULSEAUDIO_pa_stream_connect_playback) (pa_stream *, const char *,
    const pa_buffer_attr *, pa_stream_flags_t, pa_cvolume *, pa_stream *);
static int (*PULSEAUDIO_pa_stream_connect_record) (pa_stream *, const char *,
    const pa_buffer_attr *, pa_stream_flags_t);
static pa_stream_state_t (*PULSEAUDIO_pa_stream_get_state) (pa_stream *);
static size_t (*PULSEAUDIO_pa_stream_writable_size) (pa_stream *);
static size_t (*PULSEAUDIO_pa_stream_readable_size) (pa_stream *);
static int (*PULSEAUDIO_pa_stream_write) (pa_stream *, const void *, size_t,
    pa_free_cb_t, int64_t, pa_seek_mode_t);
static pa_operation * (*PULSEAUDIO_pa_stream_drain) (pa_stream *,
    pa_stream_success_cb_t, void *);
static int (*PULSEAUDIO_pa_stream_peek) (pa_stream *, const void **, size_t *);
static int (*PULSEAUDIO_pa_stream_drop) (pa_stream *);
static pa_operation * (*PULSEAUDIO_pa_stream_flush)	(pa_stream *,
    pa_stream_success_cb_t, void *);
static int (*PULSEAUDIO_pa_stream_disconnect) (pa_stream *);
static void (*PULSEAUDIO_pa_stream_unref) (pa_stream *);

static int load_pulseaudio_syms(void);


#ifdef SDL_AUDIO_DRIVER_PULSEAUDIO_DYNAMIC

static const char *pulseaudio_library = SDL_AUDIO_DRIVER_PULSEAUDIO_DYNAMIC;
static void *pulseaudio_handle = NULL;

static int
load_pulseaudio_sym(const char *fn, void **addr)
{
    *addr = SDL_LoadFunction(pulseaudio_handle, fn);
    if (*addr == NULL) {
        /* Don't call SDL_SetError(): SDL_LoadFunction already did. */
        return 0;
    }

    return 1;
}

/* cast funcs to char* first, to please GCC's strict aliasing rules. */
#define SDL_PULSEAUDIO_SYM(x) \
    if (!load_pulseaudio_sym(#x, (void **) (char *) &PULSEAUDIO_##x)) return -1

static void
UnloadPulseAudioLibrary(void)
{
    if (pulseaudio_handle != NULL) {
        SDL_UnloadObject(pulseaudio_handle);
        pulseaudio_handle = NULL;
    }
}

static int
LoadPulseAudioLibrary(void)
{
    int retval = 0;
    if (pulseaudio_handle == NULL) {
        pulseaudio_handle = SDL_LoadObject(pulseaudio_library);
        if (pulseaudio_handle == NULL) {
            retval = -1;
            /* Don't call SDL_SetError(): SDL_LoadObject already did. */
        } else {
            retval = load_pulseaudio_syms();
            if (retval < 0) {
                UnloadPulseAudioLibrary();
            }
        }
    }
    return retval;
}

#else

#define SDL_PULSEAUDIO_SYM(x) PULSEAUDIO_##x = x

static void
UnloadPulseAudioLibrary(void)
{
}

static int
LoadPulseAudioLibrary(void)
{
    load_pulseaudio_syms();
    return 0;
}

#endif /* SDL_AUDIO_DRIVER_PULSEAUDIO_DYNAMIC */


static int
load_pulseaudio_syms(void)
{
    SDL_PULSEAUDIO_SYM(pa_get_library_version);
    SDL_PULSEAUDIO_SYM(pa_mainloop_new);
    SDL_PULSEAUDIO_SYM(pa_mainloop_get_api);
    SDL_PULSEAUDIO_SYM(pa_mainloop_iterate);
    SDL_PULSEAUDIO_SYM(pa_mainloop_run);
    SDL_PULSEAUDIO_SYM(pa_mainloop_quit);
    SDL_PULSEAUDIO_SYM(pa_mainloop_free);
    SDL_PULSEAUDIO_SYM(pa_operation_get_state);
    SDL_PULSEAUDIO_SYM(pa_operation_cancel);
    SDL_PULSEAUDIO_SYM(pa_operation_unref);
    SDL_PULSEAUDIO_SYM(pa_context_new);
    SDL_PULSEAUDIO_SYM(pa_context_connect);
    SDL_PULSEAUDIO_SYM(pa_context_get_sink_info_list);
    SDL_PULSEAUDIO_SYM(pa_context_get_source_info_list);
    SDL_PULSEAUDIO_SYM(pa_context_get_sink_info_by_index);
    SDL_PULSEAUDIO_SYM(pa_context_get_source_info_by_index);
    SDL_PULSEAUDIO_SYM(pa_context_get_state);
    SDL_PULSEAUDIO_SYM(pa_context_subscribe);
    SDL_PULSEAUDIO_SYM(pa_context_set_subscribe_callback);
    SDL_PULSEAUDIO_SYM(pa_context_disconnect);
    SDL_PULSEAUDIO_SYM(pa_context_unref);
    SDL_PULSEAUDIO_SYM(pa_stream_new);
    SDL_PULSEAUDIO_SYM(pa_stream_connect_playback);
    SDL_PULSEAUDIO_SYM(pa_stream_connect_record);
    SDL_PULSEAUDIO_SYM(pa_stream_get_state);
    SDL_PULSEAUDIO_SYM(pa_stream_writable_size);
    SDL_PULSEAUDIO_SYM(pa_stream_readable_size);
    SDL_PULSEAUDIO_SYM(pa_stream_write);
    SDL_PULSEAUDIO_SYM(pa_stream_drain);
    SDL_PULSEAUDIO_SYM(pa_stream_disconnect);
    SDL_PULSEAUDIO_SYM(pa_stream_peek);
    SDL_PULSEAUDIO_SYM(pa_stream_drop);
    SDL_PULSEAUDIO_SYM(pa_stream_flush);
    SDL_PULSEAUDIO_SYM(pa_stream_unref);
    SDL_PULSEAUDIO_SYM(pa_channel_map_init_auto);
    SDL_PULSEAUDIO_SYM(pa_strerror);
    return 0;
}

static SDL_INLINE int
squashVersion(const int major, const int minor, const int patch)
{
    return ((major & 0xFF) << 16) | ((minor & 0xFF) << 8) | (patch & 0xFF);
}

/* Workaround for older pulse: pa_context_new() must have non-NULL appname */
static const char *
getAppName(void)
{
    const char *verstr = PULSEAUDIO_pa_get_library_version();
    if (verstr != NULL) {
        int maj, min, patch;
        if (SDL_sscanf(verstr, "%d.%d.%d", &maj, &min, &patch) == 3) {
            if (squashVersion(maj, min, patch) >= squashVersion(0, 9, 15)) {
                return NULL;  /* 0.9.15+ handles NULL correctly. */
            }
        }
    }
    return "SDL Application";  /* oh well. */
}

static void
WaitForPulseOperation(pa_mainloop *mainloop, pa_operation *o)
{
    /* This checks for NO errors currently. Either fix that, check results elsewhere, or do things you don't care about. */
    if (mainloop && o) {
        SDL_bool okay = SDL_TRUE;
        while (okay && (PULSEAUDIO_pa_operation_get_state(o) == PA_OPERATION_RUNNING)) {
            okay = (PULSEAUDIO_pa_mainloop_iterate(mainloop, 1, NULL) >= 0);
        }
        PULSEAUDIO_pa_operation_unref(o);
    }
}

static void
DisconnectFromPulseServer(pa_mainloop *mainloop, pa_context *context)
{
    if (context) {
        PULSEAUDIO_pa_context_disconnect(context);
        PULSEAUDIO_pa_context_unref(context);
    }
    if (mainloop != NULL) {
        PULSEAUDIO_pa_mainloop_free(mainloop);
    }
}

static int
ConnectToPulseServer_Internal(pa_mainloop **_mainloop, pa_context **_context)
{
    pa_mainloop *mainloop = NULL;
    pa_context *context = NULL;
    pa_mainloop_api *mainloop_api = NULL;
    int state = 0;

    *_mainloop = NULL;
    *_context = NULL;

    /* Set up a new main loop */
    if (!(mainloop = PULSEAUDIO_pa_mainloop_new())) {
        return SDL_SetError("pa_mainloop_new() failed");
    }

    *_mainloop = mainloop;

    mainloop_api = PULSEAUDIO_pa_mainloop_get_api(mainloop);
    SDL_assert(mainloop_api);  /* this never fails, right? */

    context = PULSEAUDIO_pa_context_new(mainloop_api, getAppName());
    if (!context) {
        return SDL_SetError("pa_context_new() failed");
    }
    *_context = context;

    /* Connect to the PulseAudio server */
    if (PULSEAUDIO_pa_context_connect(context, NULL, 0, NULL) < 0) {
        return SDL_SetError("Could not setup connection to PulseAudio");
    }

    do {
        if (PULSEAUDIO_pa_mainloop_iterate(mainloop, 1, NULL) < 0) {
            return SDL_SetError("pa_mainloop_iterate() failed");
        }
        state = PULSEAUDIO_pa_context_get_state(context);
        if (!PA_CONTEXT_IS_GOOD(state)) {
            return SDL_SetError("Could not connect to PulseAudio");
        }
    } while (state != PA_CONTEXT_READY);

    return 0;  /* connected and ready! */
}

static int
ConnectToPulseServer(pa_mainloop **_mainloop, pa_context **_context)
{
    const int retval = ConnectToPulseServer_Internal(_mainloop, _context);
    if (retval < 0) {
        DisconnectFromPulseServer(*_mainloop, *_context);
    }
    return retval;
}


/* This function waits until it is possible to write a full sound buffer */
static void
PULSEAUDIO_WaitDevice(_THIS)
{
    struct SDL_PrivateAudioData *h = this->hidden;

    while (SDL_AtomicGet(&this->enabled)) {
        if (PULSEAUDIO_pa_context_get_state(h->context) != PA_CONTEXT_READY ||
            PULSEAUDIO_pa_stream_get_state(h->stream) != PA_STREAM_READY ||
            PULSEAUDIO_pa_mainloop_iterate(h->mainloop, 1, NULL) < 0) {
            SDL_OpenedAudioDeviceDisconnected(this);
            return;
        }
        if (PULSEAUDIO_pa_stream_writable_size(h->stream) >= h->mixlen) {
            return;
        }
    }
}

static void
PULSEAUDIO_PlayDevice(_THIS)
{
    /* Write the audio data */
    struct SDL_PrivateAudioData *h = this->hidden;
    if (SDL_AtomicGet(&this->enabled)) {
        if (PULSEAUDIO_pa_stream_write(h->stream, h->mixbuf, h->mixlen, NULL, 0LL, PA_SEEK_RELATIVE) < 0) {
            SDL_OpenedAudioDeviceDisconnected(this);
        }
    }
}

static Uint8 *
PULSEAUDIO_GetDeviceBuf(_THIS)
{
    return (this->hidden->mixbuf);
}


static int
PULSEAUDIO_CaptureFromDevice(_THIS, void *buffer, int buflen)
{
    struct SDL_PrivateAudioData *h = this->hidden;
    const void *data = NULL;
    size_t nbytes = 0;

    while (SDL_AtomicGet(&this->enabled)) {
        if (h->capturebuf != NULL) {
            const int cpy = SDL_min(buflen, h->capturelen);
            SDL_memcpy(buffer, h->capturebuf, cpy);
            /*printf("PULSEAUDIO: fed %d captured bytes\n", cpy);*/
            h->capturebuf += cpy;
            h->capturelen -= cpy;
            if (h->capturelen == 0) {
                h->capturebuf = NULL;
                PULSEAUDIO_pa_stream_drop(h->stream);  /* done with this fragment. */
            }
            return cpy;  /* new data, return it. */
        }

        if (PULSEAUDIO_pa_context_get_state(h->context) != PA_CONTEXT_READY ||
            PULSEAUDIO_pa_stream_get_state(h->stream) != PA_STREAM_READY ||
            PULSEAUDIO_pa_mainloop_iterate(h->mainloop, 1, NULL) < 0) {
            SDL_OpenedAudioDeviceDisconnected(this);
            return -1;  /* uhoh, pulse failed! */
        }

        if (PULSEAUDIO_pa_stream_readable_size(h->stream) == 0) {
            continue;  /* no data available yet. */
        }

        /* a new fragment is available! */
        PULSEAUDIO_pa_stream_peek(h->stream, &data, &nbytes);
        SDL_assert(nbytes > 0);
        if (data == NULL) {  /* NULL==buffer had a hole. Ignore that. */
            PULSEAUDIO_pa_stream_drop(h->stream);  /* drop this fragment. */
        } else {
            /* store this fragment's data, start feeding it to SDL. */
            /*printf("PULSEAUDIO: captured %d new bytes\n", (int) nbytes);*/
            h->capturebuf = (const Uint8 *) data;
            h->capturelen = nbytes;
        }
    }

    return -1;  /* not enabled? */
}

static void
PULSEAUDIO_FlushCapture(_THIS)
{
    struct SDL_PrivateAudioData *h = this->hidden;
    const void *data = NULL;
    size_t nbytes = 0;

    if (h->capturebuf != NULL) {
        PULSEAUDIO_pa_stream_drop(h->stream);
        h->capturebuf = NULL;
        h->capturelen = 0;
    }

    while (SDL_TRUE) {
        if (PULSEAUDIO_pa_context_get_state(h->context) != PA_CONTEXT_READY ||
            PULSEAUDIO_pa_stream_get_state(h->stream) != PA_STREAM_READY ||
            PULSEAUDIO_pa_mainloop_iterate(h->mainloop, 1, NULL) < 0) {
            SDL_OpenedAudioDeviceDisconnected(this);
            return;  /* uhoh, pulse failed! */
        }

        if (PULSEAUDIO_pa_stream_readable_size(h->stream) == 0) {
            break;  /* no data available, so we're done. */
        }

        /* a new fragment is available! Just dump it. */
        PULSEAUDIO_pa_stream_peek(h->stream, &data, &nbytes);
        PULSEAUDIO_pa_stream_drop(h->stream);  /* drop this fragment. */
    }
}

static void
PULSEAUDIO_CloseDevice(_THIS)
{
    if (this->hidden->stream) {
        if (this->hidden->capturebuf != NULL) {
            PULSEAUDIO_pa_stream_drop(this->hidden->stream);
        }
        PULSEAUDIO_pa_stream_disconnect(this->hidden->stream);
        PULSEAUDIO_pa_stream_unref(this->hidden->stream);
    }

    DisconnectFromPulseServer(this->hidden->mainloop, this->hidden->context);
    SDL_free(this->hidden->mixbuf);
    SDL_free(this->hidden->device_name);
    SDL_free(this->hidden);
}

static void
SinkDeviceNameCallback(pa_context *c, const pa_sink_info *i, int is_last, void *data)
{
    if (i) {
        char **devname = (char **) data;
        *devname = SDL_strdup(i->name);
    }
}

static void
SourceDeviceNameCallback(pa_context *c, const pa_source_info *i, int is_last, void *data)
{
    if (i) {
        char **devname = (char **) data;
        *devname = SDL_strdup(i->name);
    }
}

static SDL_bool
FindDeviceName(struct SDL_PrivateAudioData *h, const int iscapture, void *handle)
{
    const uint32_t idx = ((uint32_t) ((size_t) handle)) - 1;

    if (handle == NULL) {  /* NULL == default device. */
        return SDL_TRUE;
    }

    if (iscapture) {
        WaitForPulseOperation(h->mainloop,
            PULSEAUDIO_pa_context_get_source_info_by_index(h->context, idx,
                SourceDeviceNameCallback, &h->device_name));
    } else {
        WaitForPulseOperation(h->mainloop,
            PULSEAUDIO_pa_context_get_sink_info_by_index(h->context, idx,
                SinkDeviceNameCallback, &h->device_name));
    }

    return (h->device_name != NULL);
}

static int
PULSEAUDIO_OpenDevice(_THIS, void *handle, const char *devname, int iscapture)
{
    struct SDL_PrivateAudioData *h = NULL;
    Uint16 test_format = 0;
    pa_sample_spec paspec;
    pa_buffer_attr paattr;
    pa_channel_map pacmap;
    pa_stream_flags_t flags = 0;
    int state = 0;
    int rc = 0;

    /* Initialize all variables that we clean on shutdown */
    h = this->hidden = (struct SDL_PrivateAudioData *)
        SDL_malloc((sizeof *this->hidden));
    if (this->hidden == NULL) {
        return SDL_OutOfMemory();
    }
    SDL_zerop(this->hidden);

    paspec.format = PA_SAMPLE_INVALID;

    /* Try for a closest match on audio format */
    for (test_format = SDL_FirstAudioFormat(this->spec.format);
         (paspec.format == PA_SAMPLE_INVALID) && test_format;) {
#ifdef DEBUG_AUDIO
        fprintf(stderr, "Trying format 0x%4.4x\n", test_format);
#endif
        switch (test_format) {
        case AUDIO_U8:
            paspec.format = PA_SAMPLE_U8;
            break;
        case AUDIO_S16LSB:
            paspec.format = PA_SAMPLE_S16LE;
            break;
        case AUDIO_S16MSB:
            paspec.format = PA_SAMPLE_S16BE;
            break;
        case AUDIO_S32LSB:
            paspec.format = PA_SAMPLE_S32LE;
            break;
        case AUDIO_S32MSB:
            paspec.format = PA_SAMPLE_S32BE;
            break;
        case AUDIO_F32LSB:
            paspec.format = PA_SAMPLE_FLOAT32LE;
            break;
        case AUDIO_F32MSB:
            paspec.format = PA_SAMPLE_FLOAT32BE;
            break;
        default:
            paspec.format = PA_SAMPLE_INVALID;
            break;
        }
        if (paspec.format == PA_SAMPLE_INVALID) {
            test_format = SDL_NextAudioFormat();
        }
    }
    if (paspec.format == PA_SAMPLE_INVALID) {
        return SDL_SetError("Couldn't find any hardware audio formats");
    }
    this->spec.format = test_format;

    /* Calculate the final parameters for this audio specification */
#ifdef PA_STREAM_ADJUST_LATENCY
    this->spec.samples /= 2; /* Mix in smaller chunck to avoid underruns */
#endif
    SDL_CalculateAudioSpec(&this->spec);

    /* Allocate mixing buffer */
    if (!iscapture) {
        h->mixlen = this->spec.size;
        h->mixbuf = (Uint8 *) SDL_malloc(h->mixlen);
        if (h->mixbuf == NULL) {
            return SDL_OutOfMemory();
        }
        SDL_memset(h->mixbuf, this->spec.silence, this->spec.size);
    }

    paspec.channels = this->spec.channels;
    paspec.rate = this->spec.freq;

    /* Reduced prebuffering compared to the defaults. */
#ifdef PA_STREAM_ADJUST_LATENCY
    /* 2x original requested bufsize */
    paattr.tlength = h->mixlen * 4;
    paattr.prebuf = -1;
    paattr.maxlength = -1;
    /* -1 can lead to pa_stream_writable_size() >= mixlen never being true */
    paattr.minreq = h->mixlen;
    flags = PA_STREAM_ADJUST_LATENCY;
#else
    paattr.tlength = h->mixlen*2;
    paattr.prebuf = h->mixlen*2;
    paattr.maxlength = h->mixlen*2;
    paattr.minreq = h->mixlen;
#endif

    if (ConnectToPulseServer(&h->mainloop, &h->context) < 0) {
        return SDL_SetError("Could not connect to PulseAudio server");
    }

    if (!FindDeviceName(h, iscapture, handle)) {
        return SDL_SetError("Requested PulseAudio sink/source missing?");
    }

    /* The SDL ALSA output hints us that we use Windows' channel mapping */
    /* http://bugzilla.libsdl.org/show_bug.cgi?id=110 */
    PULSEAUDIO_pa_channel_map_init_auto(&pacmap, this->spec.channels,
                                        PA_CHANNEL_MAP_WAVEEX);

    h->stream = PULSEAUDIO_pa_stream_new(
        h->context,
        "Simple DirectMedia Layer", /* stream description */
        &paspec,    /* sample format spec */
        &pacmap     /* channel map */
        );

    if (h->stream == NULL) {
        return SDL_SetError("Could not set up PulseAudio stream");
    }

    /* now that we have multi-device support, don't move a stream from
        a device that was unplugged to something else, unless we're default. */
    if (h->device_name != NULL) {
        flags |= PA_STREAM_DONT_MOVE;
    }

    if (iscapture) {
        rc = PULSEAUDIO_pa_stream_connect_record(h->stream, h->device_name, &paattr, flags);
    } else {
        rc = PULSEAUDIO_pa_stream_connect_playback(h->stream, h->device_name, &paattr, flags, NULL, NULL);
    }

    if (rc < 0) {
        return SDL_SetError("Could not connect PulseAudio stream");
    }

    do {
        if (PULSEAUDIO_pa_mainloop_iterate(h->mainloop, 1, NULL) < 0) {
            return SDL_SetError("pa_mainloop_iterate() failed");
        }
        state = PULSEAUDIO_pa_stream_get_state(h->stream);
        if (!PA_STREAM_IS_GOOD(state)) {
            return SDL_SetError("Could not connect PulseAudio stream");
        }
    } while (state != PA_STREAM_READY);

    /* We're ready to rock and roll. :-) */
    return 0;
}

static pa_mainloop *hotplug_mainloop = NULL;
static pa_context *hotplug_context = NULL;
static SDL_Thread *hotplug_thread = NULL;

/* device handles are device index + 1, cast to void*, so we never pass a NULL. */

/* This is called when PulseAudio adds an output ("sink") device. */
static void
SinkInfoCallback(pa_context *c, const pa_sink_info *i, int is_last, void *data)
{
    if (i) {
        SDL_AddAudioDevice(SDL_FALSE, i->description, (void *) ((size_t) i->index+1));
    }
}

/* This is called when PulseAudio adds a capture ("source") device. */
static void
SourceInfoCallback(pa_context *c, const pa_source_info *i, int is_last, void *data)
{
    if (i) {
        /* Skip "monitor" sources. These are just output from other sinks. */
        if (i->monitor_of_sink == PA_INVALID_INDEX) {
            SDL_AddAudioDevice(SDL_TRUE, i->description, (void *) ((size_t) i->index+1));
        }
    }
}

/* This is called when PulseAudio has a device connected/removed/changed. */
static void
HotplugCallback(pa_context *c, pa_subscription_event_type_t t, uint32_t idx, void *data)
{
    const SDL_bool added = ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_NEW);
    const SDL_bool removed = ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE);

    if (added || removed) {  /* we only care about add/remove events. */
        const SDL_bool sink = ((t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) == PA_SUBSCRIPTION_EVENT_SINK);
        const SDL_bool source = ((t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) == PA_SUBSCRIPTION_EVENT_SOURCE);

        /* adds need sink details from the PulseAudio server. Another callback... */
        if (added && sink) {
            PULSEAUDIO_pa_context_get_sink_info_by_index(hotplug_context, idx, SinkInfoCallback, NULL);
        } else if (added && source) {
            PULSEAUDIO_pa_context_get_source_info_by_index(hotplug_context, idx, SourceInfoCallback, NULL);
        } else if (removed && (sink || source)) {
            /* removes we can handle just with the device index. */
            SDL_RemoveAudioDevice(source != 0, (void *) ((size_t) idx+1));
        }
    }
}

/* this runs as a thread while the Pulse target is initialized to catch hotplug events. */
static int SDLCALL
HotplugThread(void *data)
{
    pa_operation *o;
    SDL_SetThreadPriority(SDL_THREAD_PRIORITY_LOW);
    PULSEAUDIO_pa_context_set_subscribe_callback(hotplug_context, HotplugCallback, NULL);
    o = PULSEAUDIO_pa_context_subscribe(hotplug_context, PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SOURCE, NULL, NULL);
    PULSEAUDIO_pa_operation_unref(o);  /* don't wait for it, just do our thing. */
    PULSEAUDIO_pa_mainloop_run(hotplug_mainloop, NULL);
    return 0;
}

static void
PULSEAUDIO_DetectDevices()
{
    WaitForPulseOperation(hotplug_mainloop, PULSEAUDIO_pa_context_get_sink_info_list(hotplug_context, SinkInfoCallback, NULL));
    WaitForPulseOperation(hotplug_mainloop, PULSEAUDIO_pa_context_get_source_info_list(hotplug_context, SourceInfoCallback, NULL));

    /* ok, we have a sane list, let's set up hotplug notifications now... */
    hotplug_thread = SDL_CreateThreadInternal(HotplugThread, "PulseHotplug", 256 * 1024, NULL);
}

static void
PULSEAUDIO_Deinitialize(void)
{
    if (hotplug_thread) {
        PULSEAUDIO_pa_mainloop_quit(hotplug_mainloop, 0);
        SDL_WaitThread(hotplug_thread, NULL);
        hotplug_thread = NULL;
    }

    DisconnectFromPulseServer(hotplug_mainloop, hotplug_context);
    hotplug_mainloop = NULL;
    hotplug_context = NULL;

    UnloadPulseAudioLibrary();
}

static int
PULSEAUDIO_Init(SDL_AudioDriverImpl * impl)
{
    if (LoadPulseAudioLibrary() < 0) {
        return 0;
    }

    if (ConnectToPulseServer(&hotplug_mainloop, &hotplug_context) < 0) {
        UnloadPulseAudioLibrary();
        return 0;
    }

    /* Set the function pointers */
    impl->DetectDevices = PULSEAUDIO_DetectDevices;
    impl->OpenDevice = PULSEAUDIO_OpenDevice;
    impl->PlayDevice = PULSEAUDIO_PlayDevice;
    impl->WaitDevice = PULSEAUDIO_WaitDevice;
    impl->GetDeviceBuf = PULSEAUDIO_GetDeviceBuf;
    impl->CloseDevice = PULSEAUDIO_CloseDevice;
    impl->Deinitialize = PULSEAUDIO_Deinitialize;
    impl->CaptureFromDevice = PULSEAUDIO_CaptureFromDevice;
    impl->FlushCapture = PULSEAUDIO_FlushCapture;

    impl->HasCaptureSupport = SDL_TRUE;

    return 1;   /* this audio target is available. */
}

AudioBootStrap PULSEAUDIO_bootstrap = {
    "pulseaudio", "PulseAudio", PULSEAUDIO_Init, 0
};

#endif /* SDL_AUDIO_DRIVER_PULSEAUDIO */

/* vi: set ts=4 sw=4 expandtab: */
