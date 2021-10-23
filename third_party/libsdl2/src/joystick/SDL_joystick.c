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

/* This is the joystick API for Simple DirectMedia Layer */

#include "SDL.h"
#include "SDL_atomic.h"
#include "SDL_events.h"
#include "SDL_sysjoystick.h"
#include "SDL_hints.h"

#if !SDL_EVENTS_DISABLED
#include "../events/SDL_events_c.h"
#endif
#include "../video/SDL_sysvideo.h"
#include "hidapi/SDL_hidapijoystick_c.h"

/* This is included in only one place because it has a large static list of controllers */
#include "controller_type.h"

#ifdef __WIN32__
/* Needed for checking for input remapping programs */
#include "../core/windows/SDL_windows.h"

#undef UNICODE          /* We want ASCII functions */
#include <tlhelp32.h>
#endif

#if SDL_JOYSTICK_VIRTUAL
#include "./virtual/SDL_virtualjoystick_c.h"
#endif

static SDL_JoystickDriver *SDL_joystick_drivers[] = {
#ifdef SDL_JOYSTICK_HIDAPI /* Before WINDOWS_ driver, as WINDOWS wants to check if this driver is handling things */
    &SDL_HIDAPI_JoystickDriver,
#endif
#ifdef SDL_JOYSTICK_RAWINPUT /* Before WINDOWS_ driver, as WINDOWS wants to check if this driver is handling things */
    &SDL_RAWINPUT_JoystickDriver,
#endif
#if defined(SDL_JOYSTICK_WGI)
    &SDL_WGI_JoystickDriver,
#endif
#if defined(SDL_JOYSTICK_DINPUT) || defined(SDL_JOYSTICK_XINPUT)
    &SDL_WINDOWS_JoystickDriver,
#endif
#if defined(SDL_JOYSTICK_WINMM)
    &SDL_WINMM_JoystickDriver,
#endif
#ifdef SDL_JOYSTICK_LINUX
    &SDL_LINUX_JoystickDriver,
#endif
#ifdef SDL_JOYSTICK_IOKIT
    &SDL_DARWIN_JoystickDriver,
#endif
#if (defined(__MACOSX__) || defined(__IPHONEOS__) || defined(__TVOS__)) && !defined(SDL_JOYSTICK_DISABLED)
    &SDL_IOS_JoystickDriver,
#endif
#ifdef SDL_JOYSTICK_ANDROID
    &SDL_ANDROID_JoystickDriver,
#endif
#ifdef SDL_JOYSTICK_EMSCRIPTEN
    &SDL_EMSCRIPTEN_JoystickDriver,
#endif
#ifdef SDL_JOYSTICK_HAIKU
    &SDL_HAIKU_JoystickDriver,
#endif
#ifdef SDL_JOYSTICK_USBHID  /* !!! FIXME: "USBHID" is a generic name, and doubly-confusing with HIDAPI next to it. This is the *BSD interface, rename this. */
    &SDL_BSD_JoystickDriver,
#endif
#ifdef SDL_JOYSTICK_OS2
    &SDL_OS2_JoystickDriver,
#endif
#ifdef SDL_JOYSTICK_PSP
    &SDL_PSP_JoystickDriver,
#endif
#ifdef SDL_JOYSTICK_VIRTUAL
    &SDL_VIRTUAL_JoystickDriver,
#endif
#ifdef SDL_JOYSTICK_VITA
    &SDL_VITA_JoystickDriver,
#endif
#if defined(SDL_JOYSTICK_DUMMY) || defined(SDL_JOYSTICK_DISABLED)
    &SDL_DUMMY_JoystickDriver
#endif
};
static SDL_bool SDL_joystick_allows_background_events = SDL_FALSE;
static SDL_Joystick *SDL_joysticks = NULL;
static SDL_bool SDL_updating_joystick = SDL_FALSE;
static SDL_mutex *SDL_joystick_lock = NULL; /* This needs to support recursive locks */
static SDL_atomic_t SDL_next_joystick_instance_id;
static int SDL_joystick_player_count = 0;
static SDL_JoystickID *SDL_joystick_players = NULL;

void
SDL_LockJoysticks(void)
{
    if (SDL_joystick_lock) {
        SDL_LockMutex(SDL_joystick_lock);
    }
}

void
SDL_UnlockJoysticks(void)
{
    if (SDL_joystick_lock) {
        SDL_UnlockMutex(SDL_joystick_lock);
    }
}

static int
SDL_FindFreePlayerIndex()
{
    int player_index;

    for (player_index = 0; player_index < SDL_joystick_player_count; ++player_index) {
        if (SDL_joystick_players[player_index] == -1) {
            return player_index;
        }
    }
    return player_index;
}

static int
SDL_GetPlayerIndexForJoystickID(SDL_JoystickID instance_id)
{
    int player_index;

    for (player_index = 0; player_index < SDL_joystick_player_count; ++player_index) {
        if (instance_id == SDL_joystick_players[player_index]) {
            break;
        }
    }
    if (player_index == SDL_joystick_player_count) {
        player_index = -1;
    }
    return player_index;
}

static SDL_JoystickID
SDL_GetJoystickIDForPlayerIndex(int player_index)
{
    if (player_index < 0 || player_index >= SDL_joystick_player_count) {
        return -1;
    }
    return SDL_joystick_players[player_index];
}

static SDL_bool
SDL_SetJoystickIDForPlayerIndex(int player_index, SDL_JoystickID instance_id)
{
    SDL_JoystickID existing_instance = SDL_GetJoystickIDForPlayerIndex(player_index);
    SDL_JoystickDriver *driver;
    int device_index;
    int existing_player_index;

    if (player_index >= SDL_joystick_player_count) {
        SDL_JoystickID *new_players = (SDL_JoystickID *)SDL_realloc(SDL_joystick_players, (player_index + 1)*sizeof(*SDL_joystick_players));
        if (!new_players) {
            SDL_OutOfMemory();
            return SDL_FALSE;
        }

        SDL_joystick_players = new_players;
        SDL_memset(&SDL_joystick_players[SDL_joystick_player_count], 0xFF, (player_index - SDL_joystick_player_count + 1) * sizeof(SDL_joystick_players[0]));
        SDL_joystick_player_count = player_index + 1;
    } else if (SDL_joystick_players[player_index] == instance_id) {
        /* Joystick is already assigned the requested player index */
        return SDL_TRUE;
    }

    /* Clear the old player index */
    existing_player_index = SDL_GetPlayerIndexForJoystickID(instance_id);
    if (existing_player_index >= 0) {
        SDL_joystick_players[existing_player_index] = -1;
    }

    if (player_index >= 0) {
        SDL_joystick_players[player_index] = instance_id;
    }

    /* Update the driver with the new index */
    device_index = SDL_JoystickGetDeviceIndexFromInstanceID(instance_id);
    if (SDL_GetDriverAndJoystickIndex(device_index, &driver, &device_index)) {
        driver->SetDevicePlayerIndex(device_index, player_index);
    }

    /* Move any existing joystick to another slot */
    if (existing_instance >= 0) {
        SDL_SetJoystickIDForPlayerIndex(SDL_FindFreePlayerIndex(), existing_instance);
    }
    return SDL_TRUE;
}

static void SDLCALL
SDL_JoystickAllowBackgroundEventsChanged(void *userdata, const char *name, const char *oldValue, const char *hint)
{
    if (hint && *hint == '1') {
        SDL_joystick_allows_background_events = SDL_TRUE;
    } else {
        SDL_joystick_allows_background_events = SDL_FALSE;
    }
}

int
SDL_JoystickInit(void)
{
    int i, status;

    SDL_GameControllerInitMappings();

    /* Create the joystick list lock */
    if (!SDL_joystick_lock) {
        SDL_joystick_lock = SDL_CreateMutex();
    }

    /* See if we should allow joystick events while in the background */
    SDL_AddHintCallback(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,
                        SDL_JoystickAllowBackgroundEventsChanged, NULL);

#if !SDL_EVENTS_DISABLED
    if (SDL_InitSubSystem(SDL_INIT_EVENTS) < 0) {
        return -1;
    }
#endif /* !SDL_EVENTS_DISABLED */

    status = -1;
    for (i = 0; i < SDL_arraysize(SDL_joystick_drivers); ++i) {
        if (SDL_joystick_drivers[i]->Init() >= 0) {
            status = 0;
        }
    }
    return status;
}

/*
 * Count the number of joysticks attached to the system
 */
int
SDL_NumJoysticks(void)
{
    int i, total_joysticks = 0;
    SDL_LockJoysticks();
    for (i = 0; i < SDL_arraysize(SDL_joystick_drivers); ++i) {
        total_joysticks += SDL_joystick_drivers[i]->GetCount();
    }
    SDL_UnlockJoysticks();
    return total_joysticks;
}

/*
 * Return the next available joystick instance ID
 * This may be called by drivers from multiple threads, unprotected by any locks
 */
SDL_JoystickID SDL_GetNextJoystickInstanceID()
{
    return SDL_AtomicIncRef(&SDL_next_joystick_instance_id);
}

/*
 * Get the driver and device index for an API device index
 * This should be called while the joystick lock is held, to prevent another thread from updating the list
 */
SDL_bool
SDL_GetDriverAndJoystickIndex(int device_index, SDL_JoystickDriver **driver, int *driver_index)
{
    int i, num_joysticks, total_joysticks = 0;

    if (device_index >= 0) {
        for (i = 0; i < SDL_arraysize(SDL_joystick_drivers); ++i) {
            num_joysticks = SDL_joystick_drivers[i]->GetCount();
            if (device_index < num_joysticks) {
                *driver = SDL_joystick_drivers[i];
                *driver_index = device_index;
                return SDL_TRUE;
            }
            device_index -= num_joysticks;
            total_joysticks += num_joysticks;
        }
    }

    SDL_SetError("There are %d joysticks available", total_joysticks);
    return SDL_FALSE;
}

/*
 * Get the implementation dependent name of a joystick
 */
const char *
SDL_JoystickNameForIndex(int device_index)
{
    SDL_JoystickDriver *driver;
    const char *name = NULL;

    SDL_LockJoysticks();
    if (SDL_GetDriverAndJoystickIndex(device_index, &driver, &device_index)) {
        name = driver->GetDeviceName(device_index);
    }
    SDL_UnlockJoysticks();

    /* FIXME: Really we should reference count this name so it doesn't go away after unlock */
    return name;
}

/*
 *  Get the player index of a joystick, or -1 if it's not available
 */
int
SDL_JoystickGetDevicePlayerIndex(int device_index)
{
    int player_index;

    SDL_LockJoysticks();
    player_index = SDL_GetPlayerIndexForJoystickID(SDL_JoystickGetDeviceInstanceID(device_index));
    SDL_UnlockJoysticks();

    return player_index;
}

/*
 * Return true if this joystick is known to have all axes centered at zero
 * This isn't generally needed unless the joystick never generates an initial axis value near zero,
 * e.g. it's emulating axes with digital buttons
 */
static SDL_bool
SDL_JoystickAxesCenteredAtZero(SDL_Joystick *joystick)
{
    static Uint32 zero_centered_joysticks[] = {
        MAKE_VIDPID(0x0e8f, 0x3013),    /* HuiJia SNES USB adapter */
        MAKE_VIDPID(0x05a0, 0x3232),    /* 8Bitdo Zero Gamepad */
    };

    int i;
    Uint32 id = MAKE_VIDPID(SDL_JoystickGetVendor(joystick),
                            SDL_JoystickGetProduct(joystick));

    /*printf("JOYSTICK '%s' VID/PID 0x%.4x/0x%.4x AXES: %d\n", joystick->name, vendor, product, joystick->naxes);*/

    if (joystick->naxes == 2) {
        /* Assume D-pad or thumbstick style axes are centered at 0 */
        return SDL_TRUE;
    }

    for (i = 0; i < SDL_arraysize(zero_centered_joysticks); ++i) {
        if (id == zero_centered_joysticks[i]) {
            return SDL_TRUE;
        }
    }
    return SDL_FALSE;
}

/*
 * Open a joystick for use - the index passed as an argument refers to
 * the N'th joystick on the system.  This index is the value which will
 * identify this joystick in future joystick events.
 *
 * This function returns a joystick identifier, or NULL if an error occurred.
 */
