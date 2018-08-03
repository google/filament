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

/*
  Contributed by Brandon Schaefer, <brandon.schaefer@canonical.com>
*/

#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_MIR

#include "../../events/SDL_events_c.h"
#include "../../events/SDL_keyboard_c.h"
#include "../../events/SDL_touch_c.h"
#include "../../events/scancodes_xfree86.h"

#include "SDL_mirevents.h"
#include "SDL_mirwindow.h"

#include <xkbcommon/xkbcommon.h>

#include "SDL_mirdyn.h"

static void
HandleKeyText(int32_t key_code)
{
    char text[8];
    int size = 0;

    size = MIR_xkb_keysym_to_utf8(key_code, text, sizeof text);

    if (size > 0) {
        text[size] = '\0';
        SDL_SendKeyboardText(text);
    }
}

/* FIXME
   Mir still needs to implement its IM API, for now we assume
   a single key press produces a character.
*/
static void
HandleKeyEvent(MirKeyboardEvent const* key_event, SDL_Window* window)
{
    xkb_keysym_t key_code;
    Uint8 key_state;
    int event_scancode;
    uint32_t sdl_scancode = SDL_SCANCODE_UNKNOWN;

    MirKeyboardAction action = MIR_mir_keyboard_event_action(key_event);

    key_state      = SDL_PRESSED;
    key_code       = MIR_mir_keyboard_event_key_code(key_event);
    event_scancode = MIR_mir_keyboard_event_scan_code(key_event);

    if (action == mir_keyboard_action_up)
        key_state = SDL_RELEASED;

    if (event_scancode < SDL_arraysize(xfree86_scancode_table2))
        sdl_scancode = xfree86_scancode_table2[event_scancode];

    if (sdl_scancode != SDL_SCANCODE_UNKNOWN)
        SDL_SendKeyboardKey(key_state, sdl_scancode);

    if (key_state == SDL_PRESSED)
        HandleKeyText(key_code);
}

static void
HandleMouseButton(SDL_Window* sdl_window, Uint8 state, MirPointerEvent const* pointer)
{
    uint32_t sdl_button           = SDL_BUTTON_LEFT;
    MirPointerButton button_state = mir_pointer_button_primary;

    static uint32_t old_button_states = 0;
    uint32_t new_button_states = MIR_mir_pointer_event_buttons(pointer);

    // XOR on our old button states vs our new states to get the newley pressed/released button
    button_state = new_button_states ^ old_button_states;

    switch (button_state) {
        case mir_pointer_button_primary:
            sdl_button = SDL_BUTTON_LEFT;
            break;
        case mir_pointer_button_secondary:
            sdl_button = SDL_BUTTON_RIGHT;
            break;
        case mir_pointer_button_tertiary:
            sdl_button = SDL_BUTTON_MIDDLE;
            break;
        case mir_pointer_button_forward:
            sdl_button = SDL_BUTTON_X1;
            break;
        case mir_pointer_button_back:
            sdl_button = SDL_BUTTON_X2;
            break;
        default:
            break;
    }

    old_button_states = new_button_states;

    SDL_SendMouseButton(sdl_window, 0, state, sdl_button);
}

static void
HandleMouseMotion(SDL_Window* sdl_window, int x, int y)
{
    SDL_Mouse* mouse = SDL_GetMouse();
    SDL_SendMouseMotion(sdl_window, 0, mouse->relative_mode, x, y);
}

static void
HandleTouchPress(int device_id, int source_id, SDL_bool down, float x, float y, float pressure)
{
    SDL_SendTouch(device_id, source_id, down, x, y, pressure);
}

static void
HandleTouchMotion(int device_id, int source_id, float x, float y, float pressure)
{
    SDL_SendTouchMotion(device_id, source_id, x, y, pressure);
}

static void
HandleMouseScroll(SDL_Window* sdl_window, float hscroll, float vscroll)
{
    SDL_SendMouseWheel(sdl_window, 0, hscroll, vscroll, SDL_MOUSEWHEEL_NORMAL);
}

static void
AddTouchDevice(int device_id)
{
    if (SDL_AddTouch(device_id, "") < 0)
        SDL_SetError("Error: can't add touch %s, %d", __FILE__, __LINE__);
}

static void
HandleTouchEvent(MirTouchEvent const* touch, int device_id, SDL_Window* sdl_window)
{
    int i, point_count;
    point_count = MIR_mir_touch_event_point_count(touch);

    AddTouchDevice(device_id);

    for (i = 0; i < point_count; i++) {
        int id = MIR_mir_touch_event_id(touch, i);

        int width  = sdl_window->w;
        int height = sdl_window->h;

        float x = MIR_mir_touch_event_axis_value(touch, i, mir_touch_axis_x);
        float y = MIR_mir_touch_event_axis_value(touch, i, mir_touch_axis_y);

        float n_x = x / width;
        float n_y = y / height;

        float pressure = MIR_mir_touch_event_axis_value(touch, i, mir_touch_axis_pressure);

        switch (MIR_mir_touch_event_action(touch, i)) {
            case mir_touch_action_up:
                HandleTouchPress(device_id, id, SDL_FALSE, n_x, n_y, pressure);
                break;
            case mir_touch_action_down:
                HandleTouchPress(device_id, id, SDL_TRUE, n_x, n_y, pressure);
                break;
            case mir_touch_action_change:
                HandleTouchMotion(device_id, id, n_x, n_y, pressure);
                break;
            case mir_touch_actions:
                break;
        }
    }
}

