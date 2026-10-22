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

#if SDL_VIDEO_DRIVER_ANDROID

/* Android SDL video driver implementation
*/

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"
#include "../../events/SDL_windowevents_c.h"

#include "SDL_androidvideo.h"
#include "SDL_androidgl.h"
#include "SDL_androidclipboard.h"
#include "SDL_androidevents.h"
#include "SDL_androidkeyboard.h"
#include "SDL_androidmouse.h"
#include "SDL_androidtouch.h"
#include "SDL_androidwindow.h"
#include "SDL_androidvulkan.h"

#define ANDROID_VID_DRIVER_NAME "Android"

/* Initialization/Query functions */
static int Android_VideoInit(_THIS);
static void Android_VideoQuit(_THIS);
int Android_GetDisplayDPI(_THIS, SDL_VideoDisplay * display, float * ddpi, float * hdpi, float * vdpi);

#include "../SDL_egl_c.h"
#define Android_GLES_GetProcAddress SDL_EGL_GetProcAddress
#define Android_GLES_UnloadLibrary SDL_EGL_UnloadLibrary
#define Android_GLES_SetSwapInterval SDL_EGL_SetSwapInterval
#define Android_GLES_GetSwapInterval SDL_EGL_GetSwapInterval
#define Android_GLES_DeleteContext SDL_EGL_DeleteContext

/* Android driver bootstrap functions */


/* These are filled in with real values in Android_SetScreenResolution on init (before SDL_main()) */
int Android_ScreenWidth = 0;
int Android_ScreenHeight = 0;
Uint32 Android_ScreenFormat = SDL_PIXELFORMAT_UNKNOWN;
static int Android_ScreenRate = 0;

SDL_sem *Android_PauseSem = NULL, *Android_ResumeSem = NULL;

/* Currently only one window */
SDL_Window *Android_Window = NULL;

static int
Android_Available(void)
{
    return 1;
}

static void
Android_SuspendScreenSaver(_THIS)
{
    Android_JNI_SuspendScreenSaver(_this->suspend_screensaver);
}

static void
Android_DeleteDevice(SDL_VideoDevice * device)
{
    SDL_free(device->driverdata);
    SDL_free(device);
}

static SDL_VideoDevice *
Android_CreateDevice(int devindex)
{
    SDL_VideoDevice *device;
    SDL_VideoData *data;

    /* Initialize all variables that we clean on shutdown */
    device = (SDL_VideoDevice *) SDL_calloc(1, sizeof(SDL_VideoDevice));
    if (!device) {
        SDL_OutOfMemory();
        return NULL;
    }

    data = (SDL_VideoData*) SDL_calloc(1, sizeof(SDL_VideoData));
    if (!data) {
        SDL_OutOfMemory();
        SDL_free(device);
        return NULL;
    }

    device->driverdata = data;

    /* Set the function pointers */
    device->VideoInit = Android_VideoInit;
    device->VideoQuit = Android_VideoQuit;
    device->PumpEvents = Android_PumpEvents;

    device->GetDisplayDPI = Android_GetDisplayDPI;

    device->CreateSDLWindow = Android_CreateWindow;
    device->SetWindowTitle = Android_SetWindowTitle;
    device->SetWindowFullscreen = Android_SetWindowFullscreen;
    device->DestroyWindow = Android_DestroyWindow;
    device->GetWindowWMInfo = Android_GetWindowWMInfo;

    device->free = Android_DeleteDevice;

    /* GL pointers */
    device->GL_LoadLibrary = Android_GLES_LoadLibrary;
    device->GL_GetProcAddress = Android_GLES_GetProcAddress;
    device->GL_UnloadLibrary = Android_GLES_UnloadLibrary;
    device->GL_CreateContext = Android_GLES_CreateContext;
    device->GL_MakeCurrent = Android_GLES_MakeCurrent;
    device->GL_SetSwapInterval = Android_GLES_SetSwapInterval;
    device->GL_GetSwapInterval = Android_GLES_GetSwapInterval;
    device->GL_SwapWindow = Android_GLES_SwapWindow;
    device->GL_DeleteContext = Android_GLES_DeleteContext;

#if SDL_VIDEO_VULKAN
    device->Vulkan_LoadLibrary = Android_Vulkan_LoadLibrary;
    device->Vulkan_UnloadLibrary = Android_Vulkan_UnloadLibrary;
    device->Vulkan_GetInstanceExtensions = Android_Vulkan_GetInstanceExtensions;
    device->Vulkan_CreateSurface = Android_Vulkan_CreateSurface;
#endif

    /* Screensaver */
    device->SuspendScreenSaver = Android_SuspendScreenSaver;

    /* Text input */
    device->StartTextInput = Android_StartTextInput;
    device->StopTextInput = Android_StopTextInput;
    device->SetTextInputRect = Android_SetTextInputRect;

    /* Screen keyboard */
    device->HasScreenKeyboardSupport = Android_HasScreenKeyboardSupport;
    device->IsScreenKeyboardShown = Android_IsScreenKeyboardShown;

    /* Clipboard */
    device->SetClipboardText = Android_SetClipboardText;
    device->GetClipboardText = Android_GetClipboardText;
    device->HasClipboardText = Android_HasClipboardText;

    return device;
}

