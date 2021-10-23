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

#include "../SDL_sysjoystick.h"

#if SDL_JOYSTICK_XINPUT

#include "SDL_hints.h"
#include "SDL_timer.h"
#include "SDL_windowsjoystick_c.h"
#include "SDL_xinputjoystick_c.h"
#include "SDL_rawinputjoystick_c.h"
#include "../hidapi/SDL_hidapijoystick_c.h"

/*
 * Internal stuff.
 */
static SDL_bool s_bXInputEnabled = SDL_TRUE;
static char *s_arrXInputDevicePath[XUSER_MAX_COUNT];


static SDL_bool
SDL_XInputUseOldJoystickMapping()
{
#ifdef __WINRT__
    /* TODO: remove this __WINRT__ block, but only after integrating with UWP/WinRT's HID API */
    /* FIXME: Why are Win8/10 different here? -flibit */
    return (NTDDI_VERSION < NTDDI_WIN10);
#else
    static int s_XInputUseOldJoystickMapping = -1;
    if (s_XInputUseOldJoystickMapping < 0) {
        s_XInputUseOldJoystickMapping = SDL_GetHintBoolean(SDL_HINT_XINPUT_USE_OLD_JOYSTICK_MAPPING, SDL_FALSE);
    }
    return (s_XInputUseOldJoystickMapping > 0);
#endif
}

SDL_bool SDL_XINPUT_Enabled(void)
{
    return s_bXInputEnabled;
}

int
SDL_XINPUT_JoystickInit(void)
{
    s_bXInputEnabled = SDL_GetHintBoolean(SDL_HINT_XINPUT_ENABLED, SDL_TRUE);

#ifdef SDL_JOYSTICK_RAWINPUT
    if (RAWINPUT_IsEnabled()) {
        /* The raw input driver handles more than 4 controllers, so prefer that when available */
        s_bXInputEnabled = SDL_FALSE;
    }
#endif

    if (s_bXInputEnabled && WIN_LoadXInputDLL() < 0) {
        s_bXInputEnabled = SDL_FALSE;  /* oh well. */
    }
    return 0;
}

static const char *
GetXInputName(const Uint8 userid, BYTE SubType)
{
    static char name[32];

    if (SDL_XInputUseOldJoystickMapping()) {
        SDL_snprintf(name, sizeof(name), "X360 Controller #%u", 1 + userid);
    } else {
        switch (SubType) {
        case XINPUT_DEVSUBTYPE_GAMEPAD:
            SDL_snprintf(name, sizeof(name), "XInput Controller #%u", 1 + userid);
            break;
        case XINPUT_DEVSUBTYPE_WHEEL:
            SDL_snprintf(name, sizeof(name), "XInput Wheel #%u", 1 + userid);
            break;
        case XINPUT_DEVSUBTYPE_ARCADE_STICK:
            SDL_snprintf(name, sizeof(name), "XInput ArcadeStick #%u", 1 + userid);
            break;
        case XINPUT_DEVSUBTYPE_FLIGHT_STICK:
            SDL_snprintf(name, sizeof(name), "XInput FlightStick #%u", 1 + userid);
            break;
        case XINPUT_DEVSUBTYPE_DANCE_PAD:
            SDL_snprintf(name, sizeof(name), "XInput DancePad #%u", 1 + userid);
            break;
        case XINPUT_DEVSUBTYPE_GUITAR:
        case XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE:
        case XINPUT_DEVSUBTYPE_GUITAR_BASS:
            SDL_snprintf(name, sizeof(name), "XInput Guitar #%u", 1 + userid);
            break;
        case XINPUT_DEVSUBTYPE_DRUM_KIT:
            SDL_snprintf(name, sizeof(name), "XInput DrumKit #%u", 1 + userid);
            break;
        case XINPUT_DEVSUBTYPE_ARCADE_PAD:
            SDL_snprintf(name, sizeof(name), "XInput ArcadePad #%u", 1 + userid);
            break;
        default:
            SDL_snprintf(name, sizeof(name), "XInput Device #%u", 1 + userid);
            break;
        }
    }
    return name;
}

/* We can't really tell what device is being used for XInput, but we can guess
   and we'll be correct for the case where only one device is connected.
 */
