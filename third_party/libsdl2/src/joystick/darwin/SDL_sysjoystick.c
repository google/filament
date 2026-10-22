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

#ifdef SDL_JOYSTICK_IOKIT

#include <IOKit/hid/IOHIDLib.h>

/* For force feedback testing. */
#include <ForceFeedback/ForceFeedback.h>
#include <ForceFeedback/ForceFeedbackConstants.h>

#include "SDL_joystick.h"
#include "../SDL_sysjoystick.h"
#include "../SDL_joystick_c.h"
#include "SDL_sysjoystick_c.h"
#include "SDL_events.h"
#include "../../haptic/darwin/SDL_syshaptic_c.h"    /* For haptic hot plugging */

#define SDL_JOYSTICK_RUNLOOP_MODE CFSTR("SDLJoystick")

/* The base object of the HID Manager API */
static IOHIDManagerRef hidman = NULL;

/* Linked list of all available devices */
static recDevice *gpDeviceList = NULL;

/* static incrementing counter for new joystick devices seen on the system. Devices should start with index 0 */
static int s_joystick_instance_id = -1;

static recDevice *GetDeviceForIndex(int device_index)
{
    recDevice *device = gpDeviceList;
    while (device) {
        if (!device->removed) {
            if (device_index == 0)
                break;

            --device_index;
        }
        device = device->pNext;
    }
    return device;
}

static void
FreeElementList(recElement *pElement)
{
    while (pElement) {
        recElement *pElementNext = pElement->pNext;
        SDL_free(pElement);
        pElement = pElementNext;
    }
}

static recDevice *
FreeDevice(recDevice *removeDevice)
{
    recDevice *pDeviceNext = NULL;
    if (removeDevice) {
        if (removeDevice->deviceRef) {
            IOHIDDeviceUnscheduleFromRunLoop(removeDevice->deviceRef, CFRunLoopGetCurrent(), SDL_JOYSTICK_RUNLOOP_MODE);
            removeDevice->deviceRef = NULL;
        }

        /* save next device prior to disposing of this device */
        pDeviceNext = removeDevice->pNext;

        if ( gpDeviceList == removeDevice ) {
            gpDeviceList = pDeviceNext;
        } else {
            recDevice *device = gpDeviceList;
            while (device->pNext != removeDevice) {
                device = device->pNext;
            }
            device->pNext = pDeviceNext;
        }
        removeDevice->pNext = NULL;

        /* free element lists */
        FreeElementList(removeDevice->firstAxis);
        FreeElementList(removeDevice->firstButton);
        FreeElementList(removeDevice->firstHat);

        SDL_free(removeDevice);
    }
    return pDeviceNext;
}

static SDL_bool
GetHIDElementState(recDevice *pDevice, recElement *pElement, SInt32 *pValue)
{
    SInt32 value = 0;
    int returnValue = SDL_FALSE;

    if (pDevice && pElement) {
        IOHIDValueRef valueRef;
        if (IOHIDDeviceGetValue(pDevice->deviceRef, pElement->elementRef, &valueRef) == kIOReturnSuccess) {
            value = (SInt32) IOHIDValueGetIntegerValue(valueRef);

            /* record min and max for auto calibration */
            if (value < pElement->minReport) {
                pElement->minReport = value;
            }
            if (value > pElement->maxReport) {
                pElement->maxReport = value;
            }
            *pValue = value;

            returnValue = SDL_TRUE;
        }
    }
    return returnValue;
}

static SDL_bool
GetHIDScaledCalibratedState(recDevice * pDevice, recElement * pElement, SInt32 min, SInt32 max, SInt32 *pValue)
{
    const float deviceScale = max - min;
    const float readScale = pElement->maxReport - pElement->minReport;
    int returnValue = SDL_FALSE;
    if (GetHIDElementState(pDevice, pElement, pValue))
    {
        if (readScale == 0) {
            returnValue = SDL_TRUE;           /* no scaling at all */
        }
        else
        {
            *pValue = ((*pValue - pElement->minReport) * deviceScale / readScale) + min;
            returnValue = SDL_TRUE;
        }
    } 
    return returnValue;
}

static void
JoystickDeviceWasRemovedCallback(void *ctx, IOReturn result, void *sender)
{
    recDevice *device = (recDevice *) ctx;
    device->removed = SDL_TRUE;
    device->deviceRef = NULL; // deviceRef was invalidated due to the remove
#if SDL_HAPTIC_IOKIT
    MacHaptic_MaybeRemoveDevice(device->ffservice);
#endif

    SDL_PrivateJoystickRemoved(device->instance_id);
}


