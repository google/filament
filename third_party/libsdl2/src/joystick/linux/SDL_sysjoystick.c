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

#ifdef SDL_JOYSTICK_LINUX

#ifndef SDL_INPUT_LINUXEV
#error SDL now requires a Linux 2.4+ kernel with /dev/input/event support.
#endif

/* This is the Linux implementation of the SDL joystick API */

#include <sys/stat.h>
#include <errno.h>              /* errno, strerror */
#include <fcntl.h>
#include <limits.h>             /* For the definition of PATH_MAX */
#ifdef HAVE_INOTIFY
#include <sys/inotify.h>
#endif
#include <sys/ioctl.h>
#include <unistd.h>
#include <dirent.h>
#include <linux/joystick.h>

#include "SDL_hints.h"
#include "SDL_joystick.h"
#include "SDL_log.h"
#include "SDL_endian.h"
#include "SDL_timer.h"
#include "../../events/SDL_events_c.h"
#include "../SDL_sysjoystick.h"
#include "../SDL_joystick_c.h"
#include "../steam/SDL_steamcontroller.h"
#include "SDL_sysjoystick_c.h"
#include "../hidapi/SDL_hidapijoystick_c.h"

/* This isn't defined in older Linux kernel headers */
#ifndef SYN_DROPPED
#define SYN_DROPPED 3
#endif
#ifndef BTN_DPAD_UP
#define BTN_DPAD_UP     0x220
#endif
#ifndef BTN_DPAD_DOWN
#define BTN_DPAD_DOWN   0x221
#endif
#ifndef BTN_DPAD_LEFT
#define BTN_DPAD_LEFT   0x222
#endif
#ifndef BTN_DPAD_RIGHT
#define BTN_DPAD_RIGHT  0x223
#endif

#include "../../core/linux/SDL_evdev_capabilities.h"
#include "../../core/linux/SDL_udev.h"

#if 0
#define DEBUG_INPUT_EVENTS 1
#endif

typedef enum
{
    ENUMERATION_UNSET,
    ENUMERATION_LIBUDEV,
    ENUMERATION_FALLBACK
} EnumerationMethod;

static EnumerationMethod enumeration_method = ENUMERATION_UNSET;

static int MaybeAddDevice(const char *path);
static int MaybeRemoveDevice(const char *path);

/* A linked list of available joysticks */
typedef struct SDL_joylist_item
{
    int device_instance;
    char *path;   /* "/dev/input/event2" or whatever */
    char *name;   /* "SideWinder 3D Pro" or whatever */
    SDL_JoystickGUID guid;
    dev_t devnum;
    struct joystick_hwdata *hwdata;
    struct SDL_joylist_item *next;

    /* Steam Controller support */
    SDL_bool m_bSteamController;

    SDL_GamepadMapping *mapping;
} SDL_joylist_item;

static SDL_joylist_item *SDL_joylist = NULL;
static SDL_joylist_item *SDL_joylist_tail = NULL;
static int numjoysticks = 0;
static int inotify_fd = -1;

static Uint32 last_joy_detect_time;
static time_t last_input_dir_mtime;

static void
FixupDeviceInfoForMapping(int fd, struct input_id *inpid)
{
    if (inpid->vendor == 0x045e && inpid->product == 0x0b05 && inpid->version == 0x0903) {
        /* This is a Microsoft Xbox One Elite Series 2 controller */
        unsigned long keybit[NBITS(KEY_MAX)] = { 0 };

        /* The first version of the firmware duplicated all the inputs */
        if ((ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keybit)), keybit) >= 0) &&
            test_bit(0x2c0, keybit)) {
            /* Change the version to 0x0902, so we can map it differently */
            inpid->version = 0x0902;
        }
    }

    /* For Atari vcs modern and classic controllers have the version reflecting
     * firmware version, but the mapping stays stable so ignore
     * version information */
    if (inpid->vendor == 0x3250
            && (inpid->product == 0x1001 || inpid->product == 0x1002)) {
        inpid->version = 0;
    }
}

#ifdef SDL_JOYSTICK_HIDAPI
static SDL_bool
IsVirtualJoystick(Uint16 vendor, Uint16 product, Uint16 version, const char *name)
{
    if (vendor == USB_VENDOR_MICROSOFT && product == USB_PRODUCT_XBOX_ONE_S && version == 0 &&
        SDL_strcmp(name, "Xbox One S Controller") == 0) {
        /* This is the virtual device created by the xow driver */
        return SDL_TRUE;
    }
    return SDL_FALSE;
}
#endif /* SDL_JOYSTICK_HIDAPI */

static int
GuessIsJoystick(int fd)
{
    unsigned long evbit[NBITS(EV_MAX)] = { 0 };
    unsigned long keybit[NBITS(KEY_MAX)] = { 0 };
    unsigned long absbit[NBITS(ABS_MAX)] = { 0 };
    unsigned long relbit[NBITS(REL_MAX)] = { 0 };
    int devclass;

    if ((ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), evbit) < 0) ||
        (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keybit)), keybit) < 0) ||
        (ioctl(fd, EVIOCGBIT(EV_REL, sizeof(relbit)), relbit) < 0) ||
        (ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absbit)), absbit) < 0)) {
        return (0);
    }

    devclass = SDL_EVDEV_GuessDeviceClass(evbit, absbit, keybit, relbit);

    if (devclass & SDL_UDEV_DEVICE_JOYSTICK) {
        return 1;
    }

    return 0;
}

static int
IsJoystick(int fd, char **name_return, SDL_JoystickGUID *guid)
{
    struct input_id inpid;
    Uint16 *guid16 = (Uint16 *)guid->data;
    char *name;
    char product_string[128];

    /* When udev is enabled we only get joystick devices here, so there's no need to test them */
    if (enumeration_method != ENUMERATION_LIBUDEV && !GuessIsJoystick(fd)) {
        return 0;
    }

    if (ioctl(fd, EVIOCGID, &inpid) < 0) {
        return 0;
    }

    if (ioctl(fd, EVIOCGNAME(sizeof(product_string)), product_string) < 0) {
        return 0;
    }

    name = SDL_CreateJoystickName(inpid.vendor, inpid.product, NULL, product_string);
    if (!name) {
        return 0;
    }

#ifdef SDL_JOYSTICK_HIDAPI
    if (!IsVirtualJoystick(inpid.vendor, inpid.product, inpid.version, name) &&
        HIDAPI_IsDevicePresent(inpid.vendor, inpid.product, inpid.version, name)) {
        /* The HIDAPI driver is taking care of this device */
        SDL_free(name);
        return 0;
    }
#endif

    FixupDeviceInfoForMapping(fd, &inpid);

#ifdef DEBUG_JOYSTICK
    printf("Joystick: %s, bustype = %d, vendor = 0x%.4x, product = 0x%.4x, version = %d\n", name, inpid.bustype, inpid.vendor, inpid.product, inpid.version);
#endif

    SDL_memset(guid->data, 0, sizeof(guid->data));

    /* We only need 16 bits for each of these; space them out to fill 128. */
    /* Byteswap so devices get same GUID on little/big endian platforms. */
    *guid16++ = SDL_SwapLE16(inpid.bustype);
    *guid16++ = 0;

    if (inpid.vendor && inpid.product) {
        *guid16++ = SDL_SwapLE16(inpid.vendor);
        *guid16++ = 0;
        *guid16++ = SDL_SwapLE16(inpid.product);
        *guid16++ = 0;
        *guid16++ = SDL_SwapLE16(inpid.version);
        *guid16++ = 0;
    } else {
        SDL_strlcpy((char*)guid16, name, sizeof(guid->data) - 4);
    }

    if (SDL_ShouldIgnoreJoystick(name, *guid)) {
        SDL_free(name);
        return 0;
    }
    *name_return = name;
    return 1;
}

