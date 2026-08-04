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

#ifndef TNT_FILAMENT_FILAMENTAPP_DISPLAY_MANAGER_H
#define TNT_FILAMENT_FILAMENTAPP_DISPLAY_MANAGER_H

#include "AppEvent.h"

#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp2.h>

#include <filament/Engine.h>
#include <filament/Renderer.h>

#include <cstdint>
#include <functional>
#include <vector>

namespace filament::app {

/**
 * Display manager interface for FilamentApp.
 */
class DisplayManager {
public:
    virtual ~DisplayManager() = default;

    /**
     * Initializes the display manager.
     * @param config The application configuration.
     * @return true if successful.
     */
    virtual bool init(const Config& config) = 0;

    /**
     * Terminates the display manager.
     */
    virtual void terminate() = 0;

    /**
     * Creates a new window.
     * @param title The title of the window.
     * @param w The width of the window.
     * @param h The height of the window.
     * @param resizable Whether the window is resizable.
     * @param headless Whether the window should be created in headless mode.
     * @return A handle to the created window.
     */
    virtual FilamentApp2::Window::Handle createWindow(const char* title, uint32_t w, uint32_t h,
            bool resizable, bool headless) = 0;

    /**
     * Destroys a window.
     * @param window The handle of the window to destroy.
     */
    virtual void destroyWindow(FilamentApp2::Window::Handle window) = 0;

    /**
     * Returns the underlying native window handle for the specified platform window.
     * @param window The platform window handle.
     * @return The native window handle (e.g., HWND, NSWindow*). Returns nullptr for headless/web
     * platforms.
     */
    virtual void* getNativeWindow(FilamentApp2::Window::Handle window) const = 0;

    /**
     * Sets the title of a window.
     * @param window The window handle.
     * @param title The new title.
     */
    virtual void setWindowTitle(FilamentApp2::Window::Handle window, const char* title) = 0;

    /**
     * Returns the size of a window.
     * @param window The window handle.
     * @param w Pointer to store the width.
     * @param h Pointer to store the height.
     */
    virtual void getWindowSize(FilamentApp2::Window::Handle window, uint32_t* w,
            uint32_t* h) const = 0;

    /**
     * Returns the drawable size of a window.
     * @param window The window handle.
     * @param w Pointer to store the width.
     * @param h Pointer to store the height.
     */
    virtual void getDrawableSize(FilamentApp2::Window::Handle window, uint32_t* w,
            uint32_t* h) const = 0;

    /**
     * Polls for pending events.
     * @param events Vector to store the polled events.
     */
    virtual void pollEvents(std::vector<AppEvent>& events) = 0;

    /**
     * Called when a window has been resized.
     * @param window The window handle.
     */
    virtual void onWindowResized(FilamentApp2::Window::Handle window) {}

    /**
     * Returns the current mouse state.
     * @param x Pointer to store the x coordinate.
     * @param y Pointer to store the y coordinate.
     * @return Bitmask of pressed buttons.
     */
    virtual uint32_t getMouseState(int* x, int* y) const { return 0; }

    /**
     * Returns whether the specified window has input focus.
     * @param window The window handle.
     * @return true if the window has focus.
     */
    virtual bool isWindowFocused(FilamentApp2::Window::Handle window) const { return true; }

    /**
     * Returns the current time in seconds.
     */
    virtual double getTime() const = 0;

    /**
     * Called when a frame has finished rendering.
     * @param window The window handle.
     * @param engine The Filament engine.
     * @param renderer The Filament renderer.
     */
    virtual void onFrameFinished(FilamentApp2::Window::Handle window, filament::Engine* engine,
            filament::Renderer* renderer) {}

    /**
     * Yields main thread to the display manager to let it drive rendering.
     *
     * On Android, this will not block but provide the app with the doFrame() function, which can
     * then be called from the Choreographer.
     *
     * Similarly, on the Web/WASM (*not* the headless HtmlDisplayManager), the render will be driven
     * by requestAnimationFrame().
     */
    virtual void startRendering(std::function<bool()> doFrame) = 0;
};

} // namespace filament::app

#endif // TNT_FILAMENT_FILAMENTAPP_DISPLAY_MANAGER_H
