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

#include "SDL_assert.h"
#include "SDL_uikitmodes.h"

@implementation SDL_DisplayData

@synthesize uiscreen;

@end

@implementation SDL_DisplayModeData

@synthesize uiscreenmode;

@end


static int
UIKit_AllocateDisplayModeData(SDL_DisplayMode * mode,
    UIScreenMode * uiscreenmode)
{
    SDL_DisplayModeData *data = nil;

    if (uiscreenmode != nil) {
        /* Allocate the display mode data */
        data = [[SDL_DisplayModeData alloc] init];
        if (!data) {
            return SDL_OutOfMemory();
        }

        data.uiscreenmode = uiscreenmode;
    }

    mode->driverdata = (void *) CFBridgingRetain(data);

    return 0;
}

static void
UIKit_FreeDisplayModeData(SDL_DisplayMode * mode)
{
    if (mode->driverdata != NULL) {
        CFRelease(mode->driverdata);
        mode->driverdata = NULL;
    }
}

static NSUInteger
UIKit_GetDisplayModeRefreshRate(UIScreen *uiscreen)
{
#ifdef __IPHONE_10_3
    if ([uiscreen respondsToSelector:@selector(maximumFramesPerSecond)]) {
        return uiscreen.maximumFramesPerSecond;
    }
#endif
    return 0;
}

static int
UIKit_AddSingleDisplayMode(SDL_VideoDisplay * display, int w, int h,
    UIScreen * uiscreen, UIScreenMode * uiscreenmode)
{
    SDL_DisplayMode mode;
    SDL_zero(mode);

    if (UIKit_AllocateDisplayModeData(&mode, uiscreenmode) < 0) {
        return -1;
    }

    mode.format = SDL_PIXELFORMAT_ABGR8888;
    mode.refresh_rate = (int) UIKit_GetDisplayModeRefreshRate(uiscreen);
    mode.w = w;
    mode.h = h;

    if (SDL_AddDisplayMode(display, &mode)) {
        return 0;
    } else {
        UIKit_FreeDisplayModeData(&mode);
        return -1;
    }
}

static int
UIKit_AddDisplayMode(SDL_VideoDisplay * display, int w, int h, UIScreen * uiscreen,
                     UIScreenMode * uiscreenmode, SDL_bool addRotation)
{
    if (UIKit_AddSingleDisplayMode(display, w, h, uiscreen, uiscreenmode) < 0) {
        return -1;
    }

    if (addRotation) {
        /* Add the rotated version */
        if (UIKit_AddSingleDisplayMode(display, h, w, uiscreen, uiscreenmode) < 0) {
            return -1;
        }
    }

    return 0;
}

static int
UIKit_AddDisplay(UIScreen *uiscreen)
{
    UIScreenMode *uiscreenmode = uiscreen.currentMode;
    CGSize size = uiscreen.bounds.size;
    SDL_VideoDisplay display;
    SDL_DisplayMode mode;
    SDL_zero(mode);

    /* Make sure the width/height are oriented correctly */
    if (UIKit_IsDisplayLandscape(uiscreen) != (size.width > size.height)) {
        CGFloat height = size.width;
        size.width = size.height;
        size.height = height;
    }

    mode.format = SDL_PIXELFORMAT_ABGR8888;
    mode.refresh_rate = (int) UIKit_GetDisplayModeRefreshRate(uiscreen);
    mode.w = (int) size.width;
    mode.h = (int) size.height;

    if (UIKit_AllocateDisplayModeData(&mode, uiscreenmode) < 0) {
        return -1;
    }

    SDL_zero(display);
    display.desktop_mode = mode;
    display.current_mode = mode;

    /* Allocate the display data */
    SDL_DisplayData *data = [[SDL_DisplayData alloc] init];
    if (!data) {
        UIKit_FreeDisplayModeData(&display.desktop_mode);
        return SDL_OutOfMemory();
    }

    data.uiscreen = uiscreen;

    display.driverdata = (void *) CFBridgingRetain(data);
    SDL_AddVideoDisplay(&display);

    return 0;
}

SDL_bool
UIKit_IsDisplayLandscape(UIScreen *uiscreen)
{
#if !TARGET_OS_TV
    if (uiscreen == [UIScreen mainScreen]) {
        return UIInterfaceOrientationIsLandscape([UIApplication sharedApplication].statusBarOrientation);
    } else
#endif /* !TARGET_OS_TV */
    {
        CGSize size = uiscreen.bounds.size;
        return (size.width > size.height);
    }
}

