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

#if SDL_AUDIO_DRIVER_COREAUDIO

/* !!! FIXME: clean out some of the macro salsa in here. */

#include "SDL_audio.h"
#include "SDL_hints.h"
#include "../SDL_audio_c.h"
#include "../SDL_sysaudio.h"
#include "SDL_coreaudio.h"
#include "../../thread/SDL_systhread.h"

#define DEBUG_COREAUDIO 0

#if DEBUG_COREAUDIO
    #define CHECK_RESULT(msg) \
        if (result != noErr) { \
            printf("COREAUDIO: Got error %d from '%s'!\n", (int) result, msg); \
            SDL_SetError("CoreAudio error (%s): %d", msg, (int) result); \
            return 0; \
        }
#else
    #define CHECK_RESULT(msg) \
        if (result != noErr) { \
            SDL_SetError("CoreAudio error (%s): %d", msg, (int) result); \
            return 0; \
        }
#endif


#if MACOSX_COREAUDIO
static const AudioObjectPropertyAddress devlist_address = {
    kAudioHardwarePropertyDevices,
    kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMaster
};

typedef void (*addDevFn)(const char *name, SDL_AudioSpec *spec, const int iscapture, AudioDeviceID devId, void *data);

typedef struct AudioDeviceList
{
    AudioDeviceID devid;
    SDL_bool alive;
    struct AudioDeviceList *next;
} AudioDeviceList;

static AudioDeviceList *output_devs = NULL;
static AudioDeviceList *capture_devs = NULL;

static SDL_bool
add_to_internal_dev_list(const int iscapture, AudioDeviceID devId)
{
    AudioDeviceList *item = (AudioDeviceList *) SDL_malloc(sizeof (AudioDeviceList));
    if (item == NULL) {
        return SDL_FALSE;
    }
    item->devid = devId;
    item->alive = SDL_TRUE;
    item->next = iscapture ? capture_devs : output_devs;
    if (iscapture) {
        capture_devs = item;
    } else {
        output_devs = item;
    }

    return SDL_TRUE;
}

static void
addToDevList(const char *name, SDL_AudioSpec *spec, const int iscapture, AudioDeviceID devId, void *data)
{
    if (add_to_internal_dev_list(iscapture, devId)) {
        SDL_AddAudioDevice(iscapture, name, spec, (void *) ((size_t) devId));
    }
}

static void
build_device_list(int iscapture, addDevFn addfn, void *addfndata)
{
    OSStatus result = noErr;
    UInt32 size = 0;
    AudioDeviceID *devs = NULL;
    UInt32 i = 0;
    UInt32 max = 0;

    result = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject,
                                            &devlist_address, 0, NULL, &size);
    if (result != kAudioHardwareNoError)
        return;

    devs = (AudioDeviceID *) alloca(size);
    if (devs == NULL)
        return;

    result = AudioObjectGetPropertyData(kAudioObjectSystemObject,
                                        &devlist_address, 0, NULL, &size, devs);
    if (result != kAudioHardwareNoError)
        return;

    max = size / sizeof (AudioDeviceID);
    for (i = 0; i < max; i++) {
        CFStringRef cfstr = NULL;
        char *ptr = NULL;
        AudioDeviceID dev = devs[i];
        AudioBufferList *buflist = NULL;
        int usable = 0;
        CFIndex len = 0;
        double sampleRate = 0;
        SDL_AudioSpec spec;
        const AudioObjectPropertyAddress addr = {
            kAudioDevicePropertyStreamConfiguration,
            iscapture ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput,
            kAudioObjectPropertyElementMaster
        };
        const AudioObjectPropertyAddress nameaddr = {
            kAudioObjectPropertyName,
            iscapture ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput,
            kAudioObjectPropertyElementMaster
        };
        const AudioObjectPropertyAddress freqaddr = {
            kAudioDevicePropertyNominalSampleRate,
            iscapture ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput,
            kAudioObjectPropertyElementMaster
        };

        result = AudioObjectGetPropertyDataSize(dev, &addr, 0, NULL, &size);
        if (result != noErr)
            continue;

        buflist = (AudioBufferList *) SDL_malloc(size);
        if (buflist == NULL)
            continue;

        result = AudioObjectGetPropertyData(dev, &addr, 0, NULL,
                                            &size, buflist);

        SDL_zero(spec);
        if (result == noErr) {
            UInt32 j;
            for (j = 0; j < buflist->mNumberBuffers; j++) {
                spec.channels += buflist->mBuffers[j].mNumberChannels;
            }
        }

        SDL_free(buflist);

        if (spec.channels == 0)
            continue;

        size = sizeof (sampleRate);
        result = AudioObjectGetPropertyData(dev, &freqaddr, 0, NULL, &size, &sampleRate);
        if (result == noErr) {
            spec.freq = (int) sampleRate;
        }

        size = sizeof (CFStringRef);
        result = AudioObjectGetPropertyData(dev, &nameaddr, 0, NULL, &size, &cfstr);
        if (result != kAudioHardwareNoError)
            continue;

        len = CFStringGetMaximumSizeForEncoding(CFStringGetLength(cfstr),
                                                kCFStringEncodingUTF8);

        ptr = (char *) SDL_malloc(len + 1);
        usable = ((ptr != NULL) &&
                  (CFStringGetCString
                   (cfstr, ptr, len + 1, kCFStringEncodingUTF8)));

        CFRelease(cfstr);

        if (usable) {
            len = strlen(ptr);
            /* Some devices have whitespace at the end...trim it. */
            while ((len > 0) && (ptr[len - 1] == ' ')) {
                len--;
            }
            usable = (len > 0);
        }

        if (usable) {
            ptr[len] = '\0';

#if DEBUG_COREAUDIO
            printf("COREAUDIO: Found %s device #%d: '%s' (devid %d)\n",
                   ((iscapture) ? "capture" : "output"),
                   (int) i, ptr, (int) dev);
#endif
            addfn(ptr, &spec, iscapture, dev, addfndata);
        }
        SDL_free(ptr);  /* addfn() would have copied the string. */
    }
}

