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

#ifdef SDL_JOYSTICK_HIDAPI

#include "SDL_atomic.h"
#include "SDL_endian.h"
#include "SDL_hints.h"
#include "SDL_thread.h"
#include "SDL_timer.h"
#include "SDL_joystick.h"
#include "SDL_log.h"
#include "../SDL_sysjoystick.h"
#include "SDL_hidapijoystick_c.h"
#include "SDL_hidapi_rumble.h"
#include "../../SDL_hints_c.h"

#if defined(__WIN32__)
#include "../../core/windows/SDL_windows.h"
#include "../windows/SDL_rawinputjoystick_c.h"
#endif

#if defined(__MACOSX__)
#include <CoreFoundation/CoreFoundation.h>
#include <mach/mach.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDDevice.h>
#include <IOKit/usb/USBSpec.h>
#endif

#ifdef SDL_USE_LIBUDEV
#include "../../core/linux/SDL_udev.h"
#include <poll.h>
#include <unistd.h>
#endif

#ifdef HAVE_INOTIFY
#include <unistd.h>  /* just in case we didn't use that SDL_USE_LIBUDEV block... */
#include <errno.h>              /* errno, strerror */
#include <fcntl.h>
#include <limits.h>             /* For the definition of NAME_MAX */
#include <sys/inotify.h>
#endif

#if defined(SDL_USE_LIBUDEV)
typedef enum
{
    ENUMERATION_UNSET,
    ENUMERATION_LIBUDEV,
    ENUMERATION_FALLBACK
} LinuxEnumerationMethod;

static LinuxEnumerationMethod linux_enumeration_method = ENUMERATION_UNSET;
#endif

struct joystick_hwdata
{
    SDL_HIDAPI_Device *device;
};

static SDL_HIDAPI_DeviceDriver *SDL_HIDAPI_drivers[] = {
#ifdef SDL_JOYSTICK_HIDAPI_GAMECUBE
    &SDL_HIDAPI_DriverGameCube,
#endif
#ifdef SDL_JOYSTICK_HIDAPI_LUNA
    &SDL_HIDAPI_DriverLuna,
#endif
#ifdef SDL_JOYSTICK_HIDAPI_PS4
    &SDL_HIDAPI_DriverPS4,
#endif
#ifdef SDL_JOYSTICK_HIDAPI_PS5
    &SDL_HIDAPI_DriverPS5,
#endif
#ifdef SDL_JOYSTICK_HIDAPI_STADIA
    &SDL_HIDAPI_DriverStadia,
#endif
#ifdef SDL_JOYSTICK_HIDAPI_STEAM
    &SDL_HIDAPI_DriverSteam,
#endif
#ifdef SDL_JOYSTICK_HIDAPI_SWITCH
    &SDL_HIDAPI_DriverSwitch,
#endif
#ifdef SDL_JOYSTICK_HIDAPI_XBOX360
    &SDL_HIDAPI_DriverXbox360,
    &SDL_HIDAPI_DriverXbox360W,
#endif
#ifdef SDL_JOYSTICK_HIDAPI_XBOXONE
    &SDL_HIDAPI_DriverXboxOne,
#endif
};
static int SDL_HIDAPI_numdrivers = 0;
static SDL_SpinLock SDL_HIDAPI_spinlock;
static SDL_HIDAPI_Device *SDL_HIDAPI_devices;
static int SDL_HIDAPI_numjoysticks = 0;
static SDL_bool initialized = SDL_FALSE;
static SDL_bool shutting_down = SDL_FALSE;

#if defined(HAVE_INOTIFY)
static int inotify_fd = -1;
#endif

#if defined(SDL_USE_LIBUDEV)
static const SDL_UDEV_Symbols * usyms = NULL;
#endif

static struct
{
    SDL_bool m_bHaveDevicesChanged;
    SDL_bool m_bCanGetNotifications;
    Uint32 m_unLastDetect;

#if defined(__WIN32__)
    SDL_threadID m_nThreadID;
    WNDCLASSEXA m_wndClass;
    HWND m_hwndMsg;
    HDEVNOTIFY m_hNotify;
    double m_flLastWin32MessageCheck;
#endif

#if defined(__MACOSX__)
    IONotificationPortRef m_notificationPort;
    mach_port_t m_notificationMach;
#endif

#if defined(SDL_USE_LIBUDEV)
    struct udev *m_pUdev;
    struct udev_monitor *m_pUdevMonitor;
    int m_nUdevFd;
#endif
} SDL_HIDAPI_discovery;


#ifdef __WIN32__
struct _DEV_BROADCAST_HDR
{
    DWORD       dbch_size;
    DWORD       dbch_devicetype;
    DWORD       dbch_reserved;
};

typedef struct _DEV_BROADCAST_DEVICEINTERFACE_A
{
    DWORD       dbcc_size;
    DWORD       dbcc_devicetype;
    DWORD       dbcc_reserved;
    GUID        dbcc_classguid;
    char        dbcc_name[ 1 ];
} DEV_BROADCAST_DEVICEINTERFACE_A, *PDEV_BROADCAST_DEVICEINTERFACE_A;

typedef struct  _DEV_BROADCAST_HDR      DEV_BROADCAST_HDR;
#define DBT_DEVICEARRIVAL               0x8000  /* system detected a new device */
#define DBT_DEVICEREMOVECOMPLETE        0x8004  /* device was removed from the system */
#define DBT_DEVTYP_DEVICEINTERFACE      0x00000005  /* device interface class */
#define DBT_DEVNODES_CHANGED            0x0007
#define DBT_CONFIGCHANGED               0x0018
#define DBT_DEVICETYPESPECIFIC          0x8005  /* type specific event */
#define DBT_DEVINSTSTARTED              0x8008  /* device installed and started */

#include <initguid.h>
DEFINE_GUID(GUID_DEVINTERFACE_USB_DEVICE, 0xA5DCBF10L, 0x6530, 0x11D2, 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED);

static LRESULT CALLBACK ControllerWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_DEVICECHANGE:
        switch (wParam) {
        case DBT_DEVICEARRIVAL:
        case DBT_DEVICEREMOVECOMPLETE:
            if (((DEV_BROADCAST_HDR*)lParam)->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
                SDL_HIDAPI_discovery.m_bHaveDevicesChanged = SDL_TRUE;
            }
            break;
        }
        return TRUE;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}
#endif /* __WIN32__ */


#if defined(__MACOSX__)
static void CallbackIOServiceFunc(void *context, io_iterator_t portIterator)
{
    /* Must drain the iterator, or we won't receive new notifications */
    io_object_t entry;
    while ((entry = IOIteratorNext(portIterator)) != 0) {
        IOObjectRelease(entry);
        *(SDL_bool*)context = SDL_TRUE;
    }
}
#endif /* __MACOSX__ */

#ifdef HAVE_INOTIFY
#ifdef HAVE_INOTIFY_INIT1
static int SDL_inotify_init1(void) {
    return inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
}
#else
static int SDL_inotify_init1(void) {
    int fd = inotify_init();
    if (fd  < 0) return -1;
    fcntl(fd, F_SETFL, O_NONBLOCK);
    fcntl(fd, F_SETFD, FD_CLOEXEC);
    return fd;
}
#endif

