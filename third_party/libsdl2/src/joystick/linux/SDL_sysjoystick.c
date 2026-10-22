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

#ifdef SDL_JOYSTICK_LINUX

#ifndef SDL_INPUT_LINUXEV
#error SDL now requires a Linux 2.4+ kernel with /dev/input/event support.
#endif

/* This is the Linux implementation of the SDL joystick API */

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <limits.h>             /* For the definition of PATH_MAX */
#include <linux/joystick.h>

#include "SDL_assert.h"
#include "SDL_joystick.h"
#include "SDL_endian.h"
#include "../../events/SDL_events_c.h"
#include "../SDL_sysjoystick.h"
#include "../SDL_joystick_c.h"
#include "../steam/SDL_steamcontroller.h"
#include "SDL_sysjoystick_c.h"

/* This isn't defined in older Linux kernel headers */
#ifndef SYN_DROPPED
#define SYN_DROPPED 3
#endif

#include "../../core/linux/SDL_udev.h"

static int MaybeAddDevice(const char *path);
#if SDL_USE_LIBUDEV
static int MaybeRemoveDevice(const char *path);
static void joystick_udev_callback(SDL_UDEV_deviceevent udev_type, int udev_class, const char *devpath);
#endif /* SDL_USE_LIBUDEV */


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
} SDL_joylist_item;

static SDL_joylist_item *SDL_joylist = NULL;
static SDL_joylist_item *SDL_joylist_tail = NULL;
static int numjoysticks = 0;
static int instance_counter = 0;


#define test_bit(nr, addr) \
    (((1UL << ((nr) % (sizeof(long) * 8))) & ((addr)[(nr) / (sizeof(long) * 8)])) != 0)
#define NBITS(x) ((((x)-1)/(sizeof(long) * 8))+1)