static void
GuessXInputDevice(Uint8 userid, Uint16 *pVID, Uint16 *pPID, Uint16 *pVersion)
{
#ifndef __WINRT__   /* TODO: remove this ifndef __WINRT__ block, but only after integrating with UWP/WinRT's HID API */

    PRAWINPUTDEVICELIST devices = NULL;
    UINT i, j, device_count = 0;

    if ((GetRawInputDeviceList(NULL, &device_count, sizeof(RAWINPUTDEVICELIST)) == -1) || (!device_count)) {
        return;  /* oh well. */
    }

    devices = (PRAWINPUTDEVICELIST)SDL_malloc(sizeof(RAWINPUTDEVICELIST) * device_count);
    if (devices == NULL) {
        return;
    }

    if (GetRawInputDeviceList(devices, &device_count, sizeof(RAWINPUTDEVICELIST)) == -1) {
        SDL_free(devices);
        return;  /* oh well. */
    }

    /* First see if we have a cached entry for this index */
    if (s_arrXInputDevicePath[userid]) {
        for (i = 0; i < device_count; i++) {
            RID_DEVICE_INFO rdi;
            char devName[128];
            UINT rdiSize = sizeof(rdi);
            UINT nameSize = SDL_arraysize(devName);

            rdi.cbSize = sizeof(rdi);
            if (devices[i].dwType == RIM_TYPEHID &&
                GetRawInputDeviceInfoA(devices[i].hDevice, RIDI_DEVICEINFO, &rdi, &rdiSize) != (UINT)-1 &&
                GetRawInputDeviceInfoA(devices[i].hDevice, RIDI_DEVICENAME, devName, &nameSize) != (UINT)-1) {
                if (SDL_strcmp(devName, s_arrXInputDevicePath[userid]) == 0) {
                    *pVID = (Uint16)rdi.hid.dwVendorId;
                    *pPID = (Uint16)rdi.hid.dwProductId;
                    *pVersion = (Uint16)rdi.hid.dwVersionNumber;
                    SDL_free(devices);
                    return;
                }
            }
        }
    }

    for (i = 0; i < device_count; i++) {
        RID_DEVICE_INFO rdi;
        char devName[MAX_PATH];
        UINT rdiSize = sizeof(rdi);
        UINT nameSize = SDL_arraysize(devName);

        rdi.cbSize = sizeof(rdi);
        if (devices[i].dwType == RIM_TYPEHID &&
            GetRawInputDeviceInfoA(devices[i].hDevice, RIDI_DEVICEINFO, &rdi, &rdiSize) != (UINT)-1 &&
            GetRawInputDeviceInfoA(devices[i].hDevice, RIDI_DEVICENAME, devName, &nameSize) != (UINT)-1) {
#ifdef DEBUG_JOYSTICK
            SDL_Log("Raw input device: VID = 0x%x, PID = 0x%x, %s\n", rdi.hid.dwVendorId, rdi.hid.dwProductId, devName);
#endif
            if (SDL_strstr(devName, "IG_") != NULL) {
                SDL_bool found = SDL_FALSE;
                for (j = 0; j < SDL_arraysize(s_arrXInputDevicePath); ++j) {
                    if (!s_arrXInputDevicePath[j]) {
                        continue;
                    }
                    if (SDL_strcmp(devName, s_arrXInputDevicePath[j]) == 0) {
                        found = SDL_TRUE;
                        break;
                    }
                }
                if (found) {
                    /* We already have this device in our XInput device list */
                    continue;
                }

                /* We don't actually know if this is the right device for this
                 * userid, but we'll record it so we'll at least be consistent
                 * when the raw device list changes.
                 */
                *pVID = (Uint16)rdi.hid.dwVendorId;
                *pPID = (Uint16)rdi.hid.dwProductId;
                *pVersion = (Uint16)rdi.hid.dwVersionNumber;
                if (s_arrXInputDevicePath[userid]) {
                    SDL_free(s_arrXInputDevicePath[userid]);
                }
                s_arrXInputDevicePath[userid] = SDL_strdup(devName);
                SDL_free(devices);
                return;
            }
        }
    }
    SDL_free(devices);
#endif  /* !__WINRT__ */

    /* The device wasn't in the raw HID device list, it's probably Bluetooth */
    *pVID = 0x045e; /* Microsoft */
    *pPID = 0x02fd; /* XBox One S Bluetooth */
    *pVersion = 0;
}