static int
StrHasPrefix(const char *string, const char *prefix)
{
    return (SDL_strncmp(string, prefix, SDL_strlen(prefix)) == 0);
}

static int
StrIsInteger(const char *string)
{
    const char *p;

    if (*string == '\0') {
        return 0;
    }

    for (p = string; *p != '\0'; p++) {
        if (*p < '0' || *p > '9') {
            return 0;
        }
    }

    return 1;
}
#endif

static void
HIDAPI_InitializeDiscovery()
{
    SDL_HIDAPI_discovery.m_bHaveDevicesChanged = SDL_TRUE;
    SDL_HIDAPI_discovery.m_bCanGetNotifications = SDL_FALSE;
    SDL_HIDAPI_discovery.m_unLastDetect = 0;

#if defined(__WIN32__)
    SDL_HIDAPI_discovery.m_nThreadID = SDL_ThreadID();

    SDL_zero(SDL_HIDAPI_discovery.m_wndClass);
    SDL_HIDAPI_discovery.m_wndClass.hInstance = GetModuleHandle(NULL);
    SDL_HIDAPI_discovery.m_wndClass.lpszClassName = "SDL_HIDAPI_DEVICE_DETECTION";
    SDL_HIDAPI_discovery.m_wndClass.lpfnWndProc = ControllerWndProc;      /* This function is called by windows */
    SDL_HIDAPI_discovery.m_wndClass.cbSize = sizeof(WNDCLASSEX);

    RegisterClassExA(&SDL_HIDAPI_discovery.m_wndClass);
    SDL_HIDAPI_discovery.m_hwndMsg = CreateWindowExA(0, "SDL_HIDAPI_DEVICE_DETECTION", NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);

    {
        DEV_BROADCAST_DEVICEINTERFACE_A devBroadcast;

        SDL_zero(devBroadcast);
        devBroadcast.dbcc_size = sizeof( devBroadcast );
        devBroadcast.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        devBroadcast.dbcc_classguid = GUID_DEVINTERFACE_USB_DEVICE;

        /* DEVICE_NOTIFY_ALL_INTERFACE_CLASSES is important, makes GUID_DEVINTERFACE_USB_DEVICE ignored,
         * but that seems to be necessary to get a notice after each individual usb input device actually
         * installs, rather than just as the composite device is seen.
         */
        SDL_HIDAPI_discovery.m_hNotify = RegisterDeviceNotification( SDL_HIDAPI_discovery.m_hwndMsg, &devBroadcast, DEVICE_NOTIFY_WINDOW_HANDLE | DEVICE_NOTIFY_ALL_INTERFACE_CLASSES );
        SDL_HIDAPI_discovery.m_bCanGetNotifications = ( SDL_HIDAPI_discovery.m_hNotify != 0 );
    }
#endif /* __WIN32__ */

#if defined(__MACOSX__)
    SDL_HIDAPI_discovery.m_notificationPort = IONotificationPortCreate(kIOMasterPortDefault);
    if (SDL_HIDAPI_discovery.m_notificationPort) {
        {
            io_iterator_t portIterator = 0;
            io_object_t entry;
            IOReturn result = IOServiceAddMatchingNotification(
                SDL_HIDAPI_discovery.m_notificationPort,
                kIOFirstMatchNotification,
                IOServiceMatching(kIOHIDDeviceKey),
                CallbackIOServiceFunc, &SDL_HIDAPI_discovery.m_bHaveDevicesChanged, &portIterator);

            if (result == 0) {
                /* Must drain the existing iterator, or we won't receive new notifications */
                while ((entry = IOIteratorNext(portIterator)) != 0) {
                    IOObjectRelease(entry);
                }
            } else {
                IONotificationPortDestroy(SDL_HIDAPI_discovery.m_notificationPort);
                SDL_HIDAPI_discovery.m_notificationPort = nil;
            }
        }
        {
            io_iterator_t portIterator = 0;
            io_object_t entry;
            IOReturn result = IOServiceAddMatchingNotification(
                SDL_HIDAPI_discovery.m_notificationPort,
                kIOTerminatedNotification,
                IOServiceMatching(kIOHIDDeviceKey),
                CallbackIOServiceFunc, &SDL_HIDAPI_discovery.m_bHaveDevicesChanged, &portIterator);

            if (result == 0) {
                /* Must drain the existing iterator, or we won't receive new notifications */
                while ((entry = IOIteratorNext(portIterator)) != 0) {
                    IOObjectRelease(entry);
                }
            } else {
                IONotificationPortDestroy(SDL_HIDAPI_discovery.m_notificationPort);
                SDL_HIDAPI_discovery.m_notificationPort = nil;
            }
        }
    }

    SDL_HIDAPI_discovery.m_notificationMach = MACH_PORT_NULL;
    if (SDL_HIDAPI_discovery.m_notificationPort) {
        SDL_HIDAPI_discovery.m_notificationMach = IONotificationPortGetMachPort(SDL_HIDAPI_discovery.m_notificationPort);
    }

    SDL_HIDAPI_discovery.m_bCanGetNotifications = (SDL_HIDAPI_discovery.m_notificationMach != MACH_PORT_NULL);

#endif // __MACOSX__

#if defined(SDL_USE_LIBUDEV)
    if (linux_enumeration_method == ENUMERATION_LIBUDEV) {
        SDL_HIDAPI_discovery.m_pUdev = NULL;
        SDL_HIDAPI_discovery.m_pUdevMonitor = NULL;
        SDL_HIDAPI_discovery.m_nUdevFd = -1;

        usyms = SDL_UDEV_GetUdevSyms();
        if (usyms) {
            SDL_HIDAPI_discovery.m_pUdev = usyms->udev_new();
        }
        if (SDL_HIDAPI_discovery.m_pUdev) {
            SDL_HIDAPI_discovery.m_pUdevMonitor = usyms->udev_monitor_new_from_netlink(SDL_HIDAPI_discovery.m_pUdev, "udev");
            if (SDL_HIDAPI_discovery.m_pUdevMonitor) {
                usyms->udev_monitor_enable_receiving(SDL_HIDAPI_discovery.m_pUdevMonitor);
                SDL_HIDAPI_discovery.m_nUdevFd = usyms->udev_monitor_get_fd(SDL_HIDAPI_discovery.m_pUdevMonitor);
                SDL_HIDAPI_discovery.m_bCanGetNotifications = SDL_TRUE;
            }
        }
    }
    else
#endif /* SDL_USE_LIBUDEV */
    {
#if defined(HAVE_INOTIFY)
        inotify_fd = SDL_inotify_init1();

        if (inotify_fd < 0) {
            SDL_LogWarn(SDL_LOG_CATEGORY_INPUT,
                        "Unable to initialize inotify, falling back to polling: %s",
                        strerror(errno));
            return;
        }

        /* We need to watch for attribute changes in addition to
         * creation, because when a device is first created, it has
         * permissions that we can't read. When udev chmods it to
         * something that we maybe *can* read, we'll get an
         * IN_ATTRIB event to tell us. */
        if (inotify_add_watch(inotify_fd, "/dev",
                              IN_CREATE | IN_DELETE | IN_MOVE | IN_ATTRIB) < 0) {
            close(inotify_fd);
            inotify_fd = -1;
            SDL_LogWarn(SDL_LOG_CATEGORY_INPUT,
                        "Unable to add inotify watch, falling back to polling: %s",
                        strerror (errno));
            return;
        }

        SDL_HIDAPI_discovery.m_bCanGetNotifications = SDL_TRUE;
#endif /* HAVE_INOTIFY */
    }
}

