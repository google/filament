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
import androidx.annotation.Nullable;

import java.nio.Buffer;
import java.nio.BufferOverflowException;

/**
 * A generic GPU buffer containing data.
 *
 * Usage of this BufferObject is optional. For simple use cases it is not necessary. It is useful
 * only when you need to share data between multiple VertexBuffer instances. It also allows you to
 * efficiently swap-out the buffers in VertexBuffer.
 *
 * NOTE: For now this is only used for vertex data, but in the future we may use it for other things
 * (e.g. compute).
 *
 * @see VertexBuffer
 */
public class BufferObject {
    private long mNativeObject;

    private BufferObject(long nativeBufferObject) {
        mNativeObject = nativeBufferObject;
    }

    public static class Builder {
        @SuppressWarnings({"FieldCanBeLocal", "UnusedDeclaration"})
        // Keep to finalize native resources
        private final BuilderFinalizer mFinalizer;
        private final long mNativeBuilder;

        public enum BindingType {
            VERTEX,
        }

        public Builder() {
            mNativeBuilder = nCreateBuilder();
            mFinalizer = new BuilderFinalizer(mNativeBuilder);
        }

        /**
         * Size of the buffer in bytes.
         *
         * @param byteCount Maximum number of bytes the BufferObject can hold.
         * @return A reference to this Builder for chaining calls.
         */
        @NonNull
        public Builder size(@IntRange(from = 1) int byteCount) {
            nBuilderSize(mNativeBuilder, byteCount);
            return this;
        }

        /**
         * The binding type for this buffer object. (defaults to VERTEX)
         *
         * @param bindingType Distinguishes between SSBO, VBO, etc. For now this must be VERTEX.
         * @return A reference to this Builder for chaining calls.
         */
        @NonNull
        public Builder bindingType(@NonNull BindingType bindingType) {
            nBuilderBindingType(mNativeBuilder, bindingType.ordinal());
            return this;
        }

        /**
         * Creates and returns the <code>BufferObject</code> object. After creation, the buffer
         * is uninitialized. Use {@link #setBuffer} to initialize the <code>BufferObject</code>.
         *
         * @param engine reference to the {@link Engine} to associate this <code>BufferObject</code>
         *               with
         *
         * @return the newly created <code>BufferObject</code> object
         *
         * @exception IllegalStateException if the BufferObject could not be created
         *
         * @see #setBuffer
         */
        @NonNull
        public BufferObject build(@NonNull Engine engine) {
            long nativeBufferObject = nBuilderBuild(mNativeBuilder, engine.getNativeObject());
            if (nativeBufferObject == 0)
                throw new IllegalStateException("Couldn't create BufferObject");
            return new BufferObject(nativeBufferObject);
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
     * Returns the size of this <code>BufferObject</code> in elements.
     *
     * @return the number of bytes the <code>BufferObject</code> holds
     */
    @IntRange(from = 0)
    public int getByteCount() {
        return nGetByteCount(getNativeObject());
    }

    /**
     * Asynchronously copy-initializes this <code>BufferObject</code> from the data provided.
     *
     * @param engine            reference to the {@link Engine} to associate this
     *                          <code>BufferObject</code> with
     * @param buffer            a CPU-side {@link Buffer} with the data used to initialize the
     *                          <code>BufferObject</code>.
     */
    public void setBuffer(@NonNull Engine engine, @NonNull Buffer buffer) {
        setBuffer(engine, buffer, 0, 0, null, null);
    }

    /**
     * Asynchronously copy-initializes a region of this <code>BufferObject</code> from the data
     * provided.
     *
     * @param engine            reference to the {@link Engine} to associate this
     *                          <code>BufferObject</code> with
     * @param buffer            a CPU-side {@link Buffer} with the data used to initialize the
     *                          <code>BufferObject</code>.
     * @param destOffsetInBytes offset in <i>bytes</i> into the <code>BufferObject</code>
     * @param count             number of bytes to consume, defaults to
     *                          <code>buffer.remaining()</code>
     */
    public void setBuffer(@NonNull Engine engine, @NonNull Buffer buffer,
            @IntRange(from = 0) int destOffsetInBytes, @IntRange(from = 0) int count) {
        setBuffer(engine, buffer, destOffsetInBytes, count, null, null);
    }

    /**
     * Asynchronously copy-initializes a region of this <code>BufferObject</code> from the data
     * provided.
     *
     * @param engine            reference to the {@link Engine} to associate this
     *                          <code>BufferObject</code> with
     * @param buffer            a CPU-side {@link Buffer} with the data used to initialize the
     *                          <code>BufferObject</code>.
     * @param destOffsetInBytes offset in <i>bytes</i> into the <code>BufferObject</code>
     * @param count             number of bytes to consume, defaults to
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
            throw new IllegalStateException("Calling method on destroyed BufferObject");
        }
        return mNativeObject;
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native long nCreateBuilder();
    private static native void nDestroyBuilder(long nativeBuilder);
    private static native void nBuilderSize(long nativeBuilder, int byteCount);
    private static native void nBuilderBindingType(long nativeBuilder, int bindingType);
    private static native long nBuilderBuild(long nativeBuilder, long nativeEngine);

    private static native int nGetByteCount(long nativeBufferObject);
    private static native int nSetBuffer(long nativeBufferObject, long nativeEngine,
            Buffer buffer, int remaining,
            int destOffsetInBytes, int count, Object handler, Runnable callback);
}
