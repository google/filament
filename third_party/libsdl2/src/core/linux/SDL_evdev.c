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

#ifdef SDL_INPUT_LINUXEV

/* This is based on the linux joystick driver */
/* References: https://www.kernel.org/doc/Documentation/input/input.txt 
 *             https://www.kernel.org/doc/Documentation/input/event-codes.txt
 *             /usr/include/linux/input.h
 *             The evtest application is also useful to debug the protocol
 */

#include "SDL_evdev.h"
#include "SDL_evdev_kbd.h"

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/input.h>

#include "SDL.h"
#include "SDL_assert.h"
#include "SDL_endian.h"
#include "SDL_scancode.h"
#include "../../events/SDL_events_c.h"
#include "../../events/scancodes_linux.h" /* adds linux_scancode_table */
#include "../../core/linux/SDL_udev.h"

/* These are not defined in older Linux kernel headers */
#ifndef SYN_DROPPED
#define SYN_DROPPED 3
#endif
#ifndef ABS_MT_SLOT
#define ABS_MT_SLOT         0x2f
#define ABS_MT_POSITION_X   0x35
#define ABS_MT_POSITION_Y   0x36
#define ABS_MT_TRACKING_ID  0x39
#endif

typedef struct SDL_evdevlist_item
{
    char *path;
    int fd;

    /* TODO: use this for every device, not just touchscreen */
    int out_of_sync;

    /* TODO: expand on this to have data for every possible class (mouse,
       keyboard, touchpad, etc.). Also there's probably some things in here we
       can pull out to the SDL_evdevlist_item i.e. name */
    int is_touchscreen;
    struct {
        char* name;

        int min_x, max_x, range_x;
        int min_y, max_y, range_y;

        int max_slots;
        int current_slot;
        struct {
            enum {
                EVDEV_TOUCH_SLOTDELTA_NONE = 0,
                EVDEV_TOUCH_SLOTDELTA_DOWN,
                EVDEV_TOUCH_SLOTDELTA_UP,
                EVDEV_TOUCH_SLOTDELTA_MOVE
            } delta;
            int tracking_id;
            int x, y;
        } * slots;
    } * touchscreen_data;

    struct SDL_evdevlist_item *next;
} SDL_evdevlist_item;

typedef struct SDL_EVDEV_PrivateData
{
    int ref_count;
    int num_devices;
    SDL_evdevlist_item *first;
    SDL_evdevlist_item *last;
    SDL_EVDEV_keyboard_state *kbd;
} SDL_EVDEV_PrivateData;

#define _THIS SDL_EVDEV_PrivateData *_this
static _THIS = NULL;

static SDL_Scancode SDL_EVDEV_translate_keycode(int keycode);
static void SDL_EVDEV_sync_device(SDL_evdevlist_item *item);
static int SDL_EVDEV_device_removed(const char *dev_path);

#if SDL_USE_LIBUDEV
static int SDL_EVDEV_device_added(const char *dev_path, int udev_class);
static void SDL_EVDEV_udev_callback(SDL_UDEV_deviceevent udev_type, int udev_class,
    const char *dev_path);
#endif /* SDL_USE_LIBUDEV */

static Uint8 EVDEV_MouseButtons[] = {
    SDL_BUTTON_LEFT,            /*  BTN_LEFT        0x110 */
    SDL_BUTTON_RIGHT,           /*  BTN_RIGHT       0x111 */
    SDL_BUTTON_MIDDLE,          /*  BTN_MIDDLE      0x112 */
    SDL_BUTTON_X1,              /*  BTN_SIDE        0x113 */
    SDL_BUTTON_X2,              /*  BTN_EXTRA       0x114 */
    SDL_BUTTON_X2 + 1,          /*  BTN_FORWARD     0x115 */
    SDL_BUTTON_X2 + 2,          /*  BTN_BACK        0x116 */
    SDL_BUTTON_X2 + 3           /*  BTN_TASK        0x117 */
};