static void
HIDAPI_UpdateDiscovery()
{
    if (!SDL_HIDAPI_discovery.m_bCanGetNotifications) {
        const Uint32 SDL_HIDAPI_DETECT_INTERVAL_MS = 3000;  /* Update every 3 seconds */
        Uint32 now = SDL_GetTicks();
        if (!SDL_HIDAPI_discovery.m_unLastDetect || SDL_TICKS_PASSED(now, SDL_HIDAPI_discovery.m_unLastDetect + SDL_HIDAPI_DETECT_INTERVAL_MS)) {
            SDL_HIDAPI_discovery.m_bHaveDevicesChanged = SDL_TRUE;
            SDL_HIDAPI_discovery.m_unLastDetect = now;
        }
        return;
    }

#if defined(__WIN32__)
#if 0 /* just let the usual SDL_PumpEvents loop dispatch these, fixing bug 4286. --ryan. */
    /* We'll only get messages on the same thread that created the window */
    if (SDL_ThreadID() == SDL_HIDAPI_discovery.m_nThreadID) {
        MSG msg;
        while (PeekMessage(&msg, SDL_HIDAPI_discovery.m_hwndMsg, 0, 0, PM_NOREMOVE)) {
            if (GetMessageA(&msg, SDL_HIDAPI_discovery.m_hwndMsg, 0, 0) != 0) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }
#endif
#endif /* __WIN32__ */

#if defined(__MACOSX__)
    if (SDL_HIDAPI_discovery.m_notificationPort) {
        struct { mach_msg_header_t hdr; char payload[ 4096 ]; } msg;
        while (mach_msg(&msg.hdr, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, sizeof(msg), SDL_HIDAPI_discovery.m_notificationMach, 0, MACH_PORT_NULL) == KERN_SUCCESS) {
            IODispatchCalloutFromMessage(NULL, &msg.hdr, SDL_HIDAPI_discovery.m_notificationPort);
        }
    }
#endif

#if defined(SDL_USE_LIBUDEV)
    if (linux_enumeration_method == ENUMERATION_LIBUDEV) {
        if (SDL_HIDAPI_discovery.m_nUdevFd >= 0) {
            /* Drain all notification events.
             * We don't expect a lot of device notifications so just
             * do a new discovery on any kind or number of notifications.
             * This could be made more restrictive if necessary.
             */
            for (;;) {
                struct pollfd PollUdev;
                struct udev_device *pUdevDevice;

                PollUdev.fd = SDL_HIDAPI_discovery.m_nUdevFd;
                PollUdev.events = POLLIN;
                if (poll(&PollUdev, 1, 0) != 1) {
                    break;
                }

                pUdevDevice = usyms->udev_monitor_receive_device(SDL_HIDAPI_discovery.m_pUdevMonitor);
                if (pUdevDevice) {
                    const char *action = NULL;
                    action = usyms->udev_device_get_action(pUdevDevice);
                    if (!action || SDL_strcmp(action, "add") == 0 || SDL_strcmp(action, "remove") == 0) {
                        SDL_HIDAPI_discovery.m_bHaveDevicesChanged = SDL_TRUE;
                    }
                    usyms->udev_device_unref(pUdevDevice);
                }
            }
        }
    }
    else
#endif /* SDL_USE_LIBUDEV */
    {
#if defined(HAVE_INOTIFY)
        if (inotify_fd >= 0) {
            union
            {
                struct inotify_event event;
                char storage[4096];
                char enough_for_inotify[sizeof (struct inotify_event) + NAME_MAX + 1];
            } buf;
            ssize_t bytes;
            size_t remain = 0;
            size_t len;

            bytes = read(inotify_fd, &buf, sizeof (buf));

            if (bytes > 0) {
                remain = (size_t) bytes;
            }

            while (remain > 0) {
                if (buf.event.len > 0 &&
                    !SDL_HIDAPI_discovery.m_bHaveDevicesChanged) {
                    if (StrHasPrefix(buf.event.name, "hidraw") &&
                        StrIsInteger(buf.event.name + strlen ("hidraw"))) {
                        SDL_HIDAPI_discovery.m_bHaveDevicesChanged = SDL_TRUE;
                        /* We found an hidraw change. We still continue to
                         * drain the inotify fd to avoid leaving old
                         * notifications in the queue. */
                    }
                }

                len = sizeof (struct inotify_event) + buf.event.len;
                remain -= len;

                if (remain != 0) {
                    memmove(&buf.storage[0], &buf.storage[len], remain);
                }
            }
        }
#endif /* HAVE_INOTIFY */
    }
}

static void
HIDAPI_ShutdownDiscovery()
{
#if defined(__WIN32__)
    if (SDL_HIDAPI_discovery.m_hNotify)
        UnregisterDeviceNotification(SDL_HIDAPI_discovery.m_hNotify);

    if (SDL_HIDAPI_discovery.m_hwndMsg) {
        DestroyWindow(SDL_HIDAPI_discovery.m_hwndMsg);
    }

    UnregisterClassA(SDL_HIDAPI_discovery.m_wndClass.lpszClassName, SDL_HIDAPI_discovery.m_wndClass.hInstance);
#endif

#if defined(__MACOSX__)
    if (SDL_HIDAPI_discovery.m_notificationPort) {
        IONotificationPortDestroy(SDL_HIDAPI_discovery.m_notificationPort);
    }
#endif

#if defined(SDL_USE_LIBUDEV)
    if (linux_enumeration_method == ENUMERATION_LIBUDEV &&
        usyms) {
        if (SDL_HIDAPI_discovery.m_pUdevMonitor) {
            usyms->udev_monitor_unref(SDL_HIDAPI_discovery.m_pUdevMonitor);
        }
        if (SDL_HIDAPI_discovery.m_pUdev) {
            usyms->udev_unref(SDL_HIDAPI_discovery.m_pUdev);
        }
        SDL_UDEV_ReleaseUdevSyms();
        usyms = NULL;
    }
#endif
}

void
HIDAPI_DumpPacket(const char *prefix, Uint8 *data, int size)
{
    int i;
    char *buffer;
    size_t length = SDL_strlen(prefix) + 11*(USB_PACKET_LENGTH/8) + (5*USB_PACKET_LENGTH*2) + 1 + 1;
    int start = 0, amount = size;

    buffer = (char *)SDL_malloc(length);
    SDL_snprintf(buffer, length, prefix, size);
    for (i = start; i < start+amount; ++i) {
        if ((i % 8) == 0) {
            SDL_snprintf(&buffer[SDL_strlen(buffer)], length - SDL_strlen(buffer), "\n%.2d:      ", i);
        }
        SDL_snprintf(&buffer[SDL_strlen(buffer)], length - SDL_strlen(buffer), " 0x%.2x", data[i]);
    }
    SDL_strlcat(buffer, "\n", length);
    SDL_Log("%s", buffer);
    SDL_free(buffer);
}

float
HIDAPI_RemapVal(float val, float val_min, float val_max, float output_min, float output_max)
{
    return output_min + (output_max - output_min) * (val - val_min) / (val_max - val_min);
}

static void HIDAPI_JoystickDetect(void);
static void HIDAPI_JoystickClose(SDL_Joystick *joystick);

static SDL_bool
HIDAPI_IsDeviceSupported(Uint16 vendor_id, Uint16 product_id, Uint16 version, const char *name)
{
    int i;
    SDL_GameControllerType type = SDL_GetJoystickGameControllerType(name, vendor_id, product_id, -1, 0, 0, 0);

    for (i = 0; i < SDL_arraysize(SDL_HIDAPI_drivers); ++i) {
        SDL_HIDAPI_DeviceDriver *driver = SDL_HIDAPI_drivers[i];
        if (driver->enabled && driver->IsSupportedDevice(name, type, vendor_id, product_id, version, -1, 0, 0, 0)) {
            return SDL_TRUE;
        }
    }
    return SDL_FALSE;
}

static SDL_HIDAPI_DeviceDriver *
HIDAPI_GetDeviceDriver(SDL_HIDAPI_Device *device)
{
    const Uint16 USAGE_PAGE_GENERIC_DESKTOP = 0x0001;
    const Uint16 USAGE_JOYSTICK = 0x0004;
    const Uint16 USAGE_GAMEPAD = 0x0005;
    const Uint16 USAGE_MULTIAXISCONTROLLER = 0x0008;
    int i;
    SDL_GameControllerType type;
    SDL_JoystickGUID check_guid;

    /* Make sure we have a generic GUID here, otherwise if we pass a HIDAPI
       guid, this call will create a game controller mapping for the device.
     */
    check_guid = device->guid;
    check_guid.data[14] = 0;
    if (SDL_ShouldIgnoreJoystick(device->name, check_guid)) {
        return NULL;
    }

    if (device->vendor_id != USB_VENDOR_VALVE) {
        if (device->usage_page && device->usage_page != USAGE_PAGE_GENERIC_DESKTOP) {
            return NULL;
        }
        if (device->usage && device->usage != USAGE_JOYSTICK && device->usage != USAGE_GAMEPAD && device->usage != USAGE_MULTIAXISCONTROLLER) {
            return NULL;
        }
    }

    type = SDL_GetJoystickGameControllerType(device->name, device->vendor_id, device->product_id, device->interface_number, device->interface_class, device->interface_subclass, device->interface_protocol);
    for (i = 0; i < SDL_arraysize(SDL_HIDAPI_drivers); ++i) {
        SDL_HIDAPI_DeviceDriver *driver = SDL_HIDAPI_drivers[i];
        if (driver->enabled && driver->IsSupportedDevice(device->name, type, device->vendor_id, device->product_id, device->version, device->interface_number, device->interface_class, device->interface_subclass, device->interface_protocol)) {
            return driver;
        }
    }
    return NULL;
}

static SDL_HIDAPI_Device *
HIDAPI_GetDeviceByIndex(int device_index, SDL_JoystickID *pJoystickID)
{
    SDL_HIDAPI_Device *device = SDL_HIDAPI_devices;
    while (device) {
        if (device->driver) {
            if (device_index < device->num_joysticks) {
                if (pJoystickID) {
                    *pJoystickID = device->joysticks[device_index];
                }
                return device;
            }
            device_index -= device->num_joysticks;
        }
        device = device->next;
    }
    return NULL;
}

static SDL_HIDAPI_Device *
HIDAPI_GetJoystickByInfo(const char *path, Uint16 vendor_id, Uint16 product_id)
{
    SDL_HIDAPI_Device *device = SDL_HIDAPI_devices;
    while (device) {
        if (device->vendor_id == vendor_id && device->product_id == product_id &&
            SDL_strcmp(device->path, path) == 0) {
            break;
        }
        device = device->next;
    }
    return device;
}

static void
HIDAPI_SetupDeviceDriver(SDL_HIDAPI_Device *device)
{
    if (device->driver) {
        /* Already setup */
        return;
    }

    device->driver = HIDAPI_GetDeviceDriver(device);
    if (device->driver) {
        const char *name = device->driver->GetDeviceName(device->vendor_id, device->product_id);
        if (name) {
            SDL_free(device->name);
            device->name = SDL_strdup(name);
        }
    }

    /* Initialize the device, which may cause a connected event */
    if (device->driver && !device->driver->InitDevice(device)) {
        device->driver = NULL;
    }
}

static void
HIDAPI_CleanupDeviceDriver(SDL_HIDAPI_Device *device)
{
    if (!device->driver) {
        /* Already cleaned up */
        return;
    }

    /* Disconnect any joysticks */
    while (device->num_joysticks) {
        HIDAPI_JoystickDisconnected(device, device->joysticks[0]);
    }

    device->driver->FreeDevice(device);
    device->driver = NULL;
}

static void SDLCALL
SDL_HIDAPIDriverHintChanged(void *userdata, const char *name, const char *oldValue, const char *hint)
{
    int i;
    SDL_HIDAPI_Device *device;
    SDL_bool enabled = SDL_GetStringBoolean(hint, SDL_TRUE);

    if (SDL_strcmp(name, SDL_HINT_JOYSTICK_HIDAPI) == 0) {
        for (i = 0; i < SDL_arraysize(SDL_HIDAPI_drivers); ++i) {
            SDL_HIDAPI_DeviceDriver *driver = SDL_HIDAPI_drivers[i];
            driver->enabled = SDL_GetHintBoolean(driver->hint, enabled);
        }
    } else {
        for (i = 0; i < SDL_arraysize(SDL_HIDAPI_drivers); ++i) {
            SDL_HIDAPI_DeviceDriver *driver = SDL_HIDAPI_drivers[i];
            if (SDL_strcmp(name, driver->hint) == 0) {
                driver->enabled = enabled;
            }
        }
    }

    SDL_HIDAPI_numdrivers = 0;
    for (i = 0; i < SDL_arraysize(SDL_HIDAPI_drivers); ++i) {
        SDL_HIDAPI_DeviceDriver *driver = SDL_HIDAPI_drivers[i];
        if (driver->enabled) {
            ++SDL_HIDAPI_numdrivers;
        }
    }

    /* Update device list if driver availability changes */
    SDL_LockJoysticks();

    for (device = SDL_HIDAPI_devices; device; device = device->next) {
        if (device->driver && !device->driver->enabled) {
            HIDAPI_CleanupDeviceDriver(device);
        }
        HIDAPI_SetupDeviceDriver(device);
    }

    SDL_UnlockJoysticks();
}

static int
HIDAPI_JoystickInit(void)
{
    int i;

    if (initialized) {
        return 0;
    }

#if defined(SDL_USE_LIBUDEV)
    if (linux_enumeration_method == ENUMERATION_UNSET) {
        if (SDL_getenv("SDL_HIDAPI_JOYSTICK_DISABLE_UDEV") != NULL) {
            SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
                         "udev disabled by SDL_HIDAPI_JOYSTICK_DISABLE_UDEV");
            linux_enumeration_method = ENUMERATION_FALLBACK;
        } else if (access("/.flatpak-info", F_OK) == 0
                   || access("/run/host/container-manager", F_OK) == 0) {
            /* Explicitly check `/.flatpak-info` because, for old versions of
             * Flatpak, this was the only available way to tell if we were in
             * a Flatpak container. */
            SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
                         "Container detected, disabling HIDAPI udev integration");
            linux_enumeration_method = ENUMERATION_FALLBACK;
        } else {
            SDL_LogDebug(SDL_LOG_CATEGORY_INPUT,
                         "Using udev for HIDAPI joystick device discovery");
            linux_enumeration_method = ENUMERATION_LIBUDEV;
        }
    }
