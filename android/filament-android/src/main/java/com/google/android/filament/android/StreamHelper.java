/*
 * Copyright (C) 2026 The Android Open Source Project
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

package com.google.android.filament.android;

import android.graphics.SurfaceTexture;
import android.hardware.HardwareBuffer;
import android.os.Build;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;
import androidx.annotation.Size;

import com.google.android.filament.Stream;

public final class StreamHelper {
    private StreamHelper() {}

    /**
     * Associates a {@link SurfaceTexture} with a {@link Stream.Builder} to create a NATIVE stream.
     *
     * @param builder The Stream.Builder to configure.
     * @param surfaceTexture The SurfaceTexture to use as the stream source.
     * @return The same builder instance for method chaining.
     */
    @NonNull
    public static Stream.Builder setStreamSource(@NonNull Stream.Builder builder,
            @NonNull SurfaceTexture surfaceTexture) {
        nSetStreamSource(builder, surfaceTexture);
        return new Stream.Builder() {
            @NonNull
            @Override
            public Stream.Builder width(int width) {
                builder.width(width);
                return this;
            }

            @NonNull
            @Override
            public Stream.Builder height(int height) {
                builder.height(height);
                return this;
            }

            @NonNull
            @Override
            public Stream.Builder name(@NonNull String name) {
                builder.name(name);
                return this;
            }

            @NonNull
            @Override
            public Stream build(@NonNull com.google.android.filament.Engine engine) {
                try {
                    return builder.build(engine);
                } finally {
                    nClearStreamSource(builder);
                }
            }
        };
    }

    /**
     * Updates an ACQUIRED stream with a HardwareBuffer image.
     *
     * @param stream The Stream to update.
     * @param hwbuffer The HardwareBuffer to acquire.
     * @param handler An Executor or Handler to run the release callback on.
     * @param callback A callback invoked when the buffer is released by Filament.
     */
    @RequiresApi(Build.VERSION_CODES.O)
    public static void setAcquiredImage(@NonNull Stream stream,
            @NonNull HardwareBuffer hwbuffer,
            @Nullable Object handler,
            @NonNull Runnable callback) {
        setAcquiredImage(stream, hwbuffer, handler, callback, null);
    }

    /**
     * Updates an ACQUIRED stream with a HardwareBuffer image and an optional 3x3 transform matrix.
     *
     * @param stream The Stream to update.
     * @param hwbuffer The HardwareBuffer to acquire.
     * @param handler An Executor or Handler to run the release callback on.
     * @param callback A callback invoked when the buffer is released by Filament.
     * @param transform An optional 3x3 column-major transform matrix.
     */
    @RequiresApi(Build.VERSION_CODES.O)
    public static void setAcquiredImage(@NonNull Stream stream,
            @NonNull HardwareBuffer hwbuffer,
            @Nullable Object handler,
            @NonNull Runnable callback,
            @Nullable @Size(min = 9) float[] transform) {
        if (transform != null && transform.length < 9) {
            throw new IllegalArgumentException("transform must have at least 9 elements");
        }
        nSetAcquiredImage(stream.getNativeObject(), hwbuffer, handler, callback, transform);
    }

    private static native void nSetStreamSource(Stream.Builder builder, SurfaceTexture surfaceTexture);
    private static native void nClearStreamSource(Stream.Builder builder);
    private static native void nSetAcquiredImage(long nativeStream, HardwareBuffer hwbuffer,
            Object handler, Runnable callback, float[] transform);
}
