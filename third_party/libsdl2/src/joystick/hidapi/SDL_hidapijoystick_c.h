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

#ifndef SDL_JOYSTICK_HIDAPI_H
#define SDL_JOYSTICK_HIDAPI_H

#include "SDL_atomic.h"
#include "SDL_mutex.h"
#include "SDL_joystick.h"
#include "SDL_gamecontroller.h"
#include "../../hidapi/hidapi/hidapi.h"
#include "../usb_ids.h"

/* This is the full set of HIDAPI drivers available */
#define SDL_JOYSTICK_HIDAPI_GAMECUBE
#define SDL_JOYSTICK_HIDAPI_LUNA
#define SDL_JOYSTICK_HIDAPI_PS4
#define SDL_JOYSTICK_HIDAPI_PS5
#define SDL_JOYSTICK_HIDAPI_STADIA
#define SDL_JOYSTICK_HIDAPI_SWITCH
#define SDL_JOYSTICK_HIDAPI_XBOX360
#define SDL_JOYSTICK_HIDAPI_XBOXONE

#if defined(__IPHONEOS__) || defined(__TVOS__) || defined(__ANDROID__)
/* Very basic Steam Controller support on mobile devices */
#define SDL_JOYSTICK_HIDAPI_STEAM
#endif

/* The maximum size of a USB packet for HID devices */
#define USB_PACKET_LENGTH   64

/* Forward declaration */
struct _SDL_HIDAPI_DeviceDriver;

typedef struct _SDL_HIDAPI_Device
{
    char *name;
    char *path;
    Uint16 vendor_id;
    Uint16 product_id;
    Uint16 version;
    char *serial;
    SDL_JoystickGUID guid;
    int interface_number;   /* Available on Windows and Linux */
    int interface_class;
    int interface_subclass;
    int interface_protocol;
    Uint16 usage_page;      /* Available on Windows and Mac OS X */
    Uint16 usage;           /* Available on Windows and Mac OS X */

    struct _SDL_HIDAPI_DeviceDriver *driver;
    void *context;
    SDL_mutex *dev_lock;
    hid_device *dev;
    SDL_atomic_t rumble_pending;
    int num_joysticks;
    SDL_JoystickID *joysticks;

    /* Used during scanning for device changes */
    SDL_bool seen;

    /* Used to flag that the device is being updated */
    SDL_bool updating;

    struct _SDL_HIDAPI_Device *next;
} SDL_HIDAPI_Device;

typedef struct _SDL_HIDAPI_DeviceDriver
{
    const char *hint;
    SDL_bool enabled;
    SDL_bool (*IsSupportedDevice)(const char *name, SDL_GameControllerType type, Uint16 vendor_id, Uint16 product_id, Uint16 version, int interface_number, int interface_class, int interface_subclass, int interface_protocol);
    const char *(*GetDeviceName)(Uint16 vendor_id, Uint16 product_id);
    SDL_bool (*InitDevice)(SDL_HIDAPI_Device *device);
    int (*GetDevicePlayerIndex)(SDL_HIDAPI_Device *device, SDL_JoystickID instance_id);
    void (*SetDevicePlayerIndex)(SDL_HIDAPI_Device *device, SDL_JoystickID instance_id, int player_index);
    SDL_bool (*UpdateDevice)(SDL_HIDAPI_Device *device);
    SDL_bool (*OpenJoystick)(SDL_HIDAPI_Device *device, SDL_Joystick *joystick);
    int (*RumbleJoystick)(SDL_HIDAPI_Device *device, SDL_Joystick *joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble);
    int (*RumbleJoystickTriggers)(SDL_HIDAPI_Device *device, SDL_Joystick *joystick, Uint16 left_rumble, Uint16 right_rumble);
    SDL_bool (*HasJoystickLED)(SDL_HIDAPI_Device *device, SDL_Joystick *joystick);
    int (*SetJoystickLED)(SDL_HIDAPI_Device *device, SDL_Joystick *joystick, Uint8 red, Uint8 green, Uint8 blue);
    int (*SendJoystickEffect)(SDL_HIDAPI_Device *device, SDL_Joystick *joystick, const void *data, int size);
    int (*SetJoystickSensorsEnabled)(SDL_HIDAPI_Device *device, SDL_Joystick *joystick, SDL_bool enabled);
    void (*CloseJoystick)(SDL_HIDAPI_Device *device, SDL_Joystick *joystick);
    void (*FreeDevice)(SDL_HIDAPI_Device *device);

} SDL_HIDAPI_DeviceDriver;


/* HIDAPI device support */
extern SDL_HIDAPI_DeviceDriver SDL_HIDAPI_DriverGameCube;
extern SDL_HIDAPI_DeviceDriver SDL_HIDAPI_DriverLuna;
extern SDL_HIDAPI_DeviceDriver SDL_HIDAPI_DriverPS4;
extern SDL_HIDAPI_DeviceDriver SDL_HIDAPI_DriverPS5;
extern SDL_HIDAPI_DeviceDriver SDL_HIDAPI_DriverStadia;
extern SDL_HIDAPI_DeviceDriver SDL_HIDAPI_DriverSteam;
extern SDL_HIDAPI_DeviceDriver SDL_HIDAPI_DriverSwitch;
extern SDL_HIDAPI_DeviceDriver SDL_HIDAPI_DriverXbox360;
extern SDL_HIDAPI_DeviceDriver SDL_HIDAPI_DriverXbox360W;
extern SDL_HIDAPI_DeviceDriver SDL_HIDAPI_DriverXboxOne;

/* Return true if a HID device is present and supported as a joystick of the given type */
extern SDL_bool HIDAPI_IsDeviceTypePresent(SDL_GameControllerType type);

/* Return true if a HID device is present and supported as a joystick */
extern SDL_bool HIDAPI_IsDevicePresent(Uint16 vendor_id, Uint16 product_id, Uint16 version, const char *name);

extern void HIDAPI_UpdateDevices(void);
extern SDL_bool HIDAPI_JoystickConnected(SDL_HIDAPI_Device *device, SDL_JoystickID *pJoystickID);
extern void HIDAPI_JoystickDisconnected(SDL_HIDAPI_Device *device, SDL_JoystickID joystickID);

extern void HIDAPI_DumpPacket(const char *prefix, Uint8 *data, int size);

extern float HIDAPI_RemapVal(float val, float val_min, float val_max, float output_min, float output_max);

#endif /* SDL_JOYSTICK_HIDAPI_H */

/* vi: set ts=4 sw=4 expandtab: */