static int
IsJoystick(int fd, char *namebuf, const size_t namebuflen, SDL_JoystickGUID *guid)
{
    /* This list is taken from:
       https://raw.githubusercontent.com/denilsonsa/udev-joystick-blacklist/master/generate_rules.py
     */
    static Uint32 joystick_blacklist[] = {
        /* Microsoft Microsoft Wireless Optical Desktop® 2.10 */
        /* Microsoft Wireless Desktop - Comfort Edition */
        MAKE_VIDPID(0x045e, 0x009d),

        /* Microsoft Microsoft® Digital Media Pro Keyboard */
        /* Microsoft Corp. Digital Media Pro Keyboard */
        MAKE_VIDPID(0x045e, 0x00b0),

        /* Microsoft Microsoft® Digital Media Keyboard */
        /* Microsoft Corp. Digital Media Keyboard 1.0A */
        MAKE_VIDPID(0x045e, 0x00b4),

        /* Microsoft Microsoft® Digital Media Keyboard 3000 */
        MAKE_VIDPID(0x045e, 0x0730),

        /* Microsoft Microsoft® 2.4GHz Transceiver v6.0 */
        /* Microsoft Microsoft® 2.4GHz Transceiver v8.0 */
        /* Microsoft Corp. Nano Transceiver v1.0 for Bluetooth */
        /* Microsoft Wireless Mobile Mouse 1000 */
        /* Microsoft Wireless Desktop 3000 */
        MAKE_VIDPID(0x045e, 0x0745),

        /* Microsoft® SideWinder(TM) 2.4GHz Transceiver */
        MAKE_VIDPID(0x045e, 0x0748),

        /* Microsoft Corp. Wired Keyboard 600 */
        MAKE_VIDPID(0x045e, 0x0750),

        /* Microsoft Corp. Sidewinder X4 keyboard */
        MAKE_VIDPID(0x045e, 0x0768),

        /* Microsoft Corp. Arc Touch Mouse Transceiver */
        MAKE_VIDPID(0x045e, 0x0773),

        /* Microsoft® 2.4GHz Transceiver v9.0 */
        /* Microsoft® Nano Transceiver v2.1 */
        /* Microsoft Sculpt Ergonomic Keyboard (5KV-00001) */
        MAKE_VIDPID(0x045e, 0x07a5),

        /* Microsoft® Nano Transceiver v1.0 */
        /* Microsoft Wireless Keyboard 800 */
        MAKE_VIDPID(0x045e, 0x07b2),

        /* Microsoft® Nano Transceiver v2.0 */
        MAKE_VIDPID(0x045e, 0x0800),

        /* List of Wacom devices at: http://linuxwacom.sourceforge.net/wiki/index.php/Device_IDs */
        MAKE_VIDPID(0x056a, 0x0010),  /* Wacom ET-0405 Graphire */
        MAKE_VIDPID(0x056a, 0x0011),  /* Wacom ET-0405A Graphire2 (4x5) */
        MAKE_VIDPID(0x056a, 0x0012),  /* Wacom ET-0507A Graphire2 (5x7) */
        MAKE_VIDPID(0x056a, 0x0013),  /* Wacom CTE-430 Graphire3 (4x5) */
        MAKE_VIDPID(0x056a, 0x0014),  /* Wacom CTE-630 Graphire3 (6x8) */
        MAKE_VIDPID(0x056a, 0x0015),  /* Wacom CTE-440 Graphire4 (4x5) */
        MAKE_VIDPID(0x056a, 0x0016),  /* Wacom CTE-640 Graphire4 (6x8) */
        MAKE_VIDPID(0x056a, 0x0017),  /* Wacom CTE-450 Bamboo Fun (4x5) */
        MAKE_VIDPID(0x056a, 0x0016),  /* Wacom CTE-640 Graphire 4 6x8 */
        MAKE_VIDPID(0x056a, 0x0017),  /* Wacom CTE-450 Bamboo Fun 4x5 */
        MAKE_VIDPID(0x056a, 0x0018),  /* Wacom CTE-650 Bamboo Fun 6x8 */
        MAKE_VIDPID(0x056a, 0x0019),  /* Wacom CTE-631 Bamboo One */
        MAKE_VIDPID(0x056a, 0x00d1),  /* Wacom Bamboo Pen and Touch CTH-460 */

        MAKE_VIDPID(0x09da, 0x054f),  /* A4 Tech Co., G7 750 mouse */
        MAKE_VIDPID(0x09da, 0x3043),  /* A4 Tech Co., Ltd Bloody R8A Gaming Mouse */
        MAKE_VIDPID(0x09da, 0x31b5),  /* A4 Tech Co., Ltd Bloody TL80 Terminator Laser Gaming Mouse */
        MAKE_VIDPID(0x09da, 0x3997),  /* A4 Tech Co., Ltd Bloody RT7 Terminator Wireless */
        MAKE_VIDPID(0x09da, 0x3f8b),  /* A4 Tech Co., Ltd Bloody V8 mouse */
        MAKE_VIDPID(0x09da, 0x51f4),  /* Modecom MC-5006 Keyboard */
        MAKE_VIDPID(0x09da, 0x5589),  /* A4 Tech Co., Ltd Terminator TL9 Laser Gaming Mouse */
        MAKE_VIDPID(0x09da, 0x7b22),  /* A4 Tech Co., Ltd Bloody V5 */
        MAKE_VIDPID(0x09da, 0x7f2d),  /* A4 Tech Co., Ltd Bloody R3 mouse */
        MAKE_VIDPID(0x09da, 0x8090),  /* A4 Tech Co., Ltd X-718BK Oscar Optical Gaming Mouse */
        MAKE_VIDPID(0x09da, 0x9066),  /* A4 Tech Co., Sharkoon Fireglider Optical */
        MAKE_VIDPID(0x09da, 0x9090),  /* A4 Tech Co., Ltd XL-730K / XL-750BK / XL-755BK Laser Mouse */
        MAKE_VIDPID(0x09da, 0x90c0),  /* A4 Tech Co., Ltd X7 G800V keyboard */
        MAKE_VIDPID(0x09da, 0xf012),  /* A4 Tech Co., Ltd Bloody V7 mouse */
        MAKE_VIDPID(0x09da, 0xf32a),  /* A4 Tech Co., Ltd Bloody B540 keyboard */
        MAKE_VIDPID(0x09da, 0xf613),  /* A4 Tech Co., Ltd Bloody V2 mouse */
        MAKE_VIDPID(0x09da, 0xf624),  /* A4 Tech Co., Ltd Bloody B120 Keyboard */

        MAKE_VIDPID(0x1d57, 0xad03),  /* [T3] 2.4GHz and IR Air Mouse Remote Control */

        MAKE_VIDPID(0x1e7d, 0x2e4a),  /* Roccat Tyon Mouse */

        MAKE_VIDPID(0x20a0, 0x422d),  /* Winkeyless.kr Keyboards */

        MAKE_VIDPID(0x2516, 0x001f),  /* Cooler Master Storm Mizar Mouse */
        MAKE_VIDPID(0x2516, 0x0028),  /* Cooler Master Storm Alcor Mouse */
    };
    struct input_id inpid;
    int i;
    Uint32 id;
    Uint16 *guid16 = (Uint16 *)guid->data;

#if !SDL_USE_LIBUDEV
    /* When udev is enabled we only get joystick devices here, so there's no need to test them */
    unsigned long evbit[NBITS(EV_MAX)] = { 0 };
    unsigned long keybit[NBITS(KEY_MAX)] = { 0 };
    unsigned long absbit[NBITS(ABS_MAX)] = { 0 };

    if ((ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), evbit) < 0) ||
        (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keybit)), keybit) < 0) ||
        (ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absbit)), absbit) < 0)) {
        return (0);
    }

    if (!(test_bit(EV_KEY, evbit) && test_bit(EV_ABS, evbit) &&
          test_bit(ABS_X, absbit) && test_bit(ABS_Y, absbit))) {
        return 0;
    }
