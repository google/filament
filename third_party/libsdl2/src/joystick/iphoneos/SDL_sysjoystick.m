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

/* This is the iOS implementation of the SDL joystick API */
#include "SDL_sysjoystick_c.h"

/* needed for SDL_IPHONE_MAX_GFORCE macro */
#include "SDL_config_iphoneos.h"

#include "SDL_assert.h"
#include "SDL_events.h"
#include "SDL_joystick.h"
#include "SDL_hints.h"
#include "SDL_stdinc.h"
#include "../SDL_sysjoystick.h"
#include "../SDL_joystick_c.h"
#include "../steam/SDL_steamcontroller.h"


#if !SDL_EVENTS_DISABLED
#include "../../events/SDL_events_c.h"
#endif

#if !TARGET_OS_TV
#import <CoreMotion/CoreMotion.h>
#endif

#ifdef SDL_JOYSTICK_MFI
#import <GameController/GameController.h>

static id connectObserver = nil;
static id disconnectObserver = nil;
#endif /* SDL_JOYSTICK_MFI */

#if !TARGET_OS_TV
static const char *accelerometerName = "iOS Accelerometer";
static CMMotionManager *motionManager = nil;
#endif /* !TARGET_OS_TV */

static SDL_JoystickDeviceItem *deviceList = NULL;

static int numjoysticks = 0;
static SDL_JoystickID instancecounter = 0;
int SDL_AppleTVRemoteOpenedAsJoystick = 0;

static SDL_JoystickDeviceItem *
GetDeviceForIndex(int device_index)
{
    SDL_JoystickDeviceItem *device = deviceList;
    int i = 0;

    while (i < device_index) {
        if (device == NULL) {
            return NULL;
        }
        device = device->next;
        i++;
    }

    return device;
}

static void
SDL_SYS_AddMFIJoystickDevice(SDL_JoystickDeviceItem *device, GCController *controller)
{
#ifdef SDL_JOYSTICK_MFI
    const char *name = NULL;
    /* Explicitly retain the controller because SDL_JoystickDeviceItem is a
     * struct, and ARC doesn't work with structs. */
    device->controller = (__bridge GCController *) CFBridgingRetain(controller);

    if (controller.vendorName) {
        name = controller.vendorName.UTF8String;
    }

    if (!name) {
        name = "MFi Gamepad";
    }

    device->name = SDL_strdup(name);

    device->guid.data[0] = 'M';
    device->guid.data[1] = 'F';
    device->guid.data[2] = 'i';
    device->guid.data[3] = 'G';
    device->guid.data[4] = 'a';
    device->guid.data[5] = 'm';
    device->guid.data[6] = 'e';
    device->guid.data[7] = 'p';
    device->guid.data[8] = 'a';
    device->guid.data[9] = 'd';

    if (controller.extendedGamepad) {
        device->guid.data[10] = 1;
    } else if (controller.gamepad) {
        device->guid.data[10] = 2;
    }
#if TARGET_OS_TV
    else if (controller.microGamepad) {
        device->guid.data[10] = 3;
        device->remote = SDL_TRUE;
    }
#endif /* TARGET_OS_TV */

    if (controller.extendedGamepad) {
        device->naxes = 6; /* 2 thumbsticks and 2 triggers */
        device->nhats = 1; /* d-pad */
        device->nbuttons = 7; /* ABXY, shoulder buttons, pause button */
    } else if (controller.gamepad) {
        device->naxes = 0; /* no traditional analog inputs */
        device->nhats = 1; /* d-pad */
        device->nbuttons = 7; /* ABXY, shoulder buttons, pause button */
    }
#if TARGET_OS_TV
    else if (controller.microGamepad) {
        device->naxes = 2; /* treat the touch surface as two axes */
        device->nhats = 0; /* apparently the touch surface-as-dpad is buggy */
        device->nbuttons = 3; /* AX, pause button */

        controller.microGamepad.allowsRotation = SDL_GetHintBoolean(SDL_HINT_APPLE_TV_REMOTE_ALLOW_ROTATION, SDL_FALSE);
    }
#endif /* TARGET_OS_TV */

    /* This will be set when the first button press of the controller is
     * detected. */
    controller.playerIndex = -1;

#endif /* SDL_JOYSTICK_MFI */
}

