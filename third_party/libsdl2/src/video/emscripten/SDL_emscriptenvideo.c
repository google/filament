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

#if SDL_VIDEO_DRIVER_EMSCRIPTEN

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "SDL_hints.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../SDL_egl_c.h"
#include "../../events/SDL_events_c.h"

#include "SDL_emscriptenvideo.h"
#include "SDL_emscriptenopengles.h"
#include "SDL_emscriptenframebuffer.h"
#include "SDL_emscriptenevents.h"
#include "SDL_emscriptenmouse.h"

#define EMSCRIPTENVID_DRIVER_NAME "emscripten"

/* Initialization/Query functions */
static int Emscripten_VideoInit(_THIS);
static int Emscripten_SetDisplayMode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode);
static void Emscripten_VideoQuit(_THIS);

static int Emscripten_CreateWindow(_THIS, SDL_Window * window);
static void Emscripten_SetWindowSize(_THIS, SDL_Window * window);
static void Emscripten_DestroyWindow(_THIS, SDL_Window * window);
static void Emscripten_SetWindowFullscreen(_THIS, SDL_Window * window, SDL_VideoDisplay * display, SDL_bool fullscreen);
static void Emscripten_PumpEvents(_THIS);
static void Emscripten_SetWindowTitle(_THIS, SDL_Window * window);


/* Emscripten driver bootstrap functions */

static int
Emscripten_Available(void)
{
    return (1);
}

static void
Emscripten_DeleteDevice(SDL_VideoDevice * device)
{
    SDL_free(device);
}

static SDL_VideoDevice *
Emscripten_CreateDevice(int devindex)
{
    SDL_VideoDevice *device;

    /* Initialize all variables that we clean on shutdown */
    device = (SDL_VideoDevice *) SDL_calloc(1, sizeof(SDL_VideoDevice));
    if (!device) {
        SDL_OutOfMemory();
        return (0);
    }

    /* Firefox sends blur event which would otherwise prevent full screen
     * when the user clicks to allow full screen.
     * See https://bugzilla.mozilla.org/show_bug.cgi?id=1144964
    */
    SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

    /* Set the function pointers */
    device->VideoInit = Emscripten_VideoInit;
    device->VideoQuit = Emscripten_VideoQuit;
    device->SetDisplayMode = Emscripten_SetDisplayMode;


    device->PumpEvents = Emscripten_PumpEvents;

    device->CreateSDLWindow = Emscripten_CreateWindow;
    device->SetWindowTitle = Emscripten_SetWindowTitle;
    /*device->SetWindowIcon = Emscripten_SetWindowIcon;
    device->SetWindowPosition = Emscripten_SetWindowPosition;*/
    device->SetWindowSize = Emscripten_SetWindowSize;
    /*device->ShowWindow = Emscripten_ShowWindow;
    device->HideWindow = Emscripten_HideWindow;
    device->RaiseWindow = Emscripten_RaiseWindow;
    device->MaximizeWindow = Emscripten_MaximizeWindow;
    device->MinimizeWindow = Emscripten_MinimizeWindow;
    device->RestoreWindow = Emscripten_RestoreWindow;
    device->SetWindowGrab = Emscripten_SetWindowGrab;*/
    device->DestroyWindow = Emscripten_DestroyWindow;
    device->SetWindowFullscreen = Emscripten_SetWindowFullscreen;

    device->CreateWindowFramebuffer = Emscripten_CreateWindowFramebuffer;
    device->UpdateWindowFramebuffer = Emscripten_UpdateWindowFramebuffer;
    device->DestroyWindowFramebuffer = Emscripten_DestroyWindowFramebuffer;

#if SDL_VIDEO_OPENGL_EGL
    device->GL_LoadLibrary = Emscripten_GLES_LoadLibrary;
    device->GL_GetProcAddress = Emscripten_GLES_GetProcAddress;
    device->GL_UnloadLibrary = Emscripten_GLES_UnloadLibrary;
    device->GL_CreateContext = Emscripten_GLES_CreateContext;
    device->GL_MakeCurrent = Emscripten_GLES_MakeCurrent;
    device->GL_SetSwapInterval = Emscripten_GLES_SetSwapInterval;
    device->GL_GetSwapInterval = Emscripten_GLES_GetSwapInterval;
    device->GL_SwapWindow = Emscripten_GLES_SwapWindow;
    device->GL_DeleteContext = Emscripten_GLES_DeleteContext;
    device->GL_GetDrawableSize = Emscripten_GLES_GetDrawableSize;
#endif

    device->free = Emscripten_DeleteDevice;

    return device;
}

VideoBootStrap Emscripten_bootstrap = {
    EMSCRIPTENVID_DRIVER_NAME, "SDL emscripten video driver",
    Emscripten_Available, Emscripten_CreateDevice
};


int
Emscripten_VideoInit(_THIS)
{
    SDL_DisplayMode mode;

    /* Use a fake 32-bpp desktop mode */
    mode.format = SDL_PIXELFORMAT_RGB888;

    mode.w = EM_ASM_INT_V({
        return screen.width;
    });

    mode.h = EM_ASM_INT_V({
        return screen.height;
    });

    mode.refresh_rate = 0;
    mode.driverdata = NULL;
    if (SDL_AddBasicVideoDisplay(&mode) < 0) {
        return -1;
    }

    SDL_zero(mode);
    SDL_AddDisplayMode(&_this->displays[0], &mode);

    Emscripten_InitMouse();

    /* We're done! */
    return 0;
}

static int
Emscripten_SetDisplayMode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode)
{
    /* can't do this */
    return 0;
}

static void
Emscripten_VideoQuit(_THIS)
{
    Emscripten_FiniMouse();
}

static void
Emscripten_PumpEvents(_THIS)
{
    /* do nothing. */
}