static void
free_audio_device_list(AudioDeviceList **list)
{
    AudioDeviceList *item = *list;
    while (item) {
        AudioDeviceList *next = item->next;
        SDL_free(item);
        item = next;
    }
    *list = NULL;
}

static void
COREAUDIO_DetectDevices(void)
{
    build_device_list(SDL_TRUE, addToDevList, NULL);
    build_device_list(SDL_FALSE, addToDevList, NULL);
}

static void
build_device_change_list(const char *name, SDL_AudioSpec *spec, const int iscapture, AudioDeviceID devId, void *data)
{
    AudioDeviceList **list = (AudioDeviceList **) data;
    AudioDeviceList *item;
    for (item = *list; item != NULL; item = item->next) {
        if (item->devid == devId) {
            item->alive = SDL_TRUE;
            return;
        }
    }

    add_to_internal_dev_list(iscapture, devId);  /* new device, add it. */
    SDL_AddAudioDevice(iscapture, name, spec, (void *) ((size_t) devId));
}

static void
reprocess_device_list(const int iscapture, AudioDeviceList **list)
{
    AudioDeviceList *item;
    AudioDeviceList *prev = NULL;
    for (item = *list; item != NULL; item = item->next) {
        item->alive = SDL_FALSE;
    }

    build_device_list(iscapture, build_device_change_list, list);

    /* free items in the list that aren't still alive. */
    item = *list;
    while (item != NULL) {
        AudioDeviceList *next = item->next;
        if (item->alive) {
            prev = item;
        } else {
            SDL_RemoveAudioDevice(iscapture, (void *) ((size_t) item->devid));
            if (prev) {
                prev->next = item->next;
            } else {
                *list = item->next;
            }
            SDL_free(item);
        }
        item = next;
    }
}

/* this is called when the system's list of available audio devices changes. */
static OSStatus
device_list_changed(AudioObjectID systemObj, UInt32 num_addr, const AudioObjectPropertyAddress *addrs, void *data)
{
    reprocess_device_list(SDL_TRUE, &capture_devs);
    reprocess_device_list(SDL_FALSE, &output_devs);
    return 0;
}
#endif


static int open_playback_devices;
static int open_capture_devices;
static int num_open_devices;
static SDL_AudioDevice **open_devices;

#if !MACOSX_COREAUDIO

static BOOL session_active = NO;

static void pause_audio_devices()
{
    int i;

    if (!open_devices) {
        return;
    }

    for (i = 0; i < num_open_devices; ++i) {
        SDL_AudioDevice *device = open_devices[i];
        if (device->hidden->audioQueue && !device->hidden->interrupted) {
            AudioQueuePause(device->hidden->audioQueue);
        }
    }
}

static void resume_audio_devices()
{
    int i;

    if (!open_devices) {
        return;
    }

    for (i = 0; i < num_open_devices; ++i) {
        SDL_AudioDevice *device = open_devices[i];
        if (device->hidden->audioQueue && !device->hidden->interrupted) {
            AudioQueueStart(device->hidden->audioQueue, NULL);
        }
    }
}

static void interruption_begin(_THIS)
{
    if (this != NULL && this->hidden->audioQueue != NULL) {
        this->hidden->interrupted = SDL_TRUE;
        AudioQueuePause(this->hidden->audioQueue);
    }
}

static void interruption_end(_THIS)
{
    if (this != NULL && this->hidden != NULL && this->hidden->audioQueue != NULL
    && this->hidden->interrupted
    && AudioQueueStart(this->hidden->audioQueue, NULL) == AVAudioSessionErrorCodeNone) {
        this->hidden->interrupted = SDL_FALSE;
    }
}

@interface SDLInterruptionListener : NSObject

@property (nonatomic, assign) SDL_AudioDevice *device;

@end

@implementation SDLInterruptionListener

- (void)audioSessionInterruption:(NSNotification *)note
{
    @synchronized (self) {
        NSNumber *type = note.userInfo[AVAudioSessionInterruptionTypeKey];
        if (type.unsignedIntegerValue == AVAudioSessionInterruptionTypeBegan) {
            interruption_begin(self.device);
        } else {
            interruption_end(self.device);
        }
    }
}

- (void)applicationBecameActive:(NSNotification *)note
{
    @synchronized (self) {
        interruption_end(self.device);
    }
}

@end

