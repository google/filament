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

#include <backend/CallbackHandler.h>
#include <backend/DriverEnums.h>

#include <utils/compiler.h>
#include <utils/Invocable.h>

#include <stdint.h>

namespace filament {

class Engine;

/**
 * A swap chain represents an Operating System's *native* renderable surface.
 *
 * Typically, it's a native window or a view. Because a SwapChain is initialized from a
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
 * Platform        | nativeWindow type
 * :---------------|:----------------------------:
 * Android         | ANativeWindow*
 * macOS - OpenGL  | NSView*
 * macOS - Metal   | CAMetalLayer*
 * iOS - OpenGL    | CAEAGLLayer*
 * iOS - Metal     | CAMetalLayer*
 * X11             | Window
 * Windows         | HWND
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
 * FILAMENT_CHECK_POSTCONDITION(SDL_GetWindowWMInfo(sdlWindow, &wmi)) << "SDL version unsupported!";
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
    using FrameCompletedCallback = utils::Invocable<void(SwapChain* UTILS_NONNULL)>;

    /**
     * Requests a SwapChain with an alpha channel.
     */
    static constexpr uint64_t CONFIG_TRANSPARENT = backend::SWAP_CHAIN_CONFIG_TRANSPARENT;

    /**
     * This flag indicates that the swap chain may be used as a source surface
     * for reading back render results.  This config must be set when creating
     * any swap chain that will be used as the source for a blit operation.
     *
     * @see
     * Renderer.copyFrame()
     */
    static constexpr uint64_t CONFIG_READABLE = backend::SWAP_CHAIN_CONFIG_READABLE;

    /**
     * Indicates that the native X11 window is an XCB window rather than an XLIB window.
     * This is ignored on non-Linux platforms and in builds that support only one X11 API.
     */
    static constexpr uint64_t CONFIG_ENABLE_XCB = backend::SWAP_CHAIN_CONFIG_ENABLE_XCB;

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
    static constexpr uint64_t CONFIG_APPLE_CVPIXELBUFFER =
            backend::SWAP_CHAIN_CONFIG_APPLE_CVPIXELBUFFER;

    /**
     * Indicates that the SwapChain must automatically perform linear to sRGB encoding.
     *
     * This flag is ignored if isSRGBSwapChainSupported() is false.
     *
     * When using this flag, the output colorspace in ColorGrading should be set to
     * Rec709-Linear-D65, or post-processing should be disabled.
     *
     * @see isSRGBSwapChainSupported()
     * @see ColorGrading.outputColorSpace()
     * @see View.setPostProcessingEnabled()
     */
    static constexpr uint64_t CONFIG_SRGB_COLORSPACE = backend::SWAP_CHAIN_CONFIG_SRGB_COLORSPACE;

    /**
     * Indicates that this SwapChain should allocate a stencil buffer in addition to a depth buffer.
     *
     * This flag is necessary when using View::setStencilBufferEnabled and rendering directly into
     * the SwapChain (when post-processing is disabled).
     *
     * The specific format of the stencil buffer depends on platform support. The following pixel
     * formats are tried, in order of preference:
     *
     * Depth only (without CONFIG_HAS_STENCIL_BUFFER):
     * - DEPTH32F
     * - DEPTH24
     *
     * Depth + stencil (with CONFIG_HAS_STENCIL_BUFFER):
     * - DEPTH32F_STENCIL8
     * - DEPTH24F_STENCIL8
     *
     * Note that enabling the stencil buffer may hinder depth precision and should only be used if
     * necessary.
     *
     * @see View.setStencilBufferEnabled
     * @see View.setPostProcessingEnabled
     */
    static constexpr uint64_t CONFIG_HAS_STENCIL_BUFFER = backend::SWAP_CHAIN_CONFIG_HAS_STENCIL_BUFFER;

    /**
     * The SwapChain contains protected content. Only supported when isProtectedContentSupported()
     * is true.
     */
    static constexpr uint64_t CONFIG_PROTECTED_CONTENT = backend::SWAP_CHAIN_CONFIG_PROTECTED_CONTENT;

    /**
     * Indicates that the SwapChain is configured to use Multi-Sample Anti-Aliasing (MSAA) with the
     * given sample points within each pixel. Only supported when isMSAASwapChainSupported(4) is
     * true.
     *
     * This is supported by EGL(Android) and Metal. Other GL platforms (GLX, WGL, etc) don't support
     * it because the swapchain MSAA settings must be configured before window creation.
     *
     * With Metal, this flag should only be used when rendering a single View into a SwapChain. This
     * flag is not supported when rendering multiple Filament Views into this SwapChain.
     *
     * @see isMSAASwapChainSupported(4)
     */
    static constexpr uint64_t CONFIG_MSAA_4_SAMPLES = backend::SWAP_CHAIN_CONFIG_MSAA_4_SAMPLES;

    /**
     * Return whether createSwapChain supports the CONFIG_PROTECTED_CONTENT flag.
     * The default implementation returns false.
     *
     * @param engine A pointer to the filament Engine
     * @return true if CONFIG_PROTECTED_CONTENT is supported, false otherwise.
     */
    static bool isProtectedContentSupported(Engine& engine) noexcept;

    /**
     * Return whether createSwapChain supports the CONFIG_SRGB_COLORSPACE flag.
     * The default implementation returns false.
     *
     * @param engine A pointer to the filament Engine
     * @return true if CONFIG_SRGB_COLORSPACE is supported, false otherwise.
     */
    static bool isSRGBSwapChainSupported(Engine& engine) noexcept;

