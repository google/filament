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

#if SDL_JOYSTICK_DINPUT || SDL_JOYSTICK_XINPUT

/* DirectInput joystick driver; written by Glenn Maynard, based on Andrei de
 * A. Formiga's WINMM driver.
 *
 * Hats and sliders are completely untested; the app I'm writing this for mostly
 * doesn't use them and I don't own any joysticks with them.
 *
 * We don't bother to use event notification here.  It doesn't seem to work
 * with polled devices, and it's fine to call IDirectInputDevice8_GetDeviceData and
 * let it return 0 events. */

#include "SDL_error.h"
#include "SDL_assert.h"
#include "SDL_events.h"
#include "SDL_timer.h"
#include "SDL_mutex.h"
#include "SDL_joystick.h"
#include "../SDL_sysjoystick.h"
#include "../../thread/SDL_systhread.h"
#include "../../core/windows/SDL_windows.h"
#if !defined(__WINRT__)
#include <dbt.h>
#endif

#define INITGUID /* Only set here, if set twice will cause mingw32 to break. */
#include "SDL_windowsjoystick_c.h"
#include "SDL_dinputjoystick_c.h"
#include "SDL_xinputjoystick_c.h"

#include "../../haptic/windows/SDL_dinputhaptic_c.h"    /* For haptic hot plugging */
#include "../../haptic/windows/SDL_xinputhaptic_c.h"    /* For haptic hot plugging */


#ifndef DEVICE_NOTIFY_WINDOW_HANDLE
#define DEVICE_NOTIFY_WINDOW_HANDLE 0x00000000
#endif

/* local variables */
static SDL_bool s_bDeviceAdded = SDL_FALSE;
static SDL_bool s_bDeviceRemoved = SDL_FALSE;
static SDL_JoystickID s_nInstanceID = -1;
static SDL_cond *s_condJoystickThread = NULL;
static SDL_mutex *s_mutexJoyStickEnum = NULL;
static SDL_Thread *s_threadJoystick = NULL;
static SDL_bool s_bJoystickThreadQuit = SDL_FALSE;

JoyStick_DeviceData *SYS_Joystick;    /* array to hold joystick ID values */

static SDL_bool s_bWindowsDeviceChanged = SDL_FALSE;

#ifdef __WINRT__

typedef struct
{
    int unused;
} SDL_DeviceNotificationData;

static void
SDL_CleanupDeviceNotification(SDL_DeviceNotificationData *data)
{
}

static int
SDL_CreateDeviceNotification(SDL_DeviceNotificationData *data)
{
    return 0;
}

static SDL_bool
SDL_WaitForDeviceNotification(SDL_DeviceNotificationData *data, SDL_mutex *mutex)
{
    return SDL_FALSE;
}

#else /* !__WINRT__ */

typedef struct
{
    HRESULT coinitialized;
    WNDCLASSEX wincl;
    HWND messageWindow;
    HDEVNOTIFY hNotify;
} SDL_DeviceNotificationData;

#define IDT_SDL_DEVICE_CHANGE_TIMER_1 1200
#define IDT_SDL_DEVICE_CHANGE_TIMER_2 1201

/* windowproc for our joystick detect thread message only window, to detect any USB device addition/removal */
static LRESULT CALLBACK
SDL_PrivateJoystickDetectProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_DEVICECHANGE:
        switch (wParam) {
        case DBT_DEVICEARRIVAL:
        case DBT_DEVICEREMOVECOMPLETE:
            if (((DEV_BROADCAST_HDR*)lParam)->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
                /* notify 300ms and 2 seconds later to ensure all APIs have updated status */
                SetTimer(hwnd, IDT_SDL_DEVICE_CHANGE_TIMER_1, 300, NULL);
                SetTimer(hwnd, IDT_SDL_DEVICE_CHANGE_TIMER_2, 2000, NULL);
            }
            break;
        }
        return 0;
    case WM_TIMER:
        KillTimer(hwnd, wParam);
        s_bWindowsDeviceChanged = SDL_TRUE;
        return 0;
    }

    return DefWindowProc (hwnd, message, wParam, lParam);
}

static void
SDL_CleanupDeviceNotification(SDL_DeviceNotificationData *data)
{
    if (data->hNotify)
        UnregisterDeviceNotification(data->hNotify);

    if (data->messageWindow)
        DestroyWindow(data->messageWindow);

    UnregisterClass(data->wincl.lpszClassName, data->wincl.hInstance);

    if (data->coinitialized == S_OK) {
        WIN_CoUninitialize();
    }
}

