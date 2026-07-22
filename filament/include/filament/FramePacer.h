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

#ifndef TNT_FILAMENT_FRAMEPACER_H
#define TNT_FILAMENT_FRAMEPACER_H

#include <filament/FilamentAPI.h>

#include <utils/Slice.h>

#include <chrono>
#include <vector>

#include <stddef.h>
#include <stdint.h>

#if defined(__ANDROID__)
struct AChoreographerFrameCallbackData;
#endif

namespace filament {

class Engine;
class Renderer;

/**
 * FramePacer
 *
 * Coordinates frame scheduling and presentation timestamps across multi-threaded rendering architectures.
 *
 * The FramePacer decouples the CPU rendering loop from fluctuating hardware display cadences, acting
 * as a deterministic filter between incoming platform VSYNC callbacks (such as AChoreographer on Android)
 * and native buffer presentation submissions (via Renderer::setPresentationTime).
 *
 * Creation and destruction
 * ========================
 *
 * A FramePacer object is created using the FramePacer::Builder and destroyed by calling
 * Engine::destroy(const FramePacer*).
 *
 * ~~~~~~~~~~~{.cpp}
 *  filament::Engine* engine = filament::Engine::create();
 *
 *  filament::FramePacer* pacer = filament::FramePacer::Builder()
 *              .targetFrameRate(60.0f)
 *              .latency(std::chrono::milliseconds(33)) // Configure 33ms target latency duration (~2 60Hz frames)
 *              .build(*engine);
 *
 *  // Inside your application's AChoreographer frame callback:
 *  void onChoreographerTick(const filament::FramePacer::VsyncTick& tick) {
 *      if (pacer->setupFrame(tick) != filament::FramePacer::FrameStatus::ACCEPTED) {
 *          // Yield or skip rendering to maintain pacing
 *          return;
 *      }
 *
 *      // Evaluate if the GPU is delayed, bypassing FrameSkipper check via Renderer::shouldRenderFrame()
 *      if (pacer->hasGpuFallenBehind(renderer)) {
 *          renderer->skipFrame(tick.baseTime);
 *          return;
 *      }
 *
 *      // Set presentation time which automatically switches Renderer to "paced mode"
 *      pacer->applyPresentationTime(renderer);
 *
 *      if (renderer->beginFrame(swapChain)) {
 *          renderer->render(view);
 *          renderer->endFrame();
 *      }
 *  }
 *
 *  engine->destroy(pacer);
 * ~~~~~~~~~~~
 */
class UTILS_PUBLIC FramePacer : public FilamentAPI {
public:
    using time_point_t = std::chrono::steady_clock::time_point;
    using duration_t   = std::chrono::nanoseconds;

    // Guarantee at compile-time that our time_point_t alias remains strictly tied 
    // to a monotonic, steady clock (e.g. CLOCK_MONOTONIC on Android) and is never changed to system_clock
    static_assert(time_point_t::clock::is_steady,
            "FramePacer::time_point_t requires a strictly monotonic, steady clock source to prevent NTP baseline shifting.");

    enum class FrameStatus : int8_t {
        SKIPPED_SPURIOUS = -2,  //!< Skipped to maintain target frame rate cadence (e.g. 30 FPS on 60Hz display).
        SKIPPED_STALE    = -1,  //!< Skipped to prevent out-of-order presentation (monotonic guard).
        ACCEPTED         = 0,   //!< The frame is approved for rendering.
    };

    enum class PacingStatus : int8_t {
        STEADY           = 0,   //!< Operating at or near the optimal configured latency.
        DISPLAY_STARVING = -1,  //!< Latency has shrunk. The display is starving for buffers.
        DISPLAY_STUFFED  = 1    //!< Latency has bloated. The display queue is stuffed.
    };

    /**
     * Telemetry for a single expected hardware presentation timeline.
     */
    struct HardwareTimeline {
        time_point_t expectedPresentationTime; //!< The anticipated physical buffer presentation timestamp.
        time_point_t deadline;                 //!< The CPU/GPU submission completion deadline to achieve this timeline.
    };

    using Timelines = utils::Slice<const HardwareTimeline>;