#endif

    if (ioctl(fd, EVIOCGNAME(namebuflen), namebuf) < 0) {
        return 0;
    }

    if (ioctl(fd, EVIOCGID, &inpid) < 0) {
        return 0;
    }

    /* Check the joystick blacklist */
    id = MAKE_VIDPID(inpid.vendor, inpid.product);
    for (i = 0; i < SDL_arraysize(joystick_blacklist); ++i) {
        if (id == joystick_blacklist[i]) {
            return 0;
        }
    }

#ifdef DEBUG_JOYSTICK
    printf("Joystick: %s, bustype = %d, vendor = 0x%.4x, product = 0x%.4x, version = %d\n", namebuf, inpid.bustype, inpid.vendor, inpid.product, inpid.version);
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
        SDL_strlcpy((char*)guid16, namebuf, sizeof(guid->data) - 4);
    }

    if (SDL_IsGameControllerNameAndGUID(namebuf, *guid) &&
        SDL_ShouldIgnoreGameController(namebuf, *guid)) {
        return 0;
    }
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


/* !!! FIXME: I would love to dump this code and use libudev instead. */
static int
MaybeAddDevice(const char *path)
{
    struct stat sb;
    int fd = -1;
    int isstick = 0;
    char namebuf[128];
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

    isstick = IsJoystick(fd, namebuf, sizeof (namebuf), &guid);
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
    item->name = SDL_strdup(namebuf);
    item->guid = guid;

    if ( (item->path == NULL) || (item->name == NULL) ) {
         SDL_free(item->path);
         SDL_free(item->name);
         SDL_free(item);
         return -1;
    }

    item->device_instance = instance_counter++;
    if (SDL_joylist_tail == NULL) {
        SDL_joylist = SDL_joylist_tail = item;
    } else {
        SDL_joylist_tail->next = item;
        SDL_joylist_tail = item;
    }

    /* Need to increment the joystick count before we post the event */
    ++numjoysticks;

    SDL_PrivateJoystickAdded(numjoysticks - 1);

    return numjoysticks;
}

#if SDL_USE_LIBUDEV
/* !!! FIXME: I would love to dump this code and use libudev instead. */
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

            SDL_free(item->path);
            SDL_free(item->name);
            SDL_free(item);
            return retval;
        }
        prev = item;
    }

    return -1;
}
#endif

#if ! SDL_USE_LIBUDEV
static int
JoystickInitWithoutUdev(void)
{
    int i;
    char path[PATH_MAX];

    /* !!! FIXME: only finds sticks if they're called /dev/input/event[0..31] */
    /* !!! FIXME:  we could at least readdir() through /dev/input...? */
    /* !!! FIXME:  (or delete this and rely on libudev?) */
    for (i = 0; i < 32; i++) {
        SDL_snprintf(path, SDL_arraysize(path), "/dev/input/event%d", i);
        MaybeAddDevice(path);
    }

    return numjoysticks;
}
#endif

#if SDL_USE_LIBUDEV
static int
JoystickInitWithUdev(void)
{
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

    return numjoysticks;
}
#endif

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

    *device_instance = item->device_instance = instance_counter++;
    if (SDL_joylist_tail == NULL) {
        SDL_joylist = SDL_joylist_tail = item;
    } else {
        SDL_joylist_tail->next = item;
        SDL_joylist_tail = item;
    }

    /* Need to increment the joystick count before we post the event */
    ++numjoysticks;

    SDL_PrivateJoystickAdded(numjoysticks - 1);

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

int
SDL_SYS_JoystickInit(void)
{
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

#if SDL_USE_LIBUDEV
    return JoystickInitWithUdev();
#else 
    return JoystickInitWithoutUdev();
#endif
}

int
SDL_SYS_NumJoysticks(void)
{
    return numjoysticks;
}

