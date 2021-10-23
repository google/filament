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

#if SDL_VIDEO_DRIVER_EMSCRIPTEN

#include <emscripten/html5.h>

#include "../../events/SDL_events_c.h"
#include "../../events/SDL_keyboard_c.h"
#include "../../events/SDL_touch_c.h"

#include "SDL_emscriptenevents.h"
#include "SDL_emscriptenvideo.h"

#include "SDL_hints.h"

#define FULLSCREEN_MASK ( SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_FULLSCREEN )

/*
.keyCode to scancode
https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent
https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent/keyCode
*/
static const SDL_Scancode emscripten_scancode_table[] = {
    /*  0 */    SDL_SCANCODE_UNKNOWN,
    /*  1 */    SDL_SCANCODE_UNKNOWN,
    /*  2 */    SDL_SCANCODE_UNKNOWN,
    /*  3 */    SDL_SCANCODE_CANCEL,
    /*  4 */    SDL_SCANCODE_UNKNOWN,
    /*  5 */    SDL_SCANCODE_UNKNOWN,
    /*  6 */    SDL_SCANCODE_HELP,
    /*  7 */    SDL_SCANCODE_UNKNOWN,
    /*  8 */    SDL_SCANCODE_BACKSPACE,
    /*  9 */    SDL_SCANCODE_TAB,
    /*  10 */   SDL_SCANCODE_UNKNOWN,
    /*  11 */   SDL_SCANCODE_UNKNOWN,
    /*  12 */   SDL_SCANCODE_KP_5,
    /*  13 */   SDL_SCANCODE_RETURN,
    /*  14 */   SDL_SCANCODE_UNKNOWN,
    /*  15 */   SDL_SCANCODE_UNKNOWN,
    /*  16 */   SDL_SCANCODE_LSHIFT,
    /*  17 */   SDL_SCANCODE_LCTRL,
    /*  18 */   SDL_SCANCODE_LALT,
    /*  19 */   SDL_SCANCODE_PAUSE,
    /*  20 */   SDL_SCANCODE_CAPSLOCK,
    /*  21 */   SDL_SCANCODE_UNKNOWN,
    /*  22 */   SDL_SCANCODE_UNKNOWN,
    /*  23 */   SDL_SCANCODE_UNKNOWN,
    /*  24 */   SDL_SCANCODE_UNKNOWN,
    /*  25 */   SDL_SCANCODE_UNKNOWN,
    /*  26 */   SDL_SCANCODE_UNKNOWN,
    /*  27 */   SDL_SCANCODE_ESCAPE,
    /*  28 */   SDL_SCANCODE_UNKNOWN,
    /*  29 */   SDL_SCANCODE_UNKNOWN,
    /*  30 */   SDL_SCANCODE_UNKNOWN,
    /*  31 */   SDL_SCANCODE_UNKNOWN,
    /*  32 */   SDL_SCANCODE_SPACE,
    /*  33 */   SDL_SCANCODE_PAGEUP,
    /*  34 */   SDL_SCANCODE_PAGEDOWN,
    /*  35 */   SDL_SCANCODE_END,
    /*  36 */   SDL_SCANCODE_HOME,
    /*  37 */   SDL_SCANCODE_LEFT,
    /*  38 */   SDL_SCANCODE_UP,
    /*  39 */   SDL_SCANCODE_RIGHT,
    /*  40 */   SDL_SCANCODE_DOWN,
    /*  41 */   SDL_SCANCODE_UNKNOWN,
    /*  42 */   SDL_SCANCODE_UNKNOWN,
    /*  43 */   SDL_SCANCODE_UNKNOWN,
    /*  44 */   SDL_SCANCODE_UNKNOWN,
    /*  45 */   SDL_SCANCODE_INSERT,
    /*  46 */   SDL_SCANCODE_DELETE,
    /*  47 */   SDL_SCANCODE_UNKNOWN,
    /*  48 */   SDL_SCANCODE_0,
    /*  49 */   SDL_SCANCODE_1,
    /*  50 */   SDL_SCANCODE_2,
    /*  51 */   SDL_SCANCODE_3,
    /*  52 */   SDL_SCANCODE_4,
    /*  53 */   SDL_SCANCODE_5,
    /*  54 */   SDL_SCANCODE_6,
    /*  55 */   SDL_SCANCODE_7,
    /*  56 */   SDL_SCANCODE_8,
    /*  57 */   SDL_SCANCODE_9,
    /*  58 */   SDL_SCANCODE_UNKNOWN,
    /*  59 */   SDL_SCANCODE_SEMICOLON,
    /*  60 */   SDL_SCANCODE_NONUSBACKSLASH,
    /*  61 */   SDL_SCANCODE_EQUALS,
    /*  62 */   SDL_SCANCODE_UNKNOWN,
    /*  63 */   SDL_SCANCODE_MINUS,
    /*  64 */   SDL_SCANCODE_UNKNOWN,
    /*  65 */   SDL_SCANCODE_A,
    /*  66 */   SDL_SCANCODE_B,
    /*  67 */   SDL_SCANCODE_C,
    /*  68 */   SDL_SCANCODE_D,
    /*  69 */   SDL_SCANCODE_E,
    /*  70 */   SDL_SCANCODE_F,
    /*  71 */   SDL_SCANCODE_G,
    /*  72 */   SDL_SCANCODE_H,
    /*  73 */   SDL_SCANCODE_I,
    /*  74 */   SDL_SCANCODE_J,
    /*  75 */   SDL_SCANCODE_K,
    /*  76 */   SDL_SCANCODE_L,
    /*  77 */   SDL_SCANCODE_M,
    /*  78 */   SDL_SCANCODE_N,
    /*  79 */   SDL_SCANCODE_O,
    /*  80 */   SDL_SCANCODE_P,
    /*  81 */   SDL_SCANCODE_Q,
    /*  82 */   SDL_SCANCODE_R,
    /*  83 */   SDL_SCANCODE_S,
    /*  84 */   SDL_SCANCODE_T,
    /*  85 */   SDL_SCANCODE_U,
    /*  86 */   SDL_SCANCODE_V,
    /*  87 */   SDL_SCANCODE_W,
    /*  88 */   SDL_SCANCODE_X,
    /*  89 */   SDL_SCANCODE_Y,
    /*  90 */   SDL_SCANCODE_Z,
    /*  91 */   SDL_SCANCODE_LGUI,
    /*  92 */   SDL_SCANCODE_UNKNOWN,
    /*  93 */   SDL_SCANCODE_APPLICATION,
    /*  94 */   SDL_SCANCODE_UNKNOWN,
    /*  95 */   SDL_SCANCODE_UNKNOWN,
    /*  96 */   SDL_SCANCODE_KP_0,
    /*  97 */   SDL_SCANCODE_KP_1,
    /*  98 */   SDL_SCANCODE_KP_2,
    /*  99 */   SDL_SCANCODE_KP_3,
    /* 100 */   SDL_SCANCODE_KP_4,
    /* 101 */   SDL_SCANCODE_KP_5,
    /* 102 */   SDL_SCANCODE_KP_6,
    /* 103 */   SDL_SCANCODE_KP_7,
    /* 104 */   SDL_SCANCODE_KP_8,
    /* 105 */   SDL_SCANCODE_KP_9,
    /* 106 */   SDL_SCANCODE_KP_MULTIPLY,
    /* 107 */   SDL_SCANCODE_KP_PLUS,
    /* 108 */   SDL_SCANCODE_UNKNOWN,
    /* 109 */   SDL_SCANCODE_KP_MINUS,
    /* 110 */   SDL_SCANCODE_KP_PERIOD,
    /* 111 */   SDL_SCANCODE_KP_DIVIDE,
    /* 112 */   SDL_SCANCODE_F1,
    /* 113 */   SDL_SCANCODE_F2,
    /* 114 */   SDL_SCANCODE_F3,
    /* 115 */   SDL_SCANCODE_F4,
    /* 116 */   SDL_SCANCODE_F5,
    /* 117 */   SDL_SCANCODE_F6,
    /* 118 */   SDL_SCANCODE_F7,
    /* 119 */   SDL_SCANCODE_F8,
    /* 120 */   SDL_SCANCODE_F9,
    /* 121 */   SDL_SCANCODE_F10,
    /* 122 */   SDL_SCANCODE_F11,
    /* 123 */   SDL_SCANCODE_F12,
    /* 124 */   SDL_SCANCODE_F13,
    /* 125 */   SDL_SCANCODE_F14,
    /* 126 */   SDL_SCANCODE_F15,
    /* 127 */   SDL_SCANCODE_F16,
    /* 128 */   SDL_SCANCODE_F17,
    /* 129 */   SDL_SCANCODE_F18,
    /* 130 */   SDL_SCANCODE_F19,
    /* 131 */   SDL_SCANCODE_F20,
    /* 132 */   SDL_SCANCODE_F21,
    /* 133 */   SDL_SCANCODE_F22,
    /* 134 */   SDL_SCANCODE_F23,
    /* 135 */   SDL_SCANCODE_F24,
    /* 136 */   SDL_SCANCODE_UNKNOWN,
    /* 137 */   SDL_SCANCODE_UNKNOWN,
    /* 138 */   SDL_SCANCODE_UNKNOWN,
    /* 139 */   SDL_SCANCODE_UNKNOWN,
    /* 140 */   SDL_SCANCODE_UNKNOWN,
    /* 141 */   SDL_SCANCODE_UNKNOWN,
    /* 142 */   SDL_SCANCODE_UNKNOWN,
    /* 143 */   SDL_SCANCODE_UNKNOWN,
    /* 144 */   SDL_SCANCODE_NUMLOCKCLEAR,
    /* 145 */   SDL_SCANCODE_SCROLLLOCK,
    /* 146 */   SDL_SCANCODE_UNKNOWN,
    /* 147 */   SDL_SCANCODE_UNKNOWN,
    /* 148 */   SDL_SCANCODE_UNKNOWN,
    /* 149 */   SDL_SCANCODE_UNKNOWN,
    /* 150 */   SDL_SCANCODE_UNKNOWN,
    /* 151 */   SDL_SCANCODE_UNKNOWN,
    /* 152 */   SDL_SCANCODE_UNKNOWN,
    /* 153 */   SDL_SCANCODE_UNKNOWN,
    /* 154 */   SDL_SCANCODE_UNKNOWN,
    /* 155 */   SDL_SCANCODE_UNKNOWN,
    /* 156 */   SDL_SCANCODE_UNKNOWN,
    /* 157 */   SDL_SCANCODE_UNKNOWN,
    /* 158 */   SDL_SCANCODE_UNKNOWN,
    /* 159 */   SDL_SCANCODE_UNKNOWN,
    /* 160 */   SDL_SCANCODE_GRAVE,
    /* 161 */   SDL_SCANCODE_UNKNOWN,
    /* 162 */   SDL_SCANCODE_UNKNOWN,
    /* 163 */   SDL_SCANCODE_KP_HASH, /*KaiOS phone keypad*/
    /* 164 */   SDL_SCANCODE_UNKNOWN,
    /* 165 */   SDL_SCANCODE_UNKNOWN,
    /* 166 */   SDL_SCANCODE_UNKNOWN,
    /* 167 */   SDL_SCANCODE_UNKNOWN,
    /* 168 */   SDL_SCANCODE_UNKNOWN,
    /* 169 */   SDL_SCANCODE_UNKNOWN,
    /* 170 */   SDL_SCANCODE_KP_MULTIPLY, /*KaiOS phone keypad*/
    /* 171 */   SDL_SCANCODE_RIGHTBRACKET,
    /* 172 */   SDL_SCANCODE_UNKNOWN,
    /* 173 */   SDL_SCANCODE_MINUS, /*FX*/
    /* 174 */   SDL_SCANCODE_VOLUMEDOWN, /*IE, Chrome*/
    /* 175 */   SDL_SCANCODE_VOLUMEUP, /*IE, Chrome*/
    /* 176 */   SDL_SCANCODE_AUDIONEXT, /*IE, Chrome*/
    /* 177 */   SDL_SCANCODE_AUDIOPREV, /*IE, Chrome*/
    /* 178 */   SDL_SCANCODE_UNKNOWN,
    /* 179 */   SDL_SCANCODE_AUDIOPLAY, /*IE, Chrome*/
    /* 180 */   SDL_SCANCODE_UNKNOWN,
    /* 181 */   SDL_SCANCODE_AUDIOMUTE, /*FX*/
    /* 182 */   SDL_SCANCODE_VOLUMEDOWN, /*FX*/
    /* 183 */   SDL_SCANCODE_VOLUMEUP, /*FX*/
    /* 184 */   SDL_SCANCODE_UNKNOWN,
    /* 185 */   SDL_SCANCODE_UNKNOWN,
    /* 186 */   SDL_SCANCODE_SEMICOLON, /*IE, Chrome, D3E legacy*/
    /* 187 */   SDL_SCANCODE_EQUALS, /*IE, Chrome, D3E legacy*/
    /* 188 */   SDL_SCANCODE_COMMA,
    /* 189 */   SDL_SCANCODE_MINUS, /*IE, Chrome, D3E legacy*/
    /* 190 */   SDL_SCANCODE_PERIOD,
    /* 191 */   SDL_SCANCODE_SLASH,
    /* 192 */   SDL_SCANCODE_GRAVE, /*FX, D3E legacy (SDL_SCANCODE_APOSTROPHE in IE/Chrome)*/
    /* 193 */   SDL_SCANCODE_UNKNOWN,
    /* 194 */   SDL_SCANCODE_UNKNOWN,
    /* 195 */   SDL_SCANCODE_UNKNOWN,
    /* 196 */   SDL_SCANCODE_UNKNOWN,
    /* 197 */   SDL_SCANCODE_UNKNOWN,
    /* 198 */   SDL_SCANCODE_UNKNOWN,
    /* 199 */   SDL_SCANCODE_UNKNOWN,
    /* 200 */   SDL_SCANCODE_UNKNOWN,
    /* 201 */   SDL_SCANCODE_UNKNOWN,
    /* 202 */   SDL_SCANCODE_UNKNOWN,
    /* 203 */   SDL_SCANCODE_UNKNOWN,
    /* 204 */   SDL_SCANCODE_UNKNOWN,
    /* 205 */   SDL_SCANCODE_UNKNOWN,
    /* 206 */   SDL_SCANCODE_UNKNOWN,
    /* 207 */   SDL_SCANCODE_UNKNOWN,
    /* 208 */   SDL_SCANCODE_UNKNOWN,
    /* 209 */   SDL_SCANCODE_UNKNOWN,
    /* 210 */   SDL_SCANCODE_UNKNOWN,
    /* 211 */   SDL_SCANCODE_UNKNOWN,
    /* 212 */   SDL_SCANCODE_UNKNOWN,
    /* 213 */   SDL_SCANCODE_UNKNOWN,
    /* 214 */   SDL_SCANCODE_UNKNOWN,
    /* 215 */   SDL_SCANCODE_UNKNOWN,
    /* 216 */   SDL_SCANCODE_UNKNOWN,
    /* 217 */   SDL_SCANCODE_UNKNOWN,
    /* 218 */   SDL_SCANCODE_UNKNOWN,
    /* 219 */   SDL_SCANCODE_LEFTBRACKET,
    /* 220 */   SDL_SCANCODE_BACKSLASH,
    /* 221 */   SDL_SCANCODE_RIGHTBRACKET,
    /* 222 */   SDL_SCANCODE_APOSTROPHE, /*FX, D3E legacy*/
};


