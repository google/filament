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

#ifdef SDL_JOYSTICK_ANDROID

#include <stdio.h>              /* For the definition of NULL */
#include "SDL_error.h"
#include "SDL_events.h"

#include "SDL_joystick.h"
#include "SDL_hints.h"
#include "SDL_assert.h"
#include "SDL_timer.h"
#include "SDL_log.h"
#include "SDL_sysjoystick_c.h"
#include "../SDL_joystick_c.h"
#include "../../events/SDL_keyboard_c.h"
#include "../../core/android/SDL_android.h"
#include "../steam/SDL_steamcontroller.h"

#include "android/keycodes.h"

/* As of platform android-14, android/keycodes.h is missing these defines */
#ifndef AKEYCODE_BUTTON_1
#define AKEYCODE_BUTTON_1 188
#define AKEYCODE_BUTTON_2 189
#define AKEYCODE_BUTTON_3 190
#define AKEYCODE_BUTTON_4 191
#define AKEYCODE_BUTTON_5 192
#define AKEYCODE_BUTTON_6 193
#define AKEYCODE_BUTTON_7 194
#define AKEYCODE_BUTTON_8 195
#define AKEYCODE_BUTTON_9 196
#define AKEYCODE_BUTTON_10 197
#define AKEYCODE_BUTTON_11 198
#define AKEYCODE_BUTTON_12 199
#define AKEYCODE_BUTTON_13 200
#define AKEYCODE_BUTTON_14 201
#define AKEYCODE_BUTTON_15 202
#define AKEYCODE_BUTTON_16 203
#endif

#define ANDROID_ACCELEROMETER_NAME "Android Accelerometer"
#define ANDROID_ACCELEROMETER_DEVICE_ID INT_MIN
#define ANDROID_MAX_NBUTTONS 36

static SDL_joylist_item * JoystickByDeviceId(int device_id);

static SDL_joylist_item *SDL_joylist = NULL;
static SDL_joylist_item *SDL_joylist_tail = NULL;
static int numjoysticks = 0;
static int instance_counter = 0;


/* Function to convert Android keyCodes into SDL ones.
 * This code manipulation is done to get a sequential list of codes.
 * FIXME: This is only suited for the case where we use a fixed number of buttons determined by ANDROID_MAX_NBUTTONS
 */
static int
keycode_to_SDL(int keycode)
{
    /* FIXME: If this function gets too unwieldy in the future, replace with a lookup table */
    int button = 0;
    switch (keycode) {
        /* Some gamepad buttons (API 9) */
        case AKEYCODE_BUTTON_A:
            button = SDL_CONTROLLER_BUTTON_A;
            break;
        case AKEYCODE_BUTTON_B:
            button = SDL_CONTROLLER_BUTTON_B;
            break;
        case AKEYCODE_BUTTON_X:
            button = SDL_CONTROLLER_BUTTON_X;
            break;
        case AKEYCODE_BUTTON_Y:
            button = SDL_CONTROLLER_BUTTON_Y;
            break;
        case AKEYCODE_BUTTON_L1:
            button = SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
            break;
        case AKEYCODE_BUTTON_R1:
            button = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
            break;
        case AKEYCODE_BUTTON_THUMBL:
            button = SDL_CONTROLLER_BUTTON_LEFTSTICK;
            break;
        case AKEYCODE_BUTTON_THUMBR:
            button = SDL_CONTROLLER_BUTTON_RIGHTSTICK;
            break;
        case AKEYCODE_BUTTON_START:
            button = SDL_CONTROLLER_BUTTON_START;
            break;
        case AKEYCODE_BACK:
        case AKEYCODE_BUTTON_SELECT:
            button = SDL_CONTROLLER_BUTTON_BACK;
            break;
        case AKEYCODE_BUTTON_MODE:
            button = SDL_CONTROLLER_BUTTON_GUIDE;
            break;
        case AKEYCODE_BUTTON_L2:
            button = SDL_CONTROLLER_BUTTON_MAX; /* Not supported by GameController */
            break;
        case AKEYCODE_BUTTON_R2:
            button = SDL_CONTROLLER_BUTTON_MAX+1; /* Not supported by GameController */
            break;
        case AKEYCODE_BUTTON_C:
            button = SDL_CONTROLLER_BUTTON_MAX+2; /* Not supported by GameController */
            break;
        case AKEYCODE_BUTTON_Z:
            button = SDL_CONTROLLER_BUTTON_MAX+3; /* Not supported by GameController */
            break;
                        
        /* D-Pad key codes (API 1) */
        case AKEYCODE_DPAD_UP:
            button = SDL_CONTROLLER_BUTTON_DPAD_UP;
            break;
        case AKEYCODE_DPAD_DOWN:
            button = SDL_CONTROLLER_BUTTON_DPAD_DOWN;
            break;
        case AKEYCODE_DPAD_LEFT:
            button = SDL_CONTROLLER_BUTTON_DPAD_LEFT;
            break;
        case AKEYCODE_DPAD_RIGHT:
            button = SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
            break;
        case AKEYCODE_DPAD_CENTER:
            /* This is handled better by applications as the A button */
            /*button = SDL_CONTROLLER_BUTTON_MAX+4;*/ /* Not supported by GameController */
            button = SDL_CONTROLLER_BUTTON_A;
            break;

        /* More gamepad buttons (API 12), these get mapped to 20...35*/
        case AKEYCODE_BUTTON_1:
        case AKEYCODE_BUTTON_2:
        case AKEYCODE_BUTTON_3:
        case AKEYCODE_BUTTON_4:
        case AKEYCODE_BUTTON_5:
        case AKEYCODE_BUTTON_6:
        case AKEYCODE_BUTTON_7:
        case AKEYCODE_BUTTON_8:
        case AKEYCODE_BUTTON_9:
        case AKEYCODE_BUTTON_10:
        case AKEYCODE_BUTTON_11:
        case AKEYCODE_BUTTON_12:
        case AKEYCODE_BUTTON_13:
        case AKEYCODE_BUTTON_14:
        case AKEYCODE_BUTTON_15:
        case AKEYCODE_BUTTON_16:
            button = keycode - AKEYCODE_BUTTON_1 + SDL_CONTROLLER_BUTTON_MAX + 5;
            break;
            
        default:
            return -1;
            /* break; -Wunreachable-code-break */
    }
    
    /* This is here in case future generations, probably with six fingers per hand, 
     * happily add new cases up above and forget to update the max number of buttons. 
     */
    SDL_assert(button < ANDROID_MAX_NBUTTONS);
    return button;
}

