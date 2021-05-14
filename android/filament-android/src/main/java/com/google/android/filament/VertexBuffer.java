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

import androidx.annotation.IntRange;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.nio.Buffer;
import java.nio.BufferOverflowException;

/**
 * Holds a set of buffers that define the geometry of a <code>Renderable</code>.
 *
 * <p>
 * The geometry of the <code>Renderable</code> itself is defined by a set of vertex attributes such as
 * position, color, normals, tangents, etc...
 * </p>
 *
 * <p>
 * There is no need to have a 1-to-1 mapping between attributes and buffer. A buffer can hold the
 * data of several attributes -- attributes are then referred as being "interleaved".
 * </p>
 *
 * <p>
 * The buffers themselves are GPU resources, therefore mutating their data can be relatively slow.
 * For this reason, it is best to separate the constant data from the dynamic data into multiple
 * buffers.
 * </p>
 *
 * <p>
 * It is possible, and even encouraged, to use a single vertex buffer for several
 * <code>Renderable</code>s.
 * </p>
 *
 * @see IndexBuffer
 * @see RenderableManager
 */
public class VertexBuffer {
    private long mNativeObject;

    private VertexBuffer(long nativeVertexBuffer) {
        mNativeObject = nativeVertexBuffer;
    }

    public enum VertexAttribute {
        POSITION,     // XYZ position (float3)
        TANGENTS,     // tangent, bitangent and normal, encoded as a quaternion (4 floats or half floats)
        COLOR,        // vertex color (float4)
        UV0,          // texture coordinates (float2)
        UV1,          // texture coordinates (float2)
        BONE_INDICES, // indices of 4 bones (uvec4)
        BONE_WEIGHTS, // weights of the 4 bones (normalized float4)
        UNUSED,       // reserved for future use
        CUSTOM0,      // custom or MORPH_POSITION_0
        CUSTOM1,      // custom or MORPH_POSITION_1
        CUSTOM2,      // custom or MORPH_POSITION_2
        CUSTOM3,      // custom or MORPH_POSITION_3
        CUSTOM4,      // custom or MORPH_TANGENTS_0
        CUSTOM5,      // custom or MORPH_TANGENTS_1
        CUSTOM6,      // custom or MORPH_TANGENTS_2
        CUSTOM7       // custom or MORPH_TANGENTS_3
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

        /**
         * Size of each buffer in this set, expressed in in number of vertices.
         *
         * @param vertexCount number of vertices in each buffer in this set
         *
         * @return A reference to this Builder for chaining calls.
         */
        @NonNull
        public Builder vertexCount(@IntRange(from = 1) int vertexCount) {
            nBuilderVertexCount(mNativeBuilder, vertexCount);
            return this;
        }

        /**
         * Allows buffers to be swapped out and shared using BufferObject.
         *
         * If buffer objects mode is enabled, clients must call setBufferObjectAt rather than
         * setBufferAt. This allows sharing of data between VertexBuffer objects, but it may
         * slightly increase the memory footprint of Filament's internal bookkeeping.
         *
         * @param enabled If true, enables buffer object mode.  False by default.
         */
        @NonNull
        public Builder enableBufferObjects(boolean enabled) {
            nBuilderEnableBufferObjects(mNativeBuilder, enabled);
            return this;
        }

        /**
         * Defines how many buffers will be created in this vertex buffer set. These buffers are
         * later referenced by index from 0 to <code>bufferCount</code> - 1.
         *
         * This call is mandatory. The default is 0.
         *
         * @param bufferCount number of buffers in this vertex buffer set. The maximum value is 8.
         *
         * @return this <code>Builder</code> for chaining calls
         */
        @NonNull
        public Builder bufferCount(@IntRange(from = 1) int bufferCount) {
            nBuilderBufferCount(mNativeBuilder, bufferCount);
            return this;
        }