/* "borrowed" from SDL_windowsevents.c */
static int
Emscripten_ConvertUTF32toUTF8(Uint32 codepoint, char * text)
{
    if (codepoint <= 0x7F) {
        text[0] = (char) codepoint;
        text[1] = '\0';
    } else if (codepoint <= 0x7FF) {
        text[0] = 0xC0 | (char) ((codepoint >> 6) & 0x1F);
        text[1] = 0x80 | (char) (codepoint & 0x3F);
        text[2] = '\0';
    } else if (codepoint <= 0xFFFF) {
        text[0] = 0xE0 | (char) ((codepoint >> 12) & 0x0F);
        text[1] = 0x80 | (char) ((codepoint >> 6) & 0x3F);
        text[2] = 0x80 | (char) (codepoint & 0x3F);
        text[3] = '\0';
    } else if (codepoint <= 0x10FFFF) {
        text[0] = 0xF0 | (char) ((codepoint >> 18) & 0x0F);
        text[1] = 0x80 | (char) ((codepoint >> 12) & 0x3F);
        text[2] = 0x80 | (char) ((codepoint >> 6) & 0x3F);
        text[3] = 0x80 | (char) (codepoint & 0x3F);
        text[4] = '\0';
    } else {
        return SDL_FALSE;
    }
    return SDL_TRUE;
}

