/*
 * Copyright (C) 2015 The Android Open Source Project
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

//! \file

#ifndef TNT_FILAMENT_DRIVER_DRIVERENUMS_H
#define TNT_FILAMENT_DRIVER_DRIVERENUMS_H

#include <utils/BitmaskEnum.h>
#include <utils/unwindows.h> // Because we define ERROR in the FenceStatus enum.

#include <math/vec4.h>

#include <array>    // FIXME: STL headers are not allowed in public headers

#include <stddef.h>
#include <stdint.h>

namespace filament {
/**
 * Types and enums used by filament's driver.
 *
 * Effectively these types are public but should not be used directly. Instead use public classes
 * internal redeclaration of these types.
 * For e.g. Use Texture::Sampler instead of filament::SamplerType.
 */
namespace backend {

static constexpr uint64_t SWAP_CHAIN_CONFIG_TRANSPARENT = 0x1;
static constexpr uint64_t SWAP_CHAIN_CONFIG_READABLE = 0x2;

static constexpr size_t MAX_VERTEX_ATTRIBUTE_COUNT = 16; // This is guaranteed by OpenGL ES.
static constexpr size_t MAX_SAMPLER_COUNT = 16;          // Matches the Adreno Vulkan driver.

static constexpr size_t CONFIG_UNIFORM_BINDING_COUNT = 6;
static constexpr size_t CONFIG_SAMPLER_BINDING_COUNT = 6;

/**
 * Selects which driver a particular Engine should use.
 */
enum class Backend : uint8_t {
    DEFAULT = 0,  //!< Automatically selects an appropriate driver for the platform.
    OPENGL = 1,   //!< Selects the OpenGL driver (which supports OpenGL ES as well).
    VULKAN = 2,   //!< Selects the Vulkan driver if the platform supports it.
    METAL = 3,    //!< Selects the Metal driver if the platform supports it.
    NOOP = 4,     //!< Selects the no-op driver for testing purposes.
};

/**
 * Bitmask for selecting render buffers
 */
enum class TargetBufferFlags : uint8_t {
    NONE = 0x0u,                            //!< No buffer selected.
    COLOR0 = 0x1u,                          //!< Color buffer selected.
    COLOR1 = 0x2u,                          //!< Color buffer selected.
    COLOR2 = 0x4u,                          //!< Color buffer selected.
    COLOR3 = 0x8u,                          //!< Color buffer selected.
    COLOR = COLOR0,                         //!< \deprecated
    COLOR_ALL = COLOR0 | COLOR1 | COLOR2 | COLOR3,
    DEPTH = 0x10u,                          //!< Depth buffer selected.
    STENCIL = 0x20u,                        //!< Stencil buffer selected.
    DEPTH_AND_STENCIL = DEPTH | STENCIL,    //!< depth and stencil buffer selected.
    ALL = COLOR_ALL | DEPTH | STENCIL       //!< Color, depth and stencil buffer selected.
};

inline TargetBufferFlags getMRTColorFlag(size_t index) noexcept {
    assert(index < 4);
    return TargetBufferFlags(1u << index);
}

/**
 * Frequency at which a buffer is expected to be modified and used. This is used as an hint
 * for the driver to make better decisions about managing memory internally.
 */
enum class BufferUsage : uint8_t {
    STATIC,      //!< content modified once, used many times
    DYNAMIC,     //!< content modified frequently, used many times
    STREAM,      //!< content invalidated and modified frequently, used many times
};

/**
 * Defines a viewport, which is the origin and extent of the clip-space.
 * All drawing is clipped to the viewport.
 */
struct Viewport {
    int32_t left;       //!< left coordinate in window space.
    int32_t bottom;     //!< bottom coordinate in window space.
    uint32_t width;     //!< width in pixels
    uint32_t height;    //!< height in pixels
    //! get the right coordinate in window space of the viewport
    int32_t right() const noexcept { return left + width; }
    //! get the top coordinate in window space of the viewport
    int32_t top() const noexcept { return bottom + height; }
};

/**
 * Error codes for Fence::wait()
 * @see Fence, Fence::wait()
 */
enum class FenceStatus : int8_t {
    ERROR = -1,                 //!< An error occured. The Fence condition is not satisfied.
    CONDITION_SATISFIED = 0,    //!< The Fence condition is satisfied.
    TIMEOUT_EXPIRED = 1,        //!< wait()'s timeout expired. The Fence condition is not satisfied.
};

/**
 * Status codes for sync objects
 */
enum class SyncStatus : int8_t {
    ERROR = -1,          //!< An error occured. The Sync is not signaled.
    SIGNALED = 0,        //!< The Sync is signaled.
    NOT_SIGNALED = 1,    //!< The Sync is not signaled yet
};

static constexpr uint64_t FENCE_WAIT_FOR_EVER = uint64_t(-1);

/**
 * Shader model.
 *
 * These enumerants are used across all backends and refer to a level of functionality, rather
 * than to an OpenGL specific shader model.
 */
enum class ShaderModel : uint8_t {
    //! For testing
    UNKNOWN    = 0,
    GL_ES_30   = 1,    //!< Mobile level functionality
    GL_CORE_41 = 2,    //!< Desktop level functionality
};
static constexpr size_t SHADER_MODEL_COUNT = 3;

/**
 * Primitive types
 */
enum class PrimitiveType : uint8_t {
    // don't change the enums values (made to match GL)
    POINTS      = 0,    //!< points
    LINES       = 1,    //!< lines
    TRIANGLES   = 4,    //!< triangles
    NONE        = 0xFF
};

/**
 * Supported uniform types
 */
enum class UniformType : uint8_t {
    BOOL,
    BOOL2,
    BOOL3,
    BOOL4,
    FLOAT,
    FLOAT2,
    FLOAT3,
    FLOAT4,
    INT,
    INT2,
    INT3,
    INT4,
    UINT,
    UINT2,
    UINT3,
    UINT4,
    MAT3,   //!< a 3x3 float matrix
    MAT4    //!< a 4x4 float matrix
};

enum class Precision : uint8_t {
    LOW,
    MEDIUM,
    HIGH,
    DEFAULT
};

//! Texture sampler type
enum class SamplerType : uint8_t {
    SAMPLER_2D,         //!< 2D texture
    SAMPLER_2D_ARRAY,   //!< 2D array texture
    SAMPLER_CUBEMAP,    //!< Cube map texture
    SAMPLER_EXTERNAL,   //!< External texture
};

//! Texture sampler format
enum class SamplerFormat : uint8_t {
    INT = 0,        //!< signed integer sampler
    UINT = 1,       //!< unsigned integer sampler
    FLOAT = 2,      //!< float sampler
    SHADOW = 3      //!< shadow sampler (PCF)
};

/**
 * Supported element types
 */
enum class ElementType : uint8_t {
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
};

//! Face culling Mode
enum class CullingMode : uint8_t {
    NONE,               //!< No culling, front and back faces are visible
    FRONT,              //!< Front face culling, only back faces are visible
    BACK,               //!< Back face culling, only front faces are visible
    FRONT_AND_BACK      //!< Front and Back, geometry is not visible
};

//! Pixel Data Format
enum class PixelDataFormat : uint8_t {
    R,                  //!< One Red channel, float
    R_INTEGER,          //!< One Red channel, integer
    RG,                 //!< Two Red and Green channels, float
    RG_INTEGER,         //!< Two Red and Green channels, integer
    RGB,                //!< Three Red, Green and Blue channels, float
    RGB_INTEGER,        //!< Three Red, Green and Blue channels, integer
    RGBA,               //!< Four Red, Green, Blue and Alpha channels, float
    RGBA_INTEGER,       //!< Four Red, Green, Blue and Alpha channels, integer
    UNUSED,             // used to be rgbm
    DEPTH_COMPONENT,    //!< Depth, 16-bit or 24-bits usually
    DEPTH_STENCIL,      //!< Two Depth (24-bits) + Stencil (8-bits) channels
    ALPHA               //! One Alpha channel, float
};

//! Pixel Data Type
enum class PixelDataType : uint8_t {
    UBYTE,                //!< unsigned byte
    BYTE,                 //!< signed byte
    USHORT,               //!< unsigned short (16-bit)
    SHORT,                //!< signed short (16-bit)
    UINT,                 //!< unsigned int (16-bit)
    INT,                  //!< signed int (32-bit)
    HALF,                 //!< half-float (16-bit float)
    FLOAT,                //!< float (32-bits float)
    COMPRESSED,           //!< compressed pixels, @see CompressedPixelDataType
    UINT_10F_11F_11F_REV, //!< three low precision floating-point numbers
    USHORT_565            //!< unsigned int (16-bit), encodes 3 RGB channels
};

//! Compressed pixel data types
enum class CompressedPixelDataType : uint16_t {
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
};

/** Supported texel formats
 * These formats are typically used to specify a texture's internal storage format.
 *
 * Enumerants syntax format
 * ========================
 *
 * `[components][size][type]`
 *
 * `components` : List of stored components by this format.\n
 * `size`       : Size in bit of each component.\n
 * `type`       : Type this format is stored as.\n
 *
 *
 * Name     | Component
 * :--------|:-------------------------------
 * R        | Linear Red
 * RG       | Linear Red, Green
 * RGB      | Linear Red, Green, Blue
 * RGBA     | Linear Red, Green Blue, Alpha
 * SRGB     | sRGB encoded Red, Green, Blue
 * DEPTH    | Depth
 * STENCIL  | Stencil
 *
 * \n
 * Name     | Type
 * :--------|:---------------------------------------------------
 * (none)   | Unsigned Normalized Integer [0, 1]
 * _SNORM   | Signed Normalized Integer [-1, 1]
 * UI       | Unsigned Integer @f$ [0, 2^{size}] @f$
 * I        | Signed Integer @f$ [-2^{size-1}, 2^{size-1}-1] @f$
 * F        | Floating-point
 *
 *
 * Special color formats
 * ---------------------
 *
 * There are a few special color formats that don't follow the convention above:
 *
 * Name             | Format
 * :----------------|:--------------------------------------------------------------------------
 * RGB565           |  5-bits for R and B, 6-bits for G.
 * RGB5_A1          |  5-bits for R, G and B, 1-bit for A.
 * RGB10_A2         | 10-bits for R, G and B, 2-bits for A.
 * RGB9_E5          | **Unsigned** floating point. 9-bits mantissa for RGB, 5-bits shared exponent
 * R11F_G11F_B10F   | **Unsigned** floating point. 6-bits mantissa, for R and G, 5-bits for B. 5-bits exponent.
 * SRGB8_A8         | sRGB 8-bits with linear 8-bits alpha.
 * DEPTH24_STENCIL8 | 24-bits unsigned normalized integer depth, 8-bits stencil.
 * DEPTH32F_STENCIL8| 32-bits floating-point depth, 8-bits stencil.
 *
 *
 * Compressed texture formats
 * --------------------------
 *
 * Many compressed texture formats are supported as well, which include (but are not limited to)
 * the following list:
 *
 * Name             | Format
 * :----------------|:--------------------------------------------------------------------------
 * EAC_R11          | Compresses R11UI
 * EAC_R11_SIGNED   | Compresses R11I
 * EAC_RG11         | Compresses RG11UI
 * EAC_RG11_SIGNED  | Compresses RG11I
 * ETC2_RGB8        | Compresses RGB8
 * ETC2_SRGB8       | compresses SRGB8
 * ETC2_EAC_RGBA8   | Compresses RGBA8
 * ETC2_EAC_SRGBA8  | Compresses SRGB8_A8
 * ETC2_RGB8_A1     | Compresses RGB8 with 1-bit alpha
 * ETC2_SRGB8_A1    | Compresses sRGB8 with 1-bit alpha
 *
 *
 * @see Texture
 */
enum class TextureFormat : uint16_t {
    // 8-bits per element
    R8, R8_SNORM, R8UI, R8I, STENCIL8,

