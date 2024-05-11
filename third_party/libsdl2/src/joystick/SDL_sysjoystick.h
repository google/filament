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
#include "../SDL_internal.h"

#ifndef SDL_sysjoystick_h_
#define SDL_sysjoystick_h_

/* This is the system specific header for the SDL joystick API */

#include "SDL_joystick.h"
#include "SDL_joystick_c.h"

/* The SDL joystick structure */
typedef struct _SDL_JoystickAxisInfo
{
    Sint16 initial_value;       /* Initial axis state */
    Sint16 value;               /* Current axis state */
    Sint16 zero;                /* Zero point on the axis (-32768 for triggers) */
    SDL_bool has_initial_value; /* Whether we've seen a value on the axis yet */
    SDL_bool sent_initial_value; /* Whether we've sent the initial axis value */
} SDL_JoystickAxisInfo;

struct _SDL_Joystick
{
    SDL_JoystickID instance_id; /* Device instance, monotonically increasing from 0 */
    char *name;                 /* Joystick name - system dependent */

    int naxes;                  /* Number of axis controls on the joystick */
    SDL_JoystickAxisInfo *axes;

    int nhats;                  /* Number of hats on the joystick */
    Uint8 *hats;                /* Current hat states */

    int nballs;                 /* Number of trackballs on the joystick */
    struct balldelta {
        int dx;
        int dy;
    } *balls;                   /* Current ball motion deltas */

    int nbuttons;               /* Number of buttons on the joystick */
    Uint8 *buttons;             /* Current button states */

    struct joystick_hwdata *hwdata;     /* Driver dependent information */

    int ref_count;              /* Reference count for multiple opens */

    SDL_bool is_game_controller;
    SDL_bool force_recentering; /* SDL_TRUE if this device needs to have its state reset to 0 */
    SDL_JoystickPowerLevel epowerlevel; /* power level of this joystick, SDL_JOYSTICK_POWER_UNKNOWN if not supported */
    struct _SDL_Joystick *next; /* pointer to next joystick we have allocated */
};

/* Macro to combine a USB vendor ID and product ID into a single Uint32 value */
#define MAKE_VIDPID(VID, PID)   (((Uint32)(VID))<<16|(PID))

/* Function to scan the system for joysticks.
 * Joystick 0 should be the system default joystick.
 * This function should return the number of available joysticks, or -1
 * on an unrecoverable fatal error.
 */
extern int SDL_SYS_JoystickInit(void);

/* Function to return the number of joystick devices plugged in right now */
extern int SDL_SYS_NumJoysticks(void);

/* Function to cause any queued joystick insertions to be processed */
extern void SDL_SYS_JoystickDetect(void);

/* Function to get the device-dependent name of a joystick */
extern const char *SDL_SYS_JoystickNameForDeviceIndex(int device_index);

/* Function to get the current instance id of the joystick located at device_index */
extern SDL_JoystickID SDL_SYS_GetInstanceIdOfDeviceIndex(int device_index);

/* Function to open a joystick for use.
   The joystick to open is specified by the device index.
   This should fill the nbuttons and naxes fields of the joystick structure.
   It returns 0, or -1 if there is an error.
 */
extern int SDL_SYS_JoystickOpen(SDL_Joystick * joystick, int device_index);

/* Function to query if the joystick is currently attached
 * It returns SDL_TRUE if attached, SDL_FALSE otherwise.
 */
extern SDL_bool SDL_SYS_JoystickAttached(SDL_Joystick * joystick);

/* Function to update the state of a joystick - called as a device poll.
 * This function shouldn't update the joystick structure directly,
 * but instead should call SDL_PrivateJoystick*() to deliver events
 * and update joystick device state.
 */
extern void SDL_SYS_JoystickUpdate(SDL_Joystick * joystick);

/* Function to close a joystick after use */
extern void SDL_SYS_JoystickClose(SDL_Joystick * joystick);

/* Function to perform any system-specific joystick related cleanup */
extern void SDL_SYS_JoystickQuit(void);

/* Function to return the stable GUID for a plugged in device */
extern SDL_JoystickGUID SDL_SYS_JoystickGetDeviceGUID(int device_index);

/* Function to return the stable GUID for a opened joystick */
extern SDL_JoystickGUID SDL_SYS_JoystickGetGUID(SDL_Joystick * joystick);

#if SDL_JOYSTICK_XINPUT
/* Function returns SDL_TRUE if this device is an XInput gamepad */
extern SDL_bool SDL_SYS_IsXInputGamepad_DeviceIndex(int device_index);
#endif

#if defined(__ANDROID__)
/* Function returns SDL_TRUE if this device is a DPAD (maybe a TV remote) */
extern SDL_bool SDL_SYS_IsDPAD_DeviceIndex(int device_index);
#endif

#endif /* SDL_sysjoystick_h_ */

/* vi: set ts=4 sw=4 expandtab: */
