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

#ifndef SDL_waylandevents_h_
#define SDL_waylandevents_h_

#include "SDL_waylandvideo.h"
#include "SDL_waylandwindow.h"
#include "SDL_waylanddatamanager.h"
#include "SDL_waylandkeyboard.h"

typedef struct {
    // repeat_rate in range of [1, 1000]
    int32_t repeat_rate;
    int32_t repeat_delay;
    SDL_bool is_initialized;

    SDL_bool is_key_down;
    uint32_t next_repeat_ms;
    uint32_t scancode;
    char text[8];
} SDL_WaylandKeyboardRepeat;

struct SDL_WaylandInput {
    SDL_VideoData *display;
    struct wl_seat *seat;
    struct wl_pointer *pointer;
    struct wl_touch *touch;
    struct wl_keyboard *keyboard;
    SDL_WaylandDataDevice *data_device;
    SDL_WaylandTextInput *text_input;
    struct zwp_relative_pointer_v1 *relative_pointer;
    struct zwp_confined_pointer_v1 *confined_pointer;
    SDL_Window *confined_pointer_window;
    SDL_WindowData *pointer_focus;
    SDL_WindowData *keyboard_focus;
    uint32_t pointer_enter_serial;

    /* Last motion location */
    wl_fixed_t sx_w;
    wl_fixed_t sy_w;

    double dx_frac;
    double dy_frac;

    struct {
        struct xkb_keymap *keymap;
        struct xkb_state *state;
        struct xkb_compose_table *compose_table;
        struct xkb_compose_state *compose_state;
    } xkb;

    /* information about axis events on current frame */
    struct {
        SDL_bool is_x_discrete;
        float x;

        SDL_bool is_y_discrete;
        float y;
    } pointer_curr_axis_info;

    SDL_WaylandKeyboardRepeat keyboard_repeat;
};

extern void Wayland_PumpEvents(_THIS);

extern void Wayland_add_data_device_manager(SDL_VideoData *d, uint32_t id, uint32_t version);
extern void Wayland_add_text_input_manager(SDL_VideoData *d, uint32_t id, uint32_t version);

extern void Wayland_display_add_input(SDL_VideoData *d, uint32_t id, uint32_t version);
extern void Wayland_display_destroy_input(SDL_VideoData *d);

extern void Wayland_display_add_pointer_constraints(SDL_VideoData *d, uint32_t id);
extern void Wayland_display_destroy_pointer_constraints(SDL_VideoData *d);

extern int Wayland_input_lock_pointer(struct SDL_WaylandInput *input);
extern int Wayland_input_unlock_pointer(struct SDL_WaylandInput *input);

extern int Wayland_input_confine_pointer(SDL_Window *window, struct SDL_WaylandInput *input);
extern int Wayland_input_unconfine_pointer(struct SDL_WaylandInput *input);

extern void Wayland_display_add_relative_pointer_manager(SDL_VideoData *d, uint32_t id);
extern void Wayland_display_destroy_relative_pointer_manager(SDL_VideoData *d);

extern int Wayland_input_grab_keyboard(SDL_Window *window, struct SDL_WaylandInput *input);
extern int Wayland_input_ungrab_keyboard(SDL_Window *window);

#endif /* SDL_waylandevents_h_ */

/* vi: set ts=4 sw=4 expandtab: */
