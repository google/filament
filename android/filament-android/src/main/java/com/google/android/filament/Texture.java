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
import androidx.annotation.Size;

import com.google.android.filament.proguard.UsedByReflection;

import java.nio.Buffer;
import java.nio.BufferOverflowException;
import java.nio.ByteBuffer;

import static com.google.android.filament.Texture.Type.COMPRESSED;

/**
 * Texture
 * <p>The <code>Texture</code> class supports:</p>
 * <ul>
 *  <li>2D textures</li>
 *  <li>3D textures</li>
 *  <li>Cube maps</li>
 *  <li>mip mapping</li>
 * </ul>
 *
 *
 * <h1>Usage example</h1>
 *
 * A <code>Texture</code> object is created using the {@link Texture.Builder} and destroyed by
 * calling {@link Engine#destroyTexture}. They're bound using {@link MaterialInstance#setParameter}.
 *
 * <pre>
 *  Engine engine = Engine.create();
 *
 *  Material material = new Material.Builder()
 *              .payload( ... )
 *              .build(ending);
 *
 *  MaterialInstance mi = material.getDefaultInstance();
 *
 *  Texture texture = new Texture.Builder()
 *              .width(64)
 *              .height(64)
 *              .build(engine);
 *
 *
 *  texture.setImage(engine, 0,
 *          new Texture.PixelBufferDescriptor( ... ));
 *
 *  mi.setParameter("parameterName", texture, new TextureSampler());
 * </pre>
 *
 * @see #setImage
 * @see PixelBufferDescriptor
 * @see MaterialInstance#setParameter(String, Texture, TextureSampler)
 */
public class Texture {
    private static final Sampler[] sSamplerValues = Sampler.values();
    private static final InternalFormat[] sInternalFormatValues = InternalFormat.values();

    private long mNativeObject;

    public Texture(long nativeTexture) {
        mNativeObject = nativeTexture;
    }

    /**
     * Type of sampler
     */
    public enum Sampler {
        /** 2D sampler */
        SAMPLER_2D,
        /** 2D array sampler  */
        SAMPLER_2D_ARRAY,
        /** Cubemap sampler */
        SAMPLER_CUBEMAP,
        /** External texture sampler */
        SAMPLER_EXTERNAL,
        /** 3D sampler */
        SAMPLER_3D,
    }

    /**
     * Internal texel formats
     *
     * <p>These formats are used to specify a texture's internal storage format.</p>
     *
     * <h1>Enumerants syntax format</h1>
     *
     * <code>[components][size][type]</code>
     * <br><code>components</code> : List of stored components by this format
     * <br><code>size</code>       : Size in bit of each component
     * <br><code>type</code>       : Type this format is stored as
     *
     * <center>
     * <table border="1">
     *     <tr><th> Name    </th><th> Component                     </th></tr>
     *     <tr><td> R       </td><td> Linear Red                    </td></tr>
     *     <tr><td> RG      </td><td> Linear Red, Green             </td></tr>
     *     <tr><td> RGB     </td><td> Linear Red, Green, Blue       </td></tr>
     *     <tr><td> RGBA    </td><td> Linear Red, Green Blue, Alpha </td></tr>
     *     <tr><td> SRGB    </td><td> sRGB encoded Red, Green, Blue </td></tr>
     *     <tr><td> DEPTH   </td><td> Depth                         </td></tr>
     *     <tr><td> STENCIL </td><td> Stencil                       </td></tr>
     * </table>
     * </center>
     * <br>
     *
     * <center>
     * <table border="1">
     *     <tr><th> Name   </th><th> Type                                                       </th></tr>
     *     <tr><td> (none) </td><td> Unsigned Normalized Integer [0, 1]                         </th></tr>
     *     <tr><td> _SNORM </td><td> Signed Normalized Integer [-1, 1]                          </td></tr>
     *     <tr><td> UI     </td><td> Unsigned Integer [0, 2<sup>size</sup>]                     </td></tr>
     *     <tr><td> I      </td><td> Signed Integer [-2<sup>size-1</sup>, 2<sup>size-1</sup>-1] </td></tr>
     *     <tr><td> F      </td><td> Floating-point                                             </td></tr>
     * </table>
     * </center>
     * <br>
     *
     * <h1>Special color formats</h1>
     *
     * There are a few special color formats that don't follow the convention above:
     *
     * <center>
     * <table border="1">
     *     <tr><th> Name             </th><th> Format </th></tr>
     *     <tr><td> RGB565           </td><td>  5-bits for R and B, 6-bits for G.                            </td></tr>
     *     <tr><td> RGB5_A1          </td><td>  5-bits for R, G and B, 1-bit for A.                          </td></tr>
     *     <tr><td> RGB10_A2         </td><td> 10-bits for R, G and B, 2-bits for A.                         </td></tr>
     *     <tr><td> RGB9_E5          </td><td> <b>Unsigned</b> floating point. 9-bits mantissa for RGB, 5-bits shared exponent                 </td></tr>
     *     <tr><td> R11F_G11F_B10F   </td><td> <b>Unsigned</b> floating point. 6-bits mantissa, for R and G, 5-bits for B. 5-bits exponent.    </td></tr>
     *     <tr><td> SRGB8_A8         </td><td> sRGB 8-bits with linear 8-bits alpha.                        </td></tr>
     *     <tr><td> DEPTH24_STENCIL8 </td><td> 24-bits unsigned normalized integer depth, 8-bits stencil.   </td></tr>
     * </table>
     * </center>
     * <br>
     *
     * <h1>Compressed texture formats</h1>
     *
     * Many compressed texture formats are supported as well, which include (but are not limited to)
     * the following list:
     *
     * <center>
     * <table border="1">
     *     <tr><th> Name            </th><th> Format                            </th></tr>
     *     <tr><td> EAC_R11         </td><td> Compresses R11UI                  </td></tr>
     *     <tr><td> EAC_R11_SIGNED  </td><td> Compresses R11I                   </td></tr>
     *     <tr><td> EAC_RG11        </td><td> Compresses RG11UI                 </td></tr>
     *     <tr><td> EAC_RG11_SIGNED </td><td> Compresses RG11I                  </td></tr>
     *     <tr><td> ETC2_RGB8       </td><td> Compresses RGB8                   </td></tr>
     *     <tr><td> ETC2_SRGB8      </td><td> compresses SRGB8                  </td></tr>
     *     <tr><td> ETC2_EAC_RGBA8  </td><td> Compresses RGBA8                  </td></tr>
     *     <tr><td> ETC2_EAC_SRGBA8 </td><td> Compresses SRGB8_A8               </td></tr>
     *     <tr><td> ETC2_RGB8_A1    </td><td> Compresses RGB8 with 1-bit alpha  </td></tr>
     *     <tr><td> ETC2_SRGB8_A1   </td><td> Compresses sRGB8 with 1-bit alpha </td></tr>
     * </table>
     * </center>
     */
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
        DXT1_RGB, DXT1_RGBA, DXT3_RGBA, DXT5_RGBA,
        DXT1_SRGB, DXT1_SRGBA, DXT3_SRGBA, DXT5_SRGBA,