#endif

#if defined(__MACOSX__) || defined(__IPHONEOS__) || defined(__TVOS__)
    /* The hidapi framwork is weak-linked on Apple platforms */
    int HID_API_EXPORT HID_API_CALL hid_init(void) __attribute__((weak_import));

    if (hid_init == NULL) {
        SDL_SetError("Couldn't initialize hidapi, framework not available");
        return -1;
    }
#endif /* __MACOSX__ || __IPHONEOS__ || __TVOS__ */

    if (hid_init() < 0) {
        SDL_SetError("Couldn't initialize hidapi");
        return -1;
    }

    for (i = 0; i < SDL_arraysize(SDL_HIDAPI_drivers); ++i) {
        SDL_HIDAPI_DeviceDriver *driver = SDL_HIDAPI_drivers[i];
        SDL_AddHintCallback(driver->hint, SDL_HIDAPIDriverHintChanged, NULL);
    }
    SDL_AddHintCallback(SDL_HINT_JOYSTICK_HIDAPI,
                        SDL_HIDAPIDriverHintChanged, NULL);
    HIDAPI_InitializeDiscovery();
    HIDAPI_JoystickDetect();
    HIDAPI_UpdateDevices();

    initialized = SDL_TRUE;

    return 0;
}

SDL_bool
HIDAPI_JoystickConnected(SDL_HIDAPI_Device *device, SDL_JoystickID *pJoystickID)
{
    SDL_JoystickID joystickID;
    SDL_JoystickID *joysticks = (SDL_JoystickID *)SDL_realloc(device->joysticks, (device->num_joysticks + 1)*sizeof(*device->joysticks));
    if (!joysticks) {
        return SDL_FALSE;
    }

    joystickID = SDL_GetNextJoystickInstanceID();
    device->joysticks = joysticks;
    device->joysticks[device->num_joysticks++] = joystickID;
    ++SDL_HIDAPI_numjoysticks;

    SDL_PrivateJoystickAdded(joystickID);

    if (pJoystickID) {
        *pJoystickID = joystickID;
    }
    return SDL_TRUE;
}