static BOOL update_audio_session(_THIS, SDL_bool open, SDL_bool allow_playandrecord)
{
    @autoreleasepool {
        AVAudioSession *session = [AVAudioSession sharedInstance];
        NSNotificationCenter *center = [NSNotificationCenter defaultCenter];

        NSString *category = AVAudioSessionCategoryPlayback;
        NSString *mode = AVAudioSessionModeDefault;
        NSUInteger options = AVAudioSessionCategoryOptionMixWithOthers;
        NSError *err = nil;
        const char *hint;

        hint = SDL_GetHint(SDL_HINT_AUDIO_CATEGORY);
        if (hint) {
            if (SDL_strcasecmp(hint, "AVAudioSessionCategoryAmbient") == 0) {
                category = AVAudioSessionCategoryAmbient;
            } else if (SDL_strcasecmp(hint, "AVAudioSessionCategorySoloAmbient") == 0) {
                category = AVAudioSessionCategorySoloAmbient;
                options &= ~AVAudioSessionCategoryOptionMixWithOthers;
            } else if (SDL_strcasecmp(hint, "AVAudioSessionCategoryPlayback") == 0 ||
                       SDL_strcasecmp(hint, "playback") == 0) {
                category = AVAudioSessionCategoryPlayback;
                options &= ~AVAudioSessionCategoryOptionMixWithOthers;
            } else if (SDL_strcasecmp(hint, "AVAudioSessionCategoryPlayAndRecord") == 0 ||
                       SDL_strcasecmp(hint, "playandrecord") == 0) {
                if (allow_playandrecord) {
                    category = AVAudioSessionCategoryPlayAndRecord;
                }
            }
        } else if (open_playback_devices && open_capture_devices) {
            category = AVAudioSessionCategoryPlayAndRecord;
        } else if (open_capture_devices) {
            category = AVAudioSessionCategoryRecord;
        }

#if !TARGET_OS_TV
        if (category == AVAudioSessionCategoryPlayAndRecord) {
            options |= AVAudioSessionCategoryOptionDefaultToSpeaker;
        }
#endif
        if (category == AVAudioSessionCategoryRecord ||
            category == AVAudioSessionCategoryPlayAndRecord) {
            /* AVAudioSessionCategoryOptionAllowBluetooth isn't available in the SDK for
               Apple TV but is still needed in order to output to Bluetooth devices.
             */
            options |= 0x4; /* AVAudioSessionCategoryOptionAllowBluetooth; */
        }
        if (category == AVAudioSessionCategoryPlayAndRecord) {
            options |= AVAudioSessionCategoryOptionAllowBluetoothA2DP |
                       AVAudioSessionCategoryOptionAllowAirPlay;
        }
        if (category == AVAudioSessionCategoryPlayback ||
            category == AVAudioSessionCategoryPlayAndRecord) {
            options |= AVAudioSessionCategoryOptionDuckOthers;
        }

        if ([session respondsToSelector:@selector(setCategory:mode:options:error:)]) {
            if (![session.category isEqualToString:category] || session.categoryOptions != options) {
                /* Stop the current session so we don't interrupt other application audio */
                pause_audio_devices();
                [session setActive:NO error:nil];
                session_active = NO;

                if (![session setCategory:category mode:mode options:options error:&err]) {
                    NSString *desc = err.description;
                    SDL_SetError("Could not set Audio Session category: %s", desc.UTF8String);
                    return NO;
                }
            }
        } else {
            if (![session.category isEqualToString:category]) {
                /* Stop the current session so we don't interrupt other application audio */
                pause_audio_devices();
                [session setActive:NO error:nil];
                session_active = NO;

                if (![session setCategory:category error:&err]) {
                    NSString *desc = err.description;
                    SDL_SetError("Could not set Audio Session category: %s", desc.UTF8String);
                    return NO;
                }
            }
        }

        if ((open_playback_devices || open_capture_devices) && !session_active) {
            if (![session setActive:YES error:&err]) {
                if ([err code] == AVAudioSessionErrorCodeResourceNotAvailable &&
                    category == AVAudioSessionCategoryPlayAndRecord) {
                    return update_audio_session(this, open, SDL_FALSE);
                }

                NSString *desc = err.description;
                SDL_SetError("Could not activate Audio Session: %s", desc.UTF8String);
                return NO;
            }
            session_active = YES;
            resume_audio_devices();
        } else if (!open_playback_devices && !open_capture_devices && session_active) {
            pause_audio_devices();
            [session setActive:NO error:nil];
            session_active = NO;
        }

        if (open) {
            SDLInterruptionListener *listener = [SDLInterruptionListener new];
            listener.device = this;

            [center addObserver:listener
                       selector:@selector(audioSessionInterruption:)
                           name:AVAudioSessionInterruptionNotification
                         object:session];

            /* An interruption end notification is not guaranteed to be sent if
             we were previously interrupted... resuming if needed when the app
             becomes active seems to be the way to go. */
            // Note: object: below needs to be nil, as otherwise it filters by the object, and session doesn't send foreground / active notifications.  johna
            [center addObserver:listener
                       selector:@selector(applicationBecameActive:)
                           name:UIApplicationDidBecomeActiveNotification
                         object:nil];

            [center addObserver:listener
                       selector:@selector(applicationBecameActive:)
                           name:UIApplicationWillEnterForegroundNotification
                         object:nil];

            this->hidden->interruption_listener = CFBridgingRetain(listener);
        } else {
            SDLInterruptionListener *listener = nil;
            listener = (SDLInterruptionListener *) CFBridgingRelease(this->hidden->interruption_listener);
            [center removeObserver:listener];
            @synchronized (listener) {
                listener.device = NULL;
            }
        }
    }

    return YES;
}
#endif


