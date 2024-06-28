/*
 * Copyright (C) 2022 The Android Open Source Project
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

public class MorphTargetBuffer {
    private long mNativeObject;
    private MorphTargetBuffer(long nativeMorphTargetBuffer) {
        mNativeObject = nativeMorphTargetBuffer;
    }

    public static class Builder {
        @SuppressWarnings({"FieldCanBeLocal", "UnusedDeclaration"})
        // Keep to finalize native resources
        private final MorphTargetBuffer.Builder.BuilderFinalizer mFinalizer;
        private final long mNativeBuilder;

        public Builder() {
            mNativeBuilder = nCreateBuilder();
            mFinalizer = new MorphTargetBuffer.Builder.BuilderFinalizer(mNativeBuilder);
        }

        /**
         * Size of the morph targets in vertex counts.
         *
         * @param vertexCount Number of vertex counts the morph targets can hold.
         * @return this <code>Builder</code> object for chaining calls
         */
        @NonNull
        public Builder vertexCount(@IntRange(from = 1) int vertexCount) {
            nBuilderVertexCount(mNativeBuilder, vertexCount);
            return this;
        }

        /**
         * Size of the morph targets in targets.
         *
         * @param count Number of targets the morph targets can hold.
         * @return this <code>Builder</code> object for chaining calls
         */
        @NonNull
        public Builder count(@IntRange(from = 1) int count) {
            nBuilderCount(mNativeBuilder, count);
            return this;
        }

        /**
         * Creates and returns the <code>MorphTargetBuffer</code> object.
         *
         * @param engine reference to the {@link Engine} to associate this <code>MorphTargetBuffer</code>
         *               with.
         *
         * @return the newly created <code>MorphTargetBuffer</code> object
         *
         * @exception IllegalStateException if the MorphTargetBuffer could not be created
         *
         * @see #setMorphTargetBufferOffsetAt
         */
        @NonNull
        public MorphTargetBuffer build(@NonNull Engine engine) {
            long nativeMorphTargetBuffer = nBuilderBuild(mNativeBuilder, engine.getNativeObject());
            if (nativeMorphTargetBuffer == 0)
                throw new IllegalStateException("Couldn't create MorphTargetBuffer");
            return new MorphTargetBuffer(nativeMorphTargetBuffer);
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
     * Updates float4 positions for the given morph target.
     *
     * @param engine {@link Engine} instance
     * @param targetIndex The index of morph target to be updated
     * @param positions An array with at least count*4 floats
     * @param count Number of float4 vectors in positions to be consumed
     */
    public void setPositionsAt(@NonNull Engine engine,
            @IntRange(from = 0) int targetIndex,
            @NonNull float[] positions, @IntRange(from = 0, to = 125) int count) {
        int result = nSetPositionsAt(mNativeObject, engine.getNativeObject(), targetIndex,
                positions, count);
        if (result < 0) {
            throw new BufferOverflowException();
        }
    }

    /**
     * Updates tangents for the given morph target.
     *
     * These quaternions must be represented as signed shorts, where real numbers in the [-1,+1]
     * range multiplied by 32767.
     *
     * @param engine {@link Engine} instance
     * @param targetIndex The index of morph target to be updated
     * @param tangents An array with at least "count*4" shorts
     * @param count number of short4 quaternions in tangents
     */
    public void setTangentsAt(@NonNull Engine engine,
            @IntRange(from = 0) int targetIndex,
            @NonNull short[] tangents, @IntRange(from = 0, to = 125) int count) {
        int result = nSetTangentsAt(mNativeObject, engine.getNativeObject(), targetIndex,
                tangents, count);
        if (result < 0) {
            throw new BufferOverflowException();
        }
    }

    /**
     * @return number of vertices in this {@link MorphTargetBuffer}
     */
    public int getVertexCount() {
        return nGetVertexCount(mNativeObject);
    }

    /**
     * @return number of morph targets in this {@link MorphTargetBuffer}
     */
    public int getCount() {
        return nGetCount(mNativeObject);
    }

    public long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed MorphTargetBuffer");
        }
        return mNativeObject;
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native long nCreateBuilder();
    private static native void nDestroyBuilder(long nativeBuilder);
    private static native void nBuilderVertexCount(long nativeBuilder, int vertexCount);
    private static native void nBuilderCount(long nativeBuilder, int count);
    private static native long nBuilderBuild(long nativeBuilder, long nativeEngine);

    private static native int nSetPositionsAt(long nativeObject, long nativeEngine, int targetIndex, float[] positions, int count);
    private static native int nSetTangentsAt(long nativeObject, long nativeEngine, int targetIndex, short[] tangents, int count);
    private static native int nGetVertexCount(long nativeObject);
    private static native int nGetCount(long nativeObject);
}