void
HIDAPI_JoystickDisconnected(SDL_HIDAPI_Device *device, SDL_JoystickID joystickID)
{
    int i, size;

    for (i = 0; i < device->num_joysticks; ++i) {
        if (device->joysticks[i] == joystickID) {
            SDL_Joystick *joystick = SDL_JoystickFromInstanceID(joystickID);
            if (joystick) {
                HIDAPI_JoystickClose(joystick);
            }

            size = (device->num_joysticks - i - 1) * sizeof(SDL_JoystickID);
            SDL_memmove(&device->joysticks[i], &device->joysticks[i+1], size);
            --device->num_joysticks;

            --SDL_HIDAPI_numjoysticks;
            if (device->num_joysticks == 0) {
                SDL_free(device->joysticks);
                device->joysticks = NULL;
            }

            if (!shutting_down) {
                SDL_PrivateJoystickRemoved(joystickID);
            }
            return;
        }
    }
}

static int
HIDAPI_JoystickGetCount(void)
{
    return SDL_HIDAPI_numjoysticks;
}

static char *
HIDAPI_ConvertString(const wchar_t *wide_string)
{
    char *string = NULL;

    if (wide_string) {
        string = SDL_iconv_string("UTF-8", "WCHAR_T", (char*)wide_string, (SDL_wcslen(wide_string)+1)*sizeof(wchar_t));
        if (!string) {
            if (sizeof(wchar_t) == sizeof(Uint16)) {
                string = SDL_iconv_string("UTF-8", "UCS-2-INTERNAL", (char*)wide_string, (SDL_wcslen(wide_string)+1)*sizeof(wchar_t));
            } else if (sizeof(wchar_t) == sizeof(Uint32)) {
                string = SDL_iconv_string("UTF-8", "UCS-4-INTERNAL", (char*)wide_string, (SDL_wcslen(wide_string)+1)*sizeof(wchar_t));
            }
        }
    }
    return string;
}