#if SDL_USE_LIBUDEV
static void joystick_udev_callback(SDL_UDEV_deviceevent udev_type, int udev_class, const char *devpath)
{
    if (devpath == NULL) {
        return;
    }

    switch (udev_type) {
        case SDL_UDEV_DEVICEADDED:
            if (!(udev_class & SDL_UDEV_DEVICE_JOYSTICK)) {
                return;
            }
            MaybeAddDevice(devpath);
            break;
            
        case SDL_UDEV_DEVICEREMOVED:
            MaybeRemoveDevice(devpath);
            break;
            
        default:
            break;
    }
    
}
#endif /* SDL_USE_LIBUDEV */

static int
MaybeAddDevice(const char *path)
{
    struct stat sb;
    int fd = -1;
    int isstick = 0;
    char *name = NULL;
    SDL_JoystickGUID guid;
    SDL_joylist_item *item;

    if (path == NULL) {
        return -1;
    }

    if (stat(path, &sb) == -1) {
        return -1;
    }

    /* Check to make sure it's not already in list. */
    for (item = SDL_joylist; item != NULL; item = item->next) {
        if (sb.st_rdev == item->devnum) {
            return -1;  /* already have this one */
        }
    }

    fd = open(path, O_RDONLY, 0);
    if (fd < 0) {
        return -1;
    }

#ifdef DEBUG_INPUT_EVENTS
    printf("Checking %s\n", path);
#endif

    isstick = IsJoystick(fd, &name, &guid);
    close(fd);
    if (!isstick) {
        return -1;
    }

    item = (SDL_joylist_item *) SDL_malloc(sizeof (SDL_joylist_item));
    if (item == NULL) {
        return -1;
    }

    SDL_zerop(item);
    item->devnum = sb.st_rdev;
    item->path = SDL_strdup(path);
    item->name = name;
    item->guid = guid;

    if ((item->path == NULL) || (item->name == NULL)) {
         SDL_free(item->path);
         SDL_free(item->name);
         SDL_free(item);
         return -1;
    }

    item->device_instance = SDL_GetNextJoystickInstanceID();
    if (SDL_joylist_tail == NULL) {
        SDL_joylist = SDL_joylist_tail = item;
    } else {
        SDL_joylist_tail->next = item;
        SDL_joylist_tail = item;
    }

    /* Need to increment the joystick count before we post the event */
    ++numjoysticks;

    SDL_PrivateJoystickAdded(item->device_instance);

    return numjoysticks;
}

static int
MaybeRemoveDevice(const char *path)
{
    SDL_joylist_item *item;
    SDL_joylist_item *prev = NULL;

    if (path == NULL) {
        return -1;
    }

    for (item = SDL_joylist; item != NULL; item = item->next) {
        /* found it, remove it. */
        if (SDL_strcmp(path, item->path) == 0) {
            const int retval = item->device_instance;
            if (item->hwdata) {
                item->hwdata->item = NULL;
            }
            if (prev != NULL) {
                prev->next = item->next;
            } else {
                SDL_assert(SDL_joylist == item);
                SDL_joylist = item->next;
            }
            if (item == SDL_joylist_tail) {
                SDL_joylist_tail = prev;
            }

            /* Need to decrement the joystick count before we post the event */
            --numjoysticks;

            SDL_PrivateJoystickRemoved(item->device_instance);

            if (item->mapping) {
                SDL_free(item->mapping);
            }
            SDL_free(item->path);
            SDL_free(item->name);
            SDL_free(item);
            return retval;
        }
        prev = item;
    }

    return -1;
}

static void
HandlePendingRemovals(void)
{
    SDL_joylist_item *prev = NULL;
    SDL_joylist_item *item = SDL_joylist;

    while (item != NULL) {
        if (item->hwdata && item->hwdata->gone) {
            item->hwdata->item = NULL;

            if (prev != NULL) {
                prev->next = item->next;
            } else {
                SDL_assert(SDL_joylist == item);
                SDL_joylist = item->next;
            }
            if (item == SDL_joylist_tail) {
                SDL_joylist_tail = prev;
            }

            /* Need to decrement the joystick count before we post the event */
            --numjoysticks;

            SDL_PrivateJoystickRemoved(item->device_instance);

            SDL_free(item->path);
            SDL_free(item->name);
            SDL_free(item);

            if (prev != NULL) {
                item = prev->next;
            } else {
                item = SDL_joylist;
            }
        } else {
            prev = item;
            item = item->next;
        }
    }
}

static SDL_bool SteamControllerConnectedCallback(const char *name, SDL_JoystickGUID guid, int *device_instance)
{
    SDL_joylist_item *item;

    item = (SDL_joylist_item *) SDL_calloc(1, sizeof (SDL_joylist_item));
    if (item == NULL) {
        return SDL_FALSE;
    }

    item->path = SDL_strdup("");
    item->name = SDL_strdup(name);
    item->guid = guid;
    item->m_bSteamController = SDL_TRUE;

    if ((item->path == NULL) || (item->name == NULL)) {
         SDL_free(item->path);
         SDL_free(item->name);
         SDL_free(item);
         return SDL_FALSE;
    }

    *device_instance = item->device_instance = SDL_GetNextJoystickInstanceID();
    if (SDL_joylist_tail == NULL) {
        SDL_joylist = SDL_joylist_tail = item;
    } else {
        SDL_joylist_tail->next = item;
        SDL_joylist_tail = item;
    }

    /* Need to increment the joystick count before we post the event */
    ++numjoysticks;

    SDL_PrivateJoystickAdded(item->device_instance);

    return SDL_TRUE;
}

static void SteamControllerDisconnectedCallback(int device_instance)
{
    SDL_joylist_item *item;
    SDL_joylist_item *prev = NULL;

    for (item = SDL_joylist; item != NULL; item = item->next) {
        /* found it, remove it. */
        if (item->device_instance == device_instance) {
            if (item->hwdata) {
                item->hwdata->item = NULL;
            }
            if (prev != NULL) {
                prev->next = item->next;
            } else {
                SDL_assert(SDL_joylist == item);
                SDL_joylist = item->next;
            }
            if (item == SDL_joylist_tail) {
                SDL_joylist_tail = prev;
            }

            /* Need to decrement the joystick count before we post the event */
            --numjoysticks;

            SDL_PrivateJoystickRemoved(item->device_instance);

            SDL_free(item->name);
            SDL_free(item);
            return;
        }
        prev = item;
    }
}

