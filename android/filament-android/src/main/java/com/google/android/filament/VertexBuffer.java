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

import java.nio.Buffer;
import java.nio.BufferOverflowException;

public class VertexBuffer {
    private long mNativeObject;

    private VertexBuffer(long nativeVertexBuffer) {
        mNativeObject = nativeVertexBuffer;
    }

    public enum VertexAttribute {
        POSITION, // XYZ position (float3)
        TANGENTS, // tangent, bitangent and normal, encoded as a quaternion (4 floats or half floats)
        COLOR, // vertex color (float4)
        UV0, // texture coordinates (float2)
        UV1, // texture coordinates (float2)
        BONE_INDICES, // indices of 4 bones (uvec4)
        BONE_WEIGHTS    // weights of the 4 bones (normalized float4)
    }

    public enum AttributeType {
        BYTE,
        BYTE2,
        BYTE3,
        BYTE4,
        UBYTE,
        UBYTE2,
        UBYTE3,
        UBYTE4,
        SHORT,
        SHORT2,
        SHORT3,
        SHORT4,
        USHORT,
        USHORT2,
        USHORT3,
        USHORT4,
        INT,
        UINT,
        FLOAT,
        FLOAT2,
        FLOAT3,
        FLOAT4,
        HALF,
        HALF2,
        HALF3,
        HALF4,
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
        public Builder vertexCount(@IntRange(from = 1) int vertexCount) {
            nBuilderVertexCount(mNativeBuilder, vertexCount);
            return this;
        }

        @NonNull
        public Builder bufferCount(@IntRange(from = 1) int bufferCount) {
            nBuilderBufferCount(mNativeBuilder, bufferCount);
            return this;
        }

        @NonNull
        public Builder attribute(@NonNull VertexAttribute attribute,
                @IntRange(from = 0) int bufferIndex, @NonNull AttributeType attributeType,
                @IntRange(from = 0) int byteOffset, @IntRange(from = 0) int byteStride) {
            nBuilderAttribute(mNativeBuilder, attribute.ordinal(), bufferIndex,
                    attributeType.ordinal(), byteOffset, byteStride);
            return this;
        }

        @NonNull
        public Builder attribute(@NonNull VertexAttribute attribute,
                @IntRange(from = 0) int bufferIndex, @NonNull AttributeType attributeType) {
            return attribute(attribute, bufferIndex, attributeType, 0, 0 );
        }

        @NonNull
        public Builder normalized(@NonNull VertexAttribute attribute) {
            nBuilderNormalized(mNativeBuilder, attribute.ordinal());
            return this;
        }

        @NonNull
        public VertexBuffer build(@NonNull Engine engine) {
            long nativeVertexBuffer = nBuilderBuild(mNativeBuilder, engine.getNativeObject());
            if (nativeVertexBuffer == 0) throw new IllegalStateException("Couldn't create VertexBuffer");
            return new VertexBuffer(nativeVertexBuffer);
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

    @IntRange(from = 0)
    public int getVertexCount() {
        return nGetVertexCount(getNativeObject());
    }

    public void setBufferAt(@NonNull Engine engine, int bufferIndex, @NonNull Buffer buffer) {
        setBufferAt(engine, bufferIndex, buffer, 0, 0, null, null);
    }

    public void setBufferAt(@NonNull Engine engine, int bufferIndex, @NonNull Buffer buffer,
            @IntRange(from = 0) int destOffsetInBytes, @IntRange(from = 0) int count) {
        setBufferAt(engine, bufferIndex, buffer, destOffsetInBytes, count, null, null);
    }

    /**
     * Valid handler types:
     * - Android: Handler, Executor
     * - Other: Executor
     */
    public void setBufferAt(@NonNull Engine engine, int bufferIndex, @NonNull Buffer buffer,
            @IntRange(from = 0) int destOffsetInBytes, @IntRange(from = 0) int count,
            @Nullable Object handler, @Nullable Runnable callback) {
        int result = nSetBufferAt(getNativeObject(), engine.getNativeObject(), bufferIndex,
                buffer, buffer.remaining(),
                destOffsetInBytes, count == 0 ? buffer.remaining() : count, handler, callback);
        if (result < 0) {
            throw new BufferOverflowException();
        }
    }

    long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed VertexBuffer");
        }
        return mNativeObject;
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native long nCreateBuilder();
    private static native void nDestroyBuilder(long nativeBuilder);
    private static native void nBuilderVertexCount(long nativeBuilder, int vertexCount);
    private static native void nBuilderBufferCount(long nativeBuilder, int bufferCount);
    private static native void nBuilderAttribute(long nativeBuilder, int attribute,
            int bufferIndex, int attributeType, int byteOffset, int byteStride);
    private static native void nBuilderNormalized(long nativeBuilder, int attribute);
    private static native long nBuilderBuild(long nativeBuilder, long nativeEngine);

    private static native int nGetVertexCount(long nativeVertexBuffer);
    private static native int nSetBufferAt(long nativeVertexBuffer, long nativeEngine,
            int bufferIndex, Buffer buffer, int remaining, int destOffsetInBytes, int count,
            Object handler, Runnable callback);
}
