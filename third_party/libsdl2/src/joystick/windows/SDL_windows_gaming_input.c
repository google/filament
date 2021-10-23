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

#ifdef SDL_JOYSTICK_WGI

#include "SDL_endian.h"
#include "SDL_events.h"
#include "../SDL_sysjoystick.h"
#include "../hidapi/SDL_hidapijoystick_c.h"
#include "SDL_rawinputjoystick_c.h"

#include "../../core/windows/SDL_windows.h"
#define COBJMACROS
#include "windows.gaming.input.h"


struct joystick_hwdata
{
    __x_ABI_CWindows_CGaming_CInput_CIRawGameController *controller;
    __x_ABI_CWindows_CGaming_CInput_CIGameController *gamecontroller;
    __x_ABI_CWindows_CGaming_CInput_CIGameControllerBatteryInfo *battery;
    __x_ABI_CWindows_CGaming_CInput_CIGamepad *gamepad;
    __x_ABI_CWindows_CGaming_CInput_CGamepadVibration vibration;
    UINT64 timestamp;
};

typedef struct WindowsGamingInputControllerState {
    SDL_JoystickID instance_id;
    __x_ABI_CWindows_CGaming_CInput_CIRawGameController *controller;
    char *name;
    SDL_JoystickGUID guid;
    SDL_JoystickType type;
    int naxes;
    int nhats;
    int nbuttons;
} WindowsGamingInputControllerState;

static struct {
    __x_ABI_CWindows_CGaming_CInput_CIRawGameControllerStatics *statics;
    __x_ABI_CWindows_CGaming_CInput_CIArcadeStickStatics *arcade_stick_statics;
    __x_ABI_CWindows_CGaming_CInput_CIArcadeStickStatics2 *arcade_stick_statics2;
    __x_ABI_CWindows_CGaming_CInput_CIFlightStickStatics *flight_stick_statics;
    __x_ABI_CWindows_CGaming_CInput_CIGamepadStatics *gamepad_statics;
    __x_ABI_CWindows_CGaming_CInput_CIGamepadStatics2 *gamepad_statics2;
    __x_ABI_CWindows_CGaming_CInput_CIRacingWheelStatics *racing_wheel_statics;
    __x_ABI_CWindows_CGaming_CInput_CIRacingWheelStatics2 *racing_wheel_statics2;
    EventRegistrationToken controller_added_token;
    EventRegistrationToken controller_removed_token;
    int controller_count;
    WindowsGamingInputControllerState *controllers;
} wgi;

static const IID IID_IRawGameControllerStatics = { 0xEB8D0792, 0xE95A, 0x4B19, { 0xAF, 0xC7, 0x0A, 0x59, 0xF8, 0xBF, 0x75, 0x9E } };
static const IID IID_IRawGameController = { 0x7CAD6D91, 0xA7E1, 0x4F71, { 0x9A, 0x78, 0x33, 0xE9, 0xC5, 0xDF, 0xEA, 0x62 } };
static const IID IID_IRawGameController2 = { 0x43C0C035, 0xBB73, 0x4756, { 0xA7, 0x87, 0x3E, 0xD6, 0xBE, 0xA6, 0x17, 0xBD } };
static const IID IID_IEventHandler_RawGameController = { 0x00621c22, 0x42e8, 0x529f, { 0x92, 0x70, 0x83, 0x6b, 0x32, 0x93, 0x1d, 0x72 } };
static const IID IID_IGameController = { 0x1BAF6522, 0x5F64, 0x42C5, { 0x82, 0x67, 0xB9, 0xFE, 0x22, 0x15, 0xBF, 0xBD } };
static const IID IID_IGameControllerBatteryInfo = { 0xDCECC681, 0x3963, 0x4DA6, { 0x95, 0x5D, 0x55, 0x3F, 0x3B, 0x6F, 0x61, 0x61 } };
static const IID IID_IArcadeStickStatics = { 0x5C37B8C8, 0x37B1, 0x4AD8, { 0x94, 0x58, 0x20, 0x0F, 0x1A, 0x30, 0x01, 0x8E } };
static const IID IID_IArcadeStickStatics2 = { 0x52B5D744, 0xBB86, 0x445A, { 0xB5, 0x9C, 0x59, 0x6F, 0x0E, 0x2A, 0x49, 0xDF } };
static const IID IID_IArcadeStick = { 0xB14A539D, 0xBEFB, 0x4C81, { 0x80, 0x51, 0x15, 0xEC, 0xF3, 0xB1, 0x30, 0x36 } };
static const IID IID_IFlightStickStatics = { 0x5514924A, 0xFECC, 0x435E, { 0x83, 0xDC, 0x5C, 0xEC, 0x8A, 0x18, 0xA5, 0x20 } };
static const IID IID_IFlightStick = { 0xB4A2C01C, 0xB83B, 0x4459, { 0xA1, 0xA9, 0x97, 0xB0, 0x3C, 0x33, 0xDA, 0x7C } };
static const IID IID_IGamepadStatics = { 0x8BBCE529, 0xD49C, 0x39E9, { 0x95, 0x60, 0xE4, 0x7D, 0xDE, 0x96, 0xB7, 0xC8 } };
static const IID IID_IGamepadStatics2 = { 0x42676DC5, 0x0856, 0x47C4, { 0x92, 0x13, 0xB3, 0x95, 0x50, 0x4C, 0x3A, 0x3C } };
static const IID IID_IGamepad = { 0xBC7BB43C, 0x0A69, 0x3903, { 0x9E, 0x9D, 0xA5, 0x0F, 0x86, 0xA4, 0x5D, 0xE5 } };
static const IID IID_IRacingWheelStatics = { 0x3AC12CD5, 0x581B, 0x4936, { 0x9F, 0x94, 0x69, 0xF1, 0xE6, 0x51, 0x4C, 0x7D } };
static const IID IID_IRacingWheelStatics2 = { 0xE666BCAA, 0xEDFD, 0x4323, { 0xA9, 0xF6, 0x3C, 0x38, 0x40, 0x48, 0xD1, 0xED } };
static const IID IID_IRacingWheel = { 0xF546656F, 0xE106, 0x4C82, { 0xA9, 0x0F, 0x55, 0x40, 0x12, 0x90, 0x4B, 0x85 } };