static EM_BOOL
Emscripten_HandlePointerLockChange(int eventType, const EmscriptenPointerlockChangeEvent *changeEvent, void *userData)
{
    SDL_WindowData *window_data = (SDL_WindowData *) userData;
    /* keep track of lock losses, so we can regrab if/when appropriate. */
    window_data->has_pointer_lock = changeEvent->isActive;
    return 0;
}


static EM_BOOL
Emscripten_HandleMouseMove(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
{
    SDL_WindowData *window_data = userData;
    const int isPointerLocked = window_data->has_pointer_lock;
    int mx, my;
    static double residualx = 0, residualy = 0;

    /* rescale (in case canvas is being scaled)*/
    double client_w, client_h, xscale, yscale;
    emscripten_get_element_css_size(window_data->canvas_id, &client_w, &client_h);
    xscale = window_data->window->w / client_w;
    yscale = window_data->window->h / client_h;

    if (isPointerLocked) {
        residualx += mouseEvent->movementX * xscale;
        residualy += mouseEvent->movementY * yscale;
        /* Let slow sub-pixel motion accumulate. Don't lose it. */
        mx = residualx;
        residualx -= mx;
        my = residualy;
        residualy -= my;
    } else {
        mx = mouseEvent->targetX * xscale;
        my = mouseEvent->targetY * yscale;
    }

    SDL_SendMouseMotion(window_data->window, 0, isPointerLocked, mx, my);
    return 0;
}

static EM_BOOL
Emscripten_HandleMouseButton(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
{
    SDL_WindowData *window_data = userData;
    Uint8 sdl_button;
    Uint8 sdl_button_state;
    SDL_EventType sdl_event_type;
    double css_w, css_h;

    switch (mouseEvent->button) {
        case 0:
            sdl_button = SDL_BUTTON_LEFT;
            break;
        case 1:
            sdl_button = SDL_BUTTON_MIDDLE;
            break;
        case 2:
            sdl_button = SDL_BUTTON_RIGHT;
            break;
        default:
            return 0;
    }

    if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN) {
        if (SDL_GetMouse()->relative_mode && !window_data->has_pointer_lock) {
            emscripten_request_pointerlock(window_data->canvas_id, 0);  /* try to regrab lost pointer lock. */
        }
        sdl_button_state = SDL_PRESSED;
        sdl_event_type = SDL_MOUSEBUTTONDOWN;
    } else {
        sdl_button_state = SDL_RELEASED;
        sdl_event_type = SDL_MOUSEBUTTONUP;
    }
    SDL_SendMouseButton(window_data->window, 0, sdl_button_state, sdl_button);

    /* Do not consume the event if the mouse is outside of the canvas. */
    emscripten_get_element_css_size(window_data->canvas_id, &css_w, &css_h);
    if (mouseEvent->targetX < 0 || mouseEvent->targetX >= css_w ||
        mouseEvent->targetY < 0 || mouseEvent->targetY >= css_h) {
        return 0;
    }

    return SDL_GetEventState(sdl_event_type) == SDL_ENABLE;
}

static EM_BOOL
Emscripten_HandleMouseFocus(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
{
    SDL_WindowData *window_data = userData;

    int mx = mouseEvent->targetX, my = mouseEvent->targetY;
    const int isPointerLocked = window_data->has_pointer_lock;

    if (!isPointerLocked) {
        /* rescale (in case canvas is being scaled)*/
        double client_w, client_h;
        emscripten_get_element_css_size(window_data->canvas_id, &client_w, &client_h);

        mx = mx * (window_data->window->w / client_w);
        my = my * (window_data->window->h / client_h);
        SDL_SendMouseMotion(window_data->window, 0, isPointerLocked, mx, my);
    }

    SDL_SetMouseFocus(eventType == EMSCRIPTEN_EVENT_MOUSEENTER ? window_data->window : NULL);
    return SDL_GetEventState(SDL_WINDOWEVENT) == SDL_ENABLE;
}

static EM_BOOL
Emscripten_HandleWheel(int eventType, const EmscriptenWheelEvent *wheelEvent, void *userData)
{
    SDL_WindowData *window_data = userData;
    SDL_SendMouseWheel(window_data->window, 0, (float)wheelEvent->deltaX, (float)-wheelEvent->deltaY, SDL_MOUSEWHEEL_NORMAL);
    return SDL_GetEventState(SDL_MOUSEWHEEL) == SDL_ENABLE;
}

static EM_BOOL
Emscripten_HandleFocus(int eventType, const EmscriptenFocusEvent *wheelEvent, void *userData)
{
    SDL_WindowData *window_data = userData;
    /* If the user switches away while keys are pressed (such as
     * via Alt+Tab), key release events won't be received. */
    if (eventType == EMSCRIPTEN_EVENT_BLUR) {
        SDL_ResetKeyboard();
    }


    SDL_SendWindowEvent(window_data->window, eventType == EMSCRIPTEN_EVENT_FOCUS ? SDL_WINDOWEVENT_FOCUS_GAINED : SDL_WINDOWEVENT_FOCUS_LOST, 0, 0);
    return SDL_GetEventState(SDL_WINDOWEVENT) == SDL_ENABLE;
}

static EM_BOOL
Emscripten_HandleTouch(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData)
{
    SDL_WindowData *window_data = (SDL_WindowData *) userData;
    int i;
    double client_w, client_h;
    int preventDefault = 0;

    SDL_TouchID deviceId = 1;
    if (SDL_AddTouch(deviceId, SDL_TOUCH_DEVICE_DIRECT, "") < 0) {
         return 0;
    }

    emscripten_get_element_css_size(window_data->canvas_id, &client_w, &client_h);

    for (i = 0; i < touchEvent->numTouches; i++) {
        SDL_FingerID id;
        float x, y;

        if (!touchEvent->touches[i].isChanged)
            continue;

        id = touchEvent->touches[i].identifier;
        x = touchEvent->touches[i].targetX / client_w;
        y = touchEvent->touches[i].targetY / client_h;

        if (eventType == EMSCRIPTEN_EVENT_TOUCHSTART) {
            SDL_SendTouch(deviceId, id, window_data->window, SDL_TRUE, x, y, 1.0f);

            /* disable browser scrolling/pinch-to-zoom if app handles touch events */
            if (!preventDefault && SDL_GetEventState(SDL_FINGERDOWN) == SDL_ENABLE) {
                preventDefault = 1;
            }
        } else if (eventType == EMSCRIPTEN_EVENT_TOUCHMOVE) {
            SDL_SendTouchMotion(deviceId, id, window_data->window, x, y, 1.0f);
        } else {
            SDL_SendTouch(deviceId, id, window_data->window, SDL_FALSE, x, y, 1.0f);

            /* block browser's simulated mousedown/mouseup on touchscreen devices */
            preventDefault = 1;
        }
    }

    return preventDefault;
}

static EM_BOOL
Emscripten_HandleKey(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData)
{
    Uint32 scancode;
    SDL_bool prevent_default;
    SDL_bool is_nav_key;

    /* .keyCode is deprecated, but still the most reliable way to get keys */
    if (keyEvent->keyCode < SDL_arraysize(emscripten_scancode_table)) {
        scancode = emscripten_scancode_table[keyEvent->keyCode];

        if (keyEvent->keyCode == 0) {
            /* KaiOS Left Soft Key and Right Soft Key, they act as OK/Next/Menu and Cancel/Back/Clear */
            if (SDL_strncmp(keyEvent->key, "SoftLeft", 9) == 0) {
                scancode = SDL_SCANCODE_AC_FORWARD;
            }
            if (SDL_strncmp(keyEvent->key, "SoftRight", 10) == 0) {
                scancode = SDL_SCANCODE_AC_BACK;
            }
        }

        if (scancode != SDL_SCANCODE_UNKNOWN) {

            if (keyEvent->location == DOM_KEY_LOCATION_RIGHT) {
                switch (scancode) {
                    case SDL_SCANCODE_LSHIFT:
                        scancode = SDL_SCANCODE_RSHIFT;
                        break;
                    case SDL_SCANCODE_LCTRL:
                        scancode = SDL_SCANCODE_RCTRL;
                        break;
                    case SDL_SCANCODE_LALT:
                        scancode = SDL_SCANCODE_RALT;
                        break;
                    case SDL_SCANCODE_LGUI:
                        scancode = SDL_SCANCODE_RGUI;
                        break;
                }
            }
            else if (keyEvent->location == DOM_KEY_LOCATION_NUMPAD) {
                switch (scancode) {
                    case SDL_SCANCODE_0:
                    case SDL_SCANCODE_INSERT:
                        scancode = SDL_SCANCODE_KP_0;
                        break;
                    case SDL_SCANCODE_1:
                    case SDL_SCANCODE_END:
                        scancode = SDL_SCANCODE_KP_1;
                        break;
                    case SDL_SCANCODE_2:
                    case SDL_SCANCODE_DOWN:
                        scancode = SDL_SCANCODE_KP_2;
                        break;
                    case SDL_SCANCODE_3:
                    case SDL_SCANCODE_PAGEDOWN:
                        scancode = SDL_SCANCODE_KP_3;
                        break;
                    case SDL_SCANCODE_4:
                    case SDL_SCANCODE_LEFT:
                        scancode = SDL_SCANCODE_KP_4;
                        break;
                    case SDL_SCANCODE_5:
                        scancode = SDL_SCANCODE_KP_5;
                        break;
                    case SDL_SCANCODE_6:
                    case SDL_SCANCODE_RIGHT:
                        scancode = SDL_SCANCODE_KP_6;
                        break;
                    case SDL_SCANCODE_7:
                    case SDL_SCANCODE_HOME:
                        scancode = SDL_SCANCODE_KP_7;
                        break;
                    case SDL_SCANCODE_8:
                    case SDL_SCANCODE_UP:
                        scancode = SDL_SCANCODE_KP_8;
                        break;
                    case SDL_SCANCODE_9:
                    case SDL_SCANCODE_PAGEUP:
                        scancode = SDL_SCANCODE_KP_9;
                        break;
                    case SDL_SCANCODE_RETURN:
                        scancode = SDL_SCANCODE_KP_ENTER;
                        break;
                    case SDL_SCANCODE_DELETE:
                        scancode = SDL_SCANCODE_KP_PERIOD;
                        break;
                }
            }
            SDL_SendKeyboardKey(eventType == EMSCRIPTEN_EVENT_KEYDOWN ? SDL_PRESSED : SDL_RELEASED, scancode);
        }
    }

    prevent_default = SDL_GetEventState(eventType == EMSCRIPTEN_EVENT_KEYDOWN ? SDL_KEYDOWN : SDL_KEYUP) == SDL_ENABLE;

    /* if TEXTINPUT events are enabled we can't prevent keydown or we won't get keypress
     * we need to ALWAYS prevent backspace and tab otherwise chrome takes action and does bad navigation UX
     */
    is_nav_key = keyEvent->keyCode == 8 /* backspace */ ||
                 keyEvent->keyCode == 9 /* tab */ ||
                 keyEvent->keyCode == 37 /* left */ ||
                 keyEvent->keyCode == 38 /* up */ ||
                 keyEvent->keyCode == 39 /* right */ ||
                 keyEvent->keyCode == 40 /* down */ ||
                 (keyEvent->keyCode >= 112 && keyEvent->keyCode <= 135) /* F keys*/ ||
                 keyEvent->ctrlKey;

    if (eventType == EMSCRIPTEN_EVENT_KEYDOWN && SDL_GetEventState(SDL_TEXTINPUT) == SDL_ENABLE && !is_nav_key)
        prevent_default = SDL_FALSE;

    return prevent_default;
}

static EM_BOOL
Emscripten_HandleKeyPress(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData)
{
    char text[5];
    if (Emscripten_ConvertUTF32toUTF8(keyEvent->charCode, text)) {
        SDL_SendKeyboardText(text);
    }
    return SDL_GetEventState(SDL_TEXTINPUT) == SDL_ENABLE;
}

static EM_BOOL
Emscripten_HandleFullscreenChange(int eventType, const EmscriptenFullscreenChangeEvent *fullscreenChangeEvent, void *userData)
{
    SDL_WindowData *window_data = userData;
    SDL_VideoDisplay *display;

    if(fullscreenChangeEvent->isFullscreen)
    {
        window_data->window->flags |= window_data->requested_fullscreen_mode;

        window_data->requested_fullscreen_mode = 0;

        if(!window_data->requested_fullscreen_mode)
            window_data->window->flags |= SDL_WINDOW_FULLSCREEN; /*we didn't request fullscreen*/
    }
    else
    {
        window_data->window->flags &= ~FULLSCREEN_MASK;

        /* reset fullscreen window if the browser left fullscreen */
        display = SDL_GetDisplayForWindow(window_data->window);

        if (display->fullscreen_window == window_data->window) {
            display->fullscreen_window = NULL;
        }
    }

    return 0;
}

static EM_BOOL
Emscripten_HandleResize(int eventType, const EmscriptenUiEvent *uiEvent, void *userData)
{
    SDL_WindowData *window_data = userData;
    SDL_bool force = SDL_FALSE;

    /* update pixel ratio */
    if (window_data->window->flags & SDL_WINDOW_ALLOW_HIGHDPI) {
        if (window_data->pixel_ratio != emscripten_get_device_pixel_ratio()) {
            window_data->pixel_ratio = emscripten_get_device_pixel_ratio();
            force = SDL_TRUE;
        }
    }

    if(!(window_data->window->flags & FULLSCREEN_MASK))
    {
        /* this will only work if the canvas size is set through css */
        if(window_data->window->flags & SDL_WINDOW_RESIZABLE)
        {
            double w = window_data->window->w;
            double h = window_data->window->h;

            if(window_data->external_size) {
                emscripten_get_element_css_size(window_data->canvas_id, &w, &h);
            }

            emscripten_set_canvas_element_size(window_data->canvas_id, w * window_data->pixel_ratio, h * window_data->pixel_ratio);

            /* set_canvas_size unsets this */
            if (!window_data->external_size && window_data->pixel_ratio != 1.0f) {
                emscripten_set_element_css_size(window_data->canvas_id, w, h);
            }

            if (force) {
               /* force the event to trigger, so pixel ratio changes can be handled */
               window_data->window->w = 0;
               window_data->window->h = 0;
            }

            SDL_SendWindowEvent(window_data->window, SDL_WINDOWEVENT_RESIZED, w, h);
        }
    }

    return 0;
}

EM_BOOL
Emscripten_HandleCanvasResize(int eventType, const void *reserved, void *userData)
{
    /*this is used during fullscreen changes*/
    SDL_WindowData *window_data = userData;

    if(window_data->fullscreen_resize)
    {
        double css_w, css_h;
        emscripten_get_element_css_size(window_data->canvas_id, &css_w, &css_h);
        SDL_SendWindowEvent(window_data->window, SDL_WINDOWEVENT_RESIZED, css_w, css_h);
    }

    return 0;
}

static EM_BOOL
Emscripten_HandleVisibilityChange(int eventType, const EmscriptenVisibilityChangeEvent *visEvent, void *userData)
{
    SDL_WindowData *window_data = userData;
    SDL_SendWindowEvent(window_data->window, visEvent->hidden ? SDL_WINDOWEVENT_HIDDEN : SDL_WINDOWEVENT_SHOWN, 0, 0);
    return 0;
}

static const char*
Emscripten_HandleBeforeUnload(int eventType, const void *reserved, void *userData)
{
    /* This event will need to be handled synchronously, e.g. using
       SDL_AddEventWatch, as the page is being closed *now*. */
    /* No need to send a SDL_QUIT, the app won't get control again. */
    SDL_SendAppEvent(SDL_APP_TERMINATING);
    return ""; /* don't trigger confirmation dialog */
}

void
Emscripten_RegisterEventHandlers(SDL_WindowData *data)
{
    const char *keyElement;

    /* There is only one window and that window is the canvas */
    emscripten_set_mousemove_callback(data->canvas_id, data, 0, Emscripten_HandleMouseMove);

    emscripten_set_mousedown_callback(data->canvas_id, data, 0, Emscripten_HandleMouseButton);
    emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, data, 0, Emscripten_HandleMouseButton);

    emscripten_set_mouseenter_callback(data->canvas_id, data, 0, Emscripten_HandleMouseFocus);
    emscripten_set_mouseleave_callback(data->canvas_id, data, 0, Emscripten_HandleMouseFocus);

    emscripten_set_wheel_callback(data->canvas_id, data, 0, Emscripten_HandleWheel);

    emscripten_set_focus_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, data, 0, Emscripten_HandleFocus);
    emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, data, 0, Emscripten_HandleFocus);

    emscripten_set_touchstart_callback(data->canvas_id, data, 0, Emscripten_HandleTouch);
    emscripten_set_touchend_callback(data->canvas_id, data, 0, Emscripten_HandleTouch);
    emscripten_set_touchmove_callback(data->canvas_id, data, 0, Emscripten_HandleTouch);
    emscripten_set_touchcancel_callback(data->canvas_id, data, 0, Emscripten_HandleTouch);

    emscripten_set_pointerlockchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, data, 0, Emscripten_HandlePointerLockChange);

    /* Keyboard events are awkward */
    keyElement = SDL_GetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT);
    if (!keyElement) keyElement = EMSCRIPTEN_EVENT_TARGET_WINDOW;

    emscripten_set_keydown_callback(keyElement, data, 0, Emscripten_HandleKey);
    emscripten_set_keyup_callback(keyElement, data, 0, Emscripten_HandleKey);
    emscripten_set_keypress_callback(keyElement, data, 0, Emscripten_HandleKeyPress);

    emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, data, 0, Emscripten_HandleFullscreenChange);

    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, data, 0, Emscripten_HandleResize);

    emscripten_set_visibilitychange_callback(data, 0, Emscripten_HandleVisibilityChange);

    emscripten_set_beforeunload_callback(data, Emscripten_HandleBeforeUnload);
}

