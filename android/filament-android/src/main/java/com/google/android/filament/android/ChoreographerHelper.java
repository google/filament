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
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;

import com.google.android.filament.Renderer;

/**
 * {@code ChoreographerHelper} is a utility class that encapsulates Android's {@link Choreographer} callbacks
 * to properly schedule rendering loops and feed desired presentation timestamps and deadlines to Filament's {@link Renderer}.
 *
 * <p>It automatically utilizes Android 13+ (API 33) {@code VsyncCallback} and {@code FrameTimeline} APIs when available,
 * gracefully falling back to traditional {@code FrameCallback} APIs on older Android versions.
 *
 * <p>This class can be used in two distinct architectural patterns: <b>Inheritance Mode</b> and <b>Composition Mode</b>.
 *
 * <h3>Usage Example 1: Inheritance Mode (Recommended for Custom Handlers)</h3>
 * <pre>{@code
 * public class MyFrameHandler extends ChoreographerHelper {
 *     @Override
 *     public void onFrame(long frameTimeNanos, @Nullable Object frameData) {
 *         // Optional: Configure Filament FrameData or Presentation Timestamps
 *         if (Build.VERSION.SDK_INT >= 33 && frameData instanceof Choreographer.FrameData) {
 *             modelViewer.render((Choreographer.FrameData) frameData);
 *         } else {
 *             modelViewer.render(frameTimeNanos);
 *         }
 *     }
 * }
 *
 * MyFrameHandler handler = new MyFrameHandler();
 * handler.post();
 * }</pre>
 *
 * <h3>Usage Example 2: Composition Mode (Recommended for Anonymous Callbacks)</h3>
 * <pre>{@code
 * ChoreographerHelper helper = new ChoreographerHelper(new ChoreographerHelper.Callback() {
 *     @Override
 *     public void onFrame(long frameTimeNanos) {
 *         // Render frame...
 *     }
 * });
 *
 * // Optional: Let the helper automatically apply frame timeline targets to your Renderer
 * helper.setRenderer(myRenderer);
 * helper.post();
 * }</pre>
 */
public class ChoreographerHelper {
    /**
     * Callback interface for receiving frame synchronization events in Composition Mode.
     */
    public interface Callback {
        /**
         * Called when a new frame should be rendered.
         *
         * @param frameTimeNanos Monotonic timestamp of the frame in nanoseconds.
         */
        void onFrame(long frameTimeNanos);

        /**
         * Called when a new frame should be rendered, providing optional payload telemetry.
         *
         * @param frameTimeNanos Monotonic timestamp of the frame in nanoseconds.
         * @param frameData Optional {@code Choreographer.FrameData} instance on Android 13+ (API 33).
         *                  Passed as an {@code Object} to avoid verification failures on older API levels.
         */
        default void onFrame(long frameTimeNanos, @Nullable Object frameData) {
            onFrame(frameTimeNanos);
        }
    }

    private final Choreographer mChoreographer;
    private final Callback mCallback;
    private Renderer mRenderer;

    private final Object mFrameScheduler;

    /**
     * Initializes a new {@code ChoreographerHelper} in Inheritance Mode.
     * <p>Subclasses must override {@link #onFrame(long)} or {@link #onFrame(long, Object)} to handle frame rendering.
     */
    public ChoreographerHelper() {
        mChoreographer = Choreographer.getInstance();
        mCallback = null;
        if (Build.VERSION.SDK_INT >= 33) {
            mFrameScheduler = new VsyncScheduler();
        } else {
            mFrameScheduler = new FrameScheduler();
        }
    }

    /**
     * Initializes a new {@code ChoreographerHelper} in Composition Mode with a client callback.
     *
     * @param callback A non-null {@link Callback} instance to invoke on every frame.
     */
    public ChoreographerHelper(@NonNull Callback callback) {
        mChoreographer = Choreographer.getInstance();
        mCallback = callback;
        if (Build.VERSION.SDK_INT >= 33) {
            mFrameScheduler = new VsyncScheduler();
        } else {
            mFrameScheduler = new FrameScheduler();
        }
    }

