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
#include <utils/FixedCapacityVector.h>

#include <math/vec4.h>

#include <chrono>

#include <stddef.h>
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

        UTILS_DEPRECATED uint64_t presentationDeadlineNanos = 0;
        UTILS_DEPRECATED uint64_t vsyncOffsetNanos = 0;
    };

    /**
     * Timing information about a frame
     * @see getFrameInfoHistory()
     */
    struct FrameInfo {
        /** duration in nanosecond since epoch of std::steady_clock */
        using time_point_ns = int64_t;
        /** duration in nanosecond on the std::steady_clock */
        using duration_ns = int64_t;
        static constexpr time_point_ns INVALID = -1;    //!< value not supported
        static constexpr time_point_ns PENDING = -2;    //!< value not yet available
        uint32_t frameId;                   //!< monotonically increasing frame identifier
        duration_ns gpuFrameDuration;       //!< frame duration on the GPU in nanosecond [ns]
        duration_ns denoisedGpuFrameDuration; //!< denoised frame duration on the GPU in [ns]
        time_point_ns beginFrame;           //!< Renderer::beginFrame() time since epoch [ns]
        time_point_ns endFrame;             //!< Renderer::endFrame() time since epoch [ns]
        time_point_ns backendBeginFrame;    //!< Backend thread time of frame start since epoch [ns]
        time_point_ns backendEndFrame;      //!< Backend thread time of frame end since epoch [ns]
        time_point_ns gpuFrameComplete;     //!< GPU thread time of frame end since epoch [ns] or 0
        time_point_ns vsync;                //!< VSYNC time of this frame since epoch [ns]
        time_point_ns displayPresent;       //!< Actual presentation time of this frame since epoch [ns]
        time_point_ns presentDeadline;      //!< deadline for queuing a frame [ns]
        duration_ns displayPresentInterval; //!< display refresh rate [ns]
        duration_ns compositionToPresentLatency; //!< time between the start of composition and the expected present time [ns]
        duration_ns expectedPresentLatency; //!< time between vsync and the system's expected presentation time [ns]
    };

    /**
     * Retrieve a history of frame timing information. The maximum frame history size is
     * given by getMaxFrameHistorySize().
     * All or part of the history can be lost when using a different SwapChain in beginFrame().
     * @param historySize requested history size. The returned vector could be smaller.
     * @return A vector of FrameInfo.
     * @see beginFrame()
     */
    utils::FixedCapacityVector<FrameInfo> getFrameInfoHistory(
            size_t historySize = 1) const noexcept;

    /**
     * @return the maximum supported frame history size.
     * @see getFrameInfoHistory()
     */
    size_t getMaxFrameHistorySize() const noexcept;

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
     * history:   History size. higher values, tend to filter more (clamped to 31)
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
        float headRoomRatio = 0.0f;        //!< additional headroom for the GPU
        float scaleRate = 1.0f / 8.0f;     //!< rate at which the system reacts to load changes
        uint8_t history = 15;              //!< history size
        uint8_t interval = 1;              //!< desired frame interval in unit of 1.0 / DisplayInfo::refreshRate
    };

    /**
     * ClearOptions are used at the beginning of a frame to clear or retain the SwapChain content.
     */
    struct ClearOptions {
        /**
         * Color to use to clear the RenderTarget (typically the SwapChain).
         *
         * The RenderTarget is cleared using this color, which won't be tone-mapped since
         * tone-mapping is part of View rendering (this is not).
         *
         * The value is stored as four doubles. The backend converts them as-is into the
         * matching native clear entry-point based on the attachment's format -- so the
         * caller is responsible for putting a value here that makes sense for the
         * attachment family (e.g. for a UINT attachment, a value in [0, UINT32_MAX]).
         * int32_t / uint32_t values round-trip exactly because double has a 53-bit mantissa.
         *
         * When a View is rendered, there are 3 scenarios to consider:
         * - Pixels rendered by the View replace the clear color (or blend with it in
         *   `BlendMode::TRANSLUCENT` mode).
         *
         * - With blending mode set to `BlendMode::TRANSLUCENT`, Pixels untouched by the View
         *   are considered fulling transparent and let the clear color show through.
         *
         * - With blending mode set to `BlendMode::OPAQUE`, Pixels untouched by the View
         *   are set to the clear color. However, because it is now used in the context of a View,
         *   it will go through the post-processing stage, which includes tone-mapping.
         *
         * For consistency, it is recommended to always use a Skybox to clear an opaque View's
         * background, or to use black or fully-transparent (i.e. {0,0,0,0}) as the clear color.
         */
        math::double4 clearColor = {};

        /** Value to clear the stencil buffer */
        uint8_t clearStencil = 0u;

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
     */
    void setDisplayInfo(const DisplayInfo& info) noexcept;

    /**
     * Set options controlling the desired frame-rate.
     */
    void setFrameRateOptions(FrameRateOptions const& options) noexcept;

    /**
     * Set ClearOptions which are used at the beginning of a frame to clear or retain the
     * SwapChain content.
     */
    void setClearOptions(const ClearOptions& options);

    /**
     * Returns the ClearOptions currently set.
     * @return A reference to a ClearOptions structure.
     */
    ClearOptions const& getClearOptions() const noexcept;

    /**
     * Get the Engine that created this Renderer.
     *
     * @return A pointer to the Engine instance this Renderer is associated to.
     */
    Engine* UTILS_NONNULL getEngine() noexcept;

    /**
     * Get the Engine that created this Renderer.
     *
     * @return A constant pointer to the Engine instance this Renderer is associated to.
     */
    Engine const* UTILS_NONNULL getEngine() const noexcept {
        return const_cast<Renderer *>(this)->getEngine();
    }

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
     * The use of this method is optional. It sets the VSYNC time expressed as the duration in
     * nanosecond since epoch of std::chrono::steady_clock.
     * If called, passing 0 to vsyncSteadyClockTimeNano in Renderer::BeginFrame will use this
     * time instead.
     * @param steadyClockTimeNano duration in nanosecond since epoch of std::chrono::steady_clock
     * @see Engine::getSteadyClockTimeNano()
     * @see Renderer::BeginFrame()
     */
    void setVsyncTime(uint64_t steadyClockTimeNano) noexcept;

    /**
     * Call skipFrame when momentarily skipping frames, for instance if the content of the
     * scene doesn't change.
     *
     * @param vsyncSteadyClockTimeNano
     */
    void skipFrame(uint64_t vsyncSteadyClockTimeNano = 0u);

    /**
     * Returns true if the current frame should be rendered.
     *
     * This is a convenience method that returns the same value as beginFrame().
     *
     * @return
     *      *false* the current frame should be skipped, or an unrecoverable backend exception has occurred.
     *      *true* the current frame can be rendered
     *
     * @note This method will return false once a backend exception has been delivered to the main thread.
     *
     * @see
     * beginFrame()
     */
    bool shouldRenderFrame() const noexcept;

    /**
     * Set up a frame for this Renderer.
     *
     * beginFrame() manages frame-pacing, and returns whether a frame should be drawn. The
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
     * @param vsyncSteadyClockTimeNano The time in nanosecond of when the current frame started,
     *                                 or 0 if unknown. This value should be the timestamp of
     *                                 the last h/w vsync. It is expressed in the
     *                                 std::chrono::steady_clock time base.
     *                                 On Android this should be the frame time received from
     *                                 a Choreographer.
     * @param swapChain A pointer to the SwapChain instance to use.
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
     * It is recommended to use the same swapChain for every call to beginFrame, failing to do
     * so can result is losing all or part of the FrameInfo history.
     *
     * @throws std::exception (or derived) if the backend thread encountered an unrecoverable error (when exceptions are enabled).
     *
     * @note This method will return false if called again after a backend exception was already thrown and delivered to the main thread.
     *
     * @see
     * endFrame()
     */
    bool beginFrame(SwapChain* UTILS_NONNULL swapChain,
            uint64_t vsyncSteadyClockTimeNano = 0u);

    /**
     * Set the time at which the frame must be presented to the display hardware.
     *
     * This value is used to configure the hardware and must typically be strictly smaller than the
     * desired presentation time (i.e. it must include some headroom but not too much). For instance,
     * on Android, it is typically set to desired_presentation_time - vsync_period / 2. This behavior
     * can vary on other platforms.
     *
     * This must be called before endFrame().
     *
     * @param monotonic_clock_ns  the presentation configuration timestamp in nanoseconds on the steady clock.
     */
    void setPresentationTime(int64_t monotonic_clock_ns);

    /**
     * Set the time at which the frame must be presented to the display hardware using a strong steady clock time point.
     *
     * This value is used to configure the hardware and must typically be strictly smaller than the
     * desired presentation time (i.e. it must include some headroom but not too much). For instance,
     * on Android, it is typically set to desired_presentation_time - vsync_period / 2. This behavior
     * can vary on other platforms.
     *
     * This must be called before endFrame().
     *
     * @param monotonic_clock  the presentation configuration time point on the steady clock.
     */
    void setPresentationTime(std::chrono::steady_clock::time_point monotonic_clock);

    /**
     * Set the real desired presentation time targeted for this frame.
     *
     * Unlike setPresentationTime(), which configures hardware headroom, this is the exact target
     * presentation time and is used for FrameInfo frame history reporting.
     *
     * This must be called before endFrame().
     *
     * @param monotonic_clock_ns  the desired presentation timestamp in nanoseconds on the steady clock.
     */
    void setDesiredPresentationTime(int64_t monotonic_clock_ns);

    /**
     * Set the real desired presentation time targeted for this frame.
     *
     * Unlike setPresentationTime(), which configures hardware headroom, this is the exact target
     * presentation time and is used for FrameInfo frame history reporting.
     *
     * This must be called before endFrame().
     *
     * @param monotonic_clock  the desired presentation time point on the steady clock.
     */
    void setDesiredPresentationTime(std::chrono::steady_clock::time_point monotonic_clock);

    /**
     * Set the deadline time point on the steady clock by which CPU and GPU rendering must complete
     * for the buffer to meet its target display latching window.
     *
     * This must be called before endFrame().
     *
     * @param monotonic_clock_ns  the deadline timestamp in nanoseconds on the steady clock.
     */
    void setRenderingDeadline(int64_t monotonic_clock_ns);

    /**
     * Set the deadline time point on the steady clock by which CPU and GPU rendering must complete
     * for the buffer to meet its target display latching window.
     *
     * This must be called before endFrame().
     *
     * @param monotonic_clock  the deadline time point on the steady clock.
     */
    void setRenderingDeadline(std::chrono::steady_clock::time_point monotonic_clock);

    /**
     * Render a View into this renderer's window.
     *
     * This is filament main rendering method, most of the CPU-side heavy lifting is performed
     * here. render() main function is to generate render commands which are asynchronously
     * executed by the Engine's render thread.
     *
     * render() generates commands for each of the following stages:
     *
     * 1. Shadow map passes, if needed.
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
     * @throws std::exception (or derived) if the backend thread encountered an unrecoverable error (when exceptions are enabled).
     * @throws utils::Panic if called again after a backend exception was already thrown.
     *
     * @see
     * beginFrame(), endFrame(), View
     *
     */
    void render(View const* UTILS_NONNULL view);

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
    void copyFrame(SwapChain* UTILS_NONNULL dstSwapChain, Viewport const& dstViewport,
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
     * The following formats are always supported:
     *                      - PixelBufferDescriptor::PixelDataFormat::RGBA
     *                      - PixelBufferDescriptor::PixelDataFormat::RGBA_INTEGER
     *
     * The following types are always supported:
     *                      - PixelBufferDescriptor::PixelDataType::UBYTE
     *                      - PixelBufferDescriptor::PixelDataType::UINT
     *                      - PixelBufferDescriptor::PixelDataType::INT
     *                      - PixelBufferDescriptor::PixelDataType::FLOAT
     *
     * Other combinations of format/type may be supported. If a combination is
     * not supported, this operation may fail silently. Use a DEBUG build
     * to get some logs about the failure.
     *
     *
     *  Framebuffer as seen on User buffer (PixelBufferDescriptor&)
     *  screen
     *
     *      +--------------------+
     *      |                    |                .stride         .alignment
     *      |                    |         ----------------------->-->
     *      |                    |         O----------------------+--+   low addresses
     *      |                    |         |          |           |  |
     *      |             w      |         |          | .top      |  |
     *      |       <--------->  |         |          V           |  |
     *      |       +---------+  |         |     +---------+      |  |
     *      |       |     ^   |  | ======> |     |         |      |  |
     *      |   x   |    h|   |  |         |.left|         |      |  |
     *      +------>|     v   |  |         +---->|         |      |  |
     *      |       +.........+  |         |     +.........+      |  |
     *      |            ^       |         |                      |  |
     *      |          y |       |         +----------------------+--+  high addresses
     *      O------------+-------+
     *
     *
     * readPixels() must be called within a frame, meaning after beginFrame() and before endFrame().
     * Typically, readPixels() will be called after render().
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
     * Finishes the current frame and schedules it for display.
     *
     * endFrame() schedules the current frame to be displayed on the Renderer's window.
     *
     * @note
     * All calls to render() must happen *before* endFrame(). endFrame() must be called if
     * beginFrame() returned true, otherwise, endFrame() must not be called unless the caller
     * ignored beginFrame()'s return value.
     *
     * @throws std::exception (or derived) if the backend thread encountered an unrecoverable error (when exceptions are enabled).
     * @throws utils::Panic if called again after a backend exception was already thrown.
     *
     * @see
     * beginFrame()
     */
    void endFrame();


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
     * The following formats are always supported:
     *                      - PixelBufferDescriptor::PixelDataFormat::RGBA
     *                      - PixelBufferDescriptor::PixelDataFormat::RGBA_INTEGER
     *
     * The following types are always supported:
     *                      - PixelBufferDescriptor::PixelDataType::UBYTE
     *                      - PixelBufferDescriptor::PixelDataType::UINT
     *                      - PixelBufferDescriptor::PixelDataType::INT
     *                      - PixelBufferDescriptor::PixelDataType::FLOAT
     *
     * Other combinations of format/type may be supported. If a combination is
     * not supported, this operation may fail silently. Use a DEBUG build
     * to get some logs about the failure.
     *
     *
     *  Framebuffer as seen on User buffer (PixelBufferDescriptor&)
     *  screen
     *
     *      +--------------------+
     *      |                    |                .stride         .alignment
     *      |                    |         ----------------------->-->
     *      |                    |         O----------------------+--+   low addresses
     *      |                    |         |          |           |  |
     *      |             w      |         |          | .top      |  |
     *      |       <--------->  |         |          V           |  |
     *      |       +---------+  |         |     +---------+      |  |
     *      |       |     ^   |  | ======> |     |         |      |  |
     *      |   x   |    h|   |  |         |.left|         |      |  |
     *      +------>|     v   |  |         +---->|         |      |  |
     *      |       +.........+  |         |     +.........+      |  |
     *      |            ^       |         |                      |  |
     *      |          y |       |         +----------------------+--+  high addresses
     *      O------------+-------+
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
     * OpenGL only: if issuing a readPixels on a RenderTarget backed by a Texture that had data
     * uploaded to it via setImage, the data returned from readPixels will be y-flipped with respect
     * to the setImage call.
     *
     * Note: the texture that backs the COLOR attachment for `renderTarget` must have
     * TextureUsage::BLIT_SRC as part of its usage.
     *
     * @remark
     * readPixels() is intended for debugging and testing. It will impact performance significantly.
     *
     */
    void readPixels(RenderTarget* UTILS_NONNULL renderTarget,
            uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
            backend::PixelBufferDescriptor&& buffer);

    /**
     * Render a standalone View into its associated RenderTarget
     *
     * This call is mostly equivalent to calling render(View*) inside a
     * beginFrame / endFrame block, but incurs less overhead. It can be used
     * as a poor man's compute API.
     *
     * @param view A pointer to the view to render. This View must have a RenderTarget associated
     *             to it.
     *
     * @attention
     * renderStandaloneView() must be called outside of beginFrame() / endFrame().
     *
     * @note
     * renderStandaloneView() must be called from the Engine's main thread
     * (or external synchronization must be provided). In particular, calls to
     * renderStandaloneView() on different Renderer instances **must** be synchronized.
     *
     * @remark
     * renderStandaloneView() perform potentially heavy computations and cannot be multi-threaded.
     * However, internally, renderStandaloneView() is highly multi-threaded to both improve
     * performance in mitigate the call's latency.
     */
    void renderStandaloneView(View const* UTILS_NONNULL view);


    /**
     * Returns the material time in seconds evaluated for the current frame. This value is constant for all
     * views rendered during a frame. When available, this time is projected forward to the predicted
     * presentation time on the display; otherwise, it evaluates at the vsync time of beginFrame().
     * The epoch is set with setMaterialTimeEpoch().
     *
     * In materials, this value can be queried using `vec4 getUserTime()`. The value returned
     * is a highp vec4 encoded as follows:
     *
     *      time.x = (float)Renderer.getMaterialTime();
     *      time.y = Renderer.getMaterialTime() - time.x;
     *
     * It follows that the following invariants are true:
     *
     *      (double)time.x + (double)time.y == Renderer.getMaterialTime()
     *      time.x == (float)Renderer.getMaterialTime()
     *
     * This "float-float" encoding allows the shader code to perform high precision (i.e. double) time
     * calculations when needed despite the lack of double precision in the shader (e.g. using Dekker's
     * algorithms). For example, to compute (double)time * vertex in the material, use the following construct:
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
     * In other words, it is only possible to get microsecond accuracy for about 16s or millisecond
     * accuracy for just under 5h.
     *
     * This problem can be mitigated by calling setMaterialTimeEpoch(), or using high precision time as
     * described above.
     *
     * @return The time in seconds since setMaterialTimeEpoch() was last called.
     *
     * @see
     * setMaterialTimeEpoch()
     */
    double getMaterialTime() const;

    /**
     * Backward compatibility helper for getUserTime().
     * @deprecated Use getMaterialTime() instead.
     */
    inline double getUserTime() const {
        return getMaterialTime();
    }

    /**
     * Sets the material time epoch to the specified steady clock timestamp in nanoseconds, i.e. resets
     * the material time to zero relative to that time.
     *
     * Use this method to keep the precision of time high in materials, in practice it should
     * be called at least when the application is paused, e.g. Activity.onPause() in Android.
     *
     * @param monotonic_clock_ns  the steady clock timestamp in nanoseconds to set as the material time epoch.
     *
     * @see
     * getMaterialTime()
     */
    void setMaterialTimeEpoch(int64_t monotonic_clock_ns);

    /**
     * Sets the material time epoch to the specified steady clock time point, i.e. resets
     * the material time to zero relative to that time point.
     *
     * Use this method to keep the precision of time high in materials, in practice it should
     * be called at least when the application is paused, e.g. Activity.onPause() in Android.
     *
     * @param monotonic_clock  the time point on the steady clock to set as the material time epoch.
     *
     * @see
     * getMaterialTime()
     */
    void setMaterialTimeEpoch(std::chrono::steady_clock::time_point monotonic_clock);

    /**
     * Backward compatibility helper for resetUserTime().
     * @deprecated Use setMaterialTimeEpoch() instead.
     */
    inline void resetUserTime() {
        setMaterialTimeEpoch(std::chrono::steady_clock::now());
    }


    /**
     * Requests the next frameCount frames to be skipped. For Debugging.
     * @param frameCount number of frames to skip.
     */
    void skipNextFrames(size_t frameCount) noexcept;

    /**
     * Remainder count of frame to be skipped
     * @return remaining frames to be skipped
     */
    size_t getFrameToSkipCount() const noexcept;

    /**
     * Queries whether the GPU execution has fallen behind the CPU rendering execution.
     *
     * This is highly useful when managing the application's presentation loop manually (e.g.
     * with the `FramePacer`), allowing the client to proactively detect and react to a latency build-up
     * before continuing with frame execution.
     *
     * @return true if the GPU pipeline is delayed, false if ready.
     */
    bool hasGpuFallenBehind() const noexcept;

    /**
     * Stalls the render thread (GPU submission pipeline) for the given duration in nanoseconds.
     *
     * This is useful for simulating long rendering frames (e.g. testing buffer stuffing recovery)
     * without blocking the application's main event loop thread.
     *
     * @param duration_ns  the duration to pause the render thread in nanoseconds.
     */
    void pauseRenderThread(uint64_t duration_ns);

protected:
    // prevent heap allocation
    ~Renderer() = default;
};

} // namespace filament

#endif // TNT_FILAMENT_RENDERER_H
