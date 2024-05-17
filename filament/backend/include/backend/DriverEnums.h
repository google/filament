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

#ifndef TNT_FILAMENT_BACKEND_DRIVERENUMS_H
#define TNT_FILAMENT_BACKEND_DRIVERENUMS_H

#include <utils/BitmaskEnum.h>
#include <utils/unwindows.h> // Because we define ERROR in the FenceStatus enum.

#include <backend/Platform.h>
#include <backend/PresentCallable.h>

#include <utils/Invocable.h>
#include <utils/ostream.h>

#include <math/vec4.h>

#include <array>        // FIXME: STL headers are not allowed in public headers
#include <type_traits>  // FIXME: STL headers are not allowed in public headers
#include <variant>      // FIXME: STL headers are not allowed in public headers

#include <stddef.h>
#include <stdint.h>

/**
 * Types and enums used by filament's driver.
 *
 * Effectively these types are public but should not be used directly. Instead use public classes
 * internal redeclaration of these types.
 * For e.g. Use Texture::Sampler instead of filament::SamplerType.
 */
namespace filament::backend {

/**
 * Requests a SwapChain with an alpha channel.
 */
static constexpr uint64_t SWAP_CHAIN_CONFIG_TRANSPARENT         = 0x1;

/**
 * This flag indicates that the swap chain may be used as a source surface
 * for reading back render results.  This config flag must be set when creating
 * any SwapChain that will be used as the source for a blit operation.
 */
static constexpr uint64_t SWAP_CHAIN_CONFIG_READABLE            = 0x2;

/**
 * Indicates that the native X11 window is an XCB window rather than an XLIB window.
 * This is ignored on non-Linux platforms and in builds that support only one X11 API.
 */
static constexpr uint64_t SWAP_CHAIN_CONFIG_ENABLE_XCB          = 0x4;

/**
 * Indicates that the native window is a CVPixelBufferRef.
 *
 * This is only supported by the Metal backend. The CVPixelBuffer must be in the
 * kCVPixelFormatType_32BGRA format.
 *
 * It is not necessary to add an additional retain call before passing the pixel buffer to
 * Filament. Filament will call CVPixelBufferRetain during Engine::createSwapChain, and
 * CVPixelBufferRelease when the swap chain is destroyed.
 */
static constexpr uint64_t SWAP_CHAIN_CONFIG_APPLE_CVPIXELBUFFER = 0x8;

/**
 * Indicates that the SwapChain must automatically perform linear to srgb encoding.
 */
static constexpr uint64_t SWAP_CHAIN_CONFIG_SRGB_COLORSPACE     = 0x10;

/**
 * Indicates that the SwapChain should also contain a stencil component.
 */
static constexpr uint64_t SWAP_CHAIN_CONFIG_HAS_STENCIL_BUFFER  = 0x20;
static constexpr uint64_t SWAP_CHAIN_HAS_STENCIL_BUFFER         = SWAP_CHAIN_CONFIG_HAS_STENCIL_BUFFER;

/**
 * The SwapChain contains protected content. Currently only supported by OpenGLPlatform and
 * only when OpenGLPlatform::isProtectedContextSupported() is true.
 */
static constexpr uint64_t SWAP_CHAIN_CONFIG_PROTECTED_CONTENT   = 0x40;

static constexpr size_t MAX_VERTEX_ATTRIBUTE_COUNT  = 16;   // This is guaranteed by OpenGL ES.
static constexpr size_t MAX_SAMPLER_COUNT           = 62;   // Maximum needed at feature level 3.
static constexpr size_t MAX_VERTEX_BUFFER_COUNT     = 16;   // Max number of bound buffer objects.
static constexpr size_t MAX_SSBO_COUNT              = 4;    // This is guaranteed by OpenGL ES.

static constexpr size_t MAX_PUSH_CONSTANT_COUNT     = 32;   // Vulkan 1.1 spec allows for 128-byte
                                                            // of push constant (we assume 4-byte
                                                            // types).

// Per feature level caps
// Use (int)FeatureLevel to index this array
static constexpr struct {
    const size_t MAX_VERTEX_SAMPLER_COUNT;
    const size_t MAX_FRAGMENT_SAMPLER_COUNT;
} FEATURE_LEVEL_CAPS[4] = {
        {  0,  0 }, // do not use
        { 16, 16 }, // guaranteed by OpenGL ES, Vulkan and Metal
        { 16, 16 }, // guaranteed by OpenGL ES, Vulkan and Metal
        { 31, 31 }, // guaranteed by Metal
};

static_assert(MAX_VERTEX_BUFFER_COUNT <= MAX_VERTEX_ATTRIBUTE_COUNT,
        "The number of buffer objects that can be attached to a VertexBuffer must be "
        "less than or equal to the maximum number of vertex attributes.");

static constexpr size_t CONFIG_UNIFORM_BINDING_COUNT = 9;   // This is guaranteed by OpenGL ES.
static constexpr size_t CONFIG_SAMPLER_BINDING_COUNT = 4;   // This is guaranteed by OpenGL ES.

/**
 * Defines the backend's feature levels.
 */
enum class FeatureLevel : uint8_t {
    FEATURE_LEVEL_0 = 0,  //!< OpenGL ES 2.0 features
    FEATURE_LEVEL_1,      //!< OpenGL ES 3.0 features (default)
    FEATURE_LEVEL_2,      //!< OpenGL ES 3.1 features + 16 textures units + cubemap arrays
    FEATURE_LEVEL_3       //!< OpenGL ES 3.1 features + 31 textures units + cubemap arrays
};

/**
 * Selects which driver a particular Engine should use.
 */
enum class Backend : uint8_t {
    DEFAULT = 0,  //!< Automatically selects an appropriate driver for the platform.
    OPENGL = 1,   //!< Selects the OpenGL/ES driver (default on Android)
    VULKAN = 2,   //!< Selects the Vulkan driver if the platform supports it (default on Linux/Windows)
    METAL = 3,    //!< Selects the Metal driver if the platform supports it (default on MacOS/iOS).
    NOOP = 4,     //!< Selects the no-op driver for testing purposes.
};

enum class TimerQueryResult : int8_t {
    ERROR = -1,     // an error occurred, result won't be available
    NOT_READY = 0,  // result to ready yet
    AVAILABLE = 1,  // result is available
};

static constexpr const char* backendToString(Backend backend) {
    switch (backend) {
        case Backend::NOOP:
            return "Noop";
        case Backend::OPENGL:
            return "OpenGL";
        case Backend::VULKAN:
            return "Vulkan";
        case Backend::METAL:
            return "Metal";
        default:
            return "Unknown";
    }
}

/**
 * Defines the shader language. Similar to the above backend enum, with some differences:
 * - The OpenGL backend can select between two shader languages: ESSL 1.0 and ESSL 3.0.
 * - The Metal backend can prefer precompiled Metal libraries, while falling back to MSL.
 */
enum class ShaderLanguage {
    ESSL1 = 0,
    ESSL3 = 1,
    SPIRV = 2,
    MSL = 3,
    METAL_LIBRARY = 4,
};

static constexpr const char* shaderLanguageToString(ShaderLanguage shaderLanguage) {
    switch (shaderLanguage) {
        case ShaderLanguage::ESSL1:
            return "ESSL 1.0";
        case ShaderLanguage::ESSL3:
            return "ESSL 3.0";
        case ShaderLanguage::SPIRV:
            return "SPIR-V";
        case ShaderLanguage::MSL:
            return "MSL";
        case ShaderLanguage::METAL_LIBRARY:
            return "Metal precompiled library";
    }
}

/**
 * Bitmask for selecting render buffers
 */
enum class TargetBufferFlags : uint32_t {
    NONE = 0x0u,                            //!< No buffer selected.
    COLOR0 = 0x00000001u,                   //!< Color buffer selected.
    COLOR1 = 0x00000002u,                   //!< Color buffer selected.
    COLOR2 = 0x00000004u,                   //!< Color buffer selected.
    COLOR3 = 0x00000008u,                   //!< Color buffer selected.
    COLOR4 = 0x00000010u,                   //!< Color buffer selected.
    COLOR5 = 0x00000020u,                   //!< Color buffer selected.
    COLOR6 = 0x00000040u,                   //!< Color buffer selected.
    COLOR7 = 0x00000080u,                   //!< Color buffer selected.

