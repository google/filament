/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILAMENT_FILAMENTAPP_APPEVENT_H
#define TNT_FILAMENT_FILAMENTAPP_APPEVENT_H

#include <filamentapp/FilamentApp2.h>

#include <cstdint>
#include <string>

namespace filament::app {

/**
 * Key codes for FilamentApp.
 */
enum class AppKey : uint32_t {
    UNKNOWN = 0,
    ESCAPE,
    ENTER,
    TAB,
    BACKSPACE,
    INSERT,
    DEL,
    RIGHT,
    LEFT,
    DOWN,
    UP,
    PAGE_UP,
    PAGE_DOWN,
    HOME,
    END,
    CAPS_LOCK,
    SCROLL_LOCK,
    NUM_LOCK,
    PRINT_SCREEN,
    PAUSE,
    KEYPAD_0,
    KEYPAD_1,
    KEYPAD_2,
    KEYPAD_3,
    KEYPAD_4,
    KEYPAD_5,
    KEYPAD_6,
    KEYPAD_7,
    KEYPAD_8,
    KEYPAD_9,
    KEYPAD_DECIMAL,
    KEYPAD_DIVIDE,
    KEYPAD_MULTIPLY,
    KEYPAD_SUBTRACT,
    KEYPAD_ADD,
    KEYPAD_ENTER,
    KEYPAD_EQUAL,
    LEFT_CTRL,
    LEFT_SHIFT,
    LEFT_ALT,
    LEFT_SUPER,
    RIGHT_CTRL,
    RIGHT_SHIFT,
    RIGHT_ALT,
    RIGHT_SUPER,
    MENU,
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    _0,
    _1,
    _2,
    _3,
    _4,
    _5,
    _6,
    _7,
    _8,
    _9,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,
    F16,
    F17,
    F18,
    F19,
    F20,
    F21,
    F22,
    F23,
    F24,
    APP_BACK,
    APP_FORWARD,
    SPACE,
    COMMA,
    PERIOD,
    SEMICOLON,
    GRAVE,
    MINUS,
    EQUAL,
    LEFT_BRACKET,
    RIGHT_BRACKET,
    BACKSLASH,
    APOSTROPHE,
    SLASH
};

/**
 * Event structure for FilamentApp.
 */
struct AppEvent {
    enum class Type {
        QUIT,
        KEYDOWN,
        KEYUP,
        MOUSE_WHEEL,
        MOUSE_BUTTON_DOWN,
        MOUSE_BUTTON_UP,
        MOUSE_MOVE,
        DROP_FILE,
        RESIZED,
        TEXTINPUT
    } type;

    FilamentApp2::Window::Handle windowId = nullptr;

    union {
        struct {
            AppKey code;
            uint16_t modifiers;
        } key;
        struct {
            char text[32];
        } text;
        struct {
            int button;
            int32_t x;
            int32_t y;
        } mouseButton;
        struct {
            int32_t x;
            int32_t y;
        } mouseMove;
        struct {
            int32_t delta;
        } mouseWheel;
        struct {
            char const* path;
        } dropFile;
        struct {
            uint32_t w;
            uint32_t h;
        } resize;
    };
};

/**
 * Key modifiers.
 */
enum AppKeyModifier : uint16_t {
    NONE = 0,
    LSHIFT = 1 << 0,
    RSHIFT = 1 << 1,
    LCTRL = 1 << 2,
    RCTRL = 1 << 3,
    LALT = 1 << 4,
    RALT = 1 << 5,
    LSUPER = 1 << 6,
    RSUPER = 1 << 7,
    SHIFT = LSHIFT | RSHIFT,
    CTRL = LCTRL | RCTRL,
    ALT = LALT | RALT,
    SUPER = LSUPER | RSUPER,
};

} // namespace filament::app

#endif // TNT_FILAMENT_FILAMENTAPP_APPEVENT_H