int
SDL_EVDEV_Init(void)
{
    if (_this == NULL) {
        _this = (SDL_EVDEV_PrivateData*)SDL_calloc(1, sizeof(*_this));
        if (_this == NULL) {
            return SDL_OutOfMemory();
        }

#if SDL_USE_LIBUDEV
        if (SDL_UDEV_Init() < 0) {
            SDL_free(_this);
            _this = NULL;
            return -1;
        }

        /* Set up the udev callback */
        if (SDL_UDEV_AddCallback(SDL_EVDEV_udev_callback) < 0) {
            SDL_UDEV_Quit();
            SDL_free(_this);
            _this = NULL;
            return -1;
        }

        /* Force a scan to build the initial device list */
        SDL_UDEV_Scan();
#else
        /* TODO: Scan the devices manually, like a caveman */
#endif /* SDL_USE_LIBUDEV */

        _this->kbd = SDL_EVDEV_kbd_init();
    }

    _this->ref_count += 1;

    return 0;
}

void
SDL_EVDEV_Quit(void)
{
    if (_this == NULL) {
        return;
    }

    _this->ref_count -= 1;

    if (_this->ref_count < 1) {
#if SDL_USE_LIBUDEV
        SDL_UDEV_DelCallback(SDL_EVDEV_udev_callback);
        SDL_UDEV_Quit();
#endif /* SDL_USE_LIBUDEV */

        SDL_EVDEV_kbd_quit(_this->kbd);

        /* Remove existing devices */
        while(_this->first != NULL) {
            SDL_EVDEV_device_removed(_this->first->path);
        }

        SDL_assert(_this->first == NULL);
        SDL_assert(_this->last == NULL);
        SDL_assert(_this->num_devices == 0);

        SDL_free(_this);
        _this = NULL;
    }
}

#if SDL_USE_LIBUDEV
static void SDL_EVDEV_udev_callback(SDL_UDEV_deviceevent udev_event, int udev_class,
    const char* dev_path)
{
    if (dev_path == NULL) {
        return;
    }

    switch(udev_event) {
    case SDL_UDEV_DEVICEADDED:
        if (!(udev_class & (SDL_UDEV_DEVICE_MOUSE | SDL_UDEV_DEVICE_KEYBOARD |
            SDL_UDEV_DEVICE_TOUCHSCREEN)))
            return;

        SDL_EVDEV_device_added(dev_path, udev_class);
        break;  
    case SDL_UDEV_DEVICEREMOVED:
        SDL_EVDEV_device_removed(dev_path);
        break;
    default:
        break;
    }
}
#endif /* SDL_USE_LIBUDEV */