static void
HandleMouseEvent(MirPointerEvent const* pointer, SDL_Window* sdl_window)
{
    SDL_SetMouseFocus(sdl_window);

    switch (MIR_mir_pointer_event_action(pointer)) {
        case mir_pointer_action_button_down:
            HandleMouseButton(sdl_window, SDL_PRESSED, pointer);
            break;
        case mir_pointer_action_button_up:
            HandleMouseButton(sdl_window, SDL_RELEASED, pointer);
            break;
        case mir_pointer_action_motion: {
            int x, y;
            float hscroll, vscroll;
            SDL_Mouse* mouse = SDL_GetMouse();
            x = MIR_mir_pointer_event_axis_value(pointer, mir_pointer_axis_x);
            y = MIR_mir_pointer_event_axis_value(pointer, mir_pointer_axis_y);

            if (mouse) {
                if (mouse->relative_mode) {
                    int relative_x = MIR_mir_pointer_event_axis_value(pointer, mir_pointer_axis_relative_x);
                    int relative_y = MIR_mir_pointer_event_axis_value(pointer, mir_pointer_axis_relative_y);
                    HandleMouseMotion(sdl_window, relative_x, relative_y);
                }
                else if (mouse->x != x || mouse->y != y) {
                    HandleMouseMotion(sdl_window, x, y);
                }
            }

            hscroll = MIR_mir_pointer_event_axis_value(pointer, mir_pointer_axis_hscroll);
            vscroll = MIR_mir_pointer_event_axis_value(pointer, mir_pointer_axis_vscroll);
            if (vscroll != 0 || hscroll != 0)
                HandleMouseScroll(sdl_window, hscroll, vscroll);
        }
            break;
        case mir_pointer_action_leave:
            SDL_SetMouseFocus(NULL);
            break;
        case mir_pointer_action_enter:
        default:
            break;
    }
}

static void
HandleInput(MirInputEvent const* input_event, SDL_Window* window)
{
    switch (MIR_mir_input_event_get_type(input_event)) {
        case (mir_input_event_type_key):
            HandleKeyEvent(MIR_mir_input_event_get_keyboard_event(input_event), window);
            break;
        case (mir_input_event_type_pointer):
            HandleMouseEvent(MIR_mir_input_event_get_pointer_event(input_event), window);
            break;
        case (mir_input_event_type_touch):
            HandleTouchEvent(MIR_mir_input_event_get_touch_event(input_event),
                             MIR_mir_input_event_get_device_id(input_event),
                             window);
            break;
        default:
            break;
    }
}

static void
HandleResize(MirResizeEvent const* resize_event, SDL_Window* window)
{
    int new_w = MIR_mir_resize_event_get_width (resize_event);
    int new_h = MIR_mir_resize_event_get_height(resize_event);

    int old_w = window->w;
    int old_h = window->h;

    if (new_w != old_w || new_h != old_h)
        SDL_SendWindowEvent(window, SDL_WINDOWEVENT_RESIZED, new_w, new_h);
}

static void
HandleWindow(MirWindowEvent const* event, SDL_Window* window)
{
    MirWindowAttrib attrib = MIR_mir_window_event_get_attribute(event);
    int value              = MIR_mir_window_event_get_attribute_value(event);

    if (attrib == mir_window_attrib_focus) {
        if (value == mir_window_focus_state_focused) {
            SDL_SetKeyboardFocus(window);
        }
        else if (value == mir_window_focus_state_unfocused) {
            SDL_SetKeyboardFocus(NULL);
        }
    }
}

static void
MIR_HandleClose(SDL_Window* window) {
    SDL_SendWindowEvent(window, SDL_WINDOWEVENT_CLOSE, 0, 0);
}

void
MIR_HandleEvent(MirWindow* mirwindow, MirEvent const* ev, void* context)
{
    MirEventType event_type = MIR_mir_event_get_type(ev);
    SDL_Window* window      = (SDL_Window*)context;

    if (window) {
        switch (event_type) {
            case (mir_event_type_input):
                HandleInput(MIR_mir_event_get_input_event(ev), window);
                break;
            case (mir_event_type_resize):
                HandleResize(MIR_mir_event_get_resize_event(ev), window);
                break;
            case (mir_event_type_window):
                HandleWindow(MIR_mir_event_get_window_event(ev), window);
                break;
            case (mir_event_type_close_window):
                MIR_HandleClose(window);
                break;
            default:
                break;
        }
    }
}

#endif /* SDL_VIDEO_DRIVER_MIR */

/* vi: set ts=4 sw=4 expandtab: */
