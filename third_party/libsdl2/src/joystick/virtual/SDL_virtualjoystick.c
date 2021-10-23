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

#if defined(SDL_JOYSTICK_VIRTUAL)

/* This is the virtual implementation of the SDL joystick API */

#include "SDL_virtualjoystick_c.h"
#include "../SDL_sysjoystick.h"
#include "../SDL_joystick_c.h"

extern SDL_JoystickDriver SDL_VIRTUAL_JoystickDriver;

static joystick_hwdata * g_VJoys = NULL;


static joystick_hwdata *
VIRTUAL_HWDataForIndex(int device_index)
{
    joystick_hwdata *vjoy = g_VJoys;
    while (vjoy) {
        if (device_index == 0)
            break;
        --device_index;
        vjoy = vjoy->next;
    }
    return vjoy;
}


static void
VIRTUAL_FreeHWData(joystick_hwdata *hwdata)
{
    joystick_hwdata * cur = g_VJoys;
    joystick_hwdata * prev = NULL;
 
    if (!hwdata) {
        return;
    }
    if (hwdata->axes) {
        SDL_free((void *)hwdata->axes);
        hwdata->axes = NULL;
    }
    if (hwdata->buttons) {
        SDL_free((void *)hwdata->buttons);
        hwdata->buttons = NULL;
    }
    if (hwdata->hats) {
        SDL_free(hwdata->hats);
        hwdata->hats = NULL;
    }

    /* Remove hwdata from SDL-global list */
    while (cur) {
        if (hwdata == cur) {
            if (prev) {
                prev->next = cur->next;
            } else {
                g_VJoys = cur->next;
            }
            break;
        }
        prev = cur;
        cur = cur->next;
    }

    SDL_free(hwdata);
}


int
SDL_JoystickAttachVirtualInner(SDL_JoystickType type,
                               int naxes,
                               int nbuttons,
                               int nhats)
{
    joystick_hwdata *hwdata = NULL;
    int device_index = -1;

    hwdata = SDL_calloc(1, sizeof(joystick_hwdata));
    if (!hwdata) {
        VIRTUAL_FreeHWData(hwdata);
        return SDL_OutOfMemory();
    }

    hwdata->naxes = naxes;
    hwdata->nbuttons = nbuttons;
    hwdata->nhats = nhats;
    hwdata->name = "Virtual Joystick";

    /* Note that this is a Virtual device and what subtype it is */
    hwdata->guid.data[14] = 'v';
    hwdata->guid.data[15] = (Uint8)type;

    /* Allocate fields for different control-types */
    if (naxes > 0) {
        hwdata->axes = SDL_calloc(naxes, sizeof(Sint16));
        if (!hwdata->axes) {
            VIRTUAL_FreeHWData(hwdata);
            return SDL_OutOfMemory();
        }
    }
    if (nbuttons > 0) {
        hwdata->buttons = SDL_calloc(nbuttons, sizeof(Uint8));
        if (!hwdata->buttons) {
            VIRTUAL_FreeHWData(hwdata);
            return SDL_OutOfMemory();
        }
    }
    if (nhats > 0) {
        hwdata->hats = SDL_calloc(nhats, sizeof(Uint8));
        if (!hwdata->hats) {
            VIRTUAL_FreeHWData(hwdata);
            return SDL_OutOfMemory();
        }
    }

    /* Allocate an instance ID for this device */
    hwdata->instance_id = SDL_GetNextJoystickInstanceID();

    /* Add virtual joystick to SDL-global lists */
    hwdata->next = g_VJoys;
    g_VJoys = hwdata;
    SDL_PrivateJoystickAdded(hwdata->instance_id);

    /* Return the new virtual-device's index */
    device_index = SDL_JoystickGetDeviceIndexFromInstanceID(hwdata->instance_id);
    return device_index;
}