#ifdef HAVE_INOTIFY
#ifdef HAVE_INOTIFY_INIT1
static int SDL_inotify_init1(void) {
    return inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
}
#else
static int SDL_inotify_init1(void) {
    int fd = inotify_init();
    if (fd  < 0) return -1;
    fcntl(fd, F_SETFL, O_NONBLOCK);
    fcntl(fd, F_SETFD, FD_CLOEXEC);
    return fd;
}
#endif

static int
StrHasPrefix(const char *string, const char *prefix)
{
    return (SDL_strncmp(string, prefix, SDL_strlen(prefix)) == 0);
}

static int
StrIsInteger(const char *string)
{
    const char *p;

    if (*string == '\0') {
        return 0;
    }

    for (p = string; *p != '\0'; p++) {
        if (*p < '0' || *p > '9') {
            return 0;
        }
    }

    return 1;
}

static void
LINUX_InotifyJoystickDetect(void)
{
    union
    {
        struct inotify_event event;
        char storage[4096];
        char enough_for_inotify[sizeof (struct inotify_event) + NAME_MAX + 1];
    } buf;
    ssize_t bytes;
    size_t remain = 0;
    size_t len;

    bytes = read(inotify_fd, &buf, sizeof (buf));

    if (bytes > 0) {
        remain = (size_t) bytes;
    }

    while (remain > 0) {
        if (buf.event.len > 0) {
            if (StrHasPrefix(buf.event.name, "event") &&
                StrIsInteger(buf.event.name + strlen ("event"))) {
                char path[PATH_MAX];

                SDL_snprintf(path, SDL_arraysize(path), "/dev/input/%s", buf.event.name);

                if (buf.event.mask & (IN_CREATE | IN_MOVED_TO | IN_ATTRIB)) {
                    MaybeAddDevice(path);
                }
                else if (buf.event.mask & (IN_DELETE | IN_MOVED_FROM)) {
                    MaybeRemoveDevice(path);
                }
            }
        }

        len = sizeof (struct inotify_event) + buf.event.len;
        remain -= len;

        if (remain != 0) {
            memmove (&buf.storage[0], &buf.storage[len], remain);
        }
    }
}
#endif /* HAVE_INOTIFY */

/* Detect devices by reading /dev/input. In the inotify code path we
 * have to do this the first time, to detect devices that already existed
 * before we started; in the non-inotify code path we do this repeatedly
 * (polling). */
static int
filter_entries(const struct dirent *entry)
{
    return (SDL_strlen(entry->d_name) > 5 && SDL_strncmp(entry->d_name, "event", 5) == 0);
}
static int
sort_entries(const struct dirent **a, const struct dirent **b)
{
    int numA = SDL_atoi((*a)->d_name+5);
    int numB = SDL_atoi((*b)->d_name+5);
    return (numA - numB);
}
static void
LINUX_FallbackJoystickDetect(void)
{
    const Uint32 SDL_JOY_DETECT_INTERVAL_MS = 3000;  /* Update every 3 seconds */
    Uint32 now = SDL_GetTicks();

    if (!last_joy_detect_time || SDL_TICKS_PASSED(now, last_joy_detect_time + SDL_JOY_DETECT_INTERVAL_MS)) {
        struct stat sb;

        /* Opening input devices can generate synchronous device I/O, so avoid it if we can */
        if (stat("/dev/input", &sb) == 0 && sb.st_mtime != last_input_dir_mtime) {
            int i, count;
            struct dirent **entries;
            char path[PATH_MAX];

            count = scandir("/dev/input", &entries, filter_entries, sort_entries);
            for (i = 0; i < count; ++i) {
                SDL_snprintf(path, SDL_arraysize(path), "/dev/input/%s", entries[i]->d_name);
                MaybeAddDevice(path);

                free(entries[i]); /* This should NOT be SDL_free() */
            }
            free(entries); /* This should NOT be SDL_free() */

            last_input_dir_mtime = sb.st_mtime;
        }

        last_joy_detect_time = now;
    }
}

static void
LINUX_JoystickDetect(void)
{
#if SDL_USE_LIBUDEV
    if (enumeration_method == ENUMERATION_LIBUDEV) {
        SDL_UDEV_Poll();
    }
    else
#endif
#ifdef HAVE_INOTIFY
    if (inotify_fd >= 0 && last_joy_detect_time != 0) {
        LINUX_InotifyJoystickDetect();
    }
    else
#endif
    {
        LINUX_FallbackJoystickDetect();
    }

    HandlePendingRemovals();

    SDL_UpdateSteamControllers();
}

static int
LINUX_JoystickInit(void)
{
#if SDL_USE_LIBUDEV
    if (enumeration_method == ENUMERATION_UNSET) {
        if (SDL_getenv("SDL_JOYSTICK_DISABLE_UDEV") != NULL) {
            SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
                         "udev disabled by SDL_JOYSTICK_DISABLE_UDEV");
            enumeration_method = ENUMERATION_FALLBACK;
        }
        else if (access("/.flatpak-info", F_OK) == 0
                 || access("/run/host/container-manager", F_OK) == 0) {
            /* Explicitly check `/.flatpak-info` because, for old versions of
             * Flatpak, this was the only available way to tell if we were in
             * a Flatpak container. */
            SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
                         "Container detected, disabling udev integration");
            enumeration_method = ENUMERATION_FALLBACK;
        }
        else {
            SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
                         "Using udev for joystick device discovery");
            enumeration_method = ENUMERATION_LIBUDEV;
        }
    }
#endif

    /* First see if the user specified one or more joysticks to use */
    if (SDL_getenv("SDL_JOYSTICK_DEVICE") != NULL) {
        char *envcopy, *envpath, *delim;
        envcopy = SDL_strdup(SDL_getenv("SDL_JOYSTICK_DEVICE"));
        envpath = envcopy;
        while (envpath != NULL) {
            delim = SDL_strchr(envpath, ':');
            if (delim != NULL) {
                *delim++ = '\0';
            }
            MaybeAddDevice(envpath);
            envpath = delim;
        }
        SDL_free(envcopy);
    }

    SDL_InitSteamControllers(SteamControllerConnectedCallback,
                             SteamControllerDisconnectedCallback);

    /* Force immediate joystick detection if using fallback */
    last_joy_detect_time = 0;
    last_input_dir_mtime = 0;