static SDL_Scancode
button_to_scancode(int button)
{
    switch (button) {
    case SDL_CONTROLLER_BUTTON_A:
        return SDL_SCANCODE_RETURN;
    case SDL_CONTROLLER_BUTTON_B:
        return SDL_SCANCODE_ESCAPE;
    case SDL_CONTROLLER_BUTTON_BACK:
        return SDL_SCANCODE_ESCAPE;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
        return SDL_SCANCODE_UP;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        return SDL_SCANCODE_DOWN;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        return SDL_SCANCODE_LEFT;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        return SDL_SCANCODE_RIGHT;
    }

    /* Unsupported button */
    return SDL_SCANCODE_UNKNOWN;
}

int
Android_OnPadDown(int device_id, int keycode)
{
    SDL_joylist_item *item;
    int button = keycode_to_SDL(keycode);
    if (button >= 0) {
        item = JoystickByDeviceId(device_id);
        if (item && item->joystick) {
            SDL_PrivateJoystickButton(item->joystick, button , SDL_PRESSED);
        } else {
            SDL_SendKeyboardKey(SDL_PRESSED, button_to_scancode(button));
        }
        return 0;
    }
    
    return -1;
}

int
Android_OnPadUp(int device_id, int keycode)
{
    SDL_joylist_item *item;
    int button = keycode_to_SDL(keycode);
    if (button >= 0) {
        item = JoystickByDeviceId(device_id);
        if (item && item->joystick) {
            SDL_PrivateJoystickButton(item->joystick, button, SDL_RELEASED);
        } else {
            SDL_SendKeyboardKey(SDL_RELEASED, button_to_scancode(button));
        }
        return 0;
    }
    
    return -1;
}

int
Android_OnJoy(int device_id, int axis, float value)
{
    /* Android gives joy info normalized as [-1.0, 1.0] or [0.0, 1.0] */
    SDL_joylist_item *item = JoystickByDeviceId(device_id);
    if (item && item->joystick) {
        SDL_PrivateJoystickAxis(item->joystick, axis, (Sint16) (32767.*value));
    }
    
    return 0;
}