static void
SDL_SYS_AddJoystickDevice(GCController *controller, SDL_bool accelerometer)
{
    SDL_JoystickDeviceItem *device = deviceList;

#if TARGET_OS_TV
    if (!SDL_GetHintBoolean(SDL_HINT_TV_REMOTE_AS_JOYSTICK, SDL_TRUE)) {
        /* Ignore devices that aren't actually controllers (e.g. remotes), they'll be handled as keyboard input */
        if (controller && !controller.extendedGamepad && !controller.gamepad && controller.microGamepad) {
            return;
        }
    }
#endif

    while (device != NULL) {
        if (device->controller == controller) {
            return;
        }
        device = device->next;
    }

    device = (SDL_JoystickDeviceItem *) SDL_calloc(1, sizeof(SDL_JoystickDeviceItem));
    if (device == NULL) {
        return;
    }

    device->accelerometer = accelerometer;
    device->instance_id = instancecounter++;

    if (accelerometer) {
#if TARGET_OS_TV
        SDL_free(device);
        return;
#else
        device->name = SDL_strdup(accelerometerName);
        device->naxes = 3; /* Device acceleration in the x, y, and z axes. */
        device->nhats = 0;
        device->nbuttons = 0;

        /* Use the accelerometer name as a GUID. */
        SDL_memcpy(&device->guid.data, device->name, SDL_min(sizeof(SDL_JoystickGUID), SDL_strlen(device->name)));
#endif /* TARGET_OS_TV */
    } else if (controller) {
        SDL_SYS_AddMFIJoystickDevice(device, controller);
    }

    if (deviceList == NULL) {
        deviceList = device;
    } else {
        SDL_JoystickDeviceItem *lastdevice = deviceList;
        while (lastdevice->next != NULL) {
            lastdevice = lastdevice->next;
        }
        lastdevice->next = device;
    }

    ++numjoysticks;

    SDL_PrivateJoystickAdded(numjoysticks - 1);
}

static SDL_JoystickDeviceItem *
SDL_SYS_RemoveJoystickDevice(SDL_JoystickDeviceItem *device)
{
    SDL_JoystickDeviceItem *prev = NULL;
    SDL_JoystickDeviceItem *next = NULL;
    SDL_JoystickDeviceItem *item = deviceList;

    if (device == NULL) {
        return NULL;
    }

    next = device->next;

    while (item != NULL) {
        if (item == device) {
            break;
        }
        prev = item;
        item = item->next;
    }

    /* Unlink the device item from the device list. */
    if (prev) {
        prev->next = device->next;
    } else if (device == deviceList) {
        deviceList = device->next;
    }

    if (device->joystick) {
        device->joystick->hwdata = NULL;
    }

#ifdef SDL_JOYSTICK_MFI
    @autoreleasepool {
        if (device->controller) {
            /* The controller was explicitly retained in the struct, so it
             * should be explicitly released before freeing the struct. */
            GCController *controller = CFBridgingRelease((__bridge CFTypeRef)(device->controller));
            controller.controllerPausedHandler = nil;
            device->controller = nil;
        }
    }
#endif /* SDL_JOYSTICK_MFI */

    --numjoysticks;

    SDL_PrivateJoystickRemoved(device->instance_id);

    SDL_free(device->name);
    SDL_free(device);

    return next;
}

#if TARGET_OS_TV
static void SDLCALL
SDL_AppleTVRemoteRotationHintChanged(void *udata, const char *name, const char *oldValue, const char *newValue)
{
    BOOL allowRotation = newValue != NULL && *newValue != '0';

    @autoreleasepool {
        for (GCController *controller in [GCController controllers]) {
            if (controller.microGamepad) {
                controller.microGamepad.allowsRotation = allowRotation;
            }
        }
    }
}
#endif /* TARGET_OS_TV */

