/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef TNT_FILAMENT_SWAPCHAIN_H
#define TNT_FILAMENT_SWAPCHAIN_H

#include <filament/FilamentAPI.h>
#include <backend/DriverEnums.h>
#include <backend/PresentCallable.h>

#include <utils/compiler.h>

namespace filament {

/**
 * A swap chain represents an Operating System's *native* renderable surface.
 *
 * Typically it's a native window or a view. Because a SwapChain is initialized from a
 * native object, it is given to filament as a `void *`, which must be of the proper type
 * for each platform filament is running on.
 *
 * \code
 * SwapChain* swapChain = engine->createSwapChain(nativeWindow);
 * \endcode
 *
 * When Engine::create() is used without specifying a Platform, the `nativeWindow`
 * parameter above must be of type:
 *
 * Platform      | nativeWindow type
 * :-------------|:----------------------------:
 * Android       | ANativeWindow*
 * OSX - OpenGL  | NSView*
 * OSX - Metal   | CAMetalLayer*
 * iOS - OpenGL  | CAEAGLLayer*
 * iOS - Metal   | CAMetalLayer*
 * X11           | Window
 * Windows       | HWND
 *
 * Otherwise, the `nativeWindow` is defined by the concrete implementation of Platform.
 *
 *
 * Examples:
 *
 * Android
 * -------
 *
 * On Android, an `ANativeWindow*` can be obtained from a Java `Surface` object using:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  #include <android/native_window_jni.h>
 *  // parameters
 *  // env:         JNIEnv*
 *  // surface:     jobject
 *  ANativeWindow* win = ANativeWindow_fromSurface(env, surface);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * \warning
 * Don't use reflection to access the `mNativeObject` field, it won't work.
 *
 * A `Surface` can be retrieved from a `SurfaceView` or `SurfaceHolder` easily using
 * `SurfaceHolder.getSurface()` and/or `SurfaceView.getHolder()`.
 *
 * \note
 * To use a `TextureView` as a SwapChain, it is necessary to first get its `SurfaceTexture`,
 * for instance using `TextureView.SurfaceTextureListener` and then create a `Surface`:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.java}
 *  // using a TextureView.SurfaceTextureListener:
 *  public void onSurfaceTextureAvailable(SurfaceTexture surfaceTexture, int width, int height) {
 *      mSurface = new Surface(surfaceTexture);
 *      // mSurface can now be used in JNI to create an ANativeWindow.
 *  }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * Linux
 * -----
 *
 * Example using SDL:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * SDL_SysWMinfo wmi;
 * SDL_VERSION(&wmi.version);
 * SDL_GetWindowWMInfo(sdlWindow, &wmi);
 * Window nativeWindow = (Window) wmi.info.x11.window;
 *
 * using namespace filament;
 * Engine* engine       = Engine::create();
 * SwapChain* swapChain = engine->createSwapChain((void*) nativeWindow);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * Windows
 * -------
 *
 * Example using SDL:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * SDL_SysWMinfo wmi;
 * SDL_VERSION(&wmi.version);
 * ASSERT_POSTCONDITION(SDL_GetWindowWMInfo(sdlWindow, &wmi), "SDL version unsupported!");
 * HDC nativeWindow = (HDC) wmi.info.win.hdc;
 *
 * using namespace filament;
 * Engine* engine       = Engine::create();
 * SwapChain* swapChain = engine->createSwapChain((void*) nativeWindow);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * OSX
 * ---
 *
 * On OSX, any `NSView` can be used *directly* as a `nativeWindow` with createSwapChain().
 *
 * Example using SDL/Objective-C:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.mm}
 *  #include <filament/Engine.h>
 *
 *  #include <Cocoa/Cocoa.h>
 *  #include <SDL_syswm.h>
 *
 *  SDL_SysWMinfo wmi;
 *  SDL_VERSION(&wmi.version);
 *  NSWindow* win = (NSWindow*) wmi.info.cocoa.window;
 *  NSView* view = [win contentView];
 *  void* nativeWindow = view;
 *
 *  using namespace filament;
 *  Engine* engine       = Engine::create();
 *  SwapChain* swapChain = engine->createSwapChain(nativeWindow);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * @see Engine
 */
class UTILS_PUBLIC SwapChain : public FilamentAPI {
public:
    using FrameScheduledCallback = backend::FrameScheduledCallback;
    using FrameCompletedCallback = backend::FrameCompletedCallback;