/* The AudioQueue callback */
static void
outputCallback(void *inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer)
{
    SDL_AudioDevice *this = (SDL_AudioDevice *) inUserData;
    if (SDL_AtomicGet(&this->hidden->shutdown)) {
        return;  /* don't do anything. */
    }

    if (!SDL_AtomicGet(&this->enabled) || SDL_AtomicGet(&this->paused)) {
        /* Supply silence if audio is not enabled or paused */
        SDL_memset(inBuffer->mAudioData, this->spec.silence, inBuffer->mAudioDataBytesCapacity);
    } else if (this->stream) {
        UInt32 remaining = inBuffer->mAudioDataBytesCapacity;
        Uint8 *ptr = (Uint8 *) inBuffer->mAudioData;

        while (remaining > 0) {
            if (SDL_AudioStreamAvailable(this->stream) == 0) {
                /* Generate the data */
                SDL_LockMutex(this->mixer_lock);
                (*this->callbackspec.callback)(this->callbackspec.userdata,
                                               this->hidden->buffer, this->hidden->bufferSize);
                SDL_UnlockMutex(this->mixer_lock);
                this->hidden->bufferOffset = 0;
                SDL_AudioStreamPut(this->stream, this->hidden->buffer, this->hidden->bufferSize);
            }
            if (SDL_AudioStreamAvailable(this->stream) > 0) {
                int got;
                UInt32 len = SDL_AudioStreamAvailable(this->stream);
                if (len > remaining)
                    len = remaining;
                got = SDL_AudioStreamGet(this->stream, ptr, len);
                SDL_assert((got < 0) || (got == len));
                if (got != len) {
                    SDL_memset(ptr, this->spec.silence, len);
                }
                ptr = ptr + len;
                remaining -= len;
            }
        }
    } else {
        UInt32 remaining = inBuffer->mAudioDataBytesCapacity;
        Uint8 *ptr = (Uint8 *) inBuffer->mAudioData;

        while (remaining > 0) {
            UInt32 len;
            if (this->hidden->bufferOffset >= this->hidden->bufferSize) {
                /* Generate the data */
                SDL_LockMutex(this->mixer_lock);
                (*this->callbackspec.callback)(this->callbackspec.userdata,
                            this->hidden->buffer, this->hidden->bufferSize);
                SDL_UnlockMutex(this->mixer_lock);
                this->hidden->bufferOffset = 0;
            }

            len = this->hidden->bufferSize - this->hidden->bufferOffset;
            if (len > remaining) {
                len = remaining;
            }
            SDL_memcpy(ptr, (char *)this->hidden->buffer +
                       this->hidden->bufferOffset, len);
            ptr = ptr + len;
            remaining -= len;
            this->hidden->bufferOffset += len;
        }
    }

    AudioQueueEnqueueBuffer(this->hidden->audioQueue, inBuffer, 0, NULL);

    inBuffer->mAudioDataByteSize = inBuffer->mAudioDataBytesCapacity;
}

static void
inputCallback(void *inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer,
              const AudioTimeStamp *inStartTime, UInt32 inNumberPacketDescriptions,
              const AudioStreamPacketDescription *inPacketDescs)
{
    SDL_AudioDevice *this = (SDL_AudioDevice *) inUserData;

    if (SDL_AtomicGet(&this->shutdown)) {
        return;  /* don't do anything. */
    }

    /* ignore unless we're active. */
    if (!SDL_AtomicGet(&this->paused) && SDL_AtomicGet(&this->enabled)) {
        const Uint8 *ptr = (const Uint8 *) inBuffer->mAudioData;
        UInt32 remaining = inBuffer->mAudioDataByteSize;
        while (remaining > 0) {
            UInt32 len = this->hidden->bufferSize - this->hidden->bufferOffset;
            if (len > remaining) {
                len = remaining;
            }

            SDL_memcpy((char *)this->hidden->buffer + this->hidden->bufferOffset, ptr, len);
            ptr += len;
            remaining -= len;
            this->hidden->bufferOffset += len;

            if (this->hidden->bufferOffset >= this->hidden->bufferSize) {
                SDL_LockMutex(this->mixer_lock);
                (*this->callbackspec.callback)(this->callbackspec.userdata, this->hidden->buffer, this->hidden->bufferSize);
                SDL_UnlockMutex(this->mixer_lock);
                this->hidden->bufferOffset = 0;
            }
        }
    }

    AudioQueueEnqueueBuffer(this->hidden->audioQueue, inBuffer, 0, NULL);
}


#if MACOSX_COREAUDIO
static const AudioObjectPropertyAddress alive_address =
{
    kAudioDevicePropertyDeviceIsAlive,
    kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMaster
};

static OSStatus
device_unplugged(AudioObjectID devid, UInt32 num_addr, const AudioObjectPropertyAddress *addrs, void *data)
{
    SDL_AudioDevice *this = (SDL_AudioDevice *) data;
    SDL_bool dead = SDL_FALSE;
    UInt32 isAlive = 1;
    UInt32 size = sizeof (isAlive);
    OSStatus error;

    if (!SDL_AtomicGet(&this->enabled)) {
        return 0;  /* already known to be dead. */
    }

    error = AudioObjectGetPropertyData(this->hidden->deviceID, &alive_address,
                                       0, NULL, &size, &isAlive);

    if (error == kAudioHardwareBadDeviceError) {
        dead = SDL_TRUE;  /* device was unplugged. */
    } else if ((error == kAudioHardwareNoError) && (!isAlive)) {
        dead = SDL_TRUE;  /* device died in some other way. */
    }

    if (dead) {
        SDL_OpenedAudioDeviceDisconnected(this);
    }

    return 0;
}

