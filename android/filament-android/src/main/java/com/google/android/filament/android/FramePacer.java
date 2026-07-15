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

package com.google.android.filament.android;

import android.os.Build;
import android.view.Choreographer;
import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;

import com.google.android.filament.Engine;
import com.google.android.filament.Renderer;

/**
 * Coordinates frame scheduling and presentation timestamps across multi-threaded rendering architectures.
 *
 * <p>The FramePacer decouples the CPU rendering loop from fluctuating hardware display cadences, acting
 * as a deterministic filter between incoming platform VSYNC events (such as Android's Choreographer)
 * and native buffer presentation submissions (via {@link Renderer#setPresentationTime(long)}).
 */
public class FramePacer {
    // Pre-allocated backing storage to avoid steady-state Eden GC thrashing
    private long[] mCachedTimelines = new long[16];

    private long mNativeObject;

    /**
     * Constructs a new {@code FramePacer} instance.
     */
    public static class Builder {
        @SuppressWarnings({"FieldCanBeLocal", "UnusedDeclaration"})
        private final BuilderFinalizer mFinalizer;
        private long mNativeBuilder;

        public Builder() {
            mNativeBuilder = nCreateBuilder();
            mFinalizer = new BuilderFinalizer(mNativeBuilder);
        }

        /**
         * Sets the desired frame rendering step in Hz.
         *
         * @param fps Target frame rate (e.g., 60.0f or 30.0f). Must be greater than 0.
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder targetFrameRate(float fps) {
            nBuilderTargetFrameRate(mNativeBuilder, fps);
            return this;
        }

        /**
         * Sets the required latency window in terms of time duration.
         *
         * <p>The target latency is measured relative to the hardware VSYNC timestamp
         * ({@code frameTimeNanos}), as opposed to the callback entry/dispatch time. Note that
         * the hardware VSYNC time is always equal to or in the past with respect to the actual
         * callback dispatch time.
         *
         * @param latencyNanos Target latency duration in nanoseconds (defaults to 33.3ms). Must be greater than 0.
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder latencyNanos(long latencyNanos) {
            nBuilderLatencyNanos(mNativeBuilder, latencyNanos);
            return this;
        }

        /**
         * Sets the required latency window in terms of 60Hz display frames.
         *
         * @param frames The latency window in units of 60Hz frames (e.g. 2 frames = 33.3ms). Must be greater than 0.
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder latencyFrames(int frames) {
            long nanos = (long) ((frames / 60.0) * 1_000_000_000L);
            return latencyNanos(nanos);
        }

        /**
         * Creates the FramePacer object and returns a pointer to it.
         *
         * @param engine Reference to the {@link Engine} to associate this FramePacer with.
         * @return The newly created FramePacer instance.
         */
        @NonNull
        public FramePacer build(@NonNull Engine engine) {
            long nativePacer = nBuilderBuild(mNativeBuilder, engine.getNativeObject());
            if (nativePacer == 0) {
                throw new IllegalStateException("Couldn't create FramePacer instance");
            }
            return new FramePacer(nativePacer, engine);
        }

        private static class BuilderFinalizer {
            private final long mNativeObject;
            BuilderFinalizer(long nativeObject) { mNativeObject = nativeObject; }
            @Override
            public void finalize() {
                try {
                    super.finalize();
                } catch (Throwable t) {
                    // Ignore
                }
                nDestroyBuilder(mNativeObject);
            }
        }
    }

    private FramePacer(long nativePacer, @NonNull Engine engine) {
        mNativeObject = nativePacer;
    }

