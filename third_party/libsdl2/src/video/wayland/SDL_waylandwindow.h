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

#ifndef SDL_waylandwindow_h_
#define SDL_waylandwindow_h_

#include "../SDL_sysvideo.h"
#include "SDL_syswm.h"
#include "../../events/SDL_touch_c.h"

#include "SDL_waylandvideo.h"

struct SDL_WaylandInput;

typedef struct {
    struct xdg_surface *surface;
    union {
        struct xdg_toplevel *toplevel;
        struct xdg_popup *popup;
    } roleobj;
    SDL_bool initial_configure_seen;
} SDL_xdg_shell_surface;

#ifdef HAVE_LIBDECOR_H
typedef struct {
    struct libdecor_frame *frame;
    SDL_bool initial_configure_seen;
} SDL_libdecor_surface;
#endif

typedef struct {
    SDL_Window *sdlwindow;
    SDL_VideoData *waylandData;
    struct wl_surface *surface;
    struct wl_callback *frame_callback;
    union {
#ifdef HAVE_LIBDECOR_H
        SDL_libdecor_surface libdecor;
#endif
        SDL_xdg_shell_surface xdg;
    } shell_surface;
    struct wl_egl_window *egl_window;
    struct SDL_WaylandInput *keyboard_device;
    EGLSurface egl_surface;
    struct zwp_locked_pointer_v1 *locked_pointer;
    struct zxdg_toplevel_decoration_v1 *server_decoration;
    struct zwp_keyboard_shortcuts_inhibitor_v1 *key_inhibitor;
    struct zwp_idle_inhibitor_v1 *idle_inhibitor;
    struct xdg_activation_token_v1 *activation_token;

    /* floating dimensions for restoring from maximized and fullscreen */
    int floating_width, floating_height;

    SDL_atomic_t swap_interval_ready;

#ifdef SDL_VIDEO_DRIVER_WAYLAND_QT_TOUCH
    struct qt_extended_surface *extended_surface;
#endif /* SDL_VIDEO_DRIVER_WAYLAND_QT_TOUCH */

    SDL_WaylandOutputData **outputs;
    int num_outputs;

    float scale_factor;
} SDL_WindowData;

extern void Wayland_ShowWindow(_THIS, SDL_Window *window);
extern void Wayland_HideWindow(_THIS, SDL_Window *window);
extern void Wayland_RaiseWindow(_THIS, SDL_Window *window);
extern void Wayland_SetWindowFullscreen(_THIS, SDL_Window * window,
                                        SDL_VideoDisplay * _display,
                                        SDL_bool fullscreen);
extern void Wayland_MaximizeWindow(_THIS, SDL_Window * window);
extern void Wayland_MinimizeWindow(_THIS, SDL_Window * window);
extern void Wayland_SetWindowMouseGrab(_THIS, SDL_Window * window, SDL_bool grabbed);
extern void Wayland_SetWindowKeyboardGrab(_THIS, SDL_Window *window, SDL_bool grabbed);
extern void Wayland_RestoreWindow(_THIS, SDL_Window * window);
extern void Wayland_SetWindowBordered(_THIS, SDL_Window * window, SDL_bool bordered);
extern void Wayland_SetWindowResizable(_THIS, SDL_Window * window, SDL_bool resizable);
extern int Wayland_CreateWindow(_THIS, SDL_Window *window);
extern void Wayland_SetWindowSize(_THIS, SDL_Window * window);
extern void Wayland_SetWindowMinimumSize(_THIS, SDL_Window * window);
extern void Wayland_SetWindowMaximumSize(_THIS, SDL_Window * window);
extern int Wayland_SetWindowModalFor(_THIS, SDL_Window * modal_window, SDL_Window * parent_window);
extern void Wayland_SetWindowTitle(_THIS, SDL_Window * window);
extern void Wayland_DestroyWindow(_THIS, SDL_Window *window);
extern void Wayland_SuspendScreenSaver(_THIS);

extern SDL_bool
Wayland_GetWindowWMInfo(_THIS, SDL_Window * window, SDL_SysWMinfo * info);
extern int Wayland_SetWindowHitTest(SDL_Window *window, SDL_bool enabled);
extern int Wayland_FlashWindow(_THIS, SDL_Window * window, SDL_FlashOperation operation);

#endif /* SDL_waylandwindow_h_ */

/* vi: set ts=4 sw=4 expandtab: */
