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

#if SDL_VIDEO_DRIVER_WAYLAND && SDL_VIDEO_OPENGL_EGL

#include "../SDL_sysvideo.h"
#include "../../events/SDL_windowevents_c.h"
#include "../SDL_egl_c.h"
#include "SDL_waylandevents_c.h"
#include "SDL_waylandwindow.h"
#include "SDL_waylandvideo.h"
#include "SDL_waylandtouch.h"
#include "SDL_waylanddyn.h"
#include "SDL_hints.h"

#include "xdg-shell-unstable-v6-client-protocol.h"

/* On modern desktops, we probably will use the xdg-shell protocol instead
   of wl_shell, but wl_shell might be useful on older Wayland installs that
   don't have the newer protocol, or embedded things that don't have a full
   window manager. */

static void
handle_ping_wl_shell_surface(void *data, struct wl_shell_surface *shell_surface,
            uint32_t serial)
{
    wl_shell_surface_pong(shell_surface, serial);
}

static void
handle_configure_wl_shell_surface(void *data, struct wl_shell_surface *shell_surface,
                 uint32_t edges, int32_t width, int32_t height)
{
    SDL_WindowData *wind = (SDL_WindowData *)data;
    SDL_Window *window = wind->sdlwindow;
    struct wl_region *region;

    /* wl_shell_surface spec states that this is a suggestion.
       Ignore if less than or greater than max/min size. */

    if (width == 0 || height == 0) {
        return;
    }

    if (!(window->flags & SDL_WINDOW_FULLSCREEN)) {
        if ((window->flags & SDL_WINDOW_RESIZABLE)) {
            if (window->max_w > 0) {
                width = SDL_min(width, window->max_w);
            } 
            width = SDL_max(width, window->min_w);

            if (window->max_h > 0) {
                height = SDL_min(height, window->max_h);
            }
            height = SDL_max(height, window->min_h);
        } else {
            return;
        }
    }

    if (width == window->w && height == window->h) {
        return;
    }

    window->w = width;
    window->h = height;
    WAYLAND_wl_egl_window_resize(wind->egl_window, window->w, window->h, 0, 0);

    region = wl_compositor_create_region(wind->waylandData->compositor);
    wl_region_add(region, 0, 0, window->w, window->h);
    wl_surface_set_opaque_region(wind->surface, region);
    wl_region_destroy(region);
    SDL_SendWindowEvent(window, SDL_WINDOWEVENT_RESIZED, window->w, window->h);
}

static void
handle_popup_done_wl_shell_surface(void *data, struct wl_shell_surface *shell_surface)
{
}

static const struct wl_shell_surface_listener shell_surface_listener_wl = {
    handle_ping_wl_shell_surface,
    handle_configure_wl_shell_surface,
    handle_popup_done_wl_shell_surface
};




static void
handle_configure_zxdg_shell_surface(void *data, struct zxdg_surface_v6 *zxdg, uint32_t serial)
{
    SDL_WindowData *wind = (SDL_WindowData *)data;
    SDL_Window *window = wind->sdlwindow;
    struct wl_region *region;
    WAYLAND_wl_egl_window_resize(wind->egl_window, window->w, window->h, 0, 0);

    region = wl_compositor_create_region(wind->waylandData->compositor);
    wl_region_add(region, 0, 0, window->w, window->h);
    wl_surface_set_opaque_region(wind->surface, region);
    wl_region_destroy(region);
    SDL_SendWindowEvent(window, SDL_WINDOWEVENT_RESIZED, window->w, window->h);
    zxdg_surface_v6_ack_configure(zxdg, serial);
}

static const struct zxdg_surface_v6_listener shell_surface_listener_zxdg = {
    handle_configure_zxdg_shell_surface
};


static void
handle_configure_zxdg_toplevel(void *data,
			  struct zxdg_toplevel_v6 *zxdg_toplevel_v6,
			  int32_t width,
			  int32_t height,
			  struct wl_array *states)
{
    SDL_WindowData *wind = (SDL_WindowData *)data;
    SDL_Window *window = wind->sdlwindow;

