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

#include "SDLDisplayManager.h"

#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp2.h>
#include <filamentapp/NativeWindowHelper.h>

#include <utils/Panic.h>

#include <SDL_syswm.h>

#include <thread>

namespace filament::app {

SDLDisplayManager::SDLDisplayManager() {}

SDLDisplayManager::~SDLDisplayManager() {}

bool SDLDisplayManager::init(const Config& config) {
    mConfig = config;
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        return false;
    }
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    return true;
}

void SDLDisplayManager::terminate() { SDL_Quit(); }


FilamentApp2::Window::Handle SDLDisplayManager::createWindow(const char* title, uint32_t w,
        uint32_t h, bool resizable, bool headless) {
    uint32_t windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
    if (resizable) {
        windowFlags |= SDL_WINDOW_RESIZABLE;
    }
    if (headless) {
        windowFlags |= SDL_WINDOW_HIDDEN;
    }

    SDL_Window* window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            (int) w, (int) h, windowFlags);

    // This must be called before prepareNativeWindow()
    void* nativeWindow = ::getNativeWindowFromSDL(window);

    if (window && !headless) {
#if defined(__APPLE__)
        ::prepareNativeWindow(window);
#endif
    }

#if defined(__APPLE__)
    if (mConfig.backend == filament::Engine::Backend::METAL ||
            mConfig.backend == filament::Engine::Backend::VULKAN ||
            mConfig.backend == filament::Engine::Backend::WEBGPU) {
        nativeWindow = ::setUpMetalLayer(nativeWindow);
    }
#endif

    mNativeWindowMap[window] = nativeWindow;
    return (FilamentApp2::Window::Handle) window;
}

void SDLDisplayManager::destroyWindow(FilamentApp2::Window::Handle window) {
    SDL_DestroyWindow((SDL_Window*) window);
}

void* SDLDisplayManager::getNativeWindow(FilamentApp2::Window::Handle window) const {
    assert_invariant(mNativeWindowMap.count(window) > 0);
    return mNativeWindowMap[window];
}

void SDLDisplayManager::setWindowTitle(FilamentApp2::Window::Handle window, const char* title) {
    SDL_SetWindowTitle((SDL_Window*) window, title);
}

void SDLDisplayManager::getWindowSize(FilamentApp2::Window::Handle window, uint32_t* w,
        uint32_t* h) const {
    int iw, ih;
    SDL_GetWindowSize((SDL_Window*) window, &iw, &ih);
    *w = (uint32_t) iw;
    *h = (uint32_t) ih;
}

void SDLDisplayManager::getDrawableSize(FilamentApp2::Window::Handle window, uint32_t* w,
        uint32_t* h) const {
    int iw, ih;
    SDL_GL_GetDrawableSize((SDL_Window*) window, &iw, &ih);
    *w = (uint32_t) iw;
    *h = (uint32_t) ih;
}

uint32_t SDLDisplayManager::getMouseState(int* x, int* y) const { return SDL_GetMouseState(x, y); }

bool SDLDisplayManager::isWindowFocused(FilamentApp2::Window::Handle window) const {
    return (SDL_GetWindowFlags((SDL_Window*) window) & SDL_WINDOW_INPUT_FOCUS) != 0;
}

