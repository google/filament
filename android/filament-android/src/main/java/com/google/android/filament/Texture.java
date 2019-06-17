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

import com.google.android.filament.proguard.UsedByReflection;

import android.support.annotation.IntRange;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.annotation.Size;

import java.nio.Buffer;
import java.nio.BufferOverflowException;
import java.nio.ByteBuffer;

import static com.google.android.filament.Texture.Type.COMPRESSED;

public class Texture {
    private long mNativeObject;

    @UsedByReflection("KtxLoader.java")
    Texture(long nativeTexture) {
        mNativeObject = nativeTexture;
    }

    public enum Sampler {
        SAMPLER_2D,
        SAMPLER_CUBEMAP,
        SAMPLER_EXTERNAL
    }

    public enum InternalFormat {
        // 8-bits per element
        R8, R8_SNORM, R8UI, R8I, STENCIL8,

        // 16-bits per element
        R16F, R16UI, R16I,
        RG8, RG8_SNORM, RG8UI, RG8I,
        RGB565, RGB9_E5, RGB5_A1,
        RGBA4,
        DEPTH16,

        // 24-bits per element
        RGB8, SRGB8, RGB8_SNORM, RGB8UI, RGB8I,
        DEPTH24,

        // 32-bits per element
        R32F, R32UI, R32I,
        RG16F, RG16UI, RG16I,
        R11F_G11F_B10F,
        RGBA8, SRGB8_A8, RGBA8_SNORM,
        UNUSED, // used to be rgbm
        RGB10_A2, RGBA8UI, RGBA8I,
        DEPTH32F, DEPTH24_STENCIL8, DEPTH32F_STENCIL8,

        // 48-bits per element
        RGB16F, RGB16UI, RGB16I,

        // 64-bits per element
        RG32F, RG32UI, RG32I,
        RGBA16F, RGBA16UI, RGBA16I,

        // 96-bits per element
        RGB32F, RGB32UI, RGB32I,

        // 128-bits per element
        RGBA32F, RGBA32UI, RGBA32I,

        // compressed formats

        // Mandatory in GLES 3.0 and GL 4.3
        EAC_R11, EAC_R11_SIGNED, EAC_RG11, EAC_RG11_SIGNED,
        ETC2_RGB8, ETC2_SRGB8,
        ETC2_RGB8_A1, ETC2_SRGB8_A1,
        ETC2_EAC_RGBA8, ETC2_EAC_SRGBA8,

        // Available everywhere except Android/iOS
        DXT1_RGB, DXT1_RGBA, DXT3_RGBA, DXT5_RGBA
    }

    public enum CompressedFormat {
        // Mandatory in GLES 3.0 and GL 4.3
        EAC_R11, EAC_R11_SIGNED, EAC_RG11, EAC_RG11_SIGNED,
        ETC2_RGB8, ETC2_SRGB8,
        ETC2_RGB8_A1, ETC2_SRGB8_A1,
        ETC2_EAC_RGBA8, ETC2_EAC_SRGBA8,

        // Available everywhere except Android/iOS
        DXT1_RGB, DXT1_RGBA, DXT3_RGBA, DXT5_RGBA
    }

    public enum CubemapFace {
        POSITIVE_X,
        NEGATIVE_X,
        POSITIVE_Y,
        NEGATIVE_Y,
        POSITIVE_Z,
        NEGATIVE_Z
    }

    public enum Format {
        R,
        R_INTEGER,
        RG,
        RG_INTEGER,
        RGB,
        RGB_INTEGER,
        RGBA,
        RGBA_INTEGER,
        UNUSED,
        DEPTH_COMPONENT,
        DEPTH_STENCIL,
        STENCIL_INDEX,
        ALPHA
    }

    public enum Type {
        UBYTE,
        BYTE,
        USHORT,
        SHORT,
        UINT,
        INT,
        HALF,
        FLOAT,
        COMPRESSED,
        UINT_10F_11F_11F_REV
    }

    public static class PixelBufferDescriptor {
        public Buffer storage;

        public Type type;
        public int alignment = 1;
        public int left = 0;
        public int top = 0;

        // used for non-compressed pixel data
        public int stride = 0;
        public Format format;

        // used only for compressed pixel data
        public int compressedSizeInBytes;
        public CompressedFormat compressedFormat;

        @Nullable public Object handler;
        @Nullable public Runnable callback;