static void
HIDAPI_AddDevice(struct hid_device_info *info)
{
    SDL_HIDAPI_Device *device;
    SDL_HIDAPI_Device *curr, *last = NULL;

    for (curr = SDL_HIDAPI_devices, last = NULL; curr; last = curr, curr = curr->next) {
        continue;
    }

    device = (SDL_HIDAPI_Device *)SDL_calloc(1, sizeof(*device));
    if (!device) {
        return;
    }
    device->path = SDL_strdup(info->path);
    if (!device->path) {
        SDL_free(device);
        return;
    }
    device->seen = SDL_TRUE;
    device->vendor_id = info->vendor_id;
    device->product_id = info->product_id;
    device->version = info->release_number;
    device->interface_number = info->interface_number;
    device->interface_class = info->interface_class;
    device->interface_subclass = info->interface_subclass;
    device->interface_protocol = info->interface_protocol;
    device->usage_page = info->usage_page;
    device->usage = info->usage;
    {
        /* FIXME: Is there any way to tell whether this is a Bluetooth device? */
        const Uint16 vendor = device->vendor_id;
        const Uint16 product = device->product_id;
        const Uint16 version = device->version;
        Uint16 *guid16 = (Uint16 *)device->guid.data;

        *guid16++ = SDL_SwapLE16(SDL_HARDWARE_BUS_USB);
        *guid16++ = 0;
        *guid16++ = SDL_SwapLE16(vendor);
        *guid16++ = 0;
        *guid16++ = SDL_SwapLE16(product);
        *guid16++ = 0;
        *guid16++ = SDL_SwapLE16(version);
        *guid16++ = 0;

        /* Note that this is a HIDAPI device for special handling elsewhere */
        device->guid.data[14] = 'h';
        device->guid.data[15] = 0;
    }
    device->dev_lock = SDL_CreateMutex();

    /* Need the device name before getting the driver to know whether to ignore this device */
    {
        char *manufacturer_string = HIDAPI_ConvertString(info->manufacturer_string);
        char *product_string = HIDAPI_ConvertString(info->product_string);
        char *serial_number = HIDAPI_ConvertString(info->serial_number);

        device->name = SDL_CreateJoystickName(device->vendor_id, device->product_id, manufacturer_string, product_string);
        if (SDL_strncmp(device->name, "0x", 2) == 0) {
            /* Couldn't find a controller name, try to give it one based on device type */
            const char *name = NULL;

            switch (SDL_GetJoystickGameControllerType(NULL, device->vendor_id, device->product_id, device->interface_number, device->interface_class, device->interface_subclass, device->interface_protocol)) {
            case SDL_CONTROLLER_TYPE_XBOX360:
                name = "Xbox 360 Controller";
                break;
            case SDL_CONTROLLER_TYPE_XBOXONE:
                name = "Xbox One Controller";
                break;
            case SDL_CONTROLLER_TYPE_PS3:
                name = "PS3 Controller";
                break;
            case SDL_CONTROLLER_TYPE_PS4:
                name = "PS4 Controller";
                break;
            case SDL_CONTROLLER_TYPE_PS5:
                name = "PS5 Controller";
                break;
            case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO:
                name = "Nintendo Switch Pro Controller";
                break;
            default:
                break;
            }

            if (name) {
                SDL_free(device->name);
                device->name = SDL_strdup(name);
            }
        }

        if (manufacturer_string) {
            SDL_free(manufacturer_string);
        }
        if (product_string) {
            SDL_free(product_string);
        }

        if (serial_number && *serial_number) {
            device->serial = serial_number;
        } else {
            SDL_free(serial_number);
        }

        if (!device->name) {
            SDL_free(device->serial);
            SDL_free(device->path);
            SDL_free(device);
            return;
        }
    }

    /* Add it to the list */
    if (last) {
        last->next = device;
    } else {
        SDL_HIDAPI_devices = device;
    }

    HIDAPI_SetupDeviceDriver(device);

#ifdef DEBUG_HIDAPI
    SDL_Log("Added HIDAPI device '%s' VID 0x%.4x, PID 0x%.4x, version %d, serial %s, interface %d, interface_class %d, interface_subclass %d, interface_protocol %d, usage page 0x%.4x, usage 0x%.4x, path = %s, driver = %s (%s)\n", device->name, device->vendor_id, device->product_id, device->version, device->serial ? device->serial : "NONE", device->interface_number, device->interface_class, device->interface_subclass, device->interface_protocol, device->usage_page, device->usage, device->path, device->driver ? device->driver->hint : "NONE", device->driver && device->driver->enabled ? "ENABLED" : "DISABLED");
#endif
}


static void
HIDAPI_DelDevice(SDL_HIDAPI_Device *device)
{
    SDL_HIDAPI_Device *curr, *last;

#ifdef DEBUG_HIDAPI
    SDL_Log("Removing HIDAPI device '%s' VID 0x%.4x, PID 0x%.4x, version %d, serial %s, interface %d, interface_class %d, interface_subclass %d, interface_protocol %d, usage page 0x%.4x, usage 0x%.4x, path = %s, driver = %s (%s)\n", device->name, device->vendor_id, device->product_id, device->version, device->serial ? device->serial : "NONE", device->interface_number, device->interface_class, device->interface_subclass, device->interface_protocol, device->usage_page, device->usage, device->path, device->driver ? device->driver->hint : "NONE", device->driver && device->driver->enabled ? "ENABLED" : "DISABLED");
#endif

    for (curr = SDL_HIDAPI_devices, last = NULL; curr; last = curr, curr = curr->next) {
        if (curr == device) {
            if (last) {
                last->next = curr->next;
            } else {
                SDL_HIDAPI_devices = curr->next;
            }

            HIDAPI_CleanupDeviceDriver(device);

            /* Make sure the rumble thread is done with this device */
            while (SDL_AtomicGet(&device->rumble_pending) > 0) {
                SDL_Delay(10);
            }

            SDL_DestroyMutex(device->dev_lock);
            SDL_free(device->serial);
            SDL_free(device->name);
            SDL_free(device->path);
            SDL_free(device);
            return;
        }
    }
}

static void
HIDAPI_UpdateDeviceList(void)
{
    SDL_HIDAPI_Device *device;
    struct hid_device_info *devs, *info;

    SDL_LockJoysticks();

    /* Prepare the existing device list */
    device = SDL_HIDAPI_devices;
    while (device) {
        device->seen = SDL_FALSE;
        device = device->next;
    }

    /* Enumerate the devices */
    if (SDL_HIDAPI_numdrivers > 0) {
        devs = hid_enumerate(0, 0);
        if (devs) {
            for (info = devs; info; info = info->next) {
                device = HIDAPI_GetJoystickByInfo(info->path, info->vendor_id, info->product_id);
                if (device) {
                    device->seen = SDL_TRUE;
                } else {
                    HIDAPI_AddDevice(info);
                }
            }
            hid_free_enumeration(devs);
        }
    }

    /* Remove any devices that weren't seen or have been disconnected due to read errors */
    device = SDL_HIDAPI_devices;
    while (device) {
        SDL_HIDAPI_Device *next = device->next;

        if (!device->seen ||
            (device->driver && device->num_joysticks == 0 && !device->dev)) {
            HIDAPI_DelDevice(device);
        }
        device = next;
    }

    SDL_UnlockJoysticks();
}

static SDL_bool
HIDAPI_IsEquivalentToDevice(Uint16 vendor_id, Uint16 product_id, SDL_HIDAPI_Device *device)
{
    if (vendor_id == device->vendor_id && product_id == device->product_id) {
        return SDL_TRUE;
    }

    if (vendor_id == USB_VENDOR_MICROSOFT) {
        /* If we're looking for the wireless XBox 360 controller, also look for the dongle */
        if (product_id == USB_PRODUCT_XBOX360_XUSB_CONTROLLER && device->product_id == USB_PRODUCT_XBOX360_WIRELESS_RECEIVER) {
            return SDL_TRUE;
        }

        /* If we're looking for the raw input Xbox One controller, match it against any other Xbox One controller */
        if (product_id == USB_PRODUCT_XBOX_ONE_XBOXGIP_CONTROLLER &&
            SDL_GetJoystickGameControllerType(device->name, device->vendor_id, device->product_id, device->interface_number, device->interface_class, device->interface_subclass, device->interface_protocol) == SDL_CONTROLLER_TYPE_XBOXONE) {
            return SDL_TRUE;
        }

        /* If we're looking for an XInput controller, match it against any other Xbox controller */
        if (product_id == USB_PRODUCT_XBOX_ONE_XINPUT_CONTROLLER) {
            SDL_GameControllerType type = SDL_GetJoystickGameControllerType(device->name, device->vendor_id, device->product_id, device->interface_number, device->interface_class, device->interface_subclass, device->interface_protocol);
            if (type == SDL_CONTROLLER_TYPE_XBOX360 || type == SDL_CONTROLLER_TYPE_XBOXONE) {
                return SDL_TRUE;
            }
        }
    }
    return SDL_FALSE;
}

