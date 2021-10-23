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

#ifdef SDL_JOYSTICK_USBHID

/*
 * Joystick driver for the uhid(4) interface found in OpenBSD,
 * NetBSD and FreeBSD.
 *
 * Maintainer: <vedge at csoft.org>
 */

#include <sys/param.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#ifndef __FreeBSD_kernel_version
#define __FreeBSD_kernel_version __FreeBSD_version
#endif

#if defined(HAVE_USB_H)
#include <usb.h>
#endif
#ifdef __DragonFly__
#include <bus/u4b/usb.h>
#include <bus/u4b/usbhid.h>
#else
#include <dev/usb/usb.h>
#include <dev/usb/usbhid.h>
#endif

#if defined(HAVE_USBHID_H)
#include <usbhid.h>
#elif defined(HAVE_LIBUSB_H)
#include <libusb.h>
#elif defined(HAVE_LIBUSBHID_H)
#include <libusbhid.h>
#endif

#if defined(__FREEBSD__) || defined(__FreeBSD_kernel__)
#include <osreldate.h>
#if __FreeBSD_kernel_version > 800063
#include <dev/usb/usb_ioctl.h>
#endif
#include <sys/joystick.h>
#elif defined(__DragonFly__)
#include <bus/u4b/usb_ioctl.h>
#include <sys/joystick.h>
#endif

#if SDL_HAVE_MACHINE_JOYSTICK_H
#include <machine/joystick.h>
#endif

#include "SDL_joystick.h"
#include "../SDL_sysjoystick.h"
#include "../SDL_joystick_c.h"

#define MAX_UHID_JOYS   64
#define MAX_JOY_JOYS    2
#define MAX_JOYS    (MAX_UHID_JOYS + MAX_JOY_JOYS)

#ifdef __OpenBSD__

#define HUG_DPAD_UP         0x90
#define HUG_DPAD_DOWN       0x91
#define HUG_DPAD_RIGHT      0x92
#define HUG_DPAD_LEFT       0x93

#define HAT_CENTERED        0x00
#define HAT_UP              0x01
#define HAT_RIGHT           0x02
#define HAT_DOWN            0x04
#define HAT_LEFT            0x08
#define HAT_RIGHTUP         (HAT_RIGHT|HAT_UP)
#define HAT_RIGHTDOWN       (HAT_RIGHT|HAT_DOWN)
#define HAT_LEFTUP          (HAT_LEFT|HAT_UP)
#define HAT_LEFTDOWN        (HAT_LEFT|HAT_DOWN)

/* calculate the value from the state of the dpad */
int
dpad_to_sdl(Sint32 *dpad)
{
    if (dpad[2]) {
        if (dpad[0])
            return HAT_RIGHTUP;
        else if (dpad[1])
            return HAT_RIGHTDOWN;
        else
            return HAT_RIGHT;
    } else if (dpad[3]) {
        if (dpad[0])
            return HAT_LEFTUP;
        else if (dpad[1])
            return HAT_LEFTDOWN;
        else
            return HAT_LEFT;
    } else if (dpad[0]) {
        return HAT_UP;
    } else if (dpad[1]) {
        return HAT_DOWN;
    }
    return HAT_CENTERED;
}
#endif

struct report
{
#if defined(__FREEBSD__) && (__FreeBSD_kernel_version > 900000) || \
    defined(__DragonFly__)
    void *buf; /* Buffer */
#elif defined(__FREEBSD__) && (__FreeBSD_kernel_version > 800063)
    struct usb_gen_descriptor *buf; /* Buffer */
#else
    struct usb_ctl_report *buf; /* Buffer */
#endif
    size_t size;                /* Buffer size */
    int rid;                    /* Report ID */
    enum
    {
        SREPORT_UNINIT,
        SREPORT_CLEAN,
        SREPORT_DIRTY
    } status;
};

static struct
{
    int uhid_report;
    hid_kind_t kind;
    const char *name;
} const repinfo[] = {
    {UHID_INPUT_REPORT, hid_input, "input"},
    {UHID_OUTPUT_REPORT, hid_output, "output"},
    {UHID_FEATURE_REPORT, hid_feature, "feature"}
};

enum
{
    REPORT_INPUT = 0,
    REPORT_OUTPUT = 1,
    REPORT_FEATURE = 2
};

