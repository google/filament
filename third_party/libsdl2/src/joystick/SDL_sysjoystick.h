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
    SDL_bool has_second_value;  /* Whether we've seen a second value on the axis yet */
    SDL_bool sent_initial_value; /* Whether we've sent the initial axis value */
} SDL_JoystickAxisInfo;

typedef struct _SDL_JoystickTouchpadFingerInfo
{
    Uint8 state;
    float x;
    float y;
    float pressure;
} SDL_JoystickTouchpadFingerInfo;

typedef struct _SDL_JoystickTouchpadInfo
{
    int nfingers;
    SDL_JoystickTouchpadFingerInfo *fingers;
} SDL_JoystickTouchpadInfo;

typedef struct _SDL_JoystickSensorInfo
{
    SDL_SensorType type;
    SDL_bool enabled;
    float rate;
    float data[3];      /* If this needs to expand, update SDL_ControllerSensorEvent */
} SDL_JoystickSensorInfo;

struct _SDL_Joystick
{
    SDL_JoystickID instance_id; /* Device instance, monotonically increasing from 0 */
    char *name;                 /* Joystick name - system dependent */
    char *serial;               /* Joystick serial */
    SDL_JoystickGUID guid;      /* Joystick guid */

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

    int ntouchpads;             /* Number of touchpads on the joystick */
    SDL_JoystickTouchpadInfo *touchpads;    /* Current touchpad states */

    int nsensors;               /* Number of sensors on the joystick */
    int nsensors_enabled;
    SDL_JoystickSensorInfo *sensors;

    Uint16 low_frequency_rumble;
    Uint16 high_frequency_rumble;
    Uint32 rumble_expiration;

    Uint16 left_trigger_rumble;
    Uint16 right_trigger_rumble;
    Uint32 trigger_rumble_expiration;

    Uint8 led_red;
    Uint8 led_green;
    Uint8 led_blue;
    Uint32 led_expiration;

    SDL_bool attached;
    SDL_bool is_game_controller;
    SDL_bool delayed_guide_button; /* SDL_TRUE if this device has the guide button event delayed */
    SDL_JoystickPowerLevel epowerlevel; /* power level of this joystick, SDL_JOYSTICK_POWER_UNKNOWN if not supported */

    struct _SDL_JoystickDriver *driver;

    struct joystick_hwdata *hwdata;     /* Driver dependent information */

    int ref_count;              /* Reference count for multiple opens */

    struct _SDL_Joystick *next; /* pointer to next joystick we have allocated */
};

/* Device bus definitions */
#define SDL_HARDWARE_BUS_USB        0x03
#define SDL_HARDWARE_BUS_BLUETOOTH  0x05

/* Macro to combine a USB vendor ID and product ID into a single Uint32 value */
#define MAKE_VIDPID(VID, PID)   (((Uint32)(VID))<<16|(PID))

typedef struct _SDL_JoystickDriver
{
    /* Function to scan the system for joysticks.
     * Joystick 0 should be the system default joystick.
     * This function should return 0, or -1 on an unrecoverable error.
     */
    int (*Init)(void);

    /* Function to return the number of joystick devices plugged in right now */
    int (*GetCount)(void);

    /* Function to cause any queued joystick insertions to be processed */
    void (*Detect)(void);

    /* Function to get the device-dependent name of a joystick */
    const char *(*GetDeviceName)(int device_index);

    /* Function to get the player index of a joystick */
    int (*GetDevicePlayerIndex)(int device_index);

    /* Function to get the player index of a joystick */
    void (*SetDevicePlayerIndex)(int device_index, int player_index);

    /* Function to return the stable GUID for a plugged in device */
    SDL_JoystickGUID (*GetDeviceGUID)(int device_index);

    /* Function to get the current instance id of the joystick located at device_index */
    SDL_JoystickID (*GetDeviceInstanceID)(int device_index);

    /* Function to open a joystick for use.
       The joystick to open is specified by the device index.
       This should fill the nbuttons and naxes fields of the joystick structure.
       It returns 0, or -1 if there is an error.
     */
    int (*Open)(SDL_Joystick *joystick, int device_index);

    /* Rumble functionality */
    int (*Rumble)(SDL_Joystick *joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble);
    int (*RumbleTriggers)(SDL_Joystick *joystick, Uint16 left_rumble, Uint16 right_rumble);

    /* LED functionality */
    SDL_bool (*HasLED)(SDL_Joystick *joystick);
    int (*SetLED)(SDL_Joystick *joystick, Uint8 red, Uint8 green, Uint8 blue);

    /* General effects */
    int (*SendEffect)(SDL_Joystick *joystick, const void *data, int size);

    /* Sensor functionality */
    int (*SetSensorsEnabled)(SDL_Joystick *joystick, SDL_bool enabled);

    /* Function to update the state of a joystick - called as a device poll.
     * This function shouldn't update the joystick structure directly,
     * but instead should call SDL_PrivateJoystick*() to deliver events
     * and update joystick device state.
     */
    void (*Update)(SDL_Joystick *joystick);

    /* Function to close a joystick after use */
    void (*Close)(SDL_Joystick *joystick);

    /* Function to perform any system-specific joystick related cleanup */
    void (*Quit)(void);

    /* Function to get the autodetected controller mapping; returns false if there isn't any. */
    SDL_bool (*GetGamepadMapping)(int device_index, SDL_GamepadMapping * out);

} SDL_JoystickDriver;

/* Windows and Mac OSX has a limit of MAX_DWORD / 1000, Linux kernel has a limit of 0xFFFF */
#define SDL_MAX_RUMBLE_DURATION_MS  0xFFFF

#define SDL_LED_MIN_REPEAT_MS  5000

/* The available joystick drivers */
extern SDL_JoystickDriver SDL_ANDROID_JoystickDriver;
extern SDL_JoystickDriver SDL_BSD_JoystickDriver;
extern SDL_JoystickDriver SDL_DARWIN_JoystickDriver;
extern SDL_JoystickDriver SDL_DUMMY_JoystickDriver;
extern SDL_JoystickDriver SDL_EMSCRIPTEN_JoystickDriver;
extern SDL_JoystickDriver SDL_HAIKU_JoystickDriver;
extern SDL_JoystickDriver SDL_HIDAPI_JoystickDriver;
extern SDL_JoystickDriver SDL_RAWINPUT_JoystickDriver;
extern SDL_JoystickDriver SDL_IOS_JoystickDriver;
extern SDL_JoystickDriver SDL_LINUX_JoystickDriver;
extern SDL_JoystickDriver SDL_VIRTUAL_JoystickDriver;
extern SDL_JoystickDriver SDL_WGI_JoystickDriver;
extern SDL_JoystickDriver SDL_WINDOWS_JoystickDriver;
extern SDL_JoystickDriver SDL_WINMM_JoystickDriver;
extern SDL_JoystickDriver SDL_OS2_JoystickDriver;
extern SDL_JoystickDriver SDL_PSP_JoystickDriver;
extern SDL_JoystickDriver SDL_VITA_JoystickDriver;

#endif /* SDL_sysjoystick_h_ */

/* vi: set ts=4 sw=4 expandtab: */
