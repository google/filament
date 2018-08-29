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

package com.google.android.filament;

import android.support.annotation.IntRange;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.annotation.Size;

import static com.google.android.filament.Colors.*;

public class View {
    private long mNativeObject;
    private String mName;
    private Scene mScene;
    private Camera mCamera;
    private Viewport mViewport = new Viewport(0, 0, 0, 0);
    private DynamicResolutionOptions mDynamicResolution;
    private DepthPrepass mDepthPrepass = DepthPrepass.DEFAULT;

    public static class DynamicResolutionOptions {
        public boolean enabled = false;
        public boolean homogeneousScaling = false;
        public float targetFrameTimeMilli = 1000.0f / 60.0f;
        public float headRoomRatio = 0.0f;
        public float scaleRate = 0.125f;
        public float minScale = 0.5f;
        public float maxScale = 1.0f;
        public int history = 9;
    };

    public enum AntiAliasing {
        NONE,
        FXAA
    }

    public enum DepthPrepass {
        DEFAULT(-1),
        DISABLED(0),
        ENABLED(1);

        final int value;

        DepthPrepass(int value) {
            this.value = value;
        }
    };

    View(long nativeView) {
        mNativeObject = nativeView;
    }

    public void setName(@NonNull String name) {
        mName = name;
        nSetName(getNativeObject(), name);
    }

    @Nullable
    public String getName() {
        return mName;
    }

    public void setScene(@Nullable Scene scene) {
        mScene = scene;
        nSetScene(getNativeObject(), scene == null ? 0 : scene.getNativeObject());
    }

    @Nullable
    public Scene getScene() {
        return mScene;
    }

    public void setCamera(@Nullable Camera camera) {
        mCamera = camera;
        nSetCamera(getNativeObject(), camera == null ? 0 : camera.getNativeObject());
    }

    @Nullable
    public Camera getCamera() {
        return mCamera;
    }

    public void setViewport(@NonNull Viewport viewport) {
        mViewport = viewport;
        nSetViewport(getNativeObject(),
                mViewport.left, mViewport.bottom, mViewport.width, mViewport.height);
    }

    @NonNull
    public Viewport getViewport() {
        return mViewport;
    }

    public void setClearColor(
            @LinearColor float r, @LinearColor float g, @LinearColor float b, float a) {
        nSetClearColor(getNativeObject(), r, g, b, a);
    }

    @NonNull @Size(min = 4)
    public float[] getClearColor(@NonNull @Size(min = 4) float[] out) {
        out = assertFloat4(out);
        nGetClearColor(getNativeObject(), out);
        return out;
    }

    public void setClearTargets(boolean color, boolean depth, boolean stencil) {
        nSetClearTargets(getNativeObject(), color, depth, stencil);
    }

    public void setVisibleLayers(
            @IntRange(from = 0, to = 255) int select,
            @IntRange(from = 0, to = 255) int values) {
        nSetVisibleLayers(getNativeObject(), select & 0xFF, values & 0xFF);
    }

    public void setShadowsEnabled(boolean enabled) {
        nSetShadowsEnabled(getNativeObject(), enabled);
    }

    public void setSampleCount(int count) {
        nSetSampleCount(getNativeObject(), count);
    }

    public int getSampleCount() {
        return nGetSampleCount(getNativeObject());
    }

    public void setAntiAliasing(@NonNull AntiAliasing type) {
        nSetAntiAliasing(getNativeObject(), type.ordinal());
    }

    @NonNull
    public AntiAliasing getAntiAliasing() {
        return AntiAliasing.values()[nGetAntiAliasing(getNativeObject())];
    }

    public void setDynamicResolutionOptions(@NonNull DynamicResolutionOptions options) {
        mDynamicResolution = options;
        nSetDynamicResolutionOptions(getNativeObject(),
                options.enabled,
                options.homogeneousScaling,
                options.targetFrameTimeMilli,
                options.headRoomRatio,
                options.scaleRate,
                options.minScale,
                options.maxScale,
                options.history);
    }

    @NonNull
    public DynamicResolutionOptions getDynamicResolutionOptions() {
        if (mDynamicResolution == null) {
            mDynamicResolution = new DynamicResolutionOptions();
        }
        return mDynamicResolution;
    }

    @NonNull
    public DepthPrepass getDepthPrepass() {
        return mDepthPrepass;
    }

    public void setDepthPrepass(@NonNull DepthPrepass depthPrepass) {
        mDepthPrepass = mDepthPrepass;
        nSetDepthPrepass(getNativeObject(), depthPrepass.value);
    }

    public boolean isPostProcessingEnabled() {
        return nIsPostProcessingEnabled(getNativeObject());
    }

    public void setPostProcessingEnabled(boolean enabled) {
        nSetPostProcessingEnabled(getNativeObject(), enabled);
    }

    public void setDynamicLightingOptions(float zLightNear, float zLightFar) {
        nSetDynamicLightingOptions(getNativeObject(), zLightNear, zLightFar);
    }

    long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed View");
        }
        return mNativeObject;
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    @NonNull @Size(min = 4)
    private static float[] assertFloat4(@Nullable float[] out) {
        if (out == null) out = new float[4];
        else if (out.length < 4) {
            throw new ArrayIndexOutOfBoundsException("Array length must be at least 4");
        }
        return out;
    }

    private static native void nSetName(long nativeView, String name);
    private static native void nSetScene(long nativeView, long nativeScene);
    private static native void nSetCamera(long nativeView, long nativeCamera);
    private static native void nSetViewport(long nativeView, int left, int bottom, int width, int height);
    private static native void nSetClearColor(long nativeView, float r, float g, float b, float a);
    private static native void nGetClearColor(long nativeView, float[] out);
    private static native void nSetClearTargets(long nativeView, boolean color, boolean depth, boolean stencil);
    private static native void nSetVisibleLayers(long nativeView, int select, int value);
    private static native void nSetShadowsEnabled(long nativeView, boolean enabled);
    private static native void nSetSampleCount(long nativeView, int count);
    private static native int nGetSampleCount(long nativeView);
    private static native void nSetAntiAliasing(long nativeView, int type);
    private static native int nGetAntiAliasing(long nativeView);
    private static native void nSetDynamicResolutionOptions(long nativeView,
            boolean enabled, boolean homogeneousScaling,
            float targetFrameTimeMilli, float headRoomRatio, float scaleRate,
            float minScale, float maxScale, int history);
    private static native void nSetDynamicLightingOptions(long nativeView, float zLightNear, float zLightFar);
    private static native void nSetDepthPrepass(long nativeView, int value);
    private static native void nSetPostProcessingEnabled(long nativeView, boolean enabled);
    private static native boolean nIsPostProcessingEnabled(long nativeView);
}
