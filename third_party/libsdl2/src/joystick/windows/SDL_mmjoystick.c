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

#ifdef SDL_JOYSTICK_WINMM

/* Win32 MultiMedia Joystick driver, contributed by Andrei de A. Formiga */

#include "../../core/windows/SDL_windows.h"
#include <mmsystem.h>
#include <regstr.h>

#include "SDL_events.h"
#include "SDL_joystick.h"
#include "../SDL_sysjoystick.h"
#include "../SDL_joystick_c.h"

#ifdef REGSTR_VAL_JOYOEMNAME 
#undef REGSTR_VAL_JOYOEMNAME 
#endif
#define REGSTR_VAL_JOYOEMNAME "OEMName"

#define MAX_JOYSTICKS   16
#define MAX_AXES    6       /* each joystick can have up to 6 axes */
#define MAX_BUTTONS 32      /* and 32 buttons                      */
#define JOY_BUTTON_FLAG(n)  (1<<n)


/* array to hold joystick ID values */
static UINT SYS_JoystickID[MAX_JOYSTICKS];
static JOYCAPSA SYS_Joystick[MAX_JOYSTICKS];
static char *SYS_JoystickName[MAX_JOYSTICKS];

/* The private structure used to keep track of a joystick */
struct joystick_hwdata
{
    /* joystick ID */
    UINT id;

    /* values used to translate device-specific coordinates into
       SDL-standard ranges */
    struct _transaxis
    {
        int offset;
        float scale;
    } transaxis[6];
};

/* Convert a Windows Multimedia API return code to a text message */
static void SetMMerror(char *function, int code);


static char *
GetJoystickName(int index, const char *szRegKey)
{
    /* added 7/24/2004 by Eckhard Stolberg */
    /*
       see if there is a joystick for the current
       index (1-16) listed in the registry
     */
    char *name = NULL;
    HKEY hTopKey;
    HKEY hKey;
    DWORD regsize;
    LONG regresult;
    char regkey[256];
    char regvalue[256];
    char regname[256];

    SDL_snprintf(regkey, SDL_arraysize(regkey),
#ifdef UNICODE
                 "%S\\%s\\%S",
#else
                 "%s\\%s\\%s",
#endif
                 REGSTR_PATH_JOYCONFIG, szRegKey, REGSTR_KEY_JOYCURR);
    hTopKey = HKEY_LOCAL_MACHINE;
    regresult = RegOpenKeyExA(hTopKey, regkey, 0, KEY_READ, &hKey);
    if (regresult != ERROR_SUCCESS) {
        hTopKey = HKEY_CURRENT_USER;
        regresult = RegOpenKeyExA(hTopKey, regkey, 0, KEY_READ, &hKey);
    }
    if (regresult != ERROR_SUCCESS) {
        return NULL;
    }

    /* find the registry key name for the joystick's properties */
    regsize = sizeof(regname);
    SDL_snprintf(regvalue, SDL_arraysize(regvalue), "Joystick%d%s", index + 1,
                 REGSTR_VAL_JOYOEMNAME);
    regresult =
        RegQueryValueExA(hKey, regvalue, 0, 0, (LPBYTE) regname, &regsize);
    RegCloseKey(hKey);

    if (regresult != ERROR_SUCCESS) {
        return NULL;
    }

    /* open that registry key */
    SDL_snprintf(regkey, SDL_arraysize(regkey),
#ifdef UNICODE
                 "%S\\%s",
#else
                 "%s\\%s",
#endif
                 REGSTR_PATH_JOYOEM, regname);
    regresult = RegOpenKeyExA(hTopKey, regkey, 0, KEY_READ, &hKey);
    if (regresult != ERROR_SUCCESS) {
        return NULL;
    }

    /* find the size for the OEM name text */
    regsize = sizeof(regvalue);
    regresult =
        RegQueryValueExA(hKey, REGSTR_VAL_JOYOEMNAME, 0, 0, NULL, &regsize);
    if (regresult == ERROR_SUCCESS) {
        /* allocate enough memory for the OEM name text ... */
        name = (char *) SDL_malloc(regsize);
        if (name) {
            /* ... and read it from the registry */
            regresult = RegQueryValueExA(hKey,
                                         REGSTR_VAL_JOYOEMNAME, 0, 0,
                                         (LPBYTE) name, &regsize);
        }
    }
    RegCloseKey(hKey);

    return (name);
}

