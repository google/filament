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

#if SDL_VIDEO_DRIVER_WAYLAND && SDL_VIDEO_OPENGL_EGL

#include "SDL_timer.h"
#include "../../core/unix/SDL_poll.h"
#include "../SDL_sysvideo.h"
#include "../../events/SDL_windowevents_c.h"
#include "SDL_waylandvideo.h"
#include "SDL_waylandopengles.h"
#include "SDL_waylandwindow.h"
#include "SDL_waylandevents_c.h"

#include "xdg-shell-client-protocol.h"

/* EGL implementation of SDL OpenGL ES support */

int
Wayland_GLES_LoadLibrary(_THIS, const char *path) {
    int ret;
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;

    ret = SDL_EGL_LoadLibrary(_this, path, (NativeDisplayType) data->display, 0);

    Wayland_PumpEvents(_this);
    WAYLAND_wl_display_flush(data->display);

    return ret;
}


SDL_GLContext
Wayland_GLES_CreateContext(_THIS, SDL_Window * window)
{
    SDL_GLContext context;
    context = SDL_EGL_CreateContext(_this, ((SDL_WindowData *) window->driverdata)->egl_surface);
    WAYLAND_wl_display_flush( ((SDL_VideoData*)_this->driverdata)->display );

    return context;
}

/* Wayland wants to tell you when to provide new frames, and if you have a non-zero
   swap interval, Mesa will block until a callback tells it to do so. On some
   compositors, they might decide that a minimized window _never_ gets a callback,
   which causes apps to hang during swapping forever. So we always set the official
   eglSwapInterval to zero to avoid blocking inside EGL, and manage this ourselves.
   If a swap blocks for too long waiting on a callback, we just go on, under the
   assumption the frame will be wasted, but this is better than freezing the app.
   I frown upon platforms that dictate this sort of control inversion (the callback
   is intended for _rendering_, not stalling until vsync), but we can work around
   this for now.  --ryan. */
/* Addendum: several recent APIs demand this sort of control inversion: Emscripten,
   libretro, Wayland, probably others...it feels like we're eventually going to have
   to give in with a future SDL API revision, since we can bend the other APIs to
   this style, but this style is much harder to bend the other way.  :/ */
int
Wayland_GLES_SetSwapInterval(_THIS, int interval)
{
    if (!_this->egl_data) {
        return SDL_SetError("EGL not initialized");
    }
    
    /* technically, this is _all_ adaptive vsync (-1), because we can't
       actually wait for the _next_ vsync if you set 1, but things that
       request 1 probably won't care _that_ much. I hope. No matter what
       you do, though, you never see tearing on Wayland. */
    if (interval > 1) {
        interval = 1;
    } else if (interval < -1) {
        interval = -1;
    }

    /* !!! FIXME: technically, this should be per-context, right? */
    _this->egl_data->egl_swapinterval = interval;
    _this->egl_data->eglSwapInterval(_this->egl_data->egl_display, 0);
    return 0;
}

int
Wayland_GLES_GetSwapInterval(_THIS)
{
    if (!_this->egl_data) {
        SDL_SetError("EGL not initialized");
        return 0;
    }

    return _this->egl_data->egl_swapinterval;
}

int
Wayland_GLES_SwapWindow(_THIS, SDL_Window *window)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    const int swap_interval = _this->egl_data->egl_swapinterval;

    /* For windows that we know are hidden, skip swaps entirely, if we don't do
     * this compositors will intentionally stall us indefinitely and there's no
     * way for an end user to show the window, unlike other situations (i.e.
     * the window is minimized, behind another window, etc.).
     *
     * FIXME: Request EGL_WAYLAND_swap_buffers_with_timeout.
     * -flibit
     */
    if (window->flags & SDL_WINDOW_HIDDEN) {
        return 0;
    }

    /* Control swap interval ourselves. See comments on Wayland_GLES_SetSwapInterval */
    if (swap_interval != 0) {
        struct wl_display *display = ((SDL_VideoData *)_this->driverdata)->display;
        SDL_VideoDisplay *sdldisplay = SDL_GetDisplayForWindow(window);
        const Uint32 max_wait = SDL_GetTicks() + (10000 / sdldisplay->current_mode.refresh_rate);  /* ~10 frames, so we'll progress even if throttled to zero. */
        while (SDL_AtomicGet(&data->swap_interval_ready) == 0) {
            Uint32 now;

            /* !!! FIXME: this is just the crucial piece of Wayland_PumpEvents */
            WAYLAND_wl_display_flush(display);
            if (WAYLAND_wl_display_dispatch_pending(display) > 0) {
                /* We dispatched some pending events. Check if the frame callback happened. */
                continue;
            }

            now = SDL_GetTicks();
            if (SDL_TICKS_PASSED(now, max_wait)) {
                /* Timeout expired */
                break;
            }

            if (SDL_IOReady(WAYLAND_wl_display_get_fd(display), SDL_FALSE, max_wait - now) <= 0) {
                /* Error or timeout expired without any events for us */
                break;
            }

            WAYLAND_wl_display_dispatch(display);
        }
        SDL_AtomicSet(&data->swap_interval_ready, 0);
    }

    /* Feed the frame to Wayland. This will set it so the wl_surface_frame callback can fire again. */
    if (!_this->egl_data->eglSwapBuffers(_this->egl_data->egl_display, data->egl_surface)) {
        return SDL_EGL_SetError("unable to show color buffer in an OS-native window", "eglSwapBuffers");
    }

    WAYLAND_wl_display_flush( data->waylandData->display );

    return 0;
}

int
Wayland_GLES_MakeCurrent(_THIS, SDL_Window * window, SDL_GLContext context)
{
    int ret;

    if (window && context) {
        ret = SDL_EGL_MakeCurrent(_this, ((SDL_WindowData *) window->driverdata)->egl_surface, context);
    }
    else {
        ret = SDL_EGL_MakeCurrent(_this, NULL, NULL);
    }

    WAYLAND_wl_display_flush( ((SDL_VideoData*)_this->driverdata)->display );

    _this->egl_data->eglSwapInterval(_this->egl_data->egl_display, 0);  /* see comments on Wayland_GLES_SetSwapInterval. */

    return ret;
}

void
Wayland_GLES_GetDrawableSize(_THIS, SDL_Window * window, int * w, int * h)
{
    SDL_WindowData *data;
    if (window->driverdata) {
        data = (SDL_WindowData *) window->driverdata;

        if (w) {
            *w = window->w * data->scale_factor;
        }

        if (h) {
            *h = window->h * data->scale_factor;
        }
    }
}

void
Wayland_GLES_DeleteContext(_THIS, SDL_GLContext context)
{
    SDL_EGL_DeleteContext(_this, context);
    WAYLAND_wl_display_flush( ((SDL_VideoData*)_this->driverdata)->display );
}

#endif /* SDL_VIDEO_DRIVER_WAYLAND && SDL_VIDEO_OPENGL_EGL */

/* vi: set ts=4 sw=4 expandtab: */
