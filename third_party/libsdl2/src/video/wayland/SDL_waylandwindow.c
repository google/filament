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

#include "../SDL_sysvideo.h"
#include "../../events/SDL_windowevents_c.h"
#include "../SDL_egl_c.h"
#include "SDL_waylandevents_c.h"
#include "SDL_waylandwindow.h"
#include "SDL_waylandvideo.h"
#include "SDL_waylandtouch.h"
#include "SDL_hints.h"

#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-unstable-v1-client-protocol.h"
#include "idle-inhibit-unstable-v1-client-protocol.h"
#include "xdg-activation-v1-client-protocol.h"

#ifdef HAVE_LIBDECOR_H
#include <libdecor.h>
#endif

static void
CommitMinMaxDimensions(SDL_Window *window)
{
    SDL_WindowData *wind = window->driverdata;
    SDL_VideoData *viddata = wind->waylandData;
    int min_width, min_height, max_width, max_height;

    if (window->flags & SDL_WINDOW_FULLSCREEN) {
        min_width = 0;
        min_height = 0;
        max_width = 0;
        max_height = 0;
    } else if (window->flags & SDL_WINDOW_RESIZABLE) {
        min_width = window->min_w;
        min_height = window->min_h;
        max_width = window->max_w;
        max_height = window->max_h;
    } else {
        min_width = window->windowed.w;
        min_height = window->windowed.h;
        max_width = window->windowed.w;
        max_height = window->windowed.h;
    }

#ifdef HAVE_LIBDECOR_H
    if (viddata->shell.libdecor) {
        if (wind->shell_surface.libdecor.frame == NULL) {
            return; /* Can't do anything yet, wait for ShowWindow */
        }
        libdecor_frame_set_min_content_size(wind->shell_surface.libdecor.frame,
                                            min_width,
                                            min_height);
        libdecor_frame_set_max_content_size(wind->shell_surface.libdecor.frame,
                                            max_width,
                                            max_height);
    } else
#endif
    if (viddata->shell.xdg) {
        if (wind->shell_surface.xdg.roleobj.toplevel == NULL) {
            return; /* Can't do anything yet, wait for ShowWindow */
        }
        xdg_toplevel_set_min_size(wind->shell_surface.xdg.roleobj.toplevel,
                                  min_width,
                                  min_height);
        xdg_toplevel_set_max_size(wind->shell_surface.xdg.roleobj.toplevel,
                                  max_width,
                                  max_height);
        wl_surface_commit(wind->surface);
    }
}

static void
SetFullscreen(SDL_Window *window, struct wl_output *output)
{
    SDL_WindowData *wind = window->driverdata;
    SDL_VideoData *viddata = wind->waylandData;

    /* The desktop may try to enforce min/max sizes here, so turn them off for
     * fullscreen and on (if applicable) for windowed
     */
    CommitMinMaxDimensions(window);

#ifdef HAVE_LIBDECOR_H
    if (viddata->shell.libdecor) {
        if (wind->shell_surface.libdecor.frame == NULL) {
            return; /* Can't do anything yet, wait for ShowWindow */
        }
        if (output) {
            if (!(window->flags & SDL_WINDOW_RESIZABLE)) {
                /* ensure that window is resizable before going into fullscreen */
                libdecor_frame_set_capabilities(wind->shell_surface.libdecor.frame, LIBDECOR_ACTION_RESIZE);
                wl_surface_commit(wind->surface);
            }
            libdecor_frame_set_fullscreen(wind->shell_surface.libdecor.frame, output);
        } else {
            libdecor_frame_unset_fullscreen(wind->shell_surface.libdecor.frame);
            if (!(window->flags & SDL_WINDOW_RESIZABLE)) {
                /* restore previous RESIZE capability */
                libdecor_frame_unset_capabilities(wind->shell_surface.libdecor.frame, LIBDECOR_ACTION_RESIZE);
                wl_surface_commit(wind->surface);
            }
        }
    } else
#endif
    if (viddata->shell.xdg) {
        if (wind->shell_surface.xdg.roleobj.toplevel == NULL) {
            return; /* Can't do anything yet, wait for ShowWindow */
        }
        if (output) {
            xdg_toplevel_set_fullscreen(wind->shell_surface.xdg.roleobj.toplevel, output);
        } else {
            xdg_toplevel_unset_fullscreen(wind->shell_surface.xdg.roleobj.toplevel);
        }
    }
}

static const struct wl_callback_listener surface_frame_listener;

static void
handle_surface_frame_done(void *data, struct wl_callback *cb, uint32_t time)
{
    SDL_WindowData *wind = (SDL_WindowData *) data;
    SDL_AtomicSet(&wind->swap_interval_ready, 1);  /* mark window as ready to present again. */

    /* reset this callback to fire again once a new frame was presented and compositor wants the next one. */
    wind->frame_callback = wl_surface_frame(wind->surface);
    wl_callback_destroy(cb);
    wl_callback_add_listener(wind->frame_callback, &surface_frame_listener, data);
}

static const struct wl_callback_listener surface_frame_listener = {
    handle_surface_frame_done
};


static void Wayland_HandleResize(SDL_Window *window, int width, int height, float scale);

static void
handle_configure_xdg_shell_surface(void *data, struct xdg_surface *xdg, uint32_t serial)
{
    SDL_WindowData *wind = (SDL_WindowData *)data;
    SDL_Window *window = wind->sdlwindow;

    Wayland_HandleResize(window, window->w, window->h, wind->scale_factor);
    xdg_surface_ack_configure(xdg, serial);

    wind->shell_surface.xdg.initial_configure_seen = SDL_TRUE;
}

static const struct xdg_surface_listener shell_surface_listener_xdg = {
    handle_configure_xdg_shell_surface
};

static void
handle_configure_xdg_toplevel(void *data,
              struct xdg_toplevel *xdg_toplevel,
              int32_t width,
              int32_t height,
              struct wl_array *states)
{
    SDL_WindowData *wind = (SDL_WindowData *)data;
    SDL_Window *window = wind->sdlwindow;
    SDL_WaylandOutputData *driverdata;