#if SDL_USE_LIBUDEV
    if (enumeration_method == ENUMERATION_LIBUDEV) {
        if (SDL_UDEV_Init() < 0) {
            return SDL_SetError("Could not initialize UDEV");
        }

        /* Set up the udev callback */
        if (SDL_UDEV_AddCallback(joystick_udev_callback) < 0) {
            SDL_UDEV_Quit();
            return SDL_SetError("Could not set up joystick <-> udev callback");
        }

        /* Force a scan to build the initial device list */
        SDL_UDEV_Scan();
    }
    else
#endif
    {
#if defined(HAVE_INOTIFY)
        inotify_fd = SDL_inotify_init1();

        if (inotify_fd < 0) {
            SDL_LogWarn(SDL_LOG_CATEGORY_INPUT,
                        "Unable to initialize inotify, falling back to polling: %s",
                        strerror (errno));
        } else {
            /* We need to watch for attribute changes in addition to
             * creation, because when a device is first created, it has
             * permissions that we can't read. When udev chmods it to
             * something that we maybe *can* read, we'll get an
             * IN_ATTRIB event to tell us. */
            if (inotify_add_watch(inotify_fd, "/dev/input",
                                  IN_CREATE | IN_DELETE | IN_MOVE | IN_ATTRIB) < 0) {
                close(inotify_fd);
                inotify_fd = -1;
                SDL_LogWarn(SDL_LOG_CATEGORY_INPUT,
                            "Unable to add inotify watch, falling back to polling: %s",
                            strerror (errno));
            }
        }
#endif /* HAVE_INOTIFY */

        /* Report all devices currently present */
        LINUX_JoystickDetect();
    }

    return 0;
}

static int
LINUX_JoystickGetCount(void)
{
    return numjoysticks;
}

static SDL_joylist_item *
JoystickByDevIndex(int device_index)
{
    SDL_joylist_item *item = SDL_joylist;

    if ((device_index < 0) || (device_index >= numjoysticks)) {
        return NULL;
    }

    while (device_index > 0) {
        SDL_assert(item != NULL);
        device_index--;
        item = item->next;
    }

    return item;
}

/* Function to get the device-dependent name of a joystick */
static const char *
LINUX_JoystickGetDeviceName(int device_index)
{
    return JoystickByDevIndex(device_index)->name;
}

static int
LINUX_JoystickGetDevicePlayerIndex(int device_index)
{
    return -1;
}

static void
LINUX_JoystickSetDevicePlayerIndex(int device_index, int player_index)
{
}

static SDL_JoystickGUID
LINUX_JoystickGetDeviceGUID( int device_index )
{
    return JoystickByDevIndex(device_index)->guid;
}

/* Function to perform the mapping from device index to the instance id for this index */
static SDL_JoystickID
LINUX_JoystickGetDeviceInstanceID(int device_index)
{
    return JoystickByDevIndex(device_index)->device_instance;
}

static int
allocate_hatdata(SDL_Joystick *joystick)
{
    int i;

    joystick->hwdata->hats =
        (struct hwdata_hat *) SDL_malloc(joystick->nhats *
                                         sizeof(struct hwdata_hat));
    if (joystick->hwdata->hats == NULL) {
        return (-1);
    }
    for (i = 0; i < joystick->nhats; ++i) {
        joystick->hwdata->hats[i].axis[0] = 1;
        joystick->hwdata->hats[i].axis[1] = 1;
    }
    return (0);
}

static int
allocate_balldata(SDL_Joystick *joystick)
{
    int i;

    joystick->hwdata->balls =
        (struct hwdata_ball *) SDL_malloc(joystick->nballs *
                                          sizeof(struct hwdata_ball));
    if (joystick->hwdata->balls == NULL) {
        return (-1);
    }
    for (i = 0; i < joystick->nballs; ++i) {
        joystick->hwdata->balls[i].axis[0] = 0;
        joystick->hwdata->balls[i].axis[1] = 0;
    }
    return (0);
}

static void
ConfigJoystick(SDL_Joystick *joystick, int fd)
{
    int i, t;
    unsigned long keybit[NBITS(KEY_MAX)] = { 0 };
    unsigned long absbit[NBITS(ABS_MAX)] = { 0 };
    unsigned long relbit[NBITS(REL_MAX)] = { 0 };
    unsigned long ffbit[NBITS(FF_MAX)] = { 0 };
    SDL_bool use_deadzones = SDL_GetHintBoolean(SDL_HINT_LINUX_JOYSTICK_DEADZONES, SDL_FALSE);

    /* See if this device uses the new unified event API */
    if ((ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keybit)), keybit) >= 0) &&
        (ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absbit)), absbit) >= 0) &&
        (ioctl(fd, EVIOCGBIT(EV_REL, sizeof(relbit)), relbit) >= 0)) {

        /* Get the number of buttons, axes, and other thingamajigs */
        for (i = BTN_JOYSTICK; i < KEY_MAX; ++i) {
            if (test_bit(i, keybit)) {
#ifdef DEBUG_INPUT_EVENTS
                printf("Joystick has button: 0x%x\n", i);
#endif
                joystick->hwdata->key_map[i] = joystick->nbuttons;
                joystick->hwdata->has_key[i] = SDL_TRUE;
                ++joystick->nbuttons;
            }
        }
        for (i = 0; i < BTN_JOYSTICK; ++i) {
            if (test_bit(i, keybit)) {
#ifdef DEBUG_INPUT_EVENTS
                printf("Joystick has button: 0x%x\n", i);
#endif
                joystick->hwdata->key_map[i] = joystick->nbuttons;
                joystick->hwdata->has_key[i] = SDL_TRUE;
                ++joystick->nbuttons;
            }
        }
        for (i = 0; i < ABS_MAX; ++i) {
            /* Skip hats */
            if (i == ABS_HAT0X) {
                i = ABS_HAT3Y;
                continue;
            }
            if (test_bit(i, absbit)) {
                struct input_absinfo absinfo;
                struct axis_correct *correct = &joystick->hwdata->abs_correct[i];

                if (ioctl(fd, EVIOCGABS(i), &absinfo) < 0) {
                    continue;
                }
#ifdef DEBUG_INPUT_EVENTS
                printf("Joystick has absolute axis: 0x%.2x\n", i);
                printf("Values = { %d, %d, %d, %d, %d }\n",
                       absinfo.value, absinfo.minimum, absinfo.maximum,
                       absinfo.fuzz, absinfo.flat);
#endif /* DEBUG_INPUT_EVENTS */
                joystick->hwdata->abs_map[i] = joystick->naxes;
                joystick->hwdata->has_abs[i] = SDL_TRUE;

                correct->minimum = absinfo.minimum;
                correct->maximum = absinfo.maximum;
                if (correct->minimum != correct->maximum) {
                    if (use_deadzones) {
                        correct->use_deadzones = SDL_TRUE;
                        correct->coef[0] = (absinfo.maximum + absinfo.minimum) - 2 * absinfo.flat;
                        correct->coef[1] = (absinfo.maximum + absinfo.minimum) + 2 * absinfo.flat;
                        t = ((absinfo.maximum - absinfo.minimum) - 4 * absinfo.flat);
                        if (t != 0) {
                            correct->coef[2] = (1 << 28) / t;
                        } else {
                            correct->coef[2] = 0;
                        }
                    } else {
                        float value_range = (correct->maximum - correct->minimum);
                        float output_range = (SDL_JOYSTICK_AXIS_MAX - SDL_JOYSTICK_AXIS_MIN);

                        correct->scale = (output_range / value_range);
                    }
                }
                ++joystick->naxes;
            }
        }
        for (i = ABS_HAT0X; i <= ABS_HAT3Y; i += 2) {
            if (test_bit(i, absbit) || test_bit(i + 1, absbit)) {
                struct input_absinfo absinfo;
                int hat_index = (i - ABS_HAT0X) / 2;

                if (ioctl(fd, EVIOCGABS(i), &absinfo) < 0) {
                    continue;
                }
#ifdef DEBUG_INPUT_EVENTS
                printf("Joystick has hat %d\n", hat_index);
                printf("Values = { %d, %d, %d, %d, %d }\n",
                       absinfo.value, absinfo.minimum, absinfo.maximum,
                       absinfo.fuzz, absinfo.flat);
#endif /* DEBUG_INPUT_EVENTS */
                joystick->hwdata->hats_indices[hat_index] = joystick->nhats++;
                joystick->hwdata->has_hat[hat_index] = SDL_TRUE;
            }
        }
        if (test_bit(REL_X, relbit) || test_bit(REL_Y, relbit)) {
            ++joystick->nballs;
        }

        /* Allocate data to keep track of these thingamajigs */
        if (joystick->nhats > 0) {
            if (allocate_hatdata(joystick) < 0) {
                joystick->nhats = 0;
            }
        }
        if (joystick->nballs > 0) {
            if (allocate_balldata(joystick) < 0) {
                joystick->nballs = 0;
            }
        }
    }

    if (ioctl(fd, EVIOCGBIT(EV_FF, sizeof(ffbit)), ffbit) >= 0) {
        if (test_bit(FF_RUMBLE, ffbit)) {
            joystick->hwdata->ff_rumble = SDL_TRUE;
        }
        if (test_bit(FF_SINE, ffbit)) {
            joystick->hwdata->ff_sine = SDL_TRUE;
        }
    }
}