    COLOR = COLOR0,                         //!< \deprecated
    COLOR_ALL = COLOR0 | COLOR1 | COLOR2 | COLOR3 | COLOR4 | COLOR5 | COLOR6 | COLOR7,
    DEPTH   = 0x10000000u,                  //!< Depth buffer selected.
    STENCIL = 0x20000000u,                  //!< Stencil buffer selected.
    DEPTH_AND_STENCIL = DEPTH | STENCIL,    //!< depth and stencil buffer selected.
    ALL = COLOR_ALL | DEPTH | STENCIL       //!< Color, depth and stencil buffer selected.
};

inline constexpr TargetBufferFlags getTargetBufferFlagsAt(size_t index) noexcept {
    if (index == 0u) return TargetBufferFlags::COLOR0;
    if (index == 1u) return TargetBufferFlags::COLOR1;
    if (index == 2u) return TargetBufferFlags::COLOR2;
    if (index == 3u) return TargetBufferFlags::COLOR3;
    if (index == 4u) return TargetBufferFlags::COLOR4;
    if (index == 5u) return TargetBufferFlags::COLOR5;
    if (index == 6u) return TargetBufferFlags::COLOR6;
    if (index == 7u) return TargetBufferFlags::COLOR7;
    if (index == 8u) return TargetBufferFlags::DEPTH;
    if (index == 9u) return TargetBufferFlags::STENCIL;
    return TargetBufferFlags::NONE;
}

/**
 * Frequency at which a buffer is expected to be modified and used. This is used as an hint
 * for the driver to make better decisions about managing memory internally.
 */
enum class BufferUsage : uint8_t {
    STATIC,      //!< content modified once, used many times
    DYNAMIC,     //!< content modified frequently, used many times
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
    int32_t right() const noexcept { return left + int32_t(width); }
    //! get the top coordinate in window space of the viewport
    int32_t top() const noexcept { return bottom + int32_t(height); }
};


/**
 * Specifies the mapping of the near and far clipping plane to window coordinates.
 */
struct DepthRange {
    float near = 0.0f;    //!< mapping of the near plane to window coordinates.
    float far = 1.0f;     //!< mapping of the far plane to window coordinates.
};

/**
 * Error codes for Fence::wait()
 * @see Fence, Fence::wait()
 */
enum class FenceStatus : int8_t {
    ERROR = -1,                 //!< An error occurred. The Fence condition is not satisfied.
    CONDITION_SATISFIED = 0,    //!< The Fence condition is satisfied.
    TIMEOUT_EXPIRED = 1,        //!< wait()'s timeout expired. The Fence condition is not satisfied.
};

/**
 * Status codes for sync objects
 */
enum class SyncStatus : int8_t {
    ERROR = -1,          //!< An error occurred. The Sync is not signaled.
    SIGNALED = 0,        //!< The Sync is signaled.
    NOT_SIGNALED = 1,    //!< The Sync is not signaled yet
};

static constexpr uint64_t FENCE_WAIT_FOR_EVER = uint64_t(-1);

/**
 * Shader model.
 *
 * These enumerants are used across all backends and refer to a level of functionality and quality.
 *
 * For example, the OpenGL backend returns `MOBILE` if it supports OpenGL ES, or `DESKTOP` if it
 * supports Desktop OpenGL, this is later used to select the proper shader.
 *
 * Shader quality vs. performance is also affected by ShaderModel.
 */
enum class ShaderModel : uint8_t {
    MOBILE  = 1,    //!< Mobile level functionality
    DESKTOP = 2,    //!< Desktop level functionality
};
static constexpr size_t SHADER_MODEL_COUNT = 2;

/**
 * Primitive types
 */
enum class PrimitiveType : uint8_t {
    // don't change the enums values (made to match GL)
    POINTS         = 0,    //!< points
    LINES          = 1,    //!< lines
    LINE_STRIP     = 3,    //!< line strip
    TRIANGLES      = 4,    //!< triangles
    TRIANGLE_STRIP = 5     //!< triangle strip
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
    MAT4,   //!< a 4x4 float matrix
    STRUCT
};

/**
 * Supported constant parameter types
 */
enum class ConstantType : uint8_t {
  INT,
  FLOAT,
  BOOL
};

enum class Precision : uint8_t {
    LOW,
    MEDIUM,
    HIGH,
    DEFAULT
};

/**
 * Shader compiler priority queue
 */
enum class CompilerPriorityQueue : uint8_t {
    HIGH,
    LOW
};

//! Texture sampler type
enum class SamplerType : uint8_t {
    SAMPLER_2D,             //!< 2D texture
    SAMPLER_2D_ARRAY,       //!< 2D array texture
    SAMPLER_CUBEMAP,        //!< Cube map texture
    SAMPLER_EXTERNAL,       //!< External texture
    SAMPLER_3D,             //!< 3D texture
    SAMPLER_CUBEMAP_ARRAY,  //!< Cube map array texture (feature level 2)
};

//! Subpass type
enum class SubpassType : uint8_t {
    SUBPASS_INPUT
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

//! Buffer object binding type
enum class BufferObjectBinding : uint8_t {
    VERTEX,
    UNIFORM,
    SHADER_STORAGE
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
    UINT,                 //!< unsigned int (32-bit)
    INT,                  //!< signed int (32-bit)
    HALF,                 //!< half-float (16-bit float)
    FLOAT,                //!< float (32-bits float)
    COMPRESSED,           //!< compressed pixels, @see CompressedPixelDataType
    UINT_10F_11F_11F_REV, //!< three low precision floating-point numbers
    USHORT_565,           //!< unsigned int (16-bit), encodes 3 RGB channels
    UINT_2_10_10_10_REV,  //!< unsigned normalized 10 bits RGB, 2 bits alpha
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