static void
AddXInputDevice(Uint8 userid, BYTE SubType, JoyStick_DeviceData **pContext)
{
    Uint16 vendor = 0;
    Uint16 product = 0;
    Uint16 version = 0;
    JoyStick_DeviceData *pPrevJoystick = NULL;
    JoyStick_DeviceData *pNewJoystick = *pContext;

    if (SDL_XInputUseOldJoystickMapping() && SubType != XINPUT_DEVSUBTYPE_GAMEPAD)
        return;

    if (SubType == XINPUT_DEVSUBTYPE_UNKNOWN)
        return;

    while (pNewJoystick) {
        if (pNewJoystick->bXInputDevice && (pNewJoystick->XInputUserId == userid) && (pNewJoystick->SubType == SubType)) {
            /* if we are replacing the front of the list then update it */
            if (pNewJoystick == *pContext) {
                *pContext = pNewJoystick->pNext;
            } else if (pPrevJoystick) {
                pPrevJoystick->pNext = pNewJoystick->pNext;
            }

            pNewJoystick->pNext = SYS_Joystick;
            SYS_Joystick = pNewJoystick;
            return;   /* already in the list. */
        }

        pPrevJoystick = pNewJoystick;
        pNewJoystick = pNewJoystick->pNext;
    }

    pNewJoystick = (JoyStick_DeviceData *)SDL_calloc(1, sizeof(JoyStick_DeviceData));
    if (!pNewJoystick) {
        return; /* better luck next time? */
    }

    pNewJoystick->bXInputDevice = SDL_TRUE;
    if (!SDL_XInputUseOldJoystickMapping()) {
        Uint16 *guid16 = (Uint16 *)pNewJoystick->guid.data;

        GuessXInputDevice(userid, &vendor, &product, &version);

        *guid16++ = SDL_SwapLE16(SDL_HARDWARE_BUS_USB);
        *guid16++ = 0;
        *guid16++ = SDL_SwapLE16(vendor);
        *guid16++ = 0;
        *guid16++ = SDL_SwapLE16(product);
        *guid16++ = 0;
        *guid16++ = SDL_SwapLE16(version);
        *guid16++ = 0;

        /* Note that this is an XInput device and what subtype it is */
        pNewJoystick->guid.data[14] = 'x';
        pNewJoystick->guid.data[15] = SubType;
    }
    pNewJoystick->SubType = SubType;
    pNewJoystick->XInputUserId = userid;
    pNewJoystick->joystickname = SDL_CreateJoystickName(vendor, product, NULL, GetXInputName(userid, SubType));
    if (!pNewJoystick->joystickname) {
        SDL_free(pNewJoystick);
        return; /* better luck next time? */
    }

    if (SDL_ShouldIgnoreJoystick(pNewJoystick->joystickname, pNewJoystick->guid)) {
        SDL_free(pNewJoystick);
        return;
    }

#ifdef SDL_JOYSTICK_HIDAPI
    /* Since we're guessing about the VID/PID, use a hard-coded VID/PID to represent XInput */
    if (HIDAPI_IsDevicePresent(USB_VENDOR_MICROSOFT, USB_PRODUCT_XBOX_ONE_XINPUT_CONTROLLER, version, pNewJoystick->joystickname)) {
        /* The HIDAPI driver is taking care of this device */
        SDL_free(pNewJoystick);
        return;
    }
#endif

#ifdef SDL_JOYSTICK_RAWINPUT
    if (RAWINPUT_IsDevicePresent(vendor, product, version, pNewJoystick->joystickname)) {
        /* The RAWINPUT driver is taking care of this device */
        SDL_free(pNewJoystick);
        return;
    }
#endif

    WINDOWS_AddJoystickDevice(pNewJoystick);
}

static void
DelXInputDevice(Uint8 userid)
{
    if (s_arrXInputDevicePath[userid]) {
        SDL_free(s_arrXInputDevicePath[userid]);
        s_arrXInputDevicePath[userid] = NULL;
    }
}