    /* wl_shell_surface spec states that this is a suggestion.
       Ignore if less than or greater than max/min size. */

    if (width == 0 || height == 0) {
        return;
    }

    if (!(window->flags & SDL_WINDOW_FULLSCREEN)) {
        if ((window->flags & SDL_WINDOW_RESIZABLE)) {
            if (window->max_w > 0) {
                width = SDL_min(width, window->max_w);
            } 
            width = SDL_max(width, window->min_w);

            if (window->max_h > 0) {
                height = SDL_min(height, window->max_h);
            }
            height = SDL_max(height, window->min_h);
        } else {
            return;
        }
    }

    if (width == window->w && height == window->h) {
        return;
    }

    window->w = width;
    window->h = height;
}

static void
handle_close_zxdg_toplevel(void *data, struct zxdg_toplevel_v6 *zxdg_toplevel_v6)
{
    SDL_WindowData *window = (SDL_WindowData *)data;
    SDL_SendWindowEvent(window->sdlwindow, SDL_WINDOWEVENT_CLOSE, 0, 0);
}

static const struct zxdg_toplevel_v6_listener toplevel_listener_zxdg = {
    handle_configure_zxdg_toplevel,
    handle_close_zxdg_toplevel
};


#ifdef SDL_VIDEO_DRIVER_WAYLAND_QT_TOUCH
static void
handle_onscreen_visibility(void *data,
        struct qt_extended_surface *qt_extended_surface, int32_t visible)
{
}

static void
handle_set_generic_property(void *data,
        struct qt_extended_surface *qt_extended_surface, const char *name,
        struct wl_array *value)
{
}

static void
handle_close(void *data, struct qt_extended_surface *qt_extended_surface)
{
    SDL_WindowData *window = (SDL_WindowData *)data;
    SDL_SendWindowEvent(window->sdlwindow, SDL_WINDOWEVENT_CLOSE, 0, 0);
}

static const struct qt_extended_surface_listener extended_surface_listener = {
    handle_onscreen_visibility,
    handle_set_generic_property,
    handle_close,
};
#endif /* SDL_VIDEO_DRIVER_WAYLAND_QT_TOUCH */

SDL_bool
Wayland_GetWindowWMInfo(_THIS, SDL_Window * window, SDL_SysWMinfo * info)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    const Uint32 version = ((((Uint32) info->version.major) * 1000000) +
                            (((Uint32) info->version.minor) * 10000) +
                            (((Uint32) info->version.patch)));

    /* Before 2.0.6, it was possible to build an SDL with Wayland support
       (SDL_SysWMinfo will be large enough to hold Wayland info), but build
       your app against SDL headers that didn't have Wayland support
       (SDL_SysWMinfo could be smaller than Wayland needs. This would lead
       to an app properly using SDL_GetWindowWMInfo() but we'd accidentally
       overflow memory on the stack or heap. To protect against this, we've
       padded out the struct unconditionally in the headers and Wayland will
       just return an error for older apps using this function. Those apps
       will need to be recompiled against newer headers or not use Wayland,
       maybe by forcing SDL_VIDEODRIVER=x11. */
    if (version < 2000006) {
        info->subsystem = SDL_SYSWM_UNKNOWN;
        SDL_SetError("Version must be 2.0.6 or newer");
        return SDL_FALSE;
    }

    info->info.wl.display = data->waylandData->display;
    info->info.wl.surface = data->surface;
    info->info.wl.shell_surface = data->shell_surface.wl;
    info->subsystem = SDL_SYSWM_WAYLAND;

    return SDL_TRUE;
}

int
Wayland_SetWindowHitTest(SDL_Window *window, SDL_bool enabled)
{
    return 0;  /* just succeed, the real work is done elsewhere. */
}