static void AddHIDElement(const void *value, void *parameter);

/* Call AddHIDElement() on all elements in an array of IOHIDElementRefs */
static void
AddHIDElements(CFArrayRef array, recDevice *pDevice)
{
    const CFRange range = { 0, CFArrayGetCount(array) };
    CFArrayApplyFunction(array, range, AddHIDElement, pDevice);
}

static SDL_bool
ElementAlreadyAdded(const IOHIDElementCookie cookie, const recElement *listitem) {
    while (listitem) {
        if (listitem->cookie == cookie) {
            return SDL_TRUE;
        }
        listitem = listitem->pNext;
    }
    return SDL_FALSE;
}

/* See if we care about this HID element, and if so, note it in our recDevice. */
static void
AddHIDElement(const void *value, void *parameter)
{
    recDevice *pDevice = (recDevice *) parameter;
    IOHIDElementRef refElement = (IOHIDElementRef) value;
    const CFTypeID elementTypeID = refElement ? CFGetTypeID(refElement) : 0;

    if (refElement && (elementTypeID == IOHIDElementGetTypeID())) {
        const IOHIDElementCookie cookie = IOHIDElementGetCookie(refElement);
        const uint32_t usagePage = IOHIDElementGetUsagePage(refElement);
        const uint32_t usage = IOHIDElementGetUsage(refElement);
        recElement *element = NULL;
        recElement **headElement = NULL;

        /* look at types of interest */
        switch (IOHIDElementGetType(refElement)) {
            case kIOHIDElementTypeInput_Misc:
            case kIOHIDElementTypeInput_Button:
            case kIOHIDElementTypeInput_Axis: {
                switch (usagePage) {    /* only interested in kHIDPage_GenericDesktop and kHIDPage_Button */
                    case kHIDPage_GenericDesktop:
                        switch (usage) {
                            case kHIDUsage_GD_X:
                            case kHIDUsage_GD_Y:
                            case kHIDUsage_GD_Z:
                            case kHIDUsage_GD_Rx:
                            case kHIDUsage_GD_Ry:
                            case kHIDUsage_GD_Rz:
                            case kHIDUsage_GD_Slider:
                            case kHIDUsage_GD_Dial:
                            case kHIDUsage_GD_Wheel:
                                if (!ElementAlreadyAdded(cookie, pDevice->firstAxis)) {
                                    element = (recElement *) SDL_calloc(1, sizeof (recElement));
                                    if (element) {
                                        pDevice->axes++;
                                        headElement = &(pDevice->firstAxis);
                                    }
                                }
                                break;

                            case kHIDUsage_GD_Hatswitch:
                                if (!ElementAlreadyAdded(cookie, pDevice->firstHat)) {
                                    element = (recElement *) SDL_calloc(1, sizeof (recElement));
                                    if (element) {
                                        pDevice->hats++;
                                        headElement = &(pDevice->firstHat);
                                    }
                                }
                                break;
                            case kHIDUsage_GD_DPadUp:
                            case kHIDUsage_GD_DPadDown:
                            case kHIDUsage_GD_DPadRight:
                            case kHIDUsage_GD_DPadLeft:
                            case kHIDUsage_GD_Start:
                            case kHIDUsage_GD_Select:
                            case kHIDUsage_GD_SystemMainMenu:
                                if (!ElementAlreadyAdded(cookie, pDevice->firstButton)) {
                                    element = (recElement *) SDL_calloc(1, sizeof (recElement));
                                    if (element) {
                                        pDevice->buttons++;
                                        headElement = &(pDevice->firstButton);
                                    }
                                }
                                break;
                        }
                        break;

                    case kHIDPage_Simulation:
                        switch (usage) {
                            case kHIDUsage_Sim_Rudder:
                            case kHIDUsage_Sim_Throttle:
                            case kHIDUsage_Sim_Accelerator:
                            case kHIDUsage_Sim_Brake:
                                if (!ElementAlreadyAdded(cookie, pDevice->firstAxis)) {
                                    element = (recElement *) SDL_calloc(1, sizeof (recElement));
                                    if (element) {
                                        pDevice->axes++;
                                        headElement = &(pDevice->firstAxis);
                                    }
                                }
                                break;

                            default:
                                break;
                        }
                        break;

                    case kHIDPage_Button:
                    case kHIDPage_Consumer: /* e.g. 'pause' button on Steelseries MFi gamepads. */
                        if (!ElementAlreadyAdded(cookie, pDevice->firstButton)) {
                            element = (recElement *) SDL_calloc(1, sizeof (recElement));
                            if (element) {
                                pDevice->buttons++;
                                headElement = &(pDevice->firstButton);
                            }
                        }
                        break;

                    default:
                        break;
                }
            }
            break;

            case kIOHIDElementTypeCollection: {
                CFArrayRef array = IOHIDElementGetChildren(refElement);
                if (array) {
                    AddHIDElements(array, pDevice);
                }
            }
            break;

            default:
                break;
        }

        if (element && headElement) {       /* add to list */
            recElement *elementPrevious = NULL;
            recElement *elementCurrent = *headElement;
            while (elementCurrent && usage >= elementCurrent->usage) {
                elementPrevious = elementCurrent;
                elementCurrent = elementCurrent->pNext;
            }
            if (elementPrevious) {
                elementPrevious->pNext = element;
            } else {
                *headElement = element;
            }

            element->elementRef = refElement;
            element->usagePage = usagePage;
            element->usage = usage;
            element->pNext = elementCurrent;

            element->minReport = element->min = (SInt32) IOHIDElementGetLogicalMin(refElement);
            element->maxReport = element->max = (SInt32) IOHIDElementGetLogicalMax(refElement);
            element->cookie = IOHIDElementGetCookie(refElement);

            pDevice->elements++;
        }
    }
}