int
Android_OnHat(int device_id, int hat_id, int x, int y)
{
    const Uint8 position_map[3][3] = {
        {SDL_HAT_LEFTUP, SDL_HAT_UP, SDL_HAT_RIGHTUP},
        {SDL_HAT_LEFT, SDL_HAT_CENTERED, SDL_HAT_RIGHT},
        {SDL_HAT_LEFTDOWN, SDL_HAT_DOWN, SDL_HAT_RIGHTDOWN}
    };

    if (x >= -1 && x <=1 && y >= -1 && y <= 1) {
        SDL_joylist_item *item = JoystickByDeviceId(device_id);
        if (item && item->joystick) {
            SDL_PrivateJoystickHat(item->joystick, hat_id, position_map[y+1][x+1]);
        }
        return 0;
    }

    return -1;
}


int
Android_AddJoystick(int device_id, const char *name, const char *desc, SDL_bool is_accelerometer, int nbuttons, int naxes, int nhats, int nballs)
{
    SDL_JoystickGUID guid;
    SDL_joylist_item *item;

    if (!SDL_GetHintBoolean(SDL_HINT_TV_REMOTE_AS_JOYSTICK, SDL_TRUE)) {
        /* Ignore devices that aren't actually controllers (e.g. remotes), they'll be handled as keyboard input */
        if (naxes < 2 && nhats < 1) {
            return -1;
        }
    }
    
    if (JoystickByDeviceId(device_id) != NULL || name == NULL) {
        return -1;
    }
    
    /* the GUID is just the first 16 chars of the name for now */
    SDL_zero(guid);
    SDL_memcpy(&guid, desc, SDL_min(sizeof(guid), SDL_strlen(desc)));

    item = (SDL_joylist_item *) SDL_malloc(sizeof (SDL_joylist_item));
    if (item == NULL) {
        return -1;
    }

    SDL_zerop(item);
    item->guid = guid;
    item->device_id = device_id;
    item->name = SDL_strdup(name);
    if (item->name == NULL) {
         SDL_free(item);
         return -1;
    }
    
    item->is_accelerometer = is_accelerometer;
    if (nbuttons > -1) {
        item->nbuttons = nbuttons;
    }
    else {
        item->nbuttons = ANDROID_MAX_NBUTTONS;
    }
    item->naxes = naxes;
    item->nhats = nhats;
    item->nballs = nballs;
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

#ifdef DEBUG_JOYSTICK
    SDL_Log("Added joystick %s with device_id %d", name, device_id);
#endif

    return numjoysticks;
}