void
SDL_SYS_JoystickDetect(void)
{
#if SDL_USE_LIBUDEV
    SDL_UDEV_Poll();
#endif

    SDL_UpdateSteamControllers();
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
const char *
SDL_SYS_JoystickNameForDeviceIndex(int device_index)
{
    return JoystickByDevIndex(device_index)->name;
}

/* Function to perform the mapping from device index to the instance id for this index */
SDL_JoystickID SDL_SYS_GetInstanceIdOfDeviceIndex(int device_index)
{
    return JoystickByDevIndex(device_index)->device_instance;
}

static int
allocate_hatdata(SDL_Joystick * joystick)
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
allocate_balldata(SDL_Joystick * joystick)
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
ConfigJoystick(SDL_Joystick * joystick, int fd)
{
    int i, t;
    unsigned long keybit[NBITS(KEY_MAX)] = { 0 };
    unsigned long absbit[NBITS(ABS_MAX)] = { 0 };
    unsigned long relbit[NBITS(REL_MAX)] = { 0 };

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
                ++joystick->nbuttons;
            }
        }
        for (i = 0; i < BTN_JOYSTICK; ++i) {
            if (test_bit(i, keybit)) {
#ifdef DEBUG_INPUT_EVENTS
                printf("Joystick has button: 0x%x\n", i);
#endif
                joystick->hwdata->key_map[i] = joystick->nbuttons;
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
                if (absinfo.minimum == absinfo.maximum) {
                    joystick->hwdata->abs_correct[i].used = 0;
                } else {
                    joystick->hwdata->abs_correct[i].used = 1;
                    joystick->hwdata->abs_correct[i].coef[0] =
                        (absinfo.maximum + absinfo.minimum) - 2 * absinfo.flat;
                    joystick->hwdata->abs_correct[i].coef[1] =
                        (absinfo.maximum + absinfo.minimum) + 2 * absinfo.flat;
                    t = ((absinfo.maximum - absinfo.minimum) - 4 * absinfo.flat);
                    if (t != 0) {
                        joystick->hwdata->abs_correct[i].coef[2] =
                            (1 << 28) / t;
                    } else {
                        joystick->hwdata->abs_correct[i].coef[2] = 0;
                    }
                }
                ++joystick->naxes;
            }
        }
        for (i = ABS_HAT0X; i <= ABS_HAT3Y; i += 2) {
            if (test_bit(i, absbit) || test_bit(i + 1, absbit)) {
                struct input_absinfo absinfo;

                if (ioctl(fd, EVIOCGABS(i), &absinfo) < 0) {
                    continue;
                }
#ifdef DEBUG_INPUT_EVENTS
                printf("Joystick has hat %d\n", (i - ABS_HAT0X) / 2);
                printf("Values = { %d, %d, %d, %d, %d }\n",
                       absinfo.value, absinfo.minimum, absinfo.maximum,
                       absinfo.fuzz, absinfo.flat);
#endif /* DEBUG_INPUT_EVENTS */
                ++joystick->nhats;
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
}


/* Function to open a joystick for use.
   The joystick to open is specified by the device index.
   This should fill the nbuttons and naxes fields of the joystick structure.
   It returns 0, or -1 if there is an error.
 */
int
SDL_SYS_JoystickOpen(SDL_Joystick * joystick, int device_index)
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
    joystick->hwdata->item = item;
    joystick->hwdata->guid = item->guid;
    joystick->hwdata->m_bSteamController = item->m_bSteamController;

    if (item->m_bSteamController) {
        joystick->hwdata->fd = -1;
        SDL_GetSteamControllerInputs(&joystick->nbuttons,
                                     &joystick->naxes,
                                     &joystick->nhats);
    } else {
        int fd = open(item->path, O_RDONLY, 0);
        if (fd < 0) {
            SDL_free(joystick->hwdata);
            joystick->hwdata = NULL;
            return SDL_SetError("Unable to open %s", item->path);
        }

        joystick->hwdata->fd = fd;
        joystick->hwdata->fname = SDL_strdup(item->path);
        if (joystick->hwdata->fname == NULL) {
            SDL_free(joystick->hwdata);
            joystick->hwdata = NULL;
            close(fd);
            return SDL_OutOfMemory();
        }

        /* Set the joystick to non-blocking read mode */
        fcntl(fd, F_SETFL, O_NONBLOCK);

        /* Get the number of buttons and axes on the joystick */
        ConfigJoystick(joystick, fd);
    }

    SDL_assert(item->hwdata == NULL);
    item->hwdata = joystick->hwdata;

    /* mark joystick as fresh and ready */
    joystick->hwdata->fresh = 1;

    return (0);
}