void
Emscripten_UnregisterEventHandlers(SDL_WindowData *data)
{
    const char *target;

    /* only works due to having one window */
    emscripten_set_mousemove_callback(data->canvas_id, NULL, 0, NULL);

    emscripten_set_mousedown_callback(data->canvas_id, NULL, 0, NULL);
    emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, NULL, 0, NULL);

    emscripten_set_mouseenter_callback(data->canvas_id, NULL, 0, NULL);
    emscripten_set_mouseleave_callback(data->canvas_id, NULL, 0, NULL);

    emscripten_set_wheel_callback(data->canvas_id, NULL, 0, NULL);

    emscripten_set_focus_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, NULL);
    emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, NULL);

    emscripten_set_touchstart_callback(data->canvas_id, NULL, 0, NULL);
    emscripten_set_touchend_callback(data->canvas_id, NULL, 0, NULL);
    emscripten_set_touchmove_callback(data->canvas_id, NULL, 0, NULL);
    emscripten_set_touchcancel_callback(data->canvas_id, NULL, 0, NULL);

    emscripten_set_pointerlockchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, NULL, 0, NULL);

    target = SDL_GetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT);
    if (!target) {
        target = EMSCRIPTEN_EVENT_TARGET_WINDOW;
    }

    emscripten_set_keydown_callback(target, NULL, 0, NULL);
    emscripten_set_keyup_callback(target, NULL, 0, NULL);
    emscripten_set_keypress_callback(target, NULL, 0, NULL);

    emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, NULL, 0, NULL);

    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, NULL);

    emscripten_set_visibilitychange_callback(NULL, 0, NULL);

    emscripten_set_beforeunload_callback(NULL, NULL);
}

#endif /* SDL_VIDEO_DRIVER_EMSCRIPTEN */

/* vi: set ts=4 sw=4 expandtab: */