SDL_Joystick *
SDL_JoystickOpen(int device_index)
{
    SDL_JoystickDriver *driver;
    SDL_JoystickID instance_id;
    SDL_Joystick *joystick;
    SDL_Joystick *joysticklist;
    const char *joystickname = NULL;

    SDL_LockJoysticks();

    if (!SDL_GetDriverAndJoystickIndex(device_index, &driver, &device_index)) {
        SDL_UnlockJoysticks();
        return NULL;
    }

    joysticklist = SDL_joysticks;
    /* If the joystick is already open, return it
     * it is important that we have a single joystick * for each instance id
     */
    instance_id = driver->GetDeviceInstanceID(device_index);
    while (joysticklist) {
        if (instance_id == joysticklist->instance_id) {
                joystick = joysticklist;
                ++joystick->ref_count;
                SDL_UnlockJoysticks();
                return joystick;
        }
        joysticklist = joysticklist->next;
    }

    /* Create and initialize the joystick */
    joystick = (SDL_Joystick *) SDL_calloc(sizeof(*joystick), 1);
    if (joystick == NULL) {
        SDL_OutOfMemory();
        SDL_UnlockJoysticks();
        return NULL;
    }
    joystick->driver = driver;
    joystick->instance_id = instance_id;
    joystick->attached = SDL_TRUE;
    joystick->epowerlevel = SDL_JOYSTICK_POWER_UNKNOWN;
    joystick->led_expiration = SDL_GetTicks();

    if (driver->Open(joystick, device_index) < 0) {
        SDL_free(joystick);
        SDL_UnlockJoysticks();
        return NULL;
    }

    joystickname = driver->GetDeviceName(device_index);
    if (joystickname) {
        joystick->name = SDL_strdup(joystickname);
    } else {
        joystick->name = NULL;
    }

    joystick->guid = driver->GetDeviceGUID(device_index);

    if (joystick->naxes > 0) {
        joystick->axes = (SDL_JoystickAxisInfo *) SDL_calloc(joystick->naxes, sizeof(SDL_JoystickAxisInfo));
    }
    if (joystick->nhats > 0) {
        joystick->hats = (Uint8 *) SDL_calloc(joystick->nhats, sizeof(Uint8));
    }
    if (joystick->nballs > 0) {
        joystick->balls = (struct balldelta *) SDL_calloc(joystick->nballs, sizeof(*joystick->balls));
    }
    if (joystick->nbuttons > 0) {
        joystick->buttons = (Uint8 *) SDL_calloc(joystick->nbuttons, sizeof(Uint8));
    }
    if (((joystick->naxes > 0) && !joystick->axes)
        || ((joystick->nhats > 0) && !joystick->hats)
        || ((joystick->nballs > 0) && !joystick->balls)
        || ((joystick->nbuttons > 0) && !joystick->buttons)) {
        SDL_OutOfMemory();
        SDL_JoystickClose(joystick);
        SDL_UnlockJoysticks();
        return NULL;
    }

    /* If this joystick is known to have all zero centered axes, skip the auto-centering code */
    if (SDL_JoystickAxesCenteredAtZero(joystick)) {
        int i;

        for (i = 0; i < joystick->naxes; ++i) {
            joystick->axes[i].has_initial_value = SDL_TRUE;
        }
    }

    joystick->is_game_controller = SDL_IsGameController(device_index);

    /* Add joystick to list */
    ++joystick->ref_count;
    /* Link the joystick in the list */
    joystick->next = SDL_joysticks;
    SDL_joysticks = joystick;

    SDL_UnlockJoysticks();

    driver->Update(joystick);

    return joystick;
}

int
SDL_JoystickAttachVirtual(SDL_JoystickType type,
                          int naxes, int nbuttons, int nhats)
{
#if SDL_JOYSTICK_VIRTUAL
    return SDL_JoystickAttachVirtualInner(type, naxes, nbuttons, nhats);
#else
    return SDL_SetError("SDL not built with virtual-joystick support");
#endif
}

int
SDL_JoystickDetachVirtual(int device_index)
{
#if SDL_JOYSTICK_VIRTUAL
    SDL_JoystickDriver *driver;

    SDL_LockJoysticks();
    if (SDL_GetDriverAndJoystickIndex(device_index, &driver, &device_index)) {
        if (driver == &SDL_VIRTUAL_JoystickDriver) {
            const int result = SDL_JoystickDetachVirtualInner(device_index);
            SDL_UnlockJoysticks();
            return result;
        }
    }
    SDL_UnlockJoysticks();

    return SDL_SetError("Virtual joystick not found at provided index");
#else
    return SDL_SetError("SDL not built with virtual-joystick support");
#endif
}

SDL_bool
SDL_JoystickIsVirtual(int device_index)
{
#if SDL_JOYSTICK_VIRTUAL
    SDL_JoystickDriver *driver;
    int driver_device_index;
    SDL_bool is_virtual = SDL_FALSE;

    SDL_LockJoysticks();
    if (SDL_GetDriverAndJoystickIndex(device_index, &driver, &driver_device_index)) {
        if (driver == &SDL_VIRTUAL_JoystickDriver) {
            is_virtual = SDL_TRUE;
        }
    }
    SDL_UnlockJoysticks();

    return is_virtual;
#else
    return SDL_FALSE;
#endif
}

int
SDL_JoystickSetVirtualAxis(SDL_Joystick *joystick, int axis, Sint16 value)
{
#if SDL_JOYSTICK_VIRTUAL
    return SDL_JoystickSetVirtualAxisInner(joystick, axis, value);
#else
    return SDL_SetError("SDL not built with virtual-joystick support");
#endif
}

int
SDL_JoystickSetVirtualButton(SDL_Joystick *joystick, int button, Uint8 value)
{
#if SDL_JOYSTICK_VIRTUAL
    return SDL_JoystickSetVirtualButtonInner(joystick, button, value);
#else
    return SDL_SetError("SDL not built with virtual-joystick support");
#endif
}

int
SDL_JoystickSetVirtualHat(SDL_Joystick *joystick, int hat, Uint8 value)
{
#if SDL_JOYSTICK_VIRTUAL
    return SDL_JoystickSetVirtualHatInner(joystick, hat, value);
#else
    return SDL_SetError("SDL not built with virtual-joystick support");
#endif
}

/*
 * Checks to make sure the joystick is valid.
 */
SDL_bool
SDL_PrivateJoystickValid(SDL_Joystick *joystick)
{
    SDL_bool valid;

    if (joystick == NULL) {
        SDL_SetError("Joystick hasn't been opened yet");
        valid = SDL_FALSE;
    } else {
        valid = SDL_TRUE;
    }

    return valid;
}

SDL_bool
SDL_PrivateJoystickGetAutoGamepadMapping(int device_index, SDL_GamepadMapping * out)
{
    SDL_JoystickDriver *driver;
    SDL_bool is_ok = SDL_FALSE;

    SDL_LockJoysticks();
    if (SDL_GetDriverAndJoystickIndex(device_index, &driver, &device_index)) {
        is_ok = driver->GetGamepadMapping(device_index, out);
    }
    SDL_UnlockJoysticks();

    return is_ok;
}

/*
 * Get the number of multi-dimensional axis controls on a joystick
 */
int
SDL_JoystickNumAxes(SDL_Joystick *joystick)
{
    if (!SDL_PrivateJoystickValid(joystick)) {
        return -1;
    }
    return joystick->naxes;
}

/*
 * Get the number of hats on a joystick
 */
int
SDL_JoystickNumHats(SDL_Joystick *joystick)
{
    if (!SDL_PrivateJoystickValid(joystick)) {
        return -1;
    }
    return joystick->nhats;
}

/*
 * Get the number of trackballs on a joystick
 */
int
SDL_JoystickNumBalls(SDL_Joystick *joystick)
{
    if (!SDL_PrivateJoystickValid(joystick)) {
        return -1;
    }
    return joystick->nballs;
}

/*
 * Get the number of buttons on a joystick
 */
int
SDL_JoystickNumButtons(SDL_Joystick *joystick)
{
    if (!SDL_PrivateJoystickValid(joystick)) {
        return -1;
    }
    return joystick->nbuttons;
}

/*
 * Get the current state of an axis control on a joystick
 */
Sint16
SDL_JoystickGetAxis(SDL_Joystick *joystick, int axis)
{
    Sint16 state;

    if (!SDL_PrivateJoystickValid(joystick)) {
        return 0;
    }
    if (axis < joystick->naxes) {
        state = joystick->axes[axis].value;
    } else {
        SDL_SetError("Joystick only has %d axes", joystick->naxes);
        state = 0;
    }
    return state;
}

/*
 * Get the initial state of an axis control on a joystick
 */
SDL_bool
SDL_JoystickGetAxisInitialState(SDL_Joystick *joystick, int axis, Sint16 *state)
{
    if (!SDL_PrivateJoystickValid(joystick)) {
        return SDL_FALSE;
    }
    if (axis >= joystick->naxes) {
        SDL_SetError("Joystick only has %d axes", joystick->naxes);
        return SDL_FALSE;
    }
    if (state) {
        *state = joystick->axes[axis].initial_value;
    }
    return joystick->axes[axis].has_initial_value;
}

/*
 * Get the current state of a hat on a joystick
 */
Uint8
SDL_JoystickGetHat(SDL_Joystick *joystick, int hat)
{
    Uint8 state;

    if (!SDL_PrivateJoystickValid(joystick)) {
        return 0;
    }
    if (hat < joystick->nhats) {
        state = joystick->hats[hat];
    } else {
        SDL_SetError("Joystick only has %d hats", joystick->nhats);
        state = 0;
    }
    return state;
}

/*
 * Get the ball axis change since the last poll
 */
int
SDL_JoystickGetBall(SDL_Joystick *joystick, int ball, int *dx, int *dy)
{
    int retval;

    if (!SDL_PrivateJoystickValid(joystick)) {
        return -1;
    }

    retval = 0;
    if (ball < joystick->nballs) {
        if (dx) {
            *dx = joystick->balls[ball].dx;
        }
        if (dy) {
            *dy = joystick->balls[ball].dy;
        }
        joystick->balls[ball].dx = 0;
        joystick->balls[ball].dy = 0;
    } else {
        return SDL_SetError("Joystick only has %d balls", joystick->nballs);
    }
    return retval;
}

/*
 * Get the current state of a button on a joystick
 */
Uint8
SDL_JoystickGetButton(SDL_Joystick *joystick, int button)
{
    Uint8 state;

    if (!SDL_PrivateJoystickValid(joystick)) {
        return 0;
    }
    if (button < joystick->nbuttons) {
        state = joystick->buttons[button];
    } else {
        SDL_SetError("Joystick only has %d buttons", joystick->nbuttons);
        state = 0;
    }
    return state;
}

/*
 * Return if the joystick in question is currently attached to the system,
 *  \return SDL_FALSE if not plugged in, SDL_TRUE if still present.
 */
SDL_bool
SDL_JoystickGetAttached(SDL_Joystick *joystick)
{
    if (!SDL_PrivateJoystickValid(joystick)) {
        return SDL_FALSE;
    }

    return joystick->attached;
}

/*
 * Get the instance id for this opened joystick
 */
SDL_JoystickID
SDL_JoystickInstanceID(SDL_Joystick *joystick)
{
    if (!SDL_PrivateJoystickValid(joystick)) {
        return -1;
    }

    return joystick->instance_id;
}

/*
 * Return the SDL_Joystick associated with an instance id.
 */
SDL_Joystick *
SDL_JoystickFromInstanceID(SDL_JoystickID instance_id)
{
    SDL_Joystick *joystick;

    SDL_LockJoysticks();
    for (joystick = SDL_joysticks; joystick; joystick = joystick->next) {
        if (joystick->instance_id == instance_id) {
            break;
        }
    }
    SDL_UnlockJoysticks();
    return joystick;
}

/**
 * Return the SDL_Joystick associated with a player index.
 */