void 
SDL_EVDEV_Poll(void)
{
    struct input_event events[32];
    int i, j, len;
    SDL_evdevlist_item *item;
    SDL_Scancode scan_code;
    int mouse_button;
    SDL_Mouse *mouse;
    float norm_x, norm_y;

    if (!_this) {
        return;
    }

#if SDL_USE_LIBUDEV
    SDL_UDEV_Poll();
#endif

    mouse = SDL_GetMouse();

    for (item = _this->first; item != NULL; item = item->next) {
        while ((len = read(item->fd, events, (sizeof events))) > 0) {
            len /= sizeof(events[0]);
            for (i = 0; i < len; ++i) {
                /* special handling for touchscreen, that should eventually be
                   used for all devices */
                if (item->out_of_sync && item->is_touchscreen &&
                    events[i].type == EV_SYN && events[i].code != SYN_REPORT) {
                    break;
                }

                switch (events[i].type) {
                case EV_KEY:
                    if (events[i].code >= BTN_MOUSE && events[i].code < BTN_MOUSE + SDL_arraysize(EVDEV_MouseButtons)) {
                        mouse_button = events[i].code - BTN_MOUSE;
                        if (events[i].value == 0) {
                            SDL_SendMouseButton(mouse->focus, mouse->mouseID, SDL_RELEASED, EVDEV_MouseButtons[mouse_button]);
                        } else if (events[i].value == 1) {
                            SDL_SendMouseButton(mouse->focus, mouse->mouseID, SDL_PRESSED, EVDEV_MouseButtons[mouse_button]);
                        }
                        break;
                    }

                    /* Probably keyboard */
                    scan_code = SDL_EVDEV_translate_keycode(events[i].code);
                    if (scan_code != SDL_SCANCODE_UNKNOWN) {
                        if (events[i].value == 0) {
                            SDL_SendKeyboardKey(SDL_RELEASED, scan_code);
                        } else if (events[i].value == 1 || events[i].value == 2 /* key repeated */) {
                            SDL_SendKeyboardKey(SDL_PRESSED, scan_code);
                        }
                    }
                    SDL_EVDEV_kbd_keycode(_this->kbd, events[i].code, events[i].value);
                    break;
                case EV_ABS:
                    switch(events[i].code) {
                    case ABS_MT_SLOT:
                        if (!item->is_touchscreen) /* FIXME: temp hack */
                            break;
                        item->touchscreen_data->current_slot = events[i].value;
                        break;
                    case ABS_MT_TRACKING_ID:
                        if (!item->is_touchscreen) /* FIXME: temp hack */
                            break;
                        if (events[i].value >= 0) {
                            item->touchscreen_data->slots[item->touchscreen_data->current_slot].tracking_id = events[i].value;
                            item->touchscreen_data->slots[item->touchscreen_data->current_slot].delta = EVDEV_TOUCH_SLOTDELTA_DOWN;
                        } else {
                            item->touchscreen_data->slots[item->touchscreen_data->current_slot].delta = EVDEV_TOUCH_SLOTDELTA_UP;
                        }
                        break;
                    case ABS_MT_POSITION_X:
                        if (!item->is_touchscreen) /* FIXME: temp hack */
                            break;
                        item->touchscreen_data->slots[item->touchscreen_data->current_slot].x = events[i].value;
                        if (item->touchscreen_data->slots[item->touchscreen_data->current_slot].delta == EVDEV_TOUCH_SLOTDELTA_NONE) {
                            item->touchscreen_data->slots[item->touchscreen_data->current_slot].delta = EVDEV_TOUCH_SLOTDELTA_MOVE;
                        }
                        break;
                    case ABS_MT_POSITION_Y:
                        if (!item->is_touchscreen) /* FIXME: temp hack */
                            break;
                        item->touchscreen_data->slots[item->touchscreen_data->current_slot].y = events[i].value;
                        if (item->touchscreen_data->slots[item->touchscreen_data->current_slot].delta == EVDEV_TOUCH_SLOTDELTA_NONE) {
                            item->touchscreen_data->slots[item->touchscreen_data->current_slot].delta = EVDEV_TOUCH_SLOTDELTA_MOVE;
                        }
                        break;
                    case ABS_X:
                        if (item->is_touchscreen) /* FIXME: temp hack */
                            break;
                        SDL_SendMouseMotion(mouse->focus, mouse->mouseID, SDL_FALSE, events[i].value, mouse->y);
                        break;
                    case ABS_Y:
                        if (item->is_touchscreen) /* FIXME: temp hack */
                            break;
                        SDL_SendMouseMotion(mouse->focus, mouse->mouseID, SDL_FALSE, mouse->x, events[i].value);
                        break;
                    default:
                        break;
                    }
                    break;
                case EV_REL:
                    switch(events[i].code) {
                    case REL_X:
                        SDL_SendMouseMotion(mouse->focus, mouse->mouseID, SDL_TRUE, events[i].value, 0);
                        break;
                    case REL_Y:
                        SDL_SendMouseMotion(mouse->focus, mouse->mouseID, SDL_TRUE, 0, events[i].value);
                        break;
                    case REL_WHEEL:
                        SDL_SendMouseWheel(mouse->focus, mouse->mouseID, 0, events[i].value, SDL_MOUSEWHEEL_NORMAL);
                        break;
                    case REL_HWHEEL:
                        SDL_SendMouseWheel(mouse->focus, mouse->mouseID, events[i].value, 0, SDL_MOUSEWHEEL_NORMAL);
                        break;
                    default:
                        break;
                    }
                    break;
                case EV_SYN:
                    switch (events[i].code) {
                    case SYN_REPORT:
                        if (!item->is_touchscreen) /* FIXME: temp hack */
                            break;

                        for(j = 0; j < item->touchscreen_data->max_slots; j++) {
                            norm_x = (float)(item->touchscreen_data->slots[j].x - item->touchscreen_data->min_x) /
                                (float)item->touchscreen_data->range_x;
                            norm_y = (float)(item->touchscreen_data->slots[j].y - item->touchscreen_data->min_y) /
                                (float)item->touchscreen_data->range_y;

                            switch(item->touchscreen_data->slots[j].delta) {
                            case EVDEV_TOUCH_SLOTDELTA_DOWN:
                                SDL_SendTouch(item->fd, item->touchscreen_data->slots[j].tracking_id, SDL_TRUE, norm_x, norm_y, 1.0f);
                                item->touchscreen_data->slots[j].delta = EVDEV_TOUCH_SLOTDELTA_NONE;
                                break;
                            case EVDEV_TOUCH_SLOTDELTA_UP:
                                SDL_SendTouch(item->fd, item->touchscreen_data->slots[j].tracking_id, SDL_FALSE, norm_x, norm_y, 1.0f);
                                item->touchscreen_data->slots[j].tracking_id = -1;
                                item->touchscreen_data->slots[j].delta = EVDEV_TOUCH_SLOTDELTA_NONE;
                                break;
                            case EVDEV_TOUCH_SLOTDELTA_MOVE:
                                SDL_SendTouchMotion(item->fd, item->touchscreen_data->slots[j].tracking_id, norm_x, norm_y, 1.0f);
                                item->touchscreen_data->slots[j].delta = EVDEV_TOUCH_SLOTDELTA_NONE;
                                break;
                            default:
                                break;
                            }
                        }

                        if (item->out_of_sync)
                            item->out_of_sync = 0;
                        break;
                    case SYN_DROPPED:
                        if (item->is_touchscreen)
                            item->out_of_sync = 1;
                        SDL_EVDEV_sync_device(item);
                        break;
                    default:
                        break;
                    }
                    break;
                }
            }
        }    
    }
}

