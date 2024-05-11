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

/* This is the joystick API for Simple DirectMedia Layer */

#include "SDL.h"
#include "SDL_events.h"
#include "SDL_sysjoystick.h"
#include "SDL_assert.h"
#include "SDL_hints.h"

#if !SDL_EVENTS_DISABLED
#include "../events/SDL_events_c.h"
#endif
#include "../video/SDL_sysvideo.h"


static SDL_bool SDL_joystick_allows_background_events = SDL_FALSE;
static SDL_Joystick *SDL_joysticks = NULL;
static SDL_bool SDL_updating_joystick = SDL_FALSE;
static SDL_mutex *SDL_joystick_lock = NULL; /* This needs to support recursive locks */

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
    int status;

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

    status = SDL_SYS_JoystickInit();
    if (status >= 0) {
        status = 0;
    }
    return (status);
}

/*
 * Count the number of joysticks attached to the system
 */
int
SDL_NumJoysticks(void)
{
    return SDL_SYS_NumJoysticks();
}

/*
 * Get the implementation dependent name of a joystick
 */
const char *
SDL_JoystickNameForIndex(int device_index)
{
    if (device_index < 0 || device_index >= SDL_NumJoysticks()) {
        SDL_SetError("There are %d joysticks available", SDL_NumJoysticks());
        return (NULL);
    }
    return (SDL_SYS_JoystickNameForDeviceIndex(device_index));
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
    SDL_Joystick *joystick;
    SDL_Joystick *joysticklist;
    const char *joystickname = NULL;

    if ((device_index < 0) || (device_index >= SDL_NumJoysticks())) {
        SDL_SetError("There are %d joysticks available", SDL_NumJoysticks());
        return (NULL);
    }

    SDL_LockJoysticks();

    joysticklist = SDL_joysticks;
    /* If the joystick is already open, return it
     * it is important that we have a single joystick * for each instance id
     */
    while (joysticklist) {
        if (SDL_JoystickGetDeviceInstanceID(device_index) == joysticklist->instance_id) {
                joystick = joysticklist;
                ++joystick->ref_count;
                SDL_UnlockJoysticks();
                return (joystick);
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

    if (SDL_SYS_JoystickOpen(joystick, device_index) < 0) {
        SDL_free(joystick);
        SDL_UnlockJoysticks();
        return NULL;
    }

    joystickname = SDL_SYS_JoystickNameForDeviceIndex(device_index);
    if (joystickname)
        joystick->name = SDL_strdup(joystickname);
    else
        joystick->name = NULL;

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
    joystick->epowerlevel = SDL_JOYSTICK_POWER_UNKNOWN;

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

    SDL_SYS_JoystickUpdate(joystick);

    return (joystick);
}


/*
 * Checks to make sure the joystick is valid.
 */
int
SDL_PrivateJoystickValid(SDL_Joystick * joystick)
{
    int valid;

    if (joystick == NULL) {
        SDL_SetError("Joystick hasn't been opened yet");
        valid = 0;
    } else {
        valid = 1;
    }

    return valid;
}

/*
 * Get the number of multi-dimensional axis controls on a joystick
 */
int
SDL_JoystickNumAxes(SDL_Joystick * joystick)
{
    if (!SDL_PrivateJoystickValid(joystick)) {
        return (-1);
    }
    return (joystick->naxes);
}

/*
 * Get the number of hats on a joystick
 */
int
SDL_JoystickNumHats(SDL_Joystick * joystick)
{
    if (!SDL_PrivateJoystickValid(joystick)) {
        return (-1);
    }
    return (joystick->nhats);
}

/*
 * Get the number of trackballs on a joystick
 */
int
SDL_JoystickNumBalls(SDL_Joystick * joystick)
{
    if (!SDL_PrivateJoystickValid(joystick)) {
        return (-1);
    }
    return (joystick->nballs);
}

/*
 * Get the number of buttons on a joystick
 */
int
SDL_JoystickNumButtons(SDL_Joystick * joystick)
{
    if (!SDL_PrivateJoystickValid(joystick)) {
        return (-1);
    }
    return (joystick->nbuttons);
}

/*
 * Get the current state of an axis control on a joystick
 */
Sint16
SDL_JoystickGetAxis(SDL_Joystick * joystick, int axis)
{
    Sint16 state;

    if (!SDL_PrivateJoystickValid(joystick)) {
        return (0);
    }
    if (axis < joystick->naxes) {
        state = joystick->axes[axis].value;
    } else {
        SDL_SetError("Joystick only has %d axes", joystick->naxes);
        state = 0;
    }
    return (state);
}

/*
 * Get the initial state of an axis control on a joystick
 */
SDL_bool
SDL_JoystickGetAxisInitialState(SDL_Joystick * joystick, int axis, Sint16 *state)
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
SDL_JoystickGetHat(SDL_Joystick * joystick, int hat)
{
    Uint8 state;

    if (!SDL_PrivateJoystickValid(joystick)) {
        return (0);
    }
    if (hat < joystick->nhats) {
        state = joystick->hats[hat];
    } else {
        SDL_SetError("Joystick only has %d hats", joystick->nhats);
        state = 0;
    }
    return (state);
}

/*
 * Get the ball axis change since the last poll
 */
int
SDL_JoystickGetBall(SDL_Joystick * joystick, int ball, int *dx, int *dy)
{
    int retval;

    if (!SDL_PrivateJoystickValid(joystick)) {
        return (-1);
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
    return (retval);
}

/*
 * Get the current state of a button on a joystick
 */
Uint8
SDL_JoystickGetButton(SDL_Joystick * joystick, int button)
{
    Uint8 state;

    if (!SDL_PrivateJoystickValid(joystick)) {
        return (0);
    }
    if (button < joystick->nbuttons) {
        state = joystick->buttons[button];
    } else {
        SDL_SetError("Joystick only has %d buttons", joystick->nbuttons);
        state = 0;
    }
    return (state);
}

/*
 * Return if the joystick in question is currently attached to the system,
 *  \return SDL_FALSE if not plugged in, SDL_TRUE if still present.
 */
SDL_bool
SDL_JoystickGetAttached(SDL_Joystick * joystick)
{
    if (!SDL_PrivateJoystickValid(joystick)) {
        return SDL_FALSE;
    }

    return SDL_SYS_JoystickAttached(joystick);
}

/*
 * Get the instance id for this opened joystick
 */
SDL_JoystickID
SDL_JoystickInstanceID(SDL_Joystick * joystick)
{
    if (!SDL_PrivateJoystickValid(joystick)) {
        return (-1);
    }

    return (joystick->instance_id);
}

/*
 * Find the SDL_Joystick that owns this instance id
 */
SDL_Joystick *
SDL_JoystickFromInstanceID(SDL_JoystickID joyid)
{
    SDL_Joystick *joystick;

    SDL_LockJoysticks();
    for (joystick = SDL_joysticks; joystick; joystick = joystick->next) {
        if (joystick->instance_id == joyid) {
            SDL_UnlockJoysticks();
            return joystick;
        }
    }
    SDL_UnlockJoysticks();
    return NULL;
}

/*
 * Get the friendly name of this joystick
 */
const char *
SDL_JoystickName(SDL_Joystick * joystick)
{
    if (!SDL_PrivateJoystickValid(joystick)) {
        return (NULL);
    }

    return (joystick->name);
}

/*
 * Close a joystick previously opened with SDL_JoystickOpen()
 */
void
SDL_JoystickClose(SDL_Joystick * joystick)
{
    SDL_Joystick *joysticklist;
    SDL_Joystick *joysticklistprev;

    if (!joystick) {
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

    SDL_SYS_JoystickClose(joystick);
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

    /* Free the data associated with this joystick */
    SDL_free(joystick->axes);
    SDL_free(joystick->hats);
    SDL_free(joystick->balls);
    SDL_free(joystick->buttons);
    SDL_free(joystick);

    SDL_UnlockJoysticks();
}

void
SDL_JoystickQuit(void)
{
    /* Make sure we're not getting called in the middle of updating joysticks */
    SDL_assert(!SDL_updating_joystick);

    SDL_LockJoysticks();

    /* Stop the event polling */
    while (SDL_joysticks) {
        SDL_joysticks->ref_count = 1;
        SDL_JoystickClose(SDL_joysticks);
    }

    /* Quit the joystick setup */
    SDL_SYS_JoystickQuit();

    SDL_UnlockJoysticks();

#if !SDL_EVENTS_DISABLED
    SDL_QuitSubSystem(SDL_INIT_EVENTS);
#endif

    SDL_DelHintCallback(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,
                        SDL_JoystickAllowBackgroundEventsChanged, NULL);

    if (SDL_joystick_lock) {
        SDL_DestroyMutex(SDL_joystick_lock);
        SDL_joystick_lock = NULL;
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

void SDL_PrivateJoystickAdded(int device_index)
{
#if !SDL_EVENTS_DISABLED
    SDL_Event event;

    event.type = SDL_JOYDEVICEADDED;

    if (SDL_GetEventState(event.type) == SDL_ENABLE) {
        event.jdevice.which = device_index;
        SDL_PushEvent(&event);
    }
#endif /* !SDL_EVENTS_DISABLED */
}

/*
 * If there is an existing add event in the queue, it needs to be modified
 * to have the right value for which, because the number of controllers in
 * the system is now one less.
 */
static void UpdateEventsForDeviceRemoval()
{
    int i, num_events;
    SDL_Event *events;

    num_events = SDL_PeepEvents(NULL, 0, SDL_PEEKEVENT, SDL_JOYDEVICEADDED, SDL_JOYDEVICEADDED);
    if (num_events <= 0) {
        return;
    }

    events = SDL_stack_alloc(SDL_Event, num_events);
    if (!events) {
        return;
    }

    num_events = SDL_PeepEvents(events, num_events, SDL_GETEVENT, SDL_JOYDEVICEADDED, SDL_JOYDEVICEADDED);
    for (i = 0; i < num_events; ++i) {
        --events[i].jdevice.which;
    }
    SDL_PeepEvents(events, num_events, SDL_ADDEVENT, 0, 0);

    SDL_stack_free(events);
}

void SDL_PrivateJoystickRemoved(SDL_JoystickID device_instance)
{
#if !SDL_EVENTS_DISABLED
    SDL_Event event;

    event.type = SDL_JOYDEVICEREMOVED;

    if (SDL_GetEventState(event.type) == SDL_ENABLE) {
        event.jdevice.which = device_instance;
        SDL_PushEvent(&event);
    }

    UpdateEventsForDeviceRemoval();
#endif /* !SDL_EVENTS_DISABLED */
}

int
SDL_PrivateJoystickAxis(SDL_Joystick * joystick, Uint8 axis, Sint16 value)
{
    int posted;

    /* Make sure we're not getting garbage or duplicate events */
    if (axis >= joystick->naxes) {
        return 0;
    }
    if (!joystick->axes[axis].has_initial_value) {
        joystick->axes[axis].initial_value = value;
        joystick->axes[axis].value = value;
        joystick->axes[axis].zero = value;
        joystick->axes[axis].has_initial_value = SDL_TRUE;
    }
    if (value == joystick->axes[axis].value) {
        return 0;
    }
    if (!joystick->axes[axis].sent_initial_value) {
        /* Make sure we don't send motion until there's real activity on this axis */
        const int MAX_ALLOWED_JITTER = SDL_JOYSTICK_AXIS_MAX / 80;  /* ShanWan PS3 controller needed 96 */
        if (SDL_abs(value - joystick->axes[axis].value) <= MAX_ALLOWED_JITTER) {
            return 0;
        }
        joystick->axes[axis].sent_initial_value = SDL_TRUE;
        joystick->axes[axis].value = value; /* Just so we pass the check above */
        SDL_PrivateJoystickAxis(joystick, axis, joystick->axes[axis].initial_value);
    }

    /* We ignore events if we don't have keyboard focus, except for centering
     * events.
     */
    if (SDL_PrivateJoystickShouldIgnoreEvent()) {
        if ((value > joystick->axes[axis].zero && value >= joystick->axes[axis].value) ||
            (value < joystick->axes[axis].zero && value <= joystick->axes[axis].value)) {
            return 0;
        }
    }

    /* Update internal joystick state */
    joystick->axes[axis].value = value;

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
    return (posted);
}

int
SDL_PrivateJoystickHat(SDL_Joystick * joystick, Uint8 hat, Uint8 value)
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
    return (posted);
}

int
SDL_PrivateJoystickBall(SDL_Joystick * joystick, Uint8 ball,
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
    return (posted);
}

int
SDL_PrivateJoystickButton(SDL_Joystick * joystick, Uint8 button, Uint8 state)
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
        return (0);
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
    return (posted);
}

void
SDL_JoystickUpdate(void)
{
    SDL_Joystick *joystick;

    SDL_LockJoysticks();

    if (SDL_updating_joystick) {
        /* The joysticks are already being updated */
        SDL_UnlockJoysticks();
        return;
    }

    SDL_updating_joystick = SDL_TRUE;

    /* Make sure the list is unlocked while dispatching events to prevent application deadlocks */
    SDL_UnlockJoysticks();

    for (joystick = SDL_joysticks; joystick; joystick = joystick->next) {
        SDL_SYS_JoystickUpdate(joystick);

        if (joystick->force_recentering) {
            int i;

            /* Tell the app that everything is centered/unpressed... */
            for (i = 0; i < joystick->naxes; i++) {
                if (joystick->axes[i].has_initial_value) {
                    SDL_PrivateJoystickAxis(joystick, i, joystick->axes[i].zero);
                }
            }

            for (i = 0; i < joystick->nbuttons; i++) {
                SDL_PrivateJoystickButton(joystick, i, 0);
            }

            for (i = 0; i < joystick->nhats; i++) {
                SDL_PrivateJoystickHat(joystick, i, SDL_HAT_CENTERED);
            }

            joystick->force_recentering = SDL_FALSE;
        }
    }

    SDL_LockJoysticks();

    SDL_updating_joystick = SDL_FALSE;

    /* If any joysticks were closed while updating, free them here */
    for (joystick = SDL_joysticks; joystick; joystick = joystick->next) {
        if (joystick->ref_count <= 0) {
            SDL_JoystickClose(joystick);
        }
    }

    /* this needs to happen AFTER walking the joystick list above, so that any
       dangling hardware data from removed devices can be free'd
     */
    SDL_SYS_JoystickDetect();

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
    return (state);
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

static SDL_bool SDL_IsJoystickProductWheel(Uint32 vidpid)
{
    static Uint32 wheel_joysticks[] = {
        MAKE_VIDPID(0x046d, 0xc294),    /* Logitech generic wheel */
        MAKE_VIDPID(0x046d, 0xc295),    /* Logitech Momo Force */
        MAKE_VIDPID(0x046d, 0xc298),    /* Logitech Driving Force Pro */
        MAKE_VIDPID(0x046d, 0xc299),    /* Logitech G25 */
        MAKE_VIDPID(0x046d, 0xc29a),    /* Logitech Driving Force GT */
        MAKE_VIDPID(0x046d, 0xc29b),    /* Logitech G27 */
        MAKE_VIDPID(0x046d, 0xc261),    /* Logitech G920 (initial mode) */
        MAKE_VIDPID(0x046d, 0xc262),    /* Logitech G920 (active mode) */
        MAKE_VIDPID(0x044f, 0xb65d),    /* Thrustmaster Wheel FFB */
        MAKE_VIDPID(0x044f, 0xb66d),    /* Thrustmaster Wheel FFB */
        MAKE_VIDPID(0x044f, 0xb677),    /* Thrustmaster T150 */
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

static SDL_bool SDL_IsJoystickProductFlightStick(Uint32 vidpid)
{
    static Uint32 flightstick_joysticks[] = {
        MAKE_VIDPID(0x044f, 0x0402),    /* HOTAS Warthog Joystick */
        MAKE_VIDPID(0x0738, 0x2221),    /* Saitek Pro Flight X-56 Rhino Stick */
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

    if (guid.data[14] == 'x') {
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

    SDL_GetJoystickGUIDInfo(guid, &vendor, &product, NULL);
    vidpid = MAKE_VIDPID(vendor, product);

    if (SDL_IsJoystickProductWheel(vidpid)) {
        return SDL_JOYSTICK_TYPE_WHEEL;
    }

    if (SDL_IsJoystickProductFlightStick(vidpid)) {
        return SDL_JOYSTICK_TYPE_FLIGHT_STICK;
    }

    if (SDL_IsJoystickProductThrottle(vidpid)) {
        return SDL_JOYSTICK_TYPE_THROTTLE;
    }

    return SDL_JOYSTICK_TYPE_UNKNOWN;
}

/* return the guid for this index */
SDL_JoystickGUID SDL_JoystickGetDeviceGUID(int device_index)
{
    if (device_index < 0 || device_index >= SDL_NumJoysticks()) {
        SDL_JoystickGUID emptyGUID;
        SDL_SetError("There are %d joysticks available", SDL_NumJoysticks());
        SDL_zero(emptyGUID);
        return emptyGUID;
    }
    return SDL_SYS_JoystickGetDeviceGUID(device_index);
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
    if (device_index < 0 || device_index >= SDL_NumJoysticks()) {
        SDL_SetError("There are %d joysticks available", SDL_NumJoysticks());
        return -1;
    }
    return SDL_SYS_GetInstanceIdOfDeviceIndex(device_index);
}

SDL_JoystickGUID SDL_JoystickGetGUID(SDL_Joystick * joystick)
{
    if (!SDL_PrivateJoystickValid(joystick)) {
        SDL_JoystickGUID emptyGUID;
        SDL_zero(emptyGUID);
        return emptyGUID;
    }
    return SDL_SYS_JoystickGetGUID(joystick);
}

Uint16 SDL_JoystickGetVendor(SDL_Joystick * joystick)
{
    Uint16 vendor;
    SDL_JoystickGUID guid = SDL_JoystickGetGUID(joystick);

    SDL_GetJoystickGUIDInfo(guid, &vendor, NULL, NULL);
    return vendor;
}

Uint16 SDL_JoystickGetProduct(SDL_Joystick * joystick)
{
    Uint16 product;
    SDL_JoystickGUID guid = SDL_JoystickGetGUID(joystick);

    SDL_GetJoystickGUIDInfo(guid, NULL, &product, NULL);
    return product;
}

Uint16 SDL_JoystickGetProductVersion(SDL_Joystick * joystick)
{
    Uint16 version;
    SDL_JoystickGUID guid = SDL_JoystickGetGUID(joystick);

    SDL_GetJoystickGUIDInfo(guid, NULL, NULL, &version);
    return version;
}

SDL_JoystickType SDL_JoystickGetType(SDL_Joystick * joystick)
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
void SDL_PrivateJoystickBatteryLevel(SDL_Joystick * joystick, SDL_JoystickPowerLevel ePowerLevel)
{
    joystick->epowerlevel = ePowerLevel;
}


/* return its power level */
SDL_JoystickPowerLevel SDL_JoystickCurrentPowerLevel(SDL_Joystick * joystick)
{
    if (!SDL_PrivateJoystickValid(joystick)) {
        return (SDL_JOYSTICK_POWER_UNKNOWN);
    }
    return joystick->epowerlevel;
}

/* vi: set ts=4 sw=4 expandtab: */
