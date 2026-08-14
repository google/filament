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

#ifndef TNT_FILAMENT_FILAMENTAPP_ANDROID_DISPLAY_MANAGER_H
#define TNT_FILAMENT_FILAMENTAPP_ANDROID_DISPLAY_MANAGER_H

#include <filamentapp/AppEvent.h>
#include <filamentapp/DisplayManager.h>

#include <filament/Engine.h>
#include <filament/Renderer.h>

#include <utils/compiler.h>
#include <utils/Mutex.h>

#include <jni.h>

#include <chrono>
#include <cstdint>
#include <functional>
#include <vector>

namespace filament::app {

class AndroidDisplayManager : public DisplayManager {
public:
    AndroidDisplayManager(JavaVM* vm, jobject surfaceView);
    ~AndroidDisplayManager() override;

    void terminate() override;

    WindowHandle createWindow(const char* title, uint32_t w, uint32_t h,
            bool resizable, bool headless) override;
    void destroyWindow(WindowHandle window) override;

    void* getNativeWindow(WindowHandle window) const override;

    void setWindowTitle(WindowHandle window, const char* title) override;
    void getWindowSize(WindowHandle window, uint32_t* w, uint32_t* h) const override;
    void getDrawableSize(WindowHandle window, uint32_t* w, uint32_t* h) const override;

    void pollEvents(std::vector<AppEvent>& events) override;

    uint32_t getMouseState(int* x, int* y) const override;
    bool isWindowFocused(WindowHandle window) const override { return true; }

    double getTime() const override;

    void onFrameFinished(WindowHandle window, filament::Engine* engine,
            filament::Renderer* renderer) override;

    // Android-specific lifecycle and event methods
    void setWindowSize(uint32_t w, uint32_t h);
    void pushEvent(const AppEvent& event);
    void pushTouchEvent(int action, float x, float y);

private:
    JavaVM* mJavaVM;
    jobject mSurfaceView = nullptr;
    uint32_t mWidth = 0;
    uint32_t mHeight = 0;
    std::vector<AppEvent> mEventQueue UTILS_GUARDED_BY(mMutex);
    mutable utils::Mutex mMutex;
    std::chrono::time_point<std::chrono::steady_clock> mStartTime;
};

} // namespace filament::app

#endif // TNT_FILAMENT_FILAMENTAPP_ANDROID_DISPLAY_MANAGER_H