    static const uint64_t CONFIG_TRANSPARENT = backend::SWAP_CHAIN_CONFIG_TRANSPARENT;
    /**
     * This flag indicates that the swap chain may be used as a source surface
     * for reading back render results.  This config must be set when creating
     * any swap chain that will be used as the source for a blit operation.
     *
     * @see
     * Renderer.copyFrame()
     */
    static const uint64_t CONFIG_READABLE = backend::SWAP_CHAIN_CONFIG_READABLE;

    /**
     * Indicates that the native X11 window is an XCB window rather than an XLIB window.
     * This is ignored on non-Linux platforms and in builds that support only one X11 API.
     */
    static const uint64_t CONFIG_ENABLE_XCB = backend::SWAP_CHAIN_CONFIG_ENABLE_XCB;

    /**
     * Indicates that the native window is a CVPixelBufferRef.
     *
     * This is only supported by the Metal backend. The CVPixelBuffer must be in the
     * kCVPixelFormatType_32BGRA format.
     *
     * It is not necessary to add an additional retain call before passing the pixel buffer to
     * Filament. Filament will call CVPixelBufferRetain during Engine::createSwapChain, and
     * CVPixelBufferRelease when the swap chain is destroyed.
     */
    static const uint64_t CONFIG_APPLE_CVPIXELBUFFER =
            backend::SWAP_CHAIN_CONFIG_APPLE_CVPIXELBUFFER;

    void* getNativeWindow() const noexcept;

    /**
     * FrameScheduledCallback is a callback function that notifies an application when Filament has
     * completed processing a frame and that frame is ready to be scheduled for presentation.
     *
     * Typically, Filament is responsible for scheduling the frame's presentation to the SwapChain.
     * If a SwapChain::FrameScheduledCallback is set, however, the application bares the
     * responsibility of scheduling a frame for presentation by calling the backend::PresentCallable
     * passed to the callback function. Currently this functionality is only supported by the Metal
     * backend.
     *
     * A FrameScheduledCallback can be set on an individual SwapChain through
     * SwapChain::setFrameScheduledCallback. If the callback is set, then the SwapChain will *not*
     * automatically schedule itself for presentation. Instead, the application must call the
     * PresentCallable passed to the FrameScheduledCallback.
     *
     * @param callback    A callback, or nullptr to unset.
     * @param user        An optional pointer to user data passed to the callback function.
     *
     * @remark Only Filament's Metal backend supports PresentCallables and frame callbacks. Other
     * backends ignore the callback (which will never be called) and proceed normally.
     *
     * @remark The SwapChain::FrameScheduledCallback is called on an arbitrary thread.
     *
     * @see PresentCallable
     */
    void setFrameScheduledCallback(FrameScheduledCallback callback, void* user = nullptr);

    /**
     * FrameCompletedCallback is a callback function that notifies an application when a frame's
     * contents have completed rendering on the GPU.
     *
     * Use SwapChain::setFrameCompletedCallback to set a callback on an individual SwapChain. Each
     * time a frame completes GPU rendering, the callback will be called with optional user data.
     *
     * The FrameCompletedCallback is guaranteed to be called on the main Filament thread.
     *
     * @param callback    A callback, or nullptr to unset.
     * @param user        An optional pointer to user data passed to the callback function.
     *
     * @remark Only Filament's Metal backend supports frame callbacks. Other backends ignore the
     * callback (which will never be called) and proceed normally.
     */
    void setFrameCompletedCallback(FrameCompletedCallback callback, void* user = nullptr);
};

} // namespace filament

#endif // TNT_FILAMENT_SWAPCHAIN_H
