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

import android.view.Choreographer;
import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;

import com.google.android.filament.Engine;
import com.google.android.filament.Renderer;

/**
 * Android helper for {@link com.google.android.filament.FramePacer}.
 *
 * <p>Coordinates frame scheduling and presentation timestamps across multi-threaded rendering architectures
 * and integrates directly with Android's {@link Choreographer} APIs (including Android 13+ {@link Choreographer.FrameData}).</p>
 */
public class FramePacer {
    private final com.google.android.filament.FramePacer mFramePacer;
    // Pre-allocated backing storage to avoid steady-state Eden GC thrashing
    private long[] mCachedTimelines = new long[16];

    public enum FrameStatus {
        SKIPPED_SPURIOUS(-2),   //!< Skipped to maintain target frame rate cadence (e.g. 30 FPS on 60Hz display).
        SKIPPED_STALE(-1),      //!< Skipped to prevent out-of-order presentation (monotonic guard).
        ACCEPTED(0);            //!< The frame is approved for rendering.

        private final int mValue;
        FrameStatus(int value) {
            mValue = value;
        }

        public int toFilamentNative() {
            return mValue;
        }

        public static FrameStatus from(int value) {
            switch (value) {
                case 0: return ACCEPTED;
                case -1: return SKIPPED_STALE;
                case -2: return SKIPPED_SPURIOUS;
            }
            throw new IllegalArgumentException("Unknown FrameStatus value: " + value);
        }

        static FrameStatus from(com.google.android.filament.FramePacer.FrameStatus status) {
            return from(status.toFilamentNative());
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

        public int toFilamentNative() {
            return mValue;
        }

        public static PacingStatus from(int value) {
            switch (value) {
                case 0: return STEADY;
                case -1: return DISPLAY_STARVING;
                case 1: return DISPLAY_STUFFED;
            }
            throw new IllegalArgumentException("Unknown PacingStatus value: " + value);
        }

        static PacingStatus from(com.google.android.filament.FramePacer.PacingStatus status) {
            return from(status.toFilamentNative());
        }
    }

    public static class Configuration {
        /** The application's desired frame rendering step in Hz. */
        public float targetFrameRate = 60.0f;
        /** Target latency duration in nanoseconds (defaults to 33.3ms). */
        public long latencyNanos     = 33_333_333L;

        public Configuration() {}

        public Configuration(float targetFrameRate, long latencyNanos) {
            this.targetFrameRate = targetFrameRate;
            this.latencyNanos = latencyNanos;
        }
    }

    public FramePacer(@NonNull com.google.android.filament.FramePacer framePacer) {
        mFramePacer = framePacer;
    }

    /**
     * Constructs a new {@code FramePacer} instance.
     */
    public static class Builder {
        private final com.google.android.filament.FramePacer.Builder mBuilder = new com.google.android.filament.FramePacer.Builder();

        public Builder() {}

        /**
         * Sets the desired frame rendering step in Hz.
         *
         * @param fps Target frame rate (e.g., 60.0f or 30.0f). Must be greater than 0.
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder targetFrameRate(float fps) {
            mBuilder.targetFrameRate(fps);
            return this;
        }

        /**
         * Sets the required latency window in terms of time duration.
         *
         * @param latencyNanos Target latency duration in nanoseconds (defaults to 33.3ms). Must be greater than 0.
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder latencyNanos(long latencyNanos) {
            mBuilder.latency(latencyNanos);
            return this;
        }

        /**
         * Sets the required latency window in terms of time duration.
         *
         * @param latencyNanos Target latency duration in nanoseconds.
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder latency(long latencyNanos) {
            mBuilder.latency(latencyNanos);
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
            mBuilder.latencyFrames(frames);
            return this;
        }

        /**
         * Creates the FramePacer object and returns a wrapper instance.
         *
         * @param engine Reference to the {@link Engine} to associate this FramePacer with.
         * @return The newly created FramePacer instance.
         */
        @NonNull
        public FramePacer build(@NonNull Engine engine) {
            return new FramePacer(mBuilder.build(engine));
        }
    }

    /**
     * @return the underlying generated {@link com.google.android.filament.FramePacer} instance.
     */
    @NonNull
    public com.google.android.filament.FramePacer getPacer() {
        return mFramePacer;
    }

    public long getNativeObject() {
        return mFramePacer.getNativeObject();
    }

    public void clearNativeObject() {
        // Handled internally by Engine.destroyFramePacer
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
        mFramePacer.configure(new com.google.android.filament.FramePacer.Configuration(targetFrameRate, latencyNanos));
    }

    /**
     * Prepares and evaluates the frame pacing state for the upcoming frame cycle.
     *
     * @param frameTimeNanos Incoming hardware base VSYNC timestamp in nanoseconds.
     * @param vsyncPeriodNanos Physical display VSYNC refresh period in nanoseconds.
     * @return FrameStatus::ACCEPTED if approved, or the specific SKIPPED reason.
     */
    public FrameStatus setupFrame(long frameTimeNanos, long vsyncPeriodNanos) {
        return FrameStatus.from(mFramePacer.setupFrame(frameTimeNanos, vsyncPeriodNanos));
    }

