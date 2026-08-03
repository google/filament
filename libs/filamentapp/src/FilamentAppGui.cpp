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

#include "FilamentAppGui.h"
#include <filamentapp/DisplayManager.h>
#if defined(FILAMENTAPP_HAS_IMGUI)
#include <filagui/ImGuiHelper.h>
#include <imgui.h>

namespace {
ImGuiKey AppKeyToImGuiKey(filament::app::AppKey key) {
    using namespace filament::app;
    switch (key) {
        case AppKey::TAB:
            return ImGuiKey_Tab;
        case AppKey::LEFT:
            return ImGuiKey_LeftArrow;
        case AppKey::RIGHT:
            return ImGuiKey_RightArrow;
        case AppKey::UP:
            return ImGuiKey_UpArrow;
        case AppKey::DOWN:
            return ImGuiKey_DownArrow;
        case AppKey::PAGE_UP:
            return ImGuiKey_PageUp;
        case AppKey::PAGE_DOWN:
            return ImGuiKey_PageDown;
        case AppKey::HOME:
            return ImGuiKey_Home;
        case AppKey::END:
            return ImGuiKey_End;
        case AppKey::INSERT:
            return ImGuiKey_Insert;
        case AppKey::DEL:
            return ImGuiKey_Delete;
        case AppKey::BACKSPACE:
            return ImGuiKey_Backspace;
        case AppKey::SPACE:
            return ImGuiKey_Space;
        case AppKey::ENTER:
            return ImGuiKey_Enter;
        case AppKey::ESCAPE:
            return ImGuiKey_Escape;
        case AppKey::COMMA:
            return ImGuiKey_Comma;
        case AppKey::PERIOD:
            return ImGuiKey_Period;
        case AppKey::SEMICOLON:
            return ImGuiKey_Semicolon;
        case AppKey::CAPS_LOCK:
            return ImGuiKey_CapsLock;
        case AppKey::SCROLL_LOCK:
            return ImGuiKey_ScrollLock;
        case AppKey::NUM_LOCK:
            return ImGuiKey_NumLock;
        case AppKey::PRINT_SCREEN:
            return ImGuiKey_PrintScreen;
        case AppKey::PAUSE:
            return ImGuiKey_Pause;
        case AppKey::KEYPAD_0:
            return ImGuiKey_Keypad0;
        case AppKey::KEYPAD_1:
            return ImGuiKey_Keypad1;
        case AppKey::KEYPAD_2:
            return ImGuiKey_Keypad2;
        case AppKey::KEYPAD_3:
            return ImGuiKey_Keypad3;
        case AppKey::KEYPAD_4:
            return ImGuiKey_Keypad4;
        case AppKey::KEYPAD_5:
            return ImGuiKey_Keypad5;
        case AppKey::KEYPAD_6:
            return ImGuiKey_Keypad6;
        case AppKey::KEYPAD_7:
            return ImGuiKey_Keypad7;
        case AppKey::KEYPAD_8:
            return ImGuiKey_Keypad8;
        case AppKey::KEYPAD_9:
            return ImGuiKey_Keypad9;
        case AppKey::KEYPAD_DECIMAL:
            return ImGuiKey_KeypadDecimal;
        case AppKey::KEYPAD_DIVIDE:
            return ImGuiKey_KeypadDivide;
        case AppKey::KEYPAD_MULTIPLY:
            return ImGuiKey_KeypadMultiply;
        case AppKey::KEYPAD_SUBTRACT:
            return ImGuiKey_KeypadSubtract;
        case AppKey::KEYPAD_ADD:
            return ImGuiKey_KeypadAdd;
        case AppKey::KEYPAD_ENTER:
            return ImGuiKey_KeypadEnter;
        case AppKey::KEYPAD_EQUAL:
            return ImGuiKey_KeypadEqual;
        case AppKey::LEFT_CTRL:
            return ImGuiKey_LeftCtrl;
        case AppKey::LEFT_SHIFT:
            return ImGuiKey_LeftShift;
        case AppKey::LEFT_ALT:
            return ImGuiKey_LeftAlt;
        case AppKey::LEFT_SUPER:
            return ImGuiKey_LeftSuper;
        case AppKey::RIGHT_CTRL:
            return ImGuiKey_RightCtrl;
        case AppKey::RIGHT_SHIFT:
            return ImGuiKey_RightShift;
        case AppKey::RIGHT_ALT:
            return ImGuiKey_RightAlt;
        case AppKey::RIGHT_SUPER:
            return ImGuiKey_RightSuper;
        case AppKey::MENU:
            return ImGuiKey_Menu;
        case AppKey::_0:
            return ImGuiKey_0;
        case AppKey::_1:
            return ImGuiKey_1;
        case AppKey::_2:
            return ImGuiKey_2;
        case AppKey::_3:
            return ImGuiKey_3;
        case AppKey::_4:
            return ImGuiKey_4;
        case AppKey::_5:
            return ImGuiKey_5;
        case AppKey::_6:
            return ImGuiKey_6;
        case AppKey::_7:
            return ImGuiKey_7;
        case AppKey::_8:
            return ImGuiKey_8;
        case AppKey::_9:
            return ImGuiKey_9;
        case AppKey::A:
            return ImGuiKey_A;
        case AppKey::B:
            return ImGuiKey_B;
        case AppKey::C:
            return ImGuiKey_C;
        case AppKey::D:
            return ImGuiKey_D;
        case AppKey::E:
            return ImGuiKey_E;
        case AppKey::F:
            return ImGuiKey_F;
        case AppKey::G:
            return ImGuiKey_G;
        case AppKey::H:
            return ImGuiKey_H;
        case AppKey::I:
            return ImGuiKey_I;
        case AppKey::J:
            return ImGuiKey_J;
        case AppKey::K:
            return ImGuiKey_K;
        case AppKey::L:
            return ImGuiKey_L;
        case AppKey::M:
            return ImGuiKey_M;
        case AppKey::N:
            return ImGuiKey_N;
        case AppKey::O:
            return ImGuiKey_O;
        case AppKey::P:
            return ImGuiKey_P;
        case AppKey::Q:
            return ImGuiKey_Q;
        case AppKey::R:
            return ImGuiKey_R;
        case AppKey::S:
            return ImGuiKey_S;
        case AppKey::T:
            return ImGuiKey_T;
        case AppKey::U:
            return ImGuiKey_U;
        case AppKey::V:
            return ImGuiKey_V;
        case AppKey::W:
            return ImGuiKey_W;
        case AppKey::X:
            return ImGuiKey_X;
        case AppKey::Y:
            return ImGuiKey_Y;
        case AppKey::Z:
            return ImGuiKey_Z;
        case AppKey::F1:
            return ImGuiKey_F1;
        case AppKey::F2:
            return ImGuiKey_F2;
        case AppKey::F3:
            return ImGuiKey_F3;
        case AppKey::F4:
            return ImGuiKey_F4;
        case AppKey::F5:
            return ImGuiKey_F5;
        case AppKey::F6:
            return ImGuiKey_F6;
        case AppKey::F7:
            return ImGuiKey_F7;
        case AppKey::F8:
            return ImGuiKey_F8;
        case AppKey::F9:
            return ImGuiKey_F9;
        case AppKey::F10:
            return ImGuiKey_F10;
        case AppKey::F11:
            return ImGuiKey_F11;
        case AppKey::F12:
            return ImGuiKey_F12;
        case AppKey::F13:
            return ImGuiKey_F13;
        case AppKey::F14:
            return ImGuiKey_F14;
        case AppKey::F15:
            return ImGuiKey_F15;
        case AppKey::F16:
            return ImGuiKey_F16;
        case AppKey::F17:
            return ImGuiKey_F17;
        case AppKey::F18:
            return ImGuiKey_F18;
        case AppKey::F19:
            return ImGuiKey_F19;
        case AppKey::F20:
            return ImGuiKey_F20;
        case AppKey::F21:
            return ImGuiKey_F21;
        case AppKey::F22:
            return ImGuiKey_F22;
        case AppKey::F23:
            return ImGuiKey_F23;
        case AppKey::F24:
            return ImGuiKey_F24;
        case AppKey::APP_BACK:
            return ImGuiKey_AppBack;
        case AppKey::APP_FORWARD:
            return ImGuiKey_AppForward;
        case AppKey::GRAVE:
            return ImGuiKey_GraveAccent;
        case AppKey::MINUS:
            return ImGuiKey_Minus;
        case AppKey::EQUAL:
            return ImGuiKey_Equal;
        case AppKey::LEFT_BRACKET:
            return ImGuiKey_LeftBracket;
        case AppKey::RIGHT_BRACKET:
            return ImGuiKey_RightBracket;
        case AppKey::BACKSLASH:
            return ImGuiKey_Backslash;
        case AppKey::APOSTROPHE:
            return ImGuiKey_Apostrophe;
        case AppKey::SLASH:
            return ImGuiKey_Slash;
        default:
            break;
    }
    return ImGuiKey_None;
}
} // namespace
#endif