void
SDL_XINPUT_JoystickDetect(JoyStick_DeviceData **pContext)
{
    int iuserid;

    if (!s_bXInputEnabled) {
        return;
    }

    /* iterate in reverse, so these are in the final list in ascending numeric order. */
    for (iuserid = XUSER_MAX_COUNT - 1; iuserid >= 0; iuserid--) {
        const Uint8 userid = (Uint8)iuserid;
        XINPUT_CAPABILITIES capabilities;
        if (XINPUTGETCAPABILITIES(userid, XINPUT_FLAG_GAMEPAD, &capabilities) == ERROR_SUCCESS) {
            /* Adding a new device, must handle all removes first, or GuessXInputDevice goes terribly wrong (returns
              a product/vendor ID that is not even attached to the system) when we get a remove and add on the same tick
              (e.g. when disconnecting a device and the OS reassigns which userid an already-attached controller is)
            */
            int iuserid2;
            for (iuserid2 = iuserid - 1; iuserid2 >= 0; iuserid2--) {
                const Uint8 userid2 = (Uint8)iuserid2;
                XINPUT_CAPABILITIES capabilities2;
                if (XINPUTGETCAPABILITIES(userid2, XINPUT_FLAG_GAMEPAD, &capabilities2) != ERROR_SUCCESS) {
                    DelXInputDevice(userid2);
                }
            }
            AddXInputDevice(userid, capabilities.SubType, pContext);
        } else {
            DelXInputDevice(userid);
        }
    }
}

int
SDL_XINPUT_JoystickOpen(SDL_Joystick * joystick, JoyStick_DeviceData *joystickdevice)
{
    const Uint8 userId = joystickdevice->XInputUserId;
    XINPUT_CAPABILITIES capabilities;
    XINPUT_VIBRATION state;

    SDL_assert(s_bXInputEnabled);
    SDL_assert(XINPUTGETCAPABILITIES);
    SDL_assert(XINPUTSETSTATE);
    SDL_assert(userId < XUSER_MAX_COUNT);

    joystick->hwdata->bXInputDevice = SDL_TRUE;

    if (XINPUTGETCAPABILITIES(userId, XINPUT_FLAG_GAMEPAD, &capabilities) != ERROR_SUCCESS) {
        SDL_free(joystick->hwdata);
        joystick->hwdata = NULL;
        return SDL_SetError("Failed to obtain XInput device capabilities. Device disconnected?");
    }
    SDL_zero(state);
    joystick->hwdata->bXInputHaptic = (XINPUTSETSTATE(userId, &state) == ERROR_SUCCESS);
    joystick->hwdata->userid = userId;

    /* The XInput API has a hard coded button/axis mapping, so we just match it */
    if (SDL_XInputUseOldJoystickMapping()) {
        joystick->naxes = 6;
        joystick->nbuttons = 15;
    } else {
        joystick->naxes = 6;
        joystick->nbuttons = 11;
        joystick->nhats = 1;
    }
    return 0;
}

static void 
UpdateXInputJoystickBatteryInformation(SDL_Joystick * joystick, XINPUT_BATTERY_INFORMATION_EX *pBatteryInformation)
{
    if (pBatteryInformation->BatteryType != BATTERY_TYPE_UNKNOWN) {
        SDL_JoystickPowerLevel ePowerLevel = SDL_JOYSTICK_POWER_UNKNOWN;
        if (pBatteryInformation->BatteryType == BATTERY_TYPE_WIRED) {
            ePowerLevel = SDL_JOYSTICK_POWER_WIRED;
        } else {
            switch (pBatteryInformation->BatteryLevel) {
            case BATTERY_LEVEL_EMPTY:
                ePowerLevel = SDL_JOYSTICK_POWER_EMPTY;
                break;
            case BATTERY_LEVEL_LOW:
                ePowerLevel = SDL_JOYSTICK_POWER_LOW;
                break;
            case BATTERY_LEVEL_MEDIUM:
                ePowerLevel = SDL_JOYSTICK_POWER_MEDIUM;
                break;
            default:
            case BATTERY_LEVEL_FULL:
                ePowerLevel = SDL_JOYSTICK_POWER_FULL;
                break;
            }
        }

        SDL_PrivateJoystickBatteryLevel(joystick, ePowerLevel);
    }
}

