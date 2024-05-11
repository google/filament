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

/* This is the game controller API for Simple DirectMedia Layer */

#include "SDL_events.h"
#include "SDL_assert.h"
#include "SDL_hints.h"
#include "SDL_sysjoystick.h"
#include "SDL_joystick_c.h"
#include "SDL_gamecontrollerdb.h"

#if !SDL_EVENTS_DISABLED
#include "../events/SDL_events_c.h"
#endif

#if defined(__ANDROID__)
#include "SDL_system.h"
#endif


#define SDL_CONTROLLER_PLATFORM_FIELD "platform:"

/* a list of currently opened game controllers */
static SDL_GameController *SDL_gamecontrollers = NULL;

typedef struct
{
    SDL_GameControllerBindType inputType;
    union
    {
        int button;

        struct {
            int axis;
            int axis_min;
            int axis_max;
        } axis;

        struct {
            int hat;
            int hat_mask;
        } hat;

    } input;

    SDL_GameControllerBindType outputType;
    union
    {
        SDL_GameControllerButton button;

        struct {
            SDL_GameControllerAxis axis;
            int axis_min;
            int axis_max;
        } axis;

    } output;

} SDL_ExtendedGameControllerBind;

/* our hard coded list of mapping support */
typedef enum
{
    SDL_CONTROLLER_MAPPING_PRIORITY_DEFAULT,
    SDL_CONTROLLER_MAPPING_PRIORITY_API,
    SDL_CONTROLLER_MAPPING_PRIORITY_USER,
} SDL_ControllerMappingPriority;

typedef struct _ControllerMapping_t
{
    SDL_JoystickGUID guid;
    char *name;
    char *mapping;
    SDL_ControllerMappingPriority priority;
    struct _ControllerMapping_t *next;
} ControllerMapping_t;

static SDL_JoystickGUID s_zeroGUID;
static ControllerMapping_t *s_pSupportedControllers = NULL;
static ControllerMapping_t *s_pXInputMapping = NULL;
static ControllerMapping_t *s_pEmscriptenMapping = NULL;

/* The SDL game controller structure */
struct _SDL_GameController
{
    SDL_Joystick *joystick; /* underlying joystick device */
    int ref_count;

    SDL_JoystickGUID guid;
    const char *name;
    int num_bindings;
    SDL_ExtendedGameControllerBind *bindings;
    SDL_ExtendedGameControllerBind **last_match_axis;
    Uint8 *last_hat_mask;

    struct _SDL_GameController *next; /* pointer to next game controller we have allocated */
};


typedef struct
{
    int num_entries;
    int max_entries;
    Uint32 *entries;
} SDL_vidpid_list;

static SDL_vidpid_list SDL_allowed_controllers;
static SDL_vidpid_list SDL_ignored_controllers;

static void
SDL_LoadVIDPIDListFromHint(const char *hint, SDL_vidpid_list *list)
{
    Uint32 entry;
    char *spot;
    char *file = NULL;

    list->num_entries = 0;

    if (hint && *hint == '@') {
        spot = file = (char *)SDL_LoadFile(hint+1, NULL);
    } else {
        spot = (char *)hint;
    }

    if (!spot) {
        return;
    }

    while ((spot = SDL_strstr(spot, "0x")) != NULL) {
        entry = (Uint16)SDL_strtol(spot, &spot, 0);
        entry <<= 16;
        spot = SDL_strstr(spot, "0x");
        if (!spot) {
            break;
        }
        entry |= (Uint16)SDL_strtol(spot, &spot, 0);

        if (list->num_entries == list->max_entries) {
            int max_entries = list->max_entries + 16;
            Uint32 *entries = (Uint32 *)SDL_realloc(list->entries, max_entries*sizeof(*list->entries));
            if (entries == NULL) {
                /* Out of memory, go with what we have already */
                break;
            }
            list->entries = entries;
            list->max_entries = max_entries;
        }
        list->entries[list->num_entries++] = entry;
    }

    if (file) {
        SDL_free(file);
    }
}

static void SDLCALL
SDL_GameControllerIgnoreDevicesChanged(void *userdata, const char *name, const char *oldValue, const char *hint)
{
    SDL_LoadVIDPIDListFromHint(hint, &SDL_ignored_controllers);
}

static void SDLCALL
SDL_GameControllerIgnoreDevicesExceptChanged(void *userdata, const char *name, const char *oldValue, const char *hint)
{
    SDL_LoadVIDPIDListFromHint(hint, &SDL_allowed_controllers);
}

static int SDL_PrivateGameControllerAxis(SDL_GameController * gamecontroller, SDL_GameControllerAxis axis, Sint16 value);
static int SDL_PrivateGameControllerButton(SDL_GameController * gamecontroller, SDL_GameControllerButton button, Uint8 state);

/*
 * If there is an existing add event in the queue, it needs to be modified
 * to have the right value for which, because the number of controllers in
 * the system is now one less.
 */
static void UpdateEventsForDeviceRemoval()
{
    int i, num_events;
    SDL_Event *events;

    num_events = SDL_PeepEvents(NULL, 0, SDL_PEEKEVENT, SDL_CONTROLLERDEVICEADDED, SDL_CONTROLLERDEVICEADDED);
    if (num_events <= 0) {
        return;
    }

    events = SDL_stack_alloc(SDL_Event, num_events);
    if (!events) {
        return;
    }

    num_events = SDL_PeepEvents(events, num_events, SDL_GETEVENT, SDL_CONTROLLERDEVICEADDED, SDL_CONTROLLERDEVICEADDED);
    for (i = 0; i < num_events; ++i) {
        --events[i].cdevice.which;
    }
    SDL_PeepEvents(events, num_events, SDL_ADDEVENT, 0, 0);

    SDL_stack_free(events);
}

static SDL_bool HasSameOutput(SDL_ExtendedGameControllerBind *a, SDL_ExtendedGameControllerBind *b)
{
    if (a->outputType != b->outputType) {
        return SDL_FALSE;
    }

    if (a->outputType == SDL_CONTROLLER_BINDTYPE_AXIS) {
        return (a->output.axis.axis == b->output.axis.axis);
    } else {
        return (a->output.button == b->output.button);
    }
}

static void ResetOutput(SDL_GameController *gamecontroller, SDL_ExtendedGameControllerBind *bind)
{
    if (bind->outputType == SDL_CONTROLLER_BINDTYPE_AXIS) {
        SDL_PrivateGameControllerAxis(gamecontroller, bind->output.axis.axis, 0);
    } else {
        SDL_PrivateGameControllerButton(gamecontroller, bind->output.button, SDL_RELEASED);
    }
}

static void HandleJoystickAxis(SDL_GameController *gamecontroller, int axis, int value)
{
    int i;
    SDL_ExtendedGameControllerBind *last_match = gamecontroller->last_match_axis[axis];
    SDL_ExtendedGameControllerBind *match = NULL;

    for (i = 0; i < gamecontroller->num_bindings; ++i) {
        SDL_ExtendedGameControllerBind *binding = &gamecontroller->bindings[i];
        if (binding->inputType == SDL_CONTROLLER_BINDTYPE_AXIS &&
            axis == binding->input.axis.axis) {
            if (binding->input.axis.axis_min < binding->input.axis.axis_max) {
                if (value >= binding->input.axis.axis_min &&
                    value <= binding->input.axis.axis_max) {
                    match = binding;
                    break;
                }
            } else {
                if (value >= binding->input.axis.axis_max &&
                    value <= binding->input.axis.axis_min) {
                    match = binding;
                    break;
                }
            }
        }
    }

    if (last_match && (!match || !HasSameOutput(last_match, match))) {
        /* Clear the last input that this axis generated */
        ResetOutput(gamecontroller, last_match);
    }

    if (match) {
        if (match->outputType == SDL_CONTROLLER_BINDTYPE_AXIS) {
            if (match->input.axis.axis_min != match->output.axis.axis_min || match->input.axis.axis_max != match->output.axis.axis_max) {
                float normalized_value = (float)(value - match->input.axis.axis_min) / (match->input.axis.axis_max - match->input.axis.axis_min);
                value = match->output.axis.axis_min + (int)(normalized_value * (match->output.axis.axis_max - match->output.axis.axis_min));
            }
            SDL_PrivateGameControllerAxis(gamecontroller, match->output.axis.axis, (Sint16)value);
        } else {
            Uint8 state;
            int threshold = match->input.axis.axis_min + (match->input.axis.axis_max - match->input.axis.axis_min) / 2;
            if (match->input.axis.axis_max < match->input.axis.axis_min) {
                state = (value <= threshold) ? SDL_PRESSED : SDL_RELEASED;
            } else {
                state = (value >= threshold) ? SDL_PRESSED : SDL_RELEASED;
            }
            SDL_PrivateGameControllerButton(gamecontroller, match->output.button, state);
        }
    }
    gamecontroller->last_match_axis[axis] = match;
}