    // RGTC formats available with a GLES extension
    RED_RGTC1,              // BC4 unsigned
    SIGNED_RED_RGTC1,       // BC4 signed
    RED_GREEN_RGTC2,        // BC5 unsigned
    SIGNED_RED_GREEN_RGTC2, // BC5 signed

    // BPTC formats available with a GLES extension
    RGB_BPTC_SIGNED_FLOAT,  // BC6H signed
    RGB_BPTC_UNSIGNED_FLOAT,// BC6H unsigned
    RGBA_BPTC_UNORM,        // BC7
    SRGB_ALPHA_BPTC_UNORM,  // BC7 sRGB
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

    // RGTC formats available with a GLES extension
    RED_RGTC1,              // BC4 unsigned
    SIGNED_RED_RGTC1,       // BC4 signed
    RED_GREEN_RGTC2,        // BC5 unsigned
    SIGNED_RED_GREEN_RGTC2, // BC5 signed

    // BPTC formats available with a GLES extension
    RGB_BPTC_SIGNED_FLOAT,  // BC6H signed
    RGB_BPTC_UNSIGNED_FLOAT,// BC6H unsigned
    RGBA_BPTC_UNORM,        // BC7
    SRGB_ALPHA_BPTC_UNORM,  // BC7 sRGB
};

//! Bitmask describing the intended Texture Usage
enum class TextureUsage : uint16_t {
    NONE                = 0x0000,
    COLOR_ATTACHMENT    = 0x0001,            //!< Texture can be used as a color attachment
    DEPTH_ATTACHMENT    = 0x0002,            //!< Texture can be used as a depth attachment
    STENCIL_ATTACHMENT  = 0x0004,            //!< Texture can be used as a stencil attachment
    UPLOADABLE          = 0x0008,            //!< Data can be uploaded into this texture (default)
    SAMPLEABLE          = 0x0010,            //!< Texture can be sampled (default)
    SUBPASS_INPUT       = 0x0020,            //!< Texture can be used as a subpass input
    BLIT_SRC            = 0x0040,            //!< Texture can be used the source of a blit()
    BLIT_DST            = 0x0080,            //!< Texture can be used the destination of a blit()
    PROTECTED           = 0x0100,            //!< Texture can be used for protected content
    DEFAULT             = UPLOADABLE | SAMPLEABLE   //!< Default texture usage
};

//! Texture swizzle
enum class TextureSwizzle : uint8_t {
    SUBSTITUTE_ZERO,
    SUBSTITUTE_ONE,
    CHANNEL_0,
    CHANNEL_1,
    CHANNEL_2,
    CHANNEL_3
};

//! returns whether this format a depth format
static constexpr bool isDepthFormat(TextureFormat format) noexcept {
    switch (format) {
        case TextureFormat::DEPTH32F:
        case TextureFormat::DEPTH24:
        case TextureFormat::DEPTH16:
        case TextureFormat::DEPTH32F_STENCIL8:
        case TextureFormat::DEPTH24_STENCIL8:
            return true;
        default:
            return false;
    }
}

static constexpr bool isStencilFormat(TextureFormat format) noexcept {
    switch (format) {
        case TextureFormat::STENCIL8:
        case TextureFormat::DEPTH24_STENCIL8:
        case TextureFormat::DEPTH32F_STENCIL8:
            return true;
        default:
            return false;
    }
}

static constexpr bool isUnsignedIntFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::R8UI:
        case TextureFormat::R16UI:
        case TextureFormat::R32UI:
        case TextureFormat::RG8UI:
        case TextureFormat::RG16UI:
        case TextureFormat::RG32UI:
        case TextureFormat::RGB8UI:
        case TextureFormat::RGB16UI:
        case TextureFormat::RGB32UI:
        case TextureFormat::RGBA8UI:
        case TextureFormat::RGBA16UI:
        case TextureFormat::RGBA32UI:
            return true;

