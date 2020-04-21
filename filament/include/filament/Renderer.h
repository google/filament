/*
 * Copyright (C) 2015 The Android Open Source Project
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

//! \file

#ifndef TNT_FILAMENT_RENDERER_H
#define TNT_FILAMENT_RENDERER_H

#include <filament/FilamentAPI.h>

#include <utils/compiler.h>

#include <backend/PresentCallable.h>

#include <stdint.h>

namespace filament {

class Engine;
class RenderTarget;
class SwapChain;
class View;
class Viewport;

namespace backend {
class PixelBufferDescriptor;
} // namespace backend

/**
 * A Renderer instance represents an operating system's window.
 *
 * Typically, applications create a Renderer per window. The Renderer generates drawing commands
 * for the render thread and manages frame latency.
 *
 * A Renderer generates drawing commands from a View, itself containing a Scene description.
 *
 * Creation and Destruction
 * ========================
 *
 * A Renderer is created using Engine.createRenderer() and destroyed using
 * Engine.destroy(const Renderer*).
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * #include <filament/Renderer.h>
 * #include <filament/Engine.h>
 * using namespace filament;
 *
 * Engine* engine = Engine::create();
 *
 * Renderer* renderer = engine->createRenderer();
 * engine->destroy(&renderer);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * @see Engine, View
 */
class UTILS_PUBLIC Renderer : public FilamentAPI {
public:

    /**
     * Use DisplayInfo to set important Display properties. This is used to achieve correct
     * frame pacing and dynamic resolution scaling.
     */
    struct DisplayInfo {
        // refresh-rate of the display in Hz. set to 0 for offscreen or turn off frame-pacing.
        float refreshRate = 60.0f;

        // how far in advance a buffer must be queued for presentation at a given time in ns
        uint64_t presentationDeadlineNanos = 0;

        // offset by which vsyncSteadyClockTimeNano provided in beginFrame() is offset in ns
        uint64_t vsyncOffsetNanos = 0;
    };

    /**
     * Use FrameRateOptions to set the desired frame rate and control how quickly the system
     * reacts to GPU load changes.
     *
     * interval: desired frame interval in multiple of the refresh period, set in DisplayInfo
     *           (as 1 / DisplayInfo::refreshRate)
     *
     * The parameters below are relevant when some Views are using dynamic resolution scaling:
     *
     * headRoomRatio: additional headroom for the GPU as a ratio of the targetFrameTime.
     *                Useful for taking into account constant costs like post-processing or
     *                GPU drivers on different platforms.
     * history:   History size. higher values, tend to filter more (clamped to 30)
     * scaleRate: rate at which the gpu load is adjusted to reach the target frame rate
     *            This value can be computed as 1 / N, where N is the number of frames
     *            needed to reach 64% of the target scale factor.
     *            Higher values make the dynamic resolution react faster.
     *
     * @see View::DynamicResolutionOptions
     * @see Renderer::DisplayInfo
     *
     */
    struct FrameRateOptions {
        float headRoomRatio = 0.0f;    //!< additional headroom for the GPU
        float scaleRate = 0.125f;      //!< rate at which the system reacts to load changes
        uint8_t history = 3;           //!< history size
        uint8_t interval = 1;          //!< desired frame interval in unit of 1.0 / DisplayInfo::refreshRate
    };

    /**
     * ClearOptions are used at the beginning of a frame to clear or retain the SwapChain content.
     */
    struct ClearOptions {
        /** Color to use to clear the SwapChain */
        math::float4 clearColor = {};
        /**
         * Whether the SwapChain should be cleared using the clearColor. Use this if translucent
         * View will be drawn, for instance.
         */
        bool clear = false;
        /**
         * Whether the SwapChain content should be discarded. clear implies discard. Set this
         * to false (along with clear to false as well) if the SwapChain already has content that
         * needs to be preserved
         */
        bool discard = true;
    };

    /**
     * Information about the display this Renderer is associated to. This information is needed
     * to accurately compute dynamic-resolution scaling and for frame-pacing.
     *
     * @param info
     */
    void setDisplayInfo(const DisplayInfo& info) noexcept;

    /**
     * Set options controlling the desired frame-rate.
     *
     * @param options
     */
    void setFrameRateOptions(FrameRateOptions const& options) noexcept;

    /**
     * Set ClearOptions which are used at the beginning of a frame to clear or retain the
     * SwapChain content.
     *
     * @param options
     */
    void setClearOptions(const ClearOptions& options);

