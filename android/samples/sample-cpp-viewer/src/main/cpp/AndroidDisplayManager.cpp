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

#include "AndroidDisplayManager.h"

#include <filamentapp/AppEvent.h>

#include <utils/Mutex.h>

#include <android/native_window.h>

#include <chrono>
#include <cstdint>
#include <utility>
#include <vector>


namespace filament::app {

AndroidDisplayManager::AndroidDisplayManager()
        : mStartTime(std::chrono::steady_clock::now()) {
}

AndroidDisplayManager::~AndroidDisplayManager() {
    terminate();
}

void AndroidDisplayManager::terminate() {
    mNativeWindow = nullptr;
    utils::LockGuard const lock(mMutex);
    mEventQueue.clear();
}

WindowHandle AndroidDisplayManager::createWindow(const char* title, uint32_t w, uint32_t h,
        bool resizable) {
    return windowHandle();
}

void AndroidDisplayManager::destroyWindow(WindowHandle window) {
    // The native window is owned by the caller of setNativeWindow().
}

void* AndroidDisplayManager::getNativeWindow(WindowHandle window) const {
    return window ? mNativeWindow : nullptr;
}

void AndroidDisplayManager::setWindowTitle(WindowHandle window, const char* title) {}

void AndroidDisplayManager::getWindowSize(WindowHandle window, uint32_t* w, uint32_t* h) const {
    ANativeWindow* nw = static_cast<ANativeWindow*>(getNativeWindow(window));
    int32_t const width = nw ? ANativeWindow_getWidth(nw) : 0;
    int32_t const height = nw ? ANativeWindow_getHeight(nw) : 0;
    // ANativeWindow_get{Width,Height} return a negative error code on failure.
    if (w) *w = width > 0 ? uint32_t(width) : 0;
    if (h) *h = height > 0 ? uint32_t(height) : 0;
}

void AndroidDisplayManager::getDrawableSize(WindowHandle window, uint32_t* w, uint32_t* h) const {
    getWindowSize(window, w, h);
}

void AndroidDisplayManager::pollEvents(std::vector<AppEvent>& events) {
    utils::LockGuard const lock(mMutex);
    events.insert(events.end(), mEventQueue.begin(), mEventQueue.end());
    mEventQueue.clear();
}

uint32_t AndroidDisplayManager::getMouseState(int* x, int* y) const {
    return 0;
}

double AndroidDisplayManager::getTime() const {
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = now - mStartTime;
    return diff.count();
}

void AndroidDisplayManager::onFrameFinished(WindowHandle window, filament::Engine* engine,
        filament::Renderer* renderer) {}

void AndroidDisplayManager::setNativeWindow(ANativeWindow* window) noexcept {
    mNativeWindow = window;
}

void AndroidDisplayManager::pushEvent(const AppEvent& event) {
    utils::LockGuard const lock(mMutex);
    mEventQueue.push_back(event);
}

void AndroidDisplayManager::pushTouchEvent(int action, float x, float y) {
    AppEvent event;
    event.windowId = windowHandle();
    switch (action) {
        case 0: // ACTION_DOWN
            event.type = AppEvent::Type::MOUSE_BUTTON_DOWN;
            event.mouseButton.button = 1;
            event.mouseButton.x = (int32_t) x;
            event.mouseButton.y = (int32_t) y;
            pushEvent(event);
            break;
        case 1: // ACTION_UP
        case 3: // ACTION_CANCEL
            event.type = AppEvent::Type::MOUSE_BUTTON_UP;
            event.mouseButton.button = 1;
            event.mouseButton.x = (int32_t) x;
            event.mouseButton.y = (int32_t) y;
            pushEvent(event);
            break;
        case 2: // ACTION_MOVE
            event.type = AppEvent::Type::MOUSE_MOVE;
            event.mouseMove.x = (int32_t) x;
            event.mouseMove.y = (int32_t) y;
            pushEvent(event);
            break;
        default:
            break;
    }
}

} // namespace filament::app