    enum xdg_toplevel_state *state;
    SDL_bool fullscreen = SDL_FALSE;
    SDL_bool maximized = SDL_FALSE;
    SDL_bool floating = SDL_TRUE;
    wl_array_for_each(state, states) {
        switch (*state) {
        case XDG_TOPLEVEL_STATE_FULLSCREEN:
            fullscreen = SDL_TRUE;
            floating = SDL_FALSE;
            break;
        case XDG_TOPLEVEL_STATE_MAXIMIZED:
            maximized = SDL_TRUE;
            floating = SDL_FALSE;
            break;
        case XDG_TOPLEVEL_STATE_TILED_LEFT:
        case XDG_TOPLEVEL_STATE_TILED_RIGHT:
        case XDG_TOPLEVEL_STATE_TILED_TOP:
        case XDG_TOPLEVEL_STATE_TILED_BOTTOM:
            floating = SDL_FALSE;
            break;
        default:
            break;
        }
    }

    driverdata = (SDL_WaylandOutputData *) SDL_GetDisplayForWindow(window)->driverdata;

    if (!fullscreen) {
        if (window->flags & SDL_WINDOW_FULLSCREEN) {
            /* We might need to re-enter fullscreen after being restored from minimized */
            SetFullscreen(window, driverdata->output);

            /* Foolishly do what the compositor says here. If it's wrong, don't
             * blame us, we were explicitly instructed to do this.
             */
            window->w = width;
            window->h = height;

            /* This part is good though. */
            if (window->flags & SDL_WINDOW_ALLOW_HIGHDPI) {
                wind->scale_factor = driverdata->scale_factor;
            }

            return;
        }

        if (width == 0 || height == 0) {
            /* This usually happens when we're being restored from a
             * non-floating state, so use the cached floating size here.
             */
            width = wind->floating_width;
            height = wind->floating_height;
        }

        /* xdg_toplevel spec states that this is a suggestion.
           Ignore if less than or greater than max/min size. */

        if ((window->flags & SDL_WINDOW_RESIZABLE)) {
            if (window->max_w > 0) {
                width = SDL_min(width, window->max_w);
            }
            width = SDL_max(width, window->min_w);

            if (window->max_h > 0) {
                height = SDL_min(height, window->max_h);
            }
            height = SDL_max(height, window->min_h);
        } else if (floating) {
            /* If we're a fixed-size window, we know our size for sure.
             * Always assume the configure is wrong.
             */
            width = window->windowed.w;
            height = window->windowed.h;
        }

        /* Always send a maximized/restore event; if the event is redundant it will
         * automatically be discarded (see src/events/SDL_windowevents.c)
         *
         * No, we do not get minimize events from xdg-shell.
         */
        SDL_SendWindowEvent(window,
                            maximized ?
                                SDL_WINDOWEVENT_MAXIMIZED :
                                SDL_WINDOWEVENT_RESTORED,
                            0, 0);

        /* Store current floating dimensions for restoring */
        if (floating) {
            wind->floating_width = width;
            wind->floating_height = height;
        }

        /* Store this now so the xdg_surface configure knows what to resize to */
        window->w = width;
        window->h = height;
    } else {
        /* For fullscreen, foolishly do what the compositor says. If it's wrong,
         * don't blame us, we were explicitly instructed to do this.
         */
        window->w = width;
        window->h = height;

        /* This part is good though. */
        if (window->flags & SDL_WINDOW_ALLOW_HIGHDPI) {
            wind->scale_factor = driverdata->scale_factor;
        }
    }
}

static void
handle_close_xdg_toplevel(void *data, struct xdg_toplevel *xdg_toplevel)
{
    SDL_WindowData *window = (SDL_WindowData *)data;
    SDL_SendWindowEvent(window->sdlwindow, SDL_WINDOWEVENT_CLOSE, 0, 0);
}

static const struct xdg_toplevel_listener toplevel_listener_xdg = {
    handle_configure_xdg_toplevel,
    handle_close_xdg_toplevel
};

