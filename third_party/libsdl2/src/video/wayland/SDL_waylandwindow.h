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

#ifndef SDL_waylandwindow_h_
#define SDL_waylandwindow_h_

#include "../SDL_sysvideo.h"
#include "SDL_syswm.h"

#include "SDL_waylandvideo.h"

struct SDL_WaylandInput;

typedef struct {
    struct zxdg_surface_v6 *surface;
    union {
        struct zxdg_toplevel_v6 *toplevel;
        struct zxdg_popup_v6 *popup;
    } roleobj;
} SDL_zxdg_shell_surface;

typedef struct {
    SDL_Window *sdlwindow;
    SDL_VideoData *waylandData;
    struct wl_surface *surface;
    union {
        /* !!! FIXME: add stable xdg_shell from 1.12 */
        SDL_zxdg_shell_surface zxdg;
        struct wl_shell_surface *wl;
    } shell_surface;
    struct wl_egl_window *egl_window;
    struct SDL_WaylandInput *keyboard_device;
    EGLSurface egl_surface;
    struct zwp_locked_pointer_v1 *locked_pointer;

#ifdef SDL_VIDEO_DRIVER_WAYLAND_QT_TOUCH
    struct qt_extended_surface *extended_surface;
#endif /* SDL_VIDEO_DRIVER_WAYLAND_QT_TOUCH */    
} SDL_WindowData;

extern void Wayland_ShowWindow(_THIS, SDL_Window *window);
extern void Wayland_SetWindowFullscreen(_THIS, SDL_Window * window,
                                        SDL_VideoDisplay * _display,
                                        SDL_bool fullscreen);
extern void Wayland_MaximizeWindow(_THIS, SDL_Window * window);
extern void Wayland_RestoreWindow(_THIS, SDL_Window * window);
extern int Wayland_CreateWindow(_THIS, SDL_Window *window);
extern void Wayland_SetWindowSize(_THIS, SDL_Window * window);
extern void Wayland_SetWindowTitle(_THIS, SDL_Window * window);
extern void Wayland_DestroyWindow(_THIS, SDL_Window *window);

extern SDL_bool
Wayland_GetWindowWMInfo(_THIS, SDL_Window * window, SDL_SysWMinfo * info);
extern int Wayland_SetWindowHitTest(SDL_Window *window, SDL_bool enabled);

#endif /* SDL_waylandwindow_h_ */

/* vi: set ts=4 sw=4 expandtab: */