static void
SetFullscreen(_THIS, SDL_Window * window, struct wl_output *output)
{
    const SDL_VideoData *viddata = (const SDL_VideoData *) _this->driverdata;
    SDL_WindowData *wind = window->driverdata;

    if (viddata->shell.zxdg) {
        if (output) {
            zxdg_toplevel_v6_set_fullscreen(wind->shell_surface.zxdg.roleobj.toplevel, output);
        } else {
            zxdg_toplevel_v6_unset_fullscreen(wind->shell_surface.zxdg.roleobj.toplevel);
        }
    } else {
        if (output) {
            wl_shell_surface_set_fullscreen(wind->shell_surface.wl,
                                            WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT,
                                            0, output);
        } else {
            wl_shell_surface_set_toplevel(wind->shell_surface.wl);
        }
    }

    WAYLAND_wl_display_flush( ((SDL_VideoData*)_this->driverdata)->display );
}

void Wayland_ShowWindow(_THIS, SDL_Window *window)
{
    struct wl_output *output = (struct wl_output *) window->fullscreen_mode.driverdata;
    SetFullscreen(_this, window, (window->flags & SDL_WINDOW_FULLSCREEN) ? output : NULL);
}

#ifdef SDL_VIDEO_DRIVER_WAYLAND_QT_TOUCH
static void SDLCALL
QtExtendedSurface_OnHintChanged(void *userdata, const char *name,
        const char *oldValue, const char *newValue)
{
    struct qt_extended_surface *qt_extended_surface = userdata;

    if (name == NULL) {
        return;
    }

    if (strcmp(name, SDL_HINT_QTWAYLAND_CONTENT_ORIENTATION) == 0) {
        int32_t orientation = QT_EXTENDED_SURFACE_ORIENTATION_PRIMARYORIENTATION;

        if (newValue != NULL) {
            if (strcmp(newValue, "portrait") == 0) {
                orientation = QT_EXTENDED_SURFACE_ORIENTATION_PORTRAITORIENTATION;
            } else if (strcmp(newValue, "landscape") == 0) {
                orientation = QT_EXTENDED_SURFACE_ORIENTATION_LANDSCAPEORIENTATION;
            } else if (strcmp(newValue, "inverted-portrait") == 0) {
                orientation = QT_EXTENDED_SURFACE_ORIENTATION_INVERTEDPORTRAITORIENTATION;
            } else if (strcmp(newValue, "inverted-landscape") == 0) {
                orientation = QT_EXTENDED_SURFACE_ORIENTATION_INVERTEDLANDSCAPEORIENTATION;
            }
        }

        qt_extended_surface_set_content_orientation(qt_extended_surface, orientation);
    } else if (strcmp(name, SDL_HINT_QTWAYLAND_WINDOW_FLAGS) == 0) {
        uint32_t flags = 0;

        if (newValue != NULL) {
            char *tmp = strdup(newValue);
            char *saveptr = NULL;

            char *flag = strtok_r(tmp, " ", &saveptr);
            while (flag) {
                if (strcmp(flag, "OverridesSystemGestures") == 0) {
                    flags |= QT_EXTENDED_SURFACE_WINDOWFLAG_OVERRIDESSYSTEMGESTURES;
                } else if (strcmp(flag, "StaysOnTop") == 0) {
                    flags |= QT_EXTENDED_SURFACE_WINDOWFLAG_STAYSONTOP;
                } else if (strcmp(flag, "BypassWindowManager") == 0) {
                    // See https://github.com/qtproject/qtwayland/commit/fb4267103d
                    flags |= 4 /* QT_EXTENDED_SURFACE_WINDOWFLAG_BYPASSWINDOWMANAGER */;
                }

                flag = strtok_r(NULL, " ", &saveptr);
            }

            free(tmp);
        }

        qt_extended_surface_set_window_flags(qt_extended_surface, flags);
    }
}

static void QtExtendedSurface_Subscribe(struct qt_extended_surface *surface, const char *name)
{
    SDL_AddHintCallback(name, QtExtendedSurface_OnHintChanged, surface);
}

static void QtExtendedSurface_Unsubscribe(struct qt_extended_surface *surface, const char *name)
{
    SDL_DelHintCallback(name, QtExtendedSurface_OnHintChanged, surface);
}
#endif /* SDL_VIDEO_DRIVER_WAYLAND_QT_TOUCH */

void
Wayland_SetWindowFullscreen(_THIS, SDL_Window * window,
                            SDL_VideoDisplay * _display, SDL_bool fullscreen)
{
    struct wl_output *output = (struct wl_output *) _display->driverdata;
    SetFullscreen(_this, window, fullscreen ? output : NULL);
}