    // 16-bits per element
    R16F, R16UI, R16I,
    RG8, RG8_SNORM, RG8UI, RG8I,
    RGB565,
    RGB9_E5, // 9995 is actually 32 bpp but it's here for historical reasons.
    RGB5_A1,
    RGBA4,
    DEPTH16,

    // 24-bits per element
    RGB8, SRGB8, RGB8_SNORM, RGB8UI, RGB8I,
    DEPTH24,

    // 32-bits per element
    R32F, R32UI, R32I,
    RG16F, RG16UI, RG16I,
    R11F_G11F_B10F,
    RGBA8, SRGB8_A8,RGBA8_SNORM,
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
};

//! Bitmask describing the intended Texture Usage
enum class TextureUsage : uint8_t {
    COLOR_ATTACHMENT    = 0x1,  //!< Texture can be used as a color attachment
    DEPTH_ATTACHMENT    = 0x2,  //!< Texture can be used as a depth attachment
    STENCIL_ATTACHMENT  = 0x4,  //!< Texture can be used as a stencil attachment
    UPLOADABLE          = 0x8,  //!< Data can be uploaded into this texture (default)
    SAMPLEABLE          = 0x10, //!< Texture can be sampled (default)
    DEFAULT = UPLOADABLE | SAMPLEABLE   //!< Default texture usage
};

//! Texture swizzle
enum class TextureSwizzle {
    SUBSTITUTE_ZERO,
    SUBSTITUTE_ONE,
    CHANNEL_0,
    CHANNEL_1,
    CHANNEL_2,
    CHANNEL_3
};

//! returns whether this format a compressed format
static constexpr bool isCompressedFormat(TextureFormat format) noexcept {
    return format >= TextureFormat::EAC_R11;
}

//! returns whether this format is an ETC2 compressed format
static constexpr bool isETC2Compression(TextureFormat format) noexcept {
    return format >= TextureFormat::EAC_R11 && format <= TextureFormat::ETC2_EAC_SRGBA8;
}

//! returns whether this format is an ETC3 compressed format
static constexpr bool isS3TCCompression(TextureFormat format) noexcept {
    return format >= TextureFormat::DXT1_RGB && format <= TextureFormat::DXT5_SRGBA;
}

static constexpr bool isS3TCSRGBCompression(TextureFormat format) noexcept {
    return format >= TextureFormat::DXT1_SRGB && format <= TextureFormat::DXT5_SRGBA;
}

//! Texture Cubemap Face
enum class TextureCubemapFace : uint8_t {
    // don't change the enums values
    POSITIVE_X = 0, //!< +x face
    NEGATIVE_X = 1, //!< -x face
    POSITIVE_Y = 2, //!< +y face
    NEGATIVE_Y = 3, //!< -y face
    POSITIVE_Z = 4, //!< +z face
    NEGATIVE_Z = 5, //!< -z face
};

//! Face offsets for all faces of a cubemap
struct FaceOffsets {
    using size_type = size_t;
    union {
        struct {
            size_type px;   //!< +x face offset in bytes
            size_type nx;   //!< -x face offset in bytes
            size_type py;   //!< +y face offset in bytes
            size_type ny;   //!< -y face offset in bytes
            size_type pz;   //!< +z face offset in bytes
            size_type nz;   //!< -z face offset in bytes
        };
        size_type offsets[6];
    };
    size_type  operator[](size_t n) const noexcept { return offsets[n]; }
    size_type& operator[](size_t n) { return offsets[n]; }
    FaceOffsets() noexcept = default;
    explicit FaceOffsets(size_type faceSize) noexcept {
        px = faceSize * 0;
        nx = faceSize * 1;
        py = faceSize * 2;
        ny = faceSize * 3;
        pz = faceSize * 4;
        nz = faceSize * 5;
    }
    FaceOffsets(const FaceOffsets& rhs) noexcept {
        px = rhs.px;
        nx = rhs.nx;
        py = rhs.py;
        ny = rhs.ny;
        pz = rhs.pz;
        nz = rhs.nz;
    }
    FaceOffsets& operator=(const FaceOffsets& rhs) noexcept {
        px = rhs.px;
        nx = rhs.nx;
        py = rhs.py;
        ny = rhs.ny;
        pz = rhs.pz;
        nz = rhs.nz;
        return *this;
    }
};

//! Sampler Wrap mode
enum class SamplerWrapMode : uint8_t {
    CLAMP_TO_EDGE,      //!< clamp-to-edge. The edge of the texture extends to infinity.
    REPEAT,             //!< repeat. The texture infinitely repeats in the wrap direction.
    MIRRORED_REPEAT,    //!< mirrored-repeat. The texture infinitely repeats and mirrors in the wrap direction.
};

//! Sampler minification filter
enum class SamplerMinFilter : uint8_t {
    // don't change the enums values
    NEAREST = 0,                //!< No filtering. Nearest neighbor is used.
    LINEAR = 1,                 //!< Box filtering. Weighted average of 4 neighbors is used.
    NEAREST_MIPMAP_NEAREST = 2, //!< Mip-mapping is activated. But no filtering occurs.
    LINEAR_MIPMAP_NEAREST = 3,  //!< Box filtering within a mip-map level.
    NEAREST_MIPMAP_LINEAR = 4,  //!< Mip-map levels are interpolated, but no other filtering occurs.
    LINEAR_MIPMAP_LINEAR = 5    //!< Both interpolated Mip-mapping and linear filtering are used.
};

//! Sampler magnification filter
enum class SamplerMagFilter : uint8_t {
    // don't change the enums values
    NEAREST = 0,                //!< No filtering. Nearest neighbor is used.
    LINEAR = 1,                 //!< Box filtering. Weighted average of 4 neighbors is used.
};

//! Sampler compare mode
enum class SamplerCompareMode : uint8_t {
    // don't change the enums values
    NONE = 0,
    COMPARE_TO_TEXTURE = 1
};

//! comparison function for the depth sampler
enum class SamplerCompareFunc : uint8_t {
    // don't change the enums values
    LE = 0,     //!< Less or equal
    GE,         //!< Greater or equal
    L,          //!< Strictly less than
    G,          //!< Strictly greater than
    E,          //!< Equal
    NE,         //!< Not equal
    A,          //!< Always. Depth testing is deactivated.
    N           //!< Never. The depth test always fails.
};

//! Sampler paramters
struct SamplerParams { // NOLINT
    union {
        struct {
            SamplerMagFilter filterMag      : 1;    //!< magnification filter (NEAREST)
            SamplerMinFilter filterMin      : 3;    //!< minification filter  (NEAREST)
            SamplerWrapMode wrapS           : 2;    //!< s-coordinate wrap mode (CLAMP_TO_EDGE)
            SamplerWrapMode wrapT           : 2;    //!< t-coordinate wrap mode (CLAMP_TO_EDGE)