static SDL_bool SteamControllerConnectedCallback(const char *name, SDL_JoystickGUID guid, int *device_instance)
{
    SDL_JoystickDeviceItem *device = (SDL_JoystickDeviceItem *)SDL_calloc(1, sizeof(SDL_JoystickDeviceItem));
    if (device == NULL) {
        return SDL_FALSE;
    }

    *device_instance = device->instance_id = instancecounter++;
    device->name = SDL_strdup(name);
    device->guid = guid;
    SDL_GetSteamControllerInputs(&device->nbuttons,
                                 &device->naxes,
                                 &device->nhats);
    device->m_bSteamController = SDL_TRUE;

    if (deviceList == NULL) {
        deviceList = device;
    } else {
        SDL_JoystickDeviceItem *lastdevice = deviceList;
        while (lastdevice->next != NULL) {
            lastdevice = lastdevice->next;
        }
        lastdevice->next = device;
    }

    ++numjoysticks;

    SDL_PrivateJoystickAdded(numjoysticks - 1);

    return SDL_TRUE;
}

static void SteamControllerDisconnectedCallback(int device_instance)
{
    SDL_JoystickDeviceItem *item;

    for (item = deviceList; item; item = item->next) {
        if (item->instance_id == device_instance) {
            SDL_SYS_RemoveJoystickDevice(item);
            break;
        }
    }
}

/* Function to scan the system for joysticks.
 * Joystick 0 should be the system default joystick.
 * It should return 0, or -1 on an unrecoverable fatal error.
 */
int
SDL_SYS_JoystickInit(void)
{
    @autoreleasepool {
        NSNotificationCenter *center = [NSNotificationCenter defaultCenter];

        SDL_InitSteamControllers(SteamControllerConnectedCallback,
                                 SteamControllerDisconnectedCallback);

#if !TARGET_OS_TV
        if (SDL_GetHintBoolean(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, SDL_TRUE)) {
            /* Default behavior, accelerometer as joystick */
            SDL_SYS_AddJoystickDevice(nil, SDL_TRUE);
        }
#endif /* !TARGET_OS_TV */

#ifdef SDL_JOYSTICK_MFI
        /* GameController.framework was added in iOS 7. */
        if (![GCController class]) {
            return numjoysticks;
        }

        for (GCController *controller in [GCController controllers]) {
            SDL_SYS_AddJoystickDevice(controller, SDL_FALSE);
        }

#if TARGET_OS_TV
        SDL_AddHintCallback(SDL_HINT_APPLE_TV_REMOTE_ALLOW_ROTATION,
                            SDL_AppleTVRemoteRotationHintChanged, NULL);
#endif /* TARGET_OS_TV */

        connectObserver = [center addObserverForName:GCControllerDidConnectNotification
                                              object:nil
                                               queue:nil
                                          usingBlock:^(NSNotification *note) {
                                              GCController *controller = note.object;
                                              SDL_SYS_AddJoystickDevice(controller, SDL_FALSE);
                                          }];

        disconnectObserver = [center addObserverForName:GCControllerDidDisconnectNotification
                                                 object:nil
                                                  queue:nil
                                             usingBlock:^(NSNotification *note) {
                                                 GCController *controller = note.object;
                                                 SDL_JoystickDeviceItem *device = deviceList;
                                                 while (device != NULL) {
                                                     if (device->controller == controller) {
                                                         SDL_SYS_RemoveJoystickDevice(device);
                                                         break;
                                                     }
                                                     device = device->next;
                                                 }
                                             }];
#endif /* SDL_JOYSTICK_MFI */
    }

    return numjoysticks;
}

int
SDL_SYS_NumJoysticks(void)
{
    return numjoysticks;
}

void
SDL_SYS_JoystickDetect(void)
{
    SDL_UpdateSteamControllers();
}

/* Function to get the device-dependent name of a joystick */
const char *
SDL_SYS_JoystickNameForDeviceIndex(int device_index)
{
    SDL_JoystickDeviceItem *device = GetDeviceForIndex(device_index);
    return device ? device->name : "Unknown";
}

/* Function to perform the mapping from device index to the instance id for this index */
SDL_JoystickID SDL_SYS_GetInstanceIdOfDeviceIndex(int device_index)
{
    SDL_JoystickDeviceItem *device = GetDeviceForIndex(device_index);
    return device ? device->instance_id : 0;
}

/* Function to open a joystick for use.
   The joystick to open is specified by the device index.
   This should fill the nbuttons and naxes fields of the joystick structure.
   It returns 0, or -1 if there is an error.
 */