#ifdef HAVE_LIBDECOR_H
static void
decoration_frame_configure(struct libdecor_frame *frame,
                           struct libdecor_configuration *configuration,
                           void *user_data)
{
    SDL_WindowData *wind = (SDL_WindowData *)user_data;
    SDL_Window *window = wind->sdlwindow;
    SDL_WaylandOutputData *driverdata;
    struct libdecor_state *state;

    enum libdecor_window_state window_state;
    int width, height;

    SDL_bool focused = SDL_FALSE;
    SDL_bool fullscreen = SDL_FALSE;
    SDL_bool maximized = SDL_FALSE;
    SDL_bool tiled = SDL_FALSE;
    SDL_bool floating;

    static const enum libdecor_window_state tiled_states = (
        LIBDECOR_WINDOW_STATE_TILED_LEFT | LIBDECOR_WINDOW_STATE_TILED_RIGHT |
        LIBDECOR_WINDOW_STATE_TILED_TOP | LIBDECOR_WINDOW_STATE_TILED_BOTTOM
    );

    /* Window State */
    if (libdecor_configuration_get_window_state(configuration, &window_state)) {
        fullscreen = (window_state & LIBDECOR_WINDOW_STATE_FULLSCREEN) != 0;
        maximized = (window_state & LIBDECOR_WINDOW_STATE_MAXIMIZED) != 0;
        focused = (window_state & LIBDECOR_WINDOW_STATE_ACTIVE) != 0;
        tiled = (window_state & tiled_states) != 0;
    }
    floating = !(fullscreen || maximized || tiled);

    driverdata = (SDL_WaylandOutputData *) SDL_GetDisplayForWindow(window)->driverdata;

    if (!fullscreen) {
        if (window->flags & SDL_WINDOW_FULLSCREEN) {
            /* We might need to re-enter fullscreen after being restored from minimized */
            SetFullscreen(window, driverdata->output);
            fullscreen = SDL_TRUE;
            floating = SDL_FALSE;
        }

        /* Always send a maximized/restore event; if the event is redundant it will
         * automatically be discarded (see src/events/SDL_windowevents.c)
         *
         * No, we do not get minimize events from libdecor.
         */
        if (!fullscreen) {
            SDL_SendWindowEvent(window,
                                maximized ?
                                    SDL_WINDOWEVENT_MAXIMIZED :
                                    SDL_WINDOWEVENT_RESTORED,
                                0, 0);
        }
    }

    /* Similar to maximized/restore events above, send focus events too! */
    SDL_SendWindowEvent(window,
                        focused ?
                            SDL_WINDOWEVENT_FOCUS_GAINED :
                            SDL_WINDOWEVENT_FOCUS_LOST,
                        0, 0);

    /* For fullscreen or fixed-size windows we know our size.
     * Always assume the configure is wrong.
     */
    if (fullscreen) {
        /* FIXME: We have been explicitly told to respect the fullscreen size
         * parameters here, even though they are known to be wrong on GNOME at
         * bare minimum. If this is wrong, don't blame us, we were explicitly
         * told to do this.
         */
        if (!libdecor_configuration_get_content_size(configuration, frame,
                                                     &width, &height)) {
            width = window->w;
            height = window->h;
        }

        /* This part is good though. */
        if (window->flags & SDL_WINDOW_ALLOW_HIGHDPI) {
            wind->scale_factor = driverdata->scale_factor;
        }
    } else if (!(window->flags & SDL_WINDOW_RESIZABLE)) {
        width = window->windowed.w;
        height = window->windowed.h;
    } else {
        /* This will never set 0 for width/height unless the function returns false */
        if (!libdecor_configuration_get_content_size(configuration, frame, &width, &height)) {
            if (floating) {
                /* This usually happens when we're being restored from a
                 * non-floating state, so use the cached floating size here.
                 */
                width = wind->floating_width;
                height = wind->floating_height;
            } else {
                width = window->w;
                height = window->h;
            }
        }
    }

    /* Store current floating dimensions for restoring */
    if (floating) {
        wind->floating_width = width;
        wind->floating_height = height;
    }

    /* Do the resize on the SDL side (this will set window->w/h)... */
    Wayland_HandleResize(window, width, height, wind->scale_factor);
    wind->shell_surface.libdecor.initial_configure_seen = SDL_TRUE;

    /* ... then commit the changes on the libdecor side. */
    state = libdecor_state_new(width, height);
    libdecor_frame_commit(frame, state, configuration);
    libdecor_state_free(state);

    /* Update the resize capability. Since this will change the capabilities and
     * commit a new frame state with the last known content dimension, this has
     * to be called after the new state has been commited and the new content
     * dimensions were updated.
     */
    Wayland_SetWindowResizable(SDL_GetVideoDevice(), window,
                               window->flags & SDL_WINDOW_RESIZABLE);
}

static void
decoration_frame_close(struct libdecor_frame *frame, void *user_data)
{
    SDL_SendWindowEvent(((SDL_WindowData *)user_data)->sdlwindow, SDL_WINDOWEVENT_CLOSE, 0, 0);
}

static void
decoration_frame_commit(struct libdecor_frame *frame, void *user_data)
{
    SDL_WindowData *wind = user_data;

    SDL_SendWindowEvent(wind->sdlwindow, SDL_WINDOWEVENT_EXPOSED, 0, 0);
}

static struct libdecor_frame_interface libdecor_frame_interface = {
    decoration_frame_configure,
    decoration_frame_close,
    decoration_frame_commit,
};
#endif

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

static void
update_scale_factor(SDL_WindowData *window)
{
    float old_factor = window->scale_factor;
    float new_factor;
    int i;

    if (!(window->sdlwindow->flags & SDL_WINDOW_ALLOW_HIGHDPI)) {
        /* Scale will always be 1, just ignore this */
        return;
    }

    if (FULLSCREEN_VISIBLE(window->sdlwindow)) {
        /* For fullscreen, use the active display's scale factor */
        SDL_VideoDisplay *display = SDL_GetDisplayForWindow(window->sdlwindow);
        SDL_WaylandOutputData* driverdata = display->driverdata;
        new_factor = driverdata->scale_factor;
    } else if (window->num_outputs == 0) {
        /* No monitor (somehow)? Just fall back. */
        new_factor = old_factor;
    } else {
        /* Check every display's factor, use the highest */
        new_factor = 0.0f;
        for (i = 0; i < window->num_outputs; i++) {
            SDL_WaylandOutputData* driverdata = window->outputs[i];
            if (driverdata->scale_factor > new_factor) {
                new_factor = driverdata->scale_factor;
            }
        }
    }

    if (new_factor != old_factor) {
        Wayland_HandleResize(window->sdlwindow, window->sdlwindow->w, window->sdlwindow->h, new_factor);
    }
}

/* While we can't get window position from the compositor, we do at least know
 * what monitor we're on, so let's send move events that put the window at the
 * center of the whatever display the wl_surface_listener events give us.
 */
static void
Wayland_move_window(SDL_Window *window,
                    SDL_WaylandOutputData *driverdata)
{
    int i, numdisplays = SDL_GetNumVideoDisplays();
    for (i = 0; i < numdisplays; i += 1) {
        if (SDL_GetDisplay(i)->driverdata == driverdata) {
            SDL_SendWindowEvent(window, SDL_WINDOWEVENT_MOVED,
                                SDL_WINDOWPOS_CENTERED_DISPLAY(i),
                                SDL_WINDOWPOS_CENTERED_DISPLAY(i));
            break;
        }
    }
}

static void
handle_surface_enter(void *data, struct wl_surface *surface,
                     struct wl_output *output)
{
    SDL_WindowData *window = data;
    SDL_WaylandOutputData *driverdata = wl_output_get_user_data(output);

    if (!SDL_WAYLAND_own_output(output) || !SDL_WAYLAND_own_surface(surface)) {
        return;
    }

    window->outputs = SDL_realloc(window->outputs,
                                  sizeof(SDL_WaylandOutputData*) * (window->num_outputs + 1));
    window->outputs[window->num_outputs++] = driverdata;
    update_scale_factor(window);

    Wayland_move_window(window->sdlwindow, driverdata);
}

static void
handle_surface_leave(void *data, struct wl_surface *surface,
                     struct wl_output *output)
{
    SDL_WindowData *window = data;
    int i, send_move_event = 0;
    SDL_WaylandOutputData *driverdata = wl_output_get_user_data(output);

    if (!SDL_WAYLAND_own_output(output) || !SDL_WAYLAND_own_surface(surface)) {
        return;
    }