extern SDL_bool SDL_XINPUT_Enabled(void);
extern SDL_bool SDL_DINPUT_JoystickPresent(Uint16 vendor, Uint16 product, Uint16 version);

static SDL_bool
SDL_IsXInputDevice(Uint16 vendor, Uint16 product)
{
    PRAWINPUTDEVICELIST raw_devices = NULL;
    UINT i, raw_device_count = 0;
    LONG vidpid = MAKELONG(vendor, product);

    if (!SDL_XINPUT_Enabled()) {
        return SDL_FALSE;
    }

    /* Go through RAWINPUT (WinXP and later) to find HID devices. */
    if ((GetRawInputDeviceList(NULL, &raw_device_count, sizeof(RAWINPUTDEVICELIST)) == -1) || (!raw_device_count)) {
        return SDL_FALSE;  /* oh well. */
    }

    raw_devices = (PRAWINPUTDEVICELIST)SDL_malloc(sizeof(RAWINPUTDEVICELIST) * raw_device_count);
    if (raw_devices == NULL) {
        SDL_OutOfMemory();
        return SDL_FALSE;
    }

    if (GetRawInputDeviceList(raw_devices, &raw_device_count, sizeof(RAWINPUTDEVICELIST)) == -1) {
        SDL_free(raw_devices);
        raw_devices = NULL;
        return SDL_FALSE;  /* oh well. */
    }

    for (i = 0; i < raw_device_count; i++) {
        RID_DEVICE_INFO rdi;
        char devName[MAX_PATH];
        UINT rdiSize = sizeof(rdi);
        UINT nameSize = SDL_arraysize(devName);

        rdi.cbSize = sizeof(rdi);
        if ((raw_devices[i].dwType == RIM_TYPEHID) &&
            (GetRawInputDeviceInfoA(raw_devices[i].hDevice, RIDI_DEVICEINFO, &rdi, &rdiSize) != ((UINT)-1)) &&
            (MAKELONG(rdi.hid.dwVendorId, rdi.hid.dwProductId) == vidpid) &&
            (GetRawInputDeviceInfoA(raw_devices[i].hDevice, RIDI_DEVICENAME, devName, &nameSize) != ((UINT)-1)) &&
            (SDL_strstr(devName, "IG_") != NULL)) {
            return SDL_TRUE;
        }
    }

    return SDL_FALSE;
}