    /**
     * Get the Engine that created this Renderer.
     *
     * @return A pointer to the Engine instance this Renderer is associated to.
     */
    Engine* getEngine() noexcept;

    /**
     * Get the Engine that created this Renderer.
     *
     * @return A constant pointer to the Engine instance this Renderer is associated to.
     */
    inline Engine const* getEngine() const noexcept {
        return const_cast<Renderer *>(this)->getEngine();
    }

    /**
     * Render a View into this renderer's window.
     *
     * This is filament main rendering method, most of the CPU-side heavy lifting is performed
     * here. render() main function is to generate render commands which are asynchronously
     * executed by the Engine's render thread.
     *
     * render() generates commands for each of the following stages:
     *
     * 1. Shadow map pass, if needed (currently only a single shadow map is supported).
     * 2. Depth pre-pass.
     * 3. Color pass.
     * 4. Post-processing pass.
     *
     * A typical render loop looks like this:
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     * #include <filament/Renderer.h>
     * #include <filament/View.h>
     * using namespace filament;
     *
     * void renderLoop(Renderer* renderer, SwapChain* swapChain) {
     *     do {
     *         // typically we wait for VSYNC and user input events
     *         if (renderer->beginFrame(swapChain)) {
     *             renderer->render(mView);
     *             renderer->endFrame();
     *         }
     *     } while (!quit());
     * }
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     *
     *
     * @param view A pointer to the view to render.
     *
     * @attention
     * render() must be called *after* beginFrame() and *before* endFrame().
     *
     * @note
     * render() must be called from the Engine's main thread (or external synchronization
     * must be provided). In particular, calls to render() on different Renderer instances
     * **must** be synchronized.
     *
     * @remark
     * render() perform potentially heavy computations and cannot be multi-threaded. However,
     * internally, render() is highly multi-threaded to both improve performance in mitigate
     * the call's latency.
     *
     * @remark
     * render() is typically called once per frame (but not necessarily).
     *
     * @see
     * beginFrame(), endFrame(), View
     *
     */
    void render(View const* view);

    /**
     * Flags used to configure the behavior of copyFrame().
     *
     * @see
     * copyFrame()
     */
    using CopyFrameFlag = uint32_t;

    /**
     * Indicates that the dstSwapChain passed into copyFrame() should be
     * committed after the frame has been copied.
     *
     * @see
     * copyFrame()
     */
    static constexpr CopyFrameFlag COMMIT = 0x1;
    /**
     * Indicates that the presentation time should be set on the dstSwapChain
     * passed into copyFrame to the monotonic clock time when the frame is
     * copied.
     *
     * @see
     * copyFrame()
     */
    static constexpr CopyFrameFlag SET_PRESENTATION_TIME = 0x2;
    /**
     * Indicates that the dstSwapChain passed into copyFrame() should be
     * cleared to black before the frame is copied into the specified viewport.
     *
     * @see
     * copyFrame()
     */
    static constexpr CopyFrameFlag CLEAR = 0x4;

    /**
     * Copy the currently rendered view to the indicated swap chain, using the
     * indicated source and destination rectangle.
     *
     * @param dstSwapChain The swap chain into which the frame should be copied.
     * @param dstViewport The destination rectangle in which to draw the view.
     * @param srcViewport The source rectangle to be copied.
     * @param flags One or more CopyFrameFlag behavior configuration flags.
     *
     * @remark
     * copyFrame() should be called after a frame is rendered using render()
     * but before endFrame() is called.
     */
    void copyFrame(SwapChain* dstSwapChain, Viewport const& dstViewport,
            Viewport const& srcViewport, uint32_t flags = 0);