    public long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed FramePacer");
        }
        return mNativeObject;
    }

    public void clearNativeObject() {
        mNativeObject = 0;
    }

    public static class Configuration {
        /** The application's desired frame rendering step in Hz. */
        public float targetFrameRate = 60.0f;
        /** Target latency duration in nanoseconds (defaults to 33.3ms). */
        public long latencyNanos     = 33_333_333L;
    }

    public enum FrameStatus {
        SKIPPED_SPURIOUS(-2),   //!< Skipped to maintain target frame rate cadence (e.g. 30 FPS on 60Hz display).
        SKIPPED_STALE(-1),      //!< Skipped to prevent out-of-order presentation (monotonic guard).
        ACCEPTED(0);            //!< The frame is approved for rendering.

        private final int mValue;
        FrameStatus(int value) {
            mValue = value;
        }

        public static FrameStatus from(int value) {
            switch (value) {
                case 0: return ACCEPTED;
                case -1: return SKIPPED_STALE;
                case -2: return SKIPPED_SPURIOUS;
            }
            throw new IllegalArgumentException("Unknown FrameStatus value: " + value);
        }
    }

    public enum PacingStatus {
        STEADY(0),
        DISPLAY_STARVING(-1),
        DISPLAY_STUFFED(1);

        private final int mValue;
        PacingStatus(int value) {
            mValue = value;
        }

        public static PacingStatus from(int value) {
            switch (value) {
                case 0: return STEADY;
                case -1: return DISPLAY_STARVING;
                case 1: return DISPLAY_STUFFED;
            }
            throw new IllegalArgumentException("Unknown PacingStatus value: " + value);
        }
    }

    /**
     * Dynamically updates the active pacing targets mid-flight (e.g., for thermal or power mitigation).
     *
     * @param config The new configuration targets to scale to on subsequent frames.
     */
    public void configure(@NonNull Configuration config) {
        configure(config.targetFrameRate, config.latencyNanos);
    }

    public void configure(float targetFrameRate, long latencyNanos) {
        nConfigure(getNativeObject(), targetFrameRate, latencyNanos);
    }

    /**
     * Prepares and evaluates the frame pacing state for the upcoming frame cycle.
     *
     * <p>This must be called at the very beginning of the host display platform's VSYNC interrupt loop
     * (e.g., within Android's {@code Choreographer} callback). It evaluates whether the CPU rendering
     * thread is ahead of the hardware pulse cadence or backlogged.
     *
     * @param frameTimeNanos Incoming hardware base VSYNC timestamp in nanoseconds.
     * @param vsyncPeriodNanos Physical display VSYNC refresh period in nanoseconds.
     * @return FrameStatus::ACCEPTED if approved, or the specific SKIPPED reason.
     */
    public FrameStatus setupFrame(long frameTimeNanos, long vsyncPeriodNanos) {
        return FrameStatus.from(nSetupFrame(getNativeObject(), frameTimeNanos, vsyncPeriodNanos, null, 0));
    }

    /**
     * Prepares and evaluates the frame pacing state for the upcoming frame cycle.
     *
     * <p>This must be called at the very beginning of the host display platform's VSYNC interrupt loop
     * (e.g., within Android's {@code Choreographer} callback). It evaluates whether the CPU rendering
     * thread is ahead of the hardware pulse cadence or backlogged.
     *
     * @param frameTimeNanos Incoming hardware base VSYNC timestamp in nanoseconds.
     * @return FrameStatus::ACCEPTED if approved, or the specific SKIPPED reason.
     */
    public FrameStatus setupFrame(long frameTimeNanos) {
        return setupFrame(frameTimeNanos, 16666666L);
    }

    /**
     * Prepares and evaluates the frame pacing state for the upcoming frame cycle using Android 13+ FrameData.
     *
     * <p>This must be called at the very beginning of the host display platform's VSYNC interrupt loop
     * (e.g., within Android's {@code Choreographer} callback). It evaluates whether the CPU rendering
     * thread is ahead of the hardware pulse cadence or backlogged.
     *
     * @param frameData Native VSYNC telemetry object received in an Android 13+ Choreographer.VsyncCallback.
     * @param vsyncPeriodNanos Physical display VSYNC refresh period in nanoseconds.
     * @return FrameStatus::ACCEPTED if approved, or the specific SKIPPED reason.
     */
    @RequiresApi(33)
    public FrameStatus setupFrame(@NonNull Choreographer.FrameData frameData, long vsyncPeriodNanos) {
        Choreographer.FrameTimeline[] timelines = frameData.getFrameTimelines();
        int neededCapacity = timelines.length * 2;
        if (mCachedTimelines.length < neededCapacity) {
            mCachedTimelines = new long[neededCapacity];
        }

        for (int i = 0; i < timelines.length; ++i) {
            mCachedTimelines[i * 2]     = timelines[i].getExpectedPresentationTimeNanos();
            mCachedTimelines[i * 2 + 1] = timelines[i].getDeadlineNanos();
        }

        return FrameStatus.from(nSetupFrame(getNativeObject(), frameData.getFrameTimeNanos(), vsyncPeriodNanos, mCachedTimelines, timelines.length));
    }

    /**
     * Applies the computed Latency Offset timestamp directly onto the rendering command stream.
     *
     * <p>This instructs the underlying display compositor (such as SurfaceFlinger) exactly when to latch
     * and present the buffer, eliminating micro-stutter. This must be called before
     * {@link Renderer#endFrame()}.
     *
     * <p>Calling this method automatically applies the target presentation time, desired presentation
     * time, and rendering deadline onto the target Renderer (via {@link Renderer#setPresentationTime(long)},
     * {@link Renderer#setDesiredPresentationTime(long)}, and {@link Renderer#setRenderingDeadline(long)}).
     *
     * @param renderer The Filament Renderer displaying the target View.
     */
    public void applyPresentationTime(@NonNull Renderer renderer) {
        nApplyPresentationTime(getNativeObject(), renderer.getNativeObject());
    }

    /**
     * Checks if the GPU rendering pipeline has fallen behind the CPU submission rate.
     *
     * <p>If the GPU is delayed, this method returns true and automatically rolls back any internal
     * cadence state advanced by {@link #setupFrame(long, long)}. The client should skip the frame (via
     * {@link Renderer#skipFrame(long)}) and refrain from calling {@code applyPresentationTime()} or {@code beginFrame()}.
     *
     * @param renderer The Filament Renderer displaying the target View.
     * @return true if the GPU has fallen behind, false otherwise.
     */
    public boolean hasGpuFallenBehind(@NonNull Renderer renderer) {
        return nHasGpuFallenBehind(getNativeObject(), renderer.getNativeObject());
    }

    /**
     * Returns the target presentation timestamp computed during the most recent call to setupFrame().
     *
     * <p>This timestamp is highly useful for client applications to calculate deterministic,
     * judder-free physics and animation transformations.
     *
     * @return The upcoming frame's expected presentation timepoint in nanoseconds.
     */
    public long getExpectedPresentationTimeNanos() {
        return nGetExpectedPresentationTime(getNativeObject());
    }

    /**
     * Returns the target rendering deadline timestamp computed during the most recent call to setupFrame().
     *
     * <p>This timepoint represents the absolute latest point on the steady clock by which the CPU and GPU
     * must complete their rendering operations so that the buffer will successfully meet its
     * display latching window.
     *
     * @return The upcoming frame's expected rendering deadline in nanoseconds.
     */
    public long getRenderingDeadlineNanos() {
        return nGetRenderingDeadline(getNativeObject());
    }

    /**
     * Destroys this FramePacer instance and frees all associated native resources.
     *
     * @param engine The Engine associated with this FramePacer.
     */
    public void destroy(@NonNull Engine engine) {
        if (mNativeObject != 0) {
            nDestroy(engine.getNativeObject(), mNativeObject);
            mNativeObject = 0;
        }
    }

    /**
     * Returns the actual pacing frame rate selected during the active frame pacing cycle.
     *
     * <p>If the requested target frame rate is fuzzy-matched to an active display hardware cadence
     * (such as 60.0 FPS requested on a 59.94Hz broadcast screen), this returns the actual hardware
     * rate (59.94 FPS).
     *
     * <p>Additionally, if the requested rate exceeds the maximum refresh capability of the active
     * physical display panel (such as 90 FPS requested on a 60Hz screen), this returns the clamped
     * maximum display refresh rate (60 FPS).
     *
     * @return The active pacing frame rate in frames per second.
     */
    public float getSelectedFrameRate() {
        return nGetSelectedFrameRate(getNativeObject());
    }

    /**
     * Returns whether the selected pacing frame rate is achieved exactly by the display hardware.
     *
     * @return true if the selected rate is an exact integer fraction of the host display platform's
     *         refresh rate, false if non-integer ratio pacing is active (such as 45 FPS on 60Hz).
     */
    public boolean isExactFrameRateAchieved() {
        return nIsExactFrameRateAchieved(getNativeObject());
    }

    /**
     * Returns the effective target latency in nanoseconds.
     * Calculated as the difference between the target presentation time and the frame's base time.
     *
     * @return The active target latency in nanoseconds.
     */
    public long getEffectiveLatencyNanos() {
        return nGetEffectiveLatencyNanos(getNativeObject());
    }

    /**
     * Returns the current flow control status of the pacing pipeline.
     *
     * If the pipeline is DISPLAY_STARVING, the application may choose to recover
     * by skipping a frame to rebuild queue depth and calling resetPacing().
     *
     * @return The active PacingStatus.
     */
    @NonNull
    public PacingStatus getPacingStatus() {
        return PacingStatus.from(nGetPacingStatus(getNativeObject()));
    }

    /**
     * Forces the FramePacer to abandon its relative pacing state and rigidly re-anchor
     * to the configured target latency on the next frame.
     *
     * The application can call this when recovering from a dropped frame or to manually
     * re-establish the queue depth.
     *
     * <pre>
     * if (pacer.setupFrame(tick) == FramePacer.FrameStatus.ACCEPTED) {
     *     // Alternatively to rendering an extra frame via setupExtraFrame(), the application
     *     // can accept a one-frame judder and recover queue depth by resetting the pacer.
     *     // This forces the next frame to rigidly re-anchor to the target latency.
     *     if (pacer.getPacingStatus() == FramePacer.PacingStatus.DISPLAY_STARVING) {
     *         pacer.resetPacing();
     *     }
     *
     *     pacer.applyPresentationTime(renderer);
     *     if (renderer.beginFrame(swapChain)) {
     *         renderer.render(view);
     *         renderer.endFrame();
     *     }
     * }
     * </pre>
     */
    public void resetPacing() {
        nResetPacing(getNativeObject());
    }

    /**
     * Advances the internal pacing pipeline to target an extra presentation frame in the future,
     * without advancing the ideal cadence clock (mExpectedBaseTime).
     *
     * This method is explicitly designed for latency recovery (Buffer Stuffing). If the pipeline
     * reports PacingStatus.DISPLAY_STARVING, the application may yield, OR it may choose to render
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
     * <pre>
     * if (pacer.setupFrame(tick) == FramePacer.FrameStatus.ACCEPTED) {
     *     pacer.applyPresentationTime(renderer);
     *     if (renderer.beginFrame(swapChain)) {
     *         renderer.render(view);
     *         renderer.endFrame();
     *     }
     *
     *     // Automatically recover queue depth by stuffing an extra frame
     *     // If the application was starved by multiple frames, it will receive DISPLAY_STARVING
     *     // over consecutive Vsync callbacks, naturally amortizing the recovery over time.
     *     if (pacer.getPacingStatus() == FramePacer.PacingStatus.DISPLAY_STARVING) {
     *         if (pacer.setupExtraFrame()) {
     *             pacer.applyPresentationTime(renderer); // Emit the future-shifted timestamp
     *             if (renderer.beginFrame(swapChain)) {
     *                 // Optional: advance simulation state again here
     *                 renderer.render(view);
     *                 renderer.endFrame();
     *             }
     *         }
     *     }
     * }
     * </pre>
     *
     * @return true if the timestamp was safely advanced, false if refused to prevent over-stuffing.
     */
    public boolean setupExtraFrame() {
        return nSetupExtraFrame(getNativeObject());
    }

    private static native long nCreateBuilder();
    private static native void nDestroyBuilder(long nativeBuilder);
    private static native void nBuilderTargetFrameRate(long nativeBuilder, float fps);
    private static native void nBuilderLatencyFrames(long nativeBuilder, int frames);
    private static native void nBuilderLatencyNanos(long nativeBuilder, long latencyNanos);
    private static native long nBuilderBuild(long nativeBuilder, long nativeEngine);

    private static native int nSetupFrame(long nativeObject, long baseTimeNanos, long vsyncPeriodNanos, long[] hardwareTimelines, int timelineCount);
    private static native long nGetExpectedPresentationTime(long nativeObject);
    private static native long nGetRenderingDeadline(long nativeObject);
    private static native long nGetEffectiveLatencyNanos(long nativeObject);
    private static native boolean nHasGpuFallenBehind(long nativeObject, long nativeRenderer);
    private static native void nApplyPresentationTime(long nativeObject, long nativeRenderer);
    private static native void nConfigure(long nativeObject, float targetFrameRate, long latencyNanos);
    private static native float nGetSelectedFrameRate(long nativeObject);
    private static native boolean nIsExactFrameRateAchieved(long nativeObject);

    private static native int nGetPacingStatus(long nativeObject);
    private static native void nResetPacing(long nativeObject);
    private static native boolean nSetupExtraFrame(long nativeObject);
    private static native void nDestroy(long nativeEngine, long nativeObject);
}
