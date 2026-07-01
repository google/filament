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
         * Sets the required latency window in terms of frame pipeline depth.
         *
         * @param frames The frame pipeline depth (defaults to 2). Must be greater than 0.
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder latencyFrames(int frames) {
            nBuilderLatencyFrames(mNativeBuilder, frames);
            return this;
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

    /**
     * Holds dynamic pacing targets and latency pipeline depth requirements.
     */
    public static class Configuration {
        /** The application's desired frame rendering step in Hz. */
        public float targetFrameRate = 60.0f;
        /** The frame pipeline depth (sets N-2 or N-3 backpressure constraints). */
        public int latencyFrames     = 2;
    }

    /**
     * Dynamically updates the active pacing targets mid-flight (e.g., for thermal or power mitigation).
     *
     * @param config The new configuration targets to scale to on subsequent frames.
     */
    public void configure(@NonNull Configuration config) {
        configure(config.targetFrameRate, config.latencyFrames);
    }

    /**
     * Dynamically updates the active pacing targets mid-flight (e.g., for thermal or power mitigation).
     *
     * @param targetFrameRate The application's desired frame rendering step in Hz.
     * @param latencyFrames   The frame pipeline depth.
     */
    public void configure(float targetFrameRate, int latencyFrames) {
        nConfigure(getNativeObject(), targetFrameRate, latencyFrames);
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
     * @return true if the frame is approved for rendering, false if it should be skipped.
     */
    public boolean setupFrame(long frameTimeNanos, long vsyncPeriodNanos) {
        return nSetupFrame(getNativeObject(), frameTimeNanos, vsyncPeriodNanos, null, 0);
    }

    /**
     * Prepares and evaluates the frame pacing state for the upcoming frame cycle.
     *
     * <p>This must be called at the very beginning of the host display platform's VSYNC interrupt loop
     * (e.g., within Android's {@code Choreographer} callback). It evaluates whether the CPU rendering
     * thread is ahead of the hardware pulse cadence or backlogged.
     *
     * @param frameTimeNanos Incoming hardware base VSYNC timestamp in nanoseconds.
     * @return true if the frame is approved for rendering, false if it should be skipped.
     */
    public boolean setupFrame(long frameTimeNanos) {
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
     * @return true if the frame is approved for rendering, false if it should be skipped.
     */
    @RequiresApi(33)
    public boolean setupFrame(@NonNull Choreographer.FrameData frameData, long vsyncPeriodNanos) {
        Choreographer.FrameTimeline[] timelines = frameData.getFrameTimelines();
        int neededCapacity = timelines.length * 2;
        if (mCachedTimelines.length < neededCapacity) {
            mCachedTimelines = new long[neededCapacity];
        }

        for (int i = 0; i < timelines.length; ++i) {
            mCachedTimelines[i * 2]     = timelines[i].getExpectedPresentationTimeNanos();
            mCachedTimelines[i * 2 + 1] = timelines[i].getDeadlineNanos();
        }

        return nSetupFrame(getNativeObject(), frameData.getFrameTimeNanos(), vsyncPeriodNanos, mCachedTimelines, timelines.length);
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

    private static native long nCreateBuilder();
    private static native void nDestroyBuilder(long nativeBuilder);
    private static native void nBuilderTargetFrameRate(long nativeBuilder, float fps);
    private static native void nBuilderLatencyFrames(long nativeBuilder, int frames);
    private static native long nBuilderBuild(long nativeBuilder, long nativeEngine);

    private static native boolean nSetupFrame(long nativeObject, long baseTimeNanos, long vsyncPeriodNanos, long[] hardwareTimelines, int timelineCount);
    private static native long nGetExpectedPresentationTime(long nativeObject);
    private static native long nGetRenderingDeadline(long nativeObject);
    private static native boolean nHasGpuFallenBehind(long nativeObject, long nativeRenderer);
    private static native void nApplyPresentationTime(long nativeObject, long nativeRenderer);
    private static native void nConfigure(long nativeObject, float targetFrameRate, int latencyFrames);
    private static native float nGetSelectedFrameRate(long nativeObject);
    private static native boolean nIsExactFrameRateAchieved(long nativeObject);
    private static native void nDestroy(long nativeEngine, long nativeObject);
}
