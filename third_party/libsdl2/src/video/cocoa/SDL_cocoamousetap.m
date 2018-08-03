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

#if SDL_VIDEO_DRIVER_COCOA

#include "SDL_cocoamousetap.h"

/* Event taps are forbidden in the Mac App Store, so we can only enable this
 * code if your app doesn't need to ship through the app store.
 * This code makes it so that a grabbed cursor cannot "leak" a mouse click
 * past the edge of the window if moving the cursor too fast.
 */
#if SDL_MAC_NO_SANDBOX

#include "SDL_keyboard.h"
#include "SDL_cocoavideo.h"
#include "../../thread/SDL_systhread.h"

#include "../../events/SDL_mouse_c.h"

typedef struct {
    CFMachPortRef tap;
    CFRunLoopRef runloop;
    CFRunLoopSourceRef runloopSource;
    SDL_Thread *thread;
    SDL_sem *runloopStartedSemaphore;
} SDL_MouseEventTapData;

static const CGEventMask movementEventsMask =
      CGEventMaskBit(kCGEventLeftMouseDragged)
    | CGEventMaskBit(kCGEventRightMouseDragged)
    | CGEventMaskBit(kCGEventMouseMoved);

static const CGEventMask allGrabbedEventsMask =
      CGEventMaskBit(kCGEventLeftMouseDown)    | CGEventMaskBit(kCGEventLeftMouseUp)
    | CGEventMaskBit(kCGEventRightMouseDown)   | CGEventMaskBit(kCGEventRightMouseUp)
    | CGEventMaskBit(kCGEventOtherMouseDown)   | CGEventMaskBit(kCGEventOtherMouseUp)
    | CGEventMaskBit(kCGEventLeftMouseDragged) | CGEventMaskBit(kCGEventRightMouseDragged)
    | CGEventMaskBit(kCGEventMouseMoved);

static CGEventRef
Cocoa_MouseTapCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon)
{
    SDL_MouseEventTapData *tapdata = (SDL_MouseEventTapData*)refcon;
    SDL_Mouse *mouse = SDL_GetMouse();
    SDL_Window *window = SDL_GetKeyboardFocus();
    NSWindow *nswindow;
    NSRect windowRect;
    CGPoint eventLocation;

    switch (type) {
        case kCGEventTapDisabledByTimeout:
            {
                CGEventTapEnable(tapdata->tap, true);
                return NULL;
            }
        case kCGEventTapDisabledByUserInput:
            {
                return NULL;
            }
        default:
            break;
    }


    if (!window || !mouse) {
        return event;
    }

    if (mouse->relative_mode) {
        return event;
    }

    if (!(window->flags & SDL_WINDOW_INPUT_GRABBED)) {
        return event;
    }

    /* This is the same coordinate system as Cocoa uses. */
    nswindow = ((SDL_WindowData *) window->driverdata)->nswindow;
    eventLocation = CGEventGetUnflippedLocation(event);
    windowRect = [nswindow contentRectForFrameRect:[nswindow frame]];

    if (!NSMouseInRect(NSPointFromCGPoint(eventLocation), windowRect, NO)) {

        /* This is in CGs global screenspace coordinate system, which has a
         * flipped Y.
         */
        CGPoint newLocation = CGEventGetLocation(event);

        if (eventLocation.x < NSMinX(windowRect)) {
            newLocation.x = NSMinX(windowRect);
        } else if (eventLocation.x >= NSMaxX(windowRect)) {
            newLocation.x = NSMaxX(windowRect) - 1.0;
        }

        if (eventLocation.y <= NSMinY(windowRect)) {
            newLocation.y -= (NSMinY(windowRect) - eventLocation.y + 1);
        } else if (eventLocation.y > NSMaxY(windowRect)) {
            newLocation.y += (eventLocation.y - NSMaxY(windowRect));
        }

        CGWarpMouseCursorPosition(newLocation);
        CGAssociateMouseAndMouseCursorPosition(YES);

        if ((CGEventMaskBit(type) & movementEventsMask) == 0) {
            /* For click events, we just constrain the event to the window, so
             * no other app receives the click event. We can't due the same to
             * movement events, since they mean that our warp cursor above
             * behaves strangely.
             */
            CGEventSetLocation(event, newLocation);
        }
    }

    return event;
}

static void
SemaphorePostCallback(CFRunLoopTimerRef timer, void *info)
{
    SDL_SemPost((SDL_sem*)info);
}

