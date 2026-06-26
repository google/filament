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
 * ChoreographerHelper is a simple utility that encapsulates Android's Choreographer callbacks
 * to properly schedule frames and feed desired presentation timestamps and deadlines to Filament's Renderer.
 */
public class ChoreographerHelper {
    public interface Callback {
        void onFrame(long frameTimeNanos);
    }

    private final Choreographer mChoreographer;
    private final Callback mCallback;
    private Renderer mRenderer;

    private final Object mFrameScheduler;

    public ChoreographerHelper() {
        mChoreographer = Choreographer.getInstance();
        mCallback = null;
        if (Build.VERSION.SDK_INT >= 33) {
            mFrameScheduler = new VsyncScheduler();
        } else {
            mFrameScheduler = new FrameScheduler();
        }
    }

    public ChoreographerHelper(@NonNull Callback callback) {
        mChoreographer = Choreographer.getInstance();
        mCallback = callback;
        if (Build.VERSION.SDK_INT >= 33) {
            mFrameScheduler = new VsyncScheduler();
        } else {
            mFrameScheduler = new FrameScheduler();
        }
    }

    public void setRenderer(@Nullable Renderer renderer) {
        mRenderer = renderer;
    }

    public void post() {
        if (Build.VERSION.SDK_INT >= 33) {
            mChoreographer.postVsyncCallback((Choreographer.VsyncCallback) mFrameScheduler);
        } else {
            mChoreographer.postFrameCallback((Choreographer.FrameCallback) mFrameScheduler);
        }
    }

    public void remove() {
        if (Build.VERSION.SDK_INT >= 33) {
            mChoreographer.removeVsyncCallback((Choreographer.VsyncCallback) mFrameScheduler);
        } else {
            mChoreographer.removeFrameCallback((Choreographer.FrameCallback) mFrameScheduler);
        }
    }

    public void onFrame(long frameTimeNanos) {
        if (mCallback != null) {
            mCallback.onFrame(frameTimeNanos);
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
            onFrame(frameData.getFrameTimeNanos());
        }
    }

    private class FrameScheduler implements Choreographer.FrameCallback {
        @Override
        public void doFrame(long frameTimeNanos) {
            mChoreographer.postFrameCallback(this);
            onFrame(frameTimeNanos);
        }
    }
}