SDL_Joystick *
SDL_JoystickFromPlayerIndex(int player_index)
{
    SDL_JoystickID instance_id;
    SDL_Joystick *joystick;

    SDL_LockJoysticks();
    instance_id = SDL_GetJoystickIDForPlayerIndex(player_index);
    for (joystick = SDL_joysticks; joystick; joystick = joystick->next) {
        if (joystick->instance_id == instance_id) {
            break;
        }
    }
    SDL_UnlockJoysticks();
    return joystick;
}

/*
 * Get the friendly name of this joystick
 */
const char *
SDL_JoystickName(SDL_Joystick *joystick)
{
    if (!SDL_PrivateJoystickValid(joystick)) {
        return NULL;
    }

    return joystick->name;
}

/**
 *  Get the player index of an opened joystick, or -1 if it's not available
 */
int
SDL_JoystickGetPlayerIndex(SDL_Joystick *joystick)
{
    int player_index;

    if (!SDL_PrivateJoystickValid(joystick)) {
        return -1;
    }

    SDL_LockJoysticks();
    player_index = SDL_GetPlayerIndexForJoystickID(joystick->instance_id);
    SDL_UnlockJoysticks();

    return player_index;
}

/**
 *  Set the player index of an opened joystick
 */
void
SDL_JoystickSetPlayerIndex(SDL_Joystick *joystick, int player_index)
{
    if (!SDL_PrivateJoystickValid(joystick)) {
        return;
    }

    SDL_LockJoysticks();
    SDL_SetJoystickIDForPlayerIndex(player_index, joystick->instance_id);
    SDL_UnlockJoysticks();
}

int
SDL_JoystickRumble(SDL_Joystick *joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble, Uint32 duration_ms)
{
    int result;

    if (!SDL_PrivateJoystickValid(joystick)) {
        return -1;
    }

    SDL_LockJoysticks();
    if (low_frequency_rumble == joystick->low_frequency_rumble &&
        high_frequency_rumble == joystick->high_frequency_rumble) {
        /* Just update the expiration */
        result = 0;
    } else {
        result = joystick->driver->Rumble(joystick, low_frequency_rumble, high_frequency_rumble);
    }

    /* Save the rumble value regardless of success, so we don't spam the driver */
    joystick->low_frequency_rumble = low_frequency_rumble;
    joystick->high_frequency_rumble = high_frequency_rumble;

    if ((low_frequency_rumble || high_frequency_rumble) && duration_ms) {
        joystick->rumble_expiration = SDL_GetTicks() + SDL_min(duration_ms, SDL_MAX_RUMBLE_DURATION_MS);
        if (!joystick->rumble_expiration) {
            joystick->rumble_expiration = 1;
        }
    } else {
        joystick->rumble_expiration = 0;
    }
    SDL_UnlockJoysticks();

    return result;
}

int
SDL_JoystickRumbleTriggers(SDL_Joystick *joystick, Uint16 left_rumble, Uint16 right_rumble, Uint32 duration_ms)
{
    int result;

    if (!SDL_PrivateJoystickValid(joystick)) {
        return -1;
    }

    SDL_LockJoysticks();
    if (left_rumble == joystick->left_trigger_rumble && right_rumble == joystick->right_trigger_rumble) {
        /* Just update the expiration */
        result = 0;
    } else {
        result = joystick->driver->RumbleTriggers(joystick, left_rumble, right_rumble);
    }

    /* Save the rumble value regardless of success, so we don't spam the driver */
    joystick->left_trigger_rumble = left_rumble;
    joystick->right_trigger_rumble = right_rumble;

    if ((left_rumble || right_rumble) && duration_ms) {
        joystick->trigger_rumble_expiration = SDL_GetTicks() + SDL_min(duration_ms, SDL_MAX_RUMBLE_DURATION_MS);
        if (!joystick->trigger_rumble_expiration) {
            joystick->trigger_rumble_expiration = 1;
        }
    } else {
        joystick->trigger_rumble_expiration = 0;
    }
    SDL_UnlockJoysticks();

    return result;
}

SDL_bool
SDL_JoystickHasLED(SDL_Joystick *joystick)
{
    SDL_bool result;

    if (!SDL_PrivateJoystickValid(joystick)) {
        return SDL_FALSE;
    }

    SDL_LockJoysticks();

    result = joystick->driver->HasLED(joystick);

    SDL_UnlockJoysticks();

    return result;
}

int
SDL_JoystickSetLED(SDL_Joystick *joystick, Uint8 red, Uint8 green, Uint8 blue)
{
    int result;
    SDL_bool isfreshvalue;

    if (!SDL_PrivateJoystickValid(joystick)) {
        return -1;
    }

    SDL_LockJoysticks();

    isfreshvalue = red != joystick->led_red ||
        green != joystick->led_green ||
        blue != joystick->led_blue;

    if ( isfreshvalue || SDL_TICKS_PASSED( SDL_GetTicks(), joystick->led_expiration ) ) {
        result = joystick->driver->SetLED(joystick, red, green, blue);
        joystick->led_expiration = SDL_GetTicks() + SDL_LED_MIN_REPEAT_MS;
    }
    else {
        /* Avoid spamming the driver */
        result = 0;
    }

    /* Save the LED value regardless of success, so we don't spam the driver */
    joystick->led_red = red;
    joystick->led_green = green;
    joystick->led_blue = blue;

    SDL_UnlockJoysticks();

    return result;
}

int
SDL_JoystickSendEffect(SDL_Joystick *joystick, const void *data, int size)
{
    int result;

    if (!SDL_PrivateJoystickValid(joystick)) {
        return -1;
    }

    SDL_LockJoysticks();

    result = joystick->driver->SendEffect(joystick, data, size);

    SDL_UnlockJoysticks();

    return result;
}

/*
 * Close a joystick previously opened with SDL_JoystickOpen()
 */
void
SDL_JoystickClose(SDL_Joystick *joystick)
{
    SDL_Joystick *joysticklist;
    SDL_Joystick *joysticklistprev;
    int i;

    if (!SDL_PrivateJoystickValid(joystick)) {
        return;
    }

    SDL_LockJoysticks();

    /* First decrement ref count */
    if (--joystick->ref_count > 0) {
        SDL_UnlockJoysticks();
        return;
    }

    if (SDL_updating_joystick) {
        SDL_UnlockJoysticks();
        return;
    }

    if (joystick->rumble_expiration) {
        SDL_JoystickRumble(joystick, 0, 0, 0);
    }
    if (joystick->trigger_rumble_expiration) {
        SDL_JoystickRumbleTriggers(joystick, 0, 0, 0);
    }

    joystick->driver->Close(joystick);
    joystick->hwdata = NULL;

    joysticklist = SDL_joysticks;
    joysticklistprev = NULL;
    while (joysticklist) {
        if (joystick == joysticklist) {
            if (joysticklistprev) {
                /* unlink this entry */
                joysticklistprev->next = joysticklist->next;
            } else {
                SDL_joysticks = joystick->next;
            }
            break;
        }
        joysticklistprev = joysticklist;
        joysticklist = joysticklist->next;
    }

    SDL_free(joystick->name);
    SDL_free(joystick->serial);

    /* Free the data associated with this joystick */
    SDL_free(joystick->axes);
    SDL_free(joystick->hats);
    SDL_free(joystick->balls);
    SDL_free(joystick->buttons);
    for (i = 0; i < joystick->ntouchpads; i++) {
        SDL_JoystickTouchpadInfo *touchpad = &joystick->touchpads[i];
        SDL_free(touchpad->fingers);
    }
    SDL_free(joystick->touchpads);
    SDL_free(joystick->sensors);
    SDL_free(joystick);

    SDL_UnlockJoysticks();
}

void
SDL_JoystickQuit(void)
{
    int i;

    /* Make sure we're not getting called in the middle of updating joysticks */
    SDL_LockJoysticks();
    while (SDL_updating_joystick) {
        SDL_UnlockJoysticks();
        SDL_Delay(1);
        SDL_LockJoysticks();
    }

    /* Stop the event polling */
    while (SDL_joysticks) {
        SDL_joysticks->ref_count = 1;
        SDL_JoystickClose(SDL_joysticks);
    }

    /* Quit the joystick setup */
    for (i = 0; i < SDL_arraysize(SDL_joystick_drivers); ++i) {
       SDL_joystick_drivers[i]->Quit();
    }

    if (SDL_joystick_players) {
        SDL_free(SDL_joystick_players);
        SDL_joystick_players = NULL;
        SDL_joystick_player_count = 0;
    }
    SDL_UnlockJoysticks();

#if !SDL_EVENTS_DISABLED
    SDL_QuitSubSystem(SDL_INIT_EVENTS);
#endif

    SDL_DelHintCallback(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,
                        SDL_JoystickAllowBackgroundEventsChanged, NULL);

    if (SDL_joystick_lock) {
        SDL_mutex *mutex = SDL_joystick_lock;
        SDL_joystick_lock = NULL;
        SDL_DestroyMutex(mutex);
    }

    SDL_GameControllerQuitMappings();
}


static SDL_bool
SDL_PrivateJoystickShouldIgnoreEvent()
{
    if (SDL_joystick_allows_background_events) {
        return SDL_FALSE;
    }

    if (SDL_HasWindows() && SDL_GetKeyboardFocus() == NULL) {
        /* We have windows but we don't have focus, ignore the event. */
        return SDL_TRUE;
    }
    return SDL_FALSE;
}

/* These are global for SDL_sysjoystick.c and SDL_events.c */

void SDL_PrivateJoystickAddTouchpad(SDL_Joystick *joystick, int nfingers)
{
    int ntouchpads = joystick->ntouchpads + 1;
    SDL_JoystickTouchpadInfo *touchpads = (SDL_JoystickTouchpadInfo *)SDL_realloc(joystick->touchpads, (ntouchpads * sizeof(SDL_JoystickTouchpadInfo)));
    if (touchpads) {
        SDL_JoystickTouchpadInfo *touchpad = &touchpads[ntouchpads - 1];
        SDL_JoystickTouchpadFingerInfo *fingers = (SDL_JoystickTouchpadFingerInfo *)SDL_calloc(nfingers, sizeof(SDL_JoystickTouchpadFingerInfo));

        if (fingers) {
            touchpad->nfingers = nfingers;
            touchpad->fingers = fingers;
        } else {
            /* Out of memory, this touchpad won't be active */
            touchpad->nfingers = 0;
            touchpad->fingers = NULL;
        }

        joystick->ntouchpads = ntouchpads;
        joystick->touchpads = touchpads;
    }
}

void SDL_PrivateJoystickAddSensor(SDL_Joystick *joystick, SDL_SensorType type, float rate)
{
    int nsensors = joystick->nsensors + 1;
    SDL_JoystickSensorInfo *sensors = (SDL_JoystickSensorInfo *)SDL_realloc(joystick->sensors, (nsensors * sizeof(SDL_JoystickSensorInfo)));
    if (sensors) {
        SDL_JoystickSensorInfo *sensor = &sensors[nsensors - 1];

        SDL_zerop(sensor);
        sensor->type = type;
        sensor->rate = rate;

        joystick->nsensors = nsensors;
        joystick->sensors = sensors;
    }
}

void SDL_PrivateJoystickAdded(SDL_JoystickID device_instance)
{
    SDL_JoystickDriver *driver;
    int driver_device_index;
    int player_index = -1;
    int device_index = SDL_JoystickGetDeviceIndexFromInstanceID(device_instance);
    if (device_index < 0) {
        return;
    }

    SDL_LockJoysticks();
    if (SDL_GetDriverAndJoystickIndex(device_index, &driver, &driver_device_index)) {
        player_index = driver->GetDevicePlayerIndex(driver_device_index);
    }
    if (player_index < 0 && SDL_IsGameController(device_index)) {
        player_index = SDL_FindFreePlayerIndex();
    }
    if (player_index >= 0) {
        SDL_SetJoystickIDForPlayerIndex(player_index, device_instance);
    }
    SDL_UnlockJoysticks();

#if !SDL_EVENTS_DISABLED
    {
        SDL_Event event;

        event.type = SDL_JOYDEVICEADDED;

        if (SDL_GetEventState(event.type) == SDL_ENABLE) {
            event.jdevice.which = device_index;
            SDL_PushEvent(&event);
        }
    }
#endif /* !SDL_EVENTS_DISABLED */
}