        default:
            return false;
    }
}

static constexpr bool isSignedIntFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::R8I:
        case TextureFormat::R16I:
        case TextureFormat::R32I:
        case TextureFormat::RG8I:
        case TextureFormat::RG16I:
        case TextureFormat::RG32I:
        case TextureFormat::RGB8I:
        case TextureFormat::RGB16I:
        case TextureFormat::RGB32I:
        case TextureFormat::RGBA8I:
        case TextureFormat::RGBA16I:
        case TextureFormat::RGBA32I:
            return true;

        default:
            return false;
    }
}

//! returns whether this format is a compressed format
static constexpr bool isCompressedFormat(TextureFormat format) noexcept {
    return format >= TextureFormat::EAC_R11;
}

//! returns whether this format is an ETC2 compressed format
static constexpr bool isETC2Compression(TextureFormat format) noexcept {
    return format >= TextureFormat::EAC_R11 && format <= TextureFormat::ETC2_EAC_SRGBA8;
}

//! returns whether this format is an S3TC compressed format
static constexpr bool isS3TCCompression(TextureFormat format) noexcept {
    return format >= TextureFormat::DXT1_RGB && format <= TextureFormat::DXT5_SRGBA;
}

static constexpr bool isS3TCSRGBCompression(TextureFormat format) noexcept {
    return format >= TextureFormat::DXT1_SRGB && format <= TextureFormat::DXT5_SRGBA;
}