int 
Android_RemoveJoystick(int device_id)
{
    SDL_joylist_item *item = SDL_joylist;
    SDL_joylist_item *prev = NULL;
    
    /* Don't call JoystickByDeviceId here or there'll be an infinite loop! */
    while (item != NULL) {
        if (item->device_id == device_id) {
            break;
        }
        prev = item;
        item = item->next;
    }
    
    if (item == NULL) {
        return -1;
    }

    if (item->joystick) {
        item->joystick->hwdata = NULL;
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

#ifdef DEBUG_JOYSTICK
    SDL_Log("Removed joystick with device_id %d", device_id);
#endif
    
    SDL_free(item->name);
    SDL_free(item);
    return numjoysticks;
}


static SDL_bool SteamControllerConnectedCallback(const char *name, SDL_JoystickGUID guid, int *device_instance)
{
    SDL_joylist_item *item;
    
    item = (SDL_joylist_item *)SDL_calloc(1, sizeof (SDL_joylist_item));
    if (item == NULL) {
        return SDL_FALSE;
    }

    *device_instance = item->device_instance = instance_counter++;
    item->device_id = -1;
    item->name = SDL_strdup(name);
    item->guid = guid;
    SDL_GetSteamControllerInputs(&item->nbuttons,
                                 &item->naxes,
                                 &item->nhats);
    item->m_bSteamController = SDL_TRUE;

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
    SDL_joylist_item *item = SDL_joylist;
    SDL_joylist_item *prev = NULL;
    
    while (item != NULL) {
        if (item->device_instance == device_instance) {
            break;
        }
        prev = item;
        item = item->next;
    }
    
    if (item == NULL) {
        return;
    }

    if (item->joystick) {
        item->joystick->hwdata = NULL;
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
}

int
SDL_SYS_JoystickInit(void)
{
    SDL_SYS_JoystickDetect();
    
    if (SDL_GetHintBoolean(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, SDL_TRUE)) {
        /* Default behavior, accelerometer as joystick */
        Android_AddJoystick(ANDROID_ACCELEROMETER_DEVICE_ID, ANDROID_ACCELEROMETER_NAME, ANDROID_ACCELEROMETER_NAME, SDL_TRUE, 0, 3, 0, 0);
    }
   
    SDL_InitSteamControllers(SteamControllerConnectedCallback,
                             SteamControllerDisconnectedCallback);

    return (numjoysticks);

}

int
SDL_SYS_NumJoysticks(void)
{
    return numjoysticks;
}

void
SDL_SYS_JoystickDetect(void)
{
    /* Support for device connect/disconnect is API >= 16 only,
     * so we poll every three seconds
     * Ref: http://developer.android.com/reference/android/hardware/input/InputManager.InputDeviceListener.html
     */
    static Uint32 timeout = 0;
    if (!timeout || SDL_TICKS_PASSED(SDL_GetTicks(), timeout)) {
        timeout = SDL_GetTicks() + 3000;
        Android_JNI_PollInputDevices();
    }

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

static SDL_joylist_item *
JoystickByDeviceId(int device_id)
{
    SDL_joylist_item *item = SDL_joylist;

    while (item != NULL) {
        if (item->device_id == device_id) {
            return item;
        }
        item = item->next;
    }
    
    /* Joystick not found, try adding it */
    SDL_SYS_JoystickDetect();
    
    while (item != NULL) {
        if (item->device_id == device_id) {
            return item;
        }
        item = item->next;
    }

    return NULL;
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
    
    if (item->joystick != NULL) {
        return SDL_SetError("Joystick already opened");
    }

    joystick->instance_id = item->device_instance;
    joystick->hwdata = (struct joystick_hwdata *) item;
    item->joystick = joystick;
    joystick->nhats = item->nhats;
    joystick->nballs = item->nballs;
    joystick->nbuttons = item->nbuttons;
    joystick->naxes = item->naxes;

    return (0);
}

/* Function to determine if this joystick is attached to the system right now */
SDL_bool SDL_SYS_JoystickAttached(SDL_Joystick *joystick)
{
    return joystick->hwdata != NULL;
}

void
SDL_SYS_JoystickUpdate(SDL_Joystick * joystick)
{
    SDL_joylist_item *item = (SDL_joylist_item *) joystick->hwdata;

    if (item == NULL) {
        return;
    }
 
    if (item->m_bSteamController) {
        SDL_UpdateSteamController(joystick);
        return;
    }

    if (item->is_accelerometer) {
        int i;
        Sint16 value;
        float values[3];

        if (Android_JNI_GetAccelerometerValues(values)) {
            for (i = 0; i < 3; i++) {
                if (values[i] > 1.0f) {
                    values[i] = 1.0f;
                } else if (values[i] < -1.0f) {
                    values[i] = -1.0f;
                }

                value = (Sint16)(values[i] * 32767.0f);
                SDL_PrivateJoystickAxis(item->joystick, i, value);
            }
        }
    }
}

/* Function to close a joystick after use */
void
SDL_SYS_JoystickClose(SDL_Joystick * joystick)
{
    SDL_joylist_item *item = (SDL_joylist_item *) joystick->hwdata;
    if (item) {
        item->joystick = NULL;
    }
}

/* Function to perform any system-specific joystick related cleanup */
void
SDL_SYS_JoystickQuit(void)
{
/* We don't have any way to scan for joysticks at init, so don't wipe the list
 * of joysticks here in case this is a reinit.
 */
#if 0
    SDL_joylist_item *item = NULL;
    SDL_joylist_item *next = NULL;

    for (item = SDL_joylist; item; item = next) {
        next = item->next;
        SDL_free(item->name);
        SDL_free(item);
    }

    SDL_joylist = SDL_joylist_tail = NULL;

    numjoysticks = 0;
    instance_counter = 0;
#endif /* 0 */

    SDL_QuitSteamControllers();
}

SDL_JoystickGUID SDL_SYS_JoystickGetDeviceGUID(int device_index)
{
    return JoystickByDevIndex(device_index)->guid;
}

SDL_JoystickGUID SDL_SYS_JoystickGetGUID(SDL_Joystick * joystick)
{
    SDL_JoystickGUID guid;
    
    if (joystick->hwdata != NULL) {
        return ((SDL_joylist_item*)joystick->hwdata)->guid;
    }
    
    SDL_zero(guid);
    return guid;
}

SDL_bool SDL_SYS_IsDPAD_DeviceIndex(int device_index)
{
    return JoystickByDevIndex(device_index)->naxes == 0;
}

#endif /* SDL_JOYSTICK_ANDROID */

/* vi: set ts=4 sw=4 expandtab: */