static SDL_Scancode
SDL_EVDEV_translate_keycode(int keycode)
{
    SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;

    if (keycode < SDL_arraysize(linux_scancode_table))
        scancode = linux_scancode_table[keycode];

    if (scancode == SDL_SCANCODE_UNKNOWN) {
        SDL_Log("The key you just pressed is not recognized by SDL. To help "
            "get this fixed, please report this to the SDL forums/mailing list "
            "<https://discourse.libsdl.org/> EVDEV KeyCode %d", keycode);
    }

    return scancode;
}

#ifdef SDL_USE_LIBUDEV
static int
SDL_EVDEV_init_touchscreen(SDL_evdevlist_item* item)
{
    int ret, i;
    char name[64];
    struct input_absinfo abs_info;

    if (!item->is_touchscreen)
        return 0;

    item->touchscreen_data = SDL_calloc(1, sizeof(*item->touchscreen_data));
    if (item->touchscreen_data == NULL)
        return SDL_OutOfMemory();

    ret = ioctl(item->fd, EVIOCGNAME(sizeof(name)), name);
    if (ret < 0) {
        SDL_free(item->touchscreen_data);
        return SDL_SetError("Failed to get evdev touchscreen name");
    }

    item->touchscreen_data->name = SDL_strdup(name);
    if (item->touchscreen_data->name == NULL) {
        SDL_free(item->touchscreen_data);
        return SDL_OutOfMemory();
    }

    ret = ioctl(item->fd, EVIOCGABS(ABS_MT_POSITION_X), &abs_info);
    if (ret < 0) {
        SDL_free(item->touchscreen_data->name);
        SDL_free(item->touchscreen_data);
        return SDL_SetError("Failed to get evdev touchscreen limits");
    }
    item->touchscreen_data->min_x = abs_info.minimum;
    item->touchscreen_data->max_x = abs_info.maximum;
    item->touchscreen_data->range_x = abs_info.maximum - abs_info.minimum;

    ret = ioctl(item->fd, EVIOCGABS(ABS_MT_POSITION_Y), &abs_info);
    if (ret < 0) {
        SDL_free(item->touchscreen_data->name);
        SDL_free(item->touchscreen_data);
        return SDL_SetError("Failed to get evdev touchscreen limits");
    }
    item->touchscreen_data->min_y = abs_info.minimum;
    item->touchscreen_data->max_y = abs_info.maximum;
    item->touchscreen_data->range_y = abs_info.maximum - abs_info.minimum;

    ret = ioctl(item->fd, EVIOCGABS(ABS_MT_SLOT), &abs_info);
    if (ret < 0) {
        SDL_free(item->touchscreen_data->name);
        SDL_free(item->touchscreen_data);
        return SDL_SetError("Failed to get evdev touchscreen limits");
    }
    item->touchscreen_data->max_slots = abs_info.maximum + 1;

    item->touchscreen_data->slots = SDL_calloc(
        item->touchscreen_data->max_slots,
        sizeof(*item->touchscreen_data->slots));
    if (item->touchscreen_data->slots == NULL) {
        SDL_free(item->touchscreen_data->name);
        SDL_free(item->touchscreen_data);
        return SDL_OutOfMemory();
    }

    for(i = 0; i < item->touchscreen_data->max_slots; i++) {
        item->touchscreen_data->slots[i].tracking_id = -1;
    }

    ret = SDL_AddTouch(item->fd, /* I guess our fd is unique enough */
        item->touchscreen_data->name);
    if (ret < 0) {
        SDL_free(item->touchscreen_data->slots);
        SDL_free(item->touchscreen_data->name);
        SDL_free(item->touchscreen_data);
        return ret;
    }

    return 0;
}
#endif /* SDL_USE_LIBUDEV */