//! returns whether this format is an RGTC compressed format
static constexpr bool isRGTCCompression(TextureFormat format) noexcept {
    return format >= TextureFormat::RED_RGTC1 && format <= TextureFormat::SIGNED_RED_GREEN_RGTC2;
}

//! returns whether this format is an BPTC compressed format
static constexpr bool isBPTCCompression(TextureFormat format) noexcept {
    return format >= TextureFormat::RGB_BPTC_SIGNED_FLOAT && format <= TextureFormat::SRGB_ALPHA_BPTC_UNORM;
}

static constexpr bool isASTCCompression(TextureFormat format) noexcept {
    return format >= TextureFormat::RGBA_ASTC_4x4 && format <= TextureFormat::SRGB8_ALPHA8_ASTC_12x12;
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

//! comparison function for the depth / stencil sampler
enum class SamplerCompareFunc : uint8_t {
    // don't change the enums values
    LE = 0,     //!< Less or equal
    GE,         //!< Greater or equal
    L,          //!< Strictly less than
    G,          //!< Strictly greater than
    E,          //!< Equal
    NE,         //!< Not equal
    A,          //!< Always. Depth / stencil testing is deactivated.
    N           //!< Never. The depth / stencil test always fails.
};

//! Sampler parameters
struct SamplerParams { // NOLINT
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

    struct Hasher {
        size_t operator()(SamplerParams p) const noexcept {
            // we don't use std::hash<> here, so we don't have to include <functional>
            return *reinterpret_cast<uint32_t const*>(reinterpret_cast<char const*>(&p));
        }
    };

    struct EqualTo {
        bool operator()(SamplerParams lhs, SamplerParams rhs) const noexcept {
            auto* pLhs = reinterpret_cast<uint32_t const*>(reinterpret_cast<char const*>(&lhs));
            auto* pRhs = reinterpret_cast<uint32_t const*>(reinterpret_cast<char const*>(&rhs));
            return *pLhs == *pRhs;
        }
    };

    struct LessThan {
        bool operator()(SamplerParams lhs, SamplerParams rhs) const noexcept {
            auto* pLhs = reinterpret_cast<uint32_t const*>(reinterpret_cast<char const*>(&lhs));
            auto* pRhs = reinterpret_cast<uint32_t const*>(reinterpret_cast<char const*>(&rhs));
            return *pLhs == *pRhs;
        }
    };

private:
    friend inline bool operator < (SamplerParams lhs, SamplerParams rhs) noexcept {
        return SamplerParams::LessThan{}(lhs, rhs);
    }
};
static_assert(sizeof(SamplerParams) == 4);

// The limitation to 64-bits max comes from how we store a SamplerParams in our JNI code
// see android/.../TextureSampler.cpp
static_assert(sizeof(SamplerParams) <= sizeof(uint64_t),
        "SamplerParams must be no more than 64 bits");

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

//! stencil operation
enum class StencilOperation : uint8_t {
    KEEP,                   //!< Keeps the current value.
    ZERO,                   //!< Sets the value to 0.
    REPLACE,                //!< Sets the value to the stencil reference value.
    INCR,                   //!< Increments the current value. Clamps to the maximum representable unsigned value.
    INCR_WRAP,              //!< Increments the current value. Wraps value to zero when incrementing the maximum representable unsigned value.
    DECR,                   //!< Decrements the current value. Clamps to 0.
    DECR_WRAP,              //!< Decrements the current value. Wraps value to the maximum representable unsigned value when decrementing a value of zero.
    INVERT,                 //!< Bitwise inverts the current value.
};

//! stencil faces
enum class StencilFace : uint8_t {
    FRONT               = 0x1,              //!< Update stencil state for front-facing polygons.
    BACK                = 0x2,              //!< Update stencil state for back-facing polygons.
    FRONT_AND_BACK      = FRONT | BACK,     //!< Update stencil state for all polygons.
};

//! Stream for external textures
enum class StreamType {
    NATIVE,     //!< Not synchronized but copy-free. Good for video.
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
    static constexpr uint8_t BUFFER_UNUSED = 0xFF;
    uint32_t offset = 0;                    //!< attribute offset in bytes
    uint8_t stride = 0;                     //!< attribute stride in bytes
    uint8_t buffer = BUFFER_UNUSED;         //!< attribute buffer index
    ElementType type = ElementType::BYTE;   //!< attribute element type
    uint8_t flags = 0x0;                    //!< attribute flags
};

using AttributeArray = std::array<Attribute, MAX_VERTEX_ATTRIBUTE_COUNT>;

//! Raster state descriptor
struct RasterState {

    using CullingMode = backend::CullingMode;
    using DepthFunc = backend::SamplerCompareFunc;
    using BlendEquation = backend::BlendEquation;
    using BlendFunction = backend::BlendFunction;

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
        // This is used to decide if blending needs to be enabled in the h/w
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
            CullingMode culling                         : 2;        //  2

            //! blend equation for the red, green and blue components
            BlendEquation blendEquationRGB              : 3;        //  5
            //! blend equation for the alpha component
            BlendEquation blendEquationAlpha            : 3;        //  8

            //! blending function for the source color
            BlendFunction blendFunctionSrcRGB           : 4;        // 12
            //! blending function for the source alpha
            BlendFunction blendFunctionSrcAlpha         : 4;        // 16
            //! blending function for the destination color
            BlendFunction blendFunctionDstRGB           : 4;        // 20
            //! blending function for the destination alpha
            BlendFunction blendFunctionDstAlpha         : 4;        // 24

            //! Whether depth-buffer writes are enabled
            bool depthWrite                             : 1;        // 25
            //! Depth test function
            DepthFunc depthFunc                         : 3;        // 28

            //! Whether color-buffer writes are enabled
            bool colorWrite                             : 1;        // 29

            //! use alpha-channel as coverage mask for anti-aliasing
            bool alphaToCoverage                        : 1;        // 30

            //! whether front face winding direction must be inverted
            bool inverseFrontFaces                      : 1;        // 31

            //! padding, must be 0
            uint8_t padding                             : 1;        // 32
        };
        uint32_t u = 0;
    };
};