        /**
         * Sets up an attribute for this vertex buffer set.
         *
         * Using <code>byteOffset</code> and <code>byteStride</code>, attributes can be interleaved
         * in the same buffer.
         *
         * <p>
         * This is a no-op if the <code>attribute</code> is an invalid enum.
         * This is a no-op if the <code>bufferIndex</code> is out of bounds.
         * </p>
         *
         * <p>
         * Warning: <code>VertexAttribute.TANGENTS</code> must be specified as a quaternion and is
         * how normals are specified.
         * </p>
         *
         * @param attribute     the attribute to set up
         * @param bufferIndex   the index of the buffer containing the data for this attribute. Must
         *                      be between 0 and bufferCount() - 1.
         * @param attributeType the type of the attribute data (e.g. byte, float3, etc...)
         * @param byteOffset    offset in <i>bytes</i> into the buffer <code>bufferIndex</code>
         * @param byteStride    stride in <i>bytes</i> to the next element of this attribute. When
         *                      set to zero the attribute size, as defined by
         *                      <code>attributeType</code> is used.
         *
         * @return A reference to this <code>Builder</code> for chaining calls.
         *
         * @see VertexAttribute
         */
        @NonNull
        public Builder attribute(@NonNull VertexAttribute attribute,
                @IntRange(from = 0) int bufferIndex, @NonNull AttributeType attributeType,
                @IntRange(from = 0) int byteOffset, @IntRange(from = 0) int byteStride) {
            nBuilderAttribute(mNativeBuilder, attribute.ordinal(), bufferIndex,
                    attributeType.ordinal(), byteOffset, byteStride);
            return this;
        }

        /**
         * Sets up an attribute for this vertex buffer set.
         *
         * Using <code>byteOffset</code> and <code>byteStride</code>, attributes can be interleaved
         * in the same buffer.
         *
         * <p>
         * This is a no-op if the <code>attribute</code> is an invalid enum.
         * This is a no-op if the <code>bufferIndex</code> is out of bounds.
         * </p>
         *
         * <p>
         * Warning: <code>VertexAttribute.TANGENTS</code> must be specified as a quaternion and is
         * how normals are specified.
         * </p>
         *
         * @param attribute     the attribute to set up
         * @param bufferIndex   the index of the buffer containing the data for this attribute. Must
         *                      be between 0 and bufferCount() - 1.
         * @param attributeType the type of the attribute data (e.g. byte, float3, etc...)
         *
         * @return A reference to this <code>Builder</code> for chaining calls.
         *
         * @see VertexAttribute
         */
        @NonNull
        public Builder attribute(@NonNull VertexAttribute attribute,
                @IntRange(from = 0) int bufferIndex, @NonNull AttributeType attributeType) {
            return attribute(attribute, bufferIndex, attributeType, 0, 0 );
        }

        /**
         * Sets whether a given attribute should be normalized. By default attributes are not
         * normalized. A normalized attribute is mapped between 0 and 1 in the shader. This applies
         * only to integer types.
         *
         * @param attribute enum of the attribute to set the normalization flag to
         *
         * @return this <code>Builder</code> object for chaining calls.
         *
         * This is a no-op if the <code>attribute</code> is an invalid enum.
         */
        @NonNull
        public Builder normalized(@NonNull VertexAttribute attribute) {
            nBuilderNormalized(mNativeBuilder, attribute.ordinal(), true);
            return this;
        }

        /**
         * Sets whether a given attribute should be normalized. By default attributes are not
         * normalized. A normalized attribute is mapped between 0 and 1 in the shader. This applies
         * only to integer types.
         *
         * @param attribute enum of the attribute to set the normalization flag to
         * @param enabled   true to automatically normalize the given attribute
         *
         * @return this <code>Builder</code> object for chaining calls.
         *
         * This is a no-op if the <code>attribute</code> is an invalid enum.
         */
        @NonNull
        public Builder normalized(@NonNull VertexAttribute attribute, boolean enabled) {
            nBuilderNormalized(mNativeBuilder, attribute.ordinal(), enabled);
            return this;
        }

        /**
         * Creates the <code>VertexBuffer</code> object and returns a pointer to it.
         *
         * @param engine reference to the {@link Engine} to associate this <code>VertexBuffer</code>
         *               with
         *
         * @return the newly created <code>VertexBuffer</code> object
         *
         * @exception IllegalStateException if the VertexBuffer could not be created
         */
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

    /**
     * Returns the vertex count.
     *
     * @return number of vertices in this vertex buffer set
     */
    @IntRange(from = 0)
    public int getVertexCount() {
        return nGetVertexCount(getNativeObject());
    }