            SamplerWrapMode wrapR           : 2;    //!< r-coordinate wrap mode (CLAMP_TO_EDGE)
            uint8_t anisotropyLog2          : 3;    //!< anisotropy level (0)
            SamplerCompareMode compareMode  : 1;    //!< sampler compare mode (NONE)
            uint8_t padding0                : 2;    //!< reserved. must be 0.

            SamplerCompareFunc compareFunc  : 3;    //!< sampler comparison function (LE)
            uint8_t padding1                : 5;    //!< reserved. must be 0.

            uint8_t padding2                : 8;    //!< reserved. must be 0.
        };
        uint32_t u;
    };
private:
    friend inline bool operator < (SamplerParams lhs, SamplerParams rhs) {
        return lhs.u < rhs.u;
    }
};

static_assert(sizeof(SamplerParams) == sizeof(uint32_t), "SamplerParams must be 32 bits");

//! blending equation function
enum class BlendEquation : uint8_t {
    ADD,                    //!< the fragment is added to the color buffer
    SUBTRACT,               //!< the fragment is subtracted from the color buffer
    REVERSE_SUBTRACT,       //!< the color buffer is subtracted from the fragment
    MIN,                    //!< the min between the fragment and color buffer
    MAX                     //!< the max between the fragment and color buffer
};

//! blending function
enum class BlendFunction : uint8_t {
    ZERO,                   //!< f(src, dst) = 0
    ONE,                    //!< f(src, dst) = 1
    SRC_COLOR,              //!< f(src, dst) = src
    ONE_MINUS_SRC_COLOR,    //!< f(src, dst) = 1-src
    DST_COLOR,              //!< f(src, dst) = dst
    ONE_MINUS_DST_COLOR,    //!< f(src, dst) = 1-dst
    SRC_ALPHA,              //!< f(src, dst) = src.a
    ONE_MINUS_SRC_ALPHA,    //!< f(src, dst) = 1-src.a
    DST_ALPHA,              //!< f(src, dst) = dst.a
    ONE_MINUS_DST_ALPHA,    //!< f(src, dst) = 1-dst.a
    SRC_ALPHA_SATURATE      //!< f(src, dst) = (1,1,1) * min(src.a, 1 - dst.a), 1
};

//! Stream for external textures
enum class StreamType {
    NATIVE,     //!< Not synchronized but copy-free. Good for video.
    TEXTURE_ID, //!< Synchronized, but GL-only and incurs copies. Good for AR on devices before API 26.
    ACQUIRED,   //!< Synchronized, copy-free, and take a release callback. Good for AR but requires API 26+.
};

//! Releases an ACQUIRED external texture, guaranteed to be called on the application thread.
using StreamCallback = void(*)(void* image, void* user);

//! Vertex attribute descriptor
struct Attribute {
    //! attribute is normalized (remapped between 0 and 1)
    static constexpr uint8_t FLAG_NORMALIZED     = 0x1;
    //! attribute is an integer
    static constexpr uint8_t FLAG_INTEGER_TARGET = 0x2;
    uint32_t offset = 0;                    //!< attribute offset in bytes
    uint8_t stride = 0;                     //!< attribute stride in bytes
    uint8_t buffer = 0xFF;                  //!< attribute buffer index
    ElementType type = ElementType::BYTE;   //!< attribute element type
    uint8_t flags = 0x0;                    //!< attribute flags
};

using AttributeArray = std::array<Attribute, MAX_VERTEX_ATTRIBUTE_COUNT>;

//! Raster state descriptor
struct RasterState {