/**
 **********************************************************************************************
 * \privatesection
 */

enum class ShaderStage : uint8_t {
    VERTEX = 0,
    FRAGMENT = 1,
    COMPUTE = 2
};

static constexpr size_t PIPELINE_STAGE_COUNT = 2;
enum class ShaderStageFlags : uint8_t {
    NONE        =    0,
    VERTEX      =    0x1,
    FRAGMENT    =    0x2,
    COMPUTE     =    0x4,
    ALL_SHADER_STAGE_FLAGS = VERTEX | FRAGMENT | COMPUTE
};

static inline constexpr bool hasShaderType(ShaderStageFlags flags, ShaderStage type) noexcept {
    switch (type) {
        case ShaderStage::VERTEX:
            return bool(uint8_t(flags) & uint8_t(ShaderStageFlags::VERTEX));
        case ShaderStage::FRAGMENT:
            return bool(uint8_t(flags) & uint8_t(ShaderStageFlags::FRAGMENT));
        case ShaderStage::COMPUTE:
            return bool(uint8_t(flags) & uint8_t(ShaderStageFlags::COMPUTE));
    }
}

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
    DepthRange depthRange{};    //!< depth range for this pass

    //! Color to use to clear the COLOR buffer. RenderPassFlags::clear must be set.
    math::float4 clearColor = {};

    //! Depth value to clear the depth buffer with
    double clearDepth = 0.0;

    //! Stencil value to clear the stencil buffer with
    uint32_t clearStencil = 0;

    /**
     * The subpass mask specifies which color attachments are designated for read-back in the second
     * subpass. If this is zero, the render pass has only one subpass. The least significant bit
     * specifies that the first color attachment in the render target is a subpass input.
     *
     * For now only 2 subpasses are supported, so only the lower 8 bits are used, one for each color
     * attachment (see MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT).
     */
    uint16_t subpassMask = 0;

    /**
     * This mask makes a promise to the backend about read-only usage of the depth attachment (bit
     * 0) and the stencil attachment (bit 1). Some backends need to know if writes are disabled in
     * order to allow sampling from the depth attachment.
     */
    uint16_t readOnlyDepthStencil = 0;

    static constexpr uint16_t READONLY_DEPTH = 1 << 0;
    static constexpr uint16_t READONLY_STENCIL = 1 << 1;
};