static SDL_bool
GetDeviceInfo(IOHIDDeviceRef hidDevice, recDevice *pDevice)
{
    const Uint16 BUS_USB = 0x03;
    const Uint16 BUS_BLUETOOTH = 0x05;
    Sint32 vendor = 0;
    Sint32 product = 0;
    Sint32 version = 0;
    CFTypeRef refCF = NULL;
    CFArrayRef array = NULL;
    Uint16 *guid16 = (Uint16 *)pDevice->guid.data;

    /* get usage page and usage */
    refCF = IOHIDDeviceGetProperty(hidDevice, CFSTR(kIOHIDPrimaryUsagePageKey));
    if (refCF) {
        CFNumberGetValue(refCF, kCFNumberSInt32Type, &pDevice->usagePage);
    }
    if (pDevice->usagePage != kHIDPage_GenericDesktop) {
        return SDL_FALSE; /* Filter device list to non-keyboard/mouse stuff */
    }

    refCF = IOHIDDeviceGetProperty(hidDevice, CFSTR(kIOHIDPrimaryUsageKey));
    if (refCF) {
        CFNumberGetValue(refCF, kCFNumberSInt32Type, &pDevice->usage);
    }

    if ((pDevice->usage != kHIDUsage_GD_Joystick &&
         pDevice->usage != kHIDUsage_GD_GamePad &&
         pDevice->usage != kHIDUsage_GD_MultiAxisController)) {
        return SDL_FALSE; /* Filter device list to non-keyboard/mouse stuff */
    }

    pDevice->deviceRef = hidDevice;

    /* get device name */
    refCF = IOHIDDeviceGetProperty(hidDevice, CFSTR(kIOHIDProductKey));
    if (!refCF) {
        /* Maybe we can't get "AwesomeJoystick2000", but we can get "Logitech"? */
        refCF = IOHIDDeviceGetProperty(hidDevice, CFSTR(kIOHIDManufacturerKey));
    }
    if ((!refCF) || (!CFStringGetCString(refCF, pDevice->product, sizeof (pDevice->product), kCFStringEncodingUTF8))) {
        SDL_strlcpy(pDevice->product, "Unidentified joystick", sizeof (pDevice->product));
    }

    refCF = IOHIDDeviceGetProperty(hidDevice, CFSTR(kIOHIDVendorIDKey));
    if (refCF) {
        CFNumberGetValue(refCF, kCFNumberSInt32Type, &vendor);
    }

    refCF = IOHIDDeviceGetProperty(hidDevice, CFSTR(kIOHIDProductIDKey));
    if (refCF) {
        CFNumberGetValue(refCF, kCFNumberSInt32Type, &product);
    }

    refCF = IOHIDDeviceGetProperty(hidDevice, CFSTR(kIOHIDVersionNumberKey));
    if (refCF) {
        CFNumberGetValue(refCF, kCFNumberSInt32Type, &version);
    }

    SDL_memset(pDevice->guid.data, 0, sizeof(pDevice->guid.data));

    if (vendor && product) {
        *guid16++ = SDL_SwapLE16(BUS_USB);
        *guid16++ = 0;
        *guid16++ = SDL_SwapLE16((Uint16)vendor);
        *guid16++ = 0;
        *guid16++ = SDL_SwapLE16((Uint16)product);
        *guid16++ = 0;
        *guid16++ = SDL_SwapLE16((Uint16)version);
        *guid16++ = 0;
    } else {
        *guid16++ = SDL_SwapLE16(BUS_BLUETOOTH);
        *guid16++ = 0;
        SDL_strlcpy((char*)guid16, pDevice->product, sizeof(pDevice->guid.data) - 4);
    }

    array = IOHIDDeviceCopyMatchingElements(hidDevice, NULL, kIOHIDOptionsTypeNone);
    if (array) {
        AddHIDElements(array, pDevice);
        CFRelease(array);
    }

    return SDL_TRUE;
}