/*
 * If there is an existing add event in the queue, it needs to be modified
 * to have the right value for which, because the number of controllers in
 * the system is now one less.
 */
static void UpdateEventsForDeviceRemoval(int device_index)
{
    int i, num_events;
    SDL_Event *events;
    SDL_bool isstack;

    num_events = SDL_PeepEvents(NULL, 0, SDL_PEEKEVENT, SDL_JOYDEVICEADDED, SDL_JOYDEVICEADDED);
    if (num_events <= 0) {
        return;
    }

    events = SDL_small_alloc(SDL_Event, num_events, &isstack);
    if (!events) {
        return;
    }

    num_events = SDL_PeepEvents(events, num_events, SDL_GETEVENT, SDL_JOYDEVICEADDED, SDL_JOYDEVICEADDED);
    for (i = 0; i < num_events; ++i) {
        if (events[i].cdevice.which < device_index) {
            /* No change for index values lower than the removed device */
        }
        else if (events[i].cdevice.which == device_index) {
            /* Drop this event entirely */
            SDL_memmove(&events[i], &events[i + 1], sizeof(*events) * (num_events - (i + 1)));
            --num_events;
            --i;
        }
        else {
            /* Fix up the device index if greater than the removed device */
            --events[i].cdevice.which;
        }
    }
    SDL_PeepEvents(events, num_events, SDL_ADDEVENT, 0, 0);

    SDL_small_free(events, isstack);
}

static void
SDL_PrivateJoystickForceRecentering(SDL_Joystick *joystick)
{
    int i, j;

    /* Tell the app that everything is centered/unpressed... */
    for (i = 0; i < joystick->naxes; i++) {
        if (joystick->axes[i].has_initial_value) {
            SDL_PrivateJoystickAxis(joystick, i, joystick->axes[i].zero);
        }
    }

    for (i = 0; i < joystick->nbuttons; i++) {
        SDL_PrivateJoystickButton(joystick, i, SDL_RELEASED);
    }

    for (i = 0; i < joystick->nhats; i++) {
        SDL_PrivateJoystickHat(joystick, i, SDL_HAT_CENTERED);
    }

    for (i = 0; i < joystick->ntouchpads; i++) {
        SDL_JoystickTouchpadInfo *touchpad = &joystick->touchpads[i];

        for (j = 0; j < touchpad->nfingers; ++j) {
            SDL_PrivateJoystickTouchpad(joystick, i, j, SDL_RELEASED, 0.0f, 0.0f, 0.0f);
        }
    }
}

void SDL_PrivateJoystickRemoved(SDL_JoystickID device_instance)
{
    SDL_Joystick *joystick = NULL;
    int player_index;
    int device_index;
#if !SDL_EVENTS_DISABLED
    SDL_Event event;
#endif

    /* Find this joystick... */
    device_index = 0;
    for (joystick = SDL_joysticks; joystick; joystick = joystick->next) {
        if (joystick->instance_id == device_instance) {
            SDL_PrivateJoystickForceRecentering(joystick);
            joystick->attached = SDL_FALSE;
            break;
        }

        ++device_index;
    }

#if !SDL_EVENTS_DISABLED
    SDL_zero(event);
    event.type = SDL_JOYDEVICEREMOVED;

    if (SDL_GetEventState(event.type) == SDL_ENABLE) {
        event.jdevice.which = device_instance;
        SDL_PushEvent(&event);
    }

    UpdateEventsForDeviceRemoval(device_index);
#endif /* !SDL_EVENTS_DISABLED */

    SDL_LockJoysticks();
    player_index = SDL_GetPlayerIndexForJoystickID(device_instance);
    if (player_index >= 0) {
        SDL_joystick_players[player_index] = -1;
    }
    SDL_UnlockJoysticks();
}

int
SDL_PrivateJoystickAxis(SDL_Joystick *joystick, Uint8 axis, Sint16 value)
{
    int posted;
    SDL_JoystickAxisInfo *info;

    /* Make sure we're not getting garbage or duplicate events */
    if (axis >= joystick->naxes) {
        return 0;
    }

    info = &joystick->axes[axis];
    if (!info->has_initial_value ||
        (!info->has_second_value && (info->initial_value <= -32767 || info->initial_value == 32767) && SDL_abs(value) < (SDL_JOYSTICK_AXIS_MAX / 4))) {
        info->initial_value = value;
        info->value = value;
        info->zero = value;
        info->has_initial_value = SDL_TRUE;
    } else if (value == info->value) {
        return 0;
    } else {
        info->has_second_value = SDL_TRUE;
    }
    if (!info->sent_initial_value) {
        /* Make sure we don't send motion until there's real activity on this axis */
        const int MAX_ALLOWED_JITTER = SDL_JOYSTICK_AXIS_MAX / 80;  /* ShanWan PS3 controller needed 96 */
        if (SDL_abs(value - info->value) <= MAX_ALLOWED_JITTER) {
            return 0;
        }
        info->sent_initial_value = SDL_TRUE;
        info->value = ~value; /* Just so we pass the check above */
        SDL_PrivateJoystickAxis(joystick, axis, info->initial_value);
    }

    /* We ignore events if we don't have keyboard focus, except for centering
     * events.
     */
    if (SDL_PrivateJoystickShouldIgnoreEvent()) {
        if ((value > info->zero && value >= info->value) ||
            (value < info->zero && value <= info->value)) {
            return 0;
        }
    }

    /* Update internal joystick state */
    info->value = value;

    /* Post the event, if desired */
    posted = 0;
#if !SDL_EVENTS_DISABLED
    if (SDL_GetEventState(SDL_JOYAXISMOTION) == SDL_ENABLE) {
        SDL_Event event;
        event.type = SDL_JOYAXISMOTION;
        event.jaxis.which = joystick->instance_id;
        event.jaxis.axis = axis;
        event.jaxis.value = value;
        posted = SDL_PushEvent(&event) == 1;
    }
#endif /* !SDL_EVENTS_DISABLED */
    return posted;
}

int
SDL_PrivateJoystickHat(SDL_Joystick *joystick, Uint8 hat, Uint8 value)
{
    int posted;

    /* Make sure we're not getting garbage or duplicate events */
    if (hat >= joystick->nhats) {
        return 0;
    }
    if (value == joystick->hats[hat]) {
        return 0;
    }

    /* We ignore events if we don't have keyboard focus, except for centering
     * events.
     */
    if (SDL_PrivateJoystickShouldIgnoreEvent()) {
        if (value != SDL_HAT_CENTERED) {
            return 0;
        }
    }

    /* Update internal joystick state */
    joystick->hats[hat] = value;

    /* Post the event, if desired */
    posted = 0;
#if !SDL_EVENTS_DISABLED
    if (SDL_GetEventState(SDL_JOYHATMOTION) == SDL_ENABLE) {
        SDL_Event event;
        event.jhat.type = SDL_JOYHATMOTION;
        event.jhat.which = joystick->instance_id;
        event.jhat.hat = hat;
        event.jhat.value = value;
        posted = SDL_PushEvent(&event) == 1;
    }
#endif /* !SDL_EVENTS_DISABLED */
    return posted;
}

int
SDL_PrivateJoystickBall(SDL_Joystick *joystick, Uint8 ball,
                        Sint16 xrel, Sint16 yrel)
{
    int posted;

    /* Make sure we're not getting garbage events */
    if (ball >= joystick->nballs) {
        return 0;
    }

    /* We ignore events if we don't have keyboard focus. */
    if (SDL_PrivateJoystickShouldIgnoreEvent()) {
        return 0;
    }

    /* Update internal mouse state */
    joystick->balls[ball].dx += xrel;
    joystick->balls[ball].dy += yrel;

    /* Post the event, if desired */
    posted = 0;
#if !SDL_EVENTS_DISABLED
    if (SDL_GetEventState(SDL_JOYBALLMOTION) == SDL_ENABLE) {
        SDL_Event event;
        event.jball.type = SDL_JOYBALLMOTION;
        event.jball.which = joystick->instance_id;
        event.jball.ball = ball;
        event.jball.xrel = xrel;
        event.jball.yrel = yrel;
        posted = SDL_PushEvent(&event) == 1;
    }
#endif /* !SDL_EVENTS_DISABLED */
    return posted;
}

int
SDL_PrivateJoystickButton(SDL_Joystick *joystick, Uint8 button, Uint8 state)
{
    int posted;
#if !SDL_EVENTS_DISABLED
    SDL_Event event;

    switch (state) {
    case SDL_PRESSED:
        event.type = SDL_JOYBUTTONDOWN;
        break;
    case SDL_RELEASED:
        event.type = SDL_JOYBUTTONUP;
        break;
    default:
        /* Invalid state -- bail */
        return 0;
    }
#endif /* !SDL_EVENTS_DISABLED */

    /* Make sure we're not getting garbage or duplicate events */
    if (button >= joystick->nbuttons) {
        return 0;
    }
    if (state == joystick->buttons[button]) {
        return 0;
    }

    /* We ignore events if we don't have keyboard focus, except for button
     * release. */
    if (SDL_PrivateJoystickShouldIgnoreEvent()) {
        if (state == SDL_PRESSED) {
            return 0;
        }
    }

    /* Update internal joystick state */
    joystick->buttons[button] = state;

    /* Post the event, if desired */
    posted = 0;
#if !SDL_EVENTS_DISABLED
    if (SDL_GetEventState(event.type) == SDL_ENABLE) {
        event.jbutton.which = joystick->instance_id;
        event.jbutton.button = button;
        event.jbutton.state = state;
        posted = SDL_PushEvent(&event) == 1;
    }
#endif /* !SDL_EVENTS_DISABLED */
    return posted;
}

void
SDL_JoystickUpdate(void)
{
    int i;
    SDL_Joystick *joystick, *next;

    if (!SDL_WasInit(SDL_INIT_JOYSTICK)) {
        return;
    }

    SDL_LockJoysticks();

    if (SDL_updating_joystick) {
        /* The joysticks are already being updated */
        SDL_UnlockJoysticks();
        return;
    }

    SDL_updating_joystick = SDL_TRUE;

    /* Make sure the list is unlocked while dispatching events to prevent application deadlocks */
    SDL_UnlockJoysticks();

#ifdef SDL_JOYSTICK_HIDAPI
    /* Special function for HIDAPI devices, as a single device can provide multiple SDL_Joysticks */
    HIDAPI_UpdateDevices();
#endif /* SDL_JOYSTICK_HIDAPI */

    for (joystick = SDL_joysticks; joystick; joystick = joystick->next) {
        if (joystick->attached) {
            /* This should always be true, but seeing a crash in the wild...? */
            if (joystick->driver) {
                joystick->driver->Update(joystick);
            }

            if (joystick->delayed_guide_button) {
                SDL_GameControllerHandleDelayedGuideButton(joystick);
            }
        }

        if (joystick->rumble_expiration) {
            SDL_LockJoysticks();
            /* Double check now that the lock is held */
            if (joystick->rumble_expiration &&
                SDL_TICKS_PASSED(SDL_GetTicks(), joystick->rumble_expiration)) {
                SDL_JoystickRumble(joystick, 0, 0, 0);
            }
            SDL_UnlockJoysticks();
        }

        if (joystick->trigger_rumble_expiration) {
            SDL_LockJoysticks();
            /* Double check now that the lock is held */
            if (joystick->trigger_rumble_expiration &&
                SDL_TICKS_PASSED(SDL_GetTicks(), joystick->trigger_rumble_expiration)) {
                SDL_JoystickRumbleTriggers(joystick, 0, 0, 0);
            }
            SDL_UnlockJoysticks();
        }
    }

    SDL_LockJoysticks();

    SDL_updating_joystick = SDL_FALSE;

    /* If any joysticks were closed while updating, free them here */
    for (joystick = SDL_joysticks; joystick; joystick = next) {
        next = joystick->next;
        if (joystick->ref_count <= 0) {
            SDL_JoystickClose(joystick);
        }
    }

    /* this needs to happen AFTER walking the joystick list above, so that any
       dangling hardware data from removed devices can be free'd
     */
    for (i = 0; i < SDL_arraysize(SDL_joystick_drivers); ++i) {
        SDL_joystick_drivers[i]->Detect();
    }

    SDL_UnlockJoysticks();
}

