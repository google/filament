/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_FILAMENT_FILAMENTAPP_KEYINPUTCONVERSION_H
#define TNT_FILAMENT_FILAMENTAPP_KEYINPUTCONVERSION_H

#include <imgui.h>
#include <SDL.h>

namespace filamentapp_utils {

// Copied from
// https://github.com/ocornut/imgui/blob/940627d008b8f0584b7f50d24574537cf24a32e1/backends/imgui_impl_sdl2.cpp#L206
ImGuiKey ImGui_ImplSDL2_KeyEventToImGuiKey(SDL_Keycode keycode, SDL_Scancode scancode) {
    switch (keycode) {
        case SDLK_TAB:
            return ImGuiKey_Tab;
        case SDLK_LEFT:
            return ImGuiKey_LeftArrow;
        case SDLK_RIGHT:
            return ImGuiKey_RightArrow;
        case SDLK_UP:
            return ImGuiKey_UpArrow;
        case SDLK_DOWN:
            return ImGuiKey_DownArrow;
        case SDLK_PAGEUP:
            return ImGuiKey_PageUp;
        case SDLK_PAGEDOWN:
            return ImGuiKey_PageDown;
        case SDLK_HOME:
            return ImGuiKey_Home;
        case SDLK_END:
            return ImGuiKey_End;
        case SDLK_INSERT:
            return ImGuiKey_Insert;
        case SDLK_DELETE:
            return ImGuiKey_Delete;
        case SDLK_BACKSPACE:
            return ImGuiKey_Backspace;
        case SDLK_SPACE:
            return ImGuiKey_Space;
        case SDLK_RETURN:
            return ImGuiKey_Enter;
        case SDLK_ESCAPE:
            return ImGuiKey_Escape;
        // case SDLK_QUOTE: return ImGuiKey_Apostrophe;
        case SDLK_COMMA:
            return ImGuiKey_Comma;
        // case SDLK_MINUS: return ImGuiKey_Minus;
        case SDLK_PERIOD:
            return ImGuiKey_Period;
        // case SDLK_SLASH: return ImGuiKey_Slash;
        case SDLK_SEMICOLON:
            return ImGuiKey_Semicolon;
        // case SDLK_EQUALS: return ImGuiKey_Equal;
        // case SDLK_LEFTBRACKET: return ImGuiKey_LeftBracket;
        // case SDLK_BACKSLASH: return ImGuiKey_Backslash;
        // case SDLK_RIGHTBRACKET: return ImGuiKey_RightBracket;
        // case SDLK_BACKQUOTE: return ImGuiKey_GraveAccent;
        case SDLK_CAPSLOCK:
            return ImGuiKey_CapsLock;
        case SDLK_SCROLLLOCK:
            return ImGuiKey_ScrollLock;
        case SDLK_NUMLOCKCLEAR:
            return ImGuiKey_NumLock;
        case SDLK_PRINTSCREEN:
            return ImGuiKey_PrintScreen;
        case SDLK_PAUSE:
            return ImGuiKey_Pause;
        case SDLK_KP_0:
            return ImGuiKey_Keypad0;
        case SDLK_KP_1:
            return ImGuiKey_Keypad1;
        case SDLK_KP_2:
            return ImGuiKey_Keypad2;
        case SDLK_KP_3:
            return ImGuiKey_Keypad3;
        case SDLK_KP_4:
            return ImGuiKey_Keypad4;
        case SDLK_KP_5:
            return ImGuiKey_Keypad5;
        case SDLK_KP_6:
            return ImGuiKey_Keypad6;
        case SDLK_KP_7:
            return ImGuiKey_Keypad7;
        case SDLK_KP_8:
            return ImGuiKey_Keypad8;
        case SDLK_KP_9:
            return ImGuiKey_Keypad9;
        case SDLK_KP_PERIOD:
            return ImGuiKey_KeypadDecimal;
        case SDLK_KP_DIVIDE:
            return ImGuiKey_KeypadDivide;
        case SDLK_KP_MULTIPLY:
            return ImGuiKey_KeypadMultiply;
        case SDLK_KP_MINUS:
            return ImGuiKey_KeypadSubtract;
        case SDLK_KP_PLUS:
            return ImGuiKey_KeypadAdd;
        case SDLK_KP_ENTER:
            return ImGuiKey_KeypadEnter;
        case SDLK_KP_EQUALS:
            return ImGuiKey_KeypadEqual;
        case SDLK_LCTRL:
            return ImGuiKey_LeftCtrl;
        case SDLK_LSHIFT:
            return ImGuiKey_LeftShift;
        case SDLK_LALT:
            return ImGuiKey_LeftAlt;
        case SDLK_LGUI:
            return ImGuiKey_LeftSuper;
        case SDLK_RCTRL:
            return ImGuiKey_RightCtrl;
        case SDLK_RSHIFT:
            return ImGuiKey_RightShift;
        case SDLK_RALT:
            return ImGuiKey_RightAlt;
        case SDLK_RGUI:
            return ImGuiKey_RightSuper;
        case SDLK_APPLICATION:
            return ImGuiKey_Menu;
        case SDLK_0:
            return ImGuiKey_0;
        case SDLK_1:
            return ImGuiKey_1;
        case SDLK_2:
            return ImGuiKey_2;
        case SDLK_3:
            return ImGuiKey_3;
        case SDLK_4:
            return ImGuiKey_4;
        case SDLK_5:
            return ImGuiKey_5;
        case SDLK_6:
            return ImGuiKey_6;
        case SDLK_7:
            return ImGuiKey_7;
        case SDLK_8:
            return ImGuiKey_8;
        case SDLK_9:
            return ImGuiKey_9;
        case SDLK_a:
            return ImGuiKey_A;
        case SDLK_b:
            return ImGuiKey_B;
        case SDLK_c:
            return ImGuiKey_C;
        case SDLK_d:
            return ImGuiKey_D;
        case SDLK_e:
            return ImGuiKey_E;
        case SDLK_f:
            return ImGuiKey_F;
        case SDLK_g:
            return ImGuiKey_G;
        case SDLK_h:
            return ImGuiKey_H;
        case SDLK_i:
            return ImGuiKey_I;
        case SDLK_j:
            return ImGuiKey_J;
        case SDLK_k:
            return ImGuiKey_K;
        case SDLK_l:
            return ImGuiKey_L;
        case SDLK_m:
            return ImGuiKey_M;
        case SDLK_n:
            return ImGuiKey_N;
        case SDLK_o:
            return ImGuiKey_O;
        case SDLK_p:
            return ImGuiKey_P;
        case SDLK_q:
            return ImGuiKey_Q;
        case SDLK_r:
            return ImGuiKey_R;
        case SDLK_s:
            return ImGuiKey_S;
        case SDLK_t:
            return ImGuiKey_T;
        case SDLK_u:
            return ImGuiKey_U;
        case SDLK_v:
            return ImGuiKey_V;
        case SDLK_w:
            return ImGuiKey_W;
        case SDLK_x:
            return ImGuiKey_X;
        case SDLK_y:
            return ImGuiKey_Y;
        case SDLK_z:
            return ImGuiKey_Z;
        case SDLK_F1:
            return ImGuiKey_F1;
        case SDLK_F2:
            return ImGuiKey_F2;
        case SDLK_F3:
            return ImGuiKey_F3;
        case SDLK_F4:
            return ImGuiKey_F4;
        case SDLK_F5:
            return ImGuiKey_F5;
        case SDLK_F6:
            return ImGuiKey_F6;
        case SDLK_F7:
            return ImGuiKey_F7;
        case SDLK_F8:
            return ImGuiKey_F8;
        case SDLK_F9:
            return ImGuiKey_F9;
        case SDLK_F10:
            return ImGuiKey_F10;
        case SDLK_F11:
            return ImGuiKey_F11;
        case SDLK_F12:
            return ImGuiKey_F12;
        case SDLK_F13:
            return ImGuiKey_F13;
        case SDLK_F14:
            return ImGuiKey_F14;
        case SDLK_F15:
            return ImGuiKey_F15;
        case SDLK_F16:
            return ImGuiKey_F16;
        case SDLK_F17:
            return ImGuiKey_F17;
        case SDLK_F18:
            return ImGuiKey_F18;
        case SDLK_F19:
            return ImGuiKey_F19;
        case SDLK_F20:
            return ImGuiKey_F20;
        case SDLK_F21:
            return ImGuiKey_F21;
        case SDLK_F22:
            return ImGuiKey_F22;
        case SDLK_F23:
            return ImGuiKey_F23;
        case SDLK_F24:
            return ImGuiKey_F24;
        case SDLK_AC_BACK:
            return ImGuiKey_AppBack;
        case SDLK_AC_FORWARD:
            return ImGuiKey_AppForward;
        default:
            break;
    }

    // Fallback to scancode
    switch (scancode) {
        case SDL_SCANCODE_GRAVE:
            return ImGuiKey_GraveAccent;
        case SDL_SCANCODE_MINUS:
            return ImGuiKey_Minus;
        case SDL_SCANCODE_EQUALS:
            return ImGuiKey_Equal;
        case SDL_SCANCODE_LEFTBRACKET:
            return ImGuiKey_LeftBracket;
        case SDL_SCANCODE_RIGHTBRACKET:
            return ImGuiKey_RightBracket;
        case SDL_SCANCODE_NONUSBACKSLASH:
            return ImGuiKey_Oem102;
        case SDL_SCANCODE_BACKSLASH:
            return ImGuiKey_Backslash;
        case SDL_SCANCODE_SEMICOLON:
            return ImGuiKey_Semicolon;
        case SDL_SCANCODE_APOSTROPHE:
            return ImGuiKey_Apostrophe;
        case SDL_SCANCODE_COMMA:
            return ImGuiKey_Comma;
        case SDL_SCANCODE_PERIOD:
            return ImGuiKey_Period;
        case SDL_SCANCODE_SLASH:
            return ImGuiKey_Slash;
        default:
            break;
    }
    return ImGuiKey_None;
}

} // namespace filamentapp_utils

#endif // TNT_FILAMENT_FILAMENTAPP_KEYINPUTCONVERSION_H