static void HandleJoystickButton(SDL_GameController *gamecontroller, int button, Uint8 state)
{
    int i;

    for (i = 0; i < gamecontroller->num_bindings; ++i) {
        SDL_ExtendedGameControllerBind *binding = &gamecontroller->bindings[i];
        if (binding->inputType == SDL_CONTROLLER_BINDTYPE_BUTTON &&
            button == binding->input.button) {
            if (binding->outputType == SDL_CONTROLLER_BINDTYPE_AXIS) {
                int value = state ? binding->output.axis.axis_max : binding->output.axis.axis_min;
                SDL_PrivateGameControllerAxis(gamecontroller, binding->output.axis.axis, (Sint16)value);
            } else {
                SDL_PrivateGameControllerButton(gamecontroller, binding->output.button, state);
            }
            break;
        }
    }
}

static void HandleJoystickHat(SDL_GameController *gamecontroller, int hat, Uint8 value)
{
    int i;
    Uint8 last_mask = gamecontroller->last_hat_mask[hat];
    Uint8 changed_mask = (last_mask ^ value);

    for (i = 0; i < gamecontroller->num_bindings; ++i) {
        SDL_ExtendedGameControllerBind *binding = &gamecontroller->bindings[i];
        if (binding->inputType == SDL_CONTROLLER_BINDTYPE_HAT && hat == binding->input.hat.hat) {
            if ((changed_mask & binding->input.hat.hat_mask) != 0) {
                if (value & binding->input.hat.hat_mask) {
                    if (binding->outputType == SDL_CONTROLLER_BINDTYPE_AXIS) {
                        SDL_PrivateGameControllerAxis(gamecontroller, binding->output.axis.axis, (Sint16)binding->output.axis.axis_max);
                    } else {
                        SDL_PrivateGameControllerButton(gamecontroller, binding->output.button, SDL_PRESSED);
                    }
                } else {
                    ResetOutput(gamecontroller, binding);
                }
            }
        }
    }
    gamecontroller->last_hat_mask[hat] = value;
}

/*
 * Event filter to fire controller events from joystick ones
 */
static int SDLCALL SDL_GameControllerEventWatcher(void *userdata, SDL_Event * event)
{
    switch(event->type) {
    case SDL_JOYAXISMOTION:
        {
            SDL_GameController *controllerlist = SDL_gamecontrollers;
            while (controllerlist) {
                if (controllerlist->joystick->instance_id == event->jaxis.which) {
                    HandleJoystickAxis(controllerlist, event->jaxis.axis, event->jaxis.value);
                    break;
                }
                controllerlist = controllerlist->next;
            }
        }
        break;
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
        {
            SDL_GameController *controllerlist = SDL_gamecontrollers;
            while (controllerlist) {
                if (controllerlist->joystick->instance_id == event->jbutton.which) {
                    HandleJoystickButton(controllerlist, event->jbutton.button, event->jbutton.state);
                    break;
                }
                controllerlist = controllerlist->next;
            }
        }
        break;
    case SDL_JOYHATMOTION:
        {
            SDL_GameController *controllerlist = SDL_gamecontrollers;
            while (controllerlist) {
                if (controllerlist->joystick->instance_id == event->jhat.which) {
                    HandleJoystickHat(controllerlist, event->jhat.hat, event->jhat.value);
                    break;
                }
                controllerlist = controllerlist->next;
            }
        }
        break;
    case SDL_JOYDEVICEADDED:
        {
            if (SDL_IsGameController(event->jdevice.which)) {
                SDL_Event deviceevent;
                deviceevent.type = SDL_CONTROLLERDEVICEADDED;
                deviceevent.cdevice.which = event->jdevice.which;
                SDL_PushEvent(&deviceevent);
            }
        }
        break;
    case SDL_JOYDEVICEREMOVED:
        {
            SDL_GameController *controllerlist = SDL_gamecontrollers;
            while (controllerlist) {
                if (controllerlist->joystick->instance_id == event->jdevice.which) {
                    SDL_Event deviceevent;

                    deviceevent.type = SDL_CONTROLLERDEVICEREMOVED;
                    deviceevent.cdevice.which = event->jdevice.which;
                    SDL_PushEvent(&deviceevent);

                    UpdateEventsForDeviceRemoval();
                    break;
                }
                controllerlist = controllerlist->next;
            }
        }
        break;
    default:
        break;
    }

    return 1;
}

/*
 * Helper function to scan the mappings database for a controller with the specified GUID
 */
static ControllerMapping_t *SDL_PrivateGetControllerMappingForGUID(SDL_JoystickGUID *guid)
{
    ControllerMapping_t *pSupportedController = s_pSupportedControllers;
    while (pSupportedController) {
        if (SDL_memcmp(guid, &pSupportedController->guid, sizeof(*guid)) == 0) {
            return pSupportedController;
        }
        pSupportedController = pSupportedController->next;
    }
    return NULL;
}

static const char* map_StringForControllerAxis[] = {
    "leftx",
    "lefty",
    "rightx",
    "righty",
    "lefttrigger",
    "righttrigger",
    NULL
};

/*
 * convert a string to its enum equivalent
 */
SDL_GameControllerAxis SDL_GameControllerGetAxisFromString(const char *pchString)
{
    int entry;

    if (pchString && (*pchString == '+' || *pchString == '-')) {
        ++pchString;
    }

    if (!pchString || !pchString[0]) {
        return SDL_CONTROLLER_AXIS_INVALID;
    }

    for (entry = 0; map_StringForControllerAxis[entry]; ++entry) {
        if (!SDL_strcasecmp(pchString, map_StringForControllerAxis[entry]))
            return (SDL_GameControllerAxis) entry;
    }
    return SDL_CONTROLLER_AXIS_INVALID;
}

/*
 * convert an enum to its string equivalent
 */
const char* SDL_GameControllerGetStringForAxis(SDL_GameControllerAxis axis)
{
    if (axis > SDL_CONTROLLER_AXIS_INVALID && axis < SDL_CONTROLLER_AXIS_MAX) {
        return map_StringForControllerAxis[axis];
    }
    return NULL;
}

static const char* map_StringForControllerButton[] = {
    "a",
    "b",
    "x",
    "y",
    "back",
    "guide",
    "start",
    "leftstick",
    "rightstick",
    "leftshoulder",
    "rightshoulder",
    "dpup",
    "dpdown",
    "dpleft",
    "dpright",
    NULL
};

/*
 * convert a string to its enum equivalent
 */
SDL_GameControllerButton SDL_GameControllerGetButtonFromString(const char *pchString)
{
    int entry;
    if (!pchString || !pchString[0])
        return SDL_CONTROLLER_BUTTON_INVALID;

    for (entry = 0; map_StringForControllerButton[entry]; ++entry) {
        if (SDL_strcasecmp(pchString, map_StringForControllerButton[entry]) == 0)
            return (SDL_GameControllerButton) entry;
    }
    return SDL_CONTROLLER_BUTTON_INVALID;
}

/*
 * convert an enum to its string equivalent
 */
const char* SDL_GameControllerGetStringForButton(SDL_GameControllerButton axis)
{
    if (axis > SDL_CONTROLLER_BUTTON_INVALID && axis < SDL_CONTROLLER_BUTTON_MAX) {
        return map_StringForControllerButton[axis];
    }
    return NULL;
}

