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

#ifndef SDL_windowshaptic_c_h_
#define SDL_windowshaptic_c_h_

#include "SDL_thread.h"
#include "../SDL_syshaptic.h"
#include "../../core/windows/SDL_directx.h"
#include "../../core/windows/SDL_xinput.h"

/*
 * Haptic system hardware data.
 */
struct haptic_hwdata
{
#if SDL_HAPTIC_DINPUT
    LPDIRECTINPUTDEVICE8 device;
#endif
    DWORD axes[3];              /* Axes to use. */
    SDL_bool is_joystick;       /* Device is loaded as joystick. */
    Uint8 bXInputHaptic; /* Supports force feedback via XInput. */
    Uint8 userid; /* XInput userid index for this joystick */
    SDL_Thread *thread;
    SDL_mutex *mutex;
    Uint32 stopTicks;
    SDL_atomic_t stopThread;
};


/*
 * Haptic system effect data.
 */
struct haptic_hweffect
{
#if SDL_HAPTIC_DINPUT
    DIEFFECT effect;
    LPDIRECTINPUTEFFECT ref;
#endif
#if SDL_HAPTIC_XINPUT
    XINPUT_VIBRATION vibration;
#endif
};

/*
* List of available haptic devices.
*/
typedef struct SDL_hapticlist_item
{
    char *name;
    SDL_Haptic *haptic;
#if SDL_HAPTIC_DINPUT
    DIDEVICEINSTANCE instance;
    DIDEVCAPS capabilities;
#endif
    SDL_bool bXInputHaptic; /* Supports force feedback via XInput. */
    Uint8 userid; /* XInput userid index for this joystick */
    struct SDL_hapticlist_item *next;
} SDL_hapticlist_item;

extern SDL_hapticlist_item *SDL_hapticlist;

extern int SDL_SYS_AddHapticDevice(SDL_hapticlist_item *item);
extern int SDL_SYS_RemoveHapticDevice(SDL_hapticlist_item *prev, SDL_hapticlist_item *item);

#endif /* SDL_windowshaptic_c_h_ */

/* vi: set ts=4 sw=4 expandtab: */