static void
UpdateXInputJoystickState_OLD(SDL_Joystick * joystick, XINPUT_STATE_EX *pXInputState, XINPUT_BATTERY_INFORMATION_EX *pBatteryInformation)
{
    static WORD s_XInputButtons[] = {
        XINPUT_GAMEPAD_DPAD_UP, XINPUT_GAMEPAD_DPAD_DOWN, XINPUT_GAMEPAD_DPAD_LEFT, XINPUT_GAMEPAD_DPAD_RIGHT,
        XINPUT_GAMEPAD_START, XINPUT_GAMEPAD_BACK, XINPUT_GAMEPAD_LEFT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB,
        XINPUT_GAMEPAD_LEFT_SHOULDER, XINPUT_GAMEPAD_RIGHT_SHOULDER,
        XINPUT_GAMEPAD_A, XINPUT_GAMEPAD_B, XINPUT_GAMEPAD_X, XINPUT_GAMEPAD_Y,
        XINPUT_GAMEPAD_GUIDE
    };
    WORD wButtons = pXInputState->Gamepad.wButtons;
    Uint8 button;

    SDL_PrivateJoystickAxis(joystick, 0, (Sint16)pXInputState->Gamepad.sThumbLX);
    SDL_PrivateJoystickAxis(joystick, 1, (Sint16)(-SDL_max(-32767, pXInputState->Gamepad.sThumbLY)));
    SDL_PrivateJoystickAxis(joystick, 2, (Sint16)pXInputState->Gamepad.sThumbRX);
    SDL_PrivateJoystickAxis(joystick, 3, (Sint16)(-SDL_max(-32767, pXInputState->Gamepad.sThumbRY)));
    SDL_PrivateJoystickAxis(joystick, 4, (Sint16)(((int)pXInputState->Gamepad.bLeftTrigger * 65535 / 255) - 32768));
    SDL_PrivateJoystickAxis(joystick, 5, (Sint16)(((int)pXInputState->Gamepad.bRightTrigger * 65535 / 255) - 32768));

    for (button = 0; button < SDL_arraysize(s_XInputButtons); ++button) {
        SDL_PrivateJoystickButton(joystick, button, (wButtons & s_XInputButtons[button]) ? SDL_PRESSED : SDL_RELEASED);
    }

    UpdateXInputJoystickBatteryInformation(joystick, pBatteryInformation);
}

static void
UpdateXInputJoystickState(SDL_Joystick * joystick, XINPUT_STATE_EX *pXInputState, XINPUT_BATTERY_INFORMATION_EX *pBatteryInformation)
{
    static WORD s_XInputButtons[] = {
        XINPUT_GAMEPAD_A, XINPUT_GAMEPAD_B, XINPUT_GAMEPAD_X, XINPUT_GAMEPAD_Y,
        XINPUT_GAMEPAD_LEFT_SHOULDER, XINPUT_GAMEPAD_RIGHT_SHOULDER, XINPUT_GAMEPAD_BACK, XINPUT_GAMEPAD_START,
        XINPUT_GAMEPAD_LEFT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB,
        XINPUT_GAMEPAD_GUIDE
    };
    WORD wButtons = pXInputState->Gamepad.wButtons;
    Uint8 button;
    Uint8 hat = SDL_HAT_CENTERED;

    SDL_PrivateJoystickAxis(joystick, 0, pXInputState->Gamepad.sThumbLX);
    SDL_PrivateJoystickAxis(joystick, 1, ~pXInputState->Gamepad.sThumbLY);
    SDL_PrivateJoystickAxis(joystick, 2, ((int)pXInputState->Gamepad.bLeftTrigger * 257) - 32768);
    SDL_PrivateJoystickAxis(joystick, 3, pXInputState->Gamepad.sThumbRX);
    SDL_PrivateJoystickAxis(joystick, 4, ~pXInputState->Gamepad.sThumbRY);
    SDL_PrivateJoystickAxis(joystick, 5, ((int)pXInputState->Gamepad.bRightTrigger * 257) - 32768);

    for (button = 0; button < SDL_arraysize(s_XInputButtons); ++button) {
        SDL_PrivateJoystickButton(joystick, button, (wButtons & s_XInputButtons[button]) ? SDL_PRESSED : SDL_RELEASED);
    }

    if (wButtons & XINPUT_GAMEPAD_DPAD_UP) {
        hat |= SDL_HAT_UP;
    }
    if (wButtons & XINPUT_GAMEPAD_DPAD_DOWN) {
        hat |= SDL_HAT_DOWN;
    }
    if (wButtons & XINPUT_GAMEPAD_DPAD_LEFT) {
        hat |= SDL_HAT_LEFT;
    }
    if (wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) {
        hat |= SDL_HAT_RIGHT;
    }
    SDL_PrivateJoystickHat(joystick, 0, hat);

    UpdateXInputJoystickBatteryInformation(joystick, pBatteryInformation);
}