/*
 * given a controller button name and a joystick name update our mapping structure with it
 */
static void SDL_PrivateGameControllerParseElement(SDL_GameController *gamecontroller, const char *szGameButton, const char *szJoystickButton)
{
    SDL_ExtendedGameControllerBind bind;
    SDL_GameControllerButton button;
    SDL_GameControllerAxis axis;
    SDL_bool invert_input = SDL_FALSE;
    char half_axis_input = 0;
    char half_axis_output = 0;

    if (*szGameButton == '+' || *szGameButton == '-') {
        half_axis_output = *szGameButton++;
    }

    axis = SDL_GameControllerGetAxisFromString(szGameButton);
    button = SDL_GameControllerGetButtonFromString(szGameButton);
    if (axis != SDL_CONTROLLER_AXIS_INVALID) {
        bind.outputType = SDL_CONTROLLER_BINDTYPE_AXIS;
        bind.output.axis.axis = axis;
        if (axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
            bind.output.axis.axis_min = 0;
            bind.output.axis.axis_max = SDL_JOYSTICK_AXIS_MAX;
        } else {
            if (half_axis_output == '+') {
                bind.output.axis.axis_min = 0;
                bind.output.axis.axis_max = SDL_JOYSTICK_AXIS_MAX;
            } else if (half_axis_output == '-') {
                bind.output.axis.axis_min = 0;
                bind.output.axis.axis_max = SDL_JOYSTICK_AXIS_MIN;
            } else {
                bind.output.axis.axis_min = SDL_JOYSTICK_AXIS_MIN;
                bind.output.axis.axis_max = SDL_JOYSTICK_AXIS_MAX;
            }
        }
    } else if (button != SDL_CONTROLLER_BUTTON_INVALID) {
        bind.outputType = SDL_CONTROLLER_BINDTYPE_BUTTON;
        bind.output.button = button;
    } else {
        SDL_SetError("Unexpected controller element %s", szGameButton);
        return;
    }

    if (*szJoystickButton == '+' || *szJoystickButton == '-') {
        half_axis_input = *szJoystickButton++;
    }
    if (szJoystickButton[SDL_strlen(szJoystickButton) - 1] == '~') {
        invert_input = SDL_TRUE;
    }

    if (szJoystickButton[0] == 'a' && SDL_isdigit(szJoystickButton[1])) {
        bind.inputType = SDL_CONTROLLER_BINDTYPE_AXIS;
        bind.input.axis.axis = SDL_atoi(&szJoystickButton[1]);
        if (half_axis_input == '+') {
            bind.input.axis.axis_min = 0;
            bind.input.axis.axis_max = SDL_JOYSTICK_AXIS_MAX;
        } else if (half_axis_input == '-') {
            bind.input.axis.axis_min = 0;
            bind.input.axis.axis_max = SDL_JOYSTICK_AXIS_MIN;
        } else {
            bind.input.axis.axis_min = SDL_JOYSTICK_AXIS_MIN;
            bind.input.axis.axis_max = SDL_JOYSTICK_AXIS_MAX;
        }
        if (invert_input) {
            int tmp = bind.input.axis.axis_min;
            bind.input.axis.axis_min = bind.input.axis.axis_max;
            bind.input.axis.axis_max = tmp;
        }
    } else if (szJoystickButton[0] == 'b' && SDL_isdigit(szJoystickButton[1])) {
        bind.inputType = SDL_CONTROLLER_BINDTYPE_BUTTON;
        bind.input.button = SDL_atoi(&szJoystickButton[1]);
    } else if (szJoystickButton[0] == 'h' && SDL_isdigit(szJoystickButton[1]) &&
               szJoystickButton[2] == '.' && SDL_isdigit(szJoystickButton[3])) {
        int hat = SDL_atoi(&szJoystickButton[1]);
        int mask = SDL_atoi(&szJoystickButton[3]);
        bind.inputType = SDL_CONTROLLER_BINDTYPE_HAT;
        bind.input.hat.hat = hat;
        bind.input.hat.hat_mask = mask;
    } else {
        SDL_SetError("Unexpected joystick element: %s", szJoystickButton);
        return;
    }

    ++gamecontroller->num_bindings;
    gamecontroller->bindings = (SDL_ExtendedGameControllerBind *)SDL_realloc(gamecontroller->bindings, gamecontroller->num_bindings * sizeof(*gamecontroller->bindings));
    if (!gamecontroller->bindings) {
        gamecontroller->num_bindings = 0;
        SDL_OutOfMemory();
        return;
    }
    gamecontroller->bindings[gamecontroller->num_bindings - 1] = bind;
}


/*
 * given a controller mapping string update our mapping object
 */
static void
SDL_PrivateGameControllerParseControllerConfigString(SDL_GameController *gamecontroller, const char *pchString)
{
    char szGameButton[20];
    char szJoystickButton[20];
    SDL_bool bGameButton = SDL_TRUE;
    int i = 0;
    const char *pchPos = pchString;

    SDL_zero(szGameButton);
    SDL_zero(szJoystickButton);

    while (pchPos && *pchPos) {
        if (*pchPos == ':') {
            i = 0;
            bGameButton = SDL_FALSE;
        } else if (*pchPos == ' ') {

        } else if (*pchPos == ',') {
            i = 0;
            bGameButton = SDL_TRUE;
            SDL_PrivateGameControllerParseElement(gamecontroller, szGameButton, szJoystickButton);
            SDL_zero(szGameButton);
            SDL_zero(szJoystickButton);

        } else if (bGameButton) {
            if (i >= sizeof(szGameButton)) {
                SDL_SetError("Button name too large: %s", szGameButton);
                return;
            }
            szGameButton[i] = *pchPos;
            i++;
        } else {
            if (i >= sizeof(szJoystickButton)) {
                SDL_SetError("Joystick button name too large: %s", szJoystickButton);
                return;
            }
            szJoystickButton[i] = *pchPos;
            i++;
        }
        pchPos++;
    }

    SDL_PrivateGameControllerParseElement(gamecontroller, szGameButton, szJoystickButton);

}

/*
 * Make a new button mapping struct
 */
static void SDL_PrivateLoadButtonMapping(SDL_GameController *gamecontroller, SDL_JoystickGUID guid, const char *pchName, const char *pchMapping)
{
    int i;

    gamecontroller->guid = guid;
    gamecontroller->name = pchName;
    gamecontroller->num_bindings = 0;
    SDL_memset(gamecontroller->last_match_axis, 0, gamecontroller->joystick->naxes * sizeof(*gamecontroller->last_match_axis));

    SDL_PrivateGameControllerParseControllerConfigString(gamecontroller, pchMapping);

    /* Set the zero point for triggers */
    for (i = 0; i < gamecontroller->num_bindings; ++i) {
        SDL_ExtendedGameControllerBind *binding = &gamecontroller->bindings[i];
        if (binding->inputType == SDL_CONTROLLER_BINDTYPE_AXIS &&
            binding->outputType == SDL_CONTROLLER_BINDTYPE_AXIS &&
            (binding->output.axis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT ||
             binding->output.axis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)) {
            if (binding->input.axis.axis < gamecontroller->joystick->naxes) {
                gamecontroller->joystick->axes[binding->input.axis.axis].value =
                gamecontroller->joystick->axes[binding->input.axis.axis].zero = (Sint16)binding->input.axis.axis_min;
            }
        }
    }
}


/*
 * grab the guid string from a mapping string
 */