enum
{
    JOYAXE_X,
    JOYAXE_Y,
    JOYAXE_Z,
    JOYAXE_SLIDER,
    JOYAXE_WHEEL,
    JOYAXE_RX,
    JOYAXE_RY,
    JOYAXE_RZ,
    JOYAXE_count
};

struct joystick_hwdata
{
    int fd;
    char *path;
    enum
    {
        BSDJOY_UHID,            /* uhid(4) */
        BSDJOY_JOY              /* joy(4) */
    } type;
    struct report_desc *repdesc;
    struct report inreport;
    int axis_map[JOYAXE_count]; /* map present JOYAXE_* to 0,1,.. */
};

static char *joynames[MAX_JOYS];
static char *joydevnames[MAX_JOYS];

static int report_alloc(struct report *, struct report_desc *, int);
static void report_free(struct report *);

#if defined(USBHID_UCR_DATA) || (defined(__FreeBSD_kernel__) && __FreeBSD_kernel_version <= 800063)
#define REP_BUF_DATA(rep) ((rep)->buf->ucr_data)
#elif (defined(__FREEBSD__) && (__FreeBSD_kernel_version > 900000)) || \
    defined(__DragonFly__)
#define REP_BUF_DATA(rep) ((rep)->buf)
#elif (defined(__FREEBSD__) && (__FreeBSD_kernel_version > 800063))
#define REP_BUF_DATA(rep) ((rep)->buf->ugd_data)
#else
#define REP_BUF_DATA(rep) ((rep)->buf->data)
#endif

static int numjoysticks = 0;

static int BSD_JoystickOpen(SDL_Joystick *joy, int device_index);
static void BSD_JoystickClose(SDL_Joystick *joy);

static int
BSD_JoystickInit(void)
{
    char s[16];
    int i, fd;

    numjoysticks = 0;

    SDL_memset(joynames, 0, sizeof(joynames));
    SDL_memset(joydevnames, 0, sizeof(joydevnames));

    for (i = 0; i < MAX_UHID_JOYS; i++) {
        SDL_Joystick nj;

        SDL_snprintf(s, SDL_arraysize(s), "/dev/uhid%d", i);

        joynames[numjoysticks] = SDL_strdup(s);

        if (BSD_JoystickOpen(&nj, numjoysticks) == 0) {
            BSD_JoystickClose(&nj);
            numjoysticks++;
        } else {
            SDL_free(joynames[numjoysticks]);
            joynames[numjoysticks] = NULL;
        }
    }
    for (i = 0; i < MAX_JOY_JOYS; i++) {
        SDL_snprintf(s, SDL_arraysize(s), "/dev/joy%d", i);
        fd = open(s, O_RDONLY);
        if (fd != -1) {
            joynames[numjoysticks++] = SDL_strdup(s);
            close(fd);
        }
    }

    /* Read the default USB HID usage table. */
    hid_init(NULL);

    return (numjoysticks);
}

static int
BSD_JoystickGetCount(void)
{
    return numjoysticks;
}

static void
BSD_JoystickDetect(void)
{
}

static const char *
BSD_JoystickGetDeviceName(int device_index)
{
    if (joydevnames[device_index] != NULL) {
        return (joydevnames[device_index]);
    }
    return (joynames[device_index]);
}

static int
BSD_JoystickGetDevicePlayerIndex(int device_index)
{
    return -1;
}

static void
BSD_JoystickSetDevicePlayerIndex(int device_index, int player_index)
{
}

/* Function to perform the mapping from device index to the instance id for this index */
static SDL_JoystickID
BSD_JoystickGetDeviceInstanceID(int device_index)
{
    return device_index;
}

static int
usage_to_joyaxe(unsigned usage)
{
    int joyaxe;
    switch (usage) {
    case HUG_X:
        joyaxe = JOYAXE_X;
        break;
    case HUG_Y:
        joyaxe = JOYAXE_Y;
        break;
    case HUG_Z:
        joyaxe = JOYAXE_Z;
        break;
    case HUG_SLIDER:
        joyaxe = JOYAXE_SLIDER;
        break;
    case HUG_WHEEL:
        joyaxe = JOYAXE_WHEEL;
        break;
    case HUG_RX:
        joyaxe = JOYAXE_RX;
        break;
    case HUG_RY:
        joyaxe = JOYAXE_RY;
        break;
    case HUG_RZ:
        joyaxe = JOYAXE_RZ;
        break;
    default:
        joyaxe = -1;
    }
    return joyaxe;
}