    /**
     * Attaches an optional Filament {@link Renderer} to be automatically paced by this helper.
     * <p>When configured, the helper automatically calls {@link Renderer#setDesiredPresentationTime(long)}
     * and {@link Renderer#setRenderingDeadline(long)} using the preferred {@code FrameTimeline} on Android 13+.
     * <p>If you are using {@code FramePacer} in your rendering loop, leave this set to {@code null}.
     *
     * @param renderer The {@link Renderer} to configure, or {@code null} to disable automated timeline pacing.
     */
    public void setRenderer(@Nullable Renderer renderer) {
        mRenderer = renderer;
    }

    /**
     * Posts a callback to Android's {@link Choreographer} to schedule the next frame.
     * <p>This method automatically posts a continuous synchronization loop until {@link #remove()} is called.
     */
    public void post() {
        if (Build.VERSION.SDK_INT >= 33) {
            mChoreographer.postVsyncCallback((Choreographer.VsyncCallback) mFrameScheduler);
        } else {
            mChoreographer.postFrameCallback((Choreographer.FrameCallback) mFrameScheduler);
        }
    }

    /**
     * Cancels any pending frame synchronization callbacks, stopping the scheduling loop.
     */
    public void remove() {
        if (Build.VERSION.SDK_INT >= 33) {
            mChoreographer.removeVsyncCallback((Choreographer.VsyncCallback) mFrameScheduler);
        } else {
            mChoreographer.removeFrameCallback((Choreographer.FrameCallback) mFrameScheduler);
        }
    }

    /**
     * Base callback invoked when a new frame should be rendered.
     * <p>In Inheritance Mode, subclasses should override this method if they do not require {@code FrameData}.
     * <p>In Composition Mode, this delegates to the attached {@link Callback#onFrame(long)}.
     *
     * @param frameTimeNanos Monotonic timestamp of the frame in nanoseconds.
     */
    public void onFrame(long frameTimeNanos) {
        if (mCallback != null) {
            mCallback.onFrame(frameTimeNanos);
        }
    }

    /**
     * Main callback invoked when a new frame should be rendered, providing optional payload telemetry.
     * <p>In Inheritance Mode, subclasses can override this method to receive Android 13+ {@code FrameData}.
     * <p>In Composition Mode, this delegates to the attached {@link Callback#onFrame(long, Object)}.
     *
     * @param frameTimeNanos Monotonic timestamp of the frame in nanoseconds.
     * @param frameData Optional {@code Choreographer.FrameData} instance on Android 13+ (API 33).
     *                  Passed as an {@code Object} to avoid verification failures on older API levels.
     */
    public void onFrame(long frameTimeNanos, @Nullable Object frameData) {
        if (mCallback != null) {
            mCallback.onFrame(frameTimeNanos, frameData);
        } else {
            onFrame(frameTimeNanos);
        }
    }

    @RequiresApi(33)
    private class VsyncScheduler implements Choreographer.VsyncCallback {
        @Override
        public void onVsync(Choreographer.FrameData frameData) {
            mChoreographer.postVsyncCallback(this);
            if (mRenderer != null) {
                Choreographer.FrameTimeline timeline = frameData.getPreferredFrameTimeline();
                mRenderer.setDesiredPresentationTime(timeline.getExpectedPresentationTimeNanos());
                mRenderer.setRenderingDeadline(timeline.getDeadlineNanos());
            }
            onFrame(frameData.getFrameTimeNanos(), frameData);
        }
    }

    private class FrameScheduler implements Choreographer.FrameCallback {
        @Override
        public void doFrame(long frameTimeNanos) {
            mChoreographer.postFrameCallback(this);
            onFrame(frameTimeNanos, null);
        }
    }
}