int
SDL_JoystickEventState(int state)
{
#if SDL_EVENTS_DISABLED
    return SDL_DISABLE;
#else
    const Uint32 event_list[] = {
        SDL_JOYAXISMOTION, SDL_JOYBALLMOTION, SDL_JOYHATMOTION,
        SDL_JOYBUTTONDOWN, SDL_JOYBUTTONUP, SDL_JOYDEVICEADDED, SDL_JOYDEVICEREMOVED
    };
    unsigned int i;

    switch (state) {
    case SDL_QUERY:
        state = SDL_DISABLE;
        for (i = 0; i < SDL_arraysize(event_list); ++i) {
            state = SDL_EventState(event_list[i], SDL_QUERY);
            if (state == SDL_ENABLE) {
                break;
            }
        }
        break;
    default:
        for (i = 0; i < SDL_arraysize(event_list); ++i) {
            SDL_EventState(event_list[i], state);
        }
        break;
    }
    return state;
#endif /* SDL_EVENTS_DISABLED */
}

void SDL_GetJoystickGUIDInfo(SDL_JoystickGUID guid, Uint16 *vendor, Uint16 *product, Uint16 *version)
{
    Uint16 *guid16 = (Uint16 *)guid.data;

    /* If the GUID fits the form of BUS 0000 VENDOR 0000 PRODUCT 0000, return the data */
    if (/* guid16[0] is device bus type */
        guid16[1] == 0x0000 &&
        /* guid16[2] is vendor ID */
        guid16[3] == 0x0000 &&
        /* guid16[4] is product ID */
        guid16[5] == 0x0000
        /* guid16[6] is product version */
   ) {
        if (vendor) {
            *vendor = guid16[2];
        }
        if (product) {
            *product = guid16[4];
        }
        if (version) {
            *version = guid16[6];
        }
    } else {
        if (vendor) {
            *vendor = 0;
        }
        if (product) {
            *product = 0;
        }
        if (version) {
            *version = 0;
        }
    }
}

static int
PrefixMatch(const char *a, const char *b)
{
    int matchlen = 0;
    while (*a && *b) {
        if (SDL_tolower(*a++) == SDL_tolower(*b++)) {
            ++matchlen;
        } else {
            break;
        }
    }
    return matchlen;
}

char *
SDL_CreateJoystickName(Uint16 vendor, Uint16 product, const char *vendor_name, const char *product_name)
{
    static struct {
        const char *prefix;
        const char *replacement;
    } replacements[] = {
        { "NVIDIA Corporation ", "" },
        { "Performance Designed Products", "PDP" },
        { "HORI CO.,LTD.", "HORI" },
        { "HORI CO.,LTD", "HORI" },
        { "Unknown ", "" },
    };
    const char *custom_name;
    char *name;
    size_t i, len;

    custom_name = GuessControllerName(vendor, product);
    if (custom_name) {
        return SDL_strdup(custom_name);
    }

    if (!vendor_name) {
        vendor_name = "";
    }
    if (!product_name) {
        product_name = "";
    }

    while (*vendor_name == ' ') {
        ++vendor_name;
    }
    while (*product_name == ' ') {
        ++product_name;
    }

    if (*vendor_name && *product_name) {
        len = (SDL_strlen(vendor_name) + 1 + SDL_strlen(product_name) + 1);
        name = (char *)SDL_malloc(len);
        if (!name) {
            return NULL;
        }
        SDL_snprintf(name, len, "%s %s", vendor_name, product_name);
    } else if (*product_name) {
        name = SDL_strdup(product_name);
    } else if (vendor || product) {
        len = (6 + 1 + 6 + 1);
        name = (char *)SDL_malloc(len);
        if (!name) {
            return NULL;
        }
        SDL_snprintf(name, len, "0x%.4x/0x%.4x", vendor, product);
    } else {
        name = SDL_strdup("Controller");
    }

    /* Trim trailing whitespace */
    for (len = SDL_strlen(name); (len > 0 && name[len - 1] == ' '); --len) {
        /* continue */
    }
    name[len] = '\0';

    /* Compress duplicate spaces */
    for (i = 0; i < (len - 1); ) {
        if (name[i] == ' ' && name[i+1] == ' ') {
            SDL_memmove(&name[i], &name[i+1], (len - i));
            --len;
        } else {
            ++i;
        }
    }

    /* Perform any manufacturer replacements */
    for (i = 0; i < SDL_arraysize(replacements); ++i) {
        size_t prefixlen = SDL_strlen(replacements[i].prefix);
        if (SDL_strncasecmp(name, replacements[i].prefix, prefixlen) == 0) {
            size_t replacementlen = SDL_strlen(replacements[i].replacement);
            SDL_memcpy(name, replacements[i].replacement, replacementlen);
            SDL_memmove(name+replacementlen, name+prefixlen, (len-prefixlen+1));
            break;
        }
    }

    /* Remove duplicate manufacturer or product in the name */
    for (i = 1; i < (len - 1); ++i) {
        int matchlen = PrefixMatch(name, &name[i]);
        if (matchlen > 0 && name[matchlen-1] == ' ') {
            SDL_memmove(name, name+matchlen, len-matchlen+1);
            len -= matchlen;
            break;
        } else if (matchlen > 0 && name[matchlen] == ' ') {
            SDL_memmove(name, name+matchlen+1, len-matchlen);
            len -= (matchlen + 1);
            break;
        }
    }

    return name;
}

SDL_GameControllerType
SDL_GetJoystickGameControllerTypeFromVIDPID(Uint16 vendor, Uint16 product)
{
    return SDL_GetJoystickGameControllerType(NULL, vendor, product, -1, 0, 0, 0);
}

SDL_GameControllerType
SDL_GetJoystickGameControllerTypeFromGUID(SDL_JoystickGUID guid, const char *name)
{
    SDL_GameControllerType type;
    Uint16 vendor, product;

    SDL_GetJoystickGUIDInfo(guid, &vendor, &product, NULL);
    type = SDL_GetJoystickGameControllerType(name, vendor, product, -1, 0, 0, 0);
    if (type == SDL_CONTROLLER_TYPE_UNKNOWN) {
        if (SDL_IsJoystickXInput(guid)) {
            /* This is probably an Xbox One controller */
            return SDL_CONTROLLER_TYPE_XBOXONE;
        }
    }
    return type;
}

SDL_GameControllerType
SDL_GetJoystickGameControllerType(const char *name, Uint16 vendor, Uint16 product, int interface_number, int interface_class, int interface_subclass, int interface_protocol)
{
    static const int LIBUSB_CLASS_VENDOR_SPEC = 0xFF;
    static const int XB360_IFACE_SUBCLASS = 93;
    static const int XB360_IFACE_PROTOCOL = 1; /* Wired */
    static const int XB360W_IFACE_PROTOCOL = 129; /* Wireless */
    static const int XBONE_IFACE_SUBCLASS = 71;
    static const int XBONE_IFACE_PROTOCOL = 208;

    SDL_GameControllerType type = SDL_CONTROLLER_TYPE_UNKNOWN;

    /* This code should match the checks in libusb/hid.c and HIDDeviceManager.java */
    if (interface_class == LIBUSB_CLASS_VENDOR_SPEC &&
        interface_subclass == XB360_IFACE_SUBCLASS &&
        (interface_protocol == XB360_IFACE_PROTOCOL ||
         interface_protocol == XB360W_IFACE_PROTOCOL)) {

        static const int SUPPORTED_VENDORS[] = {
            0x0079, /* GPD Win 2 */
            0x044f, /* Thrustmaster */
            0x045e, /* Microsoft */
            0x046d, /* Logitech */
            0x056e, /* Elecom */
            0x06a3, /* Saitek */
            0x0738, /* Mad Catz */
            0x07ff, /* Mad Catz */
            0x0e6f, /* PDP */
            0x0f0d, /* Hori */
            0x1038, /* SteelSeries */
            0x11c9, /* Nacon */
            0x12ab, /* Unknown */
            0x1430, /* RedOctane */
            0x146b, /* BigBen */
            0x1532, /* Razer Sabertooth */
            0x15e4, /* Numark */
            0x162e, /* Joytech */
            0x1689, /* Razer Onza */
            0x1949, /* Lab126, Inc. */
            0x1bad, /* Harmonix */
            0x20d6, /* PowerA */
            0x24c6, /* PowerA */
        };

        int i;
        for (i = 0; i < SDL_arraysize(SUPPORTED_VENDORS); ++i) {
            if (vendor == SUPPORTED_VENDORS[i]) {
                type = SDL_CONTROLLER_TYPE_XBOX360;
                break;
            }
        }
    }

    if (interface_number == 0 &&
        interface_class == LIBUSB_CLASS_VENDOR_SPEC &&
        interface_subclass == XBONE_IFACE_SUBCLASS &&
        interface_protocol == XBONE_IFACE_PROTOCOL) {

        static const int SUPPORTED_VENDORS[] = {
            0x045e, /* Microsoft */
            0x0738, /* Mad Catz */
            0x0e6f, /* PDP */
            0x0f0d, /* Hori */
            0x1532, /* Razer Wildcat */
            0x20d6, /* PowerA */
            0x24c6, /* PowerA */
            0x2e24, /* Hyperkin */
        };

        int i;
        for (i = 0; i < SDL_arraysize(SUPPORTED_VENDORS); ++i) {
            if (vendor == SUPPORTED_VENDORS[i]) {
                type = SDL_CONTROLLER_TYPE_XBOXONE;
                break;
            }
        }
    }

    if (type == SDL_CONTROLLER_TYPE_UNKNOWN) {
        if (vendor == 0x0000 && product == 0x0000) {
            /* Some devices are only identifiable by their name */
            if (name &&
                (SDL_strcmp(name, "Lic Pro Controller") == 0 ||
                 SDL_strcmp(name, "Nintendo Wireless Gamepad") == 0 ||
                 SDL_strcmp(name, "Wireless Gamepad") == 0)) {
                /* HORI or PowerA Switch Pro Controller clone */
                type = SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO;
            } else if (name && SDL_strcmp(name, "Virtual Joystick") == 0) {
                type = SDL_CONTROLLER_TYPE_VIRTUAL;
            } else {
                type = SDL_CONTROLLER_TYPE_UNKNOWN;
            }

        } else if (vendor == 0x0001 && product == 0x0001) {
            type = SDL_CONTROLLER_TYPE_UNKNOWN;

        } else if ((vendor == USB_VENDOR_AMAZON && product == USB_PRODUCT_AMAZON_LUNA_CONTROLLER) ||
                   (vendor == BLUETOOTH_VENDOR_AMAZON && product == BLUETOOTH_PRODUCT_LUNA_CONTROLLER)) {
            type = SDL_CONTROLLER_TYPE_AMAZON_LUNA;

        } else if (vendor == USB_VENDOR_GOOGLE && product == USB_PRODUCT_GOOGLE_STADIA_CONTROLLER) {
            type = SDL_CONTROLLER_TYPE_GOOGLE_STADIA;

        } else if (vendor == USB_VENDOR_NINTENDO && product == USB_PRODUCT_NINTENDO_SWITCH_JOY_CON_GRIP) {
                type = SDL_GetHintBoolean(SDL_HINT_JOYSTICK_HIDAPI_JOY_CONS, SDL_FALSE) ? SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO : SDL_CONTROLLER_TYPE_UNKNOWN;

        } else {
            switch (GuessControllerType(vendor, product)) {
            case k_eControllerType_XBox360Controller:
                type = SDL_CONTROLLER_TYPE_XBOX360;
                break;
            case k_eControllerType_XBoxOneController:
                type = SDL_CONTROLLER_TYPE_XBOXONE;
                break;
            case k_eControllerType_PS3Controller:
                type = SDL_CONTROLLER_TYPE_PS3;
                break;
            case k_eControllerType_PS4Controller:
                type = SDL_CONTROLLER_TYPE_PS4;
                break;
            case k_eControllerType_PS5Controller:
                type = SDL_CONTROLLER_TYPE_PS5;
                break;
            case k_eControllerType_SwitchProController:
            case k_eControllerType_SwitchInputOnlyController:
                type = SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO;
                break;
            case k_eControllerType_SwitchJoyConLeft:
            case k_eControllerType_SwitchJoyConRight:
                type = SDL_GetHintBoolean(SDL_HINT_JOYSTICK_HIDAPI_JOY_CONS, SDL_FALSE) ? SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO : SDL_CONTROLLER_TYPE_UNKNOWN;
                break;
            default:
                type = SDL_CONTROLLER_TYPE_UNKNOWN;
                break;
            }
        }
    }
    return type;
}