    using CullingMode = filament::backend::CullingMode;
    using DepthFunc = filament::backend::SamplerCompareFunc;
    using BlendEquation = filament::backend::BlendEquation;
    using BlendFunction = filament::backend::BlendFunction;

    RasterState() noexcept { // NOLINT
        static_assert(sizeof(RasterState) == sizeof(uint32_t),
                "RasterState size not what was intended");
        culling = CullingMode::BACK;
        blendEquationRGB = BlendEquation::ADD;
        blendEquationAlpha = BlendEquation::ADD;
        blendFunctionSrcRGB = BlendFunction::ONE;
        blendFunctionSrcAlpha = BlendFunction::ONE;
        blendFunctionDstRGB = BlendFunction::ZERO;
        blendFunctionDstAlpha = BlendFunction::ZERO;
    }

    bool operator == (RasterState rhs) const noexcept { return u == rhs.u; }
    bool operator != (RasterState rhs) const noexcept { return u != rhs.u; }

    void disableBlending() noexcept {
        blendEquationRGB = BlendEquation::ADD;
        blendEquationAlpha = BlendEquation::ADD;
        blendFunctionSrcRGB = BlendFunction::ONE;
        blendFunctionSrcAlpha = BlendFunction::ONE;
        blendFunctionDstRGB = BlendFunction::ZERO;
        blendFunctionDstAlpha = BlendFunction::ZERO;
    }

