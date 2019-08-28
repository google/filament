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

import com.google.android.filament.proguard.UsedByReflection;

public class Skybox {
    private long mNativeObject;

    @UsedByReflection("KtxLoader.java")
    Skybox(long nativeSkybox) {
        mNativeObject = nativeSkybox;
    }

    public static class Builder {
        @SuppressWarnings({"FieldCanBeLocal", "UnusedDeclaration"}) // Keep to finalize native resources
        private final BuilderFinalizer mFinalizer;
        private final long mNativeBuilder;

        public Builder() {
            mNativeBuilder = nCreateBuilder();
            mFinalizer = new BuilderFinalizer(mNativeBuilder);
        }

        @NonNull
        public Builder environment(@NonNull Texture texture) {
            nBuilderEnvironment(mNativeBuilder, texture.getNativeObject());
            return this;
        }

        @NonNull
        public Builder showSun(boolean show) {
            nBuilderShowSun(mNativeBuilder, show);
            return this;
        }

        @NonNull
        public Skybox build(@NonNull Engine engine) {
            long nativeSkybox = nBuilderBuild(mNativeBuilder, engine.getNativeObject());
            if (nativeSkybox == 0) throw new IllegalStateException("Couldn't create Skybox");
            return new Skybox(nativeSkybox);
        }

        private static class BuilderFinalizer {
            private final long mNativeObject;

            BuilderFinalizer(long nativeObject) { mNativeObject = nativeObject; }

            @Override
            public void finalize() {
                try {
                    super.finalize();
                } catch (Throwable t) { // Ignore
                } finally {
                    nDestroyBuilder(mNativeObject);
                }
            }
        }
    }

    public void setLayerMask(@IntRange(from = 0, to = 255) int select, @IntRange(from = 0, to = 255) int value) {
        nSetLayerMask(getNativeObject(), select & 0xff, value & 0xff);
    }

    public int getLayerMask() {
        return nGetLayerMask(getNativeObject());
    }

    public long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed Skybox");
        }
        return mNativeObject;
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native long nCreateBuilder();
    private static native void nDestroyBuilder(long nativeSkyboxBuilder);
    private static native void nBuilderEnvironment(long nativeSkyboxBuilder, long nativeTexture);
    private static native void nBuilderShowSun(long nativeSkyboxBuilder, boolean show);
    private static native long nBuilderBuild(long nativeSkyboxBuilder, long nativeEngine);
    private static native void nSetLayerMask(long nativeSkybox, int select, int value);
    private static native int  nGetLayerMask(long nativeSkybox);
}