void
Wayland_RestoreWindow(_THIS, SDL_Window * window)
{
    SDL_WindowData *wind = window->driverdata;
    const SDL_VideoData *viddata = (const SDL_VideoData *) _this->driverdata;

    if (viddata->shell.zxdg) {
    } else {
        wl_shell_surface_set_toplevel(wind->shell_surface.wl);
    }

    WAYLAND_wl_display_flush( ((SDL_VideoData*)_this->driverdata)->display );
}

void
Wayland_MaximizeWindow(_THIS, SDL_Window * window)
{
    SDL_WindowData *wind = window->driverdata;
    SDL_VideoData *viddata = (SDL_VideoData *) _this->driverdata;

    if (viddata->shell.zxdg) {
        zxdg_toplevel_v6_set_maximized(wind->shell_surface.zxdg.roleobj.toplevel);
    } else {
        wl_shell_surface_set_maximized(wind->shell_surface.wl, NULL);
    }

    WAYLAND_wl_display_flush( viddata->display );
}

int Wayland_CreateWindow(_THIS, SDL_Window *window)
{
    SDL_WindowData *data;
    SDL_VideoData *c;
    struct wl_region *region;

    data = calloc(1, sizeof *data);
    if (data == NULL)
        return SDL_OutOfMemory();

    c = _this->driverdata;
    window->driverdata = data;

    if (!(window->flags & SDL_WINDOW_OPENGL)) {
        SDL_GL_LoadLibrary(NULL);
        window->flags |= SDL_WINDOW_OPENGL;
    }

    if (window->x == SDL_WINDOWPOS_UNDEFINED) {
        window->x = 0;
    }
    if (window->y == SDL_WINDOWPOS_UNDEFINED) {
        window->y = 0;
    }

    data->waylandData = c;
    data->sdlwindow = window;

    data->surface =
        wl_compositor_create_surface(c->compositor);
    wl_surface_set_user_data(data->surface, data);

    if (c->shell.zxdg) {
        data->shell_surface.zxdg.surface = zxdg_shell_v6_get_xdg_surface(c->shell.zxdg, data->surface);
        /* !!! FIXME: add popup role */
        data->shell_surface.zxdg.roleobj.toplevel = zxdg_surface_v6_get_toplevel(data->shell_surface.zxdg.surface);
        zxdg_toplevel_v6_add_listener(data->shell_surface.zxdg.roleobj.toplevel, &toplevel_listener_zxdg, data);
        zxdg_toplevel_v6_set_app_id(data->shell_surface.zxdg.roleobj.toplevel, c->classname);
    } else {
        data->shell_surface.wl = wl_shell_get_shell_surface(c->shell.wl, data->surface);
        wl_shell_surface_set_class(data->shell_surface.wl, c->classname);
    }

#ifdef SDL_VIDEO_DRIVER_WAYLAND_QT_TOUCH
    if (c->surface_extension) {
        data->extended_surface = qt_surface_extension_get_extended_surface(
                c->surface_extension, data->surface);

        QtExtendedSurface_Subscribe(data->extended_surface, SDL_HINT_QTWAYLAND_CONTENT_ORIENTATION);
        QtExtendedSurface_Subscribe(data->extended_surface, SDL_HINT_QTWAYLAND_WINDOW_FLAGS);
    }
#endif /* SDL_VIDEO_DRIVER_WAYLAND_QT_TOUCH */

    data->egl_window = WAYLAND_wl_egl_window_create(data->surface,
                                            window->w, window->h);

    /* Create the GLES window surface */
    data->egl_surface = SDL_EGL_CreateSurface(_this, (NativeWindowType) data->egl_window);
    
    if (data->egl_surface == EGL_NO_SURFACE) {
        return SDL_SetError("failed to create a window surface");
    }

    if (c->shell.zxdg) {
        if (data->shell_surface.zxdg.surface) {
            zxdg_surface_v6_set_user_data(data->shell_surface.zxdg.surface, data);
            zxdg_surface_v6_add_listener(data->shell_surface.zxdg.surface, &shell_surface_listener_zxdg, data);
        }
    } else {
        if (data->shell_surface.wl) {
            wl_shell_surface_set_user_data(data->shell_surface.wl, data);
            wl_shell_surface_add_listener(data->shell_surface.wl, &shell_surface_listener_wl, data);
        }
    }

#ifdef SDL_VIDEO_DRIVER_WAYLAND_QT_TOUCH
    if (data->extended_surface) {
        qt_extended_surface_set_user_data(data->extended_surface, data);
        qt_extended_surface_add_listener(data->extended_surface,
                                         &extended_surface_listener, data);
    }
#endif /* SDL_VIDEO_DRIVER_WAYLAND_QT_TOUCH */

    region = wl_compositor_create_region(c->compositor);
    wl_region_add(region, 0, 0, window->w, window->h);
    wl_surface_set_opaque_region(data->surface, region);
    wl_region_destroy(region);

    if (c->relative_mouse_mode) {
        Wayland_input_lock_pointer(c->input);
    }

    wl_surface_commit(data->surface);
    WAYLAND_wl_display_flush(c->display);

    return 0;
}