    /**
     * Reads back the content of the SwapChain associated with this Renderer.
     *
     * @param xoffset   Left offset of the sub-region to read back.
     * @param yoffset   Bottom offset of the sub-region to read back.
     * @param width     Width of the sub-region to read back.
     * @param height    Height of the sub-region to read back.
     * @param buffer    Client-side buffer where the read-back will be written.
     *
     *                  The following format are always supported:
     *                      - PixelBufferDescriptor::PixelDataFormat::RGBA
     *                      - PixelBufferDescriptor::PixelDataFormat::RGBA_INTEGER
     *
     *                  The following types are always supported:
     *                      - PixelBufferDescriptor::PixelDataType::UBYTE
     *                      - PixelBufferDescriptor::PixelDataType::UINT
     *                      - PixelBufferDescriptor::PixelDataType::INT
     *                      - PixelBufferDescriptor::PixelDataType::FLOAT
     *
     *                  Other combination of format/type may be supported. If a combination is
     *                  not supported, this operation may fail silently. Use a DEBUG build
     *                  to get some logs about the failure.
     *
     *
     *  Framebuffer as seen on         User buffer (PixelBufferDescriptor&)
     *  screen
     *  +--------------------+
     *  |                    |                .stride         .alignment
     *  |                    |         ----------------------->-->
     *  |                    |         O----------------------+--+   low addresses
     *  |                    |         |          |           |  |
     *  |             w      |         |          | .top      |  |
     *  |       <--------->  |         |          V           |  |
     *  |       +---------+  |         |     +---------+      |  |
     *  |       |     ^   |  | ======> |     |         |      |  |
     *  |   x   |    h|   |  |         |.left|         |      |  |
     *  +------>|     v   |  |         +---->|         |      |  |
     *  |       +.........+  |         |     +.........+      |  |
     *  |            ^       |         |                      |  |
     *  |          y |       |         +----------------------+--+  high addresses
     *  O------------+-------+
     *
     *
     * Typically readPixels() will be called after render() and before endFrame().
     *
     * After issuing this method, the callback associated with `buffer` will be invoked on the
     * main thread, indicating that the read-back has completed. Typically, this will happen
     * after multiple calls to beginFrame(), render(), endFrame().
     *
     * It is also possible to use a Fence to wait for the read-back.
     *
     * @remark
     * readPixels() is intended for debugging and testing. It will impact performance significantly.
     *
     */
    void readPixels(uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
            backend::PixelBufferDescriptor&& buffer);


    /**
     * Reads back the content of the provided RenderTarget.
     *
     * @param renderTarget  RenderTarget to read back from.
     * @param xoffset       Left offset of the sub-region to read back.
     * @param yoffset       Bottom offset of the sub-region to read back.
     * @param width         Width of the sub-region to read back.
     * @param height        Height of the sub-region to read back.
     * @param buffer        Client-side buffer where the read-back will be written.
     *
     *                  The following format are always supported:
     *                      - PixelBufferDescriptor::PixelDataFormat::RGBA
     *                      - PixelBufferDescriptor::PixelDataFormat::RGBA_INTEGER
     *
     *                  The following types are always supported:
     *                      - PixelBufferDescriptor::PixelDataType::UBYTE
     *                      - PixelBufferDescriptor::PixelDataType::UINT
     *                      - PixelBufferDescriptor::PixelDataType::INT
     *                      - PixelBufferDescriptor::PixelDataType::FLOAT
     *
     *                  Other combination of format/type may be supported. If a combination is
     *                  not supported, this operation may fail silently. Use a DEBUG build
     *                  to get some logs about the failure.
     *
     *
     *  Framebuffer as seen on         User buffer (PixelBufferDescriptor&)
     *  screen
     *  +--------------------+
     *  |                    |                .stride         .alignment
     *  |                    |         ----------------------->-->
     *  |                    |         O----------------------+--+   low addresses
     *  |                    |         |          |           |  |
     *  |             w      |         |          | .top      |  |
     *  |       <--------->  |         |          V           |  |
     *  |       +---------+  |         |     +---------+      |  |
     *  |       |     ^   |  | ======> |     |         |      |  |
     *  |   x   |    h|   |  |         |.left|         |      |  |
     *  +------>|     v   |  |         +---->|         |      |  |
     *  |       +.........+  |         |     +.........+      |  |
     *  |            ^       |         |                      |  |
     *  |          y |       |         +----------------------+--+  high addresses
     *  O------------+-------+
     *
     *
     * Typically readPixels() will be called after render() and before endFrame().
     *
     * After issuing this method, the callback associated with `buffer` will be invoked on the
     * main thread, indicating that the read-back has completed. Typically, this will happen
     * after multiple calls to beginFrame(), render(), endFrame().
     *
     * It is also possible to use a Fence to wait for the read-back.
     *
     * @remark
     * readPixels() is intended for debugging and testing. It will impact performance significantly.
     *
     */
    void readPixels(RenderTarget* renderTarget,
            uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
            backend::PixelBufferDescriptor&& buffer);

