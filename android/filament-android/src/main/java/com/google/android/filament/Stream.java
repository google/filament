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

import java.nio.Buffer;
import java.nio.BufferOverflowException;
import java.nio.ReadOnlyBufferException;

public class Stream {
    private long mNativeObject;
    private long mNativeEngine;

    Stream(long nativeStream, Engine engine) {
        mNativeObject = nativeStream;
        mNativeEngine = engine.getNativeObject();
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
         * Accepted types for the stream source:
         * - Android: SurfaceView
         * - Other: none
         */
        @NonNull
        public Builder stream(@NonNull Object streamSource) {
            if (Platform.get().validateStreamSource(streamSource)) {
                nBuilderStreamSource(mNativeBuilder, streamSource);
                return this;
            }
            throw new IllegalArgumentException("Invalid stream source: " + streamSource);
        }

        @NonNull
        public Builder stream(long externalTextureId) {
            nBuilderStream(mNativeBuilder, externalTextureId);
            return this;
        }

        @NonNull
        public Builder width(int width) {
            nBuilderWidth(mNativeBuilder, width);
            return this;
        }

        @NonNull
        public Builder height(int height) {
            nBuilderHeight(mNativeBuilder, height);
            return this;
        }

        @NonNull
        public Stream build(@NonNull Engine engine) {
            long nativeStream = nBuilderBuild(mNativeBuilder, engine.getNativeObject());
            if (nativeStream == 0) throw new IllegalStateException("Couldn't create Stream");
            return new Stream(nativeStream, engine);
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

    public boolean isNative() {
        return nIsNative(getNativeObject());
    }

    public void setDimensions(@IntRange(from = 0) int width, @IntRange(from = 0) int height) {
        nSetDimensions(getNativeObject(), width, height);
    }

    public void readPixels(
            @IntRange(from = 0) int xoffset, @IntRange(from = 0) int yoffset,
            @IntRange(from = 0) int width, @IntRange(from = 0) int height,
            @NonNull Texture.PixelBufferDescriptor buffer) {

        if (buffer.storage.isReadOnly()) {
            throw new ReadOnlyBufferException();
        }

        int result = nReadPixels(getNativeObject(), mNativeEngine,
                xoffset, yoffset, width, height,
                buffer.storage, buffer.storage.remaining(),
                buffer.left, buffer.top, buffer.type.ordinal(), buffer.alignment,
                buffer.stride, buffer.format.ordinal(),
                buffer.handler, buffer.callback);

        if (result < 0) {
            throw new BufferOverflowException();
        }
    }

    public long getTimestamp() {
        return nGetTimestamp(getNativeObject());
    }

    public long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed Stream");
        }
        return mNativeObject;
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native long nCreateBuilder();
    private static native void nDestroyBuilder(long nativeStreamBuilder);
    private static native void nBuilderStreamSource(long nativeStreamBuilder, Object streamSource);
    private static native void nBuilderStream(long nativeStreamBuilder, long externalTextureId);
    private static native void nBuilderWidth(long nativeStreamBuilder, int width);
    private static native void nBuilderHeight(long nativeStreamBuilder, int height);
    private static native long nBuilderBuild(long nativeStreamBuilder, long nativeEngine);

    private static native void nSetDimensions(long nativeStream, int width, int height);
    private static native int nReadPixels(long nativeStream, long nativeEngine,
            int xoffset, int yoffset, int width, int height,
            Buffer storage, int remaining,
            int left, int top, int type, int alignment, int stride, int format,
            Object handler, Runnable callback);
    private static native long nGetTimestamp(long nativeStream);

    private static native boolean nIsNative(long nativeStream);
}