/* This is used to do the heavy lifting for LINUX_JoystickOpen and
   also LINUX_JoystickGetGamepadMapping, so we can query the hardware
   without adding an opened SDL_Joystick object to the system.
   This expects `joystick->hwdata` to be allocated and will not free it
   on error. Returns -1 on error, 0 on success. */
static int
PrepareJoystickHwdata(SDL_Joystick *joystick, SDL_joylist_item *item)
{
    joystick->hwdata->item = item;
    joystick->hwdata->guid = item->guid;
    joystick->hwdata->effect.id = -1;
    joystick->hwdata->m_bSteamController = item->m_bSteamController;
    SDL_memset(joystick->hwdata->abs_map, 0xFF, sizeof(joystick->hwdata->abs_map));

    if (item->m_bSteamController) {
        joystick->hwdata->fd = -1;
        SDL_GetSteamControllerInputs(&joystick->nbuttons,
                                     &joystick->naxes,
                                     &joystick->nhats);
    } else {
        const int fd = open(item->path, O_RDWR, 0);
        if (fd < 0) {
            return SDL_SetError("Unable to open %s", item->path);
        }

        joystick->hwdata->fd = fd;
        joystick->hwdata->fname = SDL_strdup(item->path);
        if (joystick->hwdata->fname == NULL) {
            close(fd);
            return SDL_OutOfMemory();
        }

        /* Set the joystick to non-blocking read mode */
        fcntl(fd, F_SETFL, O_NONBLOCK);

        /* Get the number of buttons and axes on the joystick */
        ConfigJoystick(joystick, fd);
    }
    return 0;
}


/* Function to open a joystick for use.
   The joystick to open is specified by the device index.
   This should fill the nbuttons and naxes fields of the joystick structure.
   It returns 0, or -1 if there is an error.
 */
static int
LINUX_JoystickOpen(SDL_Joystick *joystick, int device_index)
{
    SDL_joylist_item *item = JoystickByDevIndex(device_index);

    if (item == NULL) {
        return SDL_SetError("No such device");
    }

    joystick->instance_id = item->device_instance;
    joystick->hwdata = (struct joystick_hwdata *)
        SDL_calloc(1, sizeof(*joystick->hwdata));
    if (joystick->hwdata == NULL) {
        return SDL_OutOfMemory();
    }

    if (PrepareJoystickHwdata(joystick, item) == -1) {
        SDL_free(joystick->hwdata);
        joystick->hwdata = NULL;
        return -1;  /* SDL_SetError will already have been called */
    }

    SDL_assert(item->hwdata == NULL);
    item->hwdata = joystick->hwdata;

    /* mark joystick as fresh and ready */
    joystick->hwdata->fresh = SDL_TRUE;

    return 0;
}

static int
LINUX_JoystickRumble(SDL_Joystick *joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble)
{
    struct input_event event;

    if (joystick->hwdata->ff_rumble) {
        struct ff_effect *effect = &joystick->hwdata->effect;

        effect->type = FF_RUMBLE;
        effect->replay.length = SDL_MAX_RUMBLE_DURATION_MS;
        effect->u.rumble.strong_magnitude = low_frequency_rumble;
        effect->u.rumble.weak_magnitude = high_frequency_rumble;
    } else if (joystick->hwdata->ff_sine) {
        /* Scale and average the two rumble strengths */
        Sint16 magnitude = (Sint16)(((low_frequency_rumble / 2) + (high_frequency_rumble / 2)) / 2);
        struct ff_effect *effect = &joystick->hwdata->effect;

        effect->type = FF_PERIODIC;
        effect->replay.length = SDL_MAX_RUMBLE_DURATION_MS;
        effect->u.periodic.waveform = FF_SINE;
        effect->u.periodic.magnitude = magnitude;
    } else {
        return SDL_Unsupported();
    }

    if (ioctl(joystick->hwdata->fd, EVIOCSFF, &joystick->hwdata->effect) < 0) {
        /* The kernel may have lost this effect, try to allocate a new one */
        joystick->hwdata->effect.id = -1;
        if (ioctl(joystick->hwdata->fd, EVIOCSFF, &joystick->hwdata->effect) < 0) {
            return SDL_SetError("Couldn't update rumble effect: %s", strerror(errno));
        }
    }

    event.type = EV_FF;
    event.code = joystick->hwdata->effect.id;
    event.value = 1;
    if (write(joystick->hwdata->fd, &event, sizeof(event)) < 0) {
        return SDL_SetError("Couldn't start rumble effect: %s", strerror(errno));
    }
    return 0;
}