/* Function to determine if this joystick is attached to the system right now */
SDL_bool SDL_SYS_JoystickAttached(SDL_Joystick *joystick)
{
    return joystick->hwdata->item != NULL;
}

static SDL_INLINE void
HandleHat(SDL_Joystick * stick, Uint8 hat, int axis, int value)
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
                               position_map[the_hat->
                                            axis[1]][the_hat->axis[0]]);
    }
}

static SDL_INLINE void
HandleBall(SDL_Joystick * stick, Uint8 ball, int axis, int value)
{
    stick->hwdata->balls[ball].axis[axis] += value;
}


static SDL_INLINE int
AxisCorrect(SDL_Joystick * joystick, int which, int value)
{
    struct axis_correct *correct;

    correct = &joystick->hwdata->abs_correct[which];
    if (correct->used) {
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
    }

    /* Clamp and return */
    if (value < -32768)
        return -32768;
    if (value > 32767)
        return 32767;

    return value;
}

static SDL_INLINE void
PollAllValues(SDL_Joystick * joystick)
{
    struct input_absinfo absinfo;
    int a, b = 0;

    /* Poll all axis */
    for (a = ABS_X; b < ABS_MAX; a++) {
        switch (a) {
        case ABS_HAT0X:
        case ABS_HAT0Y:
        case ABS_HAT1X:
        case ABS_HAT1Y:
        case ABS_HAT2X:
        case ABS_HAT2Y:
        case ABS_HAT3X:
        case ABS_HAT3Y:
            /* ingore hats */
            break;
        default:
            if (joystick->hwdata->abs_correct[b].used) {
                if (ioctl(joystick->hwdata->fd, EVIOCGABS(a), &absinfo) >= 0) {
                    absinfo.value = AxisCorrect(joystick, b, absinfo.value);

#ifdef DEBUG_INPUT_EVENTS
                    printf("Joystick : Re-read Axis %d (%d) val= %d\n",
                        joystick->hwdata->abs_map[b], a, absinfo.value);
#endif
                    SDL_PrivateJoystickAxis(joystick,
                            joystick->hwdata->abs_map[b],
                            absinfo.value);
                }
            }
            b++;
        }
    }
}

static SDL_INLINE void
HandleInputEvents(SDL_Joystick * joystick)
{
    struct input_event events[32];
    int i, len;
    int code;

    if (joystick->hwdata->fresh) {
        PollAllValues(joystick);
        joystick->hwdata->fresh = 0;
    }

    while ((len = read(joystick->hwdata->fd, events, (sizeof events))) > 0) {
        len /= sizeof(events[0]);
        for (i = 0; i < len; ++i) {
            code = events[i].code;
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
                    HandleHat(joystick, code / 2, code % 2, events[i].value);
                    break;
                default:
                    events[i].value =
                        AxisCorrect(joystick, code, events[i].value);
                    SDL_PrivateJoystickAxis(joystick,
                                            joystick->hwdata->abs_map[code],
                                            events[i].value);
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
                    PollAllValues(joystick);
                    break;
                default:
                    break;
                }
            default:
                break;
            }
        }
    }
}

void
SDL_SYS_JoystickUpdate(SDL_Joystick * joystick)
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
void
SDL_SYS_JoystickClose(SDL_Joystick * joystick)
{
    if (joystick->hwdata) {
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
void
SDL_SYS_JoystickQuit(void)
{
    SDL_joylist_item *item = NULL;
    SDL_joylist_item *next = NULL;

    for (item = SDL_joylist; item; item = next) {
        next = item->next;
        SDL_free(item->path);
        SDL_free(item->name);
        SDL_free(item);
    }

    SDL_joylist = SDL_joylist_tail = NULL;

    numjoysticks = 0;
    instance_counter = 0;

#if SDL_USE_LIBUDEV
    SDL_UDEV_DelCallback(joystick_udev_callback);
    SDL_UDEV_Quit();
#endif

    SDL_QuitSteamControllers();
}

SDL_JoystickGUID SDL_SYS_JoystickGetDeviceGUID( int device_index )
{
    return JoystickByDevIndex(device_index)->guid;
}

SDL_JoystickGUID SDL_SYS_JoystickGetGUID(SDL_Joystick * joystick)
{
    return joystick->hwdata->guid;
}

#endif /* SDL_JOYSTICK_LINUX */

/* vi: set ts=4 sw=4 expandtab: */