static int numjoysticks = 0;

/* Function to scan the system for joysticks.
 * Joystick 0 should be the system default joystick.
 * It should return 0, or -1 on an unrecoverable fatal error.
 */
static int
WINMM_JoystickInit(void)
{
    int i;
    int maxdevs;
    JOYINFOEX joyinfo;
    JOYCAPSA joycaps;
    MMRESULT result;

    /* Reset the joystick ID & name mapping tables */
    for (i = 0; i < MAX_JOYSTICKS; ++i) {
        SYS_JoystickID[i] = 0;
        SYS_JoystickName[i] = NULL;
    }

    /* Loop over all potential joystick devices */
    numjoysticks = 0;
    maxdevs = joyGetNumDevs();
    for (i = JOYSTICKID1; i < maxdevs && numjoysticks < MAX_JOYSTICKS; ++i) {

        joyinfo.dwSize = sizeof(joyinfo);
        joyinfo.dwFlags = JOY_RETURNALL;
        result = joyGetPosEx(i, &joyinfo);
        if (result == JOYERR_NOERROR) {
            result = joyGetDevCapsA(i, &joycaps, sizeof(joycaps));
            if (result == JOYERR_NOERROR) {
                SYS_JoystickID[numjoysticks] = i;
                SYS_Joystick[numjoysticks] = joycaps;
                SYS_JoystickName[numjoysticks] =
                    GetJoystickName(i, joycaps.szRegKey);
                numjoysticks++;
            }
        }
    }
    return numjoysticks;
}

static int
WINMM_NumJoysticks(void)
{
    return numjoysticks;
}

static void
WINMM_JoystickDetect(void)
{
}

/* Function to get the device-dependent name of a joystick */
static const char *
WINMM_JoystickGetDeviceName(int device_index)
{
    if (SYS_JoystickName[device_index] != NULL) {
        return (SYS_JoystickName[device_index]);
    } else {
        return (SYS_Joystick[device_index].szPname);
    }
}

static int
WINMM_JoystickGetDevicePlayerIndex(int device_index)
{
    return -1;
}

static void
WINMM_JoystickSetDevicePlayerIndex(int device_index, int player_index)
{
}

static SDL_JoystickGUID WINMM_JoystickGetDeviceGUID(int device_index)
{
    SDL_JoystickGUID guid;
    /* the GUID is just the first 16 chars of the name for now */
    const char *name = WINMM_JoystickGetDeviceName(device_index);
    SDL_zero(guid);
    SDL_memcpy(&guid, name, SDL_min(sizeof(guid), SDL_strlen(name)));
    return guid;
}

/* Function to perform the mapping from device index to the instance id for this index */
static SDL_JoystickID WINMM_JoystickGetDeviceInstanceID(int device_index)
{
    return device_index;
}

/* Function to open a joystick for use.
   The joystick to open is specified by the device index.
   This should fill the nbuttons and naxes fields of the joystick structure.
   It returns 0, or -1 if there is an error.
 */