static SDL_bool
JoystickAlreadyKnown(IOHIDDeviceRef ioHIDDeviceObject)
{
    recDevice *i;
    for (i = gpDeviceList; i != NULL; i = i->pNext) {
        if (i->deviceRef == ioHIDDeviceObject) {
            return SDL_TRUE;
        }
    }
    return SDL_FALSE;
}


static void
JoystickDeviceWasAddedCallback(void *ctx, IOReturn res, void *sender, IOHIDDeviceRef ioHIDDeviceObject)
{
    recDevice *device;
    int device_index = 0;
    io_service_t ioservice;

    if (res != kIOReturnSuccess) {
        return;
    }

    if (JoystickAlreadyKnown(ioHIDDeviceObject)) {
        return;  /* IOKit sent us a duplicate. */
    }

    device = (recDevice *) SDL_calloc(1, sizeof(recDevice));

    if (!device) {
        SDL_OutOfMemory();
        return;
    }

    if (!GetDeviceInfo(ioHIDDeviceObject, device)) {
        SDL_free(device);
        return;   /* not a device we care about, probably. */
    }

    if (SDL_IsGameControllerNameAndGUID(device->product, device->guid) &&
        SDL_ShouldIgnoreGameController(device->product, device->guid)) {
        SDL_free(device);
        return;
    }

    /* Get notified when this device is disconnected. */
    IOHIDDeviceRegisterRemovalCallback(ioHIDDeviceObject, JoystickDeviceWasRemovedCallback, device);
    IOHIDDeviceScheduleWithRunLoop(ioHIDDeviceObject, CFRunLoopGetCurrent(), SDL_JOYSTICK_RUNLOOP_MODE);

    /* Allocate an instance ID for this device */
    device->instance_id = ++s_joystick_instance_id;

    /* We have to do some storage of the io_service_t for SDL_HapticOpenFromJoystick */
    ioservice = IOHIDDeviceGetService(ioHIDDeviceObject);
#if SDL_HAPTIC_IOKIT
    if ((ioservice) && (FFIsForceFeedback(ioservice) == FF_OK)) {
        device->ffservice = ioservice;
        MacHaptic_MaybeAddDevice(ioservice);
    }
#endif

    /* Add device to the end of the list */
    if ( !gpDeviceList ) {
        gpDeviceList = device;
    } else {
        recDevice *curdevice;

        curdevice = gpDeviceList;
        while ( curdevice->pNext ) {
            ++device_index;
            curdevice = curdevice->pNext;
        }
        curdevice->pNext = device;
        ++device_index;  /* bump by one since we counted by pNext. */
    }

    SDL_PrivateJoystickAdded(device_index);
}

static SDL_bool
ConfigHIDManager(CFArrayRef matchingArray)
{
    CFRunLoopRef runloop = CFRunLoopGetCurrent();

    if (IOHIDManagerOpen(hidman, kIOHIDOptionsTypeNone) != kIOReturnSuccess) {
        return SDL_FALSE;
    }

    IOHIDManagerSetDeviceMatchingMultiple(hidman, matchingArray);
    IOHIDManagerRegisterDeviceMatchingCallback(hidman, JoystickDeviceWasAddedCallback, NULL);
    IOHIDManagerScheduleWithRunLoop(hidman, runloop, SDL_JOYSTICK_RUNLOOP_MODE);

    while (CFRunLoopRunInMode(SDL_JOYSTICK_RUNLOOP_MODE,0,TRUE) == kCFRunLoopRunHandledSource) {
        /* no-op. Callback fires once per existing device. */
    }

    /* future hotplug events will come through SDL_JOYSTICK_RUNLOOP_MODE now. */

    return SDL_TRUE;  /* good to go. */
}