        /**
         * Valid handler types:
         * - Android: Handler, Executor
         * - Other: Executor
         */
        public PixelBufferDescriptor(@NonNull Buffer storage,
                @NonNull Format format, @NonNull Type type,
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

        public PixelBufferDescriptor(@NonNull Buffer storage,
                @NonNull Format format, @NonNull Type type) {
            this(storage, format, type, 1, 0, 0, 0, null, null);
        }

        public PixelBufferDescriptor(@NonNull Buffer storage,
                @NonNull Format format, @NonNull Type type,
                @IntRange(from = 1, to = 8) int alignment) {
            this(storage, format, type, alignment, 0, 0, 0, null, null);
        }

        public PixelBufferDescriptor(@NonNull Buffer storage,
                @NonNull Format format, @NonNull Type type,
                @IntRange(from = 1, to = 8) int alignment,
                @IntRange(from = 0) int left, @IntRange(from = 0) int top) {
            this(storage, format, type, alignment, left, top, 0, null, null);
        }

        public PixelBufferDescriptor(@NonNull ByteBuffer storage,
                @NonNull CompressedFormat format,
                @IntRange(from = 0) int compressedSizeInBytes) {
            this.storage = storage;
            this.type = COMPRESSED;
            this.alignment = 1;
            this.compressedFormat = format;
            this.compressedSizeInBytes = compressedSizeInBytes;
        }

        /**
         * Valid handler types:
         * - Android: Handler, Executor
         * - Other: Executor
         */
        public void setCallback(@Nullable Object handler, @Nullable Runnable callback) {
            this.handler = handler;
            this.callback = callback;
        }

        /**
         * Helper to calculate the buffer size (in byte) needed for given parameters
         *
         * @param format        Pixel data format.
         * @param type          Pixel data type. Can't be {@link Type#COMPRESSED}.
         * @param stride        Stride in pixels.
         * @param height        Height in pixels.
         * @param alignment     Alignment in bytes.
         * @return              Size of the buffer in bytes.
         */
        static int computeDataSize(@NonNull Format format, @NonNull Type type,
                int stride, int height, @IntRange(from = 1, to = 8) int alignment) {
            if (type == Type.COMPRESSED) {
                return 0;
            }

            int n = 0;
            switch (format) {
                case R:
                case R_INTEGER:
                case DEPTH_COMPONENT:
                case ALPHA:
                    n = 1;
                    break;
                case RG:
                case RG_INTEGER:
                case DEPTH_STENCIL:
                case STENCIL_INDEX:
                    n = 2;
                    break;
                case RGB:
                case RGB_INTEGER:
                    n = 3;
                    break;
                case RGBA:
                case RGBA_INTEGER:
                    n = 4;
                    break;
            }

            int bpp = n;
            switch (type) {
                case UBYTE:
                case BYTE:
                    // nothing to do
                    break;
                case USHORT:
                case SHORT:
                case HALF:
                    bpp *= 2;
                    break;
                case UINT:
                case INT:
                case FLOAT:
                    bpp *= 4;
                    break;
                case UINT_10F_11F_11F_REV:
                    // Special case, format must be RGB and uses 4 bytes
                    bpp = 4;
                    break;
            }

            int bpr = bpp * stride;
            int bprAligned = (bpr + (alignment - 1)) & -alignment;
            return bprAligned * height;
        }
    }

    public static class PrefilterOptions {
        public int sampleCount = 8;
        public boolean mirror = true;
    }

    public static boolean isTextureFormatSupported(@NonNull Engine engine,
            @NonNull InternalFormat format) {
        return nIsTextureFormatSupported(engine.getNativeObject(), format.ordinal());
    }

    public static class Builder {
        @SuppressWarnings({"FieldCanBeLocal", "UnusedDeclaration"})
        // Keep to finalize native resources
        private final BuilderFinalizer mFinalizer;
        private final long mNativeBuilder;

        public Builder() {
            mNativeBuilder = nCreateBuilder();
            mFinalizer = new BuilderFinalizer(mNativeBuilder);
        }

        @NonNull
        public Builder width(@IntRange(from = 1) int width) {
            nBuilderWidth(mNativeBuilder, width);
            return this;
        }

        @NonNull
        public Builder height(@IntRange(from = 1) int height) {
            nBuilderHeight(mNativeBuilder, height);
            return this;
        }

        @NonNull
        public Builder depth(@IntRange(from = 1) int depth) {
            nBuilderDepth(mNativeBuilder, depth);
            return this;
        }

        @NonNull
        public Builder levels(@IntRange(from = 1) int levels) {
            nBuilderLevels(mNativeBuilder, levels);
            return this;
        }

        @NonNull
        public Builder sampler(@NonNull Sampler target) {
            nBuilderSampler(mNativeBuilder, target.ordinal());
            return this;
        }

        @NonNull
        public Builder format(@NonNull InternalFormat format) {
            nBuilderFormat(mNativeBuilder, format.ordinal());
            return this;
        }