/* macOS calls this when the default device changed (if we have a default device open). */
static OSStatus
default_device_changed(AudioObjectID inObjectID, UInt32 inNumberAddresses, const AudioObjectPropertyAddress *inAddresses, void *inUserData)
{
    SDL_AudioDevice *this = (SDL_AudioDevice *) inUserData;
    #if DEBUG_COREAUDIO
    printf("COREAUDIO: default device changed for SDL audio device %p!\n", this);
    #endif
    SDL_AtomicSet(&this->hidden->device_change_flag, 1);  /* let the audioqueue thread pick up on this when safe to do so. */
    return noErr;
}
#endif

static void
COREAUDIO_CloseDevice(_THIS)
{
    const SDL_bool iscapture = this->iscapture;
    int i;

/* !!! FIXME: what does iOS do when a bluetooth audio device vanishes? Headphones unplugged? */
/* !!! FIXME: (we only do a "default" device on iOS right now...can we do more?) */
#if MACOSX_COREAUDIO
    if (this->handle != NULL) {  /* we don't register this listener for default devices. */
        AudioObjectRemovePropertyListener(this->hidden->deviceID, &alive_address, device_unplugged, this);
    }
#endif

    if (iscapture) {
        open_capture_devices--;
    } else {
        open_playback_devices--;
    }

#if !MACOSX_COREAUDIO
    update_audio_session(this, SDL_FALSE, SDL_TRUE);
#endif

    for (i = 0; i < num_open_devices; ++i) {
        if (open_devices[i] == this) {
            --num_open_devices;
            if (i < num_open_devices) {
                SDL_memmove(&open_devices[i], &open_devices[i+1], sizeof(open_devices[i])*(num_open_devices - i));
            }
            break;
        }
    }
    if (num_open_devices == 0) {
        SDL_free(open_devices);
        open_devices = NULL;
    }

    /* if callback fires again, feed silence; don't call into the app. */
    SDL_AtomicSet(&this->paused, 1);

    if (this->hidden->audioQueue) {
        AudioQueueDispose(this->hidden->audioQueue, 1);
    }

    if (this->hidden->thread) {
        SDL_AtomicSet(&this->hidden->shutdown, 1);
        SDL_WaitThread(this->hidden->thread, NULL);
    }

    if (this->hidden->ready_semaphore) {
        SDL_DestroySemaphore(this->hidden->ready_semaphore);
    }

    /* AudioQueueDispose() frees the actual buffer objects. */
    SDL_free(this->hidden->audioBuffer);
    SDL_free(this->hidden->thread_error);
    SDL_free(this->hidden->buffer);
    SDL_free(this->hidden);
}

#if MACOSX_COREAUDIO
static int
prepare_device(_THIS, void *handle, int iscapture)
{
    AudioDeviceID devid = (AudioDeviceID) ((size_t) handle);
    OSStatus result = noErr;
    UInt32 size = 0;
    UInt32 alive = 0;
    pid_t pid = 0;

    AudioObjectPropertyAddress addr = {
        0,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };

    if (handle == NULL) {
        size = sizeof (AudioDeviceID);
        addr.mSelector =
            ((iscapture) ? kAudioHardwarePropertyDefaultInputDevice :
            kAudioHardwarePropertyDefaultOutputDevice);
        result = AudioObjectGetPropertyData(kAudioObjectSystemObject, &addr,
                                            0, NULL, &size, &devid);
        CHECK_RESULT("AudioHardwareGetProperty (default device)");
    }

    addr.mSelector = kAudioDevicePropertyDeviceIsAlive;
    addr.mScope = iscapture ? kAudioDevicePropertyScopeInput :
                    kAudioDevicePropertyScopeOutput;

    size = sizeof (alive);
    result = AudioObjectGetPropertyData(devid, &addr, 0, NULL, &size, &alive);
    CHECK_RESULT
        ("AudioDeviceGetProperty (kAudioDevicePropertyDeviceIsAlive)");

    if (!alive) {
        SDL_SetError("CoreAudio: requested device exists, but isn't alive.");
        return 0;
    }

    addr.mSelector = kAudioDevicePropertyHogMode;
    size = sizeof (pid);
    result = AudioObjectGetPropertyData(devid, &addr, 0, NULL, &size, &pid);

    /* some devices don't support this property, so errors are fine here. */
    if ((result == noErr) && (pid != -1)) {
        SDL_SetError("CoreAudio: requested device is being hogged.");
        return 0;
    }

    this->hidden->deviceID = devid;
    return 1;
}

static int
assign_device_to_audioqueue(_THIS)
{
    const AudioObjectPropertyAddress prop = {
        kAudioDevicePropertyDeviceUID,
        this->iscapture ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput,
        kAudioObjectPropertyElementMaster
    };

    OSStatus result;
    CFStringRef devuid;
    UInt32 devuidsize = sizeof (devuid);
    result = AudioObjectGetPropertyData(this->hidden->deviceID, &prop, 0, NULL, &devuidsize, &devuid);
    CHECK_RESULT("AudioObjectGetPropertyData (kAudioDevicePropertyDeviceUID)");
    result = AudioQueueSetProperty(this->hidden->audioQueue, kAudioQueueProperty_CurrentDevice, &devuid, devuidsize);
    CHECK_RESULT("AudioQueueSetProperty (kAudioQueueProperty_CurrentDevice)");

    return 1;
}
#endif

