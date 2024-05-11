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

#if SDL_VIDEO_DRIVER_X11

#include <unistd.h> /* For getpid() and readlink() */

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "SDL_timer.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"

#include "SDL_x11video.h"
#include "SDL_x11framebuffer.h"
#include "SDL_x11shape.h"
#include "SDL_x11touch.h"
#include "SDL_x11xinput2.h"

#if SDL_VIDEO_OPENGL_EGL
#include "SDL_x11opengles.h"
#endif

#include "SDL_x11vulkan.h"

/* Initialization/Query functions */
static int X11_VideoInit(_THIS);
static void X11_VideoQuit(_THIS);

/* Find out what class name we should use */
static char *
get_classname()
{
    char *spot;
#if defined(__LINUX__) || defined(__FREEBSD__)
    char procfile[1024];
    char linkfile[1024];
    int linksize;
#endif

    /* First allow environment variable override */
    spot = SDL_getenv("SDL_VIDEO_X11_WMCLASS");
    if (spot) {
        return SDL_strdup(spot);
    }

    /* Next look at the application's executable name */
#if defined(__LINUX__) || defined(__FREEBSD__)
#if defined(__LINUX__)
    SDL_snprintf(procfile, SDL_arraysize(procfile), "/proc/%d/exe", getpid());
#elif defined(__FREEBSD__)
    SDL_snprintf(procfile, SDL_arraysize(procfile), "/proc/%d/file",
                 getpid());
#else
#error Where can we find the executable name?
#endif
    linksize = readlink(procfile, linkfile, sizeof(linkfile) - 1);
    if (linksize > 0) {
        linkfile[linksize] = '\0';
        spot = SDL_strrchr(linkfile, '/');
        if (spot) {
            return SDL_strdup(spot + 1);
        } else {
            return SDL_strdup(linkfile);
        }
    }
#endif /* __LINUX__ || __FREEBSD__ */

    /* Finally use the default we've used forever */
    return SDL_strdup("SDL_App");
}

/* X11 driver bootstrap functions */

static int
X11_Available(void)
{
    Display *display = NULL;
    if (SDL_X11_LoadSymbols()) {
        display = X11_XOpenDisplay(NULL);
        if (display != NULL) {
            X11_XCloseDisplay(display);
        }
        SDL_X11_UnloadSymbols();
    }
    return (display != NULL);
}

static void
X11_DeleteDevice(SDL_VideoDevice * device)
{
    SDL_VideoData *data = (SDL_VideoData *) device->driverdata;
    if (device->vulkan_config.loader_handle) {
        device->Vulkan_UnloadLibrary(device);
    }
    if (data->display) {
        X11_XCloseDisplay(data->display);
    }
    SDL_free(data->windowlist);
    SDL_free(device->driverdata);
    SDL_free(device);

    SDL_X11_UnloadSymbols();
}

/* An error handler to reset the vidmode and then call the default handler. */
static SDL_bool safety_net_triggered = SDL_FALSE;
static int (*orig_x11_errhandler) (Display *, XErrorEvent *) = NULL;
static int
X11_SafetyNetErrHandler(Display * d, XErrorEvent * e)
{
    SDL_VideoDevice *device = NULL;
    /* if we trigger an error in our error handler, don't try again. */
    if (!safety_net_triggered) {
        safety_net_triggered = SDL_TRUE;
        device = SDL_GetVideoDevice();
        if (device != NULL) {
            int i;
            for (i = 0; i < device->num_displays; i++) {
                SDL_VideoDisplay *display = &device->displays[i];
                if (SDL_memcmp(&display->current_mode, &display->desktop_mode,
                               sizeof (SDL_DisplayMode)) != 0) {
                    X11_SetDisplayMode(device, display, &display->desktop_mode);
                }
            }
        }
    }

    if (orig_x11_errhandler != NULL) {
        return orig_x11_errhandler(d, e);  /* probably terminate. */
    }

    return 0;
}