int
SDL_SYS_JoystickOpen(SDL_Joystick * joystick, int device_index)
{
    SDL_JoystickDeviceItem *device = GetDeviceForIndex(device_index);
    if (device == NULL) {
        return SDL_SetError("Could not open Joystick: no hardware device for the specified index");
    }

    joystick->hwdata = device;
    joystick->instance_id = device->instance_id;

    joystick->naxes = device->naxes;
    joystick->nhats = device->nhats;
    joystick->nbuttons = device->nbuttons;
    joystick->nballs = 0;

    device->joystick = joystick;

    @autoreleasepool {
        if (device->accelerometer) {
#if !TARGET_OS_TV
            if (motionManager == nil) {
                motionManager = [[CMMotionManager alloc] init];
            }

            /* Shorter times between updates can significantly increase CPU usage. */
            motionManager.accelerometerUpdateInterval = 0.1;
            [motionManager startAccelerometerUpdates];
#endif /* !TARGET_OS_TV */
        } else {
#ifdef SDL_JOYSTICK_MFI
            GCController *controller = device->controller;
            controller.controllerPausedHandler = ^(GCController *c) {
                if (joystick->hwdata) {
                    ++joystick->hwdata->num_pause_presses;
                }
            };
#endif /* SDL_JOYSTICK_MFI */
        }
    }
    if (device->remote) {
        ++SDL_AppleTVRemoteOpenedAsJoystick;
    }

    return 0;
}

/* Function to determine if this joystick is attached to the system right now */
SDL_bool
SDL_SYS_JoystickAttached(SDL_Joystick *joystick)
{
    return joystick->hwdata != NULL;
}

static void
SDL_SYS_AccelerometerUpdate(SDL_Joystick * joystick)
{
#if !TARGET_OS_TV
    const float maxgforce = SDL_IPHONE_MAX_GFORCE;
    const SInt16 maxsint16 = 0x7FFF;
    CMAcceleration accel;

    @autoreleasepool {
        if (!motionManager.isAccelerometerActive) {
            return;
        }

        accel = motionManager.accelerometerData.acceleration;
    }

    /*
     Convert accelerometer data from floating point to Sint16, which is what
     the joystick system expects.

     To do the conversion, the data is first clamped onto the interval
     [-SDL_IPHONE_MAX_G_FORCE, SDL_IPHONE_MAX_G_FORCE], then the data is multiplied
     by MAX_SINT16 so that it is mapped to the full range of an Sint16.

     You can customize the clamped range of this function by modifying the
     SDL_IPHONE_MAX_GFORCE macro in SDL_config_iphoneos.h.

     Once converted to Sint16, the accelerometer data no longer has coherent
     units. You can convert the data back to units of g-force by multiplying
     it in your application's code by SDL_IPHONE_MAX_GFORCE / 0x7FFF.
     */

    /* clamp the data */
    accel.x = SDL_min(SDL_max(accel.x, -maxgforce), maxgforce);
    accel.y = SDL_min(SDL_max(accel.y, -maxgforce), maxgforce);
    accel.z = SDL_min(SDL_max(accel.z, -maxgforce), maxgforce);

    /* pass in data mapped to range of SInt16 */
    SDL_PrivateJoystickAxis(joystick, 0,  (accel.x / maxgforce) * maxsint16);
    SDL_PrivateJoystickAxis(joystick, 1, -(accel.y / maxgforce) * maxsint16);
    SDL_PrivateJoystickAxis(joystick, 2,  (accel.z / maxgforce) * maxsint16);
#endif /* !TARGET_OS_TV */
}

#ifdef SDL_JOYSTICK_MFI
static Uint8
SDL_SYS_MFIJoystickHatStateForDPad(GCControllerDirectionPad *dpad)
{
    Uint8 hat = 0;

    if (dpad.up.isPressed) {
        hat |= SDL_HAT_UP;
    } else if (dpad.down.isPressed) {
        hat |= SDL_HAT_DOWN;
    }

    if (dpad.left.isPressed) {
        hat |= SDL_HAT_LEFT;
    } else if (dpad.right.isPressed) {
        hat |= SDL_HAT_RIGHT;
    }

    if (hat == 0) {
        return SDL_HAT_CENTERED;
    }

    return hat;
}
#endif

