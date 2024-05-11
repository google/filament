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
#include "../SDL_internal.h"

/* General mouse handling code for SDL */

#include "SDL_assert.h"
#include "SDL_hints.h"
#include "SDL_timer.h"
#include "SDL_events.h"
#include "SDL_events_c.h"
#include "../video/SDL_sysvideo.h"

/* #define DEBUG_MOUSE */

/* The mouse state */
static SDL_Mouse SDL_mouse;
static Uint32 SDL_double_click_time = 500;
static int SDL_double_click_radius = 1;

static int
SDL_PrivateSendMouseMotion(SDL_Window * window, SDL_MouseID mouseID, int relative, int x, int y);

static void SDLCALL
SDL_MouseNormalSpeedScaleChanged(void *userdata, const char *name, const char *oldValue, const char *hint)
{
    SDL_Mouse *mouse = (SDL_Mouse *)userdata;

    if (hint && *hint) {
        mouse->normal_speed_scale = (float)SDL_atof(hint);
    } else {
        mouse->normal_speed_scale = 1.0f;
    }
}

static void SDLCALL
SDL_MouseRelativeSpeedScaleChanged(void *userdata, const char *name, const char *oldValue, const char *hint)
{
    SDL_Mouse *mouse = (SDL_Mouse *)userdata;

    if (hint && *hint) {
        mouse->relative_speed_scale = (float)SDL_atof(hint);
    } else {
        mouse->relative_speed_scale = 1.0f;
    }
}

static void SDLCALL
SDL_TouchMouseEventsChanged(void *userdata, const char *name, const char *oldValue, const char *hint)
{
    SDL_Mouse *mouse = (SDL_Mouse *)userdata;

    if (hint && (*hint == '0' || SDL_strcasecmp(hint, "false") == 0)) {
        mouse->touch_mouse_events = SDL_FALSE;
    } else {
        mouse->touch_mouse_events = SDL_TRUE;
    }
}

/* Public functions */
int
SDL_MouseInit(void)
{
    SDL_Mouse *mouse = SDL_GetMouse();

    SDL_zerop(mouse);

    SDL_AddHintCallback(SDL_HINT_MOUSE_NORMAL_SPEED_SCALE,
                        SDL_MouseNormalSpeedScaleChanged, mouse);

    SDL_AddHintCallback(SDL_HINT_MOUSE_RELATIVE_SPEED_SCALE,
                        SDL_MouseRelativeSpeedScaleChanged, mouse);

    SDL_AddHintCallback(SDL_HINT_TOUCH_MOUSE_EVENTS,
                        SDL_TouchMouseEventsChanged, mouse);

    mouse->cursor_shown = SDL_TRUE;

    return (0);
}

void
SDL_SetDefaultCursor(SDL_Cursor * cursor)
{
    SDL_Mouse *mouse = SDL_GetMouse();

    mouse->def_cursor = cursor;
    if (!mouse->cur_cursor) {
        SDL_SetCursor(cursor);
    }
}

SDL_Mouse *
SDL_GetMouse(void)
{
    return &SDL_mouse;
}

void
SDL_SetDoubleClickTime(Uint32 interval)
{
    SDL_double_click_time = interval;
}

SDL_Window *
SDL_GetMouseFocus(void)
{
    SDL_Mouse *mouse = SDL_GetMouse();

    return mouse->focus;
}

#if 0
void
SDL_ResetMouse(void)
{
    SDL_Mouse *mouse = SDL_GetMouse();
    Uint8 i;

#ifdef DEBUG_MOUSE
    printf("Resetting mouse\n");
#endif
    for (i = 1; i <= sizeof(mouse->buttonstate)*8; ++i) {
        if (mouse->buttonstate & SDL_BUTTON(i)) {
            SDL_SendMouseButton(mouse->focus, mouse->mouseID, SDL_RELEASED, i);
        }
    }
    SDL_assert(mouse->buttonstate == 0);
}
#endif