static CFDictionaryRef
CreateHIDDeviceMatchDictionary(const UInt32 page, const UInt32 usage, int *okay)
{
    CFDictionaryRef retval = NULL;
    CFNumberRef pageNumRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &page);
    CFNumberRef usageNumRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage);
    const void *keys[2] = { (void *) CFSTR(kIOHIDDeviceUsagePageKey), (void *) CFSTR(kIOHIDDeviceUsageKey) };
    const void *vals[2] = { (void *) pageNumRef, (void *) usageNumRef };

    if (pageNumRef && usageNumRef) {
        retval = CFDictionaryCreate(kCFAllocatorDefault, keys, vals, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    }

    if (pageNumRef) {
        CFRelease(pageNumRef);
    }
    if (usageNumRef) {
        CFRelease(usageNumRef);
    }

    if (!retval) {
        *okay = 0;
    }

    return retval;
}

static SDL_bool
CreateHIDManager(void)
{
    SDL_bool retval = SDL_FALSE;
    int okay = 1;
    const void *vals[] = {
        (void *) CreateHIDDeviceMatchDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick, &okay),
        (void *) CreateHIDDeviceMatchDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad, &okay),
        (void *) CreateHIDDeviceMatchDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_MultiAxisController, &okay),
    };
    const size_t numElements = SDL_arraysize(vals);
    CFArrayRef array = okay ? CFArrayCreate(kCFAllocatorDefault, vals, numElements, &kCFTypeArrayCallBacks) : NULL;
    size_t i;

    for (i = 0; i < numElements; i++) {
        if (vals[i]) {
            CFRelease((CFTypeRef) vals[i]);
        }
    }

    if (array) {
        hidman = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
        if (hidman != NULL) {
            retval = ConfigHIDManager(array);
        }
        CFRelease(array);
    }

    return retval;
}


/* Function to scan the system for joysticks.
 * Joystick 0 should be the system default joystick.
 * This function should return the number of available joysticks, or -1
 * on an unrecoverable fatal error.
 */
int
SDL_SYS_JoystickInit(void)
{
    if (gpDeviceList) {
        return SDL_SetError("Joystick: Device list already inited.");
    }

    if (!CreateHIDManager()) {
        return SDL_SetError("Joystick: Couldn't initialize HID Manager");
    }

    return SDL_SYS_NumJoysticks();
}

/* Function to return the number of joystick devices plugged in right now */
int
SDL_SYS_NumJoysticks(void)
{
    recDevice *device = gpDeviceList;
    int nJoySticks = 0;

    while (device) {
        if (!device->removed) {
            nJoySticks++;
        }
        device = device->pNext;
    }

    return nJoySticks;
}

/* Function to cause any queued joystick insertions to be processed
 */
void
SDL_SYS_JoystickDetect(void)
{
    recDevice *device = gpDeviceList;
    while (device) {
        if (device->removed) {
            device = FreeDevice(device);
        } else {
            device = device->pNext;
        }
    }

	/* run this after the checks above so we don't set device->removed and delete the device before
	   SDL_SYS_JoystickUpdate can run to clean up the SDL_Joystick object that owns this device */
	while (CFRunLoopRunInMode(SDL_JOYSTICK_RUNLOOP_MODE,0,TRUE) == kCFRunLoopRunHandledSource) {
		/* no-op. Pending callbacks will fire in CFRunLoopRunInMode(). */
	}
}

/* Function to get the device-dependent name of a joystick */
const char *
SDL_SYS_JoystickNameForDeviceIndex(int device_index)
{
    recDevice *device = GetDeviceForIndex(device_index);
    return device ? device->product : "UNKNOWN";
}

/* Function to return the instance id of the joystick at device_index
 */
SDL_JoystickID
SDL_SYS_GetInstanceIdOfDeviceIndex(int device_index)
{
    recDevice *device = GetDeviceForIndex(device_index);
    return device ? device->instance_id : 0;
}

/* Function to open a joystick for use.
 * The joystick to open is specified by the device index.
 * This should fill the nbuttons and naxes fields of the joystick structure.
 * It returns 0, or -1 if there is an error.
 */
int
SDL_SYS_JoystickOpen(SDL_Joystick * joystick, int device_index)
{
    recDevice *device = GetDeviceForIndex(device_index);

    joystick->instance_id = device->instance_id;
    joystick->hwdata = device;
    joystick->name = device->product;

    joystick->naxes = device->axes;
    joystick->nhats = device->hats;
    joystick->nballs = 0;
    joystick->nbuttons = device->buttons;
    return 0;
}

