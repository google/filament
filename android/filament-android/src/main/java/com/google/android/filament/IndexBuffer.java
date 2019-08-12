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

        public enum IndexType {
            USHORT,
            UINT,
        }

        public Builder() {
            mNativeBuilder = nCreateBuilder();
            mFinalizer = new BuilderFinalizer(mNativeBuilder);
        }

        @NonNull
        public Builder indexCount(@IntRange(from = 1) int indexCount) {
            nBuilderIndexCount(mNativeBuilder, indexCount);
            return this;
        }

        @NonNull
        public Builder bufferType(@NonNull IndexType indexType) {
            nBuilderBufferType(mNativeBuilder, indexType.ordinal());
            return this;
        }

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

    @IntRange(from = 0)
    public int getIndexCount() {
        return nGetIndexCount(getNativeObject());
    }

    public void setBuffer(@NonNull Engine engine, @NonNull Buffer buffer) {
        setBuffer(engine, buffer, 0, 0, null, null);
    }

    public void setBuffer(@NonNull Engine engine, @NonNull Buffer buffer,
            @IntRange(from = 0) int destOffsetInBytes, @IntRange(from = 0) int count) {
        setBuffer(engine, buffer, destOffsetInBytes, count, null, null);
    }

    /**
     * Valid handler types:
     * - Android: Handler, Executor
     * - Other: Executor
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