static int
prepare_audioqueue(_THIS)
{
    const AudioStreamBasicDescription *strdesc = &this->hidden->strdesc;
    const int iscapture = this->iscapture;
    OSStatus result;
    int i;

    SDL_assert(CFRunLoopGetCurrent() != NULL);

    if (iscapture) {
        result = AudioQueueNewInput(strdesc, inputCallback, this, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode, 0, &this->hidden->audioQueue);
        CHECK_RESULT("AudioQueueNewInput");
    } else {
        result = AudioQueueNewOutput(strdesc, outputCallback, this, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode, 0, &this->hidden->audioQueue);
        CHECK_RESULT("AudioQueueNewOutput");
    }

    #if MACOSX_COREAUDIO
    if (!assign_device_to_audioqueue(this)) {
        return 0;
    }

    /* only listen for unplugging on specific devices, not the default device, as that should
       switch to a different device (or hang out silently if there _is_ no other device). */
    if (this->handle != NULL) {
        /* !!! FIXME: what does iOS do when a bluetooth audio device vanishes? Headphones unplugged? */
        /* !!! FIXME: (we only do a "default" device on iOS right now...can we do more?) */
        /* Fire a callback if the device stops being "alive" (disconnected, etc). */
        /* If this fails, oh well, we won't notice a device had an extraordinary event take place. */
        AudioObjectAddPropertyListener(this->hidden->deviceID, &alive_address, device_unplugged, this);
    }
    #endif

    /* Calculate the final parameters for this audio specification */
    SDL_CalculateAudioSpec(&this->spec);

    /* Set the channel layout for the audio queue */
    AudioChannelLayout layout;
    SDL_zero(layout);
    switch (this->spec.channels) {
    case 1:
        layout.mChannelLayoutTag = kAudioChannelLayoutTag_Mono;
        break;
    case 2:
        layout.mChannelLayoutTag = kAudioChannelLayoutTag_Stereo;
        break;
    case 3:
        layout.mChannelLayoutTag = kAudioChannelLayoutTag_DVD_4;
        break;
    case 4:
        layout.mChannelLayoutTag = kAudioChannelLayoutTag_Quadraphonic;
        break;
    case 5:
        layout.mChannelLayoutTag = kAudioChannelLayoutTag_MPEG_5_0_A;
        break;
    case 6:
        layout.mChannelLayoutTag = kAudioChannelLayoutTag_MPEG_5_1_A;
        break;
    case 7:
        /* FIXME: Need to move channel[4] (BC) to channel[6] */
        layout.mChannelLayoutTag = kAudioChannelLayoutTag_MPEG_6_1_A;
        break;
    case 8:
        layout.mChannelLayoutTag = kAudioChannelLayoutTag_MPEG_7_1_A;
        break;
    }
    if (layout.mChannelLayoutTag != 0) {
        result = AudioQueueSetProperty(this->hidden->audioQueue, kAudioQueueProperty_ChannelLayout, &layout, sizeof(layout));
        CHECK_RESULT("AudioQueueSetProperty(kAudioQueueProperty_ChannelLayout)");
    }

    /* Allocate a sample buffer */
    this->hidden->bufferSize = this->spec.size;
    this->hidden->bufferOffset = iscapture ? 0 : this->hidden->bufferSize;

    this->hidden->buffer = SDL_malloc(this->hidden->bufferSize);
    if (this->hidden->buffer == NULL) {
        SDL_OutOfMemory();
        return 0;
    }

    /* Make sure we can feed the device a minimum amount of time */
    double MINIMUM_AUDIO_BUFFER_TIME_MS = 15.0;
#if defined(__IPHONEOS__)
    if (floor(NSFoundationVersionNumber) <= NSFoundationVersionNumber_iOS_7_1) {
        /* Older iOS hardware, use 40 ms as a minimum time */
        MINIMUM_AUDIO_BUFFER_TIME_MS = 40.0;
    }
#endif
    const double msecs = (this->spec.samples / ((double) this->spec.freq)) * 1000.0;
    int numAudioBuffers = 2;
    if (msecs < MINIMUM_AUDIO_BUFFER_TIME_MS) {  /* use more buffers if we have a VERY small sample set. */
        numAudioBuffers = ((int)SDL_ceil(MINIMUM_AUDIO_BUFFER_TIME_MS / msecs) * 2);
    }

    this->hidden->numAudioBuffers = numAudioBuffers;
    this->hidden->audioBuffer = SDL_calloc(1, sizeof (AudioQueueBufferRef) * numAudioBuffers);
    if (this->hidden->audioBuffer == NULL) {
        SDL_OutOfMemory();
        return 0;
    }

#if DEBUG_COREAUDIO
    printf("COREAUDIO: numAudioBuffers == %d\n", numAudioBuffers);
#endif

    for (i = 0; i < numAudioBuffers; i++) {
        result = AudioQueueAllocateBuffer(this->hidden->audioQueue, this->spec.size, &this->hidden->audioBuffer[i]);
        CHECK_RESULT("AudioQueueAllocateBuffer");
        SDL_memset(this->hidden->audioBuffer[i]->mAudioData, this->spec.silence, this->hidden->audioBuffer[i]->mAudioDataBytesCapacity);
        this->hidden->audioBuffer[i]->mAudioDataByteSize = this->hidden->audioBuffer[i]->mAudioDataBytesCapacity;
        /* !!! FIXME: should we use AudioQueueEnqueueBufferWithParameters and specify all frames be "trimmed" so these are immediately ready to refill with SDL callback data? */
        result = AudioQueueEnqueueBuffer(this->hidden->audioQueue, this->hidden->audioBuffer[i], 0, NULL);
        CHECK_RESULT("AudioQueueEnqueueBuffer");
    }

    result = AudioQueueStart(this->hidden->audioQueue, NULL);
    CHECK_RESULT("AudioQueueStart");

    /* We're running! */
    return 1;
}