int
SDL_XINPUT_JoystickRumble(SDL_Joystick * joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble)
{
    XINPUT_VIBRATION XVibration;

    if (!XINPUTSETSTATE) {
        return SDL_Unsupported();
    }

    XVibration.wLeftMotorSpeed = low_frequency_rumble;
    XVibration.wRightMotorSpeed = high_frequency_rumble;
    if (XINPUTSETSTATE(joystick->hwdata->userid, &XVibration) != ERROR_SUCCESS) {
        return SDL_SetError("XInputSetState() failed");
    }
    return 0;
}

void
SDL_XINPUT_JoystickUpdate(SDL_Joystick * joystick)
{
    HRESULT result;
    XINPUT_STATE_EX XInputState;
    XINPUT_BATTERY_INFORMATION_EX XBatteryInformation;

    if (!XINPUTGETSTATE)
        return;

    result = XINPUTGETSTATE(joystick->hwdata->userid, &XInputState);
    if (result == ERROR_DEVICE_NOT_CONNECTED) {
        return;
    }

    SDL_zero(XBatteryInformation);
    if (XINPUTGETBATTERYINFORMATION) {
        result = XINPUTGETBATTERYINFORMATION(joystick->hwdata->userid, BATTERY_DEVTYPE_GAMEPAD, &XBatteryInformation);
    }

    /* only fire events if the data changed from last time */
    if (XInputState.dwPacketNumber && XInputState.dwPacketNumber != joystick->hwdata->dwPacketNumber) {
        if (SDL_XInputUseOldJoystickMapping()) {
            UpdateXInputJoystickState_OLD(joystick, &XInputState, &XBatteryInformation);
        } else {
            UpdateXInputJoystickState(joystick, &XInputState, &XBatteryInformation);
        }
        joystick->hwdata->dwPacketNumber = XInputState.dwPacketNumber;
    }
}

void
SDL_XINPUT_JoystickClose(SDL_Joystick * joystick)
{
}

void
SDL_XINPUT_JoystickQuit(void)
{
    if (s_bXInputEnabled) {
        WIN_UnloadXInputDLL();
    }
}

#else /* !SDL_JOYSTICK_XINPUT */

typedef struct JoyStick_DeviceData JoyStick_DeviceData;

SDL_bool SDL_XINPUT_Enabled(void)
{
    return SDL_FALSE;
}

int
SDL_XINPUT_JoystickInit(void)
{
    return 0;
}

void
SDL_XINPUT_JoystickDetect(JoyStick_DeviceData **pContext)
{
}

int
SDL_XINPUT_JoystickOpen(SDL_Joystick * joystick, JoyStick_DeviceData *joystickdevice)
{
    return SDL_Unsupported();
}

int
SDL_XINPUT_JoystickRumble(SDL_Joystick * joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble)
{
    return SDL_Unsupported();
}

void
SDL_XINPUT_JoystickUpdate(SDL_Joystick * joystick)
{
}

void
SDL_XINPUT_JoystickClose(SDL_Joystick * joystick)
{
}

void
SDL_XINPUT_JoystickQuit(void)
{
}

#endif /* SDL_JOYSTICK_XINPUT */

/* vi: set ts=4 sw=4 expandtab: */
