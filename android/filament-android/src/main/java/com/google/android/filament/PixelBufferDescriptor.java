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

package com.google.android.filament;

import androidx.annotation.IntRange;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.nio.Buffer;
import java.nio.ByteBuffer;

/**
 * A descriptor to an image in main memory, typically used to transfer image data from the CPU to the GPU.
 *
 * <p>A PixelBufferDescriptor owns the memory buffer it references, therefore PixelBufferDescriptor
 * cannot be copied, but can be moved.</p>
 *
 * <p>PixelBufferDescriptor releases ownership of the memory-buffer when it's destroyed.</p>
 */
public class PixelBufferDescriptor {
    public Buffer storage;
    public int left = 0;
    public int top = 0;
    public int stride = 0;
    public Texture.Format format;
    public Texture.Type type;
    public int alignment = 1;
    public int compressedSizeInBytes;
    public Texture.CompressedType compressedFormat;
    @Nullable public Object handler;
    @Nullable public Runnable callback;

    /**
     * Creates a {@code PixelBufferDescriptor}.
     *
     * @param storage   CPU-side buffer containing the image data to upload into the texture
     * @param format    Pixel {@link Texture.Format format} of the CPU-side image
     * @param type      Pixel data {@link Texture.Type type} of the CPU-side image
     * @param alignment Row-alignment in bytes of the CPU-side image (1 to 8 bytes)
     * @param left      Left coordinate in pixels of the CPU-side image
     * @param top       Top coordinate in pixels of the CPU-side image
     * @param stride    Stride in pixels of the CPU-side image
     * @param handler   An {@link java.util.concurrent.Executor Executor} or Android Handler
     * @param callback  A callback executed by {@code handler} when {@code storage} is no longer needed
     */
    public PixelBufferDescriptor(@NonNull Buffer storage,
            @NonNull Texture.Format format, @NonNull Texture.Type type,
            @IntRange(from = 1, to = 8) int alignment,
            @IntRange(from = 0) int left, @IntRange(from = 0) int top,
            @IntRange(from = 0) int stride,
            @Nullable Object handler, @Nullable Runnable callback) {
        this.storage = storage;
        this.left = left;
        this.top = top;
        this.type = type;
        this.alignment = alignment;
        this.stride = stride;
        this.format = format;
        this.handler = handler;
        this.callback = callback;
    }

    /**
     * Creates a {@code PixelBufferDescriptor} with default alignment (1) and offsets (0), without callback.
     */
    public PixelBufferDescriptor(@NonNull Buffer storage,
            @NonNull Texture.Format format, @NonNull Texture.Type type) {
        this(storage, format, type, 1, 0, 0, 0, null, null);
    }

    /**
     * Creates a {@code PixelBufferDescriptor} with specified alignment and default offsets (0), without callback.
     */
    public PixelBufferDescriptor(@NonNull Buffer storage,
            @NonNull Texture.Format format, @NonNull Texture.Type type,
            @IntRange(from = 1, to = 8) int alignment) {
        this(storage, format, type, alignment, 0, 0, 0, null, null);
    }

    /**
     * Creates a {@code PixelBufferDescriptor} with specified alignment and left/top coordinates, without callback.
     */
    public PixelBufferDescriptor(@NonNull Buffer storage,
            @NonNull Texture.Format format, @NonNull Texture.Type type,
            @IntRange(from = 1, to = 8) int alignment,
            @IntRange(from = 0) int left, @IntRange(from = 0) int top) {
        this(storage, format, type, alignment, left, top, 0, null, null);
    }

    /**
     * Creates a {@code PixelBufferDescriptor} referencing compressed image data in main memory.
     *
     * @param storage               CPU-side buffer containing the image data to upload into the texture
     * @param format                Compressed pixel {@link Texture.CompressedType format} of the CPU-side image
     * @param compressedSizeInBytes Size of the compressed data in bytes
     */
    public PixelBufferDescriptor(@NonNull ByteBuffer storage,
            @NonNull Texture.CompressedType format,
            @IntRange(from = 0) int compressedSizeInBytes) {
        this.storage = storage;
        this.type = Texture.Type.COMPRESSED;
        this.alignment = 1;
        this.compressedFormat = format;
        this.compressedSizeInBytes = compressedSizeInBytes;
    }

    /**
     * Set or replace the callback called when the CPU-side data is no longer needed.
     */
    public void setCallback(@Nullable Object handler, @Nullable Runnable callback) {
        this.handler = handler;
        this.callback = callback;
    }

    /**
     * Helper to calculate the buffer size (in bytes) needed for given parameters.
     */
    public static int computeDataSize(@NonNull Texture.Format format, @NonNull Texture.Type type,
            int stride, int height, @IntRange(from = 1, to = 8) int alignment) {
        return nComputeDataSize(format.ordinal(), type.ordinal(), stride, height, alignment);
    }

    private static native int nComputeDataSize(int format, int type, int stride, int height, int alignment);
}
