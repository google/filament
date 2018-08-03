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

#ifdef SDL_JOYSTICK_HAIKU

/* This is the Haiku implementation of the SDL joystick API */

#include <support/String.h>
#include <device/Joystick.h>

extern "C"
{

#include "SDL_joystick.h"
#include "../SDL_sysjoystick.h"
#include "../SDL_joystick_c.h"


/* The maximum number of joysticks we'll detect */
#define MAX_JOYSTICKS	16

/* A list of available joysticks */
    static char *SDL_joyport[MAX_JOYSTICKS];
    static char *SDL_joyname[MAX_JOYSTICKS];

/* The private structure used to keep track of a joystick */
    struct joystick_hwdata
    {
        BJoystick *stick;
        uint8 *new_hats;
        int16 *new_axes;
    };

    static int SDL_SYS_numjoysticks = 0;

/* Function to scan the system for joysticks.
 * Joystick 0 should be the system default joystick.
 * It should return 0, or -1 on an unrecoverable fatal error.
 */
    int SDL_SYS_JoystickInit(void)
    {
        BJoystick joystick;
        int i;
        int32 nports;
        char name[B_OS_NAME_LENGTH];

        /* Search for attached joysticks */
          nports = joystick.CountDevices();
          SDL_SYS_numjoysticks = 0;
          SDL_memset(SDL_joyport, 0, (sizeof SDL_joyport));
          SDL_memset(SDL_joyname, 0, (sizeof SDL_joyname));
        for (i = 0; (SDL_SYS_numjoysticks < MAX_JOYSTICKS) && (i < nports); ++i)
        {
            if (joystick.GetDeviceName(i, name) == B_OK) {
                if (joystick.Open(name) != B_ERROR) {
                    BString stick_name;
                      joystick.GetControllerName(&stick_name);
                      SDL_joyport[SDL_SYS_numjoysticks] = SDL_strdup(name);
                      SDL_joyname[SDL_SYS_numjoysticks] = SDL_strdup(stick_name.String());
                      SDL_SYS_numjoysticks++;
                      joystick.Close();
                }
            }
        }
        return (SDL_SYS_numjoysticks);
    }

    int SDL_SYS_NumJoysticks(void)
    {
        return SDL_SYS_numjoysticks;
    }

    void SDL_SYS_JoystickDetect(void)
    {
    }

/* Function to get the device-dependent name of a joystick */
    const char *SDL_SYS_JoystickNameForDeviceIndex(int device_index)
    {
        return SDL_joyname[device_index];
    }

/* Function to perform the mapping from device index to the instance id for this index */
    SDL_JoystickID SDL_SYS_GetInstanceIdOfDeviceIndex(int device_index)
    {
        return device_index;
    }

/* Function to open a joystick for use.
   The joystick to open is specified by the device index.
   This should fill the nbuttons and naxes fields of the joystick structure.
   It returns 0, or -1 if there is an error.
 */
    int SDL_SYS_JoystickOpen(SDL_Joystick * joystick, int device_index)
    {
        BJoystick *stick;

        /* Create the joystick data structure */
        joystick->instance_id = device_index;
        joystick->hwdata = (struct joystick_hwdata *)
            SDL_malloc(sizeof(*joystick->hwdata));
        if (joystick->hwdata == NULL) {
            return SDL_OutOfMemory();
        }
        SDL_memset(joystick->hwdata, 0, sizeof(*joystick->hwdata));
        stick = new BJoystick;
        joystick->hwdata->stick = stick;

        /* Open the requested joystick for use */
        if (stick->Open(SDL_joyport[device_index]) == B_ERROR) {
            SDL_SYS_JoystickClose(joystick);
            return SDL_SetError("Unable to open joystick");
        }

        /* Set the joystick to calibrated mode */
        stick->EnableCalibration();

        /* Get the number of buttons, hats, and axes on the joystick */
        joystick->nbuttons = stick->CountButtons();
        joystick->naxes = stick->CountAxes();
        joystick->nhats = stick->CountHats();

        joystick->hwdata->new_axes = (int16 *)
            SDL_malloc(joystick->naxes * sizeof(int16));
        joystick->hwdata->new_hats = (uint8 *)
            SDL_malloc(joystick->nhats * sizeof(uint8));
        if (!joystick->hwdata->new_hats || !joystick->hwdata->new_axes) {
            SDL_SYS_JoystickClose(joystick);
            return SDL_OutOfMemory();
        }

        /* We're done! */
        return (0);
    }

/* Function to determine if this joystick is attached to the system right now */
    SDL_bool SDL_SYS_JoystickAttached(SDL_Joystick *joystick)
    {
        return SDL_TRUE;
    }

/* Function to update the state of a joystick - called as a device poll.
 * This function shouldn't update the joystick structure directly,
 * but instead should call SDL_PrivateJoystick*() to deliver events
 * and update joystick device state.
 */
    void SDL_SYS_JoystickUpdate(SDL_Joystick * joystick)
    {
        static const Uint8 hat_map[9] = {
            SDL_HAT_CENTERED,
            SDL_HAT_UP,
            SDL_HAT_RIGHTUP,
            SDL_HAT_RIGHT,
            SDL_HAT_RIGHTDOWN,
            SDL_HAT_DOWN,
            SDL_HAT_LEFTDOWN,
            SDL_HAT_LEFT,
            SDL_HAT_LEFTUP
        };

        BJoystick *stick;
        int i;
        int16 *axes;
        uint8 *hats;
        uint32 buttons;

        /* Set up data pointers */
        stick = joystick->hwdata->stick;
        axes = joystick->hwdata->new_axes;
        hats = joystick->hwdata->new_hats;

        /* Get the new joystick state */
        stick->Update();
        stick->GetAxisValues(axes);
        stick->GetHatValues(hats);
        buttons = stick->ButtonValues();

        /* Generate axis motion events */
        for (i = 0; i < joystick->naxes; ++i) {
            SDL_PrivateJoystickAxis(joystick, i, axes[i]);
        }

        /* Generate hat change events */
        for (i = 0; i < joystick->nhats; ++i) {
            SDL_PrivateJoystickHat(joystick, i, hat_map[hats[i]]);
        }

        /* Generate button events */
        for (i = 0; i < joystick->nbuttons; ++i) {
            SDL_PrivateJoystickButton(joystick, i, (buttons & 0x01));
            buttons >>= 1;
        }
    }

/* Function to close a joystick after use */
    void SDL_SYS_JoystickClose(SDL_Joystick * joystick)
    {
        if (joystick->hwdata) {
            joystick->hwdata->stick->Close();
            delete joystick->hwdata->stick;
            SDL_free(joystick->hwdata->new_hats);
            SDL_free(joystick->hwdata->new_axes);
            SDL_free(joystick->hwdata);
        }
    }

/* Function to perform any system-specific joystick related cleanup */
    void SDL_SYS_JoystickQuit(void)
    {
        int i;

        for (i = 0; i < SDL_SYS_numjoysticks; ++i) {
            SDL_free(SDL_joyport[i]);
        }
        SDL_joyport[0] = NULL;

        for (i = 0; i < SDL_SYS_numjoysticks; ++i) {
            SDL_free(SDL_joyname[i]);
        }
        SDL_joyname[0] = NULL;
    }

    SDL_JoystickGUID SDL_SYS_JoystickGetDeviceGUID( int device_index )
    {
        SDL_JoystickGUID guid;
        /* the GUID is just the first 16 chars of the name for now */
        const char *name = SDL_SYS_JoystickNameForDeviceIndex( device_index );
        SDL_zero( guid );
        SDL_memcpy( &guid, name, SDL_min( sizeof(guid), SDL_strlen( name ) ) );
        return guid;
    }

    SDL_JoystickGUID SDL_SYS_JoystickGetGUID(SDL_Joystick * joystick)
    {
        SDL_JoystickGUID guid;
        /* the GUID is just the first 16 chars of the name for now */
        const char *name = joystick->name;
        SDL_zero( guid );
        SDL_memcpy( &guid, name, SDL_min( sizeof(guid), SDL_strlen( name ) ) );
        return guid;
    }

};                              // extern "C"

#endif /* SDL_JOYSTICK_HAIKU */

/* vi: set ts=4 sw=4 expandtab: */
