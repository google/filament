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

#include "SDL.h"
#include "SDL_endian.h"
#include "SDL_cocoavideo.h"
#include "SDL_cocoashape.h"
#include "SDL_cocoavulkan.h"
#include "SDL_assert.h"

/* Initialization/Query functions */
static int Cocoa_VideoInit(_THIS);
static void Cocoa_VideoQuit(_THIS);

/* Cocoa driver bootstrap functions */

static int
Cocoa_Available(void)
{
    return (1);
}

static void
Cocoa_DeleteDevice(SDL_VideoDevice * device)
{
    SDL_free(device->driverdata);
    SDL_free(device);
}

static SDL_VideoDevice *
Cocoa_CreateDevice(int devindex)
{
    SDL_VideoDevice *device;
    SDL_VideoData *data;

    Cocoa_RegisterApp();

    /* Initialize all variables that we clean on shutdown */
    device = (SDL_VideoDevice *) SDL_calloc(1, sizeof(SDL_VideoDevice));
    if (device) {
        data = (struct SDL_VideoData *) SDL_calloc(1, sizeof(SDL_VideoData));
    } else {
        data = NULL;
    }
    if (!data) {
        SDL_OutOfMemory();
        SDL_free(device);
        return NULL;
    }
    device->driverdata = data;

    /* Set the function pointers */
    device->VideoInit = Cocoa_VideoInit;
    device->VideoQuit = Cocoa_VideoQuit;
    device->GetDisplayBounds = Cocoa_GetDisplayBounds;
    device->GetDisplayUsableBounds = Cocoa_GetDisplayUsableBounds;
    device->GetDisplayDPI = Cocoa_GetDisplayDPI;
    device->GetDisplayModes = Cocoa_GetDisplayModes;
    device->SetDisplayMode = Cocoa_SetDisplayMode;
    device->PumpEvents = Cocoa_PumpEvents;
    device->SuspendScreenSaver = Cocoa_SuspendScreenSaver;

    device->CreateSDLWindow = Cocoa_CreateWindow;
    device->CreateSDLWindowFrom = Cocoa_CreateWindowFrom;
    device->SetWindowTitle = Cocoa_SetWindowTitle;
    device->SetWindowIcon = Cocoa_SetWindowIcon;
    device->SetWindowPosition = Cocoa_SetWindowPosition;
    device->SetWindowSize = Cocoa_SetWindowSize;
    device->SetWindowMinimumSize = Cocoa_SetWindowMinimumSize;
    device->SetWindowMaximumSize = Cocoa_SetWindowMaximumSize;
    device->SetWindowOpacity = Cocoa_SetWindowOpacity;
    device->ShowWindow = Cocoa_ShowWindow;
    device->HideWindow = Cocoa_HideWindow;
    device->RaiseWindow = Cocoa_RaiseWindow;
    device->MaximizeWindow = Cocoa_MaximizeWindow;
    device->MinimizeWindow = Cocoa_MinimizeWindow;
    device->RestoreWindow = Cocoa_RestoreWindow;
    device->SetWindowBordered = Cocoa_SetWindowBordered;
    device->SetWindowResizable = Cocoa_SetWindowResizable;
    device->SetWindowFullscreen = Cocoa_SetWindowFullscreen;
    device->SetWindowGammaRamp = Cocoa_SetWindowGammaRamp;
    device->GetWindowGammaRamp = Cocoa_GetWindowGammaRamp;
    device->SetWindowGrab = Cocoa_SetWindowGrab;
    device->DestroyWindow = Cocoa_DestroyWindow;
    device->GetWindowWMInfo = Cocoa_GetWindowWMInfo;
    device->SetWindowHitTest = Cocoa_SetWindowHitTest;

    device->shape_driver.CreateShaper = Cocoa_CreateShaper;
    device->shape_driver.SetWindowShape = Cocoa_SetWindowShape;
    device->shape_driver.ResizeWindowShape = Cocoa_ResizeWindowShape;

#if SDL_VIDEO_OPENGL_CGL
    device->GL_LoadLibrary = Cocoa_GL_LoadLibrary;
    device->GL_GetProcAddress = Cocoa_GL_GetProcAddress;
    device->GL_UnloadLibrary = Cocoa_GL_UnloadLibrary;
    device->GL_CreateContext = Cocoa_GL_CreateContext;
    device->GL_MakeCurrent = Cocoa_GL_MakeCurrent;
    device->GL_GetDrawableSize = Cocoa_GL_GetDrawableSize;
    device->GL_SetSwapInterval = Cocoa_GL_SetSwapInterval;
    device->GL_GetSwapInterval = Cocoa_GL_GetSwapInterval;
    device->GL_SwapWindow = Cocoa_GL_SwapWindow;
    device->GL_DeleteContext = Cocoa_GL_DeleteContext;
#elif SDL_VIDEO_OPENGL_EGL
    device->GL_LoadLibrary = Cocoa_GLES_LoadLibrary;
    device->GL_GetProcAddress = Cocoa_GLES_GetProcAddress;
    device->GL_UnloadLibrary = Cocoa_GLES_UnloadLibrary;
    device->GL_CreateContext = Cocoa_GLES_CreateContext;
    device->GL_MakeCurrent = Cocoa_GLES_MakeCurrent;
    device->GL_SetSwapInterval = Cocoa_GLES_SetSwapInterval;
    device->GL_GetSwapInterval = Cocoa_GLES_GetSwapInterval;
    device->GL_SwapWindow = Cocoa_GLES_SwapWindow;
    device->GL_DeleteContext = Cocoa_GLES_DeleteContext;
#endif

#if SDL_VIDEO_VULKAN
    device->Vulkan_LoadLibrary = Cocoa_Vulkan_LoadLibrary;
    device->Vulkan_UnloadLibrary = Cocoa_Vulkan_UnloadLibrary;
    device->Vulkan_GetInstanceExtensions = Cocoa_Vulkan_GetInstanceExtensions;
    device->Vulkan_CreateSurface = Cocoa_Vulkan_CreateSurface;
    device->Vulkan_GetDrawableSize = Cocoa_Vulkan_GetDrawableSize;
#endif

    device->StartTextInput = Cocoa_StartTextInput;
    device->StopTextInput = Cocoa_StopTextInput;
    device->SetTextInputRect = Cocoa_SetTextInputRect;

    device->SetClipboardText = Cocoa_SetClipboardText;
    device->GetClipboardText = Cocoa_GetClipboardText;
    device->HasClipboardText = Cocoa_HasClipboardText;

    device->free = Cocoa_DeleteDevice;

    return device;
}