void
SDL_SetMouseFocus(SDL_Window * window)
{
    SDL_Mouse *mouse = SDL_GetMouse();

    if (mouse->focus == window) {
        return;
    }

    /* Actually, this ends up being a bad idea, because most operating
       systems have an implicit grab when you press the mouse button down
       so you can drag things out of the window and then get the mouse up
       when it happens.  So, #if 0...
    */
#if 0
    if (mouse->focus && !window) {
        /* We won't get anymore mouse messages, so reset mouse state */
        SDL_ResetMouse();
    }
#endif

    /* See if the current window has lost focus */
    if (mouse->focus) {
        SDL_SendWindowEvent(mouse->focus, SDL_WINDOWEVENT_LEAVE, 0, 0);
    }

    mouse->focus = window;
    mouse->has_position = SDL_FALSE;

    if (mouse->focus) {
        SDL_SendWindowEvent(mouse->focus, SDL_WINDOWEVENT_ENTER, 0, 0);
    }

    /* Update cursor visibility */
    SDL_SetCursor(NULL);
}

/* Check to see if we need to synthesize focus events */
static SDL_bool
SDL_UpdateMouseFocus(SDL_Window * window, int x, int y, Uint32 buttonstate)
{
    SDL_Mouse *mouse = SDL_GetMouse();
    SDL_bool inWindow = SDL_TRUE;

    if (window && ((window->flags & SDL_WINDOW_MOUSE_CAPTURE) == 0)) {
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        if (x < 0 || y < 0 || x >= w || y >= h) {
            inWindow = SDL_FALSE;
        }
    }

/* Linux doesn't give you mouse events outside your window unless you grab
   the pointer.

   Windows doesn't give you mouse events outside your window unless you call
   SetCapture().

   Both of these are slightly scary changes, so for now we'll punt and if the
   mouse leaves the window you'll lose mouse focus and reset button state.
*/
#ifdef SUPPORT_DRAG_OUTSIDE_WINDOW
    if (!inWindow && !buttonstate) {
#else
    if (!inWindow) {
#endif
        if (window == mouse->focus) {
#ifdef DEBUG_MOUSE
            printf("Mouse left window, synthesizing move & focus lost event\n");
#endif
            SDL_PrivateSendMouseMotion(window, mouse->mouseID, 0, x, y);
            SDL_SetMouseFocus(NULL);
        }
        return SDL_FALSE;
    }

    if (window != mouse->focus) {
#ifdef DEBUG_MOUSE
        printf("Mouse entered window, synthesizing focus gain & move event\n");
#endif
        SDL_SetMouseFocus(window);
        SDL_PrivateSendMouseMotion(window, mouse->mouseID, 0, x, y);
    }
    return SDL_TRUE;
}

int
SDL_SendMouseMotion(SDL_Window * window, SDL_MouseID mouseID, int relative, int x, int y)
{
    if (window && !relative) {
        SDL_Mouse *mouse = SDL_GetMouse();
        if (!SDL_UpdateMouseFocus(window, x, y, mouse->buttonstate)) {
            return 0;
        }
    }

    return SDL_PrivateSendMouseMotion(window, mouseID, relative, x, y);
}

static int
GetScaledMouseDelta(float scale, int value, float *accum)
{
    if (scale != 1.0f) {
        *accum += scale * value;
        if (*accum >= 0.0f) {
            value = (int)SDL_floor(*accum);
        } else {
            value = (int)SDL_ceil(*accum);
        }
        *accum -= value;
    }
    return value;
}

static int
SDL_PrivateSendMouseMotion(SDL_Window * window, SDL_MouseID mouseID, int relative, int x, int y)
{
    SDL_Mouse *mouse = SDL_GetMouse();
    int posted;
    int xrel;
    int yrel;

    if (mouseID == SDL_TOUCH_MOUSEID && !mouse->touch_mouse_events) {
        return 0;
    }

    if (mouseID != SDL_TOUCH_MOUSEID && mouse->relative_mode_warp) {
        int center_x = 0, center_y = 0;
        SDL_GetWindowSize(window, &center_x, &center_y);
        center_x /= 2;
        center_y /= 2;
        if (x == center_x && y == center_y) {
            mouse->last_x = center_x;
            mouse->last_y = center_y;
            return 0;
        }
        SDL_WarpMouseInWindow(window, center_x, center_y);
    }

    if (relative) {
        if (mouse->relative_mode) {
            x = GetScaledMouseDelta(mouse->relative_speed_scale, x, &mouse->scale_accum_x);
            y = GetScaledMouseDelta(mouse->relative_speed_scale, y, &mouse->scale_accum_y);
        } else {
            x = GetScaledMouseDelta(mouse->normal_speed_scale, x, &mouse->scale_accum_x);
            y = GetScaledMouseDelta(mouse->normal_speed_scale, y, &mouse->scale_accum_y);
        }
        xrel = x;
        yrel = y;
        x = (mouse->last_x + xrel);
        y = (mouse->last_y + yrel);
    } else {
        xrel = x - mouse->last_x;
        yrel = y - mouse->last_y;
    }

    /* Drop events that don't change state */
    if (!xrel && !yrel) {
#ifdef DEBUG_MOUSE
        printf("Mouse event didn't change state - dropped!\n");
#endif
        return 0;
    }

    /* Ignore relative motion when first positioning the mouse */
    if (!mouse->has_position) {
        xrel = 0;
        yrel = 0;
        mouse->has_position = SDL_TRUE;
    }

    /* Ignore relative motion positioning the first touch */
    if (mouseID == SDL_TOUCH_MOUSEID && !mouse->buttonstate) {
        xrel = 0;
        yrel = 0;
    }

    /* Update internal mouse coordinates */
    if (!mouse->relative_mode) {
        mouse->x = x;
        mouse->y = y;
    } else {
        mouse->x += xrel;
        mouse->y += yrel;
    }

    /* make sure that the pointers find themselves inside the windows,
       unless we have the mouse captured. */
    if (window && ((window->flags & SDL_WINDOW_MOUSE_CAPTURE) == 0)) {
        int x_max = 0, y_max = 0;

        /* !!! FIXME: shouldn't this be (window) instead of (mouse->focus)? */
        SDL_GetWindowSize(mouse->focus, &x_max, &y_max);
        --x_max;
        --y_max;

        if (mouse->x > x_max) {
            mouse->x = x_max;
        }
        if (mouse->x < 0) {
            mouse->x = 0;
        }

        if (mouse->y > y_max) {
            mouse->y = y_max;
        }
        if (mouse->y < 0) {
            mouse->y = 0;
        }
    }

    mouse->xdelta += xrel;
    mouse->ydelta += yrel;

    /* Move the mouse cursor, if needed */
    if (mouse->cursor_shown && !mouse->relative_mode &&
        mouse->MoveCursor && mouse->cur_cursor) {
        mouse->MoveCursor(mouse->cur_cursor);
    }

    /* Post the event, if desired */
    posted = 0;
    if (SDL_GetEventState(SDL_MOUSEMOTION) == SDL_ENABLE) {
        SDL_Event event;
        event.motion.type = SDL_MOUSEMOTION;
        event.motion.windowID = mouse->focus ? mouse->focus->id : 0;
        event.motion.which = mouseID;
        event.motion.state = mouse->buttonstate;
        event.motion.x = mouse->x;
        event.motion.y = mouse->y;
        event.motion.xrel = xrel;
        event.motion.yrel = yrel;
        posted = (SDL_PushEvent(&event) > 0);
    }
    if (relative) {
        mouse->last_x = mouse->x;
        mouse->last_y = mouse->y;
    } else {
        /* Use unclamped values if we're getting events outside the window */
        mouse->last_x = x;
        mouse->last_y = y;
    }
    return posted;
}

static SDL_MouseClickState *GetMouseClickState(SDL_Mouse *mouse, Uint8 button)
{
    if (button >= mouse->num_clickstates) {
        int i, count = button + 1;
        SDL_MouseClickState *clickstate = (SDL_MouseClickState *)SDL_realloc(mouse->clickstate, count * sizeof(*mouse->clickstate));
        if (!clickstate) {
            return NULL;
        }
        mouse->clickstate = clickstate;

        for (i = mouse->num_clickstates; i < count; ++i) {
            SDL_zero(mouse->clickstate[i]);
        }
        mouse->num_clickstates = count;
    }
    return &mouse->clickstate[button];
}

static int
SDL_PrivateSendMouseButton(SDL_Window * window, SDL_MouseID mouseID, Uint8 state, Uint8 button, int clicks)
{
    SDL_Mouse *mouse = SDL_GetMouse();
    int posted;
    Uint32 type;
    Uint32 buttonstate = mouse->buttonstate;

    if (mouseID == SDL_TOUCH_MOUSEID && !mouse->touch_mouse_events) {
        return 0;
    }

    /* Figure out which event to perform */
    switch (state) {
    case SDL_PRESSED:
        type = SDL_MOUSEBUTTONDOWN;
        buttonstate |= SDL_BUTTON(button);
        break;
    case SDL_RELEASED:
        type = SDL_MOUSEBUTTONUP;
        buttonstate &= ~SDL_BUTTON(button);
        break;
    default:
        /* Invalid state -- bail */
        return 0;
    }

    /* We do this after calculating buttonstate so button presses gain focus */
    if (window && state == SDL_PRESSED) {
        SDL_UpdateMouseFocus(window, mouse->x, mouse->y, buttonstate);
    }

    if (buttonstate == mouse->buttonstate) {
        /* Ignore this event, no state change */
        return 0;
    }
    mouse->buttonstate = buttonstate;

    if (clicks < 0) {
        SDL_MouseClickState *clickstate = GetMouseClickState(mouse, button);
        if (clickstate) {
            if (state == SDL_PRESSED) {
                Uint32 now = SDL_GetTicks();

                if (SDL_TICKS_PASSED(now, clickstate->last_timestamp + SDL_double_click_time) ||
                    SDL_abs(mouse->x - clickstate->last_x) > SDL_double_click_radius ||
                    SDL_abs(mouse->y - clickstate->last_y) > SDL_double_click_radius) {
                    clickstate->click_count = 0;
                }
                clickstate->last_timestamp = now;
                clickstate->last_x = mouse->x;
                clickstate->last_y = mouse->y;
                if (clickstate->click_count < 255) {
                    ++clickstate->click_count;
                }
            }
            clicks = clickstate->click_count;
        } else {
            clicks = 1;
        }
    }

    /* Post the event, if desired */
    posted = 0;
    if (SDL_GetEventState(type) == SDL_ENABLE) {
        SDL_Event event;
        event.type = type;
        event.button.windowID = mouse->focus ? mouse->focus->id : 0;
        event.button.which = mouseID;
        event.button.state = state;
        event.button.button = button;
        event.button.clicks = (Uint8) SDL_min(clicks, 255);
        event.button.x = mouse->x;
        event.button.y = mouse->y;
        posted = (SDL_PushEvent(&event) > 0);
    }

    /* We do this after dispatching event so button releases can lose focus */
    if (window && state == SDL_RELEASED) {
        SDL_UpdateMouseFocus(window, mouse->x, mouse->y, buttonstate);
    }
    
    return posted;
}

int
SDL_SendMouseButtonClicks(SDL_Window * window, SDL_MouseID mouseID, Uint8 state, Uint8 button, int clicks)
{
    clicks = SDL_max(clicks, 0);
    return SDL_PrivateSendMouseButton(window, mouseID, state, button, clicks);
}

int
SDL_SendMouseButton(SDL_Window * window, SDL_MouseID mouseID, Uint8 state, Uint8 button)
{
    return SDL_PrivateSendMouseButton(window, mouseID, state, button, -1);
}

int
SDL_SendMouseWheel(SDL_Window * window, SDL_MouseID mouseID, float x, float y, SDL_MouseWheelDirection direction)
{
    SDL_Mouse *mouse = SDL_GetMouse();
    int posted;
    int integral_x, integral_y;

    if (window) {
        SDL_SetMouseFocus(window);
    }

    if (!x && !y) {
        return 0;
    }

    mouse->accumulated_wheel_x += x;
    if (mouse->accumulated_wheel_x > 0) {
        integral_x = (int)SDL_floor(mouse->accumulated_wheel_x);
    } else if (mouse->accumulated_wheel_x < 0) {
        integral_x = (int)SDL_ceil(mouse->accumulated_wheel_x);
    } else {
        integral_x = 0;
    }
    mouse->accumulated_wheel_x -= integral_x;

    mouse->accumulated_wheel_y += y;
    if (mouse->accumulated_wheel_y > 0) {
        integral_y = (int)SDL_floor(mouse->accumulated_wheel_y);
    } else if (mouse->accumulated_wheel_y < 0) {
        integral_y = (int)SDL_ceil(mouse->accumulated_wheel_y);
    } else {
        integral_y = 0;
    }
    mouse->accumulated_wheel_y -= integral_y;

    /* Post the event, if desired */
    posted = 0;
    if (SDL_GetEventState(SDL_MOUSEWHEEL) == SDL_ENABLE) {
        SDL_Event event;
        event.type = SDL_MOUSEWHEEL;
        event.wheel.windowID = mouse->focus ? mouse->focus->id : 0;
        event.wheel.which = mouseID;
#if 0 /* Uncomment this when it goes in for SDL 2.1 */
        event.wheel.preciseX = x;
        event.wheel.preciseY = y;
#endif
        event.wheel.x = integral_x;
        event.wheel.y = integral_y;
        event.wheel.direction = (Uint32)direction;
        posted = (SDL_PushEvent(&event) > 0);
    }
    return posted;
}

void
SDL_MouseQuit(void)
{
    SDL_Cursor *cursor, *next;
    SDL_Mouse *mouse = SDL_GetMouse();

    if (mouse->CaptureMouse) {
        SDL_CaptureMouse(SDL_FALSE);
    }
    SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_ShowCursor(1);

    cursor = mouse->cursors;
    while (cursor) {
        next = cursor->next;
        SDL_FreeCursor(cursor);
        cursor = next;
    }
    mouse->cursors = NULL;

    if (mouse->def_cursor && mouse->FreeCursor) {
        mouse->FreeCursor(mouse->def_cursor);
        mouse->def_cursor = NULL;
    }

    if (mouse->clickstate) {
        SDL_free(mouse->clickstate);
        mouse->clickstate = NULL;
    }

    SDL_DelHintCallback(SDL_HINT_MOUSE_NORMAL_SPEED_SCALE,
                        SDL_MouseNormalSpeedScaleChanged, mouse);

    SDL_DelHintCallback(SDL_HINT_MOUSE_RELATIVE_SPEED_SCALE,
                        SDL_MouseRelativeSpeedScaleChanged, mouse);
}

Uint32
SDL_GetMouseState(int *x, int *y)
{
    SDL_Mouse *mouse = SDL_GetMouse();

    if (x) {
        *x = mouse->x;
    }
    if (y) {
        *y = mouse->y;
    }
    return mouse->buttonstate;
}

Uint32
SDL_GetRelativeMouseState(int *x, int *y)
{
    SDL_Mouse *mouse = SDL_GetMouse();

    if (x) {
        *x = mouse->xdelta;
    }
    if (y) {
        *y = mouse->ydelta;
    }
    mouse->xdelta = 0;
    mouse->ydelta = 0;
    return mouse->buttonstate;
}

Uint32
SDL_GetGlobalMouseState(int *x, int *y)
{
    SDL_Mouse *mouse = SDL_GetMouse();
    int tmpx, tmpy;

    /* make sure these are never NULL for the backend implementations... */
    if (!x) {
        x = &tmpx;
    }
    if (!y) {
        y = &tmpy;
    }

    *x = *y = 0;

    if (!mouse->GetGlobalMouseState) {
        return 0;
    }

    return mouse->GetGlobalMouseState(x, y);
}

void
SDL_WarpMouseInWindow(SDL_Window * window, int x, int y)
{
    SDL_Mouse *mouse = SDL_GetMouse();

    if (window == NULL) {
        window = mouse->focus;
    }

    if (window == NULL) {
        return;
    }

    if (mouse->WarpMouse) {
        mouse->WarpMouse(window, x, y);
    } else {
        SDL_SendMouseMotion(window, mouse->mouseID, 0, x, y);
    }
}

int
SDL_WarpMouseGlobal(int x, int y)
{
    SDL_Mouse *mouse = SDL_GetMouse();

    if (mouse->WarpMouseGlobal) {
        return mouse->WarpMouseGlobal(x, y);
    }

    return SDL_Unsupported();
}

static SDL_bool
ShouldUseRelativeModeWarp(SDL_Mouse *mouse)
{
    if (!mouse->SetRelativeMouseMode) {
        return SDL_TRUE;
    }

    return SDL_GetHintBoolean(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, SDL_FALSE);
}

int
SDL_SetRelativeMouseMode(SDL_bool enabled)
{
    SDL_Mouse *mouse = SDL_GetMouse();
    SDL_Window *focusWindow = SDL_GetKeyboardFocus();

    if (enabled == mouse->relative_mode) {
        return 0;
    }

    if (enabled && focusWindow) {
        /* Center it in the focused window to prevent clicks from going through
         * to background windows.
         */
        SDL_SetMouseFocus(focusWindow);
        SDL_WarpMouseInWindow(focusWindow, focusWindow->w/2, focusWindow->h/2);
    }

    /* Set the relative mode */
    if (!enabled && mouse->relative_mode_warp) {
        mouse->relative_mode_warp = SDL_FALSE;
    } else if (enabled && ShouldUseRelativeModeWarp(mouse)) {
        mouse->relative_mode_warp = SDL_TRUE;
    } else if (mouse->SetRelativeMouseMode(enabled) < 0) {
        if (enabled) {
            /* Fall back to warp mode if native relative mode failed */
            mouse->relative_mode_warp = SDL_TRUE;
        }
    }
    mouse->relative_mode = enabled;
    mouse->scale_accum_x = 0.0f;
    mouse->scale_accum_y = 0.0f;

    if (mouse->focus) {
        SDL_UpdateWindowGrab(mouse->focus);

        /* Put the cursor back to where the application expects it */
        if (!enabled) {
            SDL_WarpMouseInWindow(mouse->focus, mouse->x, mouse->y);
        }
    }

    /* Flush pending mouse motion - ideally we would pump events, but that's not always safe */
    SDL_FlushEvent(SDL_MOUSEMOTION);

    /* Update cursor visibility */
    SDL_SetCursor(NULL);

    return 0;
}

SDL_bool
SDL_GetRelativeMouseMode()
{
    SDL_Mouse *mouse = SDL_GetMouse();

    return mouse->relative_mode;
}

int
SDL_CaptureMouse(SDL_bool enabled)
{
    SDL_Mouse *mouse = SDL_GetMouse();
    SDL_Window *focusWindow;
    SDL_bool isCaptured;

    if (!mouse->CaptureMouse) {
        return SDL_Unsupported();
    }

    focusWindow = SDL_GetKeyboardFocus();

    isCaptured = focusWindow && (focusWindow->flags & SDL_WINDOW_MOUSE_CAPTURE);
    if (isCaptured == enabled) {
        return 0;  /* already done! */
    }

    if (enabled) {
        if (!focusWindow) {
            return SDL_SetError("No window has focus");
        } else if (mouse->CaptureMouse(focusWindow) == -1) {
            return -1;  /* CaptureMouse() should call SetError */
        }
        focusWindow->flags |= SDL_WINDOW_MOUSE_CAPTURE;
    } else {
        if (mouse->CaptureMouse(NULL) == -1) {
            return -1;  /* CaptureMouse() should call SetError */
        }
        focusWindow->flags &= ~SDL_WINDOW_MOUSE_CAPTURE;
    }

    return 0;
}

SDL_Cursor *
SDL_CreateCursor(const Uint8 * data, const Uint8 * mask,
                 int w, int h, int hot_x, int hot_y)
{
    SDL_Surface *surface;
    SDL_Cursor *cursor;
    int x, y;
    Uint32 *pixel;
    Uint8 datab = 0, maskb = 0;
    const Uint32 black = 0xFF000000;
    const Uint32 white = 0xFFFFFFFF;
    const Uint32 transparent = 0x00000000;

    /* Make sure the width is a multiple of 8 */
    w = ((w + 7) & ~7);

    /* Create the surface from a bitmap */
    surface = SDL_CreateRGBSurface(0, w, h, 32,
                                   0x00FF0000,
                                   0x0000FF00,
                                   0x000000FF,
                                   0xFF000000);
    if (!surface) {
        return NULL;
    }
    for (y = 0; y < h; ++y) {
        pixel = (Uint32 *) ((Uint8 *) surface->pixels + y * surface->pitch);
        for (x = 0; x < w; ++x) {
            if ((x % 8) == 0) {
                datab = *data++;
                maskb = *mask++;
            }
            if (maskb & 0x80) {
                *pixel++ = (datab & 0x80) ? black : white;
            } else {
                *pixel++ = (datab & 0x80) ? black : transparent;
            }
            datab <<= 1;
            maskb <<= 1;
        }
    }

    cursor = SDL_CreateColorCursor(surface, hot_x, hot_y);

    SDL_FreeSurface(surface);

    return cursor;
}

SDL_Cursor *
SDL_CreateColorCursor(SDL_Surface *surface, int hot_x, int hot_y)
{
    SDL_Mouse *mouse = SDL_GetMouse();
    SDL_Surface *temp = NULL;
    SDL_Cursor *cursor;

    if (!surface) {
        SDL_SetError("Passed NULL cursor surface");
        return NULL;
    }

    if (!mouse->CreateCursor) {
        SDL_SetError("Cursors are not currently supported");
        return NULL;
    }

    /* Sanity check the hot spot */
    if ((hot_x < 0) || (hot_y < 0) ||
        (hot_x >= surface->w) || (hot_y >= surface->h)) {
        SDL_SetError("Cursor hot spot doesn't lie within cursor");
        return NULL;
    }

    if (surface->format->format != SDL_PIXELFORMAT_ARGB8888) {
        temp = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ARGB8888, 0);
        if (!temp) {
            return NULL;
        }
        surface = temp;
    }

    cursor = mouse->CreateCursor(surface, hot_x, hot_y);
    if (cursor) {
        cursor->next = mouse->cursors;
        mouse->cursors = cursor;
    }

    SDL_FreeSurface(temp);

    return cursor;
}