static unsigned
hatval_to_sdl(Sint32 hatval)
{
    static const unsigned hat_dir_map[8] = {
        SDL_HAT_UP, SDL_HAT_RIGHTUP, SDL_HAT_RIGHT, SDL_HAT_RIGHTDOWN,
        SDL_HAT_DOWN, SDL_HAT_LEFTDOWN, SDL_HAT_LEFT, SDL_HAT_LEFTUP
    };
    unsigned result;
    if ((hatval & 7) == hatval)
        result = hat_dir_map[hatval];
    else
        result = SDL_HAT_CENTERED;
    return result;
}


static int
BSD_JoystickOpen(SDL_Joystick *joy, int device_index)
{
    char *path = joynames[device_index];
    struct joystick_hwdata *hw;
    struct hid_item hitem;
    struct hid_data *hdata;
    struct report *rep = NULL;
#if defined(__NetBSD__)
    usb_device_descriptor_t udd;
    struct usb_string_desc usd;
#endif
    int fd;
    int i;

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        return SDL_SetError("%s: %s", path, strerror(errno));
    }

    joy->instance_id = device_index;
    hw = (struct joystick_hwdata *)
        SDL_malloc(sizeof(struct joystick_hwdata));
    if (hw == NULL) {
        close(fd);
        return SDL_OutOfMemory();
    }
    joy->hwdata = hw;
    hw->fd = fd;
    hw->path = SDL_strdup(path);
    if (!SDL_strncmp(path, "/dev/joy", 8)) {
        hw->type = BSDJOY_JOY;
        joy->naxes = 2;
        joy->nbuttons = 2;
        joy->nhats = 0;
        joy->nballs = 0;
        joydevnames[device_index] = SDL_strdup("Gameport joystick");
        goto usbend;
    } else {
        hw->type = BSDJOY_UHID;
    }

    {
        int ax;
        for (ax = 0; ax < JOYAXE_count; ax++)
            hw->axis_map[ax] = -1;
    }
    hw->repdesc = hid_get_report_desc(fd);
    if (hw->repdesc == NULL) {
        SDL_SetError("%s: USB_GET_REPORT_DESC: %s", hw->path,
                     strerror(errno));
        goto usberr;
    }
    rep = &hw->inreport;
