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

#ifndef TNT_FILAMENT_FILAMENTAPP_SDL_DISPLAY_MANAGER_H
#define TNT_FILAMENT_FILAMENTAPP_SDL_DISPLAY_MANAGER_H

#include <filamentapp/DisplayManager.h>

#include <SDL.h>

#include <unordered_map>

namespace filament::app {

class SDLDisplayManager : public DisplayManager {
public:
    SDLDisplayManager(filament::Engine::Backend);
    ~SDLDisplayManager() override;

    void terminate() override;

    WindowHandle createWindow(const char* title, uint32_t w, uint32_t h,
            bool resizable, bool headless) override;
    void destroyWindow(WindowHandle window) override;

    void* getNativeWindow(WindowHandle window) const override;

    void setWindowTitle(WindowHandle window, const char* title) override;
    void getWindowSize(WindowHandle window, uint32_t* w,
            uint32_t* h) const override;
    void getDrawableSize(WindowHandle window, uint32_t* w,
            uint32_t* h) const override;

    uint32_t getMouseState(int* x, int* y) const override;
    bool isWindowFocused(WindowHandle window) const override;

    void pollEvents(std::vector<AppEvent>& events) override;

    void onWindowResized(WindowHandle window) override;

    double getTime() const override;

    void startRendering(std::function<bool()> doFrame) override;

private:
    bool init();

    filament::Engine::Backend const mBackend;
    mutable std::unordered_map<WindowHandle, void*> mNativeWindowMap;
    static AppKey mapKey(SDL_Scancode scancode);
    static uint16_t getModifiers();
};

} // namespace filament::app

#endif // TNT_FILAMENT_FILAMENTAPP_SDL_DISPLAY_MANAGER_H