SDL_Cursor *
SDL_CreateSystemCursor(SDL_SystemCursor id)
{
    SDL_Mouse *mouse = SDL_GetMouse();
    SDL_Cursor *cursor;

    if (!mouse->CreateSystemCursor) {
        SDL_SetError("CreateSystemCursor is not currently supported");
        return NULL;
    }

    cursor = mouse->CreateSystemCursor(id);
    if (cursor) {
        cursor->next = mouse->cursors;
        mouse->cursors = cursor;
    }

    return cursor;
}

/* SDL_SetCursor(NULL) can be used to force the cursor redraw,
   if this is desired for any reason.  This is used when setting
   the video mode and when the SDL window gains the mouse focus.
 */
void
SDL_SetCursor(SDL_Cursor * cursor)
{
    SDL_Mouse *mouse = SDL_GetMouse();

    /* Set the new cursor */
    if (cursor) {
        /* Make sure the cursor is still valid for this mouse */
        if (cursor != mouse->def_cursor) {
            SDL_Cursor *found;
            for (found = mouse->cursors; found; found = found->next) {
                if (found == cursor) {
                    break;
                }
            }
            if (!found) {
                SDL_SetError("Cursor not associated with the current mouse");
                return;
            }
        }
        mouse->cur_cursor = cursor;
    } else {
        if (mouse->focus) {
            cursor = mouse->cur_cursor;
        } else {
            cursor = mouse->def_cursor;
        }
    }

    if (cursor && mouse->cursor_shown && !mouse->relative_mode) {
        if (mouse->ShowCursor) {
            mouse->ShowCursor(cursor);
        }
    } else {
        if (mouse->ShowCursor) {
            mouse->ShowCursor(NULL);
        }
    }
}