        /**
         * Sets the usage flags, which is necessary when attaching to {@link RenderTarget}.
         *
         * The flags argument much be a combination of {@link Usage} flags.
         */
        @NonNull
        public Builder usage(int flags) {
            nBuilderUsage(mNativeBuilder, flags);
            return this;
        }

        @NonNull
        public Texture build(@NonNull Engine engine) {
            long nativeTexture = nBuilderBuild(mNativeBuilder, engine.getNativeObject());
            if (nativeTexture == 0) throw new IllegalStateException("Couldn't create Texture");
            return new Texture(nativeTexture);
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

    public static class Usage {
        public static final int COLOR_ATTACHMENT = 0x1;
        public static final int DEPTH_ATTACHMENT = 0x2;
        public static final int STENCIL_ATTACHMENT = 0x4;
        public static final int UPLOADABLE = 0x8;
        public static final int SAMPLEABLE = 0x10;
        public static final int DEFAULT = UPLOADABLE | SAMPLEABLE;
    }

    public static final int BASE_LEVEL = 0;

    public int getWidth(@IntRange(from = 0) int level) {
        return nGetWidth(getNativeObject(), level);
    }

    public int getHeight(@IntRange(from = 0) int level) {
        return nGetHeight(getNativeObject(), level);
    }

    public int getDepth(@IntRange(from = 0) int level) {
        return nGetDepth(getNativeObject(), level);
    }

    public int getLevels() {
        return nGetLevels(getNativeObject());
    }

    @NonNull
    public Sampler getTarget() {
        return Sampler.values()[nGetTarget(getNativeObject())];
    }

    @NonNull
    public InternalFormat getFormat() {
        return InternalFormat.values()[nGetInternalFormat(getNativeObject())];
    }

    // TODO: add a setImage() version that takes an android Bitmap

    public void setImage(@NonNull Engine engine,
            @IntRange(from = 0) int level,
            @NonNull PixelBufferDescriptor buffer) {
        setImage(engine, level, 0, 0, getWidth(level), getHeight(level), buffer);
    }

    public void setImage(@NonNull Engine engine,
            @IntRange(from = 0) int level,
            @IntRange(from = 0) int xoffset, @IntRange(from = 0) int yoffset,
            @IntRange(from = 0) int width, @IntRange(from = 0) int height,
            @NonNull PixelBufferDescriptor buffer) {
        int result;
        if (buffer.type == COMPRESSED) {
            result = nSetImageCompressed(getNativeObject(), engine.getNativeObject(), level,
                    xoffset, yoffset, width, height,
                    buffer.storage, buffer.storage.remaining(),
                    buffer.left, buffer.top, buffer.type.ordinal(), buffer.alignment,
                    buffer.compressedSizeInBytes, buffer.compressedFormat.ordinal(),
                    buffer.handler, buffer.callback);
        } else {
            result = nSetImage(getNativeObject(), engine.getNativeObject(), level,
                    xoffset, yoffset, width, height,
                    buffer.storage, buffer.storage.remaining(),
                    buffer.left, buffer.top, buffer.type.ordinal(), buffer.alignment,
                    buffer.stride, buffer.format.ordinal(),
                    buffer.handler, buffer.callback);
        }
        if (result < 0) {
            throw new BufferOverflowException();
        }
    }

    // note: faceOffsetsInBytes are offsets in byte in the buffer relative to the current position()
    // note: use Texture CubemapFace to index the faceOffsetsInBytes array
    // note: we assume all 6 faces are tightly packed
    public void setImage(@NonNull Engine engine, @IntRange(from = 0) int level,
            @NonNull PixelBufferDescriptor buffer,
            @NonNull @Size(min = 6) int[] faceOffsetsInBytes) {
        int result;
        if (buffer.type == COMPRESSED) {
            result = nSetImageCubemapCompressed(getNativeObject(), engine.getNativeObject(), level,
                    buffer.storage, buffer.storage.remaining(),
                    buffer.left, buffer.top, buffer.type.ordinal(), buffer.alignment,
                    buffer.compressedSizeInBytes, buffer.compressedFormat.ordinal(),
                    faceOffsetsInBytes, buffer.handler, buffer.callback);
        } else {
            result = nSetImageCubemap(getNativeObject(), engine.getNativeObject(), level,
                    buffer.storage, buffer.storage.remaining(),
                    buffer.left, buffer.top, buffer.type.ordinal(), buffer.alignment,
                    buffer.stride, buffer.format.ordinal(),
                    faceOffsetsInBytes, buffer.handler, buffer.callback);
        }
        if (result < 0) {
            throw new BufferOverflowException();
        }
    }

    public void setExternalImage(@NonNull Engine engine, long eglImage) {
        nSetExternalImage(getNativeObject(), engine.getNativeObject(), eglImage);
    }

    public void setExternalStream(@NonNull Engine engine, @NonNull Stream stream) {
        long nativeObject = getNativeObject();
        long streamNativeObject = stream.getNativeObject();
        if (!nIsStreamValidForTexture(nativeObject, streamNativeObject)) {
            throw new IllegalStateException("Invalid texture sampler: " +
                    "When used with a stream, a texture must use a SAMPLER_EXTERNAL");
        }
        nSetExternalStream(nativeObject, engine.getNativeObject(), streamNativeObject);
    }

    public void generateMipmaps(@NonNull Engine engine) {
        nGenerateMipmaps(getNativeObject(), engine.getNativeObject());
    }

    public void generatePrefilterMipmap(@NonNull Engine engine,
        @NonNull PixelBufferDescriptor buffer, @NonNull @Size(min = 6) int[] faceOffsetsInBytes,
            PrefilterOptions options) {

        int width = getWidth(0);
        int height= getHeight(0);
        int sampleCount = 8;
        boolean mirror = true;
        if (options != null) {
            sampleCount = options.sampleCount;
            mirror = options.mirror;
        }

        int result = nGeneratePrefilterMipmap(getNativeObject(), engine.getNativeObject(),
            width, height,
            buffer.storage, buffer.storage.remaining(),
            buffer.left, buffer.top, buffer.type.ordinal(), buffer.alignment,
            buffer.stride, buffer.format.ordinal(), faceOffsetsInBytes,
            buffer.handler, buffer.callback,
            sampleCount, mirror);

        if (result < 0) {
            throw new BufferOverflowException();
        }
    }


    @UsedByReflection("TextureHelper.java")
    long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed Texture");
        }
        return mNativeObject;
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native boolean nIsTextureFormatSupported(long nativeEngine, int internalFormat);