SDL_bool
SDL_IsJoystickXboxOneElite(Uint16 vendor_id, Uint16 product_id)
{
    if (vendor_id == USB_VENDOR_MICROSOFT) {
        if (product_id == USB_PRODUCT_XBOX_ONE_ELITE_SERIES_1 ||
            product_id == USB_PRODUCT_XBOX_ONE_ELITE_SERIES_2 ||
            product_id == USB_PRODUCT_XBOX_ONE_ELITE_SERIES_2_BLUETOOTH) {
            return SDL_TRUE;
        }
    }
    return SDL_FALSE;
}

SDL_bool
SDL_IsJoystickXboxSeriesX(Uint16 vendor_id, Uint16 product_id)
{
    if (vendor_id == USB_VENDOR_MICROSOFT) {
        if (product_id == USB_PRODUCT_XBOX_SERIES_X ||
            product_id == USB_PRODUCT_XBOX_SERIES_X_BLUETOOTH) {
            return SDL_TRUE;
        }
    }
    if (vendor_id == USB_VENDOR_PDP) {
        if (product_id == USB_PRODUCT_XBOX_SERIES_X_VICTRIX_GAMBIT ||
            product_id == USB_PRODUCT_XBOX_SERIES_X_PDP_BLUE ||
            product_id == USB_PRODUCT_XBOX_SERIES_X_PDP_AFTERGLOW) {
            return SDL_TRUE;
        }
    }
    if (vendor_id == USB_VENDOR_POWERA_ALT) {
        if ((product_id >= 0x2001 && product_id <= 0x201a) ||
            product_id == USB_PRODUCT_XBOX_SERIES_X_POWERA_FUSION_PRO2 ||
            product_id == USB_PRODUCT_XBOX_SERIES_X_POWERA_SPECTRA) {
            return SDL_TRUE;
        }
    }
    return SDL_FALSE;
}

SDL_bool
SDL_IsJoystickBluetoothXboxOne(Uint16 vendor_id, Uint16 product_id)
{
    if (vendor_id == USB_VENDOR_MICROSOFT) {
        if (product_id == USB_PRODUCT_XBOX_ONE_S_REV1_BLUETOOTH ||
            product_id == USB_PRODUCT_XBOX_ONE_S_REV2_BLUETOOTH ||
            product_id == USB_PRODUCT_XBOX_ONE_ELITE_SERIES_2_BLUETOOTH ||
            product_id == USB_PRODUCT_XBOX_SERIES_X_BLUETOOTH) {
            return SDL_TRUE;
        }
    }
    return SDL_FALSE;
}

SDL_bool
SDL_IsJoystickPS4(Uint16 vendor_id, Uint16 product_id)
{
    EControllerType eType = GuessControllerType(vendor_id, product_id);
    return (eType == k_eControllerType_PS4Controller);
}

SDL_bool
SDL_IsJoystickPS5(Uint16 vendor_id, Uint16 product_id)
{
    EControllerType eType = GuessControllerType(vendor_id, product_id);
    return (eType == k_eControllerType_PS5Controller);
}

SDL_bool
SDL_IsJoystickNintendoSwitchPro(Uint16 vendor_id, Uint16 product_id)
{
    EControllerType eType = GuessControllerType(vendor_id, product_id);
    return (eType == k_eControllerType_SwitchProController ||
            eType == k_eControllerType_SwitchInputOnlyController ||
            (vendor_id == USB_VENDOR_NINTENDO && product_id == USB_PRODUCT_NINTENDO_SWITCH_JOY_CON_GRIP));
}

SDL_bool
SDL_IsJoystickNintendoSwitchProInputOnly(Uint16 vendor_id, Uint16 product_id)
{
    EControllerType eType = GuessControllerType(vendor_id, product_id);
    return (eType == k_eControllerType_SwitchInputOnlyController);
}

SDL_bool
SDL_IsJoystickNintendoSwitchJoyCon(Uint16 vendor_id, Uint16 product_id)
{
    EControllerType eType = GuessControllerType(vendor_id, product_id);
    return (eType == k_eControllerType_SwitchJoyConLeft ||
            eType == k_eControllerType_SwitchJoyConRight);
}

SDL_bool
SDL_IsJoystickNintendoSwitchJoyConLeft(Uint16 vendor_id, Uint16 product_id)
{
    EControllerType eType = GuessControllerType(vendor_id, product_id);
    return (eType == k_eControllerType_SwitchJoyConLeft);
}

SDL_bool
SDL_IsJoystickNintendoSwitchJoyConRight(Uint16 vendor_id, Uint16 product_id)
{
    EControllerType eType = GuessControllerType(vendor_id, product_id);
    return (eType == k_eControllerType_SwitchJoyConRight);
}

SDL_bool
SDL_IsJoystickSteamController(Uint16 vendor_id, Uint16 product_id)
{
    EControllerType eType = GuessControllerType(vendor_id, product_id);
    return (eType == k_eControllerType_SteamController ||
            eType == k_eControllerType_SteamControllerV2);
}

SDL_bool
SDL_IsJoystickXInput(SDL_JoystickGUID guid)
{
    return (guid.data[14] == 'x') ? SDL_TRUE : SDL_FALSE;
}

SDL_bool
SDL_IsJoystickWGI(SDL_JoystickGUID guid)
{
    return (guid.data[14] == 'w') ? SDL_TRUE : SDL_FALSE;
}

SDL_bool
SDL_IsJoystickHIDAPI(SDL_JoystickGUID guid)
{
    return (guid.data[14] == 'h') ? SDL_TRUE : SDL_FALSE;
}

SDL_bool
SDL_IsJoystickRAWINPUT(SDL_JoystickGUID guid)
{
    return (guid.data[14] == 'r') ? SDL_TRUE : SDL_FALSE;
}

SDL_bool
SDL_IsJoystickVirtual(SDL_JoystickGUID guid)
{
    return (guid.data[14] == 'v') ? SDL_TRUE : SDL_FALSE;
}

static SDL_bool SDL_IsJoystickProductWheel(Uint32 vidpid)
{
    static Uint32 wheel_joysticks[] = {
        MAKE_VIDPID(0x046d, 0xc294),    /* Logitech generic wheel */
        MAKE_VIDPID(0x046d, 0xc295),    /* Logitech Momo Force */
        MAKE_VIDPID(0x046d, 0xc298),    /* Logitech Driving Force Pro */
        MAKE_VIDPID(0x046d, 0xc299),    /* Logitech G25 */
        MAKE_VIDPID(0x046d, 0xc29a),    /* Logitech Driving Force GT */
        MAKE_VIDPID(0x046d, 0xc29b),    /* Logitech G27 */
        MAKE_VIDPID(0x046d, 0xc24f),    /* Logitech G29 (PS3) */
        MAKE_VIDPID(0x046d, 0xc260),    /* Logitech G29 (PS4) */
        MAKE_VIDPID(0x046d, 0xc261),    /* Logitech G920 (initial mode) */
        MAKE_VIDPID(0x046d, 0xc262),    /* Logitech G920 (active mode) */
        MAKE_VIDPID(0x046d, 0xc26e),    /* Logitech G923 */
        MAKE_VIDPID(0x044f, 0xb65d),    /* Thrustmaster Wheel FFB */
        MAKE_VIDPID(0x044f, 0xb66d),    /* Thrustmaster Wheel FFB */
        MAKE_VIDPID(0x044f, 0xb677),    /* Thrustmaster T150 */
        MAKE_VIDPID(0x044f, 0xb66e),    /* Thrustmaster T300RS */
        MAKE_VIDPID(0x044f, 0xb65e),    /* Thrustmaster T500RS */
        MAKE_VIDPID(0x044f, 0xb664),    /* Thrustmaster TX (initial mode) */
        MAKE_VIDPID(0x044f, 0xb669),    /* Thrustmaster TX (active mode) */
    };
    int i;

    for (i = 0; i < SDL_arraysize(wheel_joysticks); ++i) {
        if (vidpid == wheel_joysticks[i]) {
            return SDL_TRUE;
        }
    }
    return SDL_FALSE;
}

static SDL_bool SDL_IsJoystickProductArcadeStick(Uint32 vidpid)
{
    static Uint32 arcadestick_joysticks[] = {
        MAKE_VIDPID(0x0079, 0x181a),    /* Venom Arcade Stick */
        MAKE_VIDPID(0x0f0d, 0x006a),    /* Real Arcade Pro 4 */
        MAKE_VIDPID(0x0079, 0x181b),    /* Venom Arcade Stick */
        MAKE_VIDPID(0x0c12, 0x0ef6),    /* Hitbox Arcade Stick */
        MAKE_VIDPID(0x0f0d, 0x008a),    /* HORI Real Arcade Pro 4 */
        MAKE_VIDPID(0x0f0d, 0x0016),    /* Hori Real Arcade Pro.EX */
        MAKE_VIDPID(0x0f0d, 0x001b),    /* Hori Real Arcade Pro VX */
        MAKE_VIDPID(0x0f0d, 0x008c),    /* Hori Real Arcade Pro 4 */
        MAKE_VIDPID(0x1bad, 0xf03d),    /* Street Fighter IV Arcade Stick TE - Chun Li */
        MAKE_VIDPID(0x1bad, 0xf502),    /* Hori Real Arcade Pro.VX SA */
        MAKE_VIDPID(0x1bad, 0xf504),    /* Hori Real Arcade Pro. EX */
        MAKE_VIDPID(0x1bad, 0xf506),    /* Hori Real Arcade Pro.EX Premium VLX */
        MAKE_VIDPID(0x24c6, 0x5000),    /* Razer Atrox Arcade Stick */
        MAKE_VIDPID(0x24c6, 0x5501),    /* Hori Real Arcade Pro VX-SA */
        MAKE_VIDPID(0x24c6, 0x550e),    /* Hori Real Arcade Pro V Kai 360 */
        MAKE_VIDPID(0x0f0d, 0x0063),    /* Hori Real Arcade Pro Hayabusa (USA) Xbox One */
        MAKE_VIDPID(0x0f0d, 0x0078),    /* Hori Real Arcade Pro V Kai Xbox One */
        MAKE_VIDPID(0x1532, 0x0a00),    /* Razer Atrox Arcade Stick */
        MAKE_VIDPID(0x0f0d, 0x00aa),    /* HORI Real Arcade Pro V Hayabusa in Switch Mode */
        MAKE_VIDPID(0x20d6, 0xa715),    /* PowerA Nintendo Switch Fusion Arcade Stick */
    };
    int i;

    for (i = 0; i < SDL_arraysize(arcadestick_joysticks); ++i) {
        if (vidpid == arcadestick_joysticks[i]) {
            return SDL_TRUE;
        }
    }
    return SDL_FALSE;
}

static SDL_bool SDL_IsJoystickProductFlightStick(Uint32 vidpid)
{
    static Uint32 flightstick_joysticks[] = {
        MAKE_VIDPID(0x044f, 0x0402),    /* HOTAS Warthog Joystick */
        MAKE_VIDPID(0x0738, 0x2221),    /* Saitek Pro Flight X-56 Rhino Stick */
        MAKE_VIDPID(0x044f, 0xb10a),    /* ThrustMaster, Inc. T.16000M Joystick */
    };
    int i;

    for (i = 0; i < SDL_arraysize(flightstick_joysticks); ++i) {
        if (vidpid == flightstick_joysticks[i]) {
            return SDL_TRUE;
        }
    }
    return SDL_FALSE;
}