using namespace filament::app;

struct FilamentAppGui::Impl {
#if defined(FILAMENTAPP_HAS_IMGUI)
    std::unique_ptr<filagui::ImGuiHelper> helper;
#endif
};

FilamentAppGui::FilamentAppGui(filament::Engine* engine, filament::View* view,
        const utils::Path& fontPath)
        : pImpl(new Impl()) {
#if defined(FILAMENTAPP_HAS_IMGUI)
    pImpl->helper = std::make_unique<filagui::ImGuiHelper>(engine, view, fontPath);
#endif
}

FilamentAppGui::~FilamentAppGui() { delete pImpl; }

void FilamentAppGui::processAppEvents(std::vector<filament::app::AppEvent> const& events) {
#if defined(FILAMENTAPP_HAS_IMGUI)
    if (!pImpl->helper) return;

    ImGuiIO& io = ImGui::GetIO();
    for (const auto& event: events) {
        switch (event.type) {
            case AppEvent::Type::MOUSE_WHEEL: {
                io.MouseWheel += event.mouseWheel.delta;
                break;
            }
            case AppEvent::Type::MOUSE_BUTTON_DOWN: {
                // Handled in render() based on display manager state for now,
                // but we could also process them here.
                break;
            }
            case AppEvent::Type::KEYDOWN:
            case AppEvent::Type::KEYUP: {
                io.AddKeyEvent(ImGuiMod_Ctrl, (event.key.modifiers & AppKeyModifier::CTRL) != 0);
                io.AddKeyEvent(ImGuiMod_Shift, (event.key.modifiers & AppKeyModifier::SHIFT) != 0);
                io.AddKeyEvent(ImGuiMod_Alt, (event.key.modifiers & AppKeyModifier::ALT) != 0);
                io.AddKeyEvent(ImGuiMod_Super, (event.key.modifiers & AppKeyModifier::SUPER) != 0);
                io.AddKeyEvent(AppKeyToImGuiKey(event.key.code),
                        event.type == AppEvent::Type::KEYDOWN);
                break;
            }
            case AppEvent::Type::TEXTINPUT: {
                io.AddInputCharactersUTF8(event.text.text);
                break;
            }
            default:
                break;
        }
    }
#endif
}