void SDLDisplayManager::pollEvents(std::vector<AppEvent>& events) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        AppEvent appEvent;
        appEvent.windowId = nullptr;

        switch (event.type) {
            case SDL_QUIT:
                appEvent.type = AppEvent::Type::QUIT;
                events.push_back(appEvent);
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                appEvent.type = (event.type == SDL_KEYDOWN) ? AppEvent::Type::KEYDOWN
                                                            : AppEvent::Type::KEYUP;
                appEvent.key.code = mapKey(event.key.keysym.scancode);
                appEvent.key.modifiers = getModifiers();
                appEvent.windowId = (void*) SDL_GetWindowFromID(event.key.windowID);
                events.push_back(appEvent);
                break;
            case SDL_TEXTINPUT:
                appEvent.type = AppEvent::Type::TEXTINPUT;
                strncpy(appEvent.text.text, event.text.text, sizeof(appEvent.text.text) - 1);
                appEvent.text.text[sizeof(appEvent.text.text) - 1] = '\0';
                appEvent.windowId = (void*) SDL_GetWindowFromID(event.text.windowID);
                events.push_back(appEvent);
                break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                appEvent.type = (event.type == SDL_MOUSEBUTTONDOWN)
                                        ? AppEvent::Type::MOUSE_BUTTON_DOWN
                                        : AppEvent::Type::MOUSE_BUTTON_UP;
                appEvent.mouseButton.button = event.button.button;
                appEvent.mouseButton.x = event.button.x;
                appEvent.mouseButton.y = event.button.y;
                appEvent.windowId = (void*) SDL_GetWindowFromID(event.button.windowID);
                events.push_back(appEvent);
                break;
            case SDL_MOUSEMOTION:
                appEvent.type = AppEvent::Type::MOUSE_MOVE;
                appEvent.mouseMove.x = event.motion.x;
                appEvent.mouseMove.y = event.motion.y;
                appEvent.windowId = (void*) SDL_GetWindowFromID(event.motion.windowID);
                events.push_back(appEvent);
                break;
            case SDL_MOUSEWHEEL:
                appEvent.type = AppEvent::Type::MOUSE_WHEEL;
                appEvent.mouseWheel.delta = event.wheel.y;
                appEvent.windowId = (void*) SDL_GetWindowFromID(event.wheel.windowID);
                events.push_back(appEvent);
                break;
            case SDL_DROPFILE:
                appEvent.type = AppEvent::Type::DROP_FILE;
                appEvent.dropFile.path = event.drop.file;
                appEvent.windowId = (void*) SDL_GetWindowFromID(event.drop.windowID);
                events.push_back(appEvent);
                break;
            case SDL_WINDOWEVENT:
                appEvent.windowId = (void*) SDL_GetWindowFromID(event.window.windowID);
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    appEvent.type = AppEvent::Type::RESIZED;
                    appEvent.resize.w = (uint32_t) event.window.data1;
                    appEvent.resize.h = (uint32_t) event.window.data2;
                    events.push_back(appEvent);
                }
                break;
            default:
                break;
        }
    }
}

void SDLDisplayManager::onWindowResized(FilamentApp2::Window::Handle window) {
#if defined(__APPLE__)
    void* nativeWindow = getNativeWindow(window);
    if (mConfig.backend == filament::Engine::Backend::METAL ||
            mConfig.backend == filament::Engine::Backend::VULKAN ||
            mConfig.backend == filament::Engine::Backend::WEBGPU) {
        resizeMetalLayer(nativeWindow);
    }
#endif
}

double SDLDisplayManager::getTime() const {
    return (double) SDL_GetPerformanceCounter() / SDL_GetPerformanceFrequency();
}

uint16_t SDLDisplayManager::getModifiers() {
    SDL_Keymod mod = SDL_GetModState();
    uint16_t modifiers = 0;
    if (mod & KMOD_LSHIFT) modifiers |= AppKeyModifier::LSHIFT;
    if (mod & KMOD_RSHIFT) modifiers |= AppKeyModifier::RSHIFT;
    if (mod & KMOD_LCTRL) modifiers |= AppKeyModifier::LCTRL;
    if (mod & KMOD_RCTRL) modifiers |= AppKeyModifier::RCTRL;
    if (mod & KMOD_LALT) modifiers |= AppKeyModifier::LALT;
    if (mod & KMOD_RALT) modifiers |= AppKeyModifier::RALT;
    if (mod & KMOD_LGUI) modifiers |= AppKeyModifier::LSUPER;
    if (mod & KMOD_RGUI) modifiers |= AppKeyModifier::RSUPER;
    return modifiers;
}