struct PolygonOffset {
    float slope = 0;        // factor in GL-speak
    float constant = 0;     // units in GL-speak
};

struct StencilState {
    using StencilFunction = SamplerCompareFunc;

    struct StencilOperations {
        //! Stencil test function
        StencilFunction stencilFunc                     : 3;                    // 3

        //! Stencil operation when stencil test fails
        StencilOperation stencilOpStencilFail           : 3;                    // 6

        uint8_t padding0                                : 2;                    // 8

        //! Stencil operation when stencil test passes but depth test fails
        StencilOperation stencilOpDepthFail             : 3;                    // 11

        //! Stencil operation when both stencil and depth test pass
        StencilOperation stencilOpDepthStencilPass      : 3;                    // 14

        uint8_t padding1                                : 2;                    // 16

        //! Reference value for stencil comparison tests and updates
        uint8_t ref;                                                            // 24

        //! Masks the bits of the stencil values participating in the stencil comparison test.
        uint8_t readMask;                                                       // 32

        //! Masks the bits of the stencil values updated by the stencil test.
        uint8_t writeMask;                                                      // 40
    };

    //! Stencil operations for front-facing polygons
    StencilOperations front = {
            .stencilFunc = StencilFunction::A,
            .stencilOpStencilFail = StencilOperation::KEEP,
            .padding0 = 0,
            .stencilOpDepthFail = StencilOperation::KEEP,
            .stencilOpDepthStencilPass = StencilOperation::KEEP,
            .padding1 = 0,
            .ref = 0,
            .readMask = 0xff,
            .writeMask = 0xff };