static int
Cocoa_MouseTapThread(void *data)
{
    SDL_MouseEventTapData *tapdata = (SDL_MouseEventTapData*)data;

    /* Tap was created on main thread but we own it now. */
    CFMachPortRef eventTap = tapdata->tap;
    if (eventTap) {
        /* Try to create a runloop source we can schedule. */
        CFRunLoopSourceRef runloopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
        if  (runloopSource) {
            tapdata->runloopSource = runloopSource;
        } else {
            CFRelease(eventTap);
            SDL_SemPost(tapdata->runloopStartedSemaphore);
            /* TODO: Both here and in the return below, set some state in
             * tapdata to indicate that initialization failed, which we should
             * check in InitMouseEventTap, after we move the semaphore check
             * from Quit to Init.
             */
            return 1;
        }
    } else {
        SDL_SemPost(tapdata->runloopStartedSemaphore);
        return 1;
    }

    tapdata->runloop = CFRunLoopGetCurrent();
    CFRunLoopAddSource(tapdata->runloop, tapdata->runloopSource, kCFRunLoopCommonModes);
    CFRunLoopTimerContext context = {.info = tapdata->runloopStartedSemaphore};
    /* We signal the runloop started semaphore *after* the run loop has started, indicating it's safe to CFRunLoopStop it. */
    CFRunLoopTimerRef timer = CFRunLoopTimerCreate(kCFAllocatorDefault, CFAbsoluteTimeGetCurrent(), 0, 0, 0, &SemaphorePostCallback, &context);
    CFRunLoopAddTimer(tapdata->runloop, timer, kCFRunLoopCommonModes);
    CFRelease(timer);

    /* Run the event loop to handle events in the event tap. */
    CFRunLoopRun();
    /* Make sure this is signaled so that SDL_QuitMouseEventTap knows it can safely SDL_WaitThread for us. */
    if (SDL_SemValue(tapdata->runloopStartedSemaphore) < 1) {
        SDL_SemPost(tapdata->runloopStartedSemaphore);
    }
    CFRunLoopRemoveSource(tapdata->runloop, tapdata->runloopSource, kCFRunLoopCommonModes);

    /* Clean up. */
    CGEventTapEnable(tapdata->tap, false);
    CFRelease(tapdata->runloopSource);
    CFRelease(tapdata->tap);
    tapdata->runloopSource = NULL;
    tapdata->tap = NULL;

    return 0;
}

void
Cocoa_InitMouseEventTap(SDL_MouseData* driverdata)
{
    SDL_MouseEventTapData *tapdata;
    driverdata->tapdata = SDL_calloc(1, sizeof(SDL_MouseEventTapData));
    tapdata = (SDL_MouseEventTapData*)driverdata->tapdata;

    tapdata->runloopStartedSemaphore = SDL_CreateSemaphore(0);
    if (tapdata->runloopStartedSemaphore) {
        tapdata->tap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap,
                                        kCGEventTapOptionDefault, allGrabbedEventsMask,
                                        &Cocoa_MouseTapCallback, tapdata);
        if (tapdata->tap) {
            /* Tap starts disabled, until app requests mouse grab */
            CGEventTapEnable(tapdata->tap, false);
            tapdata->thread = SDL_CreateThreadInternal(&Cocoa_MouseTapThread, "Event Tap Loop", 512 * 1024, tapdata);
            if (tapdata->thread) {
                /* Success - early out. Ownership transferred to thread. */
            	return;
            }
            CFRelease(tapdata->tap);
        }
        SDL_DestroySemaphore(tapdata->runloopStartedSemaphore);
    }
    SDL_free(driverdata->tapdata);
    driverdata->tapdata = NULL;
}

void
Cocoa_EnableMouseEventTap(SDL_MouseData *driverdata, SDL_bool enabled)
{
    SDL_MouseEventTapData *tapdata = (SDL_MouseEventTapData*)driverdata->tapdata;
    if (tapdata && tapdata->tap)
    {
        CGEventTapEnable(tapdata->tap, !!enabled);
    }
}

void
Cocoa_QuitMouseEventTap(SDL_MouseData *driverdata)
{
    SDL_MouseEventTapData *tapdata = (SDL_MouseEventTapData*)driverdata->tapdata;
    int status;

    /* Ensure that the runloop has been started first.
     * TODO: Move this to InitMouseEventTap, check for error conditions that can
     * happen in Cocoa_MouseTapThread, and fall back to the non-EventTap way of
     * grabbing the mouse if it fails to Init.
     */
    status = SDL_SemWaitTimeout(tapdata->runloopStartedSemaphore, 5000);
    if (status > -1) {
        /* Then stop it, which will cause Cocoa_MouseTapThread to return. */
        CFRunLoopStop(tapdata->runloop);
        /* And then wait for Cocoa_MouseTapThread to finish cleaning up. It
         * releases some of the pointers in tapdata. */
        SDL_WaitThread(tapdata->thread, &status);
    }

    SDL_free(driverdata->tapdata);
    driverdata->tapdata = NULL;
}

#else /* SDL_MAC_NO_SANDBOX */

void
Cocoa_InitMouseEventTap(SDL_MouseData *unused)
{
}

void
Cocoa_EnableMouseEventTap(SDL_MouseData *driverdata, SDL_bool enabled)
{
}

void
Cocoa_QuitMouseEventTap(SDL_MouseData *driverdata)
{
}

#endif /* !SDL_MAC_NO_SANDBOX */

#endif /* SDL_VIDEO_DRIVER_COCOA */

/* vi: set ts=4 sw=4 expandtab: */