VideoBootStrap COCOA_bootstrap = {
    "cocoa", "SDL Cocoa video driver",
    Cocoa_Available, Cocoa_CreateDevice
};


int
Cocoa_VideoInit(_THIS)
{
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;

    Cocoa_InitModes(_this);
    Cocoa_InitKeyboard(_this);
    Cocoa_InitMouse(_this);

    data->allow_spaces = ((floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6) && SDL_GetHintBoolean(SDL_HINT_VIDEO_MAC_FULLSCREEN_SPACES, SDL_TRUE));

    /* The IOPM assertion API can disable the screensaver as of 10.7. */
    data->screensaver_use_iopm = floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6;

    return 0;
}

void
Cocoa_VideoQuit(_THIS)
{
    Cocoa_QuitModes(_this);
    Cocoa_QuitKeyboard(_this);
    Cocoa_QuitMouse(_this);
}

/* This function assumes that it's called from within an autorelease pool */
NSImage *
Cocoa_CreateImage(SDL_Surface * surface)
{
    SDL_Surface *converted;
    NSBitmapImageRep *imgrep;
    Uint8 *pixels;
    int i;
    NSImage *img;

    converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    if (!converted) {
        return nil;
    }

    imgrep = [[[NSBitmapImageRep alloc] initWithBitmapDataPlanes: NULL
                    pixelsWide: converted->w
                    pixelsHigh: converted->h
                    bitsPerSample: 8
                    samplesPerPixel: 4
                    hasAlpha: YES
                    isPlanar: NO
                    colorSpaceName: NSDeviceRGBColorSpace
                    bytesPerRow: converted->pitch
                    bitsPerPixel: converted->format->BitsPerPixel] autorelease];
    if (imgrep == nil) {
        SDL_FreeSurface(converted);
        return nil;
    }

    /* Copy the pixels */
    pixels = [imgrep bitmapData];
    SDL_memcpy(pixels, converted->pixels, converted->h * converted->pitch);
    SDL_FreeSurface(converted);

    /* Premultiply the alpha channel */
    for (i = (surface->h * surface->w); i--; ) {
        Uint8 alpha = pixels[3];
        pixels[0] = (Uint8)(((Uint16)pixels[0] * alpha) / 255);
        pixels[1] = (Uint8)(((Uint16)pixels[1] * alpha) / 255);
        pixels[2] = (Uint8)(((Uint16)pixels[2] * alpha) / 255);
        pixels += 4;
    }

    img = [[[NSImage alloc] initWithSize: NSMakeSize(surface->w, surface->h)] autorelease];
    if (img != nil) {
        [img addRepresentation: imgrep];
    }
    return img;
}

/*
 * Mac OS X log support.
 *
 * This doesn't really have aything to do with the interfaces of the SDL video
 *  subsystem, but we need to stuff this into an Objective-C source code file.
 */

void SDL_NSLog(const char *text)
{
    NSLog(@"%s", text);
}

#endif /* SDL_VIDEO_DRIVER_COCOA */

/* vim: set ts=4 sw=4 expandtab: */