static int
Emscripten_CreateWindow(_THIS, SDL_Window * window)
{
    SDL_WindowData *wdata;
    double scaled_w, scaled_h;
    double css_w, css_h;

    /* Allocate window internal data */
    wdata = (SDL_WindowData *) SDL_calloc(1, sizeof(SDL_WindowData));
    if (wdata == NULL) {
        return SDL_OutOfMemory();
    }

    if (window->flags & SDL_WINDOW_ALLOW_HIGHDPI) {
        wdata->pixel_ratio = emscripten_get_device_pixel_ratio();
    } else {
        wdata->pixel_ratio = 1.0f;
    }

    scaled_w = SDL_floor(window->w * wdata->pixel_ratio);
    scaled_h = SDL_floor(window->h * wdata->pixel_ratio);

    emscripten_set_canvas_size(scaled_w, scaled_h);

    emscripten_get_element_css_size(NULL, &css_w, &css_h);

    wdata->external_size = SDL_floor(css_w) != scaled_w || SDL_floor(css_h) != scaled_h;

    if ((window->flags & SDL_WINDOW_RESIZABLE) && wdata->external_size) {
        /* external css has resized us */
        scaled_w = css_w * wdata->pixel_ratio;
        scaled_h = css_h * wdata->pixel_ratio;

        emscripten_set_canvas_size(scaled_w, scaled_h);
        SDL_SendWindowEvent(window, SDL_WINDOWEVENT_RESIZED, css_w, css_h);
    }

    /* if the size is not being controlled by css, we need to scale down for hidpi */
    if (!wdata->external_size) {
        if (wdata->pixel_ratio != 1.0f) {
            /*scale canvas down*/
            emscripten_set_element_css_size(NULL, window->w, window->h);
        }
    }

#if SDL_VIDEO_OPENGL_EGL
    if (window->flags & SDL_WINDOW_OPENGL) {
        if (!_this->egl_data) {
            if (SDL_GL_LoadLibrary(NULL) < 0) {
                return -1;
            }
        }
        wdata->egl_surface = SDL_EGL_CreateSurface(_this, 0);

        if (wdata->egl_surface == EGL_NO_SURFACE) {
            return SDL_SetError("Could not create GLES window surface");
        }
    }
#endif

    wdata->window = window;

    /* Setup driver data for this window */
    window->driverdata = wdata;

    /* One window, it always has focus */
    SDL_SetMouseFocus(window);
    SDL_SetKeyboardFocus(window);

    Emscripten_RegisterEventHandlers(wdata);

    /* Window has been successfully created */
    return 0;
}

static void Emscripten_SetWindowSize(_THIS, SDL_Window * window)
{
    SDL_WindowData *data;

    if (window->driverdata) {
        data = (SDL_WindowData *) window->driverdata;
        /* update pixel ratio */
        if (window->flags & SDL_WINDOW_ALLOW_HIGHDPI) {
            data->pixel_ratio = emscripten_get_device_pixel_ratio();
        }
        emscripten_set_canvas_size(window->w * data->pixel_ratio, window->h * data->pixel_ratio);

        /*scale canvas down*/
        if (!data->external_size && data->pixel_ratio != 1.0f) {
            emscripten_set_element_css_size(NULL, window->w, window->h);
        }
    }
}

static void
Emscripten_DestroyWindow(_THIS, SDL_Window * window)
{
    SDL_WindowData *data;

    if(window->driverdata) {
        data = (SDL_WindowData *) window->driverdata;

        Emscripten_UnregisterEventHandlers(data);
#if SDL_VIDEO_OPENGL_EGL
        if (data->egl_surface != EGL_NO_SURFACE) {
            SDL_EGL_DestroySurface(_this, data->egl_surface);
            data->egl_surface = EGL_NO_SURFACE;
        }
#endif
        SDL_free(window->driverdata);
        window->driverdata = NULL;
    }
}

static void
Emscripten_SetWindowFullscreen(_THIS, SDL_Window * window, SDL_VideoDisplay * display, SDL_bool fullscreen)
{
    SDL_WindowData *data;
    if(window->driverdata) {
        data = (SDL_WindowData *) window->driverdata;

        if(fullscreen) {
            EmscriptenFullscreenStrategy strategy;
            SDL_bool is_desktop_fullscreen = (window->flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP;
            int res;

            strategy.scaleMode = is_desktop_fullscreen ? EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH : EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT;

            if(!is_desktop_fullscreen) {
                strategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE;
            } else if(window->flags & SDL_WINDOW_ALLOW_HIGHDPI) {
                strategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF;
            } else {
                strategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF;
            }

            strategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;

            strategy.canvasResizedCallback = Emscripten_HandleCanvasResize;
            strategy.canvasResizedCallbackUserData = data;

            data->requested_fullscreen_mode = window->flags & (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_FULLSCREEN);
            data->fullscreen_resize = is_desktop_fullscreen;

            res = emscripten_request_fullscreen_strategy(NULL, 1, &strategy);
            if(res != EMSCRIPTEN_RESULT_SUCCESS && res != EMSCRIPTEN_RESULT_DEFERRED) {
                /* unset flags, fullscreen failed */
                window->flags &= ~(SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_FULLSCREEN);
            }
        }
        else
            emscripten_exit_fullscreen();
    }
}

static void
Emscripten_SetWindowTitle(_THIS, SDL_Window * window) {
    EM_ASM_INT({
      if (typeof Module['setWindowTitle'] !== 'undefined') {
        Module['setWindowTitle'](Module['Pointer_stringify']($0));
      }
      return 0;
    }, window->title);
}

#endif /* SDL_VIDEO_DRIVER_EMSCRIPTEN */

/* vi: set ts=4 sw=4 expandtab: */