static void
SDL_EVDEV_destroy_touchscreen(SDL_evdevlist_item* item) {
    if (!item->is_touchscreen)
        return;

    SDL_DelTouch(item->fd);
    SDL_free(item->touchscreen_data->slots);
    SDL_free(item->touchscreen_data->name);
    SDL_free(item->touchscreen_data);
}

static void
SDL_EVDEV_sync_device(SDL_evdevlist_item *item) 
{
#ifdef EVIOCGMTSLOTS
    int i, ret;
    struct input_absinfo abs_info;
    /*
     * struct input_mt_request_layout {
     *     __u32 code;
     *     __s32 values[num_slots];
     * };
     *
     * this is the structure we're trying to emulate
     */
    __u32* mt_req_code;
    __s32* mt_req_values;
    size_t mt_req_size;

    /* TODO: sync devices other than touchscreen */
    if (!item->is_touchscreen)
        return;

    mt_req_size = sizeof(*mt_req_code) +
        sizeof(*mt_req_values) * item->touchscreen_data->max_slots;

    mt_req_code = SDL_calloc(1, mt_req_size);
    if (mt_req_code == NULL) {
        return;
    }

    mt_req_values = (__s32*)mt_req_code + 1;

    *mt_req_code = ABS_MT_TRACKING_ID;
    ret = ioctl(item->fd, EVIOCGMTSLOTS(mt_req_size), mt_req_code);
    if (ret < 0) {
        SDL_free(mt_req_code);
        return;
    }
    for(i = 0; i < item->touchscreen_data->max_slots; i++) {
        /*
         * This doesn't account for the very edge case of the user removing their
         * finger and replacing it on the screen during the time we're out of sync,
         * which'll mean that we're not going from down -> up or up -> down, we're
         * going from down -> down but with a different tracking id, meaning we'd
         * have to tell SDL of the two events, but since we wait till SYN_REPORT in
         * SDL_EVDEV_Poll to tell SDL, the current structure of this code doesn't
         * allow it. Lets just pray to God it doesn't happen.
         */
        if (item->touchscreen_data->slots[i].tracking_id < 0 &&
            mt_req_values[i] >= 0) {
            item->touchscreen_data->slots[i].tracking_id = mt_req_values[i];
            item->touchscreen_data->slots[i].delta = EVDEV_TOUCH_SLOTDELTA_DOWN;
        } else if (item->touchscreen_data->slots[i].tracking_id >= 0 &&
            mt_req_values[i] < 0) {
            item->touchscreen_data->slots[i].tracking_id = -1;
            item->touchscreen_data->slots[i].delta = EVDEV_TOUCH_SLOTDELTA_UP;
        }
    }

    *mt_req_code = ABS_MT_POSITION_X;
    ret = ioctl(item->fd, EVIOCGMTSLOTS(mt_req_size), mt_req_code);
    if (ret < 0) {
        SDL_free(mt_req_code);
        return;
    }
    for(i = 0; i < item->touchscreen_data->max_slots; i++) {
        if (item->touchscreen_data->slots[i].tracking_id >= 0 &&
            item->touchscreen_data->slots[i].x != mt_req_values[i]) {
            item->touchscreen_data->slots[i].x = mt_req_values[i];
            if (item->touchscreen_data->slots[i].delta ==
                EVDEV_TOUCH_SLOTDELTA_NONE) {
                item->touchscreen_data->slots[i].delta =
                    EVDEV_TOUCH_SLOTDELTA_MOVE;
            }
        }
    }

    *mt_req_code = ABS_MT_POSITION_Y;
    ret = ioctl(item->fd, EVIOCGMTSLOTS(mt_req_size), mt_req_code);
    if (ret < 0) {
        SDL_free(mt_req_code);
        return;
    }
    for(i = 0; i < item->touchscreen_data->max_slots; i++) {
        if (item->touchscreen_data->slots[i].tracking_id >= 0 &&
            item->touchscreen_data->slots[i].y != mt_req_values[i]) {
            item->touchscreen_data->slots[i].y = mt_req_values[i];
            if (item->touchscreen_data->slots[i].delta ==
                EVDEV_TOUCH_SLOTDELTA_NONE) {
                item->touchscreen_data->slots[i].delta =
                    EVDEV_TOUCH_SLOTDELTA_MOVE;
            }
        }
    }

    ret = ioctl(item->fd, EVIOCGABS(ABS_MT_SLOT), &abs_info);
    if (ret < 0) {
        SDL_free(mt_req_code);
        return;
    }
    item->touchscreen_data->current_slot = abs_info.value;

    SDL_free(mt_req_code);

#endif /* EVIOCGMTSLOTS */
}