    //! Stencil operations for back-facing polygons
    StencilOperations back  = {
            .stencilFunc = StencilFunction::A,
            .stencilOpStencilFail = StencilOperation::KEEP,
            .padding0 = 0,
            .stencilOpDepthFail = StencilOperation::KEEP,
            .stencilOpDepthStencilPass = StencilOperation::KEEP,
            .padding1 = 0,
            .ref = 0,
            .readMask = 0xff,
            .writeMask = 0xff };

    //! Whether stencil-buffer writes are enabled
    bool stencilWrite = false;

    uint8_t padding = 0;
};

using PushConstantVariant = std::variant<int32_t, float, bool>;

static_assert(sizeof(StencilState::StencilOperations) == 5u,
        "StencilOperations size not what was intended");

static_assert(sizeof(StencilState) == 12u,
        "StencilState size not what was intended");

using FrameScheduledCallback = utils::Invocable<void(backend::PresentCallable)>;

enum class Workaround : uint16_t {
    // The EASU pass must split because shader compiler flattens early-exit branch
    SPLIT_EASU,
    // Backend allows feedback loop with ancillary buffers (depth/stencil) as long as they're
    // read-only for the whole render pass.
    ALLOW_READ_ONLY_ANCILLARY_FEEDBACK_LOOP,
    // for some uniform arrays, it's needed to do an initialization to avoid crash on adreno gpu
    ADRENO_UNIFORM_ARRAY_CRASH,
    // Workaround a Metal pipeline compilation error with the message:
    // "Could not statically determine the target of a texture". See light_indirect.fs
    A8X_STATIC_TEXTURE_TARGET_ERROR,
    // Adreno drivers sometimes aren't able to blit into a layer of a texture array.
    DISABLE_BLIT_INTO_TEXTURE_ARRAY,
    // Multiple workarounds needed for PowerVR GPUs
    POWER_VR_SHADER_WORKAROUNDS,
};

using StereoscopicType = backend::Platform::StereoscopicType;

} // namespace filament::backend

template<> struct utils::EnableBitMaskOperators<filament::backend::ShaderStageFlags>
        : public std::true_type {};
template<> struct utils::EnableBitMaskOperators<filament::backend::TargetBufferFlags>
        : public std::true_type {};
template<> struct utils::EnableBitMaskOperators<filament::backend::TextureUsage>
        : public std::true_type {};
template<> struct utils::EnableBitMaskOperators<filament::backend::StencilFace>
        : public std::true_type {};
template<> struct utils::EnableIntegerOperators<filament::backend::TextureCubemapFace>
        : public std::true_type {};
template<> struct utils::EnableIntegerOperators<filament::backend::FeatureLevel>
        : public std::true_type {};

#if !defined(NDEBUG)
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::BufferUsage usage);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::CullingMode mode);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::ElementType type);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::PixelDataFormat format);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::PixelDataType type);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::Precision precision);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::PrimitiveType type);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::TargetBufferFlags f);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::SamplerCompareFunc func);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::SamplerCompareMode mode);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::SamplerFormat format);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::SamplerMagFilter filter);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::SamplerMinFilter filter);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::SamplerParams params);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::SamplerType type);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::SamplerWrapMode wrap);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::ShaderModel model);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::TextureCubemapFace face);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::TextureFormat format);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::TextureUsage usage);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::BufferObjectBinding binding);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::TextureSwizzle swizzle);
utils::io::ostream& operator<<(utils::io::ostream& out, const filament::backend::AttributeArray& type);
utils::io::ostream& operator<<(utils::io::ostream& out, const filament::backend::PolygonOffset& po);
utils::io::ostream& operator<<(utils::io::ostream& out, const filament::backend::RasterState& rs);
utils::io::ostream& operator<<(utils::io::ostream& out, const filament::backend::RenderPassParams& b);
utils::io::ostream& operator<<(utils::io::ostream& out, const filament::backend::Viewport& v);
utils::io::ostream& operator<<(utils::io::ostream& out, filament::backend::ShaderStageFlags stageFlags);
#endif

#endif // TNT_FILAMENT_BACKEND_DRIVERENUMS_H