AppKey SDLDisplayManager::mapKey(SDL_Scancode scancode) {
    switch (scancode) {
        case SDL_SCANCODE_ESCAPE:
            return AppKey::ESCAPE;
        case SDL_SCANCODE_RETURN:
            return AppKey::ENTER;
        case SDL_SCANCODE_TAB:
            return AppKey::TAB;
        case SDL_SCANCODE_BACKSPACE:
            return AppKey::BACKSPACE;
        case SDL_SCANCODE_INSERT:
            return AppKey::INSERT;
        case SDL_SCANCODE_DELETE:
            return AppKey::DEL;
        case SDL_SCANCODE_RIGHT:
            return AppKey::RIGHT;
        case SDL_SCANCODE_LEFT:
            return AppKey::LEFT;
        case SDL_SCANCODE_DOWN:
            return AppKey::DOWN;
        case SDL_SCANCODE_UP:
            return AppKey::UP;
        case SDL_SCANCODE_PAGEUP:
            return AppKey::PAGE_UP;
        case SDL_SCANCODE_PAGEDOWN:
            return AppKey::PAGE_DOWN;
        case SDL_SCANCODE_HOME:
            return AppKey::HOME;
        case SDL_SCANCODE_END:
            return AppKey::END;
        case SDL_SCANCODE_CAPSLOCK:
            return AppKey::CAPS_LOCK;
        case SDL_SCANCODE_SCROLLLOCK:
            return AppKey::SCROLL_LOCK;
        case SDL_SCANCODE_NUMLOCKCLEAR:
            return AppKey::NUM_LOCK;
        case SDL_SCANCODE_PRINTSCREEN:
            return AppKey::PRINT_SCREEN;
        case SDL_SCANCODE_PAUSE:
            return AppKey::PAUSE;
        case SDL_SCANCODE_KP_0:
            return AppKey::KEYPAD_0;
        case SDL_SCANCODE_KP_1:
            return AppKey::KEYPAD_1;
        case SDL_SCANCODE_KP_2:
            return AppKey::KEYPAD_2;
        case SDL_SCANCODE_KP_3:
            return AppKey::KEYPAD_3;
        case SDL_SCANCODE_KP_4:
            return AppKey::KEYPAD_4;
        case SDL_SCANCODE_KP_5:
            return AppKey::KEYPAD_5;
        case SDL_SCANCODE_KP_6:
            return AppKey::KEYPAD_6;
        case SDL_SCANCODE_KP_7:
            return AppKey::KEYPAD_7;
        case SDL_SCANCODE_KP_8:
            return AppKey::KEYPAD_8;
        case SDL_SCANCODE_KP_9:
            return AppKey::KEYPAD_9;
        case SDL_SCANCODE_KP_PERIOD:
            return AppKey::KEYPAD_DECIMAL;
        case SDL_SCANCODE_KP_DIVIDE:
            return AppKey::KEYPAD_DIVIDE;
        case SDL_SCANCODE_KP_MULTIPLY:
            return AppKey::KEYPAD_MULTIPLY;
        case SDL_SCANCODE_KP_MINUS:
            return AppKey::KEYPAD_SUBTRACT;
        case SDL_SCANCODE_KP_PLUS:
            return AppKey::KEYPAD_ADD;
        case SDL_SCANCODE_KP_ENTER:
            return AppKey::KEYPAD_ENTER;
        case SDL_SCANCODE_KP_EQUALS:
            return AppKey::KEYPAD_EQUAL;
        case SDL_SCANCODE_LCTRL:
            return AppKey::LEFT_CTRL;
        case SDL_SCANCODE_LSHIFT:
            return AppKey::LEFT_SHIFT;
        case SDL_SCANCODE_LALT:
            return AppKey::LEFT_ALT;
        case SDL_SCANCODE_LGUI:
            return AppKey::LEFT_SUPER;
        case SDL_SCANCODE_RCTRL:
            return AppKey::RIGHT_CTRL;
        case SDL_SCANCODE_RSHIFT:
            return AppKey::RIGHT_SHIFT;
        case SDL_SCANCODE_RALT:
            return AppKey::RIGHT_ALT;
        case SDL_SCANCODE_RGUI:
            return AppKey::RIGHT_SUPER;
        case SDL_SCANCODE_MENU:
            return AppKey::MENU;
        case SDL_SCANCODE_A:
            return AppKey::A;
        case SDL_SCANCODE_B:
            return AppKey::B;
        case SDL_SCANCODE_C:
            return AppKey::C;
        case SDL_SCANCODE_D:
            return AppKey::D;
        case SDL_SCANCODE_E:
            return AppKey::E;
        case SDL_SCANCODE_F:
            return AppKey::F;
        case SDL_SCANCODE_G:
            return AppKey::G;
        case SDL_SCANCODE_H:
            return AppKey::H;
        case SDL_SCANCODE_I:
            return AppKey::I;
        case SDL_SCANCODE_J:
            return AppKey::J;
        case SDL_SCANCODE_K:
            return AppKey::K;
        case SDL_SCANCODE_L:
            return AppKey::L;
        case SDL_SCANCODE_M:
            return AppKey::M;
        case SDL_SCANCODE_N:
            return AppKey::N;
        case SDL_SCANCODE_O:
            return AppKey::O;
        case SDL_SCANCODE_P:
            return AppKey::P;
        case SDL_SCANCODE_Q:
            return AppKey::Q;
        case SDL_SCANCODE_R:
            return AppKey::R;
        case SDL_SCANCODE_S:
            return AppKey::S;
        case SDL_SCANCODE_T:
            return AppKey::T;
        case SDL_SCANCODE_U:
            return AppKey::U;
        case SDL_SCANCODE_V:
            return AppKey::V;
        case SDL_SCANCODE_W:
            return AppKey::W;
        case SDL_SCANCODE_X:
            return AppKey::X;
        case SDL_SCANCODE_Y:
            return AppKey::Y;
        case SDL_SCANCODE_Z:
            return AppKey::Z;
        case SDL_SCANCODE_0:
            return AppKey::_0;
        case SDL_SCANCODE_1:
            return AppKey::_1;
        case SDL_SCANCODE_2:
            return AppKey::_2;
        case SDL_SCANCODE_3:
            return AppKey::_3;
        case SDL_SCANCODE_4:
            return AppKey::_4;
        case SDL_SCANCODE_5:
            return AppKey::_5;
        case SDL_SCANCODE_6:
            return AppKey::_6;
        case SDL_SCANCODE_7:
            return AppKey::_7;
        case SDL_SCANCODE_8:
            return AppKey::_8;
        case SDL_SCANCODE_9:
            return AppKey::_9;
        case SDL_SCANCODE_F1:
            return AppKey::F1;
        case SDL_SCANCODE_F2:
            return AppKey::F2;
        case SDL_SCANCODE_F3:
            return AppKey::F3;
        case SDL_SCANCODE_F4:
            return AppKey::F4;
        case SDL_SCANCODE_F5:
            return AppKey::F5;
        case SDL_SCANCODE_F6:
            return AppKey::F6;
        case SDL_SCANCODE_F7:
            return AppKey::F7;
        case SDL_SCANCODE_F8:
            return AppKey::F8;
        case SDL_SCANCODE_F9:
            return AppKey::F9;
        case SDL_SCANCODE_F10:
            return AppKey::F10;
        case SDL_SCANCODE_F11:
            return AppKey::F11;
        case SDL_SCANCODE_F12:
            return AppKey::F12;
        case SDL_SCANCODE_F13:
            return AppKey::F13;
        case SDL_SCANCODE_F14:
            return AppKey::F14;
        case SDL_SCANCODE_F15:
            return AppKey::F15;
        case SDL_SCANCODE_F16:
            return AppKey::F16;
        case SDL_SCANCODE_F17:
            return AppKey::F17;
        case SDL_SCANCODE_F18:
            return AppKey::F18;
        case SDL_SCANCODE_F19:
            return AppKey::F19;
        case SDL_SCANCODE_F20:
            return AppKey::F20;
        case SDL_SCANCODE_F21:
            return AppKey::F21;
        case SDL_SCANCODE_F22:
            return AppKey::F22;
        case SDL_SCANCODE_F23:
            return AppKey::F23;
        case SDL_SCANCODE_F24:
            return AppKey::F24;
        case SDL_SCANCODE_AC_BACK:
            return AppKey::APP_BACK;
        case SDL_SCANCODE_AC_FORWARD:
            return AppKey::APP_FORWARD;
        case SDL_SCANCODE_SPACE:
            return AppKey::SPACE;
        case SDL_SCANCODE_COMMA:
            return AppKey::COMMA;
        case SDL_SCANCODE_PERIOD:
            return AppKey::PERIOD;
        case SDL_SCANCODE_SEMICOLON:
            return AppKey::SEMICOLON;
        case SDL_SCANCODE_GRAVE:
            return AppKey::GRAVE;
        case SDL_SCANCODE_MINUS:
            return AppKey::MINUS;
        case SDL_SCANCODE_EQUALS:
            return AppKey::EQUAL;
        case SDL_SCANCODE_LEFTBRACKET:
            return AppKey::LEFT_BRACKET;
        case SDL_SCANCODE_RIGHTBRACKET:
            return AppKey::RIGHT_BRACKET;
        case SDL_SCANCODE_BACKSLASH:
            return AppKey::BACKSLASH;
        case SDL_SCANCODE_APOSTROPHE:
            return AppKey::APOSTROPHE;
        case SDL_SCANCODE_SLASH:
            return AppKey::SLASH;
        default:
            return AppKey::UNKNOWN;
    }
}

void SDLDisplayManager::startRendering(std::function<bool()> doFrame) {
    while (!doFrame()) {
        // Delay rendering for roughly one monitor refresh interval
        // TODO: Use SDL_GL_SetSwapInterval for proper vsync
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

} // namespace filament::app