#if SDL_USE_LIBUDEV
static int
SDL_EVDEV_device_added(const char *dev_path, int udev_class)
{
    int ret;
    SDL_evdevlist_item *item;

    /* Check to make sure it's not already in list. */
    for (item = _this->first; item != NULL; item = item->next) {
        if (SDL_strcmp(dev_path, item->path) == 0) {
            return -1;  /* already have this one */
        }
    }

    item = (SDL_evdevlist_item *) SDL_calloc(1, sizeof (SDL_evdevlist_item));
    if (item == NULL) {
        return SDL_OutOfMemory();
    }

    item->fd = open(dev_path, O_RDONLY | O_NONBLOCK);
    if (item->fd < 0) {
        SDL_free(item);
        return SDL_SetError("Unable to open %s", dev_path);
    }

    item->path = SDL_strdup(dev_path);
    if (item->path == NULL) {
        close(item->fd);
        SDL_free(item);
        return SDL_OutOfMemory();
    }

    if (udev_class & SDL_UDEV_DEVICE_TOUCHSCREEN) {
        item->is_touchscreen = 1;

        if ((ret = SDL_EVDEV_init_touchscreen(item)) < 0) {
            close(item->fd);
            SDL_free(item);
            return ret;
        }
    }

    if (_this->last == NULL) {
        _this->first = _this->last = item;
    } else {
        _this->last->next = item;
        _this->last = item;
    }

    SDL_EVDEV_sync_device(item);

    return _this->num_devices++;
}
#endif /* SDL_USE_LIBUDEV */

static int
SDL_EVDEV_device_removed(const char *dev_path)
{
    SDL_evdevlist_item *item;
    SDL_evdevlist_item *prev = NULL;

    for (item = _this->first; item != NULL; item = item->next) {
        /* found it, remove it. */
        if (SDL_strcmp(dev_path, item->path) == 0) {
            if (prev != NULL) {
                prev->next = item->next;
            } else {
                SDL_assert(_this->first == item);
                _this->first = item->next;
            }
            if (item == _this->last) {
                _this->last = prev;
            }
            if (item->is_touchscreen) {
                SDL_EVDEV_destroy_touchscreen(item);
            }
            close(item->fd);
            SDL_free(item->path);
            SDL_free(item);
            _this->num_devices--;
            return 0;
        }
        prev = item;
    }

    return -1;
}


#endif /* SDL_INPUT_LINUXEV */

/* vi: set ts=4 sw=4 expandtab: */