static SDL_VideoDevice *
X11_CreateDevice(int devindex)
{
    SDL_VideoDevice *device;
    SDL_VideoData *data;
    const char *display = NULL; /* Use the DISPLAY environment variable */

    if (!SDL_X11_LoadSymbols()) {
        return NULL;
    }

    /* Need for threading gl calls. This is also required for the proprietary
        nVidia driver to be threaded. */
    X11_XInitThreads();

    /* Initialize all variables that we clean on shutdown */
    device = (SDL_VideoDevice *) SDL_calloc(1, sizeof(SDL_VideoDevice));
    if (!device) {
        SDL_OutOfMemory();
        return NULL;
    }
    data = (struct SDL_VideoData *) SDL_calloc(1, sizeof(SDL_VideoData));
    if (!data) {
        SDL_free(device);
        SDL_OutOfMemory();
        return NULL;
    }
    device->driverdata = data;

    data->global_mouse_changed = SDL_TRUE;

    /* FIXME: Do we need this?
       if ( (SDL_strncmp(X11_XDisplayName(display), ":", 1) == 0) ||
       (SDL_strncmp(X11_XDisplayName(display), "unix:", 5) == 0) ) {
       local_X11 = 1;
       } else {
       local_X11 = 0;
       }
     */
    data->display = X11_XOpenDisplay(display);
#ifdef SDL_VIDEO_DRIVER_X11_DYNAMIC
    /* On some systems if linking without -lX11, it fails and you get following message.
     * Xlib: connection to ":0.0" refused by server
     * Xlib: XDM authorization key matches an existing client!
     *
     * It succeeds if retrying 1 second later
     * or if running xhost +localhost on shell.
     */
    if (data->display == NULL) {
        SDL_Delay(1000);
        data->display = X11_XOpenDisplay(display);
    }
#endif
    if (data->display == NULL) {
        SDL_free(device->driverdata);
        SDL_free(device);
        SDL_SetError("Couldn't open X11 display");
        return NULL;
    }
#ifdef X11_DEBUG
    X11_XSynchronize(data->display, True);
#endif

    /* Hook up an X11 error handler to recover the desktop resolution. */
    safety_net_triggered = SDL_FALSE;
    orig_x11_errhandler = X11_XSetErrorHandler(X11_SafetyNetErrHandler);

    /* Set the function pointers */
    device->VideoInit = X11_VideoInit;
    device->VideoQuit = X11_VideoQuit;
    device->ResetTouch = X11_ResetTouch;
    device->GetDisplayModes = X11_GetDisplayModes;
    device->GetDisplayBounds = X11_GetDisplayBounds;
    device->GetDisplayUsableBounds = X11_GetDisplayUsableBounds;
    device->GetDisplayDPI = X11_GetDisplayDPI;
    device->SetDisplayMode = X11_SetDisplayMode;
    device->SuspendScreenSaver = X11_SuspendScreenSaver;
    device->PumpEvents = X11_PumpEvents;

    device->CreateSDLWindow = X11_CreateWindow;
    device->CreateSDLWindowFrom = X11_CreateWindowFrom;
    device->SetWindowTitle = X11_SetWindowTitle;
    device->SetWindowIcon = X11_SetWindowIcon;
    device->SetWindowPosition = X11_SetWindowPosition;
    device->SetWindowSize = X11_SetWindowSize;
    device->SetWindowMinimumSize = X11_SetWindowMinimumSize;
    device->SetWindowMaximumSize = X11_SetWindowMaximumSize;
    device->GetWindowBordersSize = X11_GetWindowBordersSize;
    device->SetWindowOpacity = X11_SetWindowOpacity;
    device->SetWindowModalFor = X11_SetWindowModalFor;
    device->SetWindowInputFocus = X11_SetWindowInputFocus;
    device->ShowWindow = X11_ShowWindow;
    device->HideWindow = X11_HideWindow;
    device->RaiseWindow = X11_RaiseWindow;
    device->MaximizeWindow = X11_MaximizeWindow;
    device->MinimizeWindow = X11_MinimizeWindow;
    device->RestoreWindow = X11_RestoreWindow;
    device->SetWindowBordered = X11_SetWindowBordered;
    device->SetWindowResizable = X11_SetWindowResizable;
    device->SetWindowFullscreen = X11_SetWindowFullscreen;
    device->SetWindowGammaRamp = X11_SetWindowGammaRamp;
    device->SetWindowGrab = X11_SetWindowGrab;
    device->DestroyWindow = X11_DestroyWindow;
    device->CreateWindowFramebuffer = X11_CreateWindowFramebuffer;
    device->UpdateWindowFramebuffer = X11_UpdateWindowFramebuffer;
    device->DestroyWindowFramebuffer = X11_DestroyWindowFramebuffer;
    device->GetWindowWMInfo = X11_GetWindowWMInfo;
    device->SetWindowHitTest = X11_SetWindowHitTest;

    device->shape_driver.CreateShaper = X11_CreateShaper;
    device->shape_driver.SetWindowShape = X11_SetWindowShape;
    device->shape_driver.ResizeWindowShape = X11_ResizeWindowShape;

#if SDL_VIDEO_OPENGL_GLX
    device->GL_LoadLibrary = X11_GL_LoadLibrary;
    device->GL_GetProcAddress = X11_GL_GetProcAddress;
    device->GL_UnloadLibrary = X11_GL_UnloadLibrary;
    device->GL_CreateContext = X11_GL_CreateContext;
    device->GL_MakeCurrent = X11_GL_MakeCurrent;
    device->GL_SetSwapInterval = X11_GL_SetSwapInterval;
    device->GL_GetSwapInterval = X11_GL_GetSwapInterval;
    device->GL_SwapWindow = X11_GL_SwapWindow;
    device->GL_DeleteContext = X11_GL_DeleteContext;
#elif SDL_VIDEO_OPENGL_EGL
    device->GL_LoadLibrary = X11_GLES_LoadLibrary;
    device->GL_GetProcAddress = X11_GLES_GetProcAddress;
    device->GL_UnloadLibrary = X11_GLES_UnloadLibrary;
    device->GL_CreateContext = X11_GLES_CreateContext;
    device->GL_MakeCurrent = X11_GLES_MakeCurrent;
    device->GL_SetSwapInterval = X11_GLES_SetSwapInterval;
    device->GL_GetSwapInterval = X11_GLES_GetSwapInterval;
    device->GL_SwapWindow = X11_GLES_SwapWindow;
    device->GL_DeleteContext = X11_GLES_DeleteContext;
#endif

    device->SetClipboardText = X11_SetClipboardText;
    device->GetClipboardText = X11_GetClipboardText;
    device->HasClipboardText = X11_HasClipboardText;
    device->StartTextInput = X11_StartTextInput;
    device->StopTextInput = X11_StopTextInput;
    device->SetTextInputRect = X11_SetTextInputRect;

    device->free = X11_DeleteDevice;

#if SDL_VIDEO_VULKAN
    device->Vulkan_LoadLibrary = X11_Vulkan_LoadLibrary;
    device->Vulkan_UnloadLibrary = X11_Vulkan_UnloadLibrary;
    device->Vulkan_GetInstanceExtensions = X11_Vulkan_GetInstanceExtensions;
    device->Vulkan_CreateSurface = X11_Vulkan_CreateSurface;
#endif

    return device;
}