static int
LINUX_JoystickRumbleTriggers(SDL_Joystick *joystick, Uint16 left_rumble, Uint16 right_rumble)
{
    return SDL_Unsupported();
}

static SDL_bool
LINUX_JoystickHasLED(SDL_Joystick *joystick)
{
    return SDL_FALSE;
}

static int
LINUX_JoystickSetLED(SDL_Joystick *joystick, Uint8 red, Uint8 green, Uint8 blue)
{
    return SDL_Unsupported();
}

static int
LINUX_JoystickSendEffect(SDL_Joystick *joystick, const void *data, int size)
{
    return SDL_Unsupported();
}

static int
LINUX_JoystickSetSensorsEnabled(SDL_Joystick *joystick, SDL_bool enabled)
{
    return SDL_Unsupported();
}

static SDL_INLINE void
HandleHat(SDL_Joystick *stick, Uint8 hat, int axis, int value)
{
    struct hwdata_hat *the_hat;
    const Uint8 position_map[3][3] = {
        {SDL_HAT_LEFTUP, SDL_HAT_UP, SDL_HAT_RIGHTUP},
        {SDL_HAT_LEFT, SDL_HAT_CENTERED, SDL_HAT_RIGHT},
        {SDL_HAT_LEFTDOWN, SDL_HAT_DOWN, SDL_HAT_RIGHTDOWN}
    };

    the_hat = &stick->hwdata->hats[hat];
    if (value < 0) {
        value = 0;
    } else if (value == 0) {
        value = 1;
    } else if (value > 0) {
        value = 2;
    }
    if (value != the_hat->axis[axis]) {
        the_hat->axis[axis] = value;
        SDL_PrivateJoystickHat(stick, hat,
                               position_map[the_hat->axis[1]][the_hat->axis[0]]);
    }
}

static SDL_INLINE void
HandleBall(SDL_Joystick *stick, Uint8 ball, int axis, int value)
{
    stick->hwdata->balls[ball].axis[axis] += value;
}


static SDL_INLINE int
AxisCorrect(SDL_Joystick *joystick, int which, int value)
{
    struct axis_correct *correct;

    correct = &joystick->hwdata->abs_correct[which];
    if (correct->minimum != correct->maximum) {
        if (correct->use_deadzones) {
            value *= 2;
            if (value > correct->coef[0]) {
                if (value < correct->coef[1]) {
                    return 0;
                }
                value -= correct->coef[1];
            } else {
                value -= correct->coef[0];
            }
            value *= correct->coef[2];
            value >>= 13;
        } else {
            value = (int)SDL_floorf((value - correct->minimum) * correct->scale + SDL_JOYSTICK_AXIS_MIN + 0.5f);
        }
    }

    /* Clamp and return */
    if (value < SDL_JOYSTICK_AXIS_MIN) {
        return SDL_JOYSTICK_AXIS_MIN;
    }
    if (value > SDL_JOYSTICK_AXIS_MAX) {
        return SDL_JOYSTICK_AXIS_MAX;
    }
    return value;
}

static SDL_INLINE void
PollAllValues(SDL_Joystick *joystick)
{
    struct input_absinfo absinfo;
    unsigned long keyinfo[NBITS(KEY_MAX)];
    int i;

    /* Poll all axis */
    for (i = ABS_X; i < ABS_MAX; i++) {
        if (i == ABS_HAT0X) {  /* we handle hats in the next loop, skip them for now. */
            i = ABS_HAT3Y;
            continue;
        }
        if (joystick->hwdata->has_abs[i]) {
            if (ioctl(joystick->hwdata->fd, EVIOCGABS(i), &absinfo) >= 0) {
                absinfo.value = AxisCorrect(joystick, i, absinfo.value);

#ifdef DEBUG_INPUT_EVENTS
                printf("Joystick : Re-read Axis %d (%d) val= %d\n",
                    joystick->hwdata->abs_map[i], i, absinfo.value);
#endif
                SDL_PrivateJoystickAxis(joystick,
                        joystick->hwdata->abs_map[i],
                        absinfo.value);
            }
        }
    }

    /* Poll all hats */
    for (i = ABS_HAT0X; i <= ABS_HAT3Y; i++) {
        const int baseaxis = i - ABS_HAT0X;
        const int hatidx = baseaxis / 2;
        SDL_assert(hatidx < SDL_arraysize(joystick->hwdata->has_hat));
        if (joystick->hwdata->has_hat[hatidx]) {
            if (ioctl(joystick->hwdata->fd, EVIOCGABS(i), &absinfo) >= 0) {
                const int hataxis = baseaxis % 2;
                HandleHat(joystick, joystick->hwdata->hats_indices[hatidx], hataxis, absinfo.value);
            }
        }
    }

    /* Poll all buttons */
    SDL_zeroa(keyinfo);
    if (ioctl(joystick->hwdata->fd, EVIOCGKEY(sizeof (keyinfo)), keyinfo) >= 0) {
        for (i = 0; i < KEY_MAX; i++) {
            if (joystick->hwdata->has_key[i]) {
                const Uint8 value = test_bit(i, keyinfo) ? SDL_PRESSED : SDL_RELEASED;
#ifdef DEBUG_INPUT_EVENTS
                printf("Joystick : Re-read Button %d (%d) val= %d\n",
                    joystick->hwdata->key_map[i], i, value);
#endif
                SDL_PrivateJoystickButton(joystick,
                        joystick->hwdata->key_map[i], value);
            }
        }
    }

    /* Joyballs are relative input, so there's no poll state. Events only! */
}

static SDL_INLINE void
HandleInputEvents(SDL_Joystick *joystick)
{
    struct input_event events[32];
    int i, len;
    int code;

    if (joystick->hwdata->fresh) {
        PollAllValues(joystick);
        joystick->hwdata->fresh = SDL_FALSE;
    }

    while ((len = read(joystick->hwdata->fd, events, (sizeof events))) > 0) {
        len /= sizeof(events[0]);
        for (i = 0; i < len; ++i) {
            code = events[i].code;

            /* If the kernel sent a SYN_DROPPED, we are supposed to ignore the
               rest of the packet (the end of it signified by a SYN_REPORT) */
            if ( joystick->hwdata->recovering_from_dropped &&
                 ((events[i].type != EV_SYN) || (code != SYN_REPORT)) ) {
                continue;
            }

            switch (events[i].type) {
            case EV_KEY:
                SDL_PrivateJoystickButton(joystick,
                                          joystick->hwdata->key_map[code],
                                          events[i].value);
                break;
            case EV_ABS:
                switch (code) {
                case ABS_HAT0X:
                case ABS_HAT0Y:
                case ABS_HAT1X:
                case ABS_HAT1Y:
                case ABS_HAT2X:
                case ABS_HAT2Y:
                case ABS_HAT3X:
                case ABS_HAT3Y:
                    code -= ABS_HAT0X;
                    HandleHat(joystick, joystick->hwdata->hats_indices[code / 2], code % 2, events[i].value);
                    break;
                default:
                    if (joystick->hwdata->abs_map[code] != 0xFF) {
                        events[i].value =
                            AxisCorrect(joystick, code, events[i].value);
                        SDL_PrivateJoystickAxis(joystick,
                                                joystick->hwdata->abs_map[code],
                                                events[i].value);
                    }
                    break;
                }
                break;
            case EV_REL:
                switch (code) {
                case REL_X:
                case REL_Y:
                    code -= REL_X;
                    HandleBall(joystick, code / 2, code % 2, events[i].value);
                    break;
                default:
                    break;
                }
                break;
            case EV_SYN:
                switch (code) {
                case SYN_DROPPED :
#ifdef DEBUG_INPUT_EVENTS
                    printf("Event SYN_DROPPED detected\n");
#endif
                    joystick->hwdata->recovering_from_dropped = SDL_TRUE;
                    break;
                case SYN_REPORT :
                    if (joystick->hwdata->recovering_from_dropped) {
                        joystick->hwdata->recovering_from_dropped = SDL_FALSE;
                        PollAllValues(joystick);  /* try to sync up to current state now */
                    }
                    break;
                default:
                    break;
                }
            default:
                break;
            }
        }
    }

    if (errno == ENODEV) {
        /* We have to wait until the JoystickDetect callback to remove this */
        joystick->hwdata->gone = SDL_TRUE;
    }
}