static char *SDL_PrivateGetControllerGUIDFromMappingString(const char *pMapping)
{
    const char *pFirstComma = SDL_strchr(pMapping, ',');
    if (pFirstComma) {
        char *pchGUID = SDL_malloc(pFirstComma - pMapping + 1);
        if (!pchGUID) {
            SDL_OutOfMemory();
            return NULL;
        }
        SDL_memcpy(pchGUID, pMapping, pFirstComma - pMapping);
        pchGUID[pFirstComma - pMapping] = '\0';

        /* Convert old style GUIDs to the new style in 2.0.5 */
#if __WIN32__
        if (SDL_strlen(pchGUID) == 32 &&
            SDL_memcmp(&pchGUID[20], "504944564944", 12) == 0) {
            SDL_memcpy(&pchGUID[20], "000000000000", 12);
            SDL_memcpy(&pchGUID[16], &pchGUID[4], 4);
            SDL_memcpy(&pchGUID[8], &pchGUID[0], 4);
            SDL_memcpy(&pchGUID[0], "03000000", 8);
        }
#elif __MACOSX__
        if (SDL_strlen(pchGUID) == 32 &&
            SDL_memcmp(&pchGUID[4], "000000000000", 12) == 0 &&
            SDL_memcmp(&pchGUID[20], "000000000000", 12) == 0) {
            SDL_memcpy(&pchGUID[20], "000000000000", 12);
            SDL_memcpy(&pchGUID[8], &pchGUID[0], 4);
            SDL_memcpy(&pchGUID[0], "03000000", 8);
        }
#endif
        return pchGUID;
    }
    return NULL;
}


/*
 * grab the name string from a mapping string
 */
static char *SDL_PrivateGetControllerNameFromMappingString(const char *pMapping)
{
    const char *pFirstComma, *pSecondComma;
    char *pchName;

    pFirstComma = SDL_strchr(pMapping, ',');
    if (!pFirstComma)
        return NULL;

    pSecondComma = SDL_strchr(pFirstComma + 1, ',');
    if (!pSecondComma)
        return NULL;

    pchName = SDL_malloc(pSecondComma - pFirstComma);
    if (!pchName) {
        SDL_OutOfMemory();
        return NULL;
    }
    SDL_memcpy(pchName, pFirstComma + 1, pSecondComma - pFirstComma);
    pchName[pSecondComma - pFirstComma - 1] = 0;
    return pchName;
}


/*
 * grab the button mapping string from a mapping string
 */
static char *SDL_PrivateGetControllerMappingFromMappingString(const char *pMapping)
{
    const char *pFirstComma, *pSecondComma;

    pFirstComma = SDL_strchr(pMapping, ',');
    if (!pFirstComma)
        return NULL;

    pSecondComma = SDL_strchr(pFirstComma + 1, ',');
    if (!pSecondComma)
        return NULL;

    return SDL_strdup(pSecondComma + 1); /* mapping is everything after the 3rd comma */
}

/*
 * Helper function to refresh a mapping
 */
static void SDL_PrivateGameControllerRefreshMapping(ControllerMapping_t *pControllerMapping)
{
    SDL_GameController *gamecontrollerlist = SDL_gamecontrollers;
    while (gamecontrollerlist) {
        if (!SDL_memcmp(&gamecontrollerlist->guid, &pControllerMapping->guid, sizeof(pControllerMapping->guid))) {
            SDL_Event event;
            event.type = SDL_CONTROLLERDEVICEREMAPPED;
            event.cdevice.which = gamecontrollerlist->joystick->instance_id;
            SDL_PushEvent(&event);

            /* Not really threadsafe.  Should this lock access within SDL_GameControllerEventWatcher? */
            SDL_PrivateLoadButtonMapping(gamecontrollerlist, pControllerMapping->guid, pControllerMapping->name, pControllerMapping->mapping);
        }

        gamecontrollerlist = gamecontrollerlist->next;
    }
}

/*
 * Helper function to add a mapping for a guid
 */
static ControllerMapping_t *
SDL_PrivateAddMappingForGUID(SDL_JoystickGUID jGUID, const char *mappingString, SDL_bool *existing, SDL_ControllerMappingPriority priority)
{
    char *pchName;
    char *pchMapping;
    ControllerMapping_t *pControllerMapping;

    pchName = SDL_PrivateGetControllerNameFromMappingString(mappingString);
    if (!pchName) {
        SDL_SetError("Couldn't parse name from %s", mappingString);
        return NULL;
    }

    pchMapping = SDL_PrivateGetControllerMappingFromMappingString(mappingString);
    if (!pchMapping) {
        SDL_free(pchName);
        SDL_SetError("Couldn't parse %s", mappingString);
        return NULL;
    }

    pControllerMapping = SDL_PrivateGetControllerMappingForGUID(&jGUID);
    if (pControllerMapping) {
        /* Only overwrite the mapping if the priority is the same or higher. */
        if (pControllerMapping->priority <= priority) {
            /* Update existing mapping */
            SDL_free(pControllerMapping->name);
            pControllerMapping->name = pchName;
            SDL_free(pControllerMapping->mapping);
            pControllerMapping->mapping = pchMapping;
            pControllerMapping->priority = priority;
            /* refresh open controllers */
            SDL_PrivateGameControllerRefreshMapping(pControllerMapping);
        } else {
            SDL_free(pchName);
            SDL_free(pchMapping);
        }
        *existing = SDL_TRUE;
    } else {
        pControllerMapping = SDL_malloc(sizeof(*pControllerMapping));
        if (!pControllerMapping) {
            SDL_free(pchName);
            SDL_free(pchMapping);
            SDL_OutOfMemory();
            return NULL;
        }
        pControllerMapping->guid = jGUID;
        pControllerMapping->name = pchName;
        pControllerMapping->mapping = pchMapping;
        pControllerMapping->next = NULL;
        pControllerMapping->priority = priority;

        if (s_pSupportedControllers) {
            /* Add the mapping to the end of the list */
            ControllerMapping_t *pCurrMapping, *pPrevMapping;

            for ( pPrevMapping = s_pSupportedControllers, pCurrMapping = pPrevMapping->next;
                  pCurrMapping; 
                  pPrevMapping = pCurrMapping, pCurrMapping = pCurrMapping->next ) {
                continue;
            }
            pPrevMapping->next = pControllerMapping;
        } else {
            s_pSupportedControllers = pControllerMapping;
        }
        *existing = SDL_FALSE;
    }
    return pControllerMapping;
}

/*
 * Helper function to determine pre-calculated offset to certain joystick mappings
 */
static ControllerMapping_t *SDL_PrivateGetControllerMappingForNameAndGUID(const char *name, SDL_JoystickGUID guid)
{
    ControllerMapping_t *mapping;

    mapping = SDL_PrivateGetControllerMappingForGUID(&guid);
#if defined(SDL_JOYSTICK_EMSCRIPTEN)
    if (!mapping && s_pEmscriptenMapping) {
        mapping = s_pEmscriptenMapping;
    }
#else
    (void) s_pEmscriptenMapping;  /* pacify ARMCC */
#endif
#ifdef __LINUX__
    if (!mapping && name) {
        if (SDL_strstr(name, "Xbox 360 Wireless Receiver")) {
            /* The Linux driver xpad.c maps the wireless dpad to buttons */
            SDL_bool existing;
            mapping = SDL_PrivateAddMappingForGUID(guid,
"none,X360 Wireless Controller,a:b0,b:b1,back:b6,dpdown:b14,dpleft:b11,dpright:b12,dpup:b13,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,",
                          &existing, SDL_CONTROLLER_MAPPING_PRIORITY_DEFAULT);
        }
    }
#endif /* __LINUX__ */

    if (!mapping && name) {
        if (SDL_strstr(name, "Xbox") || SDL_strstr(name, "X-Box") || SDL_strstr(name, "XBOX")) {
            mapping = s_pXInputMapping;
        }
    }
    return mapping;
}

static ControllerMapping_t *SDL_PrivateGetControllerMapping(int device_index)
{
    const char *name;
    SDL_JoystickGUID guid;
    ControllerMapping_t *mapping;

    SDL_LockJoysticks();

    if ((device_index < 0) || (device_index >= SDL_NumJoysticks())) {
        SDL_SetError("There are %d joysticks available", SDL_NumJoysticks());
        SDL_UnlockJoysticks();
        return (NULL);
    }

    name = SDL_JoystickNameForIndex(device_index);
    guid = SDL_JoystickGetDeviceGUID(device_index);
    mapping = SDL_PrivateGetControllerMappingForNameAndGUID(name, guid);
#if SDL_JOYSTICK_XINPUT
    if (!mapping && SDL_SYS_IsXInputGamepad_DeviceIndex(device_index)) {
        mapping = s_pXInputMapping;
    }
#endif
#if defined(__ANDROID__)
    if (!mapping && SDL_SYS_IsDPAD_DeviceIndex(device_index)) {
        SDL_bool existing;
        char mapping_string[1024];
        SDL_snprintf(mapping_string, sizeof(mapping_string), "none,%s,a:b0,b:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,", name);
        mapping = SDL_PrivateAddMappingForGUID(guid, mapping_string,
                          &existing, SDL_CONTROLLER_MAPPING_PRIORITY_DEFAULT);
    }
#endif /* __ANDROID__ */
    SDL_UnlockJoysticks();
    return mapping;
}