static int
WINMM_JoystickOpen(SDL_Joystick *joystick, int device_index)
{
    int index, i;
    int caps_flags[MAX_AXES - 2] =
        { JOYCAPS_HASZ, JOYCAPS_HASR, JOYCAPS_HASU, JOYCAPS_HASV };
    int axis_min[MAX_AXES], axis_max[MAX_AXES];

    /* shortcut */
    index = device_index;
    axis_min[0] = SYS_Joystick[index].wXmin;
    axis_max[0] = SYS_Joystick[index].wXmax;
    axis_min[1] = SYS_Joystick[index].wYmin;
    axis_max[1] = SYS_Joystick[index].wYmax;
    axis_min[2] = SYS_Joystick[index].wZmin;
    axis_max[2] = SYS_Joystick[index].wZmax;
    axis_min[3] = SYS_Joystick[index].wRmin;
    axis_max[3] = SYS_Joystick[index].wRmax;
    axis_min[4] = SYS_Joystick[index].wUmin;
    axis_max[4] = SYS_Joystick[index].wUmax;
    axis_min[5] = SYS_Joystick[index].wVmin;
    axis_max[5] = SYS_Joystick[index].wVmax;

    /* allocate memory for system specific hardware data */
    joystick->instance_id = device_index;
    joystick->hwdata =
        (struct joystick_hwdata *) SDL_malloc(sizeof(*joystick->hwdata));
    if (joystick->hwdata == NULL) {
        return SDL_OutOfMemory();
    }
    SDL_memset(joystick->hwdata, 0, sizeof(*joystick->hwdata));

    /* set hardware data */
    joystick->hwdata->id = SYS_JoystickID[index];
    for (i = 0; i < MAX_AXES; ++i) {
        if ((i < 2) || (SYS_Joystick[index].wCaps & caps_flags[i - 2])) {
            joystick->hwdata->transaxis[i].offset = SDL_JOYSTICK_AXIS_MIN - axis_min[i];
            joystick->hwdata->transaxis[i].scale =
                (float) (SDL_JOYSTICK_AXIS_MAX - SDL_JOYSTICK_AXIS_MIN) / (axis_max[i] - axis_min[i]);
        } else {
            joystick->hwdata->transaxis[i].offset = 0;
            joystick->hwdata->transaxis[i].scale = 1.0; /* Just in case */
        }
    }

    /* fill nbuttons, naxes, and nhats fields */
    joystick->nbuttons = SYS_Joystick[index].wNumButtons;
    joystick->naxes = SYS_Joystick[index].wNumAxes;
    if (SYS_Joystick[index].wCaps & JOYCAPS_HASPOV) {
        joystick->nhats = 1;
    } else {
        joystick->nhats = 0;
    }
    return (0);
}

static Uint8
TranslatePOV(DWORD value)
{
    Uint8 pos;

    pos = SDL_HAT_CENTERED;
    if (value != JOY_POVCENTERED) {
        if ((value > JOY_POVLEFT) || (value < JOY_POVRIGHT)) {
            pos |= SDL_HAT_UP;
        }
        if ((value > JOY_POVFORWARD) && (value < JOY_POVBACKWARD)) {
            pos |= SDL_HAT_RIGHT;
        }
        if ((value > JOY_POVRIGHT) && (value < JOY_POVLEFT)) {
            pos |= SDL_HAT_DOWN;
        }
        if (value > JOY_POVBACKWARD) {
            pos |= SDL_HAT_LEFT;
        }
    }
    return (pos);
}

static int
WINMM_JoystickRumble(SDL_Joystick *joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble)
{
    return SDL_Unsupported();
}

static int
WINMM_JoystickRumbleTriggers(SDL_Joystick *joystick, Uint16 left_rumble, Uint16 right_rumble)
{
    return SDL_Unsupported();
}

static SDL_bool WINMM_JoystickHasLED(SDL_Joystick *joystick)
{
    return SDL_FALSE;
}

static int
WINMM_JoystickSetLED(SDL_Joystick *joystick, Uint8 red, Uint8 green, Uint8 blue)
{
    return SDL_Unsupported();
}

static int
WINMM_JoystickSendEffect(SDL_Joystick *joystick, const void *data, int size)
{
    return SDL_Unsupported();
}

static int WINMM_JoystickSetSensorsEnabled(SDL_Joystick *joystick, SDL_bool enabled)
{
    return SDL_Unsupported();
}

/* Function to update the state of a joystick - called as a device poll.
 * This function shouldn't update the joystick structure directly,
 * but instead should call SDL_PrivateJoystick*() to deliver events
 * and update joystick device state.
 */