static SDL_bool SDL_IsJoystickProductThrottle(Uint32 vidpid)
{
    static Uint32 throttle_joysticks[] = {
        MAKE_VIDPID(0x044f, 0x0404),    /* HOTAS Warthog Throttle */
        MAKE_VIDPID(0x0738, 0xa221),    /* Saitek Pro Flight X-56 Rhino Throttle */
    };
    int i;

    for (i = 0; i < SDL_arraysize(throttle_joysticks); ++i) {
        if (vidpid == throttle_joysticks[i]) {
            return SDL_TRUE;
        }
    }
    return SDL_FALSE;
}

static SDL_JoystickType SDL_GetJoystickGUIDType(SDL_JoystickGUID guid)
{
    Uint16 vendor;
    Uint16 product;
    Uint32 vidpid;

    if (SDL_IsJoystickXInput(guid)) {
        /* XInput GUID, get the type based on the XInput device subtype */
        switch (guid.data[15]) {
        case 0x01:  /* XINPUT_DEVSUBTYPE_GAMEPAD */
            return SDL_JOYSTICK_TYPE_GAMECONTROLLER;
        case 0x02:  /* XINPUT_DEVSUBTYPE_WHEEL */
            return SDL_JOYSTICK_TYPE_WHEEL;
        case 0x03:  /* XINPUT_DEVSUBTYPE_ARCADE_STICK */
            return SDL_JOYSTICK_TYPE_ARCADE_STICK;
        case 0x04:  /* XINPUT_DEVSUBTYPE_FLIGHT_STICK */
            return SDL_JOYSTICK_TYPE_FLIGHT_STICK;
        case 0x05:  /* XINPUT_DEVSUBTYPE_DANCE_PAD */
            return SDL_JOYSTICK_TYPE_DANCE_PAD;
        case 0x06:  /* XINPUT_DEVSUBTYPE_GUITAR */
        case 0x07:  /* XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE */
        case 0x0B:  /* XINPUT_DEVSUBTYPE_GUITAR_BASS */
            return SDL_JOYSTICK_TYPE_GUITAR;
        case 0x08:  /* XINPUT_DEVSUBTYPE_DRUM_KIT */
            return SDL_JOYSTICK_TYPE_DRUM_KIT;
        case 0x13:  /* XINPUT_DEVSUBTYPE_ARCADE_PAD */
            return SDL_JOYSTICK_TYPE_ARCADE_PAD;
        default:
            return SDL_JOYSTICK_TYPE_UNKNOWN;
        }
    }

    if (SDL_IsJoystickWGI(guid)) {
        return (SDL_JoystickType)guid.data[15];
    }

    if (SDL_IsJoystickVirtual(guid)) {
        return (SDL_JoystickType)guid.data[15];
    }

    SDL_GetJoystickGUIDInfo(guid, &vendor, &product, NULL);
    vidpid = MAKE_VIDPID(vendor, product);

    if (SDL_IsJoystickProductWheel(vidpid)) {
        return SDL_JOYSTICK_TYPE_WHEEL;
    }

    if (SDL_IsJoystickProductArcadeStick(vidpid)) {
        return SDL_JOYSTICK_TYPE_ARCADE_STICK;
    }

    if (SDL_IsJoystickProductFlightStick(vidpid)) {
        return SDL_JOYSTICK_TYPE_FLIGHT_STICK;
    }

    if (SDL_IsJoystickProductThrottle(vidpid)) {
        return SDL_JOYSTICK_TYPE_THROTTLE;
    }

    if (GuessControllerType(vendor, product) != k_eControllerType_UnknownNonSteamController) {
        return SDL_JOYSTICK_TYPE_GAMECONTROLLER;
    }

    return SDL_JOYSTICK_TYPE_UNKNOWN;
}

static SDL_bool SDL_IsPS4RemapperRunning(void)
{
#ifdef __WIN32__
    const char *mapper_processes[] = {
        "DS4Windows.exe",
        "InputMapper.exe",
    };
    int i;
    PROCESSENTRY32 pe32;
    SDL_bool found = SDL_FALSE;

    /* Take a snapshot of all processes in the system */
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap != INVALID_HANDLE_VALUE) {
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hProcessSnap, &pe32)) {
            do
            {
                for (i = 0; i < SDL_arraysize(mapper_processes); ++i) {
                    if (SDL_strcasecmp(pe32.szExeFile, mapper_processes[i]) == 0) {
                        found = SDL_TRUE;
                    }
                }
            } while (Process32Next(hProcessSnap, &pe32) && !found);
        }
        CloseHandle(hProcessSnap);
    }
    return found;
#else
    return SDL_FALSE;
#endif
}

SDL_bool SDL_ShouldIgnoreJoystick(const char *name, SDL_JoystickGUID guid)
{
    /* This list is taken from:
       https://raw.githubusercontent.com/denilsonsa/udev-joystick-blacklist/master/generate_rules.py
     */
    static Uint32 joystick_blacklist[] = {
        /* Microsoft Microsoft Wireless Optical Desktop 2.10 */
        /* Microsoft Wireless Desktop - Comfort Edition */
        MAKE_VIDPID(0x045e, 0x009d),

        /* Microsoft Microsoft Digital Media Pro Keyboard */
        /* Microsoft Corp. Digital Media Pro Keyboard */
        MAKE_VIDPID(0x045e, 0x00b0),

        /* Microsoft Microsoft Digital Media Keyboard */
        /* Microsoft Corp. Digital Media Keyboard 1.0A */
        MAKE_VIDPID(0x045e, 0x00b4),

        /* Microsoft Microsoft Digital Media Keyboard 3000 */
        MAKE_VIDPID(0x045e, 0x0730),

        /* Microsoft Microsoft 2.4GHz Transceiver v6.0 */
        /* Microsoft Microsoft 2.4GHz Transceiver v8.0 */
        /* Microsoft Corp. Nano Transceiver v1.0 for Bluetooth */
        /* Microsoft Wireless Mobile Mouse 1000 */
        /* Microsoft Wireless Desktop 3000 */
        MAKE_VIDPID(0x045e, 0x0745),

        /* Microsoft SideWinder(TM) 2.4GHz Transceiver */
        MAKE_VIDPID(0x045e, 0x0748),

        /* Microsoft Corp. Wired Keyboard 600 */
        MAKE_VIDPID(0x045e, 0x0750),

        /* Microsoft Corp. Sidewinder X4 keyboard */
        MAKE_VIDPID(0x045e, 0x0768),

        /* Microsoft Corp. Arc Touch Mouse Transceiver */
        MAKE_VIDPID(0x045e, 0x0773),

        /* Microsoft 2.4GHz Transceiver v9.0 */
        /* Microsoft Nano Transceiver v2.1 */
        /* Microsoft Sculpt Ergonomic Keyboard (5KV-00001) */
        MAKE_VIDPID(0x045e, 0x07a5),

        /* Microsoft Nano Transceiver v1.0 */
        /* Microsoft Wireless Keyboard 800 */
        MAKE_VIDPID(0x045e, 0x07b2),

        /* Microsoft Nano Transceiver v2.0 */
        MAKE_VIDPID(0x045e, 0x0800),

        MAKE_VIDPID(0x046d, 0xc30a),  /* Logitech, Inc. iTouch Composite keboard */

        MAKE_VIDPID(0x04d9, 0xa0df),  /* Tek Syndicate Mouse (E-Signal USB Gaming Mouse) */

        /* List of Wacom devices at: http://linuxwacom.sourceforge.net/wiki/index.php/Device_IDs */
        MAKE_VIDPID(0x056a, 0x0010),  /* Wacom ET-0405 Graphire */
        MAKE_VIDPID(0x056a, 0x0011),  /* Wacom ET-0405A Graphire2 (4x5) */
        MAKE_VIDPID(0x056a, 0x0012),  /* Wacom ET-0507A Graphire2 (5x7) */
        MAKE_VIDPID(0x056a, 0x0013),  /* Wacom CTE-430 Graphire3 (4x5) */
        MAKE_VIDPID(0x056a, 0x0014),  /* Wacom CTE-630 Graphire3 (6x8) */
        MAKE_VIDPID(0x056a, 0x0015),  /* Wacom CTE-440 Graphire4 (4x5) */
        MAKE_VIDPID(0x056a, 0x0016),  /* Wacom CTE-640 Graphire4 (6x8) */
        MAKE_VIDPID(0x056a, 0x0017),  /* Wacom CTE-450 Bamboo Fun (4x5) */
        MAKE_VIDPID(0x056a, 0x0018),  /* Wacom CTE-650 Bamboo Fun 6x8 */
        MAKE_VIDPID(0x056a, 0x0019),  /* Wacom CTE-631 Bamboo One */
        MAKE_VIDPID(0x056a, 0x00d1),  /* Wacom Bamboo Pen and Touch CTH-460 */
        MAKE_VIDPID(0x056a, 0x030e),  /* Wacom Intuos Pen (S) CTL-480 */

        MAKE_VIDPID(0x09da, 0x054f),  /* A4 Tech Co., G7 750 mouse */
        MAKE_VIDPID(0x09da, 0x1410),  /* A4 Tech Co., Ltd Bloody AL9 mouse */
        MAKE_VIDPID(0x09da, 0x3043),  /* A4 Tech Co., Ltd Bloody R8A Gaming Mouse */
        MAKE_VIDPID(0x09da, 0x31b5),  /* A4 Tech Co., Ltd Bloody TL80 Terminator Laser Gaming Mouse */
        MAKE_VIDPID(0x09da, 0x3997),  /* A4 Tech Co., Ltd Bloody RT7 Terminator Wireless */
        MAKE_VIDPID(0x09da, 0x3f8b),  /* A4 Tech Co., Ltd Bloody V8 mouse */
        MAKE_VIDPID(0x09da, 0x51f4),  /* Modecom MC-5006 Keyboard */
        MAKE_VIDPID(0x09da, 0x5589),  /* A4 Tech Co., Ltd Terminator TL9 Laser Gaming Mouse */
        MAKE_VIDPID(0x09da, 0x7b22),  /* A4 Tech Co., Ltd Bloody V5 */
        MAKE_VIDPID(0x09da, 0x7f2d),  /* A4 Tech Co., Ltd Bloody R3 mouse */
        MAKE_VIDPID(0x09da, 0x8090),  /* A4 Tech Co., Ltd X-718BK Oscar Optical Gaming Mouse */
        MAKE_VIDPID(0x09da, 0x9033),  /* A4 Tech Co., X7 X-705K */
        MAKE_VIDPID(0x09da, 0x9066),  /* A4 Tech Co., Sharkoon Fireglider Optical */
        MAKE_VIDPID(0x09da, 0x9090),  /* A4 Tech Co., Ltd XL-730K / XL-750BK / XL-755BK Laser Mouse */
        MAKE_VIDPID(0x09da, 0x90c0),  /* A4 Tech Co., Ltd X7 G800V keyboard */
        MAKE_VIDPID(0x09da, 0xf012),  /* A4 Tech Co., Ltd Bloody V7 mouse */
        MAKE_VIDPID(0x09da, 0xf32a),  /* A4 Tech Co., Ltd Bloody B540 keyboard */
        MAKE_VIDPID(0x09da, 0xf613),  /* A4 Tech Co., Ltd Bloody V2 mouse */
        MAKE_VIDPID(0x09da, 0xf624),  /* A4 Tech Co., Ltd Bloody B120 Keyboard */

        MAKE_VIDPID(0x1b1c, 0x1b3c),  /* Corsair Harpoon RGB gaming mouse */

        MAKE_VIDPID(0x1d57, 0xad03),  /* [T3] 2.4GHz and IR Air Mouse Remote Control */

        MAKE_VIDPID(0x1e7d, 0x2e4a),  /* Roccat Tyon Mouse */

        MAKE_VIDPID(0x20a0, 0x422d),  /* Winkeyless.kr Keyboards */

        MAKE_VIDPID(0x2516, 0x001f),  /* Cooler Master Storm Mizar Mouse */
        MAKE_VIDPID(0x2516, 0x0028),  /* Cooler Master Storm Alcor Mouse */

        /*****************************************************************/
        /* Additional entries                                            */
        /*****************************************************************/

        /* Anne Pro II Keyboard */
        MAKE_VIDPID(0x04d9, 0x8009),  /* OBINLB USB-HID Keyboard */
    };

    unsigned int i;
    Uint32 id;
    Uint16 vendor;
    Uint16 product;
    SDL_GameControllerType type;

    SDL_GetJoystickGUIDInfo(guid, &vendor, &product, NULL);

    /* Check the joystick blacklist */
    id = MAKE_VIDPID(vendor, product);
    for (i = 0; i < SDL_arraysize(joystick_blacklist); ++i) {
        if (id == joystick_blacklist[i]) {
            return SDL_TRUE;
        }
    }

    type = SDL_GetJoystickGameControllerType(name, vendor, product, -1, 0, 0, 0);
    if ((type == SDL_CONTROLLER_TYPE_PS4 || type == SDL_CONTROLLER_TYPE_PS5) && SDL_IsPS4RemapperRunning()) {
        return SDL_TRUE;
    }

    if (SDL_IsGameControllerNameAndGUID(name, guid) &&
        SDL_ShouldIgnoreGameController(name, guid)) {
        return SDL_TRUE;
    }

    return SDL_FALSE;
}