static void
LINUX_JoystickUpdate(SDL_Joystick *joystick)
{
    int i;

    if (joystick->hwdata->m_bSteamController) {
        SDL_UpdateSteamController(joystick);
        return;
    }

    HandleInputEvents(joystick);

    /* Deliver ball motion updates */
    for (i = 0; i < joystick->nballs; ++i) {
        int xrel, yrel;

        xrel = joystick->hwdata->balls[i].axis[0];
        yrel = joystick->hwdata->balls[i].axis[1];
        if (xrel || yrel) {
            joystick->hwdata->balls[i].axis[0] = 0;
            joystick->hwdata->balls[i].axis[1] = 0;
            SDL_PrivateJoystickBall(joystick, (Uint8) i, xrel, yrel);
        }
    }
}

/* Function to close a joystick after use */
static void
LINUX_JoystickClose(SDL_Joystick *joystick)
{
    if (joystick->hwdata) {
        if (joystick->hwdata->effect.id >= 0) {
            ioctl(joystick->hwdata->fd, EVIOCRMFF, joystick->hwdata->effect.id);
            joystick->hwdata->effect.id = -1;
        }
        if (joystick->hwdata->fd >= 0) {
            close(joystick->hwdata->fd);
        }
        if (joystick->hwdata->item) {
            joystick->hwdata->item->hwdata = NULL;
        }
        SDL_free(joystick->hwdata->hats);
        SDL_free(joystick->hwdata->balls);
        SDL_free(joystick->hwdata->fname);
        SDL_free(joystick->hwdata);
    }
}

/* Function to perform any system-specific joystick related cleanup */
static void
LINUX_JoystickQuit(void)
{
    SDL_joylist_item *item = NULL;
    SDL_joylist_item *next = NULL;

    if (inotify_fd >= 0) {
        close(inotify_fd);
        inotify_fd = -1;
    }

    for (item = SDL_joylist; item; item = next) {
        next = item->next;
        SDL_free(item->path);
        SDL_free(item->name);
        SDL_free(item);
    }

    SDL_joylist = SDL_joylist_tail = NULL;

    numjoysticks = 0;

#if SDL_USE_LIBUDEV
    if (enumeration_method == ENUMERATION_LIBUDEV) {
        SDL_UDEV_DelCallback(joystick_udev_callback);
        SDL_UDEV_Quit();
    }
#endif

    SDL_QuitSteamControllers();
}

/*
   This is based on the Linux Gamepad Specification
   available at: https://www.kernel.org/doc/html/v4.15/input/gamepad.html
 */
