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

#include "SDL_events.h"
#include "../SDL_sysjoystick.h"
#include "../../core/windows/SDL_windows.h"
#include "../../core/windows/SDL_directx.h"

#define MAX_INPUTS  256     /* each joystick can have up to 256 inputs */

typedef struct JoyStick_DeviceData
{
    SDL_JoystickGUID guid;
    char *joystickname;
    Uint8 send_add_event;
    SDL_JoystickID nInstanceID;
    SDL_bool bXInputDevice;
    BYTE SubType;
    Uint8 XInputUserId;
    DIDEVICEINSTANCE dxdevice;
    char hidPath[MAX_PATH];
    struct JoyStick_DeviceData *pNext;
} JoyStick_DeviceData;

extern JoyStick_DeviceData *SYS_Joystick;    /* array to hold joystick ID values */

typedef enum Type
{
    BUTTON,
    AXIS,
    HAT
} Type;

typedef struct input_t
{
    /* DirectInput offset for this input type: */
    DWORD ofs;

    /* Button, axis or hat: */
    Type type;

    /* SDL input offset: */
    Uint8 num;
} input_t;

/* The private structure used to keep track of a joystick */
struct joystick_hwdata
{
    SDL_JoystickGUID guid;

#if SDL_JOYSTICK_DINPUT
    LPDIRECTINPUTDEVICE8 InputDevice;
    DIDEVCAPS Capabilities;
    SDL_bool buffered;
    input_t Inputs[MAX_INPUTS];
    int NumInputs;
    int NumSliders;
    SDL_bool ff_initialized;
    DIEFFECT *ffeffect;
    LPDIRECTINPUTEFFECT ffeffect_ref;
#endif

    SDL_bool bXInputDevice; /* SDL_TRUE if this device supports using the xinput API rather than DirectInput */
    SDL_bool bXInputHaptic; /* Supports force feedback via XInput. */
    Uint8 userid; /* XInput userid index for this joystick */
    DWORD dwPacketNumber;
};

#if SDL_JOYSTICK_DINPUT
extern const DIDATAFORMAT SDL_c_dfDIJoystick2;
#endif

extern void WINDOWS_AddJoystickDevice(JoyStick_DeviceData *device);

/* vi: set ts=4 sw=4 expandtab: */
