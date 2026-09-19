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

#include <filament/Engine.h>
#include <filament/Renderer.h>

#include <chrono>
#include <cstdint>
#include <functional>
#include <thread>
#include <vector>

namespace filament::app {

/**
 * Display manager interface for FilamentApp.
 */
class DisplayManager {
public:
    virtual ~DisplayManager() = default;

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
     * @return A handle to the created window.
     */
    virtual WindowHandle createWindow(const char* title, uint32_t w, uint32_t h,
            bool resizable) = 0;

    /**
     * Destroys a window.
     * @param window The handle of the window to destroy.
     */
    virtual void destroyWindow(WindowHandle window) = 0;

    /**
     * Returns the underlying native window handle for the specified platform window.
     * @param window The platform window handle.
     * @return The native window handle (e.g., HWND, NSWindow*). Returns nullptr for headless/web
     * platforms.
     */
    virtual void* getNativeWindow(WindowHandle window) const = 0;

    /**
     * Sets the title of a window.
     * @param window The window handle.
     * @param title The new title.
     */
    virtual void setWindowTitle(WindowHandle window, const char* title) = 0;

    /**
     * Returns the size of a window.
     * @param window The window handle.
     * @param w Pointer to store the width.
     * @param h Pointer to store the height.
     */
    virtual void getWindowSize(WindowHandle window, uint32_t* w,
            uint32_t* h) const = 0;

    /**
     * Returns the drawable size of a window.
     * @param window The window handle.
     * @param w Pointer to store the width.
     * @param h Pointer to store the height.
     */
    virtual void getDrawableSize(WindowHandle window, uint32_t* w,
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
    virtual void onWindowResized(WindowHandle window) {}

    /**
     * Returns the current mouse state.
     * @param x Pointer to store the x coordinate.
     * @param y Pointer to store the y coordinate.
     * @return Bitmask of pressed buttons.
     */
    virtual uint32_t getMouseState(int* x, int* y) const {
        if (x) *x = 0;
        if (y) *y = 0;
        return 0;
    }

    /**
     * Returns whether the specified window has input focus.
     * @param window The window handle.
     * @return true if the window has focus.
     */
    virtual bool isWindowFocused(WindowHandle window) const { return true; }

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
    virtual void onFrameFinished(WindowHandle window, filament::Engine* engine,
            filament::Renderer* renderer) {}

    /**
     * Renders a single frame on behalf of runFrameLoop(). Returns true once the application has
     * finished, after which it must not be invoked again.
     */
    using FrameFn = std::function<bool()>;

    /**
     * Drives frames until `frame` returns true. Called exactly once per run.
     *
     * The frame loop belongs to the display manager because the frame source does, and the two
     * forms that source takes cannot be reconciled behind a narrower hook. When the application
     * owns the thread, this is a blocking loop, and what it blocks on is the manager's concern:
     * the default implementation sleeps, pacing to roughly 60Hz so that an interactive application
     * does not consume a core between frames, whereas another may block on vsync, on a remote
     * client acknowledging the previous frame, or on nothing at all. When the frame source belongs
     * to the platform instead, as with a browser's requestAnimationFrame or Android's
     * Choreographer, there is no loop to run: such an implementation registers `frame` with that
     * source and returns immediately.
     *
     * Teardown is not the implementation's responsibility. `frame` calls FilamentApp2::shutdown()
     * before it reports completion, so nothing remains to be done once this returns, and an
     * implementation that returns early must not treat its own return as the end of the run.
     */
    virtual void runFrameLoop(FrameFn frame) {
        while (!frame()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }

    /**
     * Returns whether this display manager renders offscreen, without an OS window.
     *
     * This governs surface handling only. A headless manager receives an offscreen swap chain
     * sized from getWindowSize() rather than one created from getNativeWindow(), and its window
     * size is taken as its drawable size, without DPI scaling. Frame delivery and pacing are
     * independent of this property and belong to runFrameLoop().
     *
     * An implementation that returns true must return nullptr from getNativeWindow(). The converse
     * does not hold: a windowed manager may also report nullptr before its surface exists.
     */
    virtual bool isHeadless() const { return false; }
};

} // namespace filament::app

#endif // TNT_FILAMENT_FILAMENTAPP_DISPLAY_MANAGER_H