/*
 * Add or update an entry into the Mappings Database
 */
int
SDL_GameControllerAddMappingsFromRW(SDL_RWops * rw, int freerw)
{
    const char *platform = SDL_GetPlatform();
    int controllers = 0;
    char *buf, *line, *line_end, *tmp, *comma, line_platform[64];
    size_t db_size, platform_len;
    
    if (rw == NULL) {
        return SDL_SetError("Invalid RWops");
    }
    db_size = (size_t)SDL_RWsize(rw);
    
    buf = (char *)SDL_malloc(db_size + 1);
    if (buf == NULL) {
        if (freerw) {
            SDL_RWclose(rw);
        }
        return SDL_SetError("Could not allocate space to read DB into memory");
    }
    
    if (SDL_RWread(rw, buf, db_size, 1) != 1) {
        if (freerw) {
            SDL_RWclose(rw);
        }
        SDL_free(buf);
        return SDL_SetError("Could not read DB");
    }
    
    if (freerw) {
        SDL_RWclose(rw);
    }
    
    buf[db_size] = '\0';
    line = buf;
    
    while (line < buf + db_size) {
        line_end = SDL_strchr(line, '\n');
        if (line_end != NULL) {
            *line_end = '\0';
        } else {
            line_end = buf + db_size;
        }
        
        /* Extract and verify the platform */
        tmp = SDL_strstr(line, SDL_CONTROLLER_PLATFORM_FIELD);
        if (tmp != NULL) {
            tmp += SDL_strlen(SDL_CONTROLLER_PLATFORM_FIELD);
            comma = SDL_strchr(tmp, ',');
            if (comma != NULL) {
                platform_len = comma - tmp + 1;
                if (platform_len + 1 < SDL_arraysize(line_platform)) {
                    SDL_strlcpy(line_platform, tmp, platform_len);
                    if (SDL_strncasecmp(line_platform, platform, platform_len) == 0 &&
                        SDL_GameControllerAddMapping(line) > 0) {
                        controllers++;
                    }
                }
            }
        }
        
        line = line_end + 1;
    }

    SDL_free(buf);
    return controllers;
}

/*
 * Add or update an entry into the Mappings Database with a priority
 */
static int
SDL_PrivateGameControllerAddMapping(const char *mappingString, SDL_ControllerMappingPriority priority)
{
    char *pchGUID;
    SDL_JoystickGUID jGUID;
    SDL_bool is_xinput_mapping = SDL_FALSE;
    SDL_bool is_emscripten_mapping = SDL_FALSE;
    SDL_bool existing = SDL_FALSE;
    ControllerMapping_t *pControllerMapping;

    if (!mappingString) {
        return SDL_InvalidParamError("mappingString");
    }

    pchGUID = SDL_PrivateGetControllerGUIDFromMappingString(mappingString);
    if (!pchGUID) {
        return SDL_SetError("Couldn't parse GUID from %s", mappingString);
    }
    if (!SDL_strcasecmp(pchGUID, "xinput")) {
        is_xinput_mapping = SDL_TRUE;
    }
    if (!SDL_strcasecmp(pchGUID, "emscripten")) {
        is_emscripten_mapping = SDL_TRUE;
    }
    jGUID = SDL_JoystickGetGUIDFromString(pchGUID);
    SDL_free(pchGUID);

    pControllerMapping = SDL_PrivateAddMappingForGUID(jGUID, mappingString, &existing, priority);
    if (!pControllerMapping) {
        return -1;
    }

    if (existing) {
        return 0;
    } else {
        if (is_xinput_mapping) {
            s_pXInputMapping = pControllerMapping;
        }
        if (is_emscripten_mapping) {
            s_pEmscriptenMapping = pControllerMapping;
        }
        return 1;
    }
}

/*
 * Add or update an entry into the Mappings Database
 */
int
SDL_GameControllerAddMapping(const char *mappingString)
{
    return SDL_PrivateGameControllerAddMapping(mappingString, SDL_CONTROLLER_MAPPING_PRIORITY_API);
}

/*
 *  Get the number of mappings installed
 */
int
SDL_GameControllerNumMappings(void)
{
    int num_mappings = 0;
    ControllerMapping_t *mapping;

    for (mapping = s_pSupportedControllers; mapping; mapping = mapping->next) {
        if (SDL_memcmp(&mapping->guid, &s_zeroGUID, sizeof(mapping->guid)) == 0) {
            continue;
        }
        ++num_mappings;
    }
    return num_mappings;
}

/*
 *  Get the mapping at a particular index.
 */
char *
SDL_GameControllerMappingForIndex(int mapping_index)
{
    ControllerMapping_t *mapping;

    for (mapping = s_pSupportedControllers; mapping; mapping = mapping->next) {
        if (SDL_memcmp(&mapping->guid, &s_zeroGUID, sizeof(mapping->guid)) == 0) {
            continue;
        }
        if (mapping_index == 0) {
            char *pMappingString;
            char pchGUID[33];
            size_t needed;

            SDL_JoystickGetGUIDString(mapping->guid, pchGUID, sizeof(pchGUID));
            /* allocate enough memory for GUID + ',' + name + ',' + mapping + \0 */
            needed = SDL_strlen(pchGUID) + 1 + SDL_strlen(mapping->name) + 1 + SDL_strlen(mapping->mapping) + 1;
            pMappingString = SDL_malloc(needed);
            if (!pMappingString) {
                SDL_OutOfMemory();
                return NULL;
            }
            SDL_snprintf(pMappingString, needed, "%s,%s,%s", pchGUID, mapping->name, mapping->mapping);
            return pMappingString;
        }
        --mapping_index;
    }
    return NULL;
}

/*
 * Get the mapping string for this GUID
 */
char *
SDL_GameControllerMappingForGUID(SDL_JoystickGUID guid)
{
    char *pMappingString = NULL;
    ControllerMapping_t *mapping = SDL_PrivateGetControllerMappingForGUID(&guid);
    if (mapping) {
        char pchGUID[33];
        size_t needed;
        SDL_JoystickGetGUIDString(guid, pchGUID, sizeof(pchGUID));
        /* allocate enough memory for GUID + ',' + name + ',' + mapping + \0 */
        needed = SDL_strlen(pchGUID) + 1 + SDL_strlen(mapping->name) + 1 + SDL_strlen(mapping->mapping) + 1;
        pMappingString = SDL_malloc(needed);
        if (!pMappingString) {
            SDL_OutOfMemory();
            return NULL;
        }
        SDL_snprintf(pMappingString, needed, "%s,%s,%s", pchGUID, mapping->name, mapping->mapping);
    }
    return pMappingString;
}

/*
 * Get the mapping string for this device
 */
char *
SDL_GameControllerMapping(SDL_GameController * gamecontroller)
{
    if (!gamecontroller) {
        return NULL;
    }

    return SDL_GameControllerMappingForGUID(gamecontroller->guid);
}

static void
SDL_GameControllerLoadHints()
{
    const char *hint = SDL_GetHint(SDL_HINT_GAMECONTROLLERCONFIG);
    if (hint && hint[0]) {
        size_t nchHints = SDL_strlen(hint);
        char *pUserMappings = SDL_malloc(nchHints + 1);
        char *pTempMappings = pUserMappings;
        SDL_memcpy(pUserMappings, hint, nchHints);
        pUserMappings[nchHints] = '\0';
        while (pUserMappings) {
            char *pchNewLine = NULL;

            pchNewLine = SDL_strchr(pUserMappings, '\n');
            if (pchNewLine)
                *pchNewLine = '\0';

            SDL_PrivateGameControllerAddMapping(pUserMappings, SDL_CONTROLLER_MAPPING_PRIORITY_USER);

            if (pchNewLine) {
                pUserMappings = pchNewLine + 1;
            } else {
                pUserMappings = NULL;
            }
        }
        SDL_free(pTempMappings);
    }
}