SDL_Cursor *
SDL_GetCursor(void)
{
    SDL_Mouse *mouse = SDL_GetMouse();

    if (!mouse) {
        return NULL;
    }
    return mouse->cur_cursor;
}

SDL_Cursor *
SDL_GetDefaultCursor(void)
{
    SDL_Mouse *mouse = SDL_GetMouse();

    if (!mouse) {
        return NULL;
    }
    return mouse->def_cursor;
}

void
SDL_FreeCursor(SDL_Cursor * cursor)
{
    SDL_Mouse *mouse = SDL_GetMouse();
    SDL_Cursor *curr, *prev;

    if (!cursor) {
        return;
    }

    if (cursor == mouse->def_cursor) {
        return;
    }
    if (cursor == mouse->cur_cursor) {
        SDL_SetCursor(mouse->def_cursor);
    }

    for (prev = NULL, curr = mouse->cursors; curr;
         prev = curr, curr = curr->next) {
        if (curr == cursor) {
            if (prev) {
                prev->next = curr->next;
            } else {
                mouse->cursors = curr->next;
            }

            if (mouse->FreeCursor) {
                mouse->FreeCursor(curr);
            }
            return;
        }
    }
}

int
SDL_ShowCursor(int toggle)
{
    SDL_Mouse *mouse = SDL_GetMouse();
    SDL_bool shown;

    if (!mouse) {
        return 0;
    }

    shown = mouse->cursor_shown;
    if (toggle >= 0) {
        if (toggle) {
            mouse->cursor_shown = SDL_TRUE;
        } else {
            mouse->cursor_shown = SDL_FALSE;
        }
        if (mouse->cursor_shown != shown) {
            SDL_SetCursor(NULL);
        }
    }
    return shown;
}

/* vi: set ts=4 sw=4 expandtab: */