    /**
     * Prepares and evaluates the frame pacing state for the upcoming frame cycle.
     *
     * @param frameTimeNanos Incoming hardware base VSYNC timestamp in nanoseconds.
     * @return FrameStatus::ACCEPTED if approved, or the specific SKIPPED reason.
     */
    public FrameStatus setupFrame(long frameTimeNanos) {
        return FrameStatus.from(mFramePacer.setupFrame(frameTimeNanos));
    }

    /**
     * Prepares and evaluates the frame pacing state for the upcoming frame cycle using Android 13+ FrameData.
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

        return FrameStatus.from(mFramePacer.setupFrame(frameData.getFrameTimeNanos(), vsyncPeriodNanos, mCachedTimelines, timelines.length));
    }

    /**
     * Applies the computed Latency Offset timestamp directly onto the rendering command stream.
     *
     * @param renderer The Filament Renderer displaying the target View.
     */
    public void applyPresentationTime(@NonNull Renderer renderer) {
        mFramePacer.applyPresentationTime(renderer);
    }

    /**
     * Checks if the GPU rendering pipeline has fallen behind the CPU submission rate.
     *
     * @param renderer The Filament Renderer displaying the target View.
     * @return true if the GPU has fallen behind, false otherwise.
     */
    public boolean hasGpuFallenBehind(@NonNull Renderer renderer) {
        return mFramePacer.hasGpuFallenBehind(renderer);
    }

    /**
     * Returns the target presentation timestamp computed during the most recent call to setupFrame().
     *
     * @return The upcoming frame's expected presentation timepoint in nanoseconds.
     */
    public long getExpectedPresentationTime() {
        return mFramePacer.getExpectedPresentationTime();
    }

    /**
     * Backwards-compatibility alias for {@link #getExpectedPresentationTime()}.
     *
     * @return The upcoming frame's expected presentation timepoint in nanoseconds.
     */
    public long getExpectedPresentationTimeNanos() {
        return mFramePacer.getExpectedPresentationTime();
    }

    /**
     * Returns the target rendering deadline timestamp computed during the most recent call to setupFrame().
     *
     * @return The upcoming frame's expected rendering deadline in nanoseconds.
     */
    public long getRenderingDeadline() {
        return mFramePacer.getRenderingDeadline();
    }

    /**
     * Backwards-compatibility alias for {@link #getRenderingDeadline()}.
     *
     * @return The upcoming frame's expected rendering deadline in nanoseconds.
     */
    public long getRenderingDeadlineNanos() {
        return mFramePacer.getRenderingDeadline();
    }

    /**
     * Returns the effective target latency in nanoseconds.
     *
     * @return The active target latency in nanoseconds.
     */
    public long getEffectiveLatency() {
        return mFramePacer.getEffectiveLatency();
    }

    /**
     * Backwards-compatibility alias for {@link #getEffectiveLatency()}.
     *
     * @return The active target latency in nanoseconds.
     */
    public long getEffectiveLatencyNanos() {
        return mFramePacer.getEffectiveLatency();
    }

    /**
     * Returns the actual pacing frame rate selected during the active frame pacing cycle.
     *
     * @return The active pacing frame rate in frames per second.
     */
    public float getSelectedFrameRate() {
        return mFramePacer.getSelectedFrameRate();
    }

    /**
     * Returns whether the selected pacing frame rate is achieved exactly by the display hardware.
     *
     * @return true if the selected rate is an exact integer fraction of the host display platform's
     *         refresh rate, false if non-integer ratio pacing is active.
     */
    public boolean isExactFrameRateAchieved() {
        return mFramePacer.isExactFrameRateAchieved();
    }

    /**
     * Returns the current flow control status of the pacing pipeline.
     *
     * @return The active PacingStatus.
     */
    @NonNull
    public PacingStatus getPacingStatus() {
        return PacingStatus.from(mFramePacer.getPacingStatus());
    }

    /**
     * Forces the FramePacer to abandon its relative pacing state and rigidly re-anchor
     * to the configured target latency on the next frame.
     */
    public void resetPacing() {
        mFramePacer.resetPacing();
    }

    /**
     * Advances the internal pacing pipeline to target an extra presentation frame in the future,
     * without advancing the ideal cadence clock (mExpectedBaseTime).
     *
     * @return true if the timestamp was safely advanced, false if refused to prevent over-stuffing.
     */
    public boolean setupExtraFrame() {
        return mFramePacer.setupExtraFrame();
    }

    /**
     * Destroys this FramePacer instance and frees all associated native resources.
     *
     * @param engine The Engine associated with this FramePacer.
     */
    public void destroy(@NonNull Engine engine) {
        engine.destroyFramePacer(mFramePacer);
    }
}