VideoBootStrap Android_bootstrap = {
    ANDROID_VID_DRIVER_NAME, "SDL Android video driver",
    Android_Available, Android_CreateDevice
};


int
Android_VideoInit(_THIS)
{
    SDL_DisplayMode mode;

    mode.format = Android_ScreenFormat;
    mode.w = Android_ScreenWidth;
    mode.h = Android_ScreenHeight;
    mode.refresh_rate = Android_ScreenRate;
    mode.driverdata = NULL;
    if (SDL_AddBasicVideoDisplay(&mode) < 0) {
        return -1;
    }

    SDL_AddDisplayMode(&_this->displays[0], &mode);

    Android_InitKeyboard();

    Android_InitTouch();

    Android_InitMouse();

    /* We're done! */
    return 0;
}

void
Android_VideoQuit(_THIS)
{
    Android_QuitTouch();
}

int
Android_GetDisplayDPI(_THIS, SDL_VideoDisplay * display, float * ddpi, float * hdpi, float * vdpi)
{
    return Android_JNI_GetDisplayDPI(ddpi, hdpi, vdpi);
}

void
Android_SetScreenResolution(int width, int height, Uint32 format, float rate)
{
	SDL_VideoDevice* device;
	SDL_VideoDisplay *display;
    Android_ScreenWidth = width;
    Android_ScreenHeight = height;
    Android_ScreenFormat = format;
    Android_ScreenRate = rate;

    /*
      Update the resolution of the desktop mode, so that the window
      can be properly resized. The screen resolution change can for
      example happen when the Activity enters or exits immersive mode,
      which can happen after VideoInit().
    */
    device = SDL_GetVideoDevice();
    if (device && device->num_displays > 0)
    {
        display = &device->displays[0];
        display->desktop_mode.format = Android_ScreenFormat;
        display->desktop_mode.w = Android_ScreenWidth;
        display->desktop_mode.h = Android_ScreenHeight;
        display->desktop_mode.refresh_rate  = Android_ScreenRate;
    }

    if (Android_Window) {
        /* Force the current mode to match the resize otherwise the SDL_WINDOWEVENT_RESTORED event
         * will fall back to the old mode */
        display = SDL_GetDisplayForWindow(Android_Window);

        display->display_modes[0].format = format;
        display->display_modes[0].w = width;
        display->display_modes[0].h = height;
        display->display_modes[0].refresh_rate = rate;
        display->current_mode = display->display_modes[0];

        SDL_SendWindowEvent(Android_Window, SDL_WINDOWEVENT_RESIZED, width, height);
    }
}

#endif /* SDL_VIDEO_DRIVER_ANDROID */

/* vi: set ts=4 sw=4 expandtab: */
