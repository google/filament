/*
 * Copyright (C) 2021 The Android Open Source Project
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

import androidx.annotation.IntRange;
import androidx.annotation.NonNull;

import java.nio.Buffer;
import java.nio.BufferOverflowException;
import java.nio.FloatBuffer;

public class SkinningBuffer {
    private long mNativeObject;
    private SkinningBuffer(long nativeSkinningBuffer) {
        mNativeObject = nativeSkinningBuffer;
    }

    public static class Builder {
        @SuppressWarnings({"FieldCanBeLocal", "UnusedDeclaration"})
        // Keep to finalize native resources
        private final SkinningBuffer.Builder.BuilderFinalizer mFinalizer;
        private final long mNativeBuilder;

        public Builder() {
            mNativeBuilder = nCreateBuilder();
            mFinalizer = new SkinningBuffer.Builder.BuilderFinalizer(mNativeBuilder);
        }

        /**
         * Size of the skinning buffer in bones.
         *
         * <p>Due to limitation in the GLSL, the SkinningBuffer must always by a multiple of
         * 256, this adjustment is done automatically, but can cause
         * some memory overhead. This memory overhead can be mitigated by using the same
         * {@link SkinningBuffer} to store the bone information for multiple RenderPrimitives.</p>
         *
         * @param boneCount Number of bones the skinning buffer can hold.
         * @return this <code>Builder</code> object for chaining calls
         */
        @NonNull
        public Builder boneCount(@IntRange(from = 1) int boneCount) {
            nBuilderBoneCount(mNativeBuilder, boneCount);
            return this;
        }

        /**
         * The new buffer is created with identity bones
         *
         * @param initialize true to initializing the buffer, false to not.
         * @return A reference to this Builder for chaining calls.
         */
        @NonNull
        public Builder initialize(boolean initialize) {
            nBuilderInitialize(mNativeBuilder, initialize);
            return this;
        }

        /**
         * Creates and returns the <code>SkinningBuffer</code> object.
         *
         * @param engine reference to the {@link Engine} to associate this <code>SkinningBuffer</code>
         *               with.
         *
         * @return the newly created <code>SkinningBuffer</code> object
         *
         * @exception IllegalStateException if the SkinningBuffer could not be created
         *
         * @see #setBonesAsMatrices
         * @see #setBonesAsQuaternions
         */
        @NonNull
        public SkinningBuffer build(@NonNull Engine engine) {
            long nativeSkinningBuffer = nBuilderBuild(mNativeBuilder, engine.getNativeObject());
            if (nativeSkinningBuffer == 0)
                throw new IllegalStateException("Couldn't create SkinningBuffer");
            return new SkinningBuffer(nativeSkinningBuffer);
        }

        private static class BuilderFinalizer {
            private final long mNativeObject;

            BuilderFinalizer(long nativeObject) {
                mNativeObject = nativeObject;
            }

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

    /**
     * Updates the bone transforms in the range [offset, offset + boneCount).
     *
     * @param engine {@link Engine} instance
     * @param matrices A {@link FloatBuffer} containing boneCount 4x4 packed matrices (i.e. 16 floats each matrix and no gap between matrices)
     * @param boneCount Number of bones to set
     * @param offset Index of the first bone to set
     */
    public void setBonesAsMatrices(@NonNull Engine engine,
            @NonNull Buffer matrices, @IntRange(from = 0, to = 255) int boneCount,
            @IntRange(from = 0) int offset) {
        int result = nSetBonesAsMatrices(mNativeObject, engine.getNativeObject(),
                matrices, matrices.remaining(), boneCount, offset);
        if (result < 0) {
            throw new BufferOverflowException();
        }
    }

    /**
     * Updates the bone transforms in the range [offset, offset + boneCount).
     *
     * @param engine {@link Engine} instance
     * @param quaternions A {@link FloatBuffer} containing boneCount transforms. Each transform consists of 8 float.
     *                    float 0 to 3 encode a unit quaternion <code>w+ix+jy+kz</code> stored as <code>x,y,z,w</code>.
     *                    float 4 to 7 encode a translation stored as <code>x,y,z,1</code>.
     * @param boneCount Number of bones to set
     * @param offset Index of the first bone to set
     */
    public void setBonesAsQuaternions(@NonNull Engine engine,
            @NonNull Buffer quaternions, @IntRange(from = 0, to = 255) int boneCount,
            @IntRange(from = 0) int offset) {
        int result = nSetBonesAsQuaternions(mNativeObject, engine.getNativeObject(),
                quaternions, quaternions.remaining(), boneCount, offset);
        if (result < 0) {
            throw new BufferOverflowException();
        }
    }

    /**
     * @return number of bones in this {@link SkinningBuffer}
     */
    public int getBoneCount() {
        return nGetBoneCount(mNativeObject);
    }

    public long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed IndexBuffer");
        }
        return mNativeObject;
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native long nCreateBuilder();
    private static native void nDestroyBuilder(long nativeBuilder);
    private static native void nBuilderBoneCount(long nativeBuilder, int boneCount);
    private static native void nBuilderInitialize(long nativeBuilder, boolean initialize);
    private static native long nBuilderBuild(long nativeBuilder, long nativeEngine);

    private static native int nSetBonesAsMatrices(long nativeObject, long nativeEngine, Buffer matrices, int remaining, int boneCount, int offset);
    private static native int nSetBonesAsQuaternions(long nativeObject, long nativeEngine, Buffer quaternions, int remaining, int boneCount, int offset);
    private static native int nGetBoneCount(long nativeObject);
}