        // ASTC formats are available with a GLES extension
        RGBA_ASTC_4x4,
        RGBA_ASTC_5x4,
        RGBA_ASTC_5x5,
        RGBA_ASTC_6x5,
        RGBA_ASTC_6x6,
        RGBA_ASTC_8x5,
        RGBA_ASTC_8x6,
        RGBA_ASTC_8x8,
        RGBA_ASTC_10x5,
        RGBA_ASTC_10x6,
        RGBA_ASTC_10x8,
        RGBA_ASTC_10x10,
        RGBA_ASTC_12x10,
        RGBA_ASTC_12x12,
        SRGB8_ALPHA8_ASTC_4x4,
        SRGB8_ALPHA8_ASTC_5x4,
        SRGB8_ALPHA8_ASTC_5x5,
        SRGB8_ALPHA8_ASTC_6x5,
        SRGB8_ALPHA8_ASTC_6x6,
        SRGB8_ALPHA8_ASTC_8x5,
        SRGB8_ALPHA8_ASTC_8x6,
        SRGB8_ALPHA8_ASTC_8x8,
        SRGB8_ALPHA8_ASTC_10x5,
        SRGB8_ALPHA8_ASTC_10x6,
        SRGB8_ALPHA8_ASTC_10x8,
        SRGB8_ALPHA8_ASTC_10x10,
        SRGB8_ALPHA8_ASTC_12x10,
        SRGB8_ALPHA8_ASTC_12x12,

        // RGTC formats available with a GLES extension
        RED_RGTC1,              // BC4 unsigned
        SIGNED_RED_RGTC1,       // BC4 signed
        RED_GREEN_RGTC2,        // BC5 unsigned
        SIGNED_RED_GREEN_RGTC2, // BC5 signed