#if defined(__FREEBSD__) && (__FreeBSD_kernel_version > 800063) || defined(__FreeBSD_kernel__) || defined(__DragonFly__)
    rep->rid = hid_get_report_id(fd);
    if (rep->rid < 0) {
#else
    if (ioctl(fd, USB_GET_REPORT_ID, &rep->rid) < 0) {
#endif
        rep->rid = -1;          /* XXX */
    }
#if defined(__NetBSD__)
    if (ioctl(fd, USB_GET_DEVICE_DESC, &udd) == -1)
        goto desc_failed;

    /* Get default language */
    usd.usd_string_index = USB_LANGUAGE_TABLE;
    usd.usd_language_id = 0;
    if (ioctl(fd, USB_GET_STRING_DESC, &usd) == -1 || usd.usd_desc.bLength < 4) {
        usd.usd_language_id = 0;
    } else {
        usd.usd_language_id = UGETW(usd.usd_desc.bString[0]);
    }

    usd.usd_string_index = udd.iProduct;
    if (ioctl(fd, USB_GET_STRING_DESC, &usd) == 0) {
        char str[128];
        char *new_name = NULL;
        int i;
        for (i = 0; i < (usd.usd_desc.bLength >> 1) - 1 && i < sizeof(str) - 1; i++) {
            str[i] = UGETW(usd.usd_desc.bString[i]);
        }
        str[i] = '\0';
        asprintf(&new_name, "%s @ %s", str, path);
        if (new_name != NULL) {
            SDL_free(joydevnames[numjoysticks]);
            joydevnames[numjoysticks] = new_name;
        }
    }
desc_failed:
#endif
    if (report_alloc(rep, hw->repdesc, REPORT_INPUT) < 0) {
        goto usberr;
    }
    if (rep->size <= 0) {
        SDL_SetError("%s: Input report descriptor has invalid length",
                     hw->path);
        goto usberr;
    }
#if defined(USBHID_NEW) || (defined(__FREEBSD__) && __FreeBSD_kernel_version >= 500111) || defined(__FreeBSD_kernel__) || defined(__DragonFly__)
    hdata = hid_start_parse(hw->repdesc, 1 << hid_input, rep->rid);
#else
    hdata = hid_start_parse(hw->repdesc, 1 << hid_input);
#endif
    if (hdata == NULL) {
        SDL_SetError("%s: Cannot start HID parser", hw->path);
        goto usberr;
    }
    joy->naxes = 0;
    joy->nbuttons = 0;
    joy->nhats = 0;
    joy->nballs = 0;
    for (i = 0; i < JOYAXE_count; i++)
        hw->axis_map[i] = -1;

    while (hid_get_item(hdata, &hitem) > 0) {
        char *sp;
        const char *s;

        switch (hitem.kind) {
        case hid_collection:
            switch (HID_PAGE(hitem.usage)) {
            case HUP_GENERIC_DESKTOP:
                switch (HID_USAGE(hitem.usage)) {
                case HUG_JOYSTICK:
                case HUG_GAME_PAD:
                    s = hid_usage_in_page(hitem.usage);
                    sp = SDL_malloc(SDL_strlen(s) + 5);
                    SDL_snprintf(sp, SDL_strlen(s) + 5, "%s (%d)",
                                 s, device_index);
                    joydevnames[device_index] = sp;
                }
            }
            break;
        case hid_input:
            switch (HID_PAGE(hitem.usage)) {
            case HUP_GENERIC_DESKTOP:
                {
                    unsigned usage = HID_USAGE(hitem.usage);
                    int joyaxe = usage_to_joyaxe(usage);
                    if (joyaxe >= 0) {
                        hw->axis_map[joyaxe] = 1;
                    } else if (usage == HUG_HAT_SWITCH
#ifdef __OpenBSD__
                               || usage == HUG_DPAD_UP
#endif
                               ) {
                        joy->nhats++;
                    }
                    break;
                }
            case HUP_BUTTON:
                joy->nbuttons++;
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
    hid_end_parse(hdata);
    for (i = 0; i < JOYAXE_count; i++)
        if (hw->axis_map[i] > 0)
            hw->axis_map[i] = joy->naxes++;

    if (joy->naxes == 0 && joy->nbuttons == 0 && joy->nhats == 0 && joy->nballs == 0) {
        SDL_SetError("%s: Not a joystick, ignoring", hw->path);
        goto usberr;
    }

  usbend:
    /* The poll blocks the event thread. */
    fcntl(fd, F_SETFL, O_NONBLOCK);
#ifdef __NetBSD__
    /* Flush pending events */
    if (rep) {
        while (read(joy->hwdata->fd, REP_BUF_DATA(rep), rep->size) == rep->size)
            ;
    }
#endif

    return (0);
  usberr:
    close(hw->fd);
    SDL_free(hw->path);
    SDL_free(hw);
    return (-1);
}

static void
BSD_JoystickUpdate(SDL_Joystick *joy)
{
    struct hid_item hitem;
    struct hid_data *hdata;
    struct report *rep;
    int nbutton, naxe = -1;
    Sint32 v;
#ifdef __OpenBSD__
    Sint32 dpad[4] = {0, 0, 0, 0};
#endif

#if defined(__FREEBSD__) || SDL_HAVE_MACHINE_JOYSTICK_H || defined(__FreeBSD_kernel__) || defined(__DragonFly_)
    struct joystick gameport;
    static int x, y, xmin = 0xffff, ymin = 0xffff, xmax = 0, ymax = 0;

    if (joy->hwdata->type == BSDJOY_JOY) {
        while (read(joy->hwdata->fd, &gameport, sizeof gameport) == sizeof gameport) {
            if (abs(x - gameport.x) > 8) {
                x = gameport.x;
                if (x < xmin) {
                    xmin = x;
                }
                if (x > xmax) {
                    xmax = x;
                }
                if (xmin == xmax) {
                    xmin--;
                    xmax++;
                }
                v = (Sint32) x;
                v -= (xmax + xmin + 1) / 2;
                v *= 32768 / ((xmax - xmin + 1) / 2);
                SDL_PrivateJoystickAxis(joy, 0, v);
            }
            if (abs(y - gameport.y) > 8) {
                y = gameport.y;
                if (y < ymin) {
                    ymin = y;
                }
                if (y > ymax) {
                    ymax = y;
                }
                if (ymin == ymax) {
                    ymin--;
                    ymax++;
                }
                v = (Sint32) y;
                v -= (ymax + ymin + 1) / 2;
                v *= 32768 / ((ymax - ymin + 1) / 2);
                SDL_PrivateJoystickAxis(joy, 1, v);
            }
            SDL_PrivateJoystickButton(joy, 0, gameport.b1);
            SDL_PrivateJoystickButton(joy, 1, gameport.b2);
        }
        return;
    }
#endif /* defined(__FREEBSD__) || SDL_HAVE_MACHINE_JOYSTICK_H */

    rep = &joy->hwdata->inreport;

    while (read(joy->hwdata->fd, REP_BUF_DATA(rep), rep->size) == rep->size) {
#if defined(USBHID_NEW) || (defined(__FREEBSD__) && __FreeBSD_kernel_version >= 500111) || defined(__FreeBSD_kernel__) || defined(__DragonFly__)
        hdata = hid_start_parse(joy->hwdata->repdesc, 1 << hid_input, rep->rid);
#else
        hdata = hid_start_parse(joy->hwdata->repdesc, 1 << hid_input);
#endif
        if (hdata == NULL) {
            /*fprintf(stderr, "%s: Cannot start HID parser\n", joy->hwdata->path);*/
            continue;
        }

        for (nbutton = 0; hid_get_item(hdata, &hitem) > 0;) {
            switch (hitem.kind) {
            case hid_input:
                switch (HID_PAGE(hitem.usage)) {
                case HUP_GENERIC_DESKTOP:
                    {
                        unsigned usage = HID_USAGE(hitem.usage);
                        int joyaxe = usage_to_joyaxe(usage);
                        if (joyaxe >= 0) {
                            naxe = joy->hwdata->axis_map[joyaxe];
                            /* scaleaxe */
                            v = (Sint32) hid_get_data(REP_BUF_DATA(rep), &hitem);
                            v -= (hitem.logical_maximum +
                                  hitem.logical_minimum + 1) / 2;
                            v *= 32768 /
                                ((hitem.logical_maximum -
                                  hitem.logical_minimum + 1) / 2);
                            SDL_PrivateJoystickAxis(joy, naxe, v);
                        } else if (usage == HUG_HAT_SWITCH) {
                            v = (Sint32) hid_get_data(REP_BUF_DATA(rep), &hitem);
                            SDL_PrivateJoystickHat(joy, 0,
                                                   hatval_to_sdl(v) -
                                                   hitem.logical_minimum);
                        }
#ifdef __OpenBSD__
                        else if (usage == HUG_DPAD_UP) {
                            dpad[0] = (Sint32) hid_get_data(REP_BUF_DATA(rep), &hitem);
                            SDL_PrivateJoystickHat(joy, 0, dpad_to_sdl(dpad));
                        }
                        else if (usage == HUG_DPAD_DOWN) {
                            dpad[1] = (Sint32) hid_get_data(REP_BUF_DATA(rep), &hitem);
                            SDL_PrivateJoystickHat(joy, 0, dpad_to_sdl(dpad));
                        }
                        else if (usage == HUG_DPAD_RIGHT) {
                            dpad[2] = (Sint32) hid_get_data(REP_BUF_DATA(rep), &hitem);
                            SDL_PrivateJoystickHat(joy, 0, dpad_to_sdl(dpad));
                        }
                        else if (usage == HUG_DPAD_LEFT) {
                            dpad[3] = (Sint32) hid_get_data(REP_BUF_DATA(rep), &hitem);
                            SDL_PrivateJoystickHat(joy, 0, dpad_to_sdl(dpad));
                        }
#endif
                        break;
                    }
                case HUP_BUTTON:
                    v = (Sint32) hid_get_data(REP_BUF_DATA(rep), &hitem);
                    SDL_PrivateJoystickButton(joy, nbutton, v);
                    nbutton++;
                    break;
                default:
                    continue;
                }
                break;
            default:
                break;
            }
        }
        hid_end_parse(hdata);
    }
}

/* Function to close a joystick after use */
static void
BSD_JoystickClose(SDL_Joystick *joy)
{
    if (SDL_strncmp(joy->hwdata->path, "/dev/joy", 8)) {
        report_free(&joy->hwdata->inreport);
        hid_dispose_report_desc(joy->hwdata->repdesc);
    }
    close(joy->hwdata->fd);
    SDL_free(joy->hwdata->path);
    SDL_free(joy->hwdata);
}

static void
BSD_JoystickQuit(void)
{
    int i;

    for (i = 0; i < MAX_JOYS; i++) {
        SDL_free(joynames[i]);
        SDL_free(joydevnames[i]);
    }

    return;
}

static SDL_JoystickGUID
BSD_JoystickGetDeviceGUID( int device_index )
{
    SDL_JoystickGUID guid;
    /* the GUID is just the first 16 chars of the name for now */
    const char *name = BSD_JoystickGetDeviceName( device_index );
    SDL_zero( guid );
    SDL_memcpy( &guid, name, SDL_min( sizeof(guid), SDL_strlen( name ) ) );
    return guid;
}

static int
report_alloc(struct report *r, struct report_desc *rd, int repind)
{
    int len;

#ifdef __DragonFly__
    len = hid_report_size(rd, repinfo[repind].kind, r->rid);
#elif __FREEBSD__
# if (__FreeBSD_kernel_version >= 460000) || defined(__FreeBSD_kernel__)
#  if (__FreeBSD_kernel_version <= 500111)
    len = hid_report_size(rd, r->rid, repinfo[repind].kind);
#  else
    len = hid_report_size(rd, repinfo[repind].kind, r->rid);
#  endif
# else
    len = hid_report_size(rd, repinfo[repind].kind, &r->rid);
# endif
#else
# ifdef USBHID_NEW
    len = hid_report_size(rd, repinfo[repind].kind, r->rid);
# else
    len = hid_report_size(rd, repinfo[repind].kind, &r->rid);
# endif
#endif

    if (len < 0) {
        return SDL_SetError("Negative HID report size");
    }
    r->size = len;

    if (r->size > 0) {
#if defined(__FREEBSD__) && (__FreeBSD_kernel_version > 900000) || defined(__DragonFly__)
        r->buf = SDL_malloc(r->size);
#else
        r->buf = SDL_malloc(sizeof(*r->buf) - sizeof(REP_BUF_DATA(r)) +
                            r->size);
#endif
        if (r->buf == NULL) {
            return SDL_OutOfMemory();
        }
    } else {
        r->buf = NULL;
    }

    r->status = SREPORT_CLEAN;
    return 0;
}

static void
report_free(struct report *r)
{
    SDL_free(r->buf);
    r->status = SREPORT_UNINIT;
}

static int
BSD_JoystickRumble(SDL_Joystick *joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble)
{
    return SDL_Unsupported();
}

static int
BSD_JoystickRumbleTriggers(SDL_Joystick *joystick, Uint16 left_rumble, Uint16 right_rumble)
{
    return SDL_Unsupported();
}

static SDL_bool
BSD_JoystickGetGamepadMapping(int device_index, SDL_GamepadMapping *out)
{
    return SDL_FALSE;
}

static SDL_bool
BSD_JoystickHasLED(SDL_Joystick *joystick)
{
    return SDL_FALSE;
}

static int
BSD_JoystickSetLED(SDL_Joystick *joystick, Uint8 red, Uint8 green, Uint8 blue)
{
    return SDL_Unsupported();
}

static int
BSD_JoystickSendEffect(SDL_Joystick *joystick, const void *data, int size)
{
    return SDL_Unsupported();
}

static int
BSD_JoystickSetSensorsEnabled(SDL_Joystick *joystick, SDL_bool enabled)
{
    return SDL_Unsupported();
}

SDL_JoystickDriver SDL_BSD_JoystickDriver =
{
    BSD_JoystickInit,
    BSD_JoystickGetCount,
    BSD_JoystickDetect,
    BSD_JoystickGetDeviceName,
    BSD_JoystickGetDevicePlayerIndex,
    BSD_JoystickSetDevicePlayerIndex,
    BSD_JoystickGetDeviceGUID,
    BSD_JoystickGetDeviceInstanceID,
    BSD_JoystickOpen,
    BSD_JoystickRumble,
    BSD_JoystickRumbleTriggers,
    BSD_JoystickHasLED,
    BSD_JoystickSetLED,
    BSD_JoystickSendEffect,
    BSD_JoystickSetSensorsEnabled,
    BSD_JoystickUpdate,
    BSD_JoystickClose,
    BSD_JoystickQuit,
    BSD_JoystickGetGamepadMapping
};

#endif /* SDL_JOYSTICK_USBHID */

/* vi: set ts=4 sw=4 expandtab: */