    private static native long nCreateBuilder();
    private static native void nDestroyBuilder(long nativeBuilder);

    private static native void nBuilderWidth(long nativeBuilder, int width);
    private static native void nBuilderHeight(long nativeBuilder, int height);
    private static native void nBuilderDepth(long nativeBuilder, int depth);
    private static native void nBuilderLevels(long nativeBuilder, int levels);
    private static native void nBuilderSampler(long nativeBuilder, int sampler);
    private static native void nBuilderFormat(long nativeBuilder, int format);
    private static native void nBuilderUsage(long nativeBuilder, int flags);
    private static native long nBuilderBuild(long nativeBuilder, long nativeEngine);

    private static native int nGetWidth(long nativeTexture, int level);
    private static native int nGetHeight(long nativeTexture, int level);
    private static native int nGetDepth(long nativeTexture, int level);
    private static native int nGetLevels(long nativeTexture);
    private static native int nGetTarget(long nativeTexture);
    private static native int nGetInternalFormat(long nativeTexture);

    private static native int nSetImage(long nativeTexture, long nativeEngine,
            int level, int xoffset, int yoffset, int width, int height,
            Buffer storage, int remaining, int left, int bottom, int type, int alignment,
            int stride, int format,
            Object handler, Runnable callback);

    private static native int nSetImageCompressed(long nativeTexture, long nativeEngine,
            int level, int xoffset, int yoffset, int width, int height,
            Buffer storage, int remaining, int left, int bottom, int type, int alignment,
            int compressedSizeInBytes, int compressedFormat,
            Object handler, Runnable callback);

    private static native int nSetImageCubemap(long nativeTexture, long nativeEngine,
            int level, Buffer storage, int remaining, int left, int bottom, int type,
            int alignment, int stride, int format,
            int[] faceOffsetsInBytes, Object handler, Runnable callback);

    private static native int nSetImageCubemapCompressed(long nativeTexture, long nativeEngine,
            int level, Buffer storage, int remaining, int left, int bottom, int type,
            int alignment, int compressedSizeInBytes, int compressedFormat,
            int[] faceOffsetsInBytes, Object handler, Runnable callback);

    private static native void nSetExternalImage(
            long nativeObject, long nativeEngine, long eglImage);

    private static native void nSetExternalStream(long nativeTexture,
            long nativeEngine, long nativeStream);

    private static native void nGenerateMipmaps(long nativeTexture, long nativeEngine);

    private static native boolean nIsStreamValidForTexture(long nativeTexture, long nativeStream);

    private static native int nGeneratePrefilterMipmap(long nativeIndirectLight, long nativeEngine,
        int width, int height, Buffer storage, int remaining, int left, int top,
        int type, int alignment, int stride, int format, int[] faceOffsetsInBytes,
        Object handler, Runnable callback, int sampleCount, boolean mirror);
}