static void
SDL_SYS_MFIJoystickUpdate(SDL_Joystick * joystick)
{
#if SDL_JOYSTICK_MFI
    @autoreleasepool {
        GCController *controller = joystick->hwdata->controller;
        Uint8 hatstate = SDL_HAT_CENTERED;
        int i;
        int updateplayerindex = 0;

        if (controller.extendedGamepad) {
            GCExtendedGamepad *gamepad = controller.extendedGamepad;

            /* Axis order matches the XInput Windows mappings. */
            Sint16 axes[] = {
                (Sint16) (gamepad.leftThumbstick.xAxis.value * 32767),
                (Sint16) (gamepad.leftThumbstick.yAxis.value * -32767),
                (Sint16) ((gamepad.leftTrigger.value * 65535) - 32768),
                (Sint16) (gamepad.rightThumbstick.xAxis.value * 32767),
                (Sint16) (gamepad.rightThumbstick.yAxis.value * -32767),
                (Sint16) ((gamepad.rightTrigger.value * 65535) - 32768),
            };

            /* Button order matches the XInput Windows mappings. */
            Uint8 buttons[] = {
                gamepad.buttonA.isPressed, gamepad.buttonB.isPressed,
                gamepad.buttonX.isPressed, gamepad.buttonY.isPressed,
                gamepad.leftShoulder.isPressed,
                gamepad.rightShoulder.isPressed,
            };

            hatstate = SDL_SYS_MFIJoystickHatStateForDPad(gamepad.dpad);

            for (i = 0; i < SDL_arraysize(axes); i++) {
                /* The triggers (axes 2 and 5) are resting at -32768 but SDL
                 * initializes its values to 0. We only want to make sure the
                 * player index is up to date if the user actually moves an axis. */
                if ((i != 2 && i != 5) || axes[i] != -32768) {
                    updateplayerindex |= (joystick->axes[i].value != axes[i]);
                }
                SDL_PrivateJoystickAxis(joystick, i, axes[i]);
            }

            for (i = 0; i < SDL_arraysize(buttons); i++) {
                updateplayerindex |= (joystick->buttons[i] != buttons[i]);
                SDL_PrivateJoystickButton(joystick, i, buttons[i]);
            }
        } else if (controller.gamepad) {
            GCGamepad *gamepad = controller.gamepad;

            /* Button order matches the XInput Windows mappings. */
            Uint8 buttons[] = {
                gamepad.buttonA.isPressed, gamepad.buttonB.isPressed,
                gamepad.buttonX.isPressed, gamepad.buttonY.isPressed,
                gamepad.leftShoulder.isPressed,
                gamepad.rightShoulder.isPressed,
            };

            hatstate = SDL_SYS_MFIJoystickHatStateForDPad(gamepad.dpad);

            for (i = 0; i < SDL_arraysize(buttons); i++) {
                updateplayerindex |= (joystick->buttons[i] != buttons[i]);
                SDL_PrivateJoystickButton(joystick, i, buttons[i]);
            }
        }
#if TARGET_OS_TV
        else if (controller.microGamepad) {
            GCMicroGamepad *gamepad = controller.microGamepad;

            Sint16 axes[] = {
                (Sint16) (gamepad.dpad.xAxis.value * 32767),
                (Sint16) (gamepad.dpad.yAxis.value * -32767),
            };

            for (i = 0; i < SDL_arraysize(axes); i++) {
                updateplayerindex |= (joystick->axes[i].value != axes[i]);
                SDL_PrivateJoystickAxis(joystick, i, axes[i]);
            }

            Uint8 buttons[] = {
                gamepad.buttonA.isPressed,
                gamepad.buttonX.isPressed,
            };

            for (i = 0; i < SDL_arraysize(buttons); i++) {
                updateplayerindex |= (joystick->buttons[i] != buttons[i]);
                SDL_PrivateJoystickButton(joystick, i, buttons[i]);
            }
        }
#endif /* TARGET_OS_TV */

        if (joystick->nhats > 0) {
            updateplayerindex |= (joystick->hats[0] != hatstate);
            SDL_PrivateJoystickHat(joystick, 0, hatstate);
        }

        for (i = 0; i < joystick->hwdata->num_pause_presses; i++) {
            /* The pause button is always last. */
            Uint8 pausebutton = joystick->nbuttons - 1;

            SDL_PrivateJoystickButton(joystick, pausebutton, SDL_PRESSED);
            SDL_PrivateJoystickButton(joystick, pausebutton, SDL_RELEASED);

            updateplayerindex = YES;
        }

        joystick->hwdata->num_pause_presses = 0;

        if (updateplayerindex && controller.playerIndex == -1) {
            BOOL usedPlayerIndexSlots[4] = {NO, NO, NO, NO};

            /* Find the player index of all other connected controllers. */
            for (GCController *c in [GCController controllers]) {
                if (c != controller && c.playerIndex >= 0) {
                    usedPlayerIndexSlots[c.playerIndex] = YES;
                }
            }

            /* Set this controller's player index to the first unused index.
             * FIXME: This logic isn't great... but SDL doesn't expose this
             * concept in its external API, so we don't have much to go on. */
            for (i = 0; i < SDL_arraysize(usedPlayerIndexSlots); i++) {
                if (!usedPlayerIndexSlots[i]) {
                    controller.playerIndex = i;
                    break;
                }
            }
        }
    }
#endif /* SDL_JOYSTICK_MFI */
}