    /**
     * Asynchronously copy-initializes the specified buffer from the given buffer data.
     *
     * @param engine            reference to the {@link Engine} to associate this
     *                          <code>VertexBuffer</code> with
     * @param bufferIndex       index of the buffer to initialize. Must be between 0 and
     *                          bufferCount() - 1.
     * @param buffer            a CPU-side {@link Buffer} representing the data used to initialize
     *                          the <code>VertexBuffer</code> at index <code>bufferIndex</code>.
     *                          <code>buffer</code> should contain raw, untyped data that will
     *                          be copied as-is into the buffer.
     */
    public void setBufferAt(@NonNull Engine engine, int bufferIndex, @NonNull Buffer buffer) {
        setBufferAt(engine, bufferIndex, buffer, 0, 0, null, null);
    }

    /**
     * Asynchronously copy-initializes a region of the specified buffer from the given buffer data.
     *
     * @param engine            reference to the {@link Engine} to associate this
     *                          <code>VertexBuffer</code> with
     * @param bufferIndex       index of the buffer to initialize. Must be between 0 and
     *                          bufferCount() - 1.
     * @param buffer            a CPU-side {@link Buffer} representing the data used to initialize
     *                          the <code>VertexBuffer</code> at index <code>bufferIndex</code>.
     *                          <code>buffer</code> should contain raw, untyped data that will
     *                          be copied as-is into the buffer.
     * @param destOffsetInBytes offset in <i>bytes</i> into the buffer at index
     *                          <code>bufferIndex</code> of this vertex buffer set.
     */
    public void setBufferAt(@NonNull Engine engine, int bufferIndex, @NonNull Buffer buffer,
            @IntRange(from = 0) int destOffsetInBytes, @IntRange(from = 0) int count) {
        setBufferAt(engine, bufferIndex, buffer, destOffsetInBytes, count, null, null);
    }

    /**
     * Asynchronously copy-initializes a region of the specified buffer from the given buffer data.
     *
     * @param engine            reference to the {@link Engine} to associate this
     *                          <code>VertexBuffer</code> with
     * @param bufferIndex       index of the buffer to initialize. Must be between 0 and
     *                          bufferCount() - 1.
     * @param buffer            a CPU-side {@link Buffer} representing the data used to initialize
     *                          the <code>VertexBuffer</code> at index <code>bufferIndex</code>.
     *                          <code>buffer</code> should contain raw, untyped data that will
     *                          be copied as-is into the buffer.
     * @param destOffsetInBytes offset in <i>bytes</i> into the buffer at index
     *                          <code>bufferIndex</code> of this vertex buffer set.
     * @param handler           an {@link java.util.concurrent.Executor Executor}. On Android this
     *                          can also be a {@link android.os.Handler Handler}.
     * @param callback          a callback executed by <code>handler</code> when <code>buffer</code>
     *                          is no longer needed.
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

    /**
     * Swaps in the given buffer object.
     *
     * To use this, you must first call enableBufferObjects() on the Builder.
     *
     * @param engine Reference to the filament::Engine to associate this VertexBuffer with.
     * @param bufferIndex Index of the buffer to initialize. Must be between 0
     *                    and Builder::bufferCount() - 1.
     * @param bufferObject The handle to the GPU data that will be used in this buffer slot.
     */
    public void setBufferObjectAt(@NonNull Engine engine, int bufferIndex,
            @NonNull BufferObject bufferObject) {
        nSetBufferObjectAt(getNativeObject(), engine.getNativeObject(), bufferIndex,
                bufferObject.getNativeObject());
    }

    public long getNativeObject() {
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
    private static native void nBuilderEnableBufferObjects(long nativeBuilder, boolean enabled);
    private static native void nBuilderBufferCount(long nativeBuilder, int bufferCount);
    private static native void nBuilderAttribute(long nativeBuilder, int attribute,
            int bufferIndex, int attributeType, int byteOffset, int byteStride);
    private static native void nBuilderNormalized(long nativeBuilder, int attribute,
            boolean normalized);
    private static native long nBuilderBuild(long nativeBuilder, long nativeEngine);

    private static native int nGetVertexCount(long nativeVertexBuffer);
    private static native int nSetBufferAt(long nativeVertexBuffer, long nativeEngine,
            int bufferIndex, Buffer buffer, int remaining, int destOffsetInBytes, int count,
            Object handler, Runnable callback);

    private static native void nSetBufferObjectAt(long nativeVertexBuffer, long nativeEngine,
            int bufferIndex, long nativeBufferObject);
}