SDL_bool
HIDAPI_IsDeviceTypePresent(SDL_GameControllerType type)
{
    SDL_HIDAPI_Device *device;
    SDL_bool result = SDL_FALSE;

    /* Make sure we're initialized, as this could be called from other drivers during startup */
    if (HIDAPI_JoystickInit() < 0) {
        return SDL_FALSE;
    }

    if (SDL_AtomicTryLock(&SDL_HIDAPI_spinlock)) {
        HIDAPI_UpdateDeviceList();
        SDL_AtomicUnlock(&SDL_HIDAPI_spinlock);
    }

    SDL_LockJoysticks();
    device = SDL_HIDAPI_devices;
    while (device) {
        if (device->driver &&
            SDL_GetJoystickGameControllerType(device->name, device->vendor_id, device->product_id, device->interface_number, device->interface_class, device->interface_subclass, device->interface_protocol) == type) {
            result = SDL_TRUE;
            break;
        }
        device = device->next;
    }
    SDL_UnlockJoysticks();

#ifdef DEBUG_HIDAPI
    SDL_Log("HIDAPI_IsDeviceTypePresent() returning %s for %d\n", result ? "true" : "false", type);
#endif
    return result;
}

SDL_bool
HIDAPI_IsDevicePresent(Uint16 vendor_id, Uint16 product_id, Uint16 version, const char *name)
{
    SDL_HIDAPI_Device *device;
    SDL_bool supported = SDL_FALSE;
    SDL_bool result = SDL_FALSE;

    /* Make sure we're initialized, as this could be called from other drivers during startup */
    if (HIDAPI_JoystickInit() < 0) {
        return SDL_FALSE;
    }

    /* Only update the device list for devices we know might be supported.
       If we did this for every device, it would hit the USB driver too hard and potentially 
       lock up the system. This won't catch devices that we support but can only detect using 
       USB interface details, like Xbox controllers, but hopefully the device list update is
       responsive enough to catch those.
     */
    supported = HIDAPI_IsDeviceSupported(vendor_id, product_id, version, name);
#if defined(SDL_JOYSTICK_HIDAPI_XBOX360) || defined(SDL_JOYSTICK_HIDAPI_XBOXONE)
    if (!supported &&
        (SDL_strstr(name, "Xbox") || SDL_strstr(name, "X-Box") || SDL_strstr(name, "XBOX"))) {
        supported = SDL_TRUE;
    }
#endif /* SDL_JOYSTICK_HIDAPI_XBOX360 || SDL_JOYSTICK_HIDAPI_XBOXONE */
    if (supported) {
        if (SDL_AtomicTryLock(&SDL_HIDAPI_spinlock)) {
            HIDAPI_UpdateDeviceList();
            SDL_AtomicUnlock(&SDL_HIDAPI_spinlock);
        }
    }

    /* Note that this isn't a perfect check - there may be multiple devices with 0 VID/PID,
       or a different name than we have it listed here, etc, but if we support the device
       and we have something similar in our device list, mark it as present.
     */
    SDL_LockJoysticks();
    device = SDL_HIDAPI_devices;
    while (device) {
        if (device->driver &&
            HIDAPI_IsEquivalentToDevice(vendor_id, product_id, device)) {
            result = SDL_TRUE;
            break;
        }
        device = device->next;
    }
    SDL_UnlockJoysticks();

#ifdef DEBUG_HIDAPI
    SDL_Log("HIDAPI_IsDevicePresent() returning %s for 0x%.4x / 0x%.4x\n", result ? "true" : "false", vendor_id, product_id);
#endif
    return result;
}

static void
HIDAPI_JoystickDetect(void)
{
    if (SDL_AtomicTryLock(&SDL_HIDAPI_spinlock)) {
        HIDAPI_UpdateDiscovery();
        if (SDL_HIDAPI_discovery.m_bHaveDevicesChanged) {
            /* FIXME: We probably need to schedule an update in a few seconds as well */
            HIDAPI_UpdateDeviceList();
            SDL_HIDAPI_discovery.m_bHaveDevicesChanged = SDL_FALSE;
        }
        SDL_AtomicUnlock(&SDL_HIDAPI_spinlock);
    }
}

void
HIDAPI_UpdateDevices(void)
{
    SDL_HIDAPI_Device *device;

    /* Update the devices, which may change connected joysticks and send events */

    /* Prepare the existing device list */
    if (SDL_AtomicTryLock(&SDL_HIDAPI_spinlock)) {
        device = SDL_HIDAPI_devices;
        while (device) {
            if (device->driver) {
                if (SDL_TryLockMutex(device->dev_lock) == 0) {
                    device->updating = SDL_TRUE;
                    device->driver->UpdateDevice(device);
                    device->updating = SDL_FALSE;
                    SDL_UnlockMutex(device->dev_lock);
                }
            }
            device = device->next;
        }
        SDL_AtomicUnlock(&SDL_HIDAPI_spinlock);
    }
}

static const char *
HIDAPI_JoystickGetDeviceName(int device_index)
{
    SDL_HIDAPI_Device *device;
    const char *name = NULL;

    device = HIDAPI_GetDeviceByIndex(device_index, NULL);
    if (device) {
        /* FIXME: The device could be freed after this name is returned... */
        name = device->name;
    }

    return name;
}

static int
HIDAPI_JoystickGetDevicePlayerIndex(int device_index)
{
    SDL_HIDAPI_Device *device;
    SDL_JoystickID instance_id;
    int player_index = -1;

    device = HIDAPI_GetDeviceByIndex(device_index, &instance_id);
    if (device) {
        player_index = device->driver->GetDevicePlayerIndex(device, instance_id);
    }

    return player_index;
}

static void
HIDAPI_JoystickSetDevicePlayerIndex(int device_index, int player_index)
{
    SDL_HIDAPI_Device *device;
    SDL_JoystickID instance_id;

    device = HIDAPI_GetDeviceByIndex(device_index, &instance_id);
    if (device) {
        device->driver->SetDevicePlayerIndex(device, instance_id, player_index);
    }
}

static SDL_JoystickGUID
HIDAPI_JoystickGetDeviceGUID(int device_index)
{
    SDL_HIDAPI_Device *device;
    SDL_JoystickGUID guid;

    device = HIDAPI_GetDeviceByIndex(device_index, NULL);
    if (device) {
        SDL_memcpy(&guid, &device->guid, sizeof(guid));
    } else {
        SDL_zero(guid);
    }

    return guid;
}