    for (i = 0; i < window->num_outputs; i++) {
        if (window->outputs[i] == driverdata) {  /* remove this one */
            if (i == (window->num_outputs-1)) {
                window->outputs[i] = NULL;
                send_move_event = 1;
            } else {
                SDL_memmove(&window->outputs[i],
                            &window->outputs[i + 1],
                            sizeof(SDL_WaylandOutputData*) * ((window->num_outputs - i) - 1));
            }
            window->num_outputs--;
            i--;
        }
    }

    if (window->num_outputs == 0) {
        SDL_free(window->outputs);
        window->outputs = NULL;
    } else if (send_move_event) {
        Wayland_move_window(window->sdlwindow,
                            window->outputs[window->num_outputs - 1]);
    }

    update_scale_factor(window);
}

static const struct wl_surface_listener surface_listener = {
    handle_surface_enter,
    handle_surface_leave
};

SDL_bool
Wayland_GetWindowWMInfo(_THIS, SDL_Window * window, SDL_SysWMinfo * info)
{
    SDL_VideoData *viddata = (SDL_VideoData *) _this->driverdata;
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    const Uint32 version = SDL_VERSIONNUM((Uint32)info->version.major,
                                          (Uint32)info->version.minor,
                                          (Uint32)info->version.patch);

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
    if (version < SDL_VERSIONNUM(2, 0, 6)) {
        info->subsystem = SDL_SYSWM_UNKNOWN;
        SDL_SetError("Version must be 2.0.6 or newer");
        return SDL_FALSE;
    }

    info->info.wl.display = data->waylandData->display;
    info->info.wl.surface = data->surface;

    if (version >= SDL_VERSIONNUM(2, 0, 15)) {
        info->info.wl.egl_window = data->egl_window;

#ifdef HAVE_LIBDECOR_H
        if (viddata->shell.libdecor && data->shell_surface.libdecor.frame != NULL) {
            info->info.wl.xdg_surface = libdecor_frame_get_xdg_surface(data->shell_surface.libdecor.frame);
        } else
#endif
        if (viddata->shell.xdg && data->shell_surface.xdg.surface != NULL) {
            info->info.wl.xdg_surface = data->shell_surface.xdg.surface;
        } else {
            info->info.wl.xdg_surface = NULL;
        }
    }

    /* Deprecated in 2.0.16 */
    info->info.wl.shell_surface = NULL;

    info->subsystem = SDL_SYSWM_WAYLAND;

    return SDL_TRUE;
}

int
Wayland_SetWindowHitTest(SDL_Window *window, SDL_bool enabled)
{
    return 0;  /* just succeed, the real work is done elsewhere. */
}

int
Wayland_SetWindowModalFor(_THIS, SDL_Window *modal_window, SDL_Window *parent_window)
{
    SDL_VideoData *viddata = (SDL_VideoData *) _this->driverdata;
    SDL_WindowData *modal_data = modal_window->driverdata;
    SDL_WindowData *parent_data = parent_window->driverdata;

#ifdef HAVE_LIBDECOR_H
    if (viddata->shell.libdecor) {
        if (modal_data->shell_surface.libdecor.frame == NULL) {
            return SDL_SetError("Modal window was hidden");
        }
        if (parent_data->shell_surface.libdecor.frame == NULL) {
            return SDL_SetError("Parent window was hidden");
        }
        libdecor_frame_set_parent(modal_data->shell_surface.libdecor.frame,
                                  parent_data->shell_surface.libdecor.frame);
    } else
#endif
    if (viddata->shell.xdg) {
        if (modal_data->shell_surface.xdg.roleobj.toplevel == NULL) {
            return SDL_SetError("Modal window was hidden");
        }
        if (parent_data->shell_surface.xdg.roleobj.toplevel == NULL) {
            return SDL_SetError("Parent window was hidden");
        }
        xdg_toplevel_set_parent(modal_data->shell_surface.xdg.roleobj.toplevel,
                                parent_data->shell_surface.xdg.roleobj.toplevel);
    } else {
        return SDL_Unsupported();
    }

    WAYLAND_wl_display_flush(viddata->display);
    return 0;
}