/*
 * Fill the given buffer with the expected controller mapping filepath. 
 * Usually this will just be CONTROLLER_MAPPING_FILE, but for Android,
 * we want to get the internal storage path.
 */
static SDL_bool SDL_GetControllerMappingFilePath(char *path, size_t size)
{
#ifdef CONTROLLER_MAPPING_FILE
#define STRING(X) SDL_STRINGIFY_ARG(X)
    return SDL_strlcpy(path, STRING(CONTROLLER_MAPPING_FILE), size) < size;
#elif defined(__ANDROID__)
    return SDL_snprintf(path, size, "%s/controller_map.txt", SDL_AndroidGetInternalStoragePath()) < size;
#else
    return SDL_FALSE;
#endif
}

/*
 * Initialize the game controller system, mostly load our DB of controller config mappings
 */
int
SDL_GameControllerInitMappings(void)
{
    char szControllerMapPath[1024];
    int i = 0;
    const char *pMappingString = NULL;
    pMappingString = s_ControllerMappings[i];
    while (pMappingString) {
        SDL_PrivateGameControllerAddMapping(pMappingString, SDL_CONTROLLER_MAPPING_PRIORITY_DEFAULT);

        i++;
        pMappingString = s_ControllerMappings[i];
    }

    if (SDL_GetControllerMappingFilePath(szControllerMapPath, sizeof(szControllerMapPath))) {
        SDL_GameControllerAddMappingsFromFile(szControllerMapPath);        
    }

    /* load in any user supplied config */
    SDL_GameControllerLoadHints();

    SDL_AddHintCallback(SDL_HINT_GAMECONTROLLER_IGNORE_DEVICES,
                        SDL_GameControllerIgnoreDevicesChanged, NULL);
    SDL_AddHintCallback(SDL_HINT_GAMECONTROLLER_IGNORE_DEVICES_EXCEPT,
                        SDL_GameControllerIgnoreDevicesExceptChanged, NULL);

    return (0);
}

int
SDL_GameControllerInit(void)
{
    int i;

    /* watch for joy events and fire controller ones if needed */
    SDL_AddEventWatch(SDL_GameControllerEventWatcher, NULL);

    /* Send added events for controllers currently attached */
    for (i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            SDL_Event deviceevent;
            deviceevent.type = SDL_CONTROLLERDEVICEADDED;
            deviceevent.cdevice.which = i;
            SDL_PushEvent(&deviceevent);
        }
    }

    return (0);
}


/*
 * Get the implementation dependent name of a controller
 */
const char *
SDL_GameControllerNameForIndex(int device_index)
{
    ControllerMapping_t *pSupportedController = SDL_PrivateGetControllerMapping(device_index);
    if (pSupportedController) {
        return pSupportedController->name;
    }
    return NULL;
}


/*
 * Return 1 if the joystick with this name and GUID is a supported controller
 */
SDL_bool
SDL_IsGameControllerNameAndGUID(const char *name, SDL_JoystickGUID guid)
{
    ControllerMapping_t *pSupportedController = SDL_PrivateGetControllerMappingForNameAndGUID(name, guid);
    if (pSupportedController) {
        return SDL_TRUE;
    }
    return SDL_FALSE;
}

/*
 * Return 1 if the joystick at this device index is a supported controller
 */
SDL_bool
SDL_IsGameController(int device_index)
{
    ControllerMapping_t *pSupportedController = SDL_PrivateGetControllerMapping(device_index);
    if (pSupportedController) {
        return SDL_TRUE;
    }
    return SDL_FALSE;
}

/*
 * Return 1 if the game controller should be ignored by SDL
 */
SDL_bool SDL_ShouldIgnoreGameController(const char *name, SDL_JoystickGUID guid)
{
    int i;
    Uint16 vendor;
    Uint16 product;
    Uint32 vidpid;

    if (SDL_allowed_controllers.num_entries == 0 &&
        SDL_ignored_controllers.num_entries == 0) {
        return SDL_FALSE;
    }

    SDL_GetJoystickGUIDInfo(guid, &vendor, &product, NULL);
    vidpid = MAKE_VIDPID(vendor, product);

    if (SDL_GetHintBoolean("SDL_GAMECONTROLLER_ALLOW_STEAM_VIRTUAL_GAMEPAD", SDL_FALSE)) {
        /* We shouldn't ignore Steam's virtual gamepad since it's using the hints to filter out the real controllers so it can remap input for the virtual controller */
        SDL_bool bSteamVirtualGamepad = SDL_FALSE;
#if defined(__LINUX__)
        bSteamVirtualGamepad = (vendor == 0x28DE && product == 0x11FF);
#elif defined(__MACOSX__)
        bSteamVirtualGamepad = (SDL_strncmp(name, "GamePad-", 8) == 0);
#elif defined(__WIN32__)
        /* We can't tell on Windows, but Steam will block others in input hooks */
        bSteamVirtualGamepad = SDL_TRUE;
#endif
        if (bSteamVirtualGamepad) {
            return SDL_FALSE;
        }
    }

    if (SDL_allowed_controllers.num_entries > 0) {
        for (i = 0; i < SDL_allowed_controllers.num_entries; ++i) {
            if (vidpid == SDL_allowed_controllers.entries[i]) {
                return SDL_FALSE;
            }
        }
        return SDL_TRUE;
    } else {
        for (i = 0; i < SDL_ignored_controllers.num_entries; ++i) {
            if (vidpid == SDL_ignored_controllers.entries[i]) {
                return SDL_TRUE;
            }
        }
        return SDL_FALSE;
    }
}

/*
 * Open a controller for use - the index passed as an argument refers to
 * the N'th controller on the system.  This index is the value which will
 * identify this controller in future controller events.
 *
 * This function returns a controller identifier, or NULL if an error occurred.
 */
SDL_GameController *
SDL_GameControllerOpen(int device_index)
{
    SDL_GameController *gamecontroller;
    SDL_GameController *gamecontrollerlist;
    ControllerMapping_t *pSupportedController = NULL;

    SDL_LockJoysticks();

    if ((device_index < 0) || (device_index >= SDL_NumJoysticks())) {
        SDL_SetError("There are %d joysticks available", SDL_NumJoysticks());
        SDL_UnlockJoysticks();
        return (NULL);
    }

    gamecontrollerlist = SDL_gamecontrollers;
    /* If the controller is already open, return it */
    while (gamecontrollerlist) {
        if (SDL_SYS_GetInstanceIdOfDeviceIndex(device_index) == gamecontrollerlist->joystick->instance_id) {
                gamecontroller = gamecontrollerlist;
                ++gamecontroller->ref_count;
                SDL_UnlockJoysticks();
                return (gamecontroller);
        }
        gamecontrollerlist = gamecontrollerlist->next;
    }

    /* Find a controller mapping */
    pSupportedController =  SDL_PrivateGetControllerMapping(device_index);
    if (!pSupportedController) {
        SDL_SetError("Couldn't find mapping for device (%d)", device_index);
        SDL_UnlockJoysticks();
        return NULL;
    }

    /* Create and initialize the controller */
    gamecontroller = (SDL_GameController *) SDL_calloc(1, sizeof(*gamecontroller));
    if (gamecontroller == NULL) {
        SDL_OutOfMemory();
        SDL_UnlockJoysticks();
        return NULL;
    }

    gamecontroller->joystick = SDL_JoystickOpen(device_index);
    if (!gamecontroller->joystick) {
        SDL_free(gamecontroller);
        SDL_UnlockJoysticks();
        return NULL;
    }

    if (gamecontroller->joystick->naxes) {
        gamecontroller->last_match_axis = (SDL_ExtendedGameControllerBind **)SDL_calloc(gamecontroller->joystick->naxes, sizeof(*gamecontroller->last_match_axis));
        if (!gamecontroller->last_match_axis) {
            SDL_OutOfMemory();
            SDL_JoystickClose(gamecontroller->joystick);
            SDL_free(gamecontroller);
            SDL_UnlockJoysticks();
            return NULL;
        }
    }
    if (gamecontroller->joystick->nhats) {
        gamecontroller->last_hat_mask = (Uint8 *)SDL_calloc(gamecontroller->joystick->nhats, sizeof(*gamecontroller->last_hat_mask));
        if (!gamecontroller->last_hat_mask) {
            SDL_OutOfMemory();
            SDL_JoystickClose(gamecontroller->joystick);
            SDL_free(gamecontroller->last_match_axis);
            SDL_free(gamecontroller);
            SDL_UnlockJoysticks();
            return NULL;
        }
    }

    SDL_PrivateLoadButtonMapping(gamecontroller, pSupportedController->guid, pSupportedController->name, pSupportedController->mapping);

    /* Add the controller to list */
    ++gamecontroller->ref_count;
    /* Link the controller in the list */
    gamecontroller->next = SDL_gamecontrollers;
    SDL_gamecontrollers = gamecontroller;

    SDL_UnlockJoysticks();

    return (gamecontroller);
}