/* Function to update the state of a joystick - called as a device poll.
 * This function shouldn't update the joystick structure directly,
 * but instead should call SDL_PrivateJoystick*() to deliver events
 * and update joystick device state.
 */
void
SDL_SYS_JoystickUpdate(SDL_Joystick * joystick)
{
    SDL_JoystickDeviceItem *device = joystick->hwdata;

    if (device == NULL) {
        return;
    }
    
    if (device->m_bSteamController) {
        SDL_UpdateSteamController(joystick);
        return;
    }

    if (device->accelerometer) {
        SDL_SYS_AccelerometerUpdate(joystick);
    } else if (device->controller) {
        SDL_SYS_MFIJoystickUpdate(joystick);
    }
}

/* Function to close a joystick after use */
void
SDL_SYS_JoystickClose(SDL_Joystick * joystick)
{
    SDL_JoystickDeviceItem *device = joystick->hwdata;

    if (device == NULL) {
        return;
    }

    device->joystick = NULL;

    @autoreleasepool {
        if (device->accelerometer) {
#if !TARGET_OS_TV
            [motionManager stopAccelerometerUpdates];
#endif /* !TARGET_OS_TV */
        } else if (device->controller) {
#ifdef SDL_JOYSTICK_MFI
            GCController *controller = device->controller;
            controller.controllerPausedHandler = nil;
            controller.playerIndex = -1;
#endif
        }
    }
    if (device->remote) {
        --SDL_AppleTVRemoteOpenedAsJoystick;
    }
}

/* Function to perform any system-specific joystick related cleanup */
void
SDL_SYS_JoystickQuit(void)
{
    @autoreleasepool {
#ifdef SDL_JOYSTICK_MFI
        NSNotificationCenter *center = [NSNotificationCenter defaultCenter];

        if (connectObserver) {
            [center removeObserver:connectObserver name:GCControllerDidConnectNotification object:nil];
            connectObserver = nil;
        }

        if (disconnectObserver) {
            [center removeObserver:disconnectObserver name:GCControllerDidDisconnectNotification object:nil];
            disconnectObserver = nil;
        }

#if TARGET_OS_TV
        SDL_DelHintCallback(SDL_HINT_APPLE_TV_REMOTE_ALLOW_ROTATION,
                            SDL_AppleTVRemoteRotationHintChanged, NULL);
#endif /* TARGET_OS_TV */
#endif /* SDL_JOYSTICK_MFI */

        while (deviceList != NULL) {
            SDL_SYS_RemoveJoystickDevice(deviceList);
        }

#if !TARGET_OS_TV
        motionManager = nil;
#endif /* !TARGET_OS_TV */
    }

    SDL_QuitSteamControllers();

    numjoysticks = 0;
}

SDL_JoystickGUID
SDL_SYS_JoystickGetDeviceGUID( int device_index )
{
    SDL_JoystickDeviceItem *device = GetDeviceForIndex(device_index);
    SDL_JoystickGUID guid;
    if (device) {
        guid = device->guid;
    } else {
        SDL_zero(guid);
    }
    return guid;
}

SDL_JoystickGUID
SDL_SYS_JoystickGetGUID(SDL_Joystick * joystick)
{
    SDL_JoystickGUID guid;
    if (joystick->hwdata) {
        guid = joystick->hwdata->guid;
    } else {
        SDL_zero(guid);
    }
    return guid;
}

/* vi: set ts=4 sw=4 expandtab: */