static SDL_JoystickID
HIDAPI_JoystickGetDeviceInstanceID(int device_index)
{
    SDL_JoystickID joystickID = -1;
    HIDAPI_GetDeviceByIndex(device_index, &joystickID);
    return joystickID;
}

static int
HIDAPI_JoystickOpen(SDL_Joystick *joystick, int device_index)
{
    SDL_JoystickID joystickID;
    SDL_HIDAPI_Device *device = HIDAPI_GetDeviceByIndex(device_index, &joystickID);
    struct joystick_hwdata *hwdata;

    hwdata = (struct joystick_hwdata *)SDL_calloc(1, sizeof(*hwdata));
    if (!hwdata) {
        return SDL_OutOfMemory();
    }
    hwdata->device = device;

    if (!device->driver->OpenJoystick(device, joystick)) {
        SDL_free(hwdata);
        return -1;
    }

    if (!joystick->serial && device->serial) {
        joystick->serial = SDL_strdup(device->serial);
    }

    joystick->hwdata = hwdata;
    return 0;
}

static int
HIDAPI_JoystickRumble(SDL_Joystick *joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble)
{
    int result;

    if (joystick->hwdata) {
        SDL_HIDAPI_Device *device = joystick->hwdata->device;

        result = device->driver->RumbleJoystick(device, joystick, low_frequency_rumble, high_frequency_rumble);
    } else {
        SDL_SetError("Rumble failed, device disconnected");
        result = -1;
    }

    return result;
}

static int
HIDAPI_JoystickRumbleTriggers(SDL_Joystick *joystick, Uint16 left_rumble, Uint16 right_rumble)
{
    int result;

    if (joystick->hwdata) {
        SDL_HIDAPI_Device *device = joystick->hwdata->device;

        result = device->driver->RumbleJoystickTriggers(device, joystick, left_rumble, right_rumble);
    } else {
        SDL_SetError("Rumble failed, device disconnected");
        result = -1;
    }

    return result;
}

static SDL_bool
HIDAPI_JoystickHasLED(SDL_Joystick *joystick)
{
    SDL_bool result = SDL_FALSE;

    if (joystick->hwdata) {
        SDL_HIDAPI_Device *device = joystick->hwdata->device;

        result = device->driver->HasJoystickLED(device, joystick);
    }

    return result;
}

static int
HIDAPI_JoystickSetLED(SDL_Joystick *joystick, Uint8 red, Uint8 green, Uint8 blue)
{
    int result;

    if (joystick->hwdata) {
        SDL_HIDAPI_Device *device = joystick->hwdata->device;

        result = device->driver->SetJoystickLED(device, joystick, red, green, blue);
    } else {
        SDL_SetError("SetLED failed, device disconnected");
        result = -1;
    }

    return result;
}

static int
HIDAPI_JoystickSendEffect(SDL_Joystick *joystick, const void *data, int size)
{
    int result;

    if (joystick->hwdata) {
        SDL_HIDAPI_Device *device = joystick->hwdata->device;

        result = device->driver->SendJoystickEffect(device, joystick, data, size);
    } else {
        SDL_SetError("SendEffect failed, device disconnected");
        result = -1;
    }

    return result;
}

static int
HIDAPI_JoystickSetSensorsEnabled(SDL_Joystick *joystick, SDL_bool enabled)
{
    int result;

    if (joystick->hwdata) {
        SDL_HIDAPI_Device *device = joystick->hwdata->device;

        result = device->driver->SetJoystickSensorsEnabled(device, joystick, enabled);
    } else {
        SDL_SetError("SetSensorsEnabled failed, device disconnected");
        result = -1;
    }

    return result;
}

static void
HIDAPI_JoystickUpdate(SDL_Joystick *joystick)
{
    /* This is handled in SDL_HIDAPI_UpdateDevices() */
}

static void
HIDAPI_JoystickClose(SDL_Joystick *joystick)
{
    if (joystick->hwdata) {
        SDL_HIDAPI_Device *device = joystick->hwdata->device;
        int i;

        /* Wait up to 30 ms for pending rumble to complete */
        if (device->updating) {
            /* Unlock the device so rumble can complete */
            SDL_UnlockMutex(device->dev_lock);
        }
        for (i = 0; i < 3; ++i) {
            if (SDL_AtomicGet(&device->rumble_pending) > 0) {
                SDL_Delay(10);
            }
        }
        if (device->updating) {
            /* Relock the device */
            SDL_LockMutex(device->dev_lock);
        }

        device->driver->CloseJoystick(device, joystick);

        SDL_free(joystick->hwdata);
        joystick->hwdata = NULL;
    }
}

static void
HIDAPI_JoystickQuit(void)
{
    int i;

    shutting_down = SDL_TRUE;

    HIDAPI_ShutdownDiscovery();

    SDL_HIDAPI_QuitRumble();

#if defined(HAVE_INOTIFY)
    if (inotify_fd >= 0) {
        close(inotify_fd);
        inotify_fd = -1;
    }
#endif

    while (SDL_HIDAPI_devices) {
        HIDAPI_DelDevice(SDL_HIDAPI_devices);
    }

    /* Make sure the drivers cleaned up properly */
    SDL_assert(SDL_HIDAPI_numjoysticks == 0);

    for (i = 0; i < SDL_arraysize(SDL_HIDAPI_drivers); ++i) {
        SDL_HIDAPI_DeviceDriver *driver = SDL_HIDAPI_drivers[i];
        SDL_DelHintCallback(driver->hint, SDL_HIDAPIDriverHintChanged, NULL);
    }
    SDL_DelHintCallback(SDL_HINT_JOYSTICK_HIDAPI,
                        SDL_HIDAPIDriverHintChanged, NULL);

    hid_exit();

    shutting_down = SDL_FALSE;
    initialized = SDL_FALSE;
}

static SDL_bool
HIDAPI_JoystickGetGamepadMapping(int device_index, SDL_GamepadMapping *out)
{
    return SDL_FALSE;
}

SDL_JoystickDriver SDL_HIDAPI_JoystickDriver =
{
    HIDAPI_JoystickInit,
    HIDAPI_JoystickGetCount,
    HIDAPI_JoystickDetect,
    HIDAPI_JoystickGetDeviceName,
    HIDAPI_JoystickGetDevicePlayerIndex,
    HIDAPI_JoystickSetDevicePlayerIndex,
    HIDAPI_JoystickGetDeviceGUID,
    HIDAPI_JoystickGetDeviceInstanceID,
    HIDAPI_JoystickOpen,
    HIDAPI_JoystickRumble,
    HIDAPI_JoystickRumbleTriggers,
    HIDAPI_JoystickHasLED,
    HIDAPI_JoystickSetLED,
    HIDAPI_JoystickSendEffect,
    HIDAPI_JoystickSetSensorsEnabled,
    HIDAPI_JoystickUpdate,
    HIDAPI_JoystickClose,
    HIDAPI_JoystickQuit,
    HIDAPI_JoystickGetGamepadMapping
};

#endif /* SDL_JOYSTICK_HIDAPI */

/* vi: set ts=4 sw=4 expandtab: */