static void
WINMM_JoystickUpdate(SDL_Joystick *joystick)
{
    MMRESULT result;
    int i;
    DWORD flags[MAX_AXES] = { JOY_RETURNX, JOY_RETURNY, JOY_RETURNZ,
                              JOY_RETURNR, JOY_RETURNU, JOY_RETURNV
    };
    DWORD pos[MAX_AXES];
    struct _transaxis *transaxis;
    int value;
    JOYINFOEX joyinfo;

    joyinfo.dwSize = sizeof(joyinfo);
    joyinfo.dwFlags = JOY_RETURNALL | JOY_RETURNPOVCTS;
    if (!joystick->hats) {
        joyinfo.dwFlags &= ~(JOY_RETURNPOV | JOY_RETURNPOVCTS);
    }
    result = joyGetPosEx(joystick->hwdata->id, &joyinfo);
    if (result != JOYERR_NOERROR) {
        SetMMerror("joyGetPosEx", result);
        return;
    }

    /* joystick motion events */
    pos[0] = joyinfo.dwXpos;
    pos[1] = joyinfo.dwYpos;
    pos[2] = joyinfo.dwZpos;
    pos[3] = joyinfo.dwRpos;
    pos[4] = joyinfo.dwUpos;
    pos[5] = joyinfo.dwVpos;

    transaxis = joystick->hwdata->transaxis;
    for (i = 0; i < joystick->naxes; i++) {
        if (joyinfo.dwFlags & flags[i]) {
            value = (int) (((float) pos[i] + transaxis[i].offset) * transaxis[i].scale);
            SDL_PrivateJoystickAxis(joystick, (Uint8) i, (Sint16) value);
        }
    }

    /* joystick button events */
    if (joyinfo.dwFlags & JOY_RETURNBUTTONS) {
        for (i = 0; i < joystick->nbuttons; ++i) {
            if (joyinfo.dwButtons & JOY_BUTTON_FLAG(i)) {
                SDL_PrivateJoystickButton(joystick, (Uint8) i, SDL_PRESSED);
            } else {
                SDL_PrivateJoystickButton(joystick, (Uint8) i, SDL_RELEASED);
            }
        }
    }

    /* joystick hat events */
    if (joyinfo.dwFlags & JOY_RETURNPOV) {
        SDL_PrivateJoystickHat(joystick, 0, TranslatePOV(joyinfo.dwPOV));
    }
}

/* Function to close a joystick after use */
static void
WINMM_JoystickClose(SDL_Joystick *joystick)
{
    SDL_free(joystick->hwdata);
}

/* Function to perform any system-specific joystick related cleanup */
static void
WINMM_JoystickQuit(void)
{
    int i;
    for (i = 0; i < MAX_JOYSTICKS; i++) {
        SDL_free(SYS_JoystickName[i]);
        SYS_JoystickName[i] = NULL;
    }
}

static SDL_bool
WINMM_JoystickGetGamepadMapping(int device_index, SDL_GamepadMapping *out)
{
    return SDL_FALSE;
}


/* implementation functions */
void
SetMMerror(char *function, int code)
{
    static char *error;
    static char errbuf[1024];

    errbuf[0] = 0;
    switch (code) {
    case MMSYSERR_NODRIVER:
        error = "Joystick driver not present";
        break;

    case MMSYSERR_INVALPARAM:
    case JOYERR_PARMS:
        error = "Invalid parameter(s)";
        break;

    case MMSYSERR_BADDEVICEID:
        error = "Bad device ID";
        break;

    case JOYERR_UNPLUGGED:
        error = "Joystick not attached";
        break;

    case JOYERR_NOCANDO:
        error = "Can't capture joystick input";
        break;

    default:
        SDL_snprintf(errbuf, SDL_arraysize(errbuf),
                     "%s: Unknown Multimedia system error: 0x%x",
                     function, code);
        break;
    }

    if (!errbuf[0]) {
        SDL_snprintf(errbuf, SDL_arraysize(errbuf), "%s: %s", function,
                     error);
    }
    SDL_SetError("%s", errbuf);
}

SDL_JoystickDriver SDL_WINMM_JoystickDriver =
{
    WINMM_JoystickInit,
    WINMM_NumJoysticks,
    WINMM_JoystickDetect,
    WINMM_JoystickGetDeviceName,
    WINMM_JoystickGetDevicePlayerIndex,
    WINMM_JoystickSetDevicePlayerIndex,
    WINMM_JoystickGetDeviceGUID,
    WINMM_JoystickGetDeviceInstanceID,
    WINMM_JoystickOpen,
    WINMM_JoystickRumble,
    WINMM_JoystickRumbleTriggers,
    WINMM_JoystickHasLED,
    WINMM_JoystickSetLED,
    WINMM_JoystickSendEffect,
    WINMM_JoystickSetSensorsEnabled,
    WINMM_JoystickUpdate,
    WINMM_JoystickClose,
    WINMM_JoystickQuit,
    WINMM_JoystickGetGamepadMapping
};

#endif /* SDL_JOYSTICK_WINMM */

/* vi: set ts=4 sw=4 expandtab: */