static HRESULT STDMETHODCALLTYPE IEventHandler_CRawGameControllerVtbl_QueryInterface(__FIEventHandler_1_Windows__CGaming__CInput__CRawGameController * This, REFIID riid, void **ppvObject)
{
    if (!ppvObject) {
        return E_INVALIDARG;
    }

    *ppvObject = NULL;
    if (WIN_IsEqualIID(riid, &IID_IUnknown) || WIN_IsEqualIID(riid, &IID_IEventHandler_RawGameController)) {
        *ppvObject = This;
        return S_OK;
    }
    return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE IEventHandler_CRawGameControllerVtbl_AddRef(__FIEventHandler_1_Windows__CGaming__CInput__CRawGameController * This)
{
    return 1;
}

static ULONG STDMETHODCALLTYPE IEventHandler_CRawGameControllerVtbl_Release(__FIEventHandler_1_Windows__CGaming__CInput__CRawGameController * This)
{
    return 1;
}

static HRESULT STDMETHODCALLTYPE IEventHandler_CRawGameControllerVtbl_InvokeAdded(__FIEventHandler_1_Windows__CGaming__CInput__CRawGameController * This, IInspectable *sender, __x_ABI_CWindows_CGaming_CInput_CIRawGameController *e)
{
    HRESULT hr;
    __x_ABI_CWindows_CGaming_CInput_CIRawGameController *controller = NULL;

    hr = IUnknown_QueryInterface((IUnknown *)e, &IID_IRawGameController, (void **)&controller);
    if (SUCCEEDED(hr)) {
        char *name = NULL;
        SDL_JoystickGUID guid;
        Uint16 vendor = 0;
        Uint16 product = 0;
        Uint16 version = 0;
        SDL_JoystickType type = SDL_JOYSTICK_TYPE_UNKNOWN;
        Uint16 *guid16 = (Uint16 *)guid.data;
        __x_ABI_CWindows_CGaming_CInput_CIRawGameController2 *controller2 = NULL;
        __x_ABI_CWindows_CGaming_CInput_CIGameController *gamecontroller = NULL;
        SDL_bool ignore_joystick = SDL_FALSE;

        __x_ABI_CWindows_CGaming_CInput_CIRawGameController_get_HardwareVendorId(controller, &vendor);
        __x_ABI_CWindows_CGaming_CInput_CIRawGameController_get_HardwareProductId(controller, &product);

        hr = __x_ABI_CWindows_CGaming_CInput_CIRawGameController_QueryInterface(controller, &IID_IRawGameController2, (void **)&controller2);
        if (SUCCEEDED(hr)) {
            HMODULE hModule = LoadLibraryA("combase.dll");
            if (hModule != NULL) {
                typedef PCWSTR (WINAPI *WindowsGetStringRawBuffer_t)(HSTRING string, UINT32 *length);
                typedef HRESULT (WINAPI *WindowsDeleteString_t)(HSTRING string);

                WindowsGetStringRawBuffer_t WindowsGetStringRawBufferFunc = (WindowsGetStringRawBuffer_t)GetProcAddress(hModule, "WindowsGetStringRawBuffer");
                WindowsDeleteString_t WindowsDeleteStringFunc = (WindowsDeleteString_t)GetProcAddress(hModule, "WindowsDeleteString");
                if (WindowsGetStringRawBufferFunc && WindowsDeleteStringFunc) {
                    HSTRING hString;
                    hr = __x_ABI_CWindows_CGaming_CInput_CIRawGameController2_get_DisplayName(controller2, &hString);
                    if (SUCCEEDED(hr)) {
                        PCWSTR string = WindowsGetStringRawBufferFunc(hString, NULL);
                        if (string) {
                            name = WIN_StringToUTF8W(string);
                        }
                        WindowsDeleteStringFunc(hString);
                    }
                }
                FreeLibrary(hModule);
            }
            __x_ABI_CWindows_CGaming_CInput_CIRawGameController2_Release(controller2);
        }
        if (!name) {
            name = "";
        }

        hr = __x_ABI_CWindows_CGaming_CInput_CIRawGameController_QueryInterface(controller, &IID_IGameController, (void **)&gamecontroller);
        if (SUCCEEDED(hr)) {
            __x_ABI_CWindows_CGaming_CInput_CIArcadeStick *arcade_stick = NULL;
            __x_ABI_CWindows_CGaming_CInput_CIFlightStick *flight_stick = NULL;
            __x_ABI_CWindows_CGaming_CInput_CIGamepad *gamepad = NULL;
            __x_ABI_CWindows_CGaming_CInput_CIRacingWheel *racing_wheel = NULL;

            if (wgi.gamepad_statics2 && SUCCEEDED(__x_ABI_CWindows_CGaming_CInput_CIGamepadStatics2_FromGameController(wgi.gamepad_statics2, gamecontroller, &gamepad)) && gamepad) {
                type = SDL_JOYSTICK_TYPE_GAMECONTROLLER;
                __x_ABI_CWindows_CGaming_CInput_CIGamepad_Release(gamepad);
            } else if (wgi.arcade_stick_statics2 && SUCCEEDED(__x_ABI_CWindows_CGaming_CInput_CIArcadeStickStatics2_FromGameController(wgi.arcade_stick_statics2, gamecontroller, &arcade_stick)) && arcade_stick) {
                type = SDL_JOYSTICK_TYPE_ARCADE_STICK;
                __x_ABI_CWindows_CGaming_CInput_CIArcadeStick_Release(arcade_stick);
            } else if (wgi.flight_stick_statics && SUCCEEDED(__x_ABI_CWindows_CGaming_CInput_CIFlightStickStatics_FromGameController(wgi.flight_stick_statics, gamecontroller, &flight_stick)) && flight_stick) {
                type = SDL_JOYSTICK_TYPE_FLIGHT_STICK;
                __x_ABI_CWindows_CGaming_CInput_CIFlightStick_Release(flight_stick);
            } else if (wgi.racing_wheel_statics2 && SUCCEEDED(__x_ABI_CWindows_CGaming_CInput_CIRacingWheelStatics2_FromGameController(wgi.racing_wheel_statics2, gamecontroller, &racing_wheel)) && racing_wheel) {
                type = SDL_JOYSTICK_TYPE_WHEEL;
                __x_ABI_CWindows_CGaming_CInput_CIRacingWheel_Release(racing_wheel);
            }
            __x_ABI_CWindows_CGaming_CInput_CIGameController_Release(gamecontroller);
        }

        /* FIXME: Is there any way to tell whether this is a Bluetooth device? */
        *guid16++ = SDL_SwapLE16(SDL_HARDWARE_BUS_USB);
        *guid16++ = 0;
        *guid16++ = SDL_SwapLE16(vendor);
        *guid16++ = 0;
        *guid16++ = SDL_SwapLE16(product);
        *guid16++ = 0;
        *guid16++ = SDL_SwapLE16(version);
        *guid16++ = 0;

        /* Note that this is a Windows Gaming Input device for special handling elsewhere */
        guid.data[14] = 'w';
        guid.data[15] = (Uint8)type;

#ifdef SDL_JOYSTICK_HIDAPI
        if (!ignore_joystick && HIDAPI_IsDevicePresent(vendor, product, version, name)) {
            ignore_joystick = SDL_TRUE;
        }
#endif

#ifdef SDL_JOYSTICK_RAWINPUT
        if (!ignore_joystick && RAWINPUT_IsDevicePresent(vendor, product, version, name)) {
            ignore_joystick = SDL_TRUE;
        }
#endif

        if (!ignore_joystick && SDL_DINPUT_JoystickPresent(vendor, product, version)) {
            ignore_joystick = SDL_TRUE;
        }

        if (!ignore_joystick && SDL_IsXInputDevice(vendor, product)) {
            ignore_joystick = SDL_TRUE;
        }

        if (!ignore_joystick && SDL_ShouldIgnoreJoystick(name, guid)) {
            ignore_joystick = SDL_TRUE;
        }

        if (ignore_joystick) {
            SDL_free(name);
        } else {
            /* New device, add it */
            WindowsGamingInputControllerState *controllers = SDL_realloc(wgi.controllers, sizeof(wgi.controllers[0]) * (wgi.controller_count + 1));
            if (controllers) {
                WindowsGamingInputControllerState *state = &controllers[wgi.controller_count];
                SDL_JoystickID joystickID = SDL_GetNextJoystickInstanceID();

                SDL_zerop(state);
                state->instance_id = joystickID;
                state->controller = controller;
                state->name = name;
                state->guid = guid;
                state->type = type;

                __x_ABI_CWindows_CGaming_CInput_CIRawGameController_get_ButtonCount(controller, &state->nbuttons);
                __x_ABI_CWindows_CGaming_CInput_CIRawGameController_get_AxisCount(controller, &state->naxes);
                __x_ABI_CWindows_CGaming_CInput_CIRawGameController_get_SwitchCount(controller, &state->nhats);

                __x_ABI_CWindows_CGaming_CInput_CIRawGameController_AddRef(controller);

                ++wgi.controller_count;
                wgi.controllers = controllers;

                SDL_PrivateJoystickAdded(joystickID);
            }
        }

        __x_ABI_CWindows_CGaming_CInput_CIRawGameController_Release(controller);
    }
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE IEventHandler_CRawGameControllerVtbl_InvokeRemoved(__FIEventHandler_1_Windows__CGaming__CInput__CRawGameController * This, IInspectable *sender, __x_ABI_CWindows_CGaming_CInput_CIRawGameController *e)
{
    HRESULT hr;
    __x_ABI_CWindows_CGaming_CInput_CIRawGameController *controller = NULL;

    hr = IUnknown_QueryInterface((IUnknown *)e, &IID_IRawGameController, (void **)&controller);
    if (SUCCEEDED(hr)) {
        int i;

        for (i = 0; i < wgi.controller_count ; i++) {
            if (wgi.controllers[i].controller == controller) {
                WindowsGamingInputControllerState *state = &wgi.controllers[i];
                SDL_JoystickID joystickID = state->instance_id;

                __x_ABI_CWindows_CGaming_CInput_CIRawGameController_Release(state->controller);

                SDL_free(state->name);

                --wgi.controller_count;
                if (i < wgi.controller_count) {
                    SDL_memmove(&wgi.controllers[i], &wgi.controllers[i + 1], (wgi.controller_count - i) * sizeof(wgi.controllers[i]));
                }

                SDL_PrivateJoystickRemoved(joystickID);
                break;
            }
        }

        __x_ABI_CWindows_CGaming_CInput_CIRawGameController_Release(controller);
    }
    return S_OK;
}

static __FIEventHandler_1_Windows__CGaming__CInput__CRawGameControllerVtbl controller_added_vtbl = {
    IEventHandler_CRawGameControllerVtbl_QueryInterface,
    IEventHandler_CRawGameControllerVtbl_AddRef,
    IEventHandler_CRawGameControllerVtbl_Release,
    IEventHandler_CRawGameControllerVtbl_InvokeAdded
};
static __FIEventHandler_1_Windows__CGaming__CInput__CRawGameController controller_added = {
    &controller_added_vtbl
};

static __FIEventHandler_1_Windows__CGaming__CInput__CRawGameControllerVtbl controller_removed_vtbl = {
    IEventHandler_CRawGameControllerVtbl_QueryInterface,
    IEventHandler_CRawGameControllerVtbl_AddRef,
    IEventHandler_CRawGameControllerVtbl_Release,
    IEventHandler_CRawGameControllerVtbl_InvokeRemoved
};
static __FIEventHandler_1_Windows__CGaming__CInput__CRawGameController controller_removed = {
    &controller_removed_vtbl
};

static int
WGI_JoystickInit(void)
{
    if (FAILED(WIN_CoInitialize())) {
        return SDL_SetError("CoInitialize() failed");
    }

    HRESULT hr;
    HMODULE hModule = LoadLibraryA("combase.dll");
    if (hModule != NULL) {
        typedef HRESULT (WINAPI *WindowsCreateStringReference_t)(PCWSTR sourceString, UINT32 length, HSTRING_HEADER *hstringHeader, HSTRING* string);
        typedef HRESULT (WINAPI *WindowsDeleteString_t)(HSTRING string);
        typedef HRESULT (WINAPI *RoGetActivationFactory_t)(HSTRING activatableClassId, REFIID iid, void** factory);

        WindowsCreateStringReference_t WindowsCreateStringReferenceFunc = (WindowsCreateStringReference_t)GetProcAddress(hModule, "WindowsCreateStringReference");
        RoGetActivationFactory_t RoGetActivationFactoryFunc = (RoGetActivationFactory_t)GetProcAddress(hModule, "RoGetActivationFactory");
        if (WindowsCreateStringReferenceFunc && RoGetActivationFactoryFunc) {
            PCWSTR pNamespace;
            HSTRING_HEADER hNamespaceStringHeader;
            HSTRING hNamespaceString;

            pNamespace = L"Windows.Gaming.Input.RawGameController";
            hr = WindowsCreateStringReferenceFunc(pNamespace, (UINT32)SDL_wcslen(pNamespace), &hNamespaceStringHeader, &hNamespaceString);
            if (SUCCEEDED(hr)) {
                hr = RoGetActivationFactoryFunc(hNamespaceString, &IID_IRawGameControllerStatics, &wgi.statics);
                if (!SUCCEEDED(hr)) {
                    SDL_SetError("Couldn't find IRawGameControllerStatics: 0x%x", hr);
                }
            }

            pNamespace = L"Windows.Gaming.Input.ArcadeStick";
            hr = WindowsCreateStringReferenceFunc(pNamespace, (UINT32)SDL_wcslen(pNamespace), &hNamespaceStringHeader, &hNamespaceString);
            if (SUCCEEDED(hr)) {
                hr = RoGetActivationFactoryFunc(hNamespaceString, &IID_IArcadeStickStatics, &wgi.arcade_stick_statics);
                if (SUCCEEDED(hr)) {
                    __x_ABI_CWindows_CGaming_CInput_CIArcadeStickStatics_QueryInterface(wgi.arcade_stick_statics, &IID_IArcadeStickStatics2, &wgi.arcade_stick_statics2);
                } else {
                    SDL_SetError("Couldn't find IID_IArcadeStickStatics: 0x%x", hr);
                }
            }

            pNamespace = L"Windows.Gaming.Input.FlightStick";
            hr = WindowsCreateStringReferenceFunc(pNamespace, (UINT32)SDL_wcslen(pNamespace), &hNamespaceStringHeader, &hNamespaceString);
            if (SUCCEEDED(hr)) {
                hr = RoGetActivationFactoryFunc(hNamespaceString, &IID_IFlightStickStatics, &wgi.flight_stick_statics);
                if (!SUCCEEDED(hr)) {
                    SDL_SetError("Couldn't find IID_IFlightStickStatics: 0x%x", hr);
                }
            }

            pNamespace = L"Windows.Gaming.Input.Gamepad";
            hr = WindowsCreateStringReferenceFunc(pNamespace, (UINT32)SDL_wcslen(pNamespace), &hNamespaceStringHeader, &hNamespaceString);
            if (SUCCEEDED(hr)) {
                hr = RoGetActivationFactoryFunc(hNamespaceString, &IID_IGamepadStatics, &wgi.gamepad_statics);
                if (SUCCEEDED(hr)) {
                    __x_ABI_CWindows_CGaming_CInput_CIGamepadStatics_QueryInterface(wgi.gamepad_statics, &IID_IGamepadStatics2, &wgi.gamepad_statics2);
                } else {
                    SDL_SetError("Couldn't find IGamepadStatics: 0x%x", hr);
                }
            }

            pNamespace = L"Windows.Gaming.Input.RacingWheel";
            hr = WindowsCreateStringReferenceFunc(pNamespace, (UINT32)SDL_wcslen(pNamespace), &hNamespaceStringHeader, &hNamespaceString);
            if (SUCCEEDED(hr)) {
                hr = RoGetActivationFactoryFunc(hNamespaceString, &IID_IRacingWheelStatics, &wgi.racing_wheel_statics);
                if (SUCCEEDED(hr)) {
                    __x_ABI_CWindows_CGaming_CInput_CIRacingWheelStatics_QueryInterface(wgi.racing_wheel_statics, &IID_IRacingWheelStatics2, &wgi.racing_wheel_statics2);
                } else {
                    SDL_SetError("Couldn't find IRacingWheelStatics: 0x%x", hr);
                }
            }
        }
        FreeLibrary(hModule);
    }

    if (wgi.statics) {
        hr = __x_ABI_CWindows_CGaming_CInput_CIRawGameControllerStatics_add_RawGameControllerAdded(wgi.statics, &controller_added, &wgi.controller_added_token);
        if (!SUCCEEDED(hr)) {
            SDL_SetError("add_RawGameControllerAdded() failed: 0x%x\n", hr);
        }

        hr = __x_ABI_CWindows_CGaming_CInput_CIRawGameControllerStatics_add_RawGameControllerRemoved(wgi.statics, &controller_removed, &wgi.controller_removed_token);
        if (!SUCCEEDED(hr)) {
            SDL_SetError("add_RawGameControllerRemoved() failed: 0x%x\n", hr);
        }
    }

    return 0;
}

static int
WGI_JoystickGetCount(void)
{
    return wgi.controller_count;
}

static void
WGI_JoystickDetect(void)
{
}

static const char *
WGI_JoystickGetDeviceName(int device_index)
{
    return wgi.controllers[device_index].name;
}

static int
WGI_JoystickGetDevicePlayerIndex(int device_index)
{
    return -1;
}

static void
WGI_JoystickSetDevicePlayerIndex(int device_index, int player_index)
{
}

static SDL_JoystickGUID
WGI_JoystickGetDeviceGUID(int device_index)
{
    return wgi.controllers[device_index].guid;
}

static SDL_JoystickID
WGI_JoystickGetDeviceInstanceID(int device_index)
{
    return wgi.controllers[device_index].instance_id;
}

static int
WGI_JoystickOpen(SDL_Joystick *joystick, int device_index)
{
    WindowsGamingInputControllerState *state = &wgi.controllers[device_index];
    struct joystick_hwdata *hwdata;
    boolean wireless = SDL_FALSE;

    hwdata = (struct joystick_hwdata *)SDL_calloc(1, sizeof(*hwdata));
    if (!hwdata) {
        return SDL_OutOfMemory();
    }
    joystick->hwdata = hwdata;

    hwdata->controller = state->controller;
    __x_ABI_CWindows_CGaming_CInput_CIRawGameController_AddRef(hwdata->controller);
    __x_ABI_CWindows_CGaming_CInput_CIRawGameController_QueryInterface(hwdata->controller, &IID_IGameController, (void **)&hwdata->gamecontroller);
    __x_ABI_CWindows_CGaming_CInput_CIRawGameController_QueryInterface(hwdata->controller, &IID_IGameControllerBatteryInfo, (void **)&hwdata->battery);

    if (wgi.gamepad_statics2) {
        __x_ABI_CWindows_CGaming_CInput_CIGamepadStatics2_FromGameController(wgi.gamepad_statics2, hwdata->gamecontroller, &hwdata->gamepad);
    }

    if (hwdata->gamecontroller) {
        __x_ABI_CWindows_CGaming_CInput_CIGameController_get_IsWireless(hwdata->gamecontroller, &wireless);
    }

    /* Initialize the joystick capabilities */
    joystick->nbuttons = state->nbuttons;
    joystick->naxes = state->naxes;
    joystick->nhats = state->nhats;
    joystick->epowerlevel = wireless ? SDL_JOYSTICK_POWER_UNKNOWN : SDL_JOYSTICK_POWER_WIRED;

    if (wireless && hwdata->battery) {
        HRESULT hr;
        __x_ABI_CWindows_CDevices_CPower_CIBatteryReport *report;

        hr = __x_ABI_CWindows_CGaming_CInput_CIGameControllerBatteryInfo_TryGetBatteryReport(hwdata->battery, &report);
        if (SUCCEEDED(hr) && report) {
            int full_capacity = 0, curr_capacity = 0;
            __FIReference_1_int *full_capacityP, *curr_capacityP;

            hr = __x_ABI_CWindows_CDevices_CPower_CIBatteryReport_get_FullChargeCapacityInMilliwattHours(report, &full_capacityP);
            if (SUCCEEDED(hr)) {
                __FIReference_1_int_get_Value(full_capacityP, &full_capacity);
                __FIReference_1_int_Release(full_capacityP);
            }

            hr = __x_ABI_CWindows_CDevices_CPower_CIBatteryReport_get_RemainingCapacityInMilliwattHours(report, &curr_capacityP);
            if (SUCCEEDED(hr)) {
                __FIReference_1_int_get_Value(curr_capacityP, &curr_capacity);
                __FIReference_1_int_Release(curr_capacityP);
            }

            if (full_capacity > 0) {
                float ratio = (float)curr_capacity / full_capacity;

                if (ratio <= 0.05f) {
                    joystick->epowerlevel = SDL_JOYSTICK_POWER_EMPTY;
                } else if (ratio <= 0.20f) {
                    joystick->epowerlevel = SDL_JOYSTICK_POWER_LOW;
                } else if (ratio <= 0.70f) {
                    joystick->epowerlevel = SDL_JOYSTICK_POWER_MEDIUM;
                } else {
                    joystick->epowerlevel = SDL_JOYSTICK_POWER_FULL;
                }
            }
            __x_ABI_CWindows_CDevices_CPower_CIBatteryReport_Release(report);
        }
    }
    return 0;
}

static int
WGI_JoystickRumble(SDL_Joystick *joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble)
{
    struct joystick_hwdata *hwdata = joystick->hwdata;

    if (hwdata->gamepad) {
        HRESULT hr;

        hwdata->vibration.LeftMotor = (DOUBLE)low_frequency_rumble / SDL_MAX_UINT16;
        hwdata->vibration.RightMotor = (DOUBLE)high_frequency_rumble / SDL_MAX_UINT16;
        hr = __x_ABI_CWindows_CGaming_CInput_CIGamepad_put_Vibration(hwdata->gamepad, hwdata->vibration);
        if (SUCCEEDED(hr)) {
            return 0;
        } else {
            return SDL_SetError("Setting vibration failed: 0x%x\n", hr);
        }
    } else {
        return SDL_Unsupported();
    }
}

static int
WGI_JoystickRumbleTriggers(SDL_Joystick *joystick, Uint16 left_rumble, Uint16 right_rumble)
{
    struct joystick_hwdata *hwdata = joystick->hwdata;

    if (hwdata->gamepad) {
        HRESULT hr;

        hwdata->vibration.LeftTrigger = (DOUBLE)left_rumble / SDL_MAX_UINT16;
        hwdata->vibration.RightTrigger = (DOUBLE)right_rumble / SDL_MAX_UINT16;
        hr = __x_ABI_CWindows_CGaming_CInput_CIGamepad_put_Vibration(hwdata->gamepad, hwdata->vibration);
        if (SUCCEEDED(hr)) {
            return 0;
        } else {
            return SDL_SetError("Setting vibration failed: 0x%x\n", hr);
        }
    } else {
        return SDL_Unsupported();
    }
}

static SDL_bool
WGI_JoystickHasLED(SDL_Joystick *joystick)
{
    return SDL_FALSE;
}

static int
WGI_JoystickSetLED(SDL_Joystick *joystick, Uint8 red, Uint8 green, Uint8 blue)
{
    return SDL_Unsupported();
}

static int
WGI_JoystickSendEffect(SDL_Joystick *joystick, const void *data, int size)
{
    return SDL_Unsupported();
}

static int
WGI_JoystickSetSensorsEnabled(SDL_Joystick *joystick, SDL_bool enabled)
{
    return SDL_Unsupported();
}

static Uint8
ConvertHatValue(__x_ABI_CWindows_CGaming_CInput_CGameControllerSwitchPosition value)
{
    switch (value) {
    case GameControllerSwitchPosition_Up:
        return SDL_HAT_UP;
    case GameControllerSwitchPosition_UpRight:
        return SDL_HAT_RIGHTUP;
    case GameControllerSwitchPosition_Right:
        return SDL_HAT_RIGHT;
    case GameControllerSwitchPosition_DownRight:
        return SDL_HAT_RIGHTDOWN;
    case GameControllerSwitchPosition_Down:
        return SDL_HAT_DOWN;
    case GameControllerSwitchPosition_DownLeft:
        return SDL_HAT_LEFTDOWN;
    case GameControllerSwitchPosition_Left:
        return SDL_HAT_LEFT;
    case GameControllerSwitchPosition_UpLeft:
        return SDL_HAT_LEFTUP;
    default:
        return SDL_HAT_CENTERED;
    }
}

static void
WGI_JoystickUpdate(SDL_Joystick *joystick)
{
    struct joystick_hwdata *hwdata = joystick->hwdata;
    HRESULT hr;
    UINT32 nbuttons = joystick->nbuttons;
    boolean *buttons = SDL_stack_alloc(boolean, nbuttons);
    UINT32 nhats = joystick->nhats;
    __x_ABI_CWindows_CGaming_CInput_CGameControllerSwitchPosition *hats = SDL_stack_alloc(__x_ABI_CWindows_CGaming_CInput_CGameControllerSwitchPosition, nhats);
    UINT32 naxes = joystick->naxes;
    DOUBLE *axes = SDL_stack_alloc(DOUBLE, naxes);
    UINT64 timestamp;

    hr = __x_ABI_CWindows_CGaming_CInput_CIRawGameController_GetCurrentReading(hwdata->controller, nbuttons, buttons, nhats, hats, naxes, axes, &timestamp);
    if (SUCCEEDED(hr) && timestamp != hwdata->timestamp) {
        UINT32 i;

        for (i = 0; i < nbuttons; ++i) {
            SDL_PrivateJoystickButton(joystick, i, buttons[i]);
        }
        for (i = 0; i < nhats; ++i) {
            SDL_PrivateJoystickHat(joystick, i, ConvertHatValue(hats[i]));
        }
        for (i = 0; i < naxes; ++i) {
            SDL_PrivateJoystickAxis(joystick, i, (int)(axes[i] * 65535) - 32768);
        }
        hwdata->timestamp = timestamp;
    }

    SDL_stack_free(buttons);
    SDL_stack_free(hats);
    SDL_stack_free(axes);
}

static void
WGI_JoystickClose(SDL_Joystick *joystick)
{
    struct joystick_hwdata *hwdata = joystick->hwdata;

    if (hwdata) {
        if (hwdata->controller) {
            __x_ABI_CWindows_CGaming_CInput_CIRawGameController_Release(hwdata->controller);
        }
        if (hwdata->gamecontroller) {
            __x_ABI_CWindows_CGaming_CInput_CIGameController_Release(hwdata->gamecontroller);
        }
        if (hwdata->battery) {
            __x_ABI_CWindows_CGaming_CInput_CIGameControllerBatteryInfo_Release(hwdata->battery);
        }
        if (hwdata->gamepad) {
            __x_ABI_CWindows_CGaming_CInput_CIGamepad_Release(hwdata->gamepad);
        }
        SDL_free(hwdata);
    }
    joystick->hwdata = NULL;
}

static void
WGI_JoystickQuit(void)
{
    if (wgi.statics) {
        while (wgi.controller_count > 0) {
            IEventHandler_CRawGameControllerVtbl_InvokeRemoved(&controller_removed, NULL, (__x_ABI_CWindows_CGaming_CInput_CIRawGameController *)wgi.controllers[wgi.controller_count - 1].controller);
        }
        if (wgi.controllers) {
            SDL_free(wgi.controllers);
        }

        if (wgi.arcade_stick_statics) {
            __x_ABI_CWindows_CGaming_CInput_CIArcadeStickStatics_Release(wgi.arcade_stick_statics);
        }
        if (wgi.arcade_stick_statics2) {
            __x_ABI_CWindows_CGaming_CInput_CIArcadeStickStatics2_Release(wgi.arcade_stick_statics2);
        }
        if (wgi.flight_stick_statics) {
            __x_ABI_CWindows_CGaming_CInput_CIFlightStickStatics_Release(wgi.flight_stick_statics);
        }
        if (wgi.gamepad_statics) {
            __x_ABI_CWindows_CGaming_CInput_CIGamepadStatics_Release(wgi.gamepad_statics);
        }
        if (wgi.gamepad_statics2) {
            __x_ABI_CWindows_CGaming_CInput_CIGamepadStatics2_Release(wgi.gamepad_statics2);
        }
        if (wgi.racing_wheel_statics) {
            __x_ABI_CWindows_CGaming_CInput_CIRacingWheelStatics_Release(wgi.racing_wheel_statics);
        }
        if (wgi.racing_wheel_statics2) {
            __x_ABI_CWindows_CGaming_CInput_CIRacingWheelStatics2_Release(wgi.racing_wheel_statics2);
        }

        __x_ABI_CWindows_CGaming_CInput_CIRawGameControllerStatics_remove_RawGameControllerAdded(wgi.statics, wgi.controller_added_token);
        __x_ABI_CWindows_CGaming_CInput_CIRawGameControllerStatics_remove_RawGameControllerRemoved(wgi.statics, wgi.controller_removed_token);
        __x_ABI_CWindows_CGaming_CInput_CIRawGameControllerStatics_Release(wgi.statics);
    }
    SDL_zero(wgi);

    /* Don't uninitialize COM because of what appears to be a bug in Microsoft WGI reference counting.
     *
     * If you plug in a non-Xbox controller and let the application run for 30 seconds, then it crashes in CoUninitialize()
     * with this stack trace:

        Windows.Gaming.Input.dll!GameController::~GameController(void)	Unknown
        Windows.Gaming.Input.dll!GameController::`vector deleting destructor'(unsigned int)	Unknown
        Windows.Gaming.Input.dll!Microsoft::WRL::Details::RuntimeClassImpl<struct Microsoft::WRL::RuntimeClassFlags<1>,1,1,0,struct Windows::Gaming::Input::IGameController,struct Windows::Gaming::Input::IGameControllerBatteryInfo,struct Microsoft::WRL::CloakedIid<struct Windows::Gaming::Input::Internal::IGameControllerPrivate>,class Microsoft::WRL::FtmBase>::Release(void)	Unknown
        Windows.Gaming.Input.dll!Windows::Gaming::Input::Custom::Details::AggregableRuntimeClass<struct Windows::Gaming::Input::IGamepad,struct Windows::Gaming::Input::IGamepad2,struct Microsoft::WRL::CloakedIid<struct Windows::Gaming::Input::Custom::IGameControllerInputSink>,struct Microsoft::WRL::CloakedIid<struct Windows::Gaming::Input::Custom::IGipGameControllerInputSink>,struct Microsoft::WRL::CloakedIid<struct Windows::Gaming::Input::Custom::IHidGameControllerInputSink>,struct Microsoft::WRL::CloakedIid<struct Windows::Gaming::Input::Custom::IXusbGameControllerInputSink>,class Microsoft::WRL::Details::Nil,class Microsoft::WRL::Details::Nil,class Microsoft::WRL::Details::Nil>::Release(void)	Unknown
        Windows.Gaming.Input.dll!Microsoft::WRL::ComPtr<`WaitForCompletion<Windows::Foundation::IAsyncOperationWithProgressCompletedHandler<Windows::Storage::Streams::IBuffer *,unsigned int>,Windows::Foundation::IAsyncOperationWithProgress<Windows::Storage::Streams::IBuffer *,unsigned int>>'::`2'::FTMEventDelegate>::~ComPtr<`WaitForCompletion<Windows::Foundation::IAsyncOperationWithProgressCompletedHandler<Windows::Storage::Streams::IBuffer *,unsigned int>,Windows::Foundation::IAsyncOperationWithProgress<Windows::Storage::Streams::IBuffer *,unsigned int>>'::`2'::FTMEventDelegate>()	Unknown
        Windows.Gaming.Input.dll!`eh vector destructor iterator'(void *,unsigned int,int,void (*)(void *))	Unknown
        Windows.Gaming.Input.dll!Windows::Gaming::Input::Custom::Details::GameControllerCollection<class Windows::Gaming::Input::RawGameController,struct Windows::Gaming::Input::IRawGameController>::~GameControllerCollection<class Windows::Gaming::Input::RawGameController,struct Windows::Gaming::Input::IRawGameController>(void)	Unknown
        Windows.Gaming.Input.dll!Windows::Gaming::Input::Custom::Details::GameControllerCollection<class Windows::Gaming::Input::RawGameController,struct Windows::Gaming::Input::IRawGameController>::`vector deleting destructor'(unsigned int)	Unknown
        Windows.Gaming.Input.dll!Microsoft::WRL::Details::RuntimeClassImpl<struct Microsoft::WRL::RuntimeClassFlags<1>,1,1,0,struct Windows::Foundation::Collections::IIterable<class Windows::Gaming::Input::ArcadeStick *>,struct Windows::Foundation::Collections::IVectorView<class Windows::Gaming::Input::ArcadeStick *>,class Microsoft::WRL::FtmBase>::Release(void)	Unknown
        Windows.Gaming.Input.dll!Windows::Gaming::Input::Custom::Details::CustomGameControllerFactoryBase<class Windows::Gaming::Input::FlightStick,class Windows::Gaming::Input::FlightStick,struct Windows::Gaming::Input::IFlightStick,struct Windows::Gaming::Input::IFlightStickStatics,class Microsoft::WRL::Details::Nil>::~CustomGameControllerFactoryBase<class Windows::Gaming::Input::FlightStick,class Windows::Gaming::Input::FlightStick,struct Windows::Gaming::Input::IFlightStick,struct Windows::Gaming::Input::IFlightStickStatics,class Microsoft::WRL::Details::Nil>(void)	Unknown
        Windows.Gaming.Input.dll!Windows::Gaming::Input::Custom::Details::CustomGameControllerFactoryBase<class Windows::Gaming::Input::FlightStick,class Windows::Gaming::Input::FlightStick,struct Windows::Gaming::Input::IFlightStick,struct Windows::Gaming::Input::IFlightStickStatics,class Microsoft::WRL::Details::Nil>::`vector deleting destructor'(unsigned int)	Unknown
        Windows.Gaming.Input.dll!Microsoft::WRL::ActivationFactory<struct Microsoft::WRL::Implements<class Microsoft::WRL::FtmBase,struct Microsoft::WRL::CloakedIid<struct Windows::Gaming::Input::Custom::ICustomGameControllerFactory> >,struct Windows::Gaming::Input::IFlightStickStatics,class Microsoft::WRL::Details::Nil,0>::Release(void)	Unknown
        Windows.Gaming.Input.dll!Microsoft::WRL::ComPtr<`WaitForCompletion<Windows::Foundation::IAsyncOperationWithProgressCompletedHandler<Windows::Storage::Streams::IBuffer *,unsigned int>,Windows::Foundation::IAsyncOperationWithProgress<Windows::Storage::Streams::IBuffer *,unsigned int>>'::`2'::FTMEventDelegate>::~ComPtr<`WaitForCompletion<Windows::Foundation::IAsyncOperationWithProgressCompletedHandler<Windows::Storage::Streams::IBuffer *,unsigned int>,Windows::Foundation::IAsyncOperationWithProgress<Windows::Storage::Streams::IBuffer *,unsigned int>>'::`2'::FTMEventDelegate>()	Unknown
        Windows.Gaming.Input.dll!NtList<struct FactoryManager::FactoryListEntry>::~NtList<struct FactoryManager::FactoryListEntry>(void)	Unknown
        Windows.Gaming.Input.dll!FactoryManager::`vector deleting destructor'(unsigned int)	Unknown
        Windows.Gaming.Input.dll!Microsoft::WRL::ActivationFactory<struct Microsoft::WRL::Implements<class Microsoft::WRL::FtmBase,struct Windows::Gaming::Input::Custom::IGameControllerFactoryManagerStatics>,struct Windows::Gaming::Input::Custom::IGameControllerFactoryManagerStatics2,struct Microsoft::WRL::CloakedIid<struct Windows::Gaming::Input::Internal::IGameControllerFactoryManagerStaticsPrivate>,0>::Release(void)	Unknown
        Windows.Gaming.Input.dll!Microsoft::WRL::Details::TerminateMap(class Microsoft::WRL::Details::ModuleBase *,unsigned short const *,bool)	Unknown
        Windows.Gaming.Input.dll!Microsoft::WRL::Module<1,class Microsoft::WRL::Details::DefaultModule<1> >::~Module<1,class Microsoft::WRL::Details::DefaultModule<1> >(void)	Unknown
        Windows.Gaming.Input.dll!Microsoft::WRL::Details::DefaultModule<1>::`vector deleting destructor'(unsigned int)	Unknown
        Windows.Gaming.Input.dll!`dynamic atexit destructor for 'Microsoft::WRL::Details::StaticStorage<Microsoft::WRL::Details::DefaultModule<1>,0,int>::instance_''()	Unknown
        Windows.Gaming.Input.dll!__CRT_INIT@12()	Unknown
        Windows.Gaming.Input.dll!__DllMainCRTStartup()	Unknown
        ntdll.dll!_LdrxCallInitRoutine@16()	Unknown
        ntdll.dll!LdrpCallInitRoutine()	Unknown
        ntdll.dll!LdrpProcessDetachNode()	Unknown
        ntdll.dll!LdrpUnloadNode()	Unknown
        ntdll.dll!LdrpDecrementModuleLoadCountEx()	Unknown
        ntdll.dll!LdrUnloadDll()	Unknown
        KernelBase.dll!FreeLibrary()	Unknown
        combase.dll!FreeLibraryWithLogging(LoadOrFreeWhy why, HINSTANCE__ * hMod, const wchar_t * pswzOptionalFileName) Line 193	C++
        combase.dll!CClassCache::CDllPathEntry::CFinishObject::Finish() Line 3311	C++
        combase.dll!CClassCache::CFinishComposite::Finish() Line 3421	C++
        combase.dll!CClassCache::CleanUpDllsForProcess() Line 7009	C++
        [Inline Frame] combase.dll!CCCleanUpDllsForProcess() Line 8773	C++
        combase.dll!ProcessUninitialize() Line 2243	C++
        combase.dll!DecrementProcessInitializeCount() Line 993	C++
        combase.dll!wCoUninitialize(COleTls & Tls, int fHostThread) Line 4126	C++
        combase.dll!CoUninitialize() Line 3945	C++
    */
    /* WIN_CoUninitialize(); */
}

static SDL_bool
WGI_JoystickGetGamepadMapping(int device_index, SDL_GamepadMapping *out)
{
    return SDL_FALSE;
}

SDL_JoystickDriver SDL_WGI_JoystickDriver =
{
    WGI_JoystickInit,
    WGI_JoystickGetCount,
    WGI_JoystickDetect,
    WGI_JoystickGetDeviceName,
    WGI_JoystickGetDevicePlayerIndex,
    WGI_JoystickSetDevicePlayerIndex,
    WGI_JoystickGetDeviceGUID,
    WGI_JoystickGetDeviceInstanceID,
    WGI_JoystickOpen,
    WGI_JoystickRumble,
    WGI_JoystickRumbleTriggers,
    WGI_JoystickHasLED,
    WGI_JoystickSetLED,
    WGI_JoystickSendEffect,
    WGI_JoystickSetSensorsEnabled,
    WGI_JoystickUpdate,
    WGI_JoystickClose,
    WGI_JoystickQuit,
    WGI_JoystickGetGamepadMapping
};

#endif /* SDL_JOYSTICK_WGI */

/* vi: set ts=4 sw=4 expandtab: */
