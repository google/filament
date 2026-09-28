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

#ifndef TNT_FILAMENT_FILAMENTAPP_HEADLESS_DISPLAY_MANAGER_H
#define TNT_FILAMENT_FILAMENTAPP_HEADLESS_DISPLAY_MANAGER_H

#include <filamentapp/DisplayManager.h>

#include <utils/CString.h>

#include <chrono>
#include <cstdint>
#include <queue>
#include <unordered_set>
#include <vector>

namespace filament::app {

/**
 * A display manager that renders offscreen: no OS window, no event source, no presentation.
 *
 * This class is not thread-safe and holds no lock. Its event queue, mouse state and synthetic
 * clock are plain fields that the frame loop reads through pollEvents(), getMouseState() and
 * getTime(), so every method, including the scripted-input and synthetic-clock helpers, must be
 * called from the thread running that loop. In practice a test drives it from a FilamentApp2
 * callback (animation, pre-render or post-render), each of which runs inline in the loop. Calling
 * pushEvent() from another thread while the loop runs is a data race on the event queue rather
 * than a merely stale read.
 */
class HeadlessDisplayManager : public DisplayManager {
public:
    HeadlessDisplayManager();
    ~HeadlessDisplayManager() override;

    void terminate() override;

    bool isHeadless() const override { return true; }

    WindowHandle createWindow(const char* title, uint32_t w, uint32_t h, bool resizable) override;
    void destroyWindow(WindowHandle window) override;

    void* getNativeWindow(WindowHandle window) const override;

    void setWindowTitle(WindowHandle window, const char* title) override;
    void getWindowSize(WindowHandle window, uint32_t* w, uint32_t* h) const override;
    void getDrawableSize(WindowHandle window, uint32_t* w, uint32_t* h) const override;

    uint32_t getMouseState(int* x, int* y) const override;
    bool isWindowFocused(WindowHandle window) const override;

    void pollEvents(std::vector<AppEvent>& events) override;

    double getTime() const override;

    // Nothing waits on wall-clock time offscreen, only on each frame completing, so this loop is
    // left unpaced. The default pacing sleep would otherwise dominate batch tools that render many
    // thousands of frames: one measured workload took roughly 115s when paced against 35s when
    // unpaced, for identical output.
    void runFrameLoop(FrameFn frame) override {
        while (!frame()) {
        }
    }

    // Scripted input and synthetic clock, for deterministic testing. These follow the threading
    // contract documented above: call them from the thread running the frame loop, which in
    // practice means from a FilamentApp2 callback.
    void pushEvent(AppEvent const& event);
    void setWindowFocused(bool focused);
    void setTime(double time);
    void advanceTime(double delta);

private:
    struct WindowInfo {
        utils::CString title;
        uint32_t width = 0;
        uint32_t height = 0;
    };

    std::unordered_set<WindowInfo*> mWindows;
    std::queue<AppEvent> mEventQueue;
    uint32_t mMouseButtons = 0;
    int32_t mMouseX = 0;
    int32_t mMouseY = 0;
    // Defaults to focused, matching DisplayManager::isWindowFocused(). An offscreen window is
    // always the one receiving scripted input, and FilamentAppGui forwards the pointer position to
    // ImGui only while the window is focused, so a default of false would let scripted clicks
    // reach the button state but never a widget. Tests can still call setWindowFocused(false) to
    // exercise the unfocused path.
    bool mWindowFocused = true;
    bool mUseSyntheticTime = false;
    double mSyntheticTime = 0.0;
    std::chrono::steady_clock::time_point mStartTime;
};

} // namespace filament::app

#endif // TNT_FILAMENT_FILAMENTAPP_HEADLESS_DISPLAY_MANAGER_H