        // BPTC formats available with a GLES extension
        RGB_BPTC_SIGNED_FLOAT,  // BC6H signed
        RGB_BPTC_UNSIGNED_FLOAT,// BC6H unsigned
        RGBA_BPTC_UNORM,        // BC7
        SRGB_ALPHA_BPTC_UNORM   // BC7 sRGB
    }

    /**
     * Compressed data types for use with {@link PixelBufferDescriptor}
     * @see InternalFormat
     */
    public enum CompressedFormat {
        // Mandatory in GLES 3.0 and GL 4.3
        EAC_R11, EAC_R11_SIGNED, EAC_RG11, EAC_RG11_SIGNED,
        ETC2_RGB8, ETC2_SRGB8,
        ETC2_RGB8_A1, ETC2_SRGB8_A1,
        ETC2_EAC_RGBA8, ETC2_EAC_SRGBA8,

        // Available everywhere except Android/iOS
        DXT1_RGB, DXT1_RGBA, DXT3_RGBA, DXT5_RGBA,
        DXT1_SRGB, DXT1_SRGBA, DXT3_SRGBA, DXT5_SRGBA,

        // ASTC formats are available with a GLES extension
        RGBA_ASTC_4x4,
        RGBA_ASTC_5x4,
        RGBA_ASTC_5x5,
        RGBA_ASTC_6x5,
        RGBA_ASTC_6x6,
        RGBA_ASTC_8x5,
        RGBA_ASTC_8x6,
        RGBA_ASTC_8x8,
        RGBA_ASTC_10x5,
        RGBA_ASTC_10x6,
        RGBA_ASTC_10x8,
        RGBA_ASTC_10x10,
        RGBA_ASTC_12x10,
        RGBA_ASTC_12x12,
        SRGB8_ALPHA8_ASTC_4x4,
        SRGB8_ALPHA8_ASTC_5x4,
        SRGB8_ALPHA8_ASTC_5x5,
        SRGB8_ALPHA8_ASTC_6x5,
        SRGB8_ALPHA8_ASTC_6x6,
        SRGB8_ALPHA8_ASTC_8x5,
        SRGB8_ALPHA8_ASTC_8x6,
        SRGB8_ALPHA8_ASTC_8x8,
        SRGB8_ALPHA8_ASTC_10x5,
        SRGB8_ALPHA8_ASTC_10x6,
        SRGB8_ALPHA8_ASTC_10x8,
        SRGB8_ALPHA8_ASTC_10x10,
        SRGB8_ALPHA8_ASTC_12x10,
        SRGB8_ALPHA8_ASTC_12x12,

        // RGTC formats available with a GLES extension
        RED_RGTC1,              // BC4 unsigned
        SIGNED_RED_RGTC1,       // BC4 signed
        RED_GREEN_RGTC2,        // BC5 unsigned
        SIGNED_RED_GREEN_RGTC2, // BC5 signed

        // BPTC formats available with a GLES extension
        RGB_BPTC_SIGNED_FLOAT,  // BC6H signed
        RGB_BPTC_UNSIGNED_FLOAT,// BC6H unsigned
        RGBA_BPTC_UNORM,        // BC7
        SRGB_ALPHA_BPTC_UNORM   // BC7 sRGB
    }

    /**
     * Cubemap faces
     */
    public enum CubemapFace {
        /** +x face */
        POSITIVE_X,
        /** -x face */
        NEGATIVE_X,
        /** +y face */
        POSITIVE_Y,
        /** -y face */
        NEGATIVE_Y,
        /** +z face */
        POSITIVE_Z,
        /** -z face */
        NEGATIVE_Z
    }

    /**
     * Pixel color format
     */
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

    /**
     * Pixel data type
     */
    public enum Type {
        /** unsigned byte, 8-bit */
        UBYTE,
        /** signed byte, 8-bit */
        BYTE,
        /** unsigned short, 16-bits*/
        USHORT,
        /** signed short, 16-bit */
        SHORT,
        /** unsigned int, 32-bit */
        UINT,
        /** signed int, 32-bit */
        INT,
        /** half-float, 16-bit float with 10 bits mantissa */
        HALF,
        /** float, 32-bit float, with 24 bits mantissa */
        FLOAT,
        /** a compressed type */
        COMPRESSED,
        /** unsigned 5.6 (5.5 for blue) float packed in a 32-bit integer. */
        UINT_10F_11F_11F_REV,
        /** unsigned 5/6 bit integers packed in a 16-bit short. */
        USHORT_565,
    }

    /**
     * Texture swizzling channels
     */
    public enum Swizzle {
        SUBSTITUTE_ZERO,    //!< specified component is substituted with 0
        SUBSTITUTE_ONE,     //!< specified component is substituted with 1
        CHANNEL_0,          //!< specified component taken from channel 0
        CHANNEL_1,          //!< specified component taken from channel 1
        CHANNEL_2,          //!< specified component taken from channel 2
        CHANNEL_3           //!< specified component taken from channel 3
    }

    /**
     * A descriptor to an image in main memory, typically used to transfer image data from the CPU
     * to the GPU.
     * <p>A <code>PixelBufferDescriptor</code> owns the memory buffer it references,
     * therefore <code>PixelBufferDescriptor</code> cannot be copied, but can be moved.</p>
     *
     * <code>PixelBufferDescriptor</code> releases ownership of the memory-buffer when it's
     * destroyed.
     *
     * @see #setImage
     */
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

        /**
         * Callback used to destroy the buffer data.
         * <p>
         * Guarantees:
         * <ul>
         *     <li>Called on the main filament thread.</li>
         * </ul>
         * </p>
         *
         * <p>
         * Limitations:
         * <ul>
         *     <li>Must be lightweight.</li>
         *     <li>Must not call filament APIs.</li>
         * </ul>
         * </p>
         */
        @Nullable public Runnable callback;

        /**
         * Creates a <code>PixelBufferDescriptor</code>
         *
         * @param storage       CPU-side buffer containing the image data to upload into the texture
         * @param format        Pixel {@link Format format} of the CPU-side image
         * @param type          Pixel data {@link Type type} of the  CPU-side image
         * @param alignment     Row-alignment in bytes of the CPU-side image (1 to 8 bytes)
         * @param left          Left coordinate in pixels of the CPU-side image
         * @param top           Top coordinate in pixels of the CPU-side image
         * @param stride        Stride in pixels of the CPU-side image (i.e. distance in pixels to the next row)
         * @param handler       An {@link java.util.concurrent.Executor Executor}. On Android this can also be a {@link android.os.Handler Handler}.
         * @param callback      A callback executed by <code>handler</code> when <code>storage</code> is no longer needed.
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

        /**
         * Creates a <code>PixelBufferDescriptor</code> with some default values and no callback.
         *
         * @param storage       CPU-side buffer containing the image data to upload into the texture
         * @param format        Pixel {@link Format format} of the CPU-side image
         * @param type          Pixel data {@link Type type} of the  CPU-side image
         *
         * @see #setCallback
         */
        public PixelBufferDescriptor(@NonNull Buffer storage,
                @NonNull Format format, @NonNull Type type) {
            this(storage, format, type, 1, 0, 0, 0, null, null);
        }

        /**
         * Creates a <code>PixelBufferDescriptor</code> with some default values and no callback.
         *
         * @param storage       CPU-side buffer containing the image data to upload into the texture
         * @param format        Pixel {@link Format format} of the CPU-side image
         * @param type          Pixel data {@link Type type} of the  CPU-side image
         * @param alignment     Row-alignment in bytes of the CPU-side image (1 to 8 bytes)
         *
         * @see #setCallback
         */
        public PixelBufferDescriptor(@NonNull Buffer storage,
                @NonNull Format format, @NonNull Type type,
                @IntRange(from = 1, to = 8) int alignment) {
            this(storage, format, type, alignment, 0, 0, 0, null, null);
        }

        /**
         * Creates a <code>PixelBufferDescriptor</code> with some default values and no callback.
         *
         * @param storage       CPU-side buffer containing the image data to upload into the texture
         * @param format        Pixel {@link Format format} of the CPU-side image
         * @param type          Pixel data {@link Type type} of the  CPU-side image
         * @param alignment     Row-alignment in bytes of the CPU-side image (1 to 8 bytes)
         * @param left          Left coordinate in pixels of the CPU-side image
         * @param top           Top coordinate in pixels of the CPU-side image
         *
         * @see #setCallback
         */
        public PixelBufferDescriptor(@NonNull Buffer storage,
                @NonNull Format format, @NonNull Type type,
                @IntRange(from = 1, to = 8) int alignment,
                @IntRange(from = 0) int left, @IntRange(from = 0) int top) {
            this(storage, format, type, alignment, left, top, 0, null, null);
        }

        /**
         *
         * @param storage       CPU-side buffer containing the image data to upload into the texture
         * @param format        Compressed pixel {@link CompressedFormat format} of the CPU-side image
         * @param compressedSizeInBytes Size of the compressed data in bytes
         *
         * @see #setCallback
         */
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
         * Set or replace the callback called when the CPU-side data is no longer needed.
         *
         * @param handler       An {@link java.util.concurrent.Executor Executor}. On Android this can also be a {@link android.os.Handler Handler}.
         * @param callback      A callback executed by <code>handler</code> when <code>storage</code> is no longer needed.
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
        public static int computeDataSize(@NonNull Format format, @NonNull Type type,
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

                default: throw new IllegalStateException("unsupported format enum");
            }

            int bpp = n;
            switch (type) {
                case UBYTE:
                case BYTE:
                case COMPRESSED:
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
                case USHORT_565:
                    bpp = 2;
                    break;
            }

            int bpr = bpp * stride;
            int bprAligned = (bpr + (alignment - 1)) & -alignment;
            return bprAligned * height;
        }
    }

    /**
     * Options of {@link #generatePrefilterMipmap}
     */
    public static class PrefilterOptions {
        /** number of samples for roughness pre-filtering */
        public int sampleCount = 8;
        /** whether to generate a reflection map (mirror) */
        public boolean mirror = true;
    }

    /**
     * Checks whether a given format is supported for texturing in this {@link Engine}.
     * This depends on the selected backend.
     *
     * @param engine {@link Engine} to test the {@link InternalFormat InternalFormat} against
     * @param format format to check
     * @return <code>true</code> if this format is supported for texturing.
     */
    public static boolean isTextureFormatSupported(@NonNull Engine engine,
            @NonNull InternalFormat format) {
        return nIsTextureFormatSupported(engine.getNativeObject(), format.ordinal());
    }

    /**
     * Checks whether texture swizzling is supported in this {@link Engine}.
     * This depends on the selected backend.
     *
     * @param engine {@link Engine}
     * @return <code>true</code> if texture swizzling.
     */
    public static boolean isTextureSwizzleSupported(@NonNull Engine engine) {
        return nIsTextureSwizzleSupported(engine.getNativeObject());
    }

    /**
     * Use <code>Builder</code> to construct a <code>Texture</code> object instance.
     */
    public static class Builder {
        @SuppressWarnings({"FieldCanBeLocal", "UnusedDeclaration"})
        // Keep to finalize native resources
        private final BuilderFinalizer mFinalizer;
        private final long mNativeBuilder;

        /**
         * Use <code>Builder</code> to construct a <code>Texture</code> object instance.
         */
        public Builder() {
            mNativeBuilder = nCreateBuilder();
            mFinalizer = new BuilderFinalizer(mNativeBuilder);
        }

        /**
         * Specifies the width of the texture in texels.
         * @param width texture width in texels, must be at least 1. Default is 1.
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder width(@IntRange(from = 1) int width) {
            nBuilderWidth(mNativeBuilder, width);
            return this;
        }

        /**
         * Specifies the height of the texture in texels.
         * @param height texture height in texels, must be at least 1. Default is 1.
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder height(@IntRange(from = 1) int height) {
            nBuilderHeight(mNativeBuilder, height);
            return this;
        }

        /**
         * Specifies the texture's number of layers. Values greater than 1 create a 3D texture.
         *
         * <p>This <code>Texture</code> instance must use
         * {@link Sampler#SAMPLER_2D_ARRAY SAMPLER_2D_ARRAY} or it has no effect.</p>
         *
         * @param depth texture number of layers. Default is 1.
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder depth(@IntRange(from = 1) int depth) {
            nBuilderDepth(mNativeBuilder, depth);
            return this;
        }

        /**
         * Specifies the number of mipmap levels
         * @param levels must be at least 1 and less or equal to <code>floor(log<sub>2</sub>(max(width, height))) + 1</code>. Default is 1.
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder levels(@IntRange(from = 1) int levels) {
            nBuilderLevels(mNativeBuilder, levels);
            return this;
        }

        /**
         * Specifies the type of sampler to use.
         * @param target {@link Sampler Sampler} type
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder sampler(@NonNull Sampler target) {
            nBuilderSampler(mNativeBuilder, target.ordinal());
            return this;
        }

        /**
         * Specifies the texture's internal format.
         * <p>The internal format specifies how texels are stored (which may be different from how
         * they're specified in {@link #setImage}). {@link InternalFormat InternalFormat} specifies
         * both the color components and the data type used.</p>
         * @param format texture's {@link InternalFormat internal format}.
         * @return This Builder, for chaining calls.
         */
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

        /**
         * Specifies how a texture's channels map to color components
         *
         * @param r  texture channel for red component
         * @param g  texture channel for green component
         * @param b  texture channel for blue component
         * @param a  texture channel for alpha component
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder swizzle(@NonNull Swizzle r, @NonNull Swizzle g, @NonNull Swizzle b, @NonNull Swizzle a) {
            nBuilderSwizzle(mNativeBuilder, r.ordinal(), g.ordinal(), b.ordinal(), a.ordinal());
            return this;
        }

        /**
         * Specify a native texture to import as a Filament texture.
         * <p>
         * The texture id is backend-specific:
         * <ul>
         *   <li> OpenGL: GLuint texture ID </li>
         * </ul>
         * </p>
         *
         *
         * @param id a backend specific texture identifier
         *
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder importTexture(long id) {
            nBuilderImportTexture(mNativeBuilder, id);
            return this;
        }

        /**
         * Creates a new <code>Texture</code> instance.
         * @param engine The {@link Engine} to associate this <code>Texture</code> with.
         * @return A newly created <code>Texture</code>
         * @exception IllegalStateException if a parameter to a builder function was invalid.
         *            A mode detailed message about the error is output in the system log.
         */
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

    /**
     * A bitmask to specify how the texture will be used.
     */
    public static class Usage {
        /** The texture will be used as a color attachment */
        public static final int COLOR_ATTACHMENT = 0x1;
        /** The texture will be used as a depth attachment */
        public static final int DEPTH_ATTACHMENT = 0x2;
        /** The texture will be used as a stencil attachment */
        public static final int STENCIL_ATTACHMENT = 0x4;
        /** The texture content can be set with {@link #setImage} */
        public static final int UPLOADABLE = 0x8;
        /** The texture can be read from a shader or blitted from */
        public static final int SAMPLEABLE = 0x10;
        /** Texture can be used as a subpass input */
        public static final int SUBPASS_INPUT = 0x20;
        /** Texture can be used the source of a blit() */
        public static final int BLIT_SRC = 0x40;
        /** Texture can be used the destination of a blit() */
        public static final int BLIT_DST = 0x80;
        /** by default textures are <code>UPLOADABLE</code> and <code>SAMPLEABLE</code>*/
        public static final int DEFAULT = UPLOADABLE | SAMPLEABLE;
    }

    public static final int BASE_LEVEL = 0;

    /**
     * Queries the width of a given level of this texture.
     * @param level to query the with of. Must be between 0 and {@link #getLevels}
     * @return The width in texel of the given level
     */
    public int getWidth(@IntRange(from = 0) int level) {
        return nGetWidth(getNativeObject(), level);
    }

    /**
     * Queries the height of a given level of this texture.
     * @param level to query the height of. Must be between 0 and {@link #getLevels}
     * @return The height in texel of the given level
     */
    public int getHeight(@IntRange(from = 0) int level) {
        return nGetHeight(getNativeObject(), level);
    }

    /**
     * Queries the number of layers of given level of this texture has.
     * @param level to query the number of layers of. Must be between 0 and {@link #getLevels}
     * @return The number of layers of the given level
     */
    public int getDepth(@IntRange(from = 0) int level) {
        return nGetDepth(getNativeObject(), level);
    }

    /**
     * @return the number of mipmap levels of this texture
     */
    public int getLevels() {
        return nGetLevels(getNativeObject());
    }

    /**
     * @return This texture {@link Sampler Sampler} type.
     */
    @NonNull
    public Sampler getTarget() {
        return sSamplerValues[nGetTarget(getNativeObject())];
    }

    /**
     * @return This texture's {@link InternalFormat InternalFormat}.
     */
    @NonNull
    public InternalFormat getFormat() {
        return sInternalFormatValues[nGetInternalFormat(getNativeObject())];
    }

    // TODO: add a setImage() version that takes an android Bitmap

    /**
     * <code>setImage</code> is used to modify the whole content of the texture from a CPU-buffer.
     *
     *  <p>This <code>Texture</code> instance must use {@link Sampler#SAMPLER_2D SAMPLER_2D} or
     *  {@link Sampler#SAMPLER_EXTERNAL SAMPLER_EXTERNAL}. If the later is specified
     *  and external textures are supported by the driver implementation,
     *  this method will have no effect, otherwise it will behave as if the
     *  texture was specified with {@link Sampler#SAMPLER_2D SAMPLER_2D}.</p>
     *
     * This is equivalent to calling: <br>
     *
     * <code>setImage(engine, level, 0, 0, getWidth(level), getHeight(level), buffer)</code>
     *
     * @param engine    {@link Engine} this texture is associated to. Must be the
     *                  instance passed to {@link Builder#build Builder.build()}.
     * @param level     Level to set the image for. Must be less than {@link #getLevels()}.
     * @param buffer    Client-side buffer containing the image to set.
     *                  <code>buffer</code>'s {@link Format format} must match that
     *                  of {@link #getFormat()}
     *
     * @exception BufferOverflowException if the specified parameters would result in reading
     * outside of <code>buffer</code>.
     *
     * @see Builder#sampler
     * @see PixelBufferDescriptor
     */
    public void setImage(@NonNull Engine engine,
            @IntRange(from = 0) int level,
            @NonNull PixelBufferDescriptor buffer) {
        setImage(engine, level, 0, 0, 0, getWidth(level), getHeight(level), 1, buffer);
    }


    /**
     * <code>setImage</code> is used to modify a sub-region of the texture from a CPU-buffer.
     *
     *  <p>This <code>Texture</code> instance must use {@link Sampler#SAMPLER_2D SAMPLER_2D} or
     *  {@link Sampler#SAMPLER_EXTERNAL SAMPLER_EXTERNAL}. If the later is specified
     *  and external textures are supported by the driver implementation,
     *  this method will have no effect, otherwise it will behave as if the
     *  texture was specified with {@link Sampler#SAMPLER_2D SAMPLER_2D}.</p>
     *
     * @param engine    {@link Engine} this texture is associated to. Must be the
     *                  instance passed to {@link Builder#build Builder.build()}.
     * @param level     Level to set the image for. Must be less than {@link #getLevels()}.
     * @param xoffset   x-offset in texel of the region to modify
     * @param yoffset   y-offset in texel of the region to modify
     * @param width     width in texel of the region to modify
     * @param height    height in texel of the region to modify
     * @param buffer    Client-side buffer containing the image to set.
     *                  <code>buffer</code>'s {@link Format format} must match that
     *                  of {@link #getFormat()}
     *
     * @exception BufferOverflowException if the specified parameters would result in reading
     * outside of <code>buffer</code>.
     *
     * @see Builder#sampler
     * @see PixelBufferDescriptor
     */
    public void setImage(@NonNull Engine engine,
            @IntRange(from = 0) int level,
            @IntRange(from = 0) int xoffset, @IntRange(from = 0) int yoffset,
            @IntRange(from = 0) int width, @IntRange(from = 0) int height,
            @NonNull PixelBufferDescriptor buffer) {
        setImage(engine, level, xoffset, yoffset, 0, width, height, 1, buffer);
    }

    /**
     * <code>setImage</code> is used to modify a sub-region of a 3D texture, 2D texture array or
     * cubemap from a CPU-buffer. Cubemaps are treated like a 2D array of six layers.
     *
     *  <p>This <code>Texture</code> instance must use {@link Sampler#SAMPLER_2D_ARRAY SAMPLER_2D_ARRAY},
     *  {@link Sampler#SAMPLER_3D SAMPLER_3D} or {@link Sampler#SAMPLER_CUBEMAP SAMPLER_CUBEMAP}.</p>
     *
     * @param engine    {@link Engine} this texture is associated to. Must be the
     *                  instance passed to {@link Builder#build Builder.build()}.
     * @param level     Level to set the image for. Must be less than {@link #getLevels()}.
     * @param xoffset   x-offset in texel of the region to modify
     * @param yoffset   y-offset in texel of the region to modify
     * @param zoffset   z-offset in texel of the region to modify
     * @param width     width in texel of the region to modify
     * @param height    height in texel of the region to modify
     * @param depth     depth in texel or index of the region to modify
     * @param buffer    Client-side buffer containing the image to set.
     *                  <code>buffer</code>'s {@link Format format} must match that
     *                  of {@link #getFormat()}
     *
     * @exception BufferOverflowException if the specified parameters would result in reading
     * outside of <code>buffer</code>.
     *
     * @see Builder#sampler
     * @see PixelBufferDescriptor
     */
    public void setImage(@NonNull Engine engine,
            @IntRange(from = 0) int level,
            @IntRange(from = 0) int xoffset, @IntRange(from = 0) int yoffset, @IntRange(from = 0) int zoffset,
            @IntRange(from = 0) int width, @IntRange(from = 0) int height, @IntRange(from = 0) int depth,
            @NonNull PixelBufferDescriptor buffer) {
        int result;
        if (buffer.type == COMPRESSED) {
            result = nSetImage3DCompressed(getNativeObject(), engine.getNativeObject(), level,
                    xoffset, yoffset, zoffset, width, height, depth,
                    buffer.storage, buffer.storage.remaining(),
                    buffer.left, buffer.top, buffer.type.ordinal(), buffer.alignment,
                    buffer.compressedSizeInBytes, buffer.compressedFormat.ordinal(),
                    buffer.handler, buffer.callback);
        } else {
            result = nSetImage3D(getNativeObject(), engine.getNativeObject(), level,
                    xoffset, yoffset, zoffset, width, height, depth,
                    buffer.storage, buffer.storage.remaining(),
                    buffer.left, buffer.top, buffer.type.ordinal(), buffer.alignment,
                    buffer.stride, buffer.format.ordinal(),
                    buffer.handler, buffer.callback);
        }
        if (result < 0) {
            throw new BufferOverflowException();
        }
    }

    /**
     * <code>setImage</code> is used to specify all six images of a cubemap level and
     * follows exactly the OpenGL conventions
     *
     *  <p>This <code>Texture</code> instance must use
     *  {@link Sampler#SAMPLER_CUBEMAP SAMPLER_CUBEMAP}.</p>
     *
     * @param engine                {@link Engine} this texture is associated to. Must be the
     *                              instance passed to {@link Builder#build Builder.build()}.
     * @param level                 Level to set the image for. Must be less than {@link #getLevels()}.
     * @param buffer                Client-side buffer containing the image to set.
     *                              <code>buffer</code>'s {@link Format format} must match that
     *                              of {@link #getFormat()}
     * @param faceOffsetsInBytes    Offsets in bytes into <code>buffer</code> for all six images.
     *                              The offsets are specified in the following order:
     *                              +x, -x, +y, -y, +z, -z.
     *
     * <p><code>faceOffsetsInBytes</code> are offsets in byte in the <code>buffer</code> relative
     * to the current {@link Buffer#position()}. Use {@link CubemapFace} to index the
     * <code>faceOffsetsInBytes</code> array. All six faces must be tightly packed.</p>
     *
     * @exception BufferOverflowException if the specified parameters would result in reading
     * outside of <code>buffer</code>.
     *
     * @see Builder#sampler
     * @see PixelBufferDescriptor
     * @deprecated use {@link #setImage(Engine, int, int, int, int, int, int, int, PixelBufferDescriptor)}
     */
     @Deprecated
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

    /**
     * Specifies the external image to associate with this <code>Texture</code>.
     *
     *  <p>This <code>Texture</code> instance must use
     *  {@link Sampler#SAMPLER_EXTERNAL SAMPLER_EXTERNAL}.</p>
     * <p>Typically the external image is OS specific, and can be a video or camera frame.
     * There are many restrictions when using an external image as a texture, such as:</p>
     * <ul>
     *   <li> only the level of detail (LOD) 0 can be specified</li>
     *   <li> only {@link TextureSampler.MagFilter#NEAREST NEAREST} or
     *             {@link TextureSampler.MagFilter#LINEAR LINEAR} filtering is supported</li>
     *   <li> the size and format of the texture is defined by the external image</li>
     * </ul>
     *
     * @param engine    {@link Engine} this texture is associated to. Must be the
     *                  instance passed to {@link Builder#build Builder.build()}.
     * @param eglImage  An opaque handle to a platform specific image. Supported types are
     *                  <code>eglImageOES</code> on Android and <code>CVPixelBufferRef</code> on iOS.
     *                  <p>On iOS the following pixel formats are supported: <ul>
     *                          <li><code>kCVPixelFormatType_32BGRA</code></li>
     *                          <li><code>kCVPixelFormatType_420YpCbCr8BiPlanarFullRange</code></li>
     *                   </ul></p>
     *
     * @see Builder#sampler
     */
    public void setExternalImage(@NonNull Engine engine, long eglImage) {
        nSetExternalImage(getNativeObject(), engine.getNativeObject(), eglImage);
    }

    /**
     * Specifies the external stream to associate with this <code>Texture</code>.
     *
     *  <p>This <code>Texture</code> instance must use
     *  {@link Sampler#SAMPLER_EXTERNAL SAMPLER_EXTERNAL}.</p>
     * <p>Typically the external image is OS specific, and can be a video or camera frame.
     * There are many restrictions when using an external image as a texture, such as:</p>
     * <ul>
     *   <li> only the level of detail (LOD) 0 can be specified</li>
     *   <li> only {@link TextureSampler.MagFilter#NEAREST NEAREST} or
     *             {@link TextureSampler.MagFilter#LINEAR LINEAR} filtering is supported</li>
     *   <li> the size and format of the texture is defined by the external image</li>
     * </ul>
     *
     * @param engine    {@link Engine} this texture is associated to. Must be the
     *                  instance passed to {@link Builder#build Builder.build()}.
     * @param stream    A {@link Stream} object
     *
     * @exception IllegalStateException if the sampler type is not
     *                                  {@link Sampler#SAMPLER_EXTERNAL SAMPLER_EXTERNAL}
     *
     * @see Stream
     * @see Builder#sampler
     *
     */
    public void setExternalStream(@NonNull Engine engine, @NonNull Stream stream) {
        long nativeObject = getNativeObject();
        long streamNativeObject = stream.getNativeObject();
        if (!nIsStreamValidForTexture(nativeObject, streamNativeObject)) {
            throw new IllegalStateException("Invalid texture sampler: " +
                    "When used with a stream, a texture must use a SAMPLER_EXTERNAL");
        }
        nSetExternalStream(nativeObject, engine.getNativeObject(), streamNativeObject);
    }

    /**
     * Generates all the mipmap levels automatically. This requires the texture to have a
     * color-renderable format.
     *
     *  <p>This <code>Texture</code> instance must <b>not</b> use
     *  {@link Sampler#SAMPLER_CUBEMAP SAMPLER_CUBEMAP}, or it has no effect.</p>
     *
     * @param engine    {@link Engine} this texture is associated to. Must be the
     *                  instance passed to {@link Builder#build Builder.build()}.
     */
    public void generateMipmaps(@NonNull Engine engine) {
        nGenerateMipmaps(getNativeObject(), engine.getNativeObject());
    }

    /**
     * Creates a reflection map from an environment map.
     *
     * <p>This is a utility function that replaces calls to {@link #setImage}.
     * The provided environment map is processed and all mipmap levels are populated. The
     * processing is similar to the offline tool <code>cmgen</code> at a lower quality setting.</p>
     *
     * <p>This function is intended to be used when the environment cannot be processed offline,
     * for instance if it's generated at runtime.</p>
     *
     * <p>The source data must obey to some constraints:</p>
     * <ul>
     *     <li>the data {@link Format format} must be {@link Format#RGB}</li>
     *     <li>the data {@link Type type} must be one of
     *         <ul>
     *             <li>{@link Type#FLOAT}</li>
     *             <li>{@link Type#HALF}</li>
     *         </ul>
     *     </li>
     * </ul>
     *
     * <p>The current texture must be a cubemap.</p>
     *
     * <p>The reflections cubemap's {@link InternalFormat internal format} cannot be a compressed format.</p>
     *
     * <p>The reflections cubemap's dimension must be a power-of-two.</p>
     *
     * <p>This operation is computationally intensive, especially with large environments and
     *          is currently <b>synchronous</b>. Expect about 1ms for a 16 &times 16 cubemap.</p>
     *
     * @param engine    {@link Engine} this texture is associated to. Must be the
     *                  instance passed to {@link Builder#build Builder.build()}.
     * @param buffer    Client-side buffer containing the image to set.
     *                  <code>buffer</code>'s {@link Format format} and {@link Type type} must match
     *                  the constraints above.
     * @param faceOffsetsInBytes    Offsets in bytes into <code>buffer</code> for all six images.
     *                              The offsets are specified in the following order:
     *                              +x, -x, +y, -y, +z, -z.
     *
     * @param options   Optional parameter to control user-specified quality and options.
     *
     * <p><code>faceOffsetsInBytes</code> are offsets in byte in the <code>buffer</code> relative
     * to the current {@link Buffer#position()}. Use {@link CubemapFace} to index the
     * <code>faceOffsetsInBytes</code> array. All six faces must be tightly packed.</p>
     *
     * @exception BufferOverflowException if the specified parameters would result in reading
     * outside of <code>buffer</code>.
     */
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
    public long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed Texture");
        }
        return mNativeObject;
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native boolean nIsTextureFormatSupported(long nativeEngine, int internalFormat);
    private static native boolean nIsTextureSwizzleSupported(long nativeEngine);

    private static native long nCreateBuilder();
    private static native void nDestroyBuilder(long nativeBuilder);

    private static native void nBuilderWidth(long nativeBuilder, int width);
    private static native void nBuilderHeight(long nativeBuilder, int height);
    private static native void nBuilderDepth(long nativeBuilder, int depth);
    private static native void nBuilderLevels(long nativeBuilder, int levels);
    private static native void nBuilderSampler(long nativeBuilder, int sampler);
    private static native void nBuilderFormat(long nativeBuilder, int format);
    private static native void nBuilderUsage(long nativeBuilder, int flags);
    private static native void nBuilderSwizzle(long nativeBuilder, int r, int g, int b, int a);
    private static native void nBuilderImportTexture(long nativeBuilder, long id);
    private static native long nBuilderBuild(long nativeBuilder, long nativeEngine);

    private static native int nGetWidth(long nativeTexture, int level);
    private static native int nGetHeight(long nativeTexture, int level);
    private static native int nGetDepth(long nativeTexture, int level);
    private static native int nGetLevels(long nativeTexture);
    private static native int nGetTarget(long nativeTexture);
    private static native int nGetInternalFormat(long nativeTexture);

    private static native int nSetImage3D(long nativeTexture, long nativeEngine,
            int level, int xoffset, int yoffset, int zoffset, int width, int height, int depth,
            Buffer storage, int remaining, int left, int top, int type, int alignment,
            int stride, int format,
            Object handler, Runnable callback);

    private static native int nSetImage3DCompressed(long nativeTexture, long nativeEngine,
            int level, int xoffset, int yoffset, int zoffset, int width, int height, int depth,
            Buffer storage, int remaining, int left, int top, int type, int alignment,
            int compressedSizeInBytes, int compressedFormat,
            Object handler, Runnable callback);

    private static native int nSetImageCubemap(long nativeTexture, long nativeEngine,
            int level, Buffer storage, int remaining, int left, int top, int type,
            int alignment, int stride, int format,
            int[] faceOffsetsInBytes, Object handler, Runnable callback);

    private static native int nSetImageCubemapCompressed(long nativeTexture, long nativeEngine,
            int level, Buffer storage, int remaining, int left, int top, int type,
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