    /**
     * Encapsulates VSYNC synchronization telemetry received from the platform compositor.
     */
    struct VsyncTick {
        /**
         * The hardware base VSYNC tick timestamp.
         *
         * On Android, this can be retrieved using `AChoreographerFrameCallbackData_getFrameTimeNanos()`
         * or via the `frameTimeNanos` parameter in a classic `AChoreographer_vsyncCallback`.
         */
        time_point_t baseTime;

        /**
         * The physical hardware VSYNC period.
         *
         * On Android, this should typically be set to `1.0f / Display.getRefreshRate()`
         * (converted to nanoseconds) or queried via `AChoreographer_registerRefreshRateCallback()`.
         */
        duration_t vsyncPeriod = std::chrono::nanoseconds(16666666);

        /**
         * The physical clock time when the frame scheduling callback was entered.
         * Used to filter out display timelines whose submission deadlines have already passed.
         */
        time_point_t frameScheduleTime = std::chrono::steady_clock::now();

        /**
         * Slice representing the candidate hardware presentation timelines.
         *
         * Only populated on platforms supporting multiple timeline frames (such as Android API 33+).
         * The timelines in this slice are guaranteed to be sorted chronologically by their
         * expected presentation times (which is guaranteed by the Android platform).
         *
         * The lifetime of the underlying timeline elements in this Slice is guaranteed only for
         * the immediate duration of the `setupFrame()` call. Do not store or reference this slice
         * beyond that synchronous scope.
         */
        Timelines timelines;
    };

    /**
     * Holds dynamic pacing targets and latency pipeline depth requirements.
     */
    struct Configuration {
        float targetFrameRate = 60.0f; //!< The application's desired frame rendering step in Hz.
        std::chrono::nanoseconds latency = std::chrono::nanoseconds(33333333); //!< Target latency duration (defaults to 33.33ms, ~2 60Hz frames)
    };

    struct BuilderDetails;

    //! Use Builder to construct a FramePacer object instance
    class Builder : public BuilderBase<BuilderDetails> {
        friend struct BuilderDetails;
    public:
        Builder() noexcept;
        Builder(Builder const& rhs) noexcept;
        Builder(Builder&& rhs) noexcept;
        ~Builder() noexcept;
        Builder& operator=(Builder const& rhs) noexcept;
        Builder& operator=(Builder&& rhs) noexcept;

        /**
         * Sets the desired frame rendering step in Hz.
         *
         * @param fps Target frame rate (e.g., 60.0f or 30.0f). Must be greater than 0.
         * @return This Builder, for chaining calls.
         */
        Builder& targetFrameRate(float fps) noexcept;

        /**
         * Sets the required latency window in terms of time duration.
         *
         * The target latency is measured relative to the hardware VSYNC timestamp (VsyncTick::baseTime),
         * as opposed to the callback entry/dispatch time. Note that the hardware VSYNC time is
         * always equal to or in the past with respect to the actual callback dispatch time.
         *
         * @param latency Target latency duration (defaults to 33ms).
         * @return This Builder, for chaining calls.
         */
        Builder& latency(std::chrono::nanoseconds latency) noexcept;

        /**
         * Sets the required latency window in terms of 60Hz display frames.
         *
         * @param frames The latency window in units of 60Hz frames (e.g. 2 frames = 33.3ms).
         * @return This Builder, for chaining calls.
         */
        Builder& latencyFrames(uint32_t frames) noexcept;

        /**
         * Creates the FramePacer object and returns a pointer to it.
         *
         * @param engine Reference to the filament::Engine to associate this FramePacer with.
         * @return Pointer to the newly created FramePacer instance.
         */
        FramePacer* UTILS_NONNULL build(Engine& engine) const;
    
    private:
        friend class FFramePacer;
    };

    /**
     * Dynamically updates the active pacing targets mid-flight (e.g., for thermal or power mitigation).
     *
     * @param config The new configuration targets to scale to on subsequent frames.
     */
    void configure(const Configuration& config);

    /**
     * Retrieves the current configuration used by the FramePacer.
     *
     * @return The active configuration targets.
     */
    const Configuration& getConfiguration() const noexcept;