static int
audioqueue_thread(void *arg)
{
    SDL_AudioDevice *this = (SDL_AudioDevice *) arg;

    #if MACOSX_COREAUDIO
    const AudioObjectPropertyAddress default_device_address = {
        this->iscapture ? kAudioHardwarePropertyDefaultInputDevice : kAudioHardwarePropertyDefaultOutputDevice,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };

    if (this->handle == NULL) {  /* opened the default device? Register to know if the user picks a new default. */
        /* we don't care if this fails; we just won't change to new default devices, but we still otherwise function in this case. */
        AudioObjectAddPropertyListener(kAudioObjectSystemObject, &default_device_address, default_device_changed, this);
    }
    #endif

    const int rc = prepare_audioqueue(this);
    if (!rc) {
        this->hidden->thread_error = SDL_strdup(SDL_GetError());
        SDL_SemPost(this->hidden->ready_semaphore);
        return 0;
    }

    SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH);

    /* init was successful, alert parent thread and start running... */
    SDL_SemPost(this->hidden->ready_semaphore);

    while (!SDL_AtomicGet(&this->hidden->shutdown)) {
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.10, 1);

        #if MACOSX_COREAUDIO
        if ((this->handle == NULL) && SDL_AtomicGet(&this->hidden->device_change_flag)) {
            SDL_AtomicSet(&this->hidden->device_change_flag, 0);

            #if DEBUG_COREAUDIO
            printf("COREAUDIO: audioqueue_thread is trying to switch to new default device!\n");
            #endif

            /* if any of this fails, there's not much to do but wait to see if the user gives up
               and quits (flagging the audioqueue for shutdown), or toggles to some other system
               output device (in which case we'll try again). */
            const AudioDeviceID prev_devid = this->hidden->deviceID;
            if (prepare_device(this, this->handle, this->iscapture) && (prev_devid != this->hidden->deviceID)) {
                AudioQueueStop(this->hidden->audioQueue, 1);
                if (assign_device_to_audioqueue(this)) {
                    int i;
                    for (i = 0; i < this->hidden->numAudioBuffers; i++) {
                        SDL_memset(this->hidden->audioBuffer[i]->mAudioData, this->spec.silence, this->hidden->audioBuffer[i]->mAudioDataBytesCapacity);
                        /* !!! FIXME: should we use AudioQueueEnqueueBufferWithParameters and specify all frames be "trimmed" so these are immediately ready to refill with SDL callback data? */
                        AudioQueueEnqueueBuffer(this->hidden->audioQueue, this->hidden->audioBuffer[i], 0, NULL);
                    }
                    AudioQueueStart(this->hidden->audioQueue, NULL);
                }
            }
        }
        #endif
    }

    if (!this->iscapture) {  /* Drain off any pending playback. */
        const CFTimeInterval secs = (((this->spec.size / (SDL_AUDIO_BITSIZE(this->spec.format) / 8)) / this->spec.channels) / ((CFTimeInterval) this->spec.freq)) * 2.0;
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, secs, 0);
    }

    #if MACOSX_COREAUDIO
    if (this->handle == NULL) {
        /* we don't care if this fails; we just won't change to new default devices, but we still otherwise function in this case. */
        AudioObjectRemovePropertyListener(kAudioObjectSystemObject, &default_device_address, default_device_changed, this);
    }
    #endif

    return 0;
}