void Wayland_SetWindowSize(_THIS, SDL_Window * window)
{
    SDL_VideoData *data = _this->driverdata;
    SDL_WindowData *wind = window->driverdata;
    struct wl_region *region;

    WAYLAND_wl_egl_window_resize(wind->egl_window, window->w, window->h, 0, 0);

    region =wl_compositor_create_region(data->compositor);
    wl_region_add(region, 0, 0, window->w, window->h);
    wl_surface_set_opaque_region(wind->surface, region);
    wl_region_destroy(region);
}

void Wayland_SetWindowTitle(_THIS, SDL_Window * window)
{
    SDL_WindowData *wind = window->driverdata;
    SDL_VideoData *viddata = (SDL_VideoData *) _this->driverdata;
    
    if (window->title != NULL) {
        if (viddata->shell.zxdg) {
            zxdg_toplevel_v6_set_title(wind->shell_surface.zxdg.roleobj.toplevel, window->title);
        } else {
            wl_shell_surface_set_title(wind->shell_surface.wl, window->title);
        }
    }

    WAYLAND_wl_display_flush( ((SDL_VideoData*)_this->driverdata)->display );
}

void Wayland_DestroyWindow(_THIS, SDL_Window *window)
{
    SDL_VideoData *data = _this->driverdata;
    SDL_WindowData *wind = window->driverdata;

    if (data) {
        SDL_EGL_DestroySurface(_this, wind->egl_surface);
        WAYLAND_wl_egl_window_destroy(wind->egl_window);

        if (data->shell.zxdg) {
            if (wind->shell_surface.zxdg.roleobj.toplevel) {
                zxdg_toplevel_v6_destroy(wind->shell_surface.zxdg.roleobj.toplevel);
            }
            if (wind->shell_surface.zxdg.surface) {
                zxdg_surface_v6_destroy(wind->shell_surface.zxdg.surface);
            }
        } else {
            if (wind->shell_surface.wl) {
                wl_shell_surface_destroy(wind->shell_surface.wl);
            }
        }

#ifdef SDL_VIDEO_DRIVER_WAYLAND_QT_TOUCH
        if (wind->extended_surface) {
            QtExtendedSurface_Unsubscribe(wind->extended_surface, SDL_HINT_QTWAYLAND_CONTENT_ORIENTATION);
            QtExtendedSurface_Unsubscribe(wind->extended_surface, SDL_HINT_QTWAYLAND_WINDOW_FLAGS);
            qt_extended_surface_destroy(wind->extended_surface);
        }
#endif /* SDL_VIDEO_DRIVER_WAYLAND_QT_TOUCH */
        wl_surface_destroy(wind->surface);

        SDL_free(wind);
        WAYLAND_wl_display_flush(data->display);
    }
    window->driverdata = NULL;
}

#endif /* SDL_VIDEO_DRIVER_WAYLAND && SDL_VIDEO_OPENGL_EGL */

/* vi: set ts=4 sw=4 expandtab: */