    /**
     * Prepares and evaluates the frame pacing state for the upcoming frame cycle.
     *
     * This must be called at the very beginning of the host display platform's VSYNC interrupt loop
     * (e.g., within a VSYNC callback such as AChoreographer on Android). It evaluates whether the CPU rendering
     * thread is ahead of the hardware pulse cadence or backlogged.
     *
     * @attention The memory backed by the `tick.timelines` slice is guaranteed to be valid only
     *            during the immediate, synchronous execution of this call. Client implementations
     *            must not store or access this slice or its underlying pointers once `setupFrame()`
     *            returns.
     *
     * @param tick Polled hardware VSYNC timing telemetry and optional target presentation timelines.
     * @return FrameStatus::ACCEPTED if approved, or the specific SKIPPED reason.
     */
    FrameStatus setupFrame(const VsyncTick& tick);

    /**
     * Advances the internal pacing pipeline to target an extra presentation frame in the future,
     * without advancing the ideal cadence clock (mExpectedBaseTime).
     *
     * This method is explicitly designed for latency recovery (Buffer Stuffing). If the pipeline
     * reports PacingStatus::DISPLAY_STARVING, the application may yield, OR it may choose to render
     * an extra frame during the current Vsync callback to mechanically stuff the queue depth back
     * to its target latency.
     *
     * Calling this method instantly extrapolates the presentation timestamp one target frame period
     * into the future. The subsequent call to `applyPresentationTime` will emit this new timestamp,
     * allowing the queue to grow without the FramePacer erroneously rejecting the next real Vsync
     * as spurious.
     *
     * This method acts as a safeguard against runaway clock extrapolation. It returns true if
     * the timestamp was successfully advanced, or false if it refused to stuff another frame
     * because the pipeline is already at or beyond the configured target latency.
     *
     * \code
     * if (pacer->setupFrame(tick) == FramePacer::FrameStatus::ACCEPTED) {
     *     pacer->applyPresentationTime(renderer);
     *     if (renderer->beginFrame(swapChain)) {
     *         renderer->render(view);
     *         renderer->endFrame();
     *     }
     *
     *     // Automatically recover queue depth by stuffing an extra frame
     *     // If the application was starved by multiple frames, it will receive DISPLAY_STARVING
     *     // over consecutive Vsync callbacks, naturally amortizing the recovery over time.
     *     if (pacer->getPacingStatus() == FramePacer::PacingStatus::DISPLAY_STARVING) {
     *         if (pacer->setupExtraFrame()) {
     *             pacer->applyPresentationTime(renderer); // Emit the future-shifted timestamp
     *             if (renderer->beginFrame(swapChain)) {
     *                 // Optional: advance simulation state again here
     *                 renderer->render(view);
     *                 renderer->endFrame();
     *             }
     *         }
     *     }
     * }
     * \endcode
     *
     * @return true if the timestamp was safely advanced, false if refused to prevent over-stuffing.
     */
    bool setupExtraFrame() noexcept;

    /**
     * Checks if the GPU rendering pipeline has fallen behind the CPU submission rate.
     *
     * If the GPU is delayed, this method returns true and automatically rolls back any internal
     * cadence state advanced by `setupFrame()`. The client should skip the frame (via
     * `Renderer::skipFrame()`) and refrain from calling `applyPresentationTime()` or `beginFrame()`.
     *
     * @param renderer The Filament Renderer displaying the target View.
     * @return true if the GPU has fallen behind, false otherwise.
     */
    bool hasGpuFallenBehind(Renderer* UTILS_NONNULL renderer);

    /**
     * Applies the computed Latency Offset timestamp directly onto the rendering command stream.
     *
     * This instructs the underlying display compositor (such as SurfaceFlinger) exactly when to latch
     * and present the buffer, eliminating micro-stutter. This must be called before
     * `Renderer::endFrame()`.
     *
     * Calling this method automatically applies the target presentation time, desired presentation
     * time, and rendering deadline onto the target Renderer (via `Renderer::setPresentationTime`,
     * `Renderer::setDesiredPresentationTime`, and `Renderer::setRenderingDeadline`).
     *
     * @param renderer The Filament Renderer displaying the target View.
     */
    void applyPresentationTime(Renderer* UTILS_NONNULL renderer);