static int
SDL_CreateDeviceNotification(SDL_DeviceNotificationData *data)
{
    DEV_BROADCAST_DEVICEINTERFACE dbh;
    GUID GUID_DEVINTERFACE_HID = { 0x4D1E55B2L, 0xF16F, 0x11CF, { 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } };

    SDL_zerop(data);

    data->coinitialized = WIN_CoInitialize();

    data->wincl.hInstance = GetModuleHandle(NULL);
    data->wincl.lpszClassName = L"Message";
    data->wincl.lpfnWndProc = SDL_PrivateJoystickDetectProc;      /* This function is called by windows */
    data->wincl.cbSize = sizeof (WNDCLASSEX);

    if (!RegisterClassEx(&data->wincl)) {
        WIN_SetError("Failed to create register class for joystick autodetect");
        SDL_CleanupDeviceNotification(data);
        return -1;
    }

    data->messageWindow = (HWND)CreateWindowEx(0,  L"Message", NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
    if (!data->messageWindow) {
        WIN_SetError("Failed to create message window for joystick autodetect");
        SDL_CleanupDeviceNotification(data);
        return -1;
    }

    SDL_zero(dbh);
    dbh.dbcc_size = sizeof(dbh);
    dbh.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    dbh.dbcc_classguid = GUID_DEVINTERFACE_HID;

    data->hNotify = RegisterDeviceNotification(data->messageWindow, &dbh, DEVICE_NOTIFY_WINDOW_HANDLE);
    if (!data->hNotify) {
        WIN_SetError("Failed to create notify device for joystick autodetect");
        SDL_CleanupDeviceNotification(data);
        return -1;
    }
    return 0;
}

static SDL_bool
SDL_WaitForDeviceNotification(SDL_DeviceNotificationData *data, SDL_mutex *mutex)
{
    MSG msg;
    int lastret = 1;

    if (!data->messageWindow) {
        return SDL_FALSE; /* device notifications require a window */
    }

    SDL_UnlockMutex(mutex);
    while (lastret > 0 && s_bWindowsDeviceChanged == SDL_FALSE) {
        lastret = GetMessage(&msg, NULL, 0, 0); /* WM_QUIT causes return value of 0 */
        if (lastret > 0) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    SDL_LockMutex(mutex);
    return (lastret != -1) ? SDL_TRUE : SDL_FALSE;
}

#endif /* __WINRT__ */

/* Function/thread to scan the system for joysticks. */
static int
SDL_JoystickThread(void *_data)
{
    SDL_DeviceNotificationData notification_data;

#if SDL_JOYSTICK_XINPUT
    SDL_bool bOpenedXInputDevices[XUSER_MAX_COUNT];
    SDL_zero(bOpenedXInputDevices);
#endif

    if (SDL_CreateDeviceNotification(&notification_data) < 0) {
        return -1;
    }

    SDL_LockMutex(s_mutexJoyStickEnum);
    while (s_bJoystickThreadQuit == SDL_FALSE) {
        SDL_bool bXInputChanged = SDL_FALSE;

        if (SDL_WaitForDeviceNotification(&notification_data, s_mutexJoyStickEnum) == SDL_FALSE) {
#if SDL_JOYSTICK_XINPUT
            /* WM_DEVICECHANGE not working, poll for new XINPUT controllers */
            SDL_CondWaitTimeout(s_condJoystickThread, s_mutexJoyStickEnum, 1000);
            if (SDL_XINPUT_Enabled() && XINPUTGETCAPABILITIES) {
                /* scan for any change in XInput devices */
                Uint8 userId;
                for (userId = 0; userId < XUSER_MAX_COUNT; userId++) {
                    XINPUT_CAPABILITIES capabilities;
                    const DWORD result = XINPUTGETCAPABILITIES(userId, XINPUT_FLAG_GAMEPAD, &capabilities);
                    const SDL_bool available = (result == ERROR_SUCCESS);
                    if (bOpenedXInputDevices[userId] != available) {
                        bXInputChanged = SDL_TRUE;
                        bOpenedXInputDevices[userId] = available;
                    }
                }
            }
#else
            /* WM_DEVICECHANGE not working, no XINPUT, no point in keeping thread alive */
            break;
#endif /* SDL_JOYSTICK_XINPUT */
		}

        if (s_bWindowsDeviceChanged || bXInputChanged) {
            s_bDeviceRemoved = SDL_TRUE;
            s_bDeviceAdded = SDL_TRUE;
            s_bWindowsDeviceChanged = SDL_FALSE;
        }
    }
    SDL_UnlockMutex(s_mutexJoyStickEnum);

    SDL_CleanupDeviceNotification(&notification_data);

    return 1;
}

void SDL_SYS_AddJoystickDevice(JoyStick_DeviceData *device)
{
    device->send_add_event = SDL_TRUE;
    device->nInstanceID = ++s_nInstanceID;
    device->pNext = SYS_Joystick;
    SYS_Joystick = device;

    s_bDeviceAdded = SDL_TRUE;
}

/* Function to scan the system for joysticks.
 * Joystick 0 should be the system default joystick.
 * It should return 0, or -1 on an unrecoverable fatal error.
 */
int
SDL_SYS_JoystickInit(void)
{
    if (SDL_DINPUT_JoystickInit() < 0) {
        SDL_SYS_JoystickQuit();
        return -1;
    }

    if (SDL_XINPUT_JoystickInit() < 0) {
        SDL_SYS_JoystickQuit();
        return -1;
    }

    s_mutexJoyStickEnum = SDL_CreateMutex();
    s_condJoystickThread = SDL_CreateCond();
    s_bDeviceAdded = SDL_TRUE; /* force a scan of the system for joysticks this first time */

    SDL_SYS_JoystickDetect();

    if (!s_threadJoystick) {
        /* spin up the thread to detect hotplug of devices */
        s_bJoystickThreadQuit = SDL_FALSE;
        s_threadJoystick = SDL_CreateThreadInternal(SDL_JoystickThread, "SDL_joystick", 64 * 1024, NULL);
    }
    return SDL_SYS_NumJoysticks();
}

/* return the number of joysticks that are connected right now */
int
SDL_SYS_NumJoysticks(void)
{
    int nJoysticks = 0;
    JoyStick_DeviceData *device = SYS_Joystick;
    while (device) {
        nJoysticks++;
        device = device->pNext;
    }

    return nJoysticks;
}

/* detect any new joysticks being inserted into the system */
void
SDL_SYS_JoystickDetect(void)
{
    JoyStick_DeviceData *pCurList = NULL;

    /* only enum the devices if the joystick thread told us something changed */
    if (!s_bDeviceAdded && !s_bDeviceRemoved) {
        return;  /* thread hasn't signaled, nothing to do right now. */
    }

    SDL_LockMutex(s_mutexJoyStickEnum);

    s_bDeviceAdded = SDL_FALSE;
    s_bDeviceRemoved = SDL_FALSE;

    pCurList = SYS_Joystick;
    SYS_Joystick = NULL;

    /* Look for DirectInput joysticks, wheels, head trackers, gamepads, etc.. */
    SDL_DINPUT_JoystickDetect(&pCurList);

    /* Look for XInput devices. Do this last, so they're first in the final list. */
    SDL_XINPUT_JoystickDetect(&pCurList);

    SDL_UnlockMutex(s_mutexJoyStickEnum);

    while (pCurList) {
        JoyStick_DeviceData *pListNext = NULL;

        if (pCurList->bXInputDevice) {
            SDL_XINPUT_MaybeRemoveDevice(pCurList->XInputUserId);
        } else {
            SDL_DINPUT_MaybeRemoveDevice(&pCurList->dxdevice);
        }

        SDL_PrivateJoystickRemoved(pCurList->nInstanceID);

        pListNext = pCurList->pNext;
        SDL_free(pCurList->joystickname);
        SDL_free(pCurList);
        pCurList = pListNext;
    }

    if (s_bDeviceAdded) {
        JoyStick_DeviceData *pNewJoystick;
        int device_index = 0;
        s_bDeviceAdded = SDL_FALSE;
        pNewJoystick = SYS_Joystick;
        while (pNewJoystick) {
            if (pNewJoystick->send_add_event) {
                if (pNewJoystick->bXInputDevice) {
                    SDL_XINPUT_MaybeAddDevice(pNewJoystick->XInputUserId);
                } else {
                    SDL_DINPUT_MaybeAddDevice(&pNewJoystick->dxdevice);
                }

                SDL_PrivateJoystickAdded(device_index);

                pNewJoystick->send_add_event = SDL_FALSE;
            }
            device_index++;
            pNewJoystick = pNewJoystick->pNext;
        }
    }
}

/* Function to get the device-dependent name of a joystick */
const char *
SDL_SYS_JoystickNameForDeviceIndex(int device_index)
{
    JoyStick_DeviceData *device = SYS_Joystick;

    for (; device_index > 0; device_index--)
        device = device->pNext;

    return device->joystickname;
}

/* Function to perform the mapping between current device instance and this joysticks instance id */
SDL_JoystickID
SDL_SYS_GetInstanceIdOfDeviceIndex(int device_index)
{
    JoyStick_DeviceData *device = SYS_Joystick;
    int index;

    for (index = device_index; index > 0; index--)
        device = device->pNext;

    return device->nInstanceID;
}

/* Function to open a joystick for use.
   The joystick to open is specified by the device index.
   This should fill the nbuttons and naxes fields of the joystick structure.
   It returns 0, or -1 if there is an error.
 */
int
SDL_SYS_JoystickOpen(SDL_Joystick * joystick, int device_index)
{
    JoyStick_DeviceData *joystickdevice = SYS_Joystick;

    for (; device_index > 0; device_index--)
        joystickdevice = joystickdevice->pNext;

    /* allocate memory for system specific hardware data */
    joystick->instance_id = joystickdevice->nInstanceID;
    joystick->hwdata =
        (struct joystick_hwdata *) SDL_malloc(sizeof(struct joystick_hwdata));
    if (joystick->hwdata == NULL) {
        return SDL_OutOfMemory();
    }
    SDL_zerop(joystick->hwdata);
    joystick->hwdata->guid = joystickdevice->guid;

    if (joystickdevice->bXInputDevice) {
        return SDL_XINPUT_JoystickOpen(joystick, joystickdevice);
    } else {
        return SDL_DINPUT_JoystickOpen(joystick, joystickdevice);
    }
}

/* return true if this joystick is plugged in right now */
SDL_bool 
SDL_SYS_JoystickAttached(SDL_Joystick * joystick)
{
    return joystick->hwdata && !joystick->hwdata->removed;
}

void
SDL_SYS_JoystickUpdate(SDL_Joystick * joystick)
{
    if (!joystick->hwdata || joystick->hwdata->removed) {
        return;
    }

    if (joystick->hwdata->bXInputDevice) {
        SDL_XINPUT_JoystickUpdate(joystick);
    } else {
        SDL_DINPUT_JoystickUpdate(joystick);
    }

    if (joystick->hwdata->removed) {
        joystick->force_recentering = SDL_TRUE;
    }
}

/* Function to close a joystick after use */
void
SDL_SYS_JoystickClose(SDL_Joystick * joystick)
{
    if (joystick->hwdata->bXInputDevice) {
        SDL_XINPUT_JoystickClose(joystick);
    } else {
        SDL_DINPUT_JoystickClose(joystick);
    }

    SDL_free(joystick->hwdata);
}

/* Function to perform any system-specific joystick related cleanup */
void
SDL_SYS_JoystickQuit(void)
{
    JoyStick_DeviceData *device = SYS_Joystick;

    while (device) {
        JoyStick_DeviceData *device_next = device->pNext;
        SDL_free(device->joystickname);
        SDL_free(device);
        device = device_next;
    }
    SYS_Joystick = NULL;

    if (s_threadJoystick) {
        SDL_LockMutex(s_mutexJoyStickEnum);
        s_bJoystickThreadQuit = SDL_TRUE;
        SDL_CondBroadcast(s_condJoystickThread); /* signal the joystick thread to quit */
        SDL_UnlockMutex(s_mutexJoyStickEnum);
#ifndef __WINRT__
        PostThreadMessage(SDL_GetThreadID(s_threadJoystick), WM_QUIT, 0, 0);
#endif
        SDL_WaitThread(s_threadJoystick, NULL); /* wait for it to bugger off */

        SDL_DestroyMutex(s_mutexJoyStickEnum);
        SDL_DestroyCond(s_condJoystickThread);
        s_condJoystickThread= NULL;
        s_mutexJoyStickEnum = NULL;
        s_threadJoystick = NULL;
    }

    SDL_DINPUT_JoystickQuit();
    SDL_XINPUT_JoystickQuit();

    s_bDeviceAdded = SDL_FALSE;
    s_bDeviceRemoved = SDL_FALSE;
}

/* return the stable device guid for this device index */
SDL_JoystickGUID
SDL_SYS_JoystickGetDeviceGUID(int device_index)
{
    JoyStick_DeviceData *device = SYS_Joystick;
    int index;

    for (index = device_index; index > 0; index--)
        device = device->pNext;

    return device->guid;
}

SDL_JoystickGUID
SDL_SYS_JoystickGetGUID(SDL_Joystick * joystick)
{
    return joystick->hwdata->guid;
}

#endif /* SDL_JOYSTICK_DINPUT || SDL_JOYSTICK_XINPUT */

/* vi: set ts=4 sw=4 expandtab: */