    /**
     * Set-up a frame for this Renderer.
     *
     * beginFrame() manages frame pacing, and returns whether or not a frame should be drawn. The
     * goal of this is to skip frames when the GPU falls behind in order to keep the frame
     * latency low.
     *
     * If a given frame takes too much time in the GPU, the CPU will get ahead of the GPU. The
     * display will draw the same frame twice producing a stutter. At this point, the CPU is
     * ahead of the GPU and depending on how many frames are buffered, latency increases.
     *
     * beginFrame() attempts to detect this situation and returns false in that case, indicating
     * to the caller to skip the current frame.
     *
     * When beginFrame() returns true, it is mandatory to render the frame and call endFrame().
     * However, when beginFrame() returns false, the caller has the choice to either skip the
     * frame and not call endFrame(), or proceed as though true was returned.
     *
     * Typically, Filament is responsible for scheduling the frame's presentation to the SwapChain.
     * If a backend::FrameFinishedCallback is provided, however, the application bares the
     * responsibility of scheduling a frame for presentation by calling the backend::PresentCallable
     * passed to the callback function. Currently this functionality is only supported by the Metal
     * backend.
     *
     * @param vsyncSteadyClockTimeNano The time in nanosecond of when the current frame started,
     *                                 or 0 if unknown. This value should be the timestamp of
     *                                 the last h/w vsync. It is expressed in the
     *                                 std::chrono::steady_clock time base.
     * @param swapChain A pointer to the SwapChain instance to use.
     * @param callback  A callback function that will be called when the backend has finished
     *                  processing the frame.
     * @param user      User data to be passed to the callback function.
     *
     * @return
     *      *false* the current frame should be skipped,
     *      *true* the current frame must be drawn and endFrame() must be called.
     *
     * @remark
     * When skipping a frame, the whole frame is canceled, and endFrame() must not be called.
     *
     * @note
     * All calls to render() must happen *after* beginFrame().
     *
     * @see
     * endFrame(), backend::PresentCallable, backend::FrameFinishedCallback
     */
    bool beginFrame(SwapChain* swapChain,
            uint64_t vsyncSteadyClockTimeNano = 0u,
            backend::FrameFinishedCallback callback = nullptr, void* user = nullptr);

    /**
     * Finishes the current frame and schedules it for display.
     *
     * endFrame() schedules the current frame to be displayed on the Renderer's window.
     *
     * @note
     * All calls to render() must happen *before* endFrame(). endFrame() must be called if
     * beginFrame() returned true, otherwise, endFrame() must not be called unless the caller
     * ignored beginFrame()'s return value.
     *
     * @see
     * beginFrame()
     */
    void endFrame();

    /**
     * Returns the time in second of the last call to beginFrame(). This value is constant for all
     * views rendered during a frame. The epoch is set with resetUserTime().
     *
     * In materials, this value can be queried using `vec4 getUserTime()`. The value returned
     * is a highp vec4 encoded as follows:
     *
     *      time.x = (float)Renderer.getUserTime();
     *      time.y = Renderer.getUserTime() - time.x;
     *
     * It follows that the following invariants are true:
     *
     *      (double)time.x + (double)time.y == Renderer.getUserTime()
     *      time.x == (float)Renderer.getUserTime()
     *
     * This encoding allows the shader code to perform high precision (i.e. double) time
     * calculations when needed despite the lack of double precision in the shader, for e.g.:
     *
     *      To compute (double)time * vertex in the material, use the following construct:
     *
     *              vec3 result = time.x * vertex + time.y * vertex;
     *
     *
     * Most of the time, high precision computations are not required, but be aware that the
     * precision of time.x rapidly diminishes as time passes:
     *
     *          time    | precision
     *          --------+----------
     *          16.7s   |    us
     *          4h39    |    ms
     *         77h      |   1/60s
     *
     *
     * In other words, it only possible to get microsecond accuracy for about 16s or millisecond
     * accuracy for just under 5h.
     *
     * This problem can be mitigated by calling resetUserTime(), or using high precision time as
     * described above.
     *
     * @return The time is seconds since resetUserTime() was last called.
     *
     * @see
     * resetUserTime()
     */
    double getUserTime() const;

    /**
     * Sets the user time epoch to now, i.e. resets the user time to zero.
     *
     * Use this method used to keep the precision of time high in materials, in practice it should
     * be called at least when the application is paused, e.g. Activity.onPause() in Android.
     *
     * @see
     * getUserTime()
     */
    void resetUserTime();
};

} // namespace filament

#endif // TNT_FILAMENT_RENDERER_H