static SDL_bool
LINUX_JoystickGetGamepadMapping(int device_index, SDL_GamepadMapping *out)
{
    SDL_Joystick *joystick;
    SDL_joylist_item *item = JoystickByDevIndex(device_index);

    if (item->mapping) {
        SDL_memcpy(out, item->mapping, sizeof(*out));
        return SDL_TRUE;
    }

    /* We temporarily open the device to check how it's configured. Make
       a fake SDL_Joystick object to do so. */
    joystick = (SDL_Joystick *) SDL_calloc(sizeof(*joystick), 1);
    if (joystick == NULL) {
        SDL_OutOfMemory();
        return SDL_FALSE;
    }

    joystick->hwdata = (struct joystick_hwdata *)
        SDL_calloc(1, sizeof(*joystick->hwdata));
    if (joystick->hwdata == NULL) {
        SDL_free(joystick);
        SDL_OutOfMemory();
        return SDL_FALSE;
    }

    if (PrepareJoystickHwdata(joystick, item) == -1) {
        SDL_free(joystick->hwdata);
        SDL_free(joystick);
        return SDL_FALSE;  /* SDL_SetError will already have been called */
    }

    /* don't assign `item->hwdata` so it's not in any global state. */

    /* it is now safe to call LINUX_JoystickClose on this fake joystick. */

    if (!joystick->hwdata->has_key[BTN_GAMEPAD]) {
        /* Not a gamepad according to the specs. */
        LINUX_JoystickClose(joystick);
        SDL_free(joystick);
        return SDL_FALSE;
    }

    /* We have a gamepad, start filling out the mappings */

    if (joystick->hwdata->has_key[BTN_A]) {
        out->a.kind = EMappingKind_Button;
        out->a.target = joystick->hwdata->key_map[BTN_A];
    }

    if (joystick->hwdata->has_key[BTN_B]) {
        out->b.kind = EMappingKind_Button;
        out->b.target = joystick->hwdata->key_map[BTN_B];
    }

    if (joystick->hwdata->has_key[BTN_X]) {
        out->x.kind = EMappingKind_Button;
        out->x.target = joystick->hwdata->key_map[BTN_X];
    }

    if (joystick->hwdata->has_key[BTN_Y]) {
        out->y.kind = EMappingKind_Button;
        out->y.target = joystick->hwdata->key_map[BTN_Y];
    }

    if (joystick->hwdata->has_key[BTN_SELECT]) {
        out->back.kind = EMappingKind_Button;
        out->back.target = joystick->hwdata->key_map[BTN_SELECT];
    }

    if (joystick->hwdata->has_key[BTN_START]) {
        out->start.kind = EMappingKind_Button;
        out->start.target = joystick->hwdata->key_map[BTN_START];
    }

    if (joystick->hwdata->has_key[BTN_THUMBL]) {
        out->leftstick.kind = EMappingKind_Button;
        out->leftstick.target = joystick->hwdata->key_map[BTN_THUMBL];
    }

    if (joystick->hwdata->has_key[BTN_THUMBR]) {
        out->rightstick.kind = EMappingKind_Button;
        out->rightstick.target = joystick->hwdata->key_map[BTN_THUMBR];
    }

    if (joystick->hwdata->has_key[BTN_MODE]) {
        out->guide.kind = EMappingKind_Button;
        out->guide.target = joystick->hwdata->key_map[BTN_MODE];
    }

    /*
       According to the specs the D-Pad, the shoulder buttons and the triggers
       can be digital, or analog, or both at the same time.
     */

    /* Prefer digital shoulder buttons, but settle for analog if missing. */
    if (joystick->hwdata->has_key[BTN_TL]) {
        out->leftshoulder.kind = EMappingKind_Button;
        out->leftshoulder.target = joystick->hwdata->key_map[BTN_TL];
    }

    if (joystick->hwdata->has_key[BTN_TR]) {
        out->rightshoulder.kind = EMappingKind_Button;
        out->rightshoulder.target = joystick->hwdata->key_map[BTN_TR];
    }

    if (joystick->hwdata->has_hat[1] && /* Check if ABS_HAT1{X, Y} is available. */
       (!joystick->hwdata->has_key[BTN_TL] || !joystick->hwdata->has_key[BTN_TR])) {
        int hat = joystick->hwdata->hats_indices[1] << 4;
        out->leftshoulder.kind = EMappingKind_Hat;
        out->rightshoulder.kind = EMappingKind_Hat;
        out->leftshoulder.target = hat | 0x4;
        out->rightshoulder.target = hat | 0x2;
    }

    /* Prefer analog triggers, but settle for digital if missing. */
    if (joystick->hwdata->has_hat[2]) { /* Check if ABS_HAT2{X,Y} is available. */
        int hat = joystick->hwdata->hats_indices[2] << 4;
        out->lefttrigger.kind = EMappingKind_Hat;
        out->righttrigger.kind = EMappingKind_Hat;
        out->lefttrigger.target = hat | 0x4;
        out->righttrigger.target = hat | 0x2;
    } else {
        if (joystick->hwdata->has_key[BTN_TL2]) {
            out->lefttrigger.kind = EMappingKind_Button;
            out->lefttrigger.target = joystick->hwdata->key_map[BTN_TL2];
        } else if (joystick->hwdata->has_abs[ABS_Z]) {
            out->lefttrigger.kind = EMappingKind_Axis;
            out->lefttrigger.target = joystick->hwdata->abs_map[ABS_Z];
        }

        if (joystick->hwdata->has_key[BTN_TR2]) {
            out->righttrigger.kind = EMappingKind_Button;
            out->righttrigger.target = joystick->hwdata->key_map[BTN_TR2];
        } else if (joystick->hwdata->has_abs[ABS_RZ]) {
            out->righttrigger.kind = EMappingKind_Axis;
            out->righttrigger.target = joystick->hwdata->abs_map[ABS_RZ];
        }
    }

    /* Prefer digital D-Pad, but settle for analog if missing. */
    if (joystick->hwdata->has_key[BTN_DPAD_UP]) {
        out->dpup.kind = EMappingKind_Button;
        out->dpup.target = joystick->hwdata->key_map[BTN_DPAD_UP];
    }

    if (joystick->hwdata->has_key[BTN_DPAD_DOWN]) {
        out->dpdown.kind = EMappingKind_Button;
        out->dpdown.target = joystick->hwdata->key_map[BTN_DPAD_DOWN];
    }

    if (joystick->hwdata->has_key[BTN_DPAD_LEFT]) {
        out->dpleft.kind = EMappingKind_Button;
        out->dpleft.target = joystick->hwdata->key_map[BTN_DPAD_LEFT];
    }

    if (joystick->hwdata->has_key[BTN_DPAD_RIGHT]) {
        out->dpright.kind = EMappingKind_Button;
        out->dpright.target = joystick->hwdata->key_map[BTN_DPAD_RIGHT];
    }

    if (joystick->hwdata->has_hat[0] && /* Check if ABS_HAT0{X,Y} is available. */
       (!joystick->hwdata->has_key[BTN_DPAD_LEFT] || !joystick->hwdata->has_key[BTN_DPAD_RIGHT] ||
        !joystick->hwdata->has_key[BTN_DPAD_UP] || !joystick->hwdata->has_key[BTN_DPAD_DOWN])) {
       int hat = joystick->hwdata->hats_indices[0] << 4;
       out->dpleft.kind = EMappingKind_Hat;
       out->dpright.kind = EMappingKind_Hat;
       out->dpup.kind = EMappingKind_Hat;
       out->dpdown.kind = EMappingKind_Hat;
       out->dpleft.target = hat | 0x8;
       out->dpright.target = hat | 0x2;
       out->dpup.target = hat | 0x1;
       out->dpdown.target = hat | 0x4;
    }

    if (joystick->hwdata->has_abs[ABS_X] && joystick->hwdata->has_abs[ABS_Y]) {
        out->leftx.kind = EMappingKind_Axis;
        out->lefty.kind = EMappingKind_Axis;
        out->leftx.target = joystick->hwdata->abs_map[ABS_X];
        out->lefty.target = joystick->hwdata->abs_map[ABS_Y];
    }

    if (joystick->hwdata->has_abs[ABS_RX] && joystick->hwdata->has_abs[ABS_RY]) {
        out->rightx.kind = EMappingKind_Axis;
        out->righty.kind = EMappingKind_Axis;
        out->rightx.target = joystick->hwdata->abs_map[ABS_RX];
        out->righty.target = joystick->hwdata->abs_map[ABS_RY];
    }

    LINUX_JoystickClose(joystick);
    SDL_free(joystick);

    /* Cache the mapping for later */
    item->mapping = (SDL_GamepadMapping *)SDL_malloc(sizeof(*item->mapping));
    if (item->mapping) {
        SDL_memcpy(item->mapping, out, sizeof(*out));
    }

    return SDL_TRUE;
}

SDL_JoystickDriver SDL_LINUX_JoystickDriver =
{
    LINUX_JoystickInit,
    LINUX_JoystickGetCount,
    LINUX_JoystickDetect,
    LINUX_JoystickGetDeviceName,
    LINUX_JoystickGetDevicePlayerIndex,
    LINUX_JoystickSetDevicePlayerIndex,
    LINUX_JoystickGetDeviceGUID,
    LINUX_JoystickGetDeviceInstanceID,
    LINUX_JoystickOpen,
    LINUX_JoystickRumble,
    LINUX_JoystickRumbleTriggers,
    LINUX_JoystickHasLED,
    LINUX_JoystickSetLED,
    LINUX_JoystickSendEffect,
    LINUX_JoystickSetSensorsEnabled,
    LINUX_JoystickUpdate,
    LINUX_JoystickClose,
    LINUX_JoystickQuit,
    LINUX_JoystickGetGamepadMapping
};

#endif /* SDL_JOYSTICK_LINUX */

/* vi: set ts=4 sw=4 expandtab: */