    /**
     * Returns the target presentation timepoint computed during the most recent call to `setupFrame()`.
     *
     * This timestamp is highly useful for client applications to calculate deterministic,
     * judder-free physics and animation transformations.
     *
     * @return The upcoming frame's expected presentation timepoint on the steady clock.
     */
    time_point_t getExpectedPresentationTime() const noexcept;

    /**
     * Returns the target rendering deadline timepoint computed during the most recent call to `setupFrame()`.
     *
     * This timepoint represents the absolute latest point on the steady clock by which the CPU and GPU
     * must complete their rendering operations so that the buffer will successfully meet its
     * display latching window.
     *
     * @return The upcoming frame's expected rendering deadline on the steady clock.
     */
    time_point_t getRenderingDeadline() const noexcept;

    /**
     * Returns the effective target latency in nanoseconds.
     * Calculated as the difference between the target presentation time and the frame's base time.
     *
     * @return The active target latency in nanoseconds.
     */
    std::chrono::nanoseconds getEffectiveLatency() const noexcept;

    /**
     * Returns the current flow control status of the pacing pipeline.
     *
     * If the pipeline is DISPLAY_STARVING, the application may choose to recover
     * by skipping a frame to rebuild queue depth and calling resetPacing().
     *
     * @return The active PacingStatus.
     */
    PacingStatus getPacingStatus() const noexcept;

    /**
     * Forces the FramePacer to abandon its relative pacing state and rigidly re-anchor
     * to the configured target latency on the next frame.
     *
     * The application can call this when recovering from a dropped frame or to manually
     * re-establish the queue depth.
     *
     * \code
     * if (pacer->setupFrame(tick) == FramePacer::FrameStatus::ACCEPTED) {
     *     // Alternatively to rendering an extra frame via setupExtraFrame(), the application
     *     // can accept a one-frame judder and recover queue depth by resetting the pacer.
     *     // This forces the next frame to rigidly re-anchor to the target latency.
     *     if (pacer->getPacingStatus() == FramePacer::PacingStatus::DISPLAY_STARVING) {
     *         pacer->resetPacing();
     *     }
     *
     *     pacer->applyPresentationTime(renderer);
     *     if (renderer->beginFrame(swapChain)) {
     *         renderer->render(view);
     *         renderer->endFrame();
     *     }
     * }
     * \endcode
     */
    void resetPacing() noexcept;

    /**
     * Returns the actual pacing frame rate selected during the active frame pacing cycle.
     *
     * If the requested target frame rate is fuzzy-matched to an active display hardware cadence
     * (such as 60.0 FPS requested on a 59.94Hz broadcast screen), this returns the actual hardware
     * rate (59.94 FPS).
     *
     * Additionally, if the requested rate exceeds the maximum refresh capability of the active
     * physical display panel (such as 90 FPS requested on a 60Hz screen), this returns the clamped
     * maximum display refresh rate (60 FPS).
     *
     * @return The active pacing frame rate in frames per second.
     */
    float getSelectedFrameRate() const noexcept;

    /**
     * Returns whether the selected pacing frame rate is achieved exactly by the display hardware.
     *
     * @return true if the selected rate is an exact integer fraction of the host display platform's
     *         refresh rate, false if non-integer ratio pacing is active (such as 45 FPS on 60Hz).
     */
    bool isExactFrameRateAchieved() const noexcept;

#if defined(__ANDROID__)
    /**
     * Extracts all candidate hardware presentation timelines from native Android AChoreographer telemetry
     * into the provided output vector, allowing zero-allocation execution in the steady state.
     *
     * The out vector is always clear()'ed first before candidate timelines are appended.
     *
     * @param out  Target vector where candidate timelines will be populated. Reusing this vector
     *             across frames avoids repeated dynamic memory allocations.
     * @param data Native telemetry pointer received in an custom AChoreographer_vsyncCallback.
     * @return A reference to the populated out vector.
     */
    static std::vector<HardwareTimeline>& extractTimelines(
            std::vector<HardwareTimeline>& out,
            const AChoreographerFrameCallbackData* UTILS_NONNULL data) noexcept;
#endif

protected:
    // prevent destruction by the user
    ~FramePacer() = default;
};

} // namespace filament

#endif // TNT_FILAMENT_FRAMEPACER_H