VideoBootStrap X11_bootstrap = {
    "x11", "SDL X11 video driver",
    X11_Available, X11_CreateDevice
};

static int (*handler) (Display *, XErrorEvent *) = NULL;
static int
X11_CheckWindowManagerErrorHandler(Display * d, XErrorEvent * e)
{
    if (e->error_code == BadWindow) {
        return (0);
    } else {
        return (handler(d, e));
    }
}

static void
X11_CheckWindowManager(_THIS)
{
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;
    Display *display = data->display;
    Atom _NET_SUPPORTING_WM_CHECK;
    int status, real_format;
    Atom real_type;
    unsigned long items_read = 0, items_left = 0;
    unsigned char *propdata = NULL;
    Window wm_window = 0;
#ifdef DEBUG_WINDOW_MANAGER
    char *wm_name;
#endif

    /* Set up a handler to gracefully catch errors */
    X11_XSync(display, False);
    handler = X11_XSetErrorHandler(X11_CheckWindowManagerErrorHandler);

    _NET_SUPPORTING_WM_CHECK = X11_XInternAtom(display, "_NET_SUPPORTING_WM_CHECK", False);
    status = X11_XGetWindowProperty(display, DefaultRootWindow(display), _NET_SUPPORTING_WM_CHECK, 0L, 1L, False, XA_WINDOW, &real_type, &real_format, &items_read, &items_left, &propdata);
    if (status == Success) {
        if (items_read) {
            wm_window = ((Window*)propdata)[0];
        }
        if (propdata) {
            X11_XFree(propdata);
            propdata = NULL;
        }
    }

    if (wm_window) {
        status = X11_XGetWindowProperty(display, wm_window, _NET_SUPPORTING_WM_CHECK, 0L, 1L, False, XA_WINDOW, &real_type, &real_format, &items_read, &items_left, &propdata);
        if (status != Success || !items_read || wm_window != ((Window*)propdata)[0]) {
            wm_window = None;
        }
        if (status == Success && propdata) {
            X11_XFree(propdata);
            propdata = NULL;
        }
    }

    /* Reset the error handler, we're done checking */
    X11_XSync(display, False);
    X11_XSetErrorHandler(handler);

    if (!wm_window) {
#ifdef DEBUG_WINDOW_MANAGER
        printf("Couldn't get _NET_SUPPORTING_WM_CHECK property\n");
#endif
        return;
    }
    data->net_wm = SDL_TRUE;

#ifdef DEBUG_WINDOW_MANAGER
    wm_name = X11_GetWindowTitle(_this, wm_window);
    printf("Window manager: %s\n", wm_name);
    SDL_free(wm_name);
#endif
}