/* return the guid for this index */
SDL_JoystickGUID SDL_JoystickGetDeviceGUID(int device_index)
{
    SDL_JoystickDriver *driver;
    SDL_JoystickGUID guid;

    SDL_LockJoysticks();
    if (SDL_GetDriverAndJoystickIndex(device_index, &driver, &device_index)) {
        guid = driver->GetDeviceGUID(device_index);
    } else {
        SDL_zero(guid);
    }
    SDL_UnlockJoysticks();

    return guid;
}

Uint16 SDL_JoystickGetDeviceVendor(int device_index)
{
    Uint16 vendor;
    SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(device_index);

    SDL_GetJoystickGUIDInfo(guid, &vendor, NULL, NULL);
    return vendor;
}

Uint16 SDL_JoystickGetDeviceProduct(int device_index)
{
    Uint16 product;
    SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(device_index);

    SDL_GetJoystickGUIDInfo(guid, NULL, &product, NULL);
    return product;
}

Uint16 SDL_JoystickGetDeviceProductVersion(int device_index)
{
    Uint16 version;
    SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(device_index);

    SDL_GetJoystickGUIDInfo(guid, NULL, NULL, &version);
    return version;
}

SDL_JoystickType SDL_JoystickGetDeviceType(int device_index)
{
    SDL_JoystickType type;
    SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(device_index);

    type = SDL_GetJoystickGUIDType(guid);
    if (type == SDL_JOYSTICK_TYPE_UNKNOWN) {
        if (SDL_IsGameController(device_index)) {
            type = SDL_JOYSTICK_TYPE_GAMECONTROLLER;
        }
    }
    return type;
}

SDL_JoystickID SDL_JoystickGetDeviceInstanceID(int device_index)
{
    SDL_JoystickDriver *driver;
    SDL_JoystickID instance_id = -1;

    SDL_LockJoysticks();
    if (SDL_GetDriverAndJoystickIndex(device_index, &driver, &device_index)) {
        instance_id = driver->GetDeviceInstanceID(device_index);
    }
    SDL_UnlockJoysticks();

    return instance_id;
}

int SDL_JoystickGetDeviceIndexFromInstanceID(SDL_JoystickID instance_id)
{
    int i, num_joysticks, device_index = -1;

    SDL_LockJoysticks();
    num_joysticks = SDL_NumJoysticks();
    for (i = 0; i < num_joysticks; ++i) {
        if (SDL_JoystickGetDeviceInstanceID(i) == instance_id) {
            device_index = i;
            break;
        }
    }
    SDL_UnlockJoysticks();

    return device_index;
}

SDL_JoystickGUID SDL_JoystickGetGUID(SDL_Joystick *joystick)
{
    if (!SDL_PrivateJoystickValid(joystick)) {
        SDL_JoystickGUID emptyGUID;
        SDL_zero(emptyGUID);
        return emptyGUID;
    }
    return joystick->guid;
}

Uint16 SDL_JoystickGetVendor(SDL_Joystick *joystick)
{
    Uint16 vendor;
    SDL_JoystickGUID guid = SDL_JoystickGetGUID(joystick);

    SDL_GetJoystickGUIDInfo(guid, &vendor, NULL, NULL);
    return vendor;
}

Uint16 SDL_JoystickGetProduct(SDL_Joystick *joystick)
{
    Uint16 product;
    SDL_JoystickGUID guid = SDL_JoystickGetGUID(joystick);

    SDL_GetJoystickGUIDInfo(guid, NULL, &product, NULL);
    return product;
}

Uint16 SDL_JoystickGetProductVersion(SDL_Joystick *joystick)
{
    Uint16 version;
    SDL_JoystickGUID guid = SDL_JoystickGetGUID(joystick);

    SDL_GetJoystickGUIDInfo(guid, NULL, NULL, &version);
    return version;
}

const char *SDL_JoystickGetSerial(SDL_Joystick *joystick)
{
    if (!SDL_PrivateJoystickValid(joystick)) {
        return NULL;
    }
    return joystick->serial;
}

SDL_JoystickType SDL_JoystickGetType(SDL_Joystick *joystick)
{
    SDL_JoystickType type;
    SDL_JoystickGUID guid = SDL_JoystickGetGUID(joystick);

    type = SDL_GetJoystickGUIDType(guid);
    if (type == SDL_JOYSTICK_TYPE_UNKNOWN) {
        if (joystick && joystick->is_game_controller) {
            type = SDL_JOYSTICK_TYPE_GAMECONTROLLER;
        }
    }
    return type;
}

/* convert the guid to a printable string */
void SDL_JoystickGetGUIDString(SDL_JoystickGUID guid, char *pszGUID, int cbGUID)
{
    static const char k_rgchHexToASCII[] = "0123456789abcdef";
    int i;

    if ((pszGUID == NULL) || (cbGUID <= 0)) {
        return;
    }

    for (i = 0; i < sizeof(guid.data) && i < (cbGUID-1)/2; i++) {
        /* each input byte writes 2 ascii chars, and might write a null byte. */
        /* If we don't have room for next input byte, stop */
        unsigned char c = guid.data[i];

        *pszGUID++ = k_rgchHexToASCII[c >> 4];
        *pszGUID++ = k_rgchHexToASCII[c & 0x0F];
    }
    *pszGUID = '\0';
}

/*-----------------------------------------------------------------------------
 * Purpose: Returns the 4 bit nibble for a hex character
 * Input  : c -
 * Output : unsigned char
 *-----------------------------------------------------------------------------*/
static unsigned char nibble(char c)
{
    if ((c >= '0') && (c <= '9')) {
        return (unsigned char)(c - '0');
    }

    if ((c >= 'A') && (c <= 'F')) {
        return (unsigned char)(c - 'A' + 0x0a);
    }

    if ((c >= 'a') && (c <= 'f')) {
        return (unsigned char)(c - 'a' + 0x0a);
    }

    /* received an invalid character, and no real way to return an error */
    /* AssertMsg1(false, "Q_nibble invalid hex character '%c' ", c); */
    return 0;
}

/* convert the string version of a joystick guid to the struct */
SDL_JoystickGUID SDL_JoystickGetGUIDFromString(const char *pchGUID)
{
    SDL_JoystickGUID guid;
    int maxoutputbytes= sizeof(guid);
    size_t len = SDL_strlen(pchGUID);
    Uint8 *p;
    size_t i;

    /* Make sure it's even */
    len = (len) & ~0x1;

    SDL_memset(&guid, 0x00, sizeof(guid));

    p = (Uint8 *)&guid;
    for (i = 0; (i < len) && ((p - (Uint8 *)&guid) < maxoutputbytes); i+=2, p++) {
        *p = (nibble(pchGUID[i]) << 4) | nibble(pchGUID[i+1]);
    }

    return guid;
}

/* update the power level for this joystick */
void SDL_PrivateJoystickBatteryLevel(SDL_Joystick *joystick, SDL_JoystickPowerLevel ePowerLevel)
{
    joystick->epowerlevel = ePowerLevel;
}

/* return its power level */
SDL_JoystickPowerLevel SDL_JoystickCurrentPowerLevel(SDL_Joystick *joystick)
{
    if (!SDL_PrivateJoystickValid(joystick)) {
        return SDL_JOYSTICK_POWER_UNKNOWN;
    }
    return joystick->epowerlevel;
}

int SDL_PrivateJoystickTouchpad(SDL_Joystick *joystick, int touchpad, int finger, Uint8 state, float x, float y, float pressure)
{
    SDL_JoystickTouchpadInfo *touchpad_info;
    SDL_JoystickTouchpadFingerInfo *finger_info;
    int posted;
#if !SDL_EVENTS_DISABLED
    Uint32 event_type;
#endif

    if (touchpad < 0 || touchpad >= joystick->ntouchpads) {
        return 0;
    }

    touchpad_info = &joystick->touchpads[touchpad];
    if (finger < 0 || finger >= touchpad_info->nfingers) {
        return 0;
    }

    finger_info = &touchpad_info->fingers[finger];

    if (!state) {
        if (x == 0.0f && y == 0.0f) {
            x = finger_info->x;
            y = finger_info->y;
        }
        pressure = 0.0f;
    }

    if (x < 0.0f) {
        x = 0.0f;
    } else if (x > 1.0f) {
        x = 1.0f;
    }
    if (y < 0.0f) {
        y = 0.0f;
    } else if (y > 1.0f) {
        y = 1.0f;
    }
    if (pressure < 0.0f) {
        pressure = 0.0f;
    } else if (pressure > 1.0f) {
        pressure = 1.0f;
    }

    if (state == finger_info->state) {
        if (!state ||
            (x == finger_info->x && y == finger_info->y && pressure == finger_info->pressure)) {
            return 0;
        }
    }

#if !SDL_EVENTS_DISABLED
    if (state == finger_info->state) {
        event_type = SDL_CONTROLLERTOUCHPADMOTION;
    } else if (state) {
        event_type = SDL_CONTROLLERTOUCHPADDOWN;
    } else {
        event_type = SDL_CONTROLLERTOUCHPADUP;
    }
#endif

    /* Update internal joystick state */
    finger_info->state = state;
    finger_info->x = x;
    finger_info->y = y;
    finger_info->pressure = pressure;

    /* Post the event, if desired */
    posted = 0;
#if !SDL_EVENTS_DISABLED
    if (SDL_GetEventState(event_type) == SDL_ENABLE) {
        SDL_Event event;
        event.type = event_type;
        event.ctouchpad.which = joystick->instance_id;
        event.ctouchpad.touchpad = touchpad;
        event.ctouchpad.finger = finger;
        event.ctouchpad.x = x;
        event.ctouchpad.y = y;
        event.ctouchpad.pressure = pressure;
        posted = SDL_PushEvent(&event) == 1;
    }
#endif /* !SDL_EVENTS_DISABLED */
    return posted;
}

int SDL_PrivateJoystickSensor(SDL_Joystick *joystick, SDL_SensorType type, const float *data, int num_values)
{
    int i;
    int posted = 0;

    for (i = 0; i < joystick->nsensors; ++i) {
        SDL_JoystickSensorInfo *sensor = &joystick->sensors[i];

        if (sensor->type == type) {
            if (sensor->enabled) {
                num_values = SDL_min(num_values, SDL_arraysize(sensor->data));

                /* Update internal sensor state */
                SDL_memcpy(sensor->data, data, num_values*sizeof(*data));

                /* Post the event, if desired */
#if !SDL_EVENTS_DISABLED
                if (SDL_GetEventState(SDL_CONTROLLERSENSORUPDATE) == SDL_ENABLE) {
                    SDL_Event event;
                    event.type = SDL_CONTROLLERSENSORUPDATE;
                    event.csensor.which = joystick->instance_id;
                    event.csensor.sensor = type;
                    num_values = SDL_min(num_values, SDL_arraysize(event.csensor.data));
                    SDL_memset(event.csensor.data, 0, sizeof(event.csensor.data));
                    SDL_memcpy(event.csensor.data, data, num_values*sizeof(*data));
                    posted = SDL_PushEvent(&event) == 1;
                }
#endif /* !SDL_EVENTS_DISABLED */
            }
            break;
        }
    }
    return posted;
}

/* vi: set ts=4 sw=4 expandtab: */