    // note: clang reduces this entire function to a simple load/mask/compare
    bool hasBlending() const noexcept {
        // there could be other cases where blending would end-up being disabled,
        // but this is common and easy to check
        return !(blendEquationRGB == BlendEquation::ADD &&
                 blendEquationAlpha == BlendEquation::ADD &&
                 blendFunctionSrcRGB == BlendFunction::ONE &&
                 blendFunctionSrcAlpha == BlendFunction::ONE &&
                 blendFunctionDstRGB == BlendFunction::ZERO &&
                 blendFunctionDstAlpha == BlendFunction::ZERO);
    }

    union {
        struct {
            //! culling mode
            CullingMode culling                 : 2;        //  2

            //! blend equation for the red, green and blue components
            BlendEquation blendEquationRGB      : 3;        //  5
            //! blend equation for the alpha component
            BlendEquation blendEquationAlpha    : 3;        //  8

            //! blending function for the source color
            BlendFunction blendFunctionSrcRGB   : 4;        // 12
            //! blending function for the source alpha
            BlendFunction blendFunctionSrcAlpha : 4;        // 16
            //! blending function for the destination color
            BlendFunction blendFunctionDstRGB   : 4;        // 20
            //! blending function for the destination alpha
            BlendFunction blendFunctionDstAlpha : 4;        // 24

