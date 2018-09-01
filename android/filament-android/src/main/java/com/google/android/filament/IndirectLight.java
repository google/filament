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
import android.support.annotation.Size;

public class IndirectLight {
    long mNativeObject;

    private IndirectLight(long indirectLight) {
        mNativeObject = indirectLight;
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
        public Builder reflections(@NonNull Texture cubemap) {
            nBuilderReflections(mNativeBuilder, cubemap.getNativeObject());
            return this;
        }

        @NonNull
        public Builder irradiance(@IntRange(from=1, to=3) int bands, @NonNull float[] sh) {
            switch (bands) {
                case 1: if (sh.length < 3)
                        throw new ArrayIndexOutOfBoundsException(
                            "1 band SH, array must be at least 1 x float3"); else break;
                case 2: if (sh.length < 4 * 3)
                        throw new ArrayIndexOutOfBoundsException(
                            "2 bands SH, array must be at least 4 x float3"); else break;
                case 3: if (sh.length < 9 * 3)
                        throw new ArrayIndexOutOfBoundsException(
                            "3 bands SH, array must be at least 9 x float3"); else break;
                default: throw new IllegalArgumentException("bands must be 1, 2 or 3");
            }
            nIrradiance(mNativeBuilder, bands, sh);
            return this;
        }

        @NonNull
        public Builder irradiance(@NonNull Texture cubemap) {
            nIrradianceAsTexture(mNativeBuilder, cubemap.getNativeObject());
            return this;
        }

        @NonNull
        public Builder intensity(float envIntensity) {
            nIntensity(mNativeBuilder, envIntensity);
            return this;
        }

        @NonNull
        public Builder rotation(@NonNull @Size(min = 9) float rotation[]) {
            nRotation(mNativeBuilder,
                    rotation[0], rotation[1], rotation[2],
                    rotation[3], rotation[4], rotation[5],
                    rotation[6], rotation[7], rotation[8]);
            return this;
        }

        @NonNull
        public IndirectLight build(@NonNull Engine engine) {
            long nativeIndirectLight = nBuilderBuild(mNativeBuilder, engine.getNativeObject());
            if (nativeIndirectLight == 0) throw new IllegalStateException("Couldn't create IndirectLight");
            return new IndirectLight(nativeIndirectLight);
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

    public void setIntensity(float intensity) {
        nSetIntensity(getNativeObject(), intensity);
    }

    public float getIntensity() {
        return nGetIntensity(getNativeObject());
    }

    public void setRotation(@NonNull @Size(min = 9) float rotation[]) {
        nSetRotation(getNativeObject(),
                rotation[0], rotation[1], rotation[2],
                rotation[3], rotation[4], rotation[5],
                rotation[6], rotation[7], rotation[8]);
    }

    long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed IndirectLight");
        }
        return mNativeObject;
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native long nCreateBuilder();
    private static native void nDestroyBuilder(long nativeBuilder);

    private static native long nBuilderBuild(long nativeBuilder, long nativeEngine);
    private static native void nBuilderReflections(long nativeBuilder, long nativeTexture);
    private static native void nIrradiance(long nativeBuilder, int bands, float[] sh);
    private static native void nIrradianceAsTexture(long nativeBuilder, long nativeTexture);
    private static native void nIntensity(long nativeBuilder, float envIntensity);
    private static native void nRotation(long nativeBuilder, float v0, float v1, float v2, float v3, float v4, float v5, float v6, float v7, float v8) ;

    private static native void nSetIntensity(long nativeIndirectLight, float intensity);
    private static native float nGetIntensity(long nativeIndirectLight);
    private static native void nSetRotation(long nativeIndirectLight, float v0, float v1, float v2, float v3, float v4, float v5, float v6, float v7, float v8);

}