int
SDL_JoystickDetachVirtualInner(int device_index)
{
    SDL_JoystickID instance_id;
    joystick_hwdata *hwdata = VIRTUAL_HWDataForIndex(device_index);
    if (!hwdata) {
        return SDL_SetError("Virtual joystick data not found");
    }
    instance_id = hwdata->instance_id;
    VIRTUAL_FreeHWData(hwdata);
    SDL_PrivateJoystickRemoved(instance_id);
    return 0;
}


int
SDL_JoystickSetVirtualAxisInner(SDL_Joystick *joystick, int axis, Sint16 value)
{
    joystick_hwdata *hwdata;

    SDL_LockJoysticks();

    if (!joystick || !joystick->hwdata) {
        SDL_UnlockJoysticks();
        return SDL_SetError("Invalid joystick");
    }

    hwdata = (joystick_hwdata *)joystick->hwdata;
    if (axis < 0 || axis >= hwdata->naxes) {
        SDL_UnlockJoysticks();
        return SDL_SetError("Invalid axis index");
    }

    hwdata->axes[axis] = value;

    SDL_UnlockJoysticks();
    return 0;
}


int
SDL_JoystickSetVirtualButtonInner(SDL_Joystick *joystick, int button, Uint8 value)
{
    joystick_hwdata *hwdata;

    SDL_LockJoysticks();

    if (!joystick || !joystick->hwdata) {
        SDL_UnlockJoysticks();
        return SDL_SetError("Invalid joystick");
    }

    hwdata = (joystick_hwdata *)joystick->hwdata;
    if (button < 0 || button >= hwdata->nbuttons) {
        SDL_UnlockJoysticks();
        return SDL_SetError("Invalid button index");
    }

    hwdata->buttons[button] = value;

    SDL_UnlockJoysticks();
    return 0;
}


int
SDL_JoystickSetVirtualHatInner(SDL_Joystick *joystick, int hat, Uint8 value)
{
    joystick_hwdata *hwdata;

    SDL_LockJoysticks();

    if (!joystick || !joystick->hwdata) {
        SDL_UnlockJoysticks();
        return SDL_SetError("Invalid joystick");
    }

    hwdata = (joystick_hwdata *)joystick->hwdata;
    if (hat < 0 || hat >= hwdata->nhats) {
        SDL_UnlockJoysticks();
        return SDL_SetError("Invalid hat index");
    }

    hwdata->hats[hat] = value;

    SDL_UnlockJoysticks();
    return 0;
}


static int
VIRTUAL_JoystickInit(void)
{
    return 0;
}


static int
VIRTUAL_JoystickGetCount(void)
{
    int count = 0;
    joystick_hwdata *cur = g_VJoys;
    while (cur) {
        ++count;
        cur = cur->next;
    }
    return count;
}


static void
VIRTUAL_JoystickDetect(void)
{
}


static const char *
VIRTUAL_JoystickGetDeviceName(int device_index)
{
    joystick_hwdata *hwdata = VIRTUAL_HWDataForIndex(device_index);
    if (!hwdata) {
        return NULL;
    }
    return hwdata->name ? hwdata->name : "";
}


static int
VIRTUAL_JoystickGetDevicePlayerIndex(int device_index)
{
    return -1;
}


static void
VIRTUAL_JoystickSetDevicePlayerIndex(int device_index, int player_index)
{
}


static SDL_JoystickGUID
VIRTUAL_JoystickGetDeviceGUID(int device_index)
{
    joystick_hwdata *hwdata = VIRTUAL_HWDataForIndex(device_index);
    if (!hwdata) {
        SDL_JoystickGUID guid;
        SDL_zero(guid);
        return guid;
    }
    return hwdata->guid;
}


static SDL_JoystickID
VIRTUAL_JoystickGetDeviceInstanceID(int device_index)
{
    joystick_hwdata *hwdata = VIRTUAL_HWDataForIndex(device_index);
    if (!hwdata) {
        return -1;
    }
    return hwdata->instance_id;
}


