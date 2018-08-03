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

#if SDL_VIDEO_DRIVER_UIKIT

#import <UIKit/UIKit.h>

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "SDL_hints.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include "SDL_uikitvideo.h"
#include "SDL_uikitevents.h"
#include "SDL_uikitmodes.h"
#include "SDL_uikitwindow.h"
#include "SDL_uikitopengles.h"
#include "SDL_uikitclipboard.h"
#include "SDL_uikitvulkan.h"

#define UIKITVID_DRIVER_NAME "uikit"

@implementation SDL_VideoData

@end

/* Initialization/Query functions */
static int UIKit_VideoInit(_THIS);
static void UIKit_VideoQuit(_THIS);

/* DUMMY driver bootstrap functions */

static int
UIKit_Available(void)
{
    return 1;
}

static void UIKit_DeleteDevice(SDL_VideoDevice * device)
{
    @autoreleasepool {
        CFRelease(device->driverdata);
        SDL_free(device);
    }
}

static SDL_VideoDevice *
UIKit_CreateDevice(int devindex)
{
    @autoreleasepool {
        SDL_VideoDevice *device;
        SDL_VideoData *data;

        /* Initialize all variables that we clean on shutdown */
        device = (SDL_VideoDevice *) SDL_calloc(1, sizeof(SDL_VideoDevice));
        if (device) {
            data = [SDL_VideoData new];
        } else {
            SDL_free(device);
            SDL_OutOfMemory();
            return (0);
        }

        device->driverdata = (void *) CFBridgingRetain(data);

        /* Set the function pointers */
        device->VideoInit = UIKit_VideoInit;
        device->VideoQuit = UIKit_VideoQuit;
        device->GetDisplayModes = UIKit_GetDisplayModes;
        device->SetDisplayMode = UIKit_SetDisplayMode;
        device->PumpEvents = UIKit_PumpEvents;
        device->SuspendScreenSaver = UIKit_SuspendScreenSaver;
        device->CreateSDLWindow = UIKit_CreateWindow;
        device->SetWindowTitle = UIKit_SetWindowTitle;
        device->ShowWindow = UIKit_ShowWindow;
        device->HideWindow = UIKit_HideWindow;
        device->RaiseWindow = UIKit_RaiseWindow;
        device->SetWindowBordered = UIKit_SetWindowBordered;
        device->SetWindowFullscreen = UIKit_SetWindowFullscreen;
        device->DestroyWindow = UIKit_DestroyWindow;
        device->GetWindowWMInfo = UIKit_GetWindowWMInfo;
        device->GetDisplayUsableBounds = UIKit_GetDisplayUsableBounds;

#if SDL_IPHONE_KEYBOARD
        device->HasScreenKeyboardSupport = UIKit_HasScreenKeyboardSupport;
        device->ShowScreenKeyboard = UIKit_ShowScreenKeyboard;
        device->HideScreenKeyboard = UIKit_HideScreenKeyboard;
        device->IsScreenKeyboardShown = UIKit_IsScreenKeyboardShown;
        device->SetTextInputRect = UIKit_SetTextInputRect;
#endif

        device->SetClipboardText = UIKit_SetClipboardText;
        device->GetClipboardText = UIKit_GetClipboardText;
        device->HasClipboardText = UIKit_HasClipboardText;

        /* OpenGL (ES) functions */
        device->GL_MakeCurrent      = UIKit_GL_MakeCurrent;
        device->GL_GetDrawableSize  = UIKit_GL_GetDrawableSize;
        device->GL_SwapWindow       = UIKit_GL_SwapWindow;
        device->GL_CreateContext    = UIKit_GL_CreateContext;
        device->GL_DeleteContext    = UIKit_GL_DeleteContext;
        device->GL_GetProcAddress   = UIKit_GL_GetProcAddress;
        device->GL_LoadLibrary      = UIKit_GL_LoadLibrary;
        device->free = UIKit_DeleteDevice;

#if SDL_VIDEO_VULKAN
        device->Vulkan_LoadLibrary = UIKit_Vulkan_LoadLibrary;
        device->Vulkan_UnloadLibrary = UIKit_Vulkan_UnloadLibrary;
        device->Vulkan_GetInstanceExtensions
                                     = UIKit_Vulkan_GetInstanceExtensions;
        device->Vulkan_CreateSurface = UIKit_Vulkan_CreateSurface;
        device->Vulkan_GetDrawableSize = UIKit_Vulkan_GetDrawableSize;
#endif

        device->gl_config.accelerated = 1;

        return device;
    }
}

