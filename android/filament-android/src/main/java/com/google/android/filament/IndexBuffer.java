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
 * A buffer containing vertex indices into a <code>VertexBuffer</code>. Indices can be 16 or 32 bit.
 * The buffer itself is a GPU resource, therefore mutating the data can be relatively slow.
 * Typically these buffers are constant.
 *
 * It is possible, and even encouraged, to use a single index buffer for several
 * <code>Renderables</code>.
 *
 * @see VertexBuffer
 * @see RenderableManager
 */
public class IndexBuffer {
    private long mNativeObject;

    private IndexBuffer(long nativeIndexBuffer) {
        mNativeObject = nativeIndexBuffer;
    }

    public static class Builder {
        @SuppressWarnings({"FieldCanBeLocal", "UnusedDeclaration"})
        // Keep to finalize native resources
        private final BuilderFinalizer mFinalizer;
        private final long mNativeBuilder;

        /**
         * Type of the index buffer.
         */
        public enum IndexType {
            /** 16-bit indices */
            USHORT,

            /** 32-bit indices */
            UINT,
        }

        public Builder() {
            mNativeBuilder = nCreateBuilder();
            mFinalizer = new BuilderFinalizer(mNativeBuilder);
        }

        /**
         * Size of the index buffer in elements.
         *
         * @param indexCount number of indices the <code>IndexBuffer</code> can hold
         *
         * @return this <code>Builder</code> object for chaining calls
         */
        @NonNull
        public Builder indexCount(@IntRange(from = 1) int indexCount) {
            nBuilderIndexCount(mNativeBuilder, indexCount);
            return this;
        }

        /**
         * Type of the index buffer, 16-bit or 32-bit.
         *
         * @param indexType type of indices stored in the <code>IndexBuffer</code>
         *
         * @return this <code>Builder</code> object for chaining calls
         */
        @NonNull
        public Builder bufferType(@NonNull IndexType indexType) {
            nBuilderBufferType(mNativeBuilder, indexType.ordinal());
            return this;
        }

        /**
         * Creates and returns the <code>IndexBuffer</code> object. After creation, the index buffer
         * is uninitialized. Use {@link #setBuffer} to initialized the <code>IndexBuffer</code>.
         *
         * @param engine reference to the {@link Engine} to associate this <code>IndexBuffer</code>
         *               with
         *
         * @return the newly created <code>IndexBuffer</code> object
         *
         * @exception IllegalStateException if the IndexBuffer could not be created
         *
         * @see #setBuffer
         */
        @NonNull
        public IndexBuffer build(@NonNull Engine engine) {
            long nativeIndexBuffer = nBuilderBuild(mNativeBuilder, engine.getNativeObject());
            if (nativeIndexBuffer == 0)
                throw new IllegalStateException("Couldn't create IndexBuffer");
            return new IndexBuffer(nativeIndexBuffer);
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
     * Returns the size of this <code>IndexBuffer</code> in elements.
     *
     * @return the number of indices the <code>IndexBuffer</code> holds
     */
    @IntRange(from = 0)
    public int getIndexCount() {
        return nGetIndexCount(getNativeObject());
    }

    /**
     * Asynchronously copy-initializes this <code>IndexBuffer</code> from the data provided.
     *
     * @param engine            reference to the {@link Engine} to associate this
     *                          <code>IndexBuffer</code> with
     * @param buffer            a CPU-side {@link Buffer} with the data used to initialize the
     *                          <code>IndexBuffer</code>. <code>buffer</code> should contain raw,
     *                          untyped data that will be interpreted as either 16-bit or 32-bits
     *                          indices based on the <code>IndexType</code> of this
     *                          <code>IndexBuffer</code>.
     */
    public void setBuffer(@NonNull Engine engine, @NonNull Buffer buffer) {
        setBuffer(engine, buffer, 0, 0, null, null);
    }

    /**
     * Asynchronously copy-initializes a region of this <code>IndexBuffer</code> from the data
     * provided.
     *
     * @param engine            reference to the {@link Engine} to associate this
     *                          <code>IndexBuffer</code> with
     * @param buffer            a CPU-side {@link Buffer} with the data used to initialize the
     *                          <code>IndexBuffer</code>. <code>buffer</code> should contain raw,
     *                          untyped data that will be interpreted as either 16-bit or 32-bits
     *                          indices based on the <code>IndexType</code> of this
     *                          <code>IndexBuffer</code>.
     * @param destOffsetInBytes offset in <i>bytes</i> into the <code>IndexBuffer</code>
     * @param count             number of buffer elements to consume, defaults to
     *                          <code>buffer.remaining()</code>
     */
    public void setBuffer(@NonNull Engine engine, @NonNull Buffer buffer,
            @IntRange(from = 0) int destOffsetInBytes, @IntRange(from = 0) int count) {
        setBuffer(engine, buffer, destOffsetInBytes, count, null, null);
    }

    /**
     * Asynchronously copy-initializes a region of this <code>IndexBuffer</code> from the data
     * provided.
     *
     * @param engine            reference to the {@link Engine} to associate this
     *                          <code>IndexBuffer</code> with
     * @param buffer            a CPU-side {@link Buffer} with the data used to initialize the
     *                          <code>IndexBuffer</code>. <code>buffer</code> should contain raw,
     *                          untyped data that will be interpreted as either 16-bit or 32-bits
     *                          indices based on the <code>IndexType</code> of this
     *                          <code>IndexBuffer</code>.
     * @param destOffsetInBytes offset in <i>bytes</i> into the <code>IndexBuffer</code>
     * @param count             number of buffer elements to consume, defaults to
     *                          <code>buffer.remaining()</code>
     * @param handler           an {@link java.util.concurrent.Executor Executor}. On Android this
     *                          can also be a {@link android.os.Handler Handler}.
     * @param callback          a callback executed by <code>handler</code> when <code>buffer</code>
     *                          is no longer needed.
     */
    public void setBuffer(@NonNull Engine engine, @NonNull Buffer buffer,
            @IntRange(from = 0) int destOffsetInBytes, @IntRange(from = 0) int count,
            @Nullable Object handler, @Nullable Runnable callback) {
        int result = nSetBuffer(getNativeObject(), engine.getNativeObject(), buffer, buffer.remaining(),
                destOffsetInBytes, count == 0 ? buffer.remaining() : count, handler, callback);
        if (result < 0) {
            throw new BufferOverflowException();
        }
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
    private static native void nBuilderIndexCount(long nativeBuilder, int indexCount);
    private static native void nBuilderBufferType(long nativeBuilder, int indexType);
    private static native long nBuilderBuild(long nativeBuilder, long nativeEngine);

    private static native int nGetIndexCount(long nativeIndexBuffer);
    private static native int nSetBuffer(long nativeIndexBuffer, long nativeEngine,
            Buffer buffer, int remaining,
            int destOffsetInBytes, int count, Object handler, Runnable callback);
}