static int
VIRTUAL_JoystickOpen(SDL_Joystick *joystick, int device_index)
{
    joystick_hwdata *hwdata = VIRTUAL_HWDataForIndex(device_index);
    if (!hwdata) {
        return SDL_SetError("No such device");
    }
    if (hwdata->opened) {
        return SDL_SetError("Joystick already opened");
    }
    joystick->instance_id = hwdata->instance_id;
    joystick->hwdata = hwdata;
    joystick->naxes = hwdata->naxes;
    joystick->nbuttons = hwdata->nbuttons;
    joystick->nhats = hwdata->nhats;
    hwdata->opened = SDL_TRUE;
    return 0;
}


static int
VIRTUAL_JoystickRumble(SDL_Joystick *joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble)
{
    return SDL_Unsupported();
}

static int
VIRTUAL_JoystickRumbleTriggers(SDL_Joystick *joystick, Uint16 left_rumble, Uint16 right_rumble)
{
    return SDL_Unsupported();
}


static SDL_bool
VIRTUAL_JoystickHasLED(SDL_Joystick *joystick)
{
    return SDL_FALSE;
}


static int
VIRTUAL_JoystickSetLED(SDL_Joystick *joystick, Uint8 red, Uint8 green, Uint8 blue)
{
    return SDL_Unsupported();
}

static int
VIRTUAL_JoystickSendEffect(SDL_Joystick *joystick, const void *data, int size)
{
    return SDL_Unsupported();
}

static int
VIRTUAL_JoystickSetSensorsEnabled(SDL_Joystick *joystick, SDL_bool enabled)
{
    return SDL_Unsupported();
}


static void
VIRTUAL_JoystickUpdate(SDL_Joystick *joystick)
{
    joystick_hwdata *hwdata;
    int i;

    if (!joystick) {
        return;
    }
    if (!joystick->hwdata) {
        return;
    }

    hwdata = (joystick_hwdata *)joystick->hwdata;

    for (i = 0; i < hwdata->naxes; ++i) {
        SDL_PrivateJoystickAxis(joystick, i, hwdata->axes[i]);
    }
    for (i = 0; i < hwdata->nbuttons; ++i) {
        SDL_PrivateJoystickButton(joystick, i, hwdata->buttons[i]);
    }
    for (i = 0; i < hwdata->nhats; ++i) {
        SDL_PrivateJoystickHat(joystick, i, hwdata->hats[i]);
    }
}


static void
VIRTUAL_JoystickClose(SDL_Joystick *joystick)
{
    joystick_hwdata *hwdata;

    if (!joystick) {
        return;
    }
    if (!joystick->hwdata) {
        return;
    }

    hwdata = (joystick_hwdata *)joystick->hwdata;
    hwdata->opened = SDL_FALSE;
}


static void
VIRTUAL_JoystickQuit(void)
{
    while (g_VJoys) {
        VIRTUAL_FreeHWData(g_VJoys);
    }
}

static SDL_bool
VIRTUAL_JoystickGetGamepadMapping(int device_index, SDL_GamepadMapping *out)
{
    return SDL_FALSE;
}

SDL_JoystickDriver SDL_VIRTUAL_JoystickDriver =
{
    VIRTUAL_JoystickInit,
    VIRTUAL_JoystickGetCount,
    VIRTUAL_JoystickDetect,
    VIRTUAL_JoystickGetDeviceName,
    VIRTUAL_JoystickGetDevicePlayerIndex,
    VIRTUAL_JoystickSetDevicePlayerIndex,
    VIRTUAL_JoystickGetDeviceGUID,
    VIRTUAL_JoystickGetDeviceInstanceID,
    VIRTUAL_JoystickOpen,
    VIRTUAL_JoystickRumble,
    VIRTUAL_JoystickRumbleTriggers,
    VIRTUAL_JoystickHasLED,
    VIRTUAL_JoystickSetLED,
    VIRTUAL_JoystickSendEffect,
    VIRTUAL_JoystickSetSensorsEnabled,
    VIRTUAL_JoystickUpdate,
    VIRTUAL_JoystickClose,
    VIRTUAL_JoystickQuit,
    VIRTUAL_JoystickGetGamepadMapping
};

#endif /* SDL_JOYSTICK_VIRTUAL */

/* vi: set ts=4 sw=4 expandtab: */