    /**
     * Return whether createSwapChain supports the CONFIG_MSAA_*_SAMPLES flag.
     * The default implementation returns false.
     *
     * @param engine A pointer to the filament Engine
     * @param samples The number of samples
     * @return true if CONFIG_MSAA_*_SAMPLES is supported, false otherwise.
     */
    static bool isMSAASwapChainSupported(Engine& engine, uint32_t samples) noexcept;

    void* UTILS_NULLABLE getNativeWindow() const noexcept;

    /**
     * If this flag is passed to setFrameScheduledCallback, then the behavior of the default
     * CallbackHandler (when nullptr is passed as the handler argument) is altered to call the
     * callback on the Metal completion handler thread (as opposed to the main Filament thread).
     * This flag also instructs the Metal backend to release the associated CAMetalDrawable on the
     * completion handler thread.
     *
     * This flag has no effect if a custom CallbackHandler is passed or on backends other than Metal.
     *
     * @see setFrameScheduledCallback
     */
    static constexpr uint64_t CALLBACK_DEFAULT_USE_METAL_COMPLETION_HANDLER = 1;

    /**
     * FrameScheduledCallback is a callback function that notifies an application about the status
     * of a frame after Filament has finished its processing.
     *
     * The exact timing and semantics of this callback differ depending on the graphics backend in
     * use.
     *
     * Metal Backend
     * =============
     *
     * With the Metal backend, this callback signifies that Filament has completed all CPU-side
     * processing for a frame and the frame is ready to be scheduled for presentation.
     *
     * Typically, Filament is responsible for scheduling the frame's presentation to the SwapChain.
     * If a SwapChain::FrameScheduledCallback is set, however, the application bears the
     * responsibility of scheduling the frame for presentation by calling the
     * backend::PresentCallable passed to the callback function. In this mode, Filament will *not*
     * automatically schedule the frame for presentation.
     *
     * When using the Metal backend, if your application delays the call to the PresentCallable
     * (e.g., by invoking it on a separate thread), you must ensure all PresentCallables have been
     * called before shutting down the Filament Engine. You can guarantee this by calling
     * Engine::flushAndWait() before Engine::shutdown(). This is necessary to ensure the Engine has
     * a chance to clean up all memory related to frame presentation.
     *
     * Other Backends (OpenGL, Vulkan, WebGPU)
     * =======================================
     *
     * On other backends, this callback serves as a notification that Filament has completed all
     * CPU-side processing for a frame. Filament proceeds with its normal presentation logic
     * automatically, and the PresentCallable passed to the callback is a no-op that can be safely
     * ignored.
     *
     * General Behavior
     * ================
     *
     * A FrameScheduledCallback can be set on an individual SwapChain through
     * SwapChain::setFrameScheduledCallback. Each SwapChain can have only one callback set per
     * frame. If setFrameScheduledCallback is called multiple times on the same SwapChain before
     * Renderer::endFrame(), the most recent call effectively overwrites any previously set
     * callback.
     *
     * The callback set by setFrameScheduledCallback is "latched" when Renderer::endFrame() is
     * executed. At this point, the callback is fixed for the frame that was just encoded.
     * Subsequent calls to setFrameScheduledCallback after endFrame() will apply to the next frame.
     *
     * Use \c setFrameScheduledCallback() (with default arguments) to unset the callback.
     *
     * @param handler    Handler to dispatch the callback or nullptr for the default handler.
     * @param callback   Callback to be invoked when the frame processing is complete.
     * @param flags      See CALLBACK_DEFAULT_USE_METAL_COMPLETION_HANDLER
     *
     * @see CallbackHandler
     * @see PresentCallable
     */
    void setFrameScheduledCallback(backend::CallbackHandler* UTILS_NULLABLE handler = nullptr,
            FrameScheduledCallback&& callback = {}, uint64_t flags = 0);

    /**
     * Returns whether this SwapChain currently has a FrameScheduledCallback set.
     *
     * @return true, if the last call to setFrameScheduledCallback set a callback
     *
     * @see SwapChain::setFrameCompletedCallback
     */
    bool isFrameScheduledCallbackSet() const noexcept;

    /**
     * FrameCompletedCallback is a callback function that notifies an application when a frame's
     * contents have completed rendering on the GPU.
     *
     * Use SwapChain::setFrameCompletedCallback to set a callback on an individual SwapChain. Each
     * time a frame completes GPU rendering, the callback will be called.
     *
     * If handler is nullptr, the callback is guaranteed to be called on the main Filament thread.
     *
     * Use \c setFrameCompletedCallback() (with default arguments) to unset the callback.
     *
     * @param handler     Handler to dispatch the callback or nullptr for the default handler.
     * @param callback    Callback called when each frame completes.
     *
     * @remark Only Filament's Metal backend supports frame callbacks. Other backends ignore the
     * callback (which will never be called) and proceed normally.
     *
     * @see CallbackHandler
     */
    void setFrameCompletedCallback(backend::CallbackHandler* UTILS_NULLABLE handler = nullptr,
            FrameCompletedCallback&& callback = {}) noexcept;


protected:
    // prevent heap allocation
    ~SwapChain() = default;
};

} // namespace filament

#endif // TNT_FILAMENT_SWAPCHAIN_H