void Wayland_ShowWindow(_THIS, SDL_Window *window)
{
    SDL_VideoData *c = _this->driverdata;
    SDL_WindowData *data = window->driverdata;
    SDL_VideoDisplay *display = SDL_GetDisplayForWindow(window);

    /* Detach any previous buffers before resetting everything, otherwise when
     * calling this a second time you'll get an annoying protocol error
     */
    wl_surface_attach(data->surface, NULL, 0, 0);
    wl_surface_commit(data->surface);

    /* Create the shell surface and map the toplevel */
#ifdef HAVE_LIBDECOR_H
    if (c->shell.libdecor) {
        data->shell_surface.libdecor.frame = libdecor_decorate(c->shell.libdecor,
                                                               data->surface,
                                                               &libdecor_frame_interface,
                                                               data);
        if (data->shell_surface.libdecor.frame == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to create libdecor frame!");
        } else {
            libdecor_frame_set_app_id(data->shell_surface.libdecor.frame, c->classname);
            libdecor_frame_map(data->shell_surface.libdecor.frame);
        }
    } else
#endif
    if (c->shell.xdg) {
        data->shell_surface.xdg.surface = xdg_wm_base_get_xdg_surface(c->shell.xdg, data->surface);
        xdg_surface_set_user_data(data->shell_surface.xdg.surface, data);
        xdg_surface_add_listener(data->shell_surface.xdg.surface, &shell_surface_listener_xdg, data);

        /* !!! FIXME: add popup role */
        data->shell_surface.xdg.roleobj.toplevel = xdg_surface_get_toplevel(data->shell_surface.xdg.surface);
        xdg_toplevel_set_app_id(data->shell_surface.xdg.roleobj.toplevel, c->classname);
        xdg_toplevel_add_listener(data->shell_surface.xdg.roleobj.toplevel, &toplevel_listener_xdg, data);
    }

    /* Restore state that was set prior to this call */
    Wayland_SetWindowTitle(_this, window);
    if (window->flags & SDL_WINDOW_MAXIMIZED) {
        Wayland_MaximizeWindow(_this, window);
    }
    if (window->flags & SDL_WINDOW_MINIMIZED) {
        Wayland_MinimizeWindow(_this, window);
    }
    Wayland_SetWindowFullscreen(_this, window, display, (window->flags & SDL_WINDOW_FULLSCREEN) != 0);

    /* We have to wait until the surface gets a "configure" event, or use of
     * this surface will fail. This is a new rule for xdg_shell.
     */
#ifdef HAVE_LIBDECOR_H
    if (c->shell.libdecor) {
        if (data->shell_surface.libdecor.frame) {
            while (!data->shell_surface.libdecor.initial_configure_seen) {
                WAYLAND_wl_display_flush(c->display);
                WAYLAND_wl_display_dispatch(c->display);
            }
        }
    } else
#endif
    if (c->shell.xdg) {
        if (data->shell_surface.xdg.surface) {
            while (!data->shell_surface.xdg.initial_configure_seen) {
                WAYLAND_wl_display_flush(c->display);
                WAYLAND_wl_display_dispatch(c->display);
            }
        }

        /* Create the window decorations */
        if (data->shell_surface.xdg.roleobj.toplevel && c->decoration_manager) {
            data->server_decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(c->decoration_manager, data->shell_surface.xdg.roleobj.toplevel);
        }
    }

    /* Unlike the rest of window state we have to set this _after_ flushing the
     * display, because we need to create the decorations before possibly hiding
     * them immediately afterward.
     */
#ifdef HAVE_LIBDECOR_H
    if (c->shell.libdecor) {
        /* ... but don't call it redundantly for libdecor, the decorator
         * may not interpret a redundant call nicely and cause weird stuff to happen
         */
        if (window->flags & SDL_WINDOW_BORDERLESS) {
            Wayland_SetWindowBordered(_this, window, SDL_FALSE);
        }
    } else
#endif
    {
        Wayland_SetWindowBordered(_this, window, !(window->flags & SDL_WINDOW_BORDERLESS));
    }

    /* We're finally done putting the window together, raise if possible */
    if (c->activation_manager) {
        /* Note that we don't check for empty strings, as that is still
         * considered a valid activation token!
         */
        const char *activation_token = SDL_getenv("XDG_ACTIVATION_TOKEN");
        if (activation_token) {
            xdg_activation_v1_activate(c->activation_manager,
                                       activation_token,
                                       data->surface);

            /* Clear this variable, per the protocol's request */
            unsetenv("XDG_ACTIVATION_TOKEN");
        }
    }
}

void Wayland_HideWindow(_THIS, SDL_Window *window)
{
    SDL_VideoData *data = _this->driverdata;
    SDL_WindowData *wind = window->driverdata;

    if (wind->server_decoration) {
       zxdg_toplevel_decoration_v1_destroy(wind->server_decoration);
       wind->server_decoration = NULL;
    }

#ifdef HAVE_LIBDECOR_H
    if (data->shell.libdecor) {
        if (wind->shell_surface.libdecor.frame) {
            libdecor_frame_unref(wind->shell_surface.libdecor.frame);
            wind->shell_surface.libdecor.frame = NULL;
        }
    } else
#endif
    if (data->shell.xdg) {
        if (wind->shell_surface.xdg.roleobj.toplevel) {
            xdg_toplevel_destroy(wind->shell_surface.xdg.roleobj.toplevel);
            wind->shell_surface.xdg.roleobj.toplevel = NULL;
        }
        if (wind->shell_surface.xdg.surface) {
            xdg_surface_destroy(wind->shell_surface.xdg.surface);
            wind->shell_surface.xdg.surface = NULL;
        }
    }
}

static void
handle_xdg_activation_done(void *data,
                           struct xdg_activation_token_v1 *xdg_activation_token_v1,
                           const char *token)
{
    SDL_WindowData *window = data;
    if (xdg_activation_token_v1 == window->activation_token) {
        xdg_activation_v1_activate(window->waylandData->activation_manager,
                                   token,
                                   window->surface);
        xdg_activation_token_v1_destroy(window->activation_token);
        window->activation_token = NULL;
    }
}

static const struct xdg_activation_token_v1_listener activation_listener_xdg = {
    handle_xdg_activation_done
};

/* The xdg-activation protocol considers "activation" to be one of two things:
 *
 * 1: Raising a window to the top and flashing the titlebar
 * 2: Flashing the titlebar while keeping the window where it is
 *
 * As you might expect from Wayland, the general policy is to go with #2 unless
 * the client can prove to the compositor beyond a reasonable doubt that raising
 * the window will not be malicuous behavior.
 *
 * For SDL this means RaiseWindow and FlashWindow both use the same protocol,
 * but in different ways: RaiseWindow will provide as _much_ information as
 * possible while FlashWindow will provide as _little_ information as possible,
 * to nudge the compositor into doing what we want.
 *
 * This isn't _strictly_ what the protocol says will happen, but this is what
 * current implementations are doing (as of writing, YMMV in the far distant
 * future).
 *
 * -flibit
 */
static void
Wayland_activate_window(SDL_VideoData *data, SDL_WindowData *wind,
                        struct wl_surface *surface,
                        uint32_t serial, struct wl_seat *seat)
{
    if (data->activation_manager) {
        if (wind->activation_token != NULL) {
            /* We're about to overwrite this with a new request */
            xdg_activation_token_v1_destroy(wind->activation_token);
        }

        wind->activation_token = xdg_activation_v1_get_activation_token(data->activation_manager);
        xdg_activation_token_v1_add_listener(wind->activation_token,
                                             &activation_listener_xdg,
                                             wind);

        /* Note that we are not setting the app_id or serial here.
         *
         * Hypothetically we could set the app_id from data->classname, but
         * that part of the API is for _external_ programs, not ourselves.
         *
         * -flibit
         */
        if (surface != NULL) {
            xdg_activation_token_v1_set_surface(wind->activation_token, surface);
        }
        if (seat != NULL) {
            xdg_activation_token_v1_set_serial(wind->activation_token, serial, seat);
        }
        xdg_activation_token_v1_commit(wind->activation_token);
    }
}