int
X11_VideoInit(_THIS)
{
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;

    /* Get the window class name, usually the name of the application */
    data->classname = get_classname();

    /* Get the process PID to be associated to the window */
    data->pid = getpid();

    /* I have no idea how random this actually is, or has to be. */
    data->window_group = (XID) (((size_t) data->pid) ^ ((size_t) _this));

    /* Look up some useful Atoms */
#define GET_ATOM(X) data->X = X11_XInternAtom(data->display, #X, False)
    GET_ATOM(WM_PROTOCOLS);
    GET_ATOM(WM_DELETE_WINDOW);
    GET_ATOM(WM_TAKE_FOCUS);
    GET_ATOM(_NET_WM_STATE);
    GET_ATOM(_NET_WM_STATE_HIDDEN);
    GET_ATOM(_NET_WM_STATE_FOCUSED);
    GET_ATOM(_NET_WM_STATE_MAXIMIZED_VERT);
    GET_ATOM(_NET_WM_STATE_MAXIMIZED_HORZ);
    GET_ATOM(_NET_WM_STATE_FULLSCREEN);
    GET_ATOM(_NET_WM_STATE_ABOVE);
    GET_ATOM(_NET_WM_STATE_SKIP_TASKBAR);
    GET_ATOM(_NET_WM_STATE_SKIP_PAGER);
    GET_ATOM(_NET_WM_ALLOWED_ACTIONS);
    GET_ATOM(_NET_WM_ACTION_FULLSCREEN);
    GET_ATOM(_NET_WM_NAME);
    GET_ATOM(_NET_WM_ICON_NAME);
    GET_ATOM(_NET_WM_ICON);
    GET_ATOM(_NET_WM_PING);
    GET_ATOM(_NET_WM_WINDOW_OPACITY);
    GET_ATOM(_NET_WM_USER_TIME);
    GET_ATOM(_NET_ACTIVE_WINDOW);
    GET_ATOM(_NET_FRAME_EXTENTS);
    GET_ATOM(UTF8_STRING);
    GET_ATOM(PRIMARY);
    GET_ATOM(XdndEnter);
    GET_ATOM(XdndPosition);
    GET_ATOM(XdndStatus);
    GET_ATOM(XdndTypeList);
    GET_ATOM(XdndActionCopy);
    GET_ATOM(XdndDrop);
    GET_ATOM(XdndFinished);
    GET_ATOM(XdndSelection);
    GET_ATOM(XKLAVIER_STATE);

    /* Detect the window manager */
    X11_CheckWindowManager(_this);

    if (X11_InitModes(_this) < 0) {
        return -1;
    }

    X11_InitXinput2(_this);

    if (X11_InitKeyboard(_this) != 0) {
        return -1;
    }
    X11_InitMouse(_this);

    X11_InitTouch(_this);

#if SDL_USE_LIBDBUS
    SDL_DBus_Init();
#endif

    return 0;
}

void
X11_VideoQuit(_THIS)
{
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;

    if (data->clipboard_window) {
        X11_XDestroyWindow(data->display, data->clipboard_window);
    }

    SDL_free(data->classname);
#ifdef X_HAVE_UTF8_STRING
    if (data->im) {
        X11_XCloseIM(data->im);
    }
#endif

    X11_QuitModes(_this);
    X11_QuitKeyboard(_this);
    X11_QuitMouse(_this);
    X11_QuitTouch(_this);

/* !!! FIXME: other subsystems use D-Bus, so we shouldn't quit it here;
       have SDL.c do this at a higher level, or add refcounting. */
#if SDL_USE_LIBDBUS
    SDL_DBus_Quit();
#endif
}

SDL_bool
X11_UseDirectColorVisuals(void)
{
    return SDL_getenv("SDL_VIDEO_X11_NODIRECTCOLOR") ? SDL_FALSE : SDL_TRUE;
}

#endif /* SDL_VIDEO_DRIVER_X11 */

/* vim: set ts=4 sw=4 expandtab: */