bool FilamentAppGui::wantCaptureMouse() const {
#if defined(FILAMENTAPP_HAS_IMGUI)
    if (pImpl->helper) {
        ImGuiIO& io = ImGui::GetIO();
        return io.WantCaptureMouse;
    }
#endif
    return false;
}

void FilamentAppGui::render(float timeStep, filament::app::DisplayManager* displayManager,
        void* window, FilamentApp2::ImGuiCallback imguiCallback, bool mousePressed[3]) {
#if defined(FILAMENTAPP_HAS_IMGUI)
    if (pImpl->helper) {
        uint32_t windowWidth, windowHeight;
        uint32_t displayWidth, displayHeight;
        displayManager->getWindowSize(window, &windowWidth, &windowHeight);
        displayManager->getDrawableSize(window, &displayWidth, &displayHeight);
        pImpl->helper->setDisplaySize(windowWidth, windowHeight,
                windowWidth > 0 ? ((float) displayWidth / windowWidth) : 0,
                displayHeight > 0 ? ((float) displayHeight / windowHeight) : 0);

        ImGuiIO& io = ImGui::GetIO();
        int mx, my;
        uint32_t buttons = displayManager->getMouseState(&mx, &my);
        io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
        io.MouseDown[0] = mousePressed[0] || (buttons & (1 << 0)) != 0;
        io.MouseDown[1] = mousePressed[1] || (buttons & (1 << 2)) != 0;
        io.MouseDown[2] = mousePressed[2] || (buttons & (1 << 1)) != 0;
        mousePressed[0] = mousePressed[1] = mousePressed[2] = false;

        if (displayManager->isWindowFocused(window)) {
            io.MousePos = ImVec2((float) mx, (float) my);
        }

        pImpl->helper->render(timeStep, imguiCallback);
    }
#endif
}

filament::View* FilamentAppGui::getView() const noexcept {
#if defined(FILAMENTAPP_HAS_IMGUI)
    if (pImpl->helper) {
        return pImpl->helper->getView();
    }
#endif
    return nullptr;
}