            //! Whether depth-buffer writes are enabled
            bool depthWrite                     : 1;        // 25
            //! Depth test function
            DepthFunc depthFunc                 : 3;        // 28

            //! Whether color-buffer writes are enabled
            bool colorWrite                     : 1;        // 29

            //! use alpha-channel as coverage mask for anti-aliasing
            bool alphaToCoverage                : 1;        // 30

            //! whether front face winding direction must be inverted
            bool inverseFrontFaces              : 1;        // 31

            //! padding, must be 0
            uint8_t padding                     : 1;        // 32
        };
        uint32_t u = 0;
    };
};

/**
 **********************************************************************************************
 * \privatesection
 */

enum ShaderType : uint8_t {
    VERTEX = 0,
    FRAGMENT = 1
};
static constexpr size_t PIPELINE_STAGE_COUNT = 2;

/**
 * Selects which buffers to clear at the beginning of the render pass, as well as which buffers
 * can be discarded at the beginning and end of the render pass.
 *
 */
struct RenderPassFlags {
    /**
     * bitmask indicating which buffers to clear at the beginning of a render pass.
     * This implies discard.
     */
    TargetBufferFlags clear;

    /**
     * bitmask indicating which buffers to discard at the beginning of a render pass.
     * Discarded buffers have uninitialized content, they must be entirely drawn over or cleared.
     */
    TargetBufferFlags discardStart;

    /**
     * bitmask indicating which buffers to discard at the end of a render pass.
     * Discarded buffers' content becomes invalid, they must not be read from again.
     */
    TargetBufferFlags discardEnd;
};

/**
 * Parameters of a render pass.
 */
struct RenderPassParams {
    RenderPassFlags flags{};    //!< operations performed on the buffers for this pass

    Viewport viewport{};        //!< viewport for this pass

    //! Color to use to clear the COLOR buffer. RenderPassFlags::clear must be set.
    filament::math::float4 clearColor = {};

    //! Depth value to clear the depth buffer with
    double clearDepth = 1.0;

    //! Stencil value to clear the stencil buffer with
    uint32_t clearStencil = 0;

    //! reserved, must be zero
    uint32_t reserved1 = 0;
};

struct PolygonOffset {
    float slope = 0;        // factor in GL-speak
    float constant = 0;     // units in GL-speak
};


} // namespace backend
} // namespace filament

template<> struct utils::EnableBitMaskOperators<filament::backend::TargetBufferFlags>
        : public std::true_type {};
template<> struct utils::EnableBitMaskOperators<filament::backend::TextureUsage>
        : public std::true_type {};

#endif // TNT_FILAMENT_DRIVER_DRIVERENUMS_H