static int
COREAUDIO_OpenDevice(_THIS, void *handle, const char *devname, int iscapture)
{
    AudioStreamBasicDescription *strdesc;
    SDL_AudioFormat test_format = SDL_FirstAudioFormat(this->spec.format);
    int valid_datatype = 0;
    SDL_AudioDevice **new_open_devices;

    /* Initialize all variables that we clean on shutdown */
    this->hidden = (struct SDL_PrivateAudioData *)
        SDL_malloc((sizeof *this->hidden));
    if (this->hidden == NULL) {
        return SDL_OutOfMemory();
    }
    SDL_zerop(this->hidden);

    strdesc = &this->hidden->strdesc;

    if (iscapture) {
        open_capture_devices++;
    } else {
        open_playback_devices++;
    }

    new_open_devices = (SDL_AudioDevice **)SDL_realloc(open_devices, sizeof(open_devices[0]) * (num_open_devices + 1));
    if (new_open_devices) {
        open_devices = new_open_devices;
        open_devices[num_open_devices++] = this;
    }

#if !MACOSX_COREAUDIO
    if (!update_audio_session(this, SDL_TRUE, SDL_TRUE)) {
        return -1;
    }

    /* Stop CoreAudio from doing expensive audio rate conversion */
    @autoreleasepool {
        AVAudioSession* session = [AVAudioSession sharedInstance];
        [session setPreferredSampleRate:this->spec.freq error:nil];
        this->spec.freq = (int)session.sampleRate;
#if TARGET_OS_TV
        if (iscapture) {
            [session setPreferredInputNumberOfChannels:this->spec.channels error:nil];
            this->spec.channels = session.preferredInputNumberOfChannels;
        } else {
            [session setPreferredOutputNumberOfChannels:this->spec.channels error:nil];
            this->spec.channels = session.preferredOutputNumberOfChannels;
        }
#else
        /* Calling setPreferredOutputNumberOfChannels seems to break audio output on iOS */
#endif /* TARGET_OS_TV */
    }
#endif

    /* Setup a AudioStreamBasicDescription with the requested format */
    SDL_zerop(strdesc);
    strdesc->mFormatID = kAudioFormatLinearPCM;
    strdesc->mFormatFlags = kLinearPCMFormatFlagIsPacked;
    strdesc->mChannelsPerFrame = this->spec.channels;
    strdesc->mSampleRate = this->spec.freq;
    strdesc->mFramesPerPacket = 1;

    while ((!valid_datatype) && (test_format)) {
        this->spec.format = test_format;
        /* CoreAudio handles most of SDL's formats natively, but not U16, apparently. */
        switch (test_format) {
        case AUDIO_U8:
        case AUDIO_S8:
        case AUDIO_S16LSB:
        case AUDIO_S16MSB:
        case AUDIO_S32LSB:
        case AUDIO_S32MSB:
        case AUDIO_F32LSB:
        case AUDIO_F32MSB:
            valid_datatype = 1;
            strdesc->mBitsPerChannel = SDL_AUDIO_BITSIZE(this->spec.format);
            if (SDL_AUDIO_ISBIGENDIAN(this->spec.format))
                strdesc->mFormatFlags |= kLinearPCMFormatFlagIsBigEndian;

            if (SDL_AUDIO_ISFLOAT(this->spec.format))
                strdesc->mFormatFlags |= kLinearPCMFormatFlagIsFloat;
            else if (SDL_AUDIO_ISSIGNED(this->spec.format))
                strdesc->mFormatFlags |= kLinearPCMFormatFlagIsSignedInteger;
            break;

        default:
            test_format = SDL_NextAudioFormat();
            break;
        }
    }

    if (!valid_datatype) {      /* shouldn't happen, but just in case... */
        return SDL_SetError("Unsupported audio format");
    }

    strdesc->mBytesPerFrame = strdesc->mChannelsPerFrame * strdesc->mBitsPerChannel / 8;
    strdesc->mBytesPerPacket = strdesc->mBytesPerFrame * strdesc->mFramesPerPacket;

#if MACOSX_COREAUDIO
    if (!prepare_device(this, handle, iscapture)) {
        return -1;
    }
#endif

    /* This has to init in a new thread so it can get its own CFRunLoop. :/ */
    SDL_AtomicSet(&this->hidden->shutdown, 0);
    this->hidden->ready_semaphore = SDL_CreateSemaphore(0);
    if (!this->hidden->ready_semaphore) {
        return -1;  /* oh well. */
    }

    this->hidden->thread = SDL_CreateThreadInternal(audioqueue_thread, "AudioQueue thread", 512 * 1024, this);
    if (!this->hidden->thread) {
        return -1;
    }

    SDL_SemWait(this->hidden->ready_semaphore);
    SDL_DestroySemaphore(this->hidden->ready_semaphore);
    this->hidden->ready_semaphore = NULL;

    if ((this->hidden->thread != NULL) && (this->hidden->thread_error != NULL)) {
        SDL_SetError("%s", this->hidden->thread_error);
        return -1;
    }

    return (this->hidden->thread != NULL) ? 0 : -1;
}

static void
COREAUDIO_Deinitialize(void)
{
#if MACOSX_COREAUDIO
    AudioObjectRemovePropertyListener(kAudioObjectSystemObject, &devlist_address, device_list_changed, NULL);
    free_audio_device_list(&capture_devs);
    free_audio_device_list(&output_devs);
#endif
}

static int
COREAUDIO_Init(SDL_AudioDriverImpl * impl)
{
    /* Set the function pointers */
    impl->OpenDevice = COREAUDIO_OpenDevice;
    impl->CloseDevice = COREAUDIO_CloseDevice;
    impl->Deinitialize = COREAUDIO_Deinitialize;

#if MACOSX_COREAUDIO
    impl->DetectDevices = COREAUDIO_DetectDevices;
    AudioObjectAddPropertyListener(kAudioObjectSystemObject, &devlist_address, device_list_changed, NULL);
#else
    impl->OnlyHasDefaultOutputDevice = 1;
    impl->OnlyHasDefaultCaptureDevice = 1;
#endif

    impl->ProvidesOwnCallbackThread = 1;
    impl->HasCaptureSupport = 1;

    return 1;   /* this audio target is available. */
}

AudioBootStrap COREAUDIO_bootstrap = {
    "coreaudio", "CoreAudio", COREAUDIO_Init, 0
};

#endif /* SDL_AUDIO_DRIVER_COREAUDIO */

/* vi: set ts=4 sw=4 expandtab: */