VideoBootStrap UIKIT_bootstrap = {
    UIKITVID_DRIVER_NAME, "SDL UIKit video driver",
    UIKit_Available, UIKit_CreateDevice
};


int
UIKit_VideoInit(_THIS)
{
    _this->gl_config.driver_loaded = 1;

    if (UIKit_InitModes(_this) < 0) {
        return -1;
    }
    return 0;
}

void
UIKit_VideoQuit(_THIS)
{
    UIKit_QuitModes(_this);
}

void
UIKit_SuspendScreenSaver(_THIS)
{
    @autoreleasepool {
        /* Ignore ScreenSaver API calls if the idle timer hint has been set. */
        /* FIXME: The idle timer hint should be deprecated for SDL 2.1. */
        if (!SDL_GetHintBoolean(SDL_HINT_IDLE_TIMER_DISABLED, SDL_FALSE)) {
            UIApplication *app = [UIApplication sharedApplication];

            /* Prevent the display from dimming and going to sleep. */
            app.idleTimerDisabled = (_this->suspend_screensaver != SDL_FALSE);
        }
    }
}

SDL_bool
UIKit_IsSystemVersionAtLeast(double version)
{
    return [[UIDevice currentDevice].systemVersion doubleValue] >= version;
}

CGRect
UIKit_ComputeViewFrame(SDL_Window *window, UIScreen *screen)
{
    CGRect frame = screen.bounds;

#if !TARGET_OS_TV && (__IPHONE_OS_VERSION_MIN_REQUIRED < __IPHONE_7_0)
    BOOL hasiOS7 = UIKit_IsSystemVersionAtLeast(7.0);

    /* The view should always show behind the status bar in iOS 7+. */
    if (!hasiOS7 && !(window->flags & (SDL_WINDOW_BORDERLESS|SDL_WINDOW_FULLSCREEN))) {
        frame = screen.applicationFrame;
    }
#endif

#if !TARGET_OS_TV
    /* iOS 10 seems to have a bug where, in certain conditions, putting the
     * device to sleep with the a landscape-only app open, re-orienting the
     * device to portrait, and turning it back on will result in the screen
     * bounds returning portrait orientation despite the app being in landscape.
     * This is a workaround until a better solution can be found.
     * https://bugzilla.libsdl.org/show_bug.cgi?id=3505
     * https://bugzilla.libsdl.org/show_bug.cgi?id=3465
     * https://forums.developer.apple.com/thread/65337 */
    if (UIKit_IsSystemVersionAtLeast(8.0)) {
        UIInterfaceOrientation orient = [UIApplication sharedApplication].statusBarOrientation;
        BOOL isLandscape = UIInterfaceOrientationIsLandscape(orient);

        if (isLandscape != (frame.size.width > frame.size.height)) {
            float height = frame.size.width;
            frame.size.width = frame.size.height;
            frame.size.height = height;
        }
    }
#endif

    return frame;
}

/*
 * iOS log support.
 *
 * This doesn't really have aything to do with the interfaces of the SDL video
 *  subsystem, but we need to stuff this into an Objective-C source code file.
 */

void SDL_NSLog(const char *text)
{
    NSLog(@"%s", text);
}

#endif /* SDL_VIDEO_DRIVER_UIKIT */

/* vi: set ts=4 sw=4 expandtab: */