/* Function to query if the joystick is currently attached
 * It returns SDL_TRUE if attached, SDL_FALSE otherwise.
 */
SDL_bool
SDL_SYS_JoystickAttached(SDL_Joystick * joystick)
{
    return joystick->hwdata != NULL;
}

/* Function to update the state of a joystick - called as a device poll.
 * This function shouldn't update the joystick structure directly,
 * but instead should call SDL_PrivateJoystick*() to deliver events
 * and update joystick device state.
 */
void
SDL_SYS_JoystickUpdate(SDL_Joystick * joystick)
{
    recDevice *device = joystick->hwdata;
    recElement *element;
    SInt32 value, range;
    int i;

    if (!device) {
        return;
    }

    if (device->removed) {      /* device was unplugged; ignore it. */
        if (joystick->hwdata) {
            joystick->force_recentering = SDL_TRUE;
            joystick->hwdata = NULL;
        }
        return;
    }

    element = device->firstAxis;
    i = 0;

    int goodRead = SDL_FALSE;
    while (element) {
        goodRead = GetHIDScaledCalibratedState(device, element, -32768, 32767, &value);
        if (goodRead) {
            SDL_PrivateJoystickAxis(joystick, i, value);
        }

        element = element->pNext;
        ++i;
    }

    element = device->firstButton;
    i = 0;
    while (element) {
        goodRead = GetHIDElementState(device, element, &value);
        if (goodRead) {
            if (value > 1) {          /* handle pressure-sensitive buttons */
                value = 1;
            }
            SDL_PrivateJoystickButton(joystick, i, value);
        }

        element = element->pNext;
        ++i;
    }

    element = device->firstHat;
    i = 0;
    
    while (element) {
        Uint8 pos = 0;

        range = (element->max - element->min + 1);
        goodRead = GetHIDElementState(device, element, &value);
        if (goodRead) {
            value -= element->min;
            if (range == 4) {         /* 4 position hatswitch - scale up value */
                value *= 2;
            } else if (range != 8) {    /* Neither a 4 nor 8 positions - fall back to default position (centered) */
                value = -1;
            }
            switch (value) {
            case 0:
                pos = SDL_HAT_UP;
                break;
            case 1:
                pos = SDL_HAT_RIGHTUP;
                break;
            case 2:
                pos = SDL_HAT_RIGHT;
                break;
            case 3:
                pos = SDL_HAT_RIGHTDOWN;
                break;
            case 4:
                pos = SDL_HAT_DOWN;
                break;
            case 5:
                pos = SDL_HAT_LEFTDOWN;
                break;
            case 6:
                pos = SDL_HAT_LEFT;
                break;
            case 7:
                pos = SDL_HAT_LEFTUP;
                break;
            default:
                /* Every other value is mapped to center. We do that because some
                 * joysticks use 8 and some 15 for this value, and apparently
                 * there are even more variants out there - so we try to be generous.
                 */
                pos = SDL_HAT_CENTERED;
                break;
            }

            SDL_PrivateJoystickHat(joystick, i, pos);
        }
        
        element = element->pNext;
        ++i;
    }
}

/* Function to close a joystick after use */
void
SDL_SYS_JoystickClose(SDL_Joystick * joystick)
{
}

/* Function to perform any system-specific joystick related cleanup */
void
SDL_SYS_JoystickQuit(void)
{
    while (FreeDevice(gpDeviceList)) {
        /* spin */
    }

    if (hidman) {
        IOHIDManagerUnscheduleFromRunLoop(hidman, CFRunLoopGetCurrent(), SDL_JOYSTICK_RUNLOOP_MODE);
        IOHIDManagerClose(hidman, kIOHIDOptionsTypeNone);
        CFRelease(hidman);
        hidman = NULL;
    }
}


SDL_JoystickGUID SDL_SYS_JoystickGetDeviceGUID( int device_index )
{
    recDevice *device = GetDeviceForIndex(device_index);
    SDL_JoystickGUID guid;
    if (device) {
        guid = device->guid;
    } else {
        SDL_zero(guid);
    }
    return guid;
}

SDL_JoystickGUID SDL_SYS_JoystickGetGUID(SDL_Joystick *joystick)
{
    return joystick->hwdata->guid;
}

#endif /* SDL_JOYSTICK_IOKIT */

/* vi: set ts=4 sw=4 expandtab: */