/*
 * Manually pump for controller updates.
 */
void
SDL_GameControllerUpdate(void)
{
    /* Just for API completeness; the joystick API does all the work. */
    SDL_JoystickUpdate();
}

/*
 * Get the current state of an axis control on a controller
 */
Sint16
SDL_GameControllerGetAxis(SDL_GameController * gamecontroller, SDL_GameControllerAxis axis)
{
    int i;

    if (!gamecontroller)
        return 0;

    for (i = 0; i < gamecontroller->num_bindings; ++i) {
        SDL_ExtendedGameControllerBind *binding = &gamecontroller->bindings[i];
        if (binding->outputType == SDL_CONTROLLER_BINDTYPE_AXIS && binding->output.axis.axis == axis) {
            int value = 0;
            SDL_bool valid_input_range;
            SDL_bool valid_output_range;

            if (binding->inputType == SDL_CONTROLLER_BINDTYPE_AXIS) {
                value = SDL_JoystickGetAxis(gamecontroller->joystick, binding->input.axis.axis);
                if (binding->input.axis.axis_min < binding->input.axis.axis_max) {
                    valid_input_range = (value >= binding->input.axis.axis_min && value <= binding->input.axis.axis_max);
                } else {
                    valid_input_range = (value >= binding->input.axis.axis_max && value <= binding->input.axis.axis_min);
                }
                if (valid_input_range) {
                    if (binding->input.axis.axis_min != binding->output.axis.axis_min || binding->input.axis.axis_max != binding->output.axis.axis_max) {
                        float normalized_value = (float)(value - binding->input.axis.axis_min) / (binding->input.axis.axis_max - binding->input.axis.axis_min);
                        value = binding->output.axis.axis_min + (int)(normalized_value * (binding->output.axis.axis_max - binding->output.axis.axis_min));
                    }
                }
            } else if (binding->inputType == SDL_CONTROLLER_BINDTYPE_BUTTON) {
                value = SDL_JoystickGetButton(gamecontroller->joystick, binding->input.button);
                if (value == SDL_PRESSED) {
                    value = binding->output.axis.axis_max;
                }
            } else if (binding->inputType == SDL_CONTROLLER_BINDTYPE_HAT) {
                int hat_mask = SDL_JoystickGetHat(gamecontroller->joystick, binding->input.hat.hat);
                if (hat_mask & binding->input.hat.hat_mask) {
                    value = binding->output.axis.axis_max;
                }
            }

            if (binding->output.axis.axis_min < binding->output.axis.axis_max) {
                valid_output_range = (value >= binding->output.axis.axis_min && value <= binding->output.axis.axis_max);
            } else {
                valid_output_range = (value >= binding->output.axis.axis_max && value <= binding->output.axis.axis_min);
            }
            /* If the value is zero, there might be another binding that makes it non-zero */
            if (value != 0 && valid_output_range) {
                return (Sint16)value;
            }
        }
    }
    return 0;
}

/*
 * Get the current state of a button on a controller
 */
Uint8
SDL_GameControllerGetButton(SDL_GameController * gamecontroller, SDL_GameControllerButton button)
{
    int i;

    if (!gamecontroller)
        return 0;

    for (i = 0; i < gamecontroller->num_bindings; ++i) {
        SDL_ExtendedGameControllerBind *binding = &gamecontroller->bindings[i];
        if (binding->outputType == SDL_CONTROLLER_BINDTYPE_BUTTON && binding->output.button == button) {
            if (binding->inputType == SDL_CONTROLLER_BINDTYPE_AXIS) {
                SDL_bool valid_input_range;

                int value = SDL_JoystickGetAxis(gamecontroller->joystick, binding->input.axis.axis);
                int threshold = binding->input.axis.axis_min + (binding->input.axis.axis_max - binding->input.axis.axis_min) / 2;
                if (binding->input.axis.axis_min < binding->input.axis.axis_max) {
                    valid_input_range = (value >= binding->input.axis.axis_min && value <= binding->input.axis.axis_max);
                    if (valid_input_range) {
                        return (value >= threshold) ? SDL_PRESSED : SDL_RELEASED;
                    }
                } else {
                    valid_input_range = (value >= binding->input.axis.axis_max && value <= binding->input.axis.axis_min);
                    if (valid_input_range) {
                        return (value <= threshold) ? SDL_PRESSED : SDL_RELEASED;
                    }
                }
            } else if (binding->inputType == SDL_CONTROLLER_BINDTYPE_BUTTON) {
                return SDL_JoystickGetButton(gamecontroller->joystick, binding->input.button);
            } else if (binding->inputType == SDL_CONTROLLER_BINDTYPE_HAT) {
                int hat_mask = SDL_JoystickGetHat(gamecontroller->joystick, binding->input.hat.hat);
                return (hat_mask & binding->input.hat.hat_mask) ? SDL_PRESSED : SDL_RELEASED;
            }
        }
    }
    return SDL_RELEASED;
}

const char *
SDL_GameControllerName(SDL_GameController * gamecontroller)
{
    if (!gamecontroller)
        return NULL;

    return gamecontroller->name;
}

Uint16
SDL_GameControllerGetVendor(SDL_GameController * gamecontroller)
{
    return SDL_JoystickGetVendor(SDL_GameControllerGetJoystick(gamecontroller));
}

Uint16
SDL_GameControllerGetProduct(SDL_GameController * gamecontroller)
{
    return SDL_JoystickGetProduct(SDL_GameControllerGetJoystick(gamecontroller));
}

Uint16
SDL_GameControllerGetProductVersion(SDL_GameController * gamecontroller)
{
    return SDL_JoystickGetProductVersion(SDL_GameControllerGetJoystick(gamecontroller));
}

/*
 * Return if the controller in question is currently attached to the system,
 *  \return 0 if not plugged in, 1 if still present.
 */
SDL_bool
SDL_GameControllerGetAttached(SDL_GameController * gamecontroller)
{
    if (!gamecontroller)
        return SDL_FALSE;

    return SDL_JoystickGetAttached(gamecontroller->joystick);
}

/*
 * Get the joystick for this controller
 */
SDL_Joystick *SDL_GameControllerGetJoystick(SDL_GameController * gamecontroller)
{
    if (!gamecontroller)
        return NULL;

    return gamecontroller->joystick;
}


/*
 * Find the SDL_GameController that owns this instance id
 */
SDL_GameController *
SDL_GameControllerFromInstanceID(SDL_JoystickID joyid)
{
    SDL_GameController *gamecontroller;

    SDL_LockJoysticks();
    gamecontroller = SDL_gamecontrollers;
    while (gamecontroller) {
        if (gamecontroller->joystick->instance_id == joyid) {
            SDL_UnlockJoysticks();
            return gamecontroller;
        }
        gamecontroller = gamecontroller->next;
    }
    SDL_UnlockJoysticks();
    return NULL;
}


/*
 * Get the SDL joystick layer binding for this controller axis mapping
 */
