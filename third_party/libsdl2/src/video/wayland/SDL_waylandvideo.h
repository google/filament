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
#include "SDL_stdinc.h"

#ifndef SDL_waylandvideo_h_
#define SDL_waylandvideo_h_

#include <EGL/egl.h>
#include "wayland-util.h"

#include "../SDL_sysvideo.h"
#include "../../core/linux/SDL_dbus.h"
#include "../../core/linux/SDL_ime.h"

struct xkb_context;
struct SDL_WaylandInput;

#ifdef SDL_VIDEO_DRIVER_WAYLAND_QT_TOUCH
struct SDL_WaylandTouch;
struct qt_surface_extension;
struct qt_windowmanager;
#endif /* SDL_VIDEO_DRIVER_WAYLAND_QT_TOUCH */

typedef struct {
    struct wl_display *display;
    int display_disconnected;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_shm *shm;
    struct wl_cursor_theme *cursor_theme;
    struct wl_pointer *pointer;
    struct {
        struct xdg_wm_base *xdg;
#ifdef HAVE_LIBDECOR_H
        struct libdecor *libdecor;
#endif
    } shell;
    struct zwp_relative_pointer_manager_v1 *relative_pointer_manager;
    struct zwp_pointer_constraints_v1 *pointer_constraints;
    struct wl_data_device_manager *data_device_manager;
    struct zxdg_decoration_manager_v1 *decoration_manager;
    struct zwp_keyboard_shortcuts_inhibit_manager_v1 *key_inhibitor_manager;
    struct zwp_idle_inhibit_manager_v1 *idle_inhibit_manager;
    struct xdg_activation_v1 *activation_manager;
    struct zwp_text_input_manager_v3 *text_input_manager;

    EGLDisplay edpy;
    EGLContext context;
    EGLConfig econf;

    struct xkb_context *xkb_context;
    struct SDL_WaylandInput *input;

#ifdef SDL_VIDEO_DRIVER_WAYLAND_QT_TOUCH
    struct SDL_WaylandTouch *touch;
    struct qt_surface_extension *surface_extension;
    struct qt_windowmanager *windowmanager;
#endif /* SDL_VIDEO_DRIVER_WAYLAND_QT_TOUCH */

    char *classname;

    int relative_mouse_mode;
} SDL_VideoData;

typedef struct {
    struct wl_output *output;
    float scale_factor;
    int x, y, width, height, refresh, transform;
    int physical_width, physical_height;
    float ddpi, hdpi, vdpi;
    SDL_VideoDisplay placeholder;
    SDL_bool done;
} SDL_WaylandOutputData;

/* Needed here to get wl_surface declaration, fixes GitHub#4594 */
#include "SDL_waylanddyn.h"

extern void SDL_WAYLAND_register_surface(struct wl_surface *surface);
extern void SDL_WAYLAND_register_output(struct wl_output *output);
extern SDL_bool SDL_WAYLAND_own_surface(struct wl_surface *surface);
extern SDL_bool SDL_WAYLAND_own_output(struct wl_output *output);

#endif /* SDL_waylandvideo_h_ */

/* vi: set ts=4 sw=4 expandtab: */
