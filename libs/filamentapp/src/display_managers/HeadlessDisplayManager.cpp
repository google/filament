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

#include <filamentapp/HeadlessDisplayManager.h>

namespace filament::app {

HeadlessDisplayManager::HeadlessDisplayManager()
        : mStartTime(std::chrono::steady_clock::now()) {}

HeadlessDisplayManager::~HeadlessDisplayManager() { terminate(); }

void HeadlessDisplayManager::terminate() {
    for (auto* info: mWindows) {
        delete info;
    }
    mWindows.clear();
}

WindowHandle HeadlessDisplayManager::createWindow(const char* title, uint32_t w, uint32_t h,
        bool resizable) {
    auto* info = new WindowInfo{ utils::CString(title ? title : ""), w, h };
    mWindows.insert(info);
    return static_cast<WindowHandle>(info);
}

void HeadlessDisplayManager::destroyWindow(WindowHandle window) {
    auto* info = static_cast<WindowInfo*>(window);
    if (mWindows.erase(info) > 0) {
        delete info;
    }
}

void* HeadlessDisplayManager::getNativeWindow(WindowHandle window) const { return nullptr; }

void HeadlessDisplayManager::setWindowTitle(WindowHandle window, const char* title) {
    auto* info = static_cast<WindowInfo*>(window);
    if (!info || mWindows.count(info) == 0) {
        return;
    }
    info->title = utils::CString(title ? title : "");
}

void HeadlessDisplayManager::getWindowSize(WindowHandle window, uint32_t* w, uint32_t* h) const {
    auto const* info = static_cast<WindowInfo const*>(window);
    if (!info || mWindows.count(const_cast<WindowInfo*>(info)) == 0) {
        if (w) *w = 0;
        if (h) *h = 0;
        return;
    }
    if (w) {
        *w = info->width;
    }
    if (h) {
        *h = info->height;
    }
}

void HeadlessDisplayManager::getDrawableSize(WindowHandle window, uint32_t* w, uint32_t* h) const {
    getWindowSize(window, w, h);
}

uint32_t HeadlessDisplayManager::getMouseState(int* x, int* y) const {
    if (x) {
        *x = mMouseX;
    }
    if (y) {
        *y = mMouseY;
    }
    return mMouseButtons;
}

bool HeadlessDisplayManager::isWindowFocused(WindowHandle window) const { return mWindowFocused; }

void HeadlessDisplayManager::pollEvents(std::vector<AppEvent>& events) {
    while (!mEventQueue.empty()) {
        AppEvent event = mEventQueue.front();
        mEventQueue.pop();

        switch (event.type) {
            case AppEvent::Type::MOUSE_MOVE:
                mMouseX = event.mouseMove.x;
                mMouseY = event.mouseMove.y;
                break;
            case AppEvent::Type::MOUSE_BUTTON_DOWN:
                mMouseX = event.mouseButton.x;
                mMouseY = event.mouseButton.y;
                if (event.mouseButton.button > 0 && event.mouseButton.button <= 32) {
                    mMouseButtons |= (1u << (event.mouseButton.button - 1));
                }
                break;
            case AppEvent::Type::MOUSE_BUTTON_UP:
                mMouseX = event.mouseButton.x;
                mMouseY = event.mouseButton.y;
                if (event.mouseButton.button > 0 && event.mouseButton.button <= 32) {
                    mMouseButtons &= ~(1u << (event.mouseButton.button - 1));
                }
                break;
            case AppEvent::Type::RESIZED: {
                // A scripted resize is applied to the manager's own record of the window, so that
                // the size the application reads back afterwards agrees with the event it saw. A
                // test may leave windowId unset, which is unambiguous as long as a single window
                // exists; otherwise the event is forwarded without updating any window.
                auto* info = static_cast<WindowInfo*>(event.windowId);
                if (info && mWindows.count(info) > 0) {
                    info->width = event.resize.w;
                    info->height = event.resize.h;
                } else if (event.windowId == nullptr && mWindows.size() == 1) {
                    (*mWindows.begin())->width = event.resize.w;
                    (*mWindows.begin())->height = event.resize.h;
                }
                break;
            }
            default:
                break;
        }

        events.push_back(event);
    }
}

double HeadlessDisplayManager::getTime() const {
    if (mUseSyntheticTime) {
        return mSyntheticTime;
    }
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - mStartTime).count();
}

void HeadlessDisplayManager::pushEvent(AppEvent const& event) { mEventQueue.push(event); }

void HeadlessDisplayManager::setWindowFocused(bool focused) { mWindowFocused = focused; }

// Either helper switches the clock to synthetic time permanently: once a test takes control of
// time, returning to the real clock mid-run would make the timestep jump.
void HeadlessDisplayManager::setTime(double time) {
    mUseSyntheticTime = true;
    mSyntheticTime = time;
}

void HeadlessDisplayManager::advanceTime(double delta) {
    // Advancing before any setTime() starts from the time already elapsed, so that the clock a
    // test steps forward continues from where the real one left off rather than from zero.
    if (!mUseSyntheticTime) {
        mSyntheticTime = getTime();
        mUseSyntheticTime = true;
    }
    mSyntheticTime += delta;
}

} // namespace filament::app