SDL_GameControllerButtonBind SDL_GameControllerGetBindForAxis(SDL_GameController * gamecontroller, SDL_GameControllerAxis axis)
{
    int i;
    SDL_GameControllerButtonBind bind;
    SDL_zero(bind);

    if (!gamecontroller || axis == SDL_CONTROLLER_AXIS_INVALID)
        return bind;

    for (i = 0; i < gamecontroller->num_bindings; ++i) {
        SDL_ExtendedGameControllerBind *binding = &gamecontroller->bindings[i];
        if (binding->outputType == SDL_CONTROLLER_BINDTYPE_AXIS && binding->output.axis.axis == axis) {
            bind.bindType = binding->inputType;
            if (binding->inputType == SDL_CONTROLLER_BINDTYPE_AXIS) {
                /* FIXME: There might be multiple axes bound now that we have axis ranges... */
                bind.value.axis = binding->input.axis.axis;
            } else if (binding->inputType == SDL_CONTROLLER_BINDTYPE_BUTTON) {
                bind.value.button = binding->input.button;
            } else if (binding->inputType == SDL_CONTROLLER_BINDTYPE_HAT) {
                bind.value.hat.hat = binding->input.hat.hat;
                bind.value.hat.hat_mask = binding->input.hat.hat_mask;
            }
            break;
        }
    }
    return bind;
}


/*
 * Get the SDL joystick layer binding for this controller button mapping
 */
SDL_GameControllerButtonBind SDL_GameControllerGetBindForButton(SDL_GameController * gamecontroller, SDL_GameControllerButton button)
{
    int i;
    SDL_GameControllerButtonBind bind;
    SDL_zero(bind);

    if (!gamecontroller || button == SDL_CONTROLLER_BUTTON_INVALID)
        return bind;

    for (i = 0; i < gamecontroller->num_bindings; ++i) {
        SDL_ExtendedGameControllerBind *binding = &gamecontroller->bindings[i];
        if (binding->outputType == SDL_CONTROLLER_BINDTYPE_BUTTON && binding->output.button == button) {
            bind.bindType = binding->inputType;
            if (binding->inputType == SDL_CONTROLLER_BINDTYPE_AXIS) {
                bind.value.axis = binding->input.axis.axis;
            } else if (binding->inputType == SDL_CONTROLLER_BINDTYPE_BUTTON) {
                bind.value.button = binding->input.button;
            } else if (binding->inputType == SDL_CONTROLLER_BINDTYPE_HAT) {
                bind.value.hat.hat = binding->input.hat.hat;
                bind.value.hat.hat_mask = binding->input.hat.hat_mask;
            }
            break;
        }
    }
    return bind;
}


void
SDL_GameControllerClose(SDL_GameController * gamecontroller)
{
    SDL_GameController *gamecontrollerlist, *gamecontrollerlistprev;

    if (!gamecontroller)
        return;

    SDL_LockJoysticks();

    /* First decrement ref count */
    if (--gamecontroller->ref_count > 0) {
        SDL_UnlockJoysticks();
        return;
    }

    SDL_JoystickClose(gamecontroller->joystick);

    gamecontrollerlist = SDL_gamecontrollers;
    gamecontrollerlistprev = NULL;
    while (gamecontrollerlist) {
        if (gamecontroller == gamecontrollerlist) {
            if (gamecontrollerlistprev) {
                /* unlink this entry */
                gamecontrollerlistprev->next = gamecontrollerlist->next;
            } else {
                SDL_gamecontrollers = gamecontroller->next;
            }
            break;
        }
        gamecontrollerlistprev = gamecontrollerlist;
        gamecontrollerlist = gamecontrollerlist->next;
    }

    SDL_free(gamecontroller->bindings);
    SDL_free(gamecontroller->last_match_axis);
    SDL_free(gamecontroller->last_hat_mask);
    SDL_free(gamecontroller);

    SDL_UnlockJoysticks();
}


/*
 * Quit the controller subsystem
 */
void
SDL_GameControllerQuit(void)
{
    SDL_LockJoysticks();
    while (SDL_gamecontrollers) {
        SDL_gamecontrollers->ref_count = 1;
        SDL_GameControllerClose(SDL_gamecontrollers);
    }
    SDL_UnlockJoysticks();
}

void
SDL_GameControllerQuitMappings(void)
{
    ControllerMapping_t *pControllerMap;

    while (s_pSupportedControllers) {
        pControllerMap = s_pSupportedControllers;
        s_pSupportedControllers = s_pSupportedControllers->next;
        SDL_free(pControllerMap->name);
        SDL_free(pControllerMap->mapping);
        SDL_free(pControllerMap);
    }

    SDL_DelEventWatch(SDL_GameControllerEventWatcher, NULL);

    SDL_DelHintCallback(SDL_HINT_GAMECONTROLLER_IGNORE_DEVICES,
                        SDL_GameControllerIgnoreDevicesChanged, NULL);
    SDL_DelHintCallback(SDL_HINT_GAMECONTROLLER_IGNORE_DEVICES_EXCEPT,
                        SDL_GameControllerIgnoreDevicesExceptChanged, NULL);

    if (SDL_allowed_controllers.entries) {
        SDL_free(SDL_allowed_controllers.entries);
        SDL_zero(SDL_allowed_controllers);
    }
    if (SDL_ignored_controllers.entries) {
        SDL_free(SDL_ignored_controllers.entries);
        SDL_zero(SDL_ignored_controllers);
    }
}

/*
 * Event filter to transform joystick events into appropriate game controller ones
 */
static int
SDL_PrivateGameControllerAxis(SDL_GameController * gamecontroller, SDL_GameControllerAxis axis, Sint16 value)
{
    int posted;

    /* translate the event, if desired */
    posted = 0;
#if !SDL_EVENTS_DISABLED
    if (SDL_GetEventState(SDL_CONTROLLERAXISMOTION) == SDL_ENABLE) {
        SDL_Event event;
        event.type = SDL_CONTROLLERAXISMOTION;
        event.caxis.which = gamecontroller->joystick->instance_id;
        event.caxis.axis = axis;
        event.caxis.value = value;
        posted = SDL_PushEvent(&event) == 1;
    }
#endif /* !SDL_EVENTS_DISABLED */
    return (posted);
}


/*
 * Event filter to transform joystick events into appropriate game controller ones
 */
static int
SDL_PrivateGameControllerButton(SDL_GameController * gamecontroller, SDL_GameControllerButton button, Uint8 state)
{
    int posted;
#if !SDL_EVENTS_DISABLED
    SDL_Event event;

    if (button == SDL_CONTROLLER_BUTTON_INVALID)
        return (0);

    switch (state) {
    case SDL_PRESSED:
        event.type = SDL_CONTROLLERBUTTONDOWN;
        break;
    case SDL_RELEASED:
        event.type = SDL_CONTROLLERBUTTONUP;
        break;
    default:
        /* Invalid state -- bail */
        return (0);
    }
#endif /* !SDL_EVENTS_DISABLED */

    /* translate the event, if desired */
    posted = 0;
#if !SDL_EVENTS_DISABLED
    if (SDL_GetEventState(event.type) == SDL_ENABLE) {
        event.cbutton.which = gamecontroller->joystick->instance_id;
        event.cbutton.button = button;
        event.cbutton.state = state;
        posted = SDL_PushEvent(&event) == 1;
    }
#endif /* !SDL_EVENTS_DISABLED */
    return (posted);
}

/*
 * Turn off controller events
 */
int
SDL_GameControllerEventState(int state)
{
#if SDL_EVENTS_DISABLED
    return SDL_IGNORE;
#else
    const Uint32 event_list[] = {
        SDL_CONTROLLERAXISMOTION, SDL_CONTROLLERBUTTONDOWN, SDL_CONTROLLERBUTTONUP,
        SDL_CONTROLLERDEVICEADDED, SDL_CONTROLLERDEVICEREMOVED, SDL_CONTROLLERDEVICEREMAPPED,
    };
    unsigned int i;

    switch (state) {
    case SDL_QUERY:
        state = SDL_IGNORE;
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

/* vi: set ts=4 sw=4 expandtab: */
