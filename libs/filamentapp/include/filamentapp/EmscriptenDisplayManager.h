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

#ifndef TNT_FILAMENT_FILAMENTAPP_EMSCRIPTEN_DISPLAY_MANAGER_H
#define TNT_FILAMENT_FILAMENTAPP_EMSCRIPTEN_DISPLAY_MANAGER_H

#include <filamentapp/DisplayManager.h>

#include <emscripten/html5.h>
#include <emscripten/html5_webgl.h>

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include <stdint.h>

namespace filament::app {

/**
 * DisplayManager backed by a single HTML canvas element, for Emscripten builds.
 *
 * There is exactly one window, the canvas, so createWindow() resizes it rather than creating
 * anything. PlatformWebGL and WebGPUPlatformWasm both take the canvas' CSS selector as their
 * native window handle, so this manager serves either backend.
 *
 * For the OpenGL backend it also creates the WebGL 2.0 context and makes it current, which in a
 * filament.js build is JavaScript's job. For the WebGPU backend it creates nothing: a canvas can
 * hold only one kind of context, and WebGPUPlatformWasm configures the canvas itself. The WebGPU
 * device, which can only be requested asynchronously, must be placed in
 * Module.preinitializedWebGPUDevice by the page before main() runs. samples/wasm_shell.html.in
 * does this when argv selects WebGPU.
 *
 * It is never headless, so rendering always targets the canvas. Under OpenGL nothing else is
 * possible, since PlatformWebGL::createSwapChain(width, height) returns nullptr. The WebGPU driver
 * can render offscreen, but this manager has no frame loop that runs without a canvas.
 */
class EmscriptenDisplayManager : public DisplayManager {
public:
    /**
     * @param backend the backend the Engine will be created with; determines whether a WebGL
     *                context is created.
     * @param canvasSelector CSS selector of the canvas element, e.g. "#canvas".
     */
    explicit EmscriptenDisplayManager(filament::Engine::Backend backend,
            const char* canvasSelector = "#canvas");
    ~EmscriptenDisplayManager() override;

    EmscriptenDisplayManager(EmscriptenDisplayManager const&) = delete;
    EmscriptenDisplayManager& operator=(EmscriptenDisplayManager const&) = delete;

    void terminate() override;

    WindowHandle createWindow(const char* title, uint32_t w, uint32_t h, bool resizable) override;
    void destroyWindow(WindowHandle window) override;

    void* getNativeWindow(WindowHandle window) const override;

    void setWindowTitle(WindowHandle window, const char* title) override;
    void getWindowSize(WindowHandle window, uint32_t* w, uint32_t* h) const override;
    void getDrawableSize(WindowHandle window, uint32_t* w, uint32_t* h) const override;

    uint32_t getMouseState(int* x, int* y) const override;
    bool isWindowFocused(WindowHandle window) const override;

    void pollEvents(std::vector<AppEvent>& events) override;

    /**
     * Returns the requestAnimationFrame timestamp of the frame in progress, so that every read
     * within a frame agrees. It is still wall time paced by the display, so a reproducible run
     * must pass --fixed-timestep, which derives time from the frame index instead.
     */
    double getTime() const override;

    /**
     * Registers `frame` against requestAnimationFrame and returns immediately.
     *
     * Blocking the browser's thread would stop compositing, input delivery and every asynchronous
     * completion, so there is no loop to run here. Pacing is rAF's job as well.
     */
    void runFrameLoop(FrameFn frame) override;

    /**
     * Sets a callback invoked once, when the run ends, to tell a harness driving the page that no
     * further frame is coming.
     *
     * It fires after the final frame and after FilamentApp2::shutdown(), so any output written to
     * the virtual filesystem is complete by then, and it fires on teardown through terminate() as
     * well.
     */
    void setExitCallback(std::function<void()> callback) { mExitCallback = std::move(callback); }

private:
    static EM_BOOL onMouseEvent(int eventType, EmscriptenMouseEvent const* event, void* userData);
    static EM_BOOL onWheelEvent(int eventType, EmscriptenWheelEvent const* event, void* userData);
    static EM_BOOL onKeyEvent(int eventType, EmscriptenKeyboardEvent const* event, void* userData);

    static EM_BOOL onAnimationFrame(double time, void* userData);

    static AppKey mapKey(char const* domCode);
    static uint16_t mapModifiers(EmscriptenKeyboardEvent const* event);

    // Ends the run and reports it, at most once. Every path that stops the loop goes through here.
    void endFrameLoop();

    void registerEventCallbacks();
    void unregisterEventCallbacks();

    bool createWebGLContext();

    filament::Engine::Backend const mBackend;

    // Held by value so that getNativeWindow() can hand out a pointer with the lifetime of this
    // object: the backend platforms keep the pointer and dereference it lazily.
    std::string const mCanvasSelector;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE mGLContext = 0;
    bool mCallbacksRegistered = false;

    // The canvas' backing store size as of the last poll, against which a resize is detected.
    // createWindow() seeds it with the size it set, so that size is not reported as a resize.
    bool mWindowCreated = false;
    uint32_t mWindowWidth = 0;
    uint32_t mWindowHeight = 0;

    std::vector<AppEvent> mPendingEvents;

    // Both outlive runFrameLoop(), which returns as soon as the callback is registered.
    FrameFn mFrameFn;
    std::function<void()> mExitCallback;
    bool mFrameLoopActive = false;

    // Timestamp of the frame in progress, in milliseconds on the same origin as
    // emscripten_get_now(); zero until the first frame arrives.
    double mFrameTimeMs = 0.0;

    int mMouseX = 0;
    int mMouseY = 0;
    uint32_t mMouseButtons = 0;
};

} // namespace filament::app

#endif // TNT_FILAMENT_FILAMENTAPP_EMSCRIPTEN_DISPLAY_MANAGER_H