int
UIKit_InitModes(_THIS)
{
    @autoreleasepool {
        for (UIScreen *uiscreen in [UIScreen screens]) {
            if (UIKit_AddDisplay(uiscreen) < 0) {
                return -1;
            }
        }
    }

    return 0;
}

void
UIKit_GetDisplayModes(_THIS, SDL_VideoDisplay * display)
{
    @autoreleasepool {
        SDL_DisplayData *data = (__bridge SDL_DisplayData *) display->driverdata;

        SDL_bool isLandscape = UIKit_IsDisplayLandscape(data.uiscreen);
        SDL_bool addRotation = (data.uiscreen == [UIScreen mainScreen]);
        CGFloat scale = data.uiscreen.scale;
        NSArray *availableModes = nil;

#if TARGET_OS_TV
        addRotation = SDL_FALSE;
        availableModes = @[data.uiscreen.currentMode];
#else
        availableModes = data.uiscreen.availableModes;
#endif

#ifdef __IPHONE_8_0
        /* The UIScreenMode of an iPhone 6 Plus should be 1080x1920 rather than
         * 1242x2208 (414x736@3x), so we should use the native scale. */
        if ([data.uiscreen respondsToSelector:@selector(nativeScale)]) {
            scale = data.uiscreen.nativeScale;
        }
#endif

        for (UIScreenMode *uimode in availableModes) {
            /* The size of a UIScreenMode is in pixels, but we deal exclusively
             * in points (except in SDL_GL_GetDrawableSize.) */
            int w = (int)(uimode.size.width / scale);
            int h = (int)(uimode.size.height / scale);

            /* Make sure the width/height are oriented correctly */
            if (isLandscape != (w > h)) {
                int tmp = w;
                w = h;
                h = tmp;
            }

            UIKit_AddDisplayMode(display, w, h, data.uiscreen, uimode, addRotation);
        }
    }
}

int
UIKit_SetDisplayMode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode)
{
    @autoreleasepool {
        SDL_DisplayData *data = (__bridge SDL_DisplayData *) display->driverdata;

#if !TARGET_OS_TV
        SDL_DisplayModeData *modedata = (__bridge SDL_DisplayModeData *)mode->driverdata;
        [data.uiscreen setCurrentMode:modedata.uiscreenmode];
#endif

        if (data.uiscreen == [UIScreen mainScreen]) {
            /* [UIApplication setStatusBarOrientation:] no longer works reliably
             * in recent iOS versions, so we can't rotate the screen when setting
             * the display mode. */
            if (mode->w > mode->h) {
                if (!UIKit_IsDisplayLandscape(data.uiscreen)) {
                    return SDL_SetError("Screen orientation does not match display mode size");
                }
            } else if (mode->w < mode->h) {
                if (UIKit_IsDisplayLandscape(data.uiscreen)) {
                    return SDL_SetError("Screen orientation does not match display mode size");
                }
            }
        }
    }

    return 0;
}

int
UIKit_GetDisplayUsableBounds(_THIS, SDL_VideoDisplay * display, SDL_Rect * rect)
{
    @autoreleasepool {
        int displayIndex = (int) (display - _this->displays);
        SDL_DisplayData *data = (__bridge SDL_DisplayData *) display->driverdata;
        CGRect frame = data.uiscreen.bounds;

        /* the default function iterates displays to make a fake offset,
         as if all the displays were side-by-side, which is fine for iOS. */
        if (SDL_GetDisplayBounds(displayIndex, rect) < 0) {
            return -1;
        }

#if !TARGET_OS_TV && __IPHONE_OS_VERSION_MIN_REQUIRED < __IPHONE_7_0
        if (!UIKit_IsSystemVersionAtLeast(7.0)) {
            frame = [data.uiscreen applicationFrame];
        }
#endif

        rect->x += frame.origin.x;
        rect->y += frame.origin.y;
        rect->w = frame.size.width;
        rect->h = frame.size.height;
    }

    return 0;
}

void
UIKit_QuitModes(_THIS)
{
    /* Release Objective-C objects, so higher level doesn't free() them. */
    int i, j;
    @autoreleasepool {
        for (i = 0; i < _this->num_displays; i++) {
            SDL_VideoDisplay *display = &_this->displays[i];

            UIKit_FreeDisplayModeData(&display->desktop_mode);
            for (j = 0; j < display->num_display_modes; j++) {
                SDL_DisplayMode *mode = &display->display_modes[j];
                UIKit_FreeDisplayModeData(mode);
            }

            if (display->driverdata != NULL) {
                CFRelease(display->driverdata);
                display->driverdata = NULL;
            }
        }
    }
}

#endif /* SDL_VIDEO_DRIVER_UIKIT */

/* vi: set ts=4 sw=4 expandtab: */