void
Wayland_RaiseWindow(_THIS, SDL_Window *window)
{
    SDL_WindowData *wind = window->driverdata;

    /* FIXME: This Raise event is arbitrary and doesn't come from an event, so
     * it's actually very likely that this token will be ignored! Maybe add
     * support for passing serials (and the associated seat) so this can have
     * a better chance of actually raising the window.
     * -flibit
     */
    Wayland_activate_window(_this->driverdata,
                            wind,
                            wind->surface,
                            0,
                            NULL);
}

int
Wayland_FlashWindow(_THIS, SDL_Window *window, SDL_FlashOperation operation)
{
    Wayland_activate_window(_this->driverdata,
                            window->driverdata,
                            NULL,
                            0,
                            NULL);
    return 0;
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

    if (SDL_strcmp(name, SDL_HINT_QTWAYLAND_CONTENT_ORIENTATION) == 0) {
        int32_t orientation = QT_EXTENDED_SURFACE_ORIENTATION_PRIMARYORIENTATION;

        if (newValue != NULL) {
            if (SDL_strcmp(newValue, "portrait") == 0) {
                orientation = QT_EXTENDED_SURFACE_ORIENTATION_PORTRAITORIENTATION;
            } else if (SDL_strcmp(newValue, "landscape") == 0) {
                orientation = QT_EXTENDED_SURFACE_ORIENTATION_LANDSCAPEORIENTATION;
            } else if (SDL_strcmp(newValue, "inverted-portrait") == 0) {
                orientation = QT_EXTENDED_SURFACE_ORIENTATION_INVERTEDPORTRAITORIENTATION;
            } else if (SDL_strcmp(newValue, "inverted-landscape") == 0) {
                orientation = QT_EXTENDED_SURFACE_ORIENTATION_INVERTEDLANDSCAPEORIENTATION;
            }
        }

        qt_extended_surface_set_content_orientation(qt_extended_surface, orientation);
    } else if (SDL_strcmp(name, SDL_HINT_QTWAYLAND_WINDOW_FLAGS) == 0) {
        uint32_t flags = 0;

        if (newValue != NULL) {
            char *tmp = SDL_strdup(newValue);
            char *saveptr = NULL;

            char *flag = SDL_strtokr(tmp, " ", &saveptr);
            while (flag) {
                if (SDL_strcmp(flag, "OverridesSystemGestures") == 0) {
                    flags |= QT_EXTENDED_SURFACE_WINDOWFLAG_OVERRIDESSYSTEMGESTURES;
                } else if (SDL_strcmp(flag, "StaysOnTop") == 0) {
                    flags |= QT_EXTENDED_SURFACE_WINDOWFLAG_STAYSONTOP;
                } else if (SDL_strcmp(flag, "BypassWindowManager") == 0) {
                    // See https://github.com/qtproject/qtwayland/commit/fb4267103d
                    flags |= 4 /* QT_EXTENDED_SURFACE_WINDOWFLAG_BYPASSWINDOWMANAGER */;
                }

                flag = SDL_strtokr(NULL, " ", &saveptr);
            }

            SDL_free(tmp);
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
    struct wl_output *output = ((SDL_WaylandOutputData*) _display->driverdata)->output;
    SDL_VideoData *viddata = (SDL_VideoData *) _this->driverdata;
    SetFullscreen(window, fullscreen ? output : NULL);

    WAYLAND_wl_display_flush(viddata->display);
}

void
Wayland_RestoreWindow(_THIS, SDL_Window * window)
{
    SDL_WindowData *wind = window->driverdata;
    SDL_VideoData *viddata = (SDL_VideoData *) _this->driverdata;

    /* Set this flag now even if we never actually maximized, eventually
     * ShowWindow will take care of it along with the other window state.
     */
    window->flags &= ~SDL_WINDOW_MAXIMIZED;

#ifdef HAVE_LIBDECOR_H
    if (viddata->shell.libdecor) {
        if (wind->shell_surface.libdecor.frame == NULL) {
            return; /* Can't do anything yet, wait for ShowWindow */
        }
        libdecor_frame_unset_maximized(wind->shell_surface.libdecor.frame);
    } else
#endif
    /* Note that xdg-shell does NOT provide a way to unset minimize! */
    if (viddata->shell.xdg) {
        if (wind->shell_surface.xdg.roleobj.toplevel == NULL) {
            return; /* Can't do anything yet, wait for ShowWindow */
        }
        xdg_toplevel_unset_maximized(wind->shell_surface.xdg.roleobj.toplevel);
    }

    WAYLAND_wl_display_flush( viddata->display );
}

void
Wayland_SetWindowBordered(_THIS, SDL_Window * window, SDL_bool bordered)
{
    SDL_WindowData *wind = window->driverdata;
    const SDL_VideoData *viddata = (const SDL_VideoData *) _this->driverdata;
#ifdef HAVE_LIBDECOR_H
    if (viddata->shell.libdecor) {
        if (wind->shell_surface.libdecor.frame) {
            libdecor_frame_set_visibility(wind->shell_surface.libdecor.frame, bordered);
        }
    } else
#endif
    if ((viddata->decoration_manager) && (wind->server_decoration)) {
        const enum zxdg_toplevel_decoration_v1_mode mode = bordered ? ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE : ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE;
        zxdg_toplevel_decoration_v1_set_mode(wind->server_decoration, mode);
    }
}

void
Wayland_SetWindowResizable(_THIS, SDL_Window * window, SDL_bool resizable)
{
#ifdef HAVE_LIBDECOR_H
    SDL_VideoData *data = _this->driverdata;
    const SDL_WindowData *wind = window->driverdata;

    if (data->shell.libdecor) {
        if (wind->shell_surface.libdecor.frame == NULL) {
            return; /* Can't do anything yet, wait for ShowWindow */
        }
        if (resizable) {
            libdecor_frame_set_capabilities(wind->shell_surface.libdecor.frame, LIBDECOR_ACTION_RESIZE);
        } else {
            libdecor_frame_unset_capabilities(wind->shell_surface.libdecor.frame, LIBDECOR_ACTION_RESIZE);
        }
    } else
#endif
    {
        CommitMinMaxDimensions(window);
    }
}

void
Wayland_MaximizeWindow(_THIS, SDL_Window * window)
{
    SDL_WindowData *wind = window->driverdata;
    SDL_VideoData *viddata = (SDL_VideoData *) _this->driverdata;

    if (!(window->flags & SDL_WINDOW_RESIZABLE)) {
        return;
    }

    /* Set this flag now even if we don't actually maximize yet, eventually
     * ShowWindow will take care of it along with the other window state.
     */
    window->flags |= SDL_WINDOW_MAXIMIZED;

#ifdef HAVE_LIBDECOR_H
    if (viddata->shell.libdecor) {
        if (wind->shell_surface.libdecor.frame == NULL) {
            return; /* Can't do anything yet, wait for ShowWindow */
        }
        libdecor_frame_set_maximized(wind->shell_surface.libdecor.frame);
    } else
#endif
    if (viddata->shell.xdg) {
        if (wind->shell_surface.xdg.roleobj.toplevel == NULL) {
            return; /* Can't do anything yet, wait for ShowWindow */
        }
        xdg_toplevel_set_maximized(wind->shell_surface.xdg.roleobj.toplevel);
    }

    WAYLAND_wl_display_flush(viddata->display);
}

void
Wayland_MinimizeWindow(_THIS, SDL_Window * window)
{
    SDL_WindowData *wind = window->driverdata;
    SDL_VideoData *viddata = (SDL_VideoData *) _this->driverdata;

#ifdef HAVE_LIBDECOR_H
    if (viddata->shell.libdecor) {
        if (wind->shell_surface.libdecor.frame == NULL) {
            return; /* Can't do anything yet, wait for ShowWindow */
        }
        libdecor_frame_set_minimized(wind->shell_surface.libdecor.frame);
    } else
#endif
    if (viddata->shell.xdg) {
        if (wind->shell_surface.xdg.roleobj.toplevel == NULL) {
            return; /* Can't do anything yet, wait for ShowWindow */
        }
        xdg_toplevel_set_minimized(wind->shell_surface.xdg.roleobj.toplevel);
    }

    WAYLAND_wl_display_flush(viddata->display);
}

void
Wayland_SetWindowMouseGrab(_THIS, SDL_Window *window, SDL_bool grabbed)
{
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;

    if (grabbed) {
        Wayland_input_confine_pointer(window, data->input);
    } else {
        Wayland_input_unconfine_pointer(data->input);
    }
}

void
Wayland_SetWindowKeyboardGrab(_THIS, SDL_Window *window, SDL_bool grabbed)
{
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;

    if (grabbed) {
        Wayland_input_grab_keyboard(window, data->input);
    } else {
        Wayland_input_ungrab_keyboard(window);
    }
}

int Wayland_CreateWindow(_THIS, SDL_Window *window)
{
    SDL_WindowData *data;
    SDL_VideoData *c;
    struct wl_region *region;

    data = SDL_calloc(1, sizeof *data);
    if (data == NULL)
        return SDL_OutOfMemory();

    c = _this->driverdata;
    window->driverdata = data;

    if (!(window->flags & SDL_WINDOW_VULKAN)) {
        if (!(window->flags & SDL_WINDOW_OPENGL)) {
            SDL_GL_LoadLibrary(NULL);
            window->flags |= SDL_WINDOW_OPENGL;
        }
    }

    if (window->x == SDL_WINDOWPOS_UNDEFINED) {
        window->x = 0;
    }
    if (window->y == SDL_WINDOWPOS_UNDEFINED) {
        window->y = 0;
    }

    data->waylandData = c;
    data->sdlwindow = window;

    data->scale_factor = 1.0;

    if (window->flags & SDL_WINDOW_ALLOW_HIGHDPI) {
        int i;
        for (i=0; i < SDL_GetVideoDevice()->num_displays; i++) {
            float scale = ((SDL_WaylandOutputData*)SDL_GetVideoDevice()->displays[i].driverdata)->scale_factor;
            if (scale > data->scale_factor) {
                data->scale_factor = scale;
            }
        }
    }

    data->outputs = NULL;
    data->num_outputs = 0;

    data->floating_width = window->windowed.w;
    data->floating_height = window->windowed.h;

    data->surface =
        wl_compositor_create_surface(c->compositor);
    wl_surface_add_listener(data->surface, &surface_listener, data);

    SDL_WAYLAND_register_surface(data->surface);

    /* Fire a callback when the compositor wants a new frame rendered.
     * Right now this only matters for OpenGL; we use this callback to add a
     * wait timeout that avoids getting deadlocked by the compositor when the
     * window isn't visible.
     */
    if (window->flags & SDL_WINDOW_OPENGL) {
        data->frame_callback = wl_surface_frame(data->surface);
        wl_callback_add_listener(data->frame_callback, &surface_frame_listener, data);
    }

#ifdef SDL_VIDEO_DRIVER_WAYLAND_QT_TOUCH
    if (c->surface_extension) {
        data->extended_surface = qt_surface_extension_get_extended_surface(
                c->surface_extension, data->surface);

        QtExtendedSurface_Subscribe(data->extended_surface, SDL_HINT_QTWAYLAND_CONTENT_ORIENTATION);
        QtExtendedSurface_Subscribe(data->extended_surface, SDL_HINT_QTWAYLAND_WINDOW_FLAGS);
    }
#endif /* SDL_VIDEO_DRIVER_WAYLAND_QT_TOUCH */

    if (window->flags & SDL_WINDOW_OPENGL) {
        data->egl_window = WAYLAND_wl_egl_window_create(data->surface,
                                            window->w * data->scale_factor, window->h * data->scale_factor);

        /* Create the GLES window surface */
        data->egl_surface = SDL_EGL_CreateSurface(_this, (NativeWindowType) data->egl_window);

        if (data->egl_surface == EGL_NO_SURFACE) {
            return SDL_SetError("failed to create an EGL window surface");
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

    /* We may need to create an idle inhibitor for this new window */
    Wayland_SuspendScreenSaver(_this);

    return 0;
}


static void
Wayland_HandleResize(SDL_Window *window, int width, int height, float scale)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    SDL_VideoData *viddata = data->waylandData;

    struct wl_region *region;
    window->w = 0;
    window->h = 0;
    SDL_SendWindowEvent(window, SDL_WINDOWEVENT_RESIZED, width, height);
    window->w = width;
    window->h = height;
    data->scale_factor = scale;

    wl_surface_set_buffer_scale(data->surface, data->scale_factor);

    if (data->egl_window) {
        WAYLAND_wl_egl_window_resize(data->egl_window,
                                        window->w * data->scale_factor,
                                        window->h * data->scale_factor,
                                        0, 0);
    }

    region = wl_compositor_create_region(data->waylandData->compositor);
    wl_region_add(region, 0, 0, window->w, window->h);
    wl_surface_set_opaque_region(data->surface, region);
    wl_region_destroy(region);

    /* XXX: This workarounds issues with commiting buffers with old size after
     * already acknowledging the new size, which can cause protocol violations.
     * It doesn't fix the first frames after resize being glitched visually,
     * but at least lets us not be terminated by the compositor.
     * Can be removed once SDL's resize logic becomes compliant. */
    if (viddata->shell.xdg && data->shell_surface.xdg.surface) {
       xdg_surface_set_window_geometry(data->shell_surface.xdg.surface, 0, 0, window->w, window->h);
    }
}

void
Wayland_SetWindowMinimumSize(_THIS, SDL_Window * window)
{
    CommitMinMaxDimensions(window);
}

void
Wayland_SetWindowMaximumSize(_THIS, SDL_Window * window)
{
    CommitMinMaxDimensions(window);
}

void Wayland_SetWindowSize(_THIS, SDL_Window * window)
{
    SDL_VideoData *data = _this->driverdata;
    SDL_WindowData *wind = window->driverdata;
    struct wl_region *region;
#ifdef HAVE_LIBDECOR_H
    struct libdecor_state *state;
#endif

#ifdef HAVE_LIBDECOR_H
    /* we must not resize the window while we have a static (non-floating) size */
    if (data->shell.libdecor &&
        wind->shell_surface.libdecor.frame &&
        !libdecor_frame_is_floating(wind->shell_surface.libdecor.frame)) {
            return;
    }
#endif

    wl_surface_set_buffer_scale(wind->surface, wind->scale_factor);

    if (wind->egl_window) {
        WAYLAND_wl_egl_window_resize(wind->egl_window,
                                     window->w * wind->scale_factor,
                                     window->h * wind->scale_factor,
                                     0, 0);
    }

#ifdef HAVE_LIBDECOR_H
    if (data->shell.libdecor && wind->shell_surface.libdecor.frame) {
        state = libdecor_state_new(window->w, window->h);
        libdecor_frame_commit(wind->shell_surface.libdecor.frame, state, NULL);
        libdecor_state_free(state);
    }
#endif

    /* windowed is unconditionally set, so we can trust it here */
    wind->floating_width = window->windowed.w;
    wind->floating_height = window->windowed.h;

    region = wl_compositor_create_region(data->compositor);
    wl_region_add(region, 0, 0, window->w, window->h);
    wl_surface_set_opaque_region(wind->surface, region);
    wl_region_destroy(region);

    /* Update the geometry which may have been set by a hack in Wayland_HandleResize */
    if (data->shell.xdg && wind->shell_surface.xdg.surface) {
       xdg_surface_set_window_geometry(wind->shell_surface.xdg.surface, 0, 0, window->w, window->h);
    }
}

void Wayland_SetWindowTitle(_THIS, SDL_Window * window)
{
    SDL_WindowData *wind = window->driverdata;
    SDL_VideoData *viddata = _this->driverdata;

    if (window->title != NULL) {
#ifdef HAVE_LIBDECOR_H
        if (viddata->shell.libdecor) {
            if (wind->shell_surface.libdecor.frame == NULL) {
                return; /* Can't do anything yet, wait for ShowWindow */
            }
            libdecor_frame_set_title(wind->shell_surface.libdecor.frame, window->title);
        } else
#endif
        if (viddata->shell.xdg) {
            if (wind->shell_surface.xdg.roleobj.toplevel == NULL) {
                return; /* Can't do anything yet, wait for ShowWindow */
            }
            xdg_toplevel_set_title(wind->shell_surface.xdg.roleobj.toplevel, window->title);
        }
    }

    WAYLAND_wl_display_flush(viddata->display);
}

void
Wayland_SuspendScreenSaver(_THIS)
{
    SDL_VideoData *data = (SDL_VideoData *)_this->driverdata;

#if SDL_USE_LIBDBUS
    if (SDL_DBus_ScreensaverInhibit(_this->suspend_screensaver)) {
        return;
    }
#endif

    /* The idle_inhibit_unstable_v1 protocol suspends the screensaver
       on a per wl_surface basis, but SDL assumes that suspending
       the screensaver can be done independently of any window.

       To reconcile these differences, we propagate the idle inhibit
       state to each window. If there is no window active, we will
       be able to inhibit idle once the first window is created.
    */
    if (data->idle_inhibit_manager) {
        SDL_Window *window = _this->windows;
        while (window) {
            SDL_WindowData *win_data = window->driverdata;

            if (_this->suspend_screensaver && !win_data->idle_inhibitor) {
                win_data->idle_inhibitor =
                    zwp_idle_inhibit_manager_v1_create_inhibitor(data->idle_inhibit_manager,
                                                                 win_data->surface);
            } else if (!_this->suspend_screensaver && win_data->idle_inhibitor) {
                zwp_idle_inhibitor_v1_destroy(win_data->idle_inhibitor);
                win_data->idle_inhibitor = NULL;
            }

            window = window->next;
        }
    }
}

void Wayland_DestroyWindow(_THIS, SDL_Window *window)
{
    SDL_VideoData *data = _this->driverdata;
    SDL_WindowData *wind = window->driverdata;

    if (data) {
        if (wind->egl_surface) {
            SDL_EGL_DestroySurface(_this, wind->egl_surface);
        }
        if (wind->egl_window) {
            WAYLAND_wl_egl_window_destroy(wind->egl_window);
        }

        if (wind->idle_inhibitor) {
            zwp_idle_inhibitor_v1_destroy(wind->idle_inhibitor);
        }

        if (wind->activation_token) {
            xdg_activation_token_v1_destroy(wind->activation_token);
        }

        SDL_free(wind->outputs);

        if (wind->frame_callback) {
            wl_callback_destroy(wind->frame_callback);
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
