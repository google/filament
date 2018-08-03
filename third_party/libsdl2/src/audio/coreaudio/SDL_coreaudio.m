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

#if SDL_AUDIO_DRIVER_COREAUDIO

/* !!! FIXME: clean out some of the macro salsa in here. */

#include "SDL_audio.h"
#include "SDL_hints.h"
#include "../SDL_audio_c.h"
#include "../SDL_sysaudio.h"
#include "SDL_coreaudio.h"
#include "SDL_assert.h"
#include "../../thread/SDL_systhread.h"

#define DEBUG_COREAUDIO 0

#define CHECK_RESULT(msg) \
    if (result != noErr) { \
        SDL_SetError("CoreAudio error (%s): %d", msg, (int) result); \
        return 0; \
    }

#if MACOSX_COREAUDIO
static const AudioObjectPropertyAddress devlist_address = {
    kAudioHardwarePropertyDevices,
    kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMaster
};

typedef void (*addDevFn)(const char *name, const int iscapture, AudioDeviceID devId, void *data);

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
addToDevList(const char *name, const int iscapture, AudioDeviceID devId, void *data)
{
    if (add_to_internal_dev_list(iscapture, devId)) {
        SDL_AddAudioDevice(iscapture, name, (void *) ((size_t) devId));
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

        result = AudioObjectGetPropertyDataSize(dev, &addr, 0, NULL, &size);
        if (result != noErr)
            continue;

        buflist = (AudioBufferList *) SDL_malloc(size);
        if (buflist == NULL)
            continue;

        result = AudioObjectGetPropertyData(dev, &addr, 0, NULL,
                                            &size, buflist);

        if (result == noErr) {
            UInt32 j;
            for (j = 0; j < buflist->mNumberBuffers; j++) {
                if (buflist->mBuffers[j].mNumberChannels > 0) {
                    usable = 1;
                    break;
                }
            }
        }

        SDL_free(buflist);

        if (!usable)
            continue;


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
            addfn(ptr, iscapture, dev, addfndata);
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
build_device_change_list(const char *name, const int iscapture, AudioDeviceID devId, void *data)
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
    SDL_AddAudioDevice(iscapture, name, (void *) ((size_t) devId));
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


static int open_playback_devices = 0;
static int open_capture_devices = 0;

#if !MACOSX_COREAUDIO

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

static BOOL update_audio_session(_THIS, SDL_bool open)
{
    @autoreleasepool {
        AVAudioSession *session = [AVAudioSession sharedInstance];
        NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
        /* Set category to ambient by default so that other music continues playing. */
        NSString *category = AVAudioSessionCategoryAmbient;
        NSError *err = nil;

        if (open_playback_devices && open_capture_devices) {
            category = AVAudioSessionCategoryPlayAndRecord;
        } else if (open_capture_devices) {
            category = AVAudioSessionCategoryRecord;
        } else {
            const char *hint = SDL_GetHint(SDL_HINT_AUDIO_CATEGORY);
            if (hint) {
                if (SDL_strcasecmp(hint, "AVAudioSessionCategoryAmbient") == 0) {
                    category = AVAudioSessionCategoryAmbient;
                } else if (SDL_strcasecmp(hint, "AVAudioSessionCategorySoloAmbient") == 0) {
                    category = AVAudioSessionCategorySoloAmbient;
                } else if (SDL_strcasecmp(hint, "AVAudioSessionCategoryPlayback") == 0 ||
                           SDL_strcasecmp(hint, "playback") == 0) {
                    category = AVAudioSessionCategoryPlayback;
                }
            }
        }

        if (![session setCategory:category error:&err]) {
            NSString *desc = err.description;
            SDL_SetError("Could not set Audio Session category: %s", desc.UTF8String);
            return NO;
        }

        if (open_playback_devices + open_capture_devices == 1) {
            if (![session setActive:YES error:&err]) {
                NSString *desc = err.description;
                SDL_SetError("Could not activate Audio Session: %s", desc.UTF8String);
                return NO;
            }
        } else if (!open_playback_devices && !open_capture_devices) {
            [session setActive:NO error:nil];
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
            [center addObserver:listener
                       selector:@selector(applicationBecameActive:)
                           name:UIApplicationDidBecomeActiveNotification
                         object:session];

            [center addObserver:listener
                       selector:@selector(applicationBecameActive:)
                           name:UIApplicationWillEnterForegroundNotification
                         object:session];

            this->hidden->interruption_listener = CFBridgingRetain(listener);
        } else {
            if (this->hidden->interruption_listener != NULL) {
                SDLInterruptionListener *listener = nil;
                listener = (SDLInterruptionListener *) CFBridgingRelease(this->hidden->interruption_listener);
                @synchronized (listener) {
                    listener.device = NULL;
                }
                [center removeObserver:listener];
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
              const AudioStreamPacketDescription *inPacketDescs )
{
    SDL_AudioDevice *this = (SDL_AudioDevice *) inUserData;

    if (SDL_AtomicGet(&this->shutdown)) {
        return;  /* don't do anything. */
    }

    /* ignore unless we're active. */
    if (!SDL_AtomicGet(&this->paused) && SDL_AtomicGet(&this->enabled) && !SDL_AtomicGet(&this->paused)) {
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
#endif

static void
COREAUDIO_CloseDevice(_THIS)
{
    const SDL_bool iscapture = this->iscapture;

/* !!! FIXME: what does iOS do when a bluetooth audio device vanishes? Headphones unplugged? */
/* !!! FIXME: (we only do a "default" device on iOS right now...can we do more?) */
#if MACOSX_COREAUDIO
    /* Fire a callback if the device stops being "alive" (disconnected, etc). */
    AudioObjectRemovePropertyListener(this->hidden->deviceID, &alive_address, device_unplugged, this);
#endif

#if !MACOSX_COREAUDIO
    update_audio_session(this, SDL_FALSE);
#endif

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

    if (iscapture) {
        open_capture_devices--;
    } else {
        open_playback_devices--;
    }
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
{
    const AudioObjectPropertyAddress prop = {
        kAudioDevicePropertyDeviceUID,
        iscapture ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput,
        kAudioObjectPropertyElementMaster
    };
    CFStringRef devuid;
    UInt32 devuidsize = sizeof (devuid);
    result = AudioObjectGetPropertyData(this->hidden->deviceID, &prop, 0, NULL, &devuidsize, &devuid);
    CHECK_RESULT("AudioObjectGetPropertyData (kAudioDevicePropertyDeviceUID)");
    result = AudioQueueSetProperty(this->hidden->audioQueue, kAudioQueueProperty_CurrentDevice, &devuid, devuidsize);
    CHECK_RESULT("AudioQueueSetProperty (kAudioQueueProperty_CurrentDevice)");

    /* !!! FIXME: what does iOS do when a bluetooth audio device vanishes? Headphones unplugged? */
    /* !!! FIXME: (we only do a "default" device on iOS right now...can we do more?) */
    /* Fire a callback if the device stops being "alive" (disconnected, etc). */
    AudioObjectAddPropertyListener(this->hidden->deviceID, &alive_address, device_unplugged, this);
}
#endif

    /* Calculate the final parameters for this audio specification */
    SDL_CalculateAudioSpec(&this->spec);

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
    const int rc = prepare_audioqueue(this);
    if (!rc) {
        this->hidden->thread_error = SDL_strdup(SDL_GetError());
        SDL_SemPost(this->hidden->ready_semaphore);
        return 0;
    }

    /* init was successful, alert parent thread and start running... */
    SDL_SemPost(this->hidden->ready_semaphore);
    while (!SDL_AtomicGet(&this->hidden->shutdown)) {
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.10, 1);
    }

    if (!this->iscapture) {  /* Drain off any pending playback. */
        const CFTimeInterval secs = (((this->spec.size / (SDL_AUDIO_BITSIZE(this->spec.format) / 8)) / this->spec.channels) / ((CFTimeInterval) this->spec.freq)) * 2.0;
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, secs, 0);
    }

    return 0;
}

static int
COREAUDIO_OpenDevice(_THIS, void *handle, const char *devname, int iscapture)
{
    AudioStreamBasicDescription *strdesc;
    SDL_AudioFormat test_format = SDL_FirstAudioFormat(this->spec.format);
    int valid_datatype = 0;

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

#if !MACOSX_COREAUDIO
    if (!update_audio_session(this, SDL_TRUE)) {
        return -1;
    }

    /* Stop CoreAudio from doing expensive audio rate conversion */
    @autoreleasepool {
        AVAudioSession* session = [AVAudioSession sharedInstance];
        [session setPreferredSampleRate:this->spec.freq error:nil];
        this->spec.freq = (int)session.sampleRate;
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
        /* Just a list of valid SDL formats, so people don't pass junk here. */
        switch (test_format) {
        case AUDIO_U8:
        case AUDIO_S8:
        case AUDIO_U16LSB:
        case AUDIO_S16LSB:
        case AUDIO_U16MSB:
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
        }
    }

    if (!valid_datatype) {      /* shouldn't happen, but just in case... */
        return SDL_SetError("Unsupported audio format");
    }

    strdesc->mBytesPerFrame = strdesc->mBitsPerChannel * strdesc->mChannelsPerFrame / 8;
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
