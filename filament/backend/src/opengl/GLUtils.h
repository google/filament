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

#ifndef TNT_FILAMENT_DRIVER_GLUTILS_H
#define TNT_FILAMENT_DRIVER_GLUTILS_H

#include <utils/compiler.h>
#include <utils/Log.h>

#include <backend/DriverEnums.h>

#include <string>
#include <unordered_set>

#include "gl_headers.h"

namespace filament {
namespace GLUtils {

void checkGLError(utils::io::ostream& out, const char* function, size_t line) noexcept;
void checkFramebufferStatus(utils::io::ostream& out, const char* function, size_t line) noexcept;

#ifdef NDEBUG
#define CHECK_GL_ERROR(out)
#define CHECK_GL_FRAMEBUFFER_STATUS(out)
#else
#ifdef _MSC_VER
    #define __PRETTY_FUNCTION__ __FUNCSIG__
#endif
#define CHECK_GL_ERROR(out) { GLUtils::checkGLError(out, __PRETTY_FUNCTION__, __LINE__); }
#define CHECK_GL_FRAMEBUFFER_STATUS(out) { GLUtils::checkFramebufferStatus(out, __PRETTY_FUNCTION__, __LINE__); }
#endif

constexpr inline GLuint getComponentCount(backend::ElementType type) noexcept {
    using ElementType = backend::ElementType;
    switch (type) {
        case ElementType::BYTE:
        case ElementType::UBYTE:
        case ElementType::SHORT:
        case ElementType::USHORT:
        case ElementType::INT:
        case ElementType::UINT:
        case ElementType::FLOAT:
        case ElementType::HALF:
            return 1;
        case ElementType::FLOAT2:
        case ElementType::HALF2:
        case ElementType::BYTE2:
        case ElementType::UBYTE2:
        case ElementType::SHORT2:
        case ElementType::USHORT2:
            return 2;
        case ElementType::FLOAT3:
        case ElementType::HALF3:
        case ElementType::BYTE3:
        case ElementType::UBYTE3:
        case ElementType::SHORT3:
        case ElementType::USHORT3:
            return 3;
        case ElementType::FLOAT4:
        case ElementType::HALF4:
        case ElementType::BYTE4:
        case ElementType::UBYTE4:
        case ElementType::SHORT4:
        case ElementType::USHORT4:
            return 4;
    }
}

// ------------------------------------------------------------------------------------------------
// Our enums to GLenum conversions
// ------------------------------------------------------------------------------------------------

constexpr inline GLbitfield getAttachmentBitfield(backend::TargetBufferFlags flags) noexcept {
    GLbitfield mask = 0;
    if (any(flags & backend::TargetBufferFlags::COLOR)) {
        mask |= (GLbitfield)GL_COLOR_BUFFER_BIT;
    }
    if (any(flags & backend::TargetBufferFlags::DEPTH)) {
        mask |= (GLbitfield)GL_DEPTH_BUFFER_BIT;
    }
    if (any(flags & backend::TargetBufferFlags::STENCIL)) {
        mask |= (GLbitfield)GL_STENCIL_BUFFER_BIT;
    }
    return mask;
}

constexpr inline GLenum getBufferUsage(backend::BufferUsage usage) noexcept {
    switch (usage) {
        case backend::BufferUsage::STATIC:
            return GL_STATIC_DRAW;
        case backend::BufferUsage::DYNAMIC:
        case backend::BufferUsage::STREAM:
            return GL_DYNAMIC_DRAW;
    }
}

constexpr inline GLboolean getNormalization(bool normalized) noexcept {
    return GLboolean(normalized ? GL_TRUE : GL_FALSE);
}

constexpr inline GLenum getComponentType(backend::ElementType type) noexcept {
    using ElementType = backend::ElementType;
    switch (type) {
        case ElementType::BYTE:
        case ElementType::BYTE2:
        case ElementType::BYTE3:
        case ElementType::BYTE4:
            return GL_BYTE;
        case ElementType::UBYTE:
        case ElementType::UBYTE2:
        case ElementType::UBYTE3:
        case ElementType::UBYTE4:
            return GL_UNSIGNED_BYTE;
        case ElementType::SHORT:
        case ElementType::SHORT2:
        case ElementType::SHORT3:
        case ElementType::SHORT4:
            return GL_SHORT;
        case ElementType::USHORT:
        case ElementType::USHORT2:
        case ElementType::USHORT3:
        case ElementType::USHORT4:
            return GL_UNSIGNED_SHORT;
        case ElementType::INT:
            return GL_INT;
        case ElementType::UINT:
            return GL_UNSIGNED_INT;
        case ElementType::FLOAT:
        case ElementType::FLOAT2:
        case ElementType::FLOAT3:
        case ElementType::FLOAT4:
            return GL_FLOAT;
        case ElementType::HALF:
        case ElementType::HALF2:
        case ElementType::HALF3:
        case ElementType::HALF4:
            return GL_HALF_FLOAT;
    }
}

constexpr inline GLenum getCubemapTarget(backend::TextureCubemapFace face) noexcept {
    return GL_TEXTURE_CUBE_MAP_POSITIVE_X + GLenum(face);
}

constexpr inline GLenum getWrapMode(backend::SamplerWrapMode mode) noexcept {
    using SamplerWrapMode = backend::SamplerWrapMode;
    switch (mode) {
        case SamplerWrapMode::REPEAT:
            return GL_REPEAT;
        case SamplerWrapMode::CLAMP_TO_EDGE:
            return GL_CLAMP_TO_EDGE;
        case SamplerWrapMode::MIRRORED_REPEAT:
            return GL_MIRRORED_REPEAT;
    }
}

constexpr inline GLenum getTextureFilter(backend::SamplerMinFilter filter) noexcept {
    using SamplerMinFilter = backend::SamplerMinFilter;
    switch (filter) {
        case SamplerMinFilter::NEAREST:
        case SamplerMinFilter::LINEAR:
            return GL_NEAREST + GLenum(filter);
        case SamplerMinFilter::NEAREST_MIPMAP_NEAREST:
        case SamplerMinFilter::LINEAR_MIPMAP_NEAREST:
        case SamplerMinFilter::NEAREST_MIPMAP_LINEAR:
        case SamplerMinFilter::LINEAR_MIPMAP_LINEAR:
            return GL_NEAREST_MIPMAP_NEAREST
                   - GLenum(SamplerMinFilter::NEAREST_MIPMAP_NEAREST) + GLenum(filter);
    }
}

constexpr inline GLenum getTextureFilter(backend::SamplerMagFilter filter) noexcept {
    return GL_NEAREST + GLenum(filter);
}


constexpr inline GLenum getBlendEquationMode(backend::BlendEquation mode) noexcept {
    using BlendEquation = backend::BlendEquation;
    switch (mode) {
        case BlendEquation::ADD:               return GL_FUNC_ADD;
        case BlendEquation::SUBTRACT:          return GL_FUNC_SUBTRACT;
        case BlendEquation::REVERSE_SUBTRACT:  return GL_FUNC_REVERSE_SUBTRACT;
        case BlendEquation::MIN:               return GL_MIN;
        case BlendEquation::MAX:               return GL_MAX;
    }
}

constexpr inline GLenum getBlendFunctionMode(backend::BlendFunction mode) noexcept {
    using BlendFunction = backend::BlendFunction;
    switch (mode) {
        case BlendFunction::ZERO:                  return GL_ZERO;
        case BlendFunction::ONE:                   return GL_ONE;
        case BlendFunction::SRC_COLOR:             return GL_SRC_COLOR;
        case BlendFunction::ONE_MINUS_SRC_COLOR:   return GL_ONE_MINUS_SRC_COLOR;
        case BlendFunction::DST_COLOR:             return GL_DST_COLOR;
        case BlendFunction::ONE_MINUS_DST_COLOR:   return GL_ONE_MINUS_DST_COLOR;
        case BlendFunction::SRC_ALPHA:             return GL_SRC_ALPHA;
        case BlendFunction::ONE_MINUS_SRC_ALPHA:   return GL_ONE_MINUS_SRC_ALPHA;
        case BlendFunction::DST_ALPHA:             return GL_DST_ALPHA;
        case BlendFunction::ONE_MINUS_DST_ALPHA:   return GL_ONE_MINUS_DST_ALPHA;
        case BlendFunction::SRC_ALPHA_SATURATE:    return GL_SRC_ALPHA_SATURATE;
    }
}

constexpr inline GLenum getTextureCompareMode(backend::SamplerCompareMode mode) noexcept {
    return mode == backend::SamplerCompareMode::NONE ?
           GL_NONE : GL_COMPARE_REF_TO_TEXTURE;
}

constexpr inline GLenum getTextureCompareFunc(backend::SamplerCompareFunc func) noexcept {
    using SamplerCompareFunc = backend::SamplerCompareFunc;
    switch (func) {
        case SamplerCompareFunc::LE:    return GL_LEQUAL;
        case SamplerCompareFunc::GE:    return GL_GEQUAL;
        case SamplerCompareFunc::L:     return GL_LESS;
        case SamplerCompareFunc::G:     return GL_GREATER;
        case SamplerCompareFunc::E:     return GL_EQUAL;
        case SamplerCompareFunc::NE:    return GL_NOTEQUAL;
        case SamplerCompareFunc::A:     return GL_ALWAYS;
        case SamplerCompareFunc::N:     return GL_NEVER;
    }
}

constexpr inline GLenum getDepthFunc(backend::SamplerCompareFunc func) noexcept {
    return getTextureCompareFunc(func);
}

constexpr inline GLenum getFormat(backend::PixelDataFormat format) noexcept {
    using PixelDataFormat = backend::PixelDataFormat;
    switch (format) {
        case PixelDataFormat::R:                return GL_RED;
        case PixelDataFormat::R_INTEGER:        return GL_RED_INTEGER;
        case PixelDataFormat::RG:               return GL_RG;
        case PixelDataFormat::RG_INTEGER:       return GL_RG_INTEGER;
        case PixelDataFormat::RGB:              return GL_RGB;
        case PixelDataFormat::RGB_INTEGER:      return GL_RGB_INTEGER;
        case PixelDataFormat::RGBA:             return GL_RGBA;
        case PixelDataFormat::RGBA_INTEGER:     return GL_RGBA_INTEGER;
        case PixelDataFormat::UNUSED:           return GL_RGBA; // should never happen (used to be rgbm)
        case PixelDataFormat::DEPTH_COMPONENT:  return GL_DEPTH_COMPONENT;
        case PixelDataFormat::DEPTH_STENCIL:    return GL_DEPTH_STENCIL;
        case PixelDataFormat::ALPHA:            return GL_ALPHA;
    }
}

constexpr inline GLenum getType(backend::PixelDataType type) noexcept {
    using PixelDataType = backend::PixelDataType;
    switch (type) {
        case PixelDataType::UBYTE:                return GL_UNSIGNED_BYTE;
        case PixelDataType::BYTE:                 return GL_BYTE;
        case PixelDataType::USHORT:               return GL_UNSIGNED_SHORT;
        case PixelDataType::SHORT:                return GL_SHORT;
        case PixelDataType::UINT:                 return GL_UNSIGNED_INT;
        case PixelDataType::INT:                  return GL_INT;
        case PixelDataType::HALF:                 return GL_HALF_FLOAT;
        case PixelDataType::FLOAT:                return GL_FLOAT;
        case PixelDataType::UINT_10F_11F_11F_REV: return GL_UNSIGNED_INT_10F_11F_11F_REV;
        case PixelDataType::USHORT_565:           return GL_UNSIGNED_SHORT_5_6_5;
        case PixelDataType::COMPRESSED:           return 0; // should never happen
    }
}

constexpr inline GLenum getSwizzleChannel(backend::TextureSwizzle c) noexcept {
    using TextureSwizzle = backend::TextureSwizzle;
    switch (c) {
        case TextureSwizzle::SUBSTITUTE_ZERO:
            return GL_ZERO;
        case TextureSwizzle::SUBSTITUTE_ONE:
            return GL_ONE;
        case TextureSwizzle::CHANNEL_0:
            return GL_RED;
        case TextureSwizzle::CHANNEL_1:
            return GL_GREEN;
        case TextureSwizzle::CHANNEL_2:
            return GL_BLUE;
        case TextureSwizzle::CHANNEL_3:
            return GL_ALPHA;
    }
}

// clang looses it on this one, and generates a huge jump table when
// inlined. So we don't  mark it as inline (only constexpr) which solves the problem,
// strangely, when not inlined, clang simply generates an array lookup.
constexpr /* inline */ GLenum getInternalFormat(backend::TextureFormat format) noexcept {
    using TextureFormat = backend::TextureFormat;
    switch (format) {
        // 8-bits per element
        case TextureFormat::R8:                return GL_R8;
        case TextureFormat::R8_SNORM:          return GL_R8_SNORM;
        case TextureFormat::R8UI:              return GL_R8UI;
        case TextureFormat::R8I:               return GL_R8I;
        case TextureFormat::STENCIL8:          return GL_STENCIL_INDEX8;

        // 16-bits per element
        case TextureFormat::R16F:              return GL_R16F;
        case TextureFormat::R16UI:             return GL_R16UI;
        case TextureFormat::R16I:              return GL_R16I;
        case TextureFormat::RG8:               return GL_RG8;
        case TextureFormat::RG8_SNORM:         return GL_RG8_SNORM;
        case TextureFormat::RG8UI:             return GL_RG8UI;
        case TextureFormat::RG8I:              return GL_RG8I;
        case TextureFormat::RGB565:            return GL_RGB565;
        case TextureFormat::RGB5_A1:           return GL_RGB5_A1;
        case TextureFormat::RGBA4:             return GL_RGBA4;
        case TextureFormat::DEPTH16:           return GL_DEPTH_COMPONENT16;

        // 24-bits per element
        case TextureFormat::RGB8:              return GL_RGB8;
        case TextureFormat::SRGB8:             return GL_SRGB8;
        case TextureFormat::RGB8_SNORM:        return GL_RGB8_SNORM;
        case TextureFormat::RGB8UI:            return GL_RGB8UI;
        case TextureFormat::RGB8I:             return GL_RGB8I;
        case TextureFormat::DEPTH24:           return GL_DEPTH_COMPONENT24;

        // 32-bits per element
        case TextureFormat::R32F:              return GL_R32F;
        case TextureFormat::R32UI:             return GL_R32UI;
        case TextureFormat::R32I:              return GL_R32I;
        case TextureFormat::RG16F:             return GL_RG16F;
        case TextureFormat::RG16UI:            return GL_RG16UI;
        case TextureFormat::RG16I:             return GL_RG16I;
        case TextureFormat::R11F_G11F_B10F:    return GL_R11F_G11F_B10F;
        case TextureFormat::RGB9_E5:           return GL_RGB9_E5;
        case TextureFormat::RGBA8:             return GL_RGBA8;
        case TextureFormat::SRGB8_A8:          return GL_SRGB8_ALPHA8;
        case TextureFormat::RGBA8_SNORM:       return GL_RGBA8_SNORM;
        case TextureFormat::RGB10_A2:          return GL_RGB10_A2;
        case TextureFormat::RGBA8UI:           return GL_RGBA8UI;
        case TextureFormat::RGBA8I:            return GL_RGBA8I;
        case TextureFormat::DEPTH32F:          return GL_DEPTH_COMPONENT32F;
        case TextureFormat::DEPTH24_STENCIL8:  return GL_DEPTH24_STENCIL8;
        case TextureFormat::DEPTH32F_STENCIL8: return GL_DEPTH32F_STENCIL8;

        // 48-bits per element
        case TextureFormat::RGB16F:            return GL_RGB16F;
        case TextureFormat::RGB16UI:           return GL_RGB16UI;
        case TextureFormat::RGB16I:            return GL_RGB16I;

        // 64-bits per element
        case TextureFormat::RG32F:             return GL_RG32F;
        case TextureFormat::RG32UI:            return GL_RG32UI;
        case TextureFormat::RG32I:             return GL_RG32I;
        case TextureFormat::RGBA16F:           return GL_RGBA16F;
        case TextureFormat::RGBA16UI:          return GL_RGBA16UI;
        case TextureFormat::RGBA16I:           return GL_RGBA16I;

        // 96-bits per element
        case TextureFormat::RGB32F:            return GL_RGB32F;
        case TextureFormat::RGB32UI:           return GL_RGB32UI;
        case TextureFormat::RGB32I:            return GL_RGB32I;

        // 128-bits per element
        case TextureFormat::RGBA32F:           return GL_RGBA32F;
        case TextureFormat::RGBA32UI:          return GL_RGBA32UI;
        case TextureFormat::RGBA32I:           return GL_RGBA32I;

        // compressed formats
#if defined(GL_ES_VERSION_3_0) || defined(GL_VERSION_4_3) || defined(GL_ARB_ES3_compatibility)
        case TextureFormat::EAC_R11:           return GL_COMPRESSED_R11_EAC;
        case TextureFormat::EAC_R11_SIGNED:    return GL_COMPRESSED_SIGNED_R11_EAC;
        case TextureFormat::EAC_RG11:          return GL_COMPRESSED_RG11_EAC;
        case TextureFormat::EAC_RG11_SIGNED:   return GL_COMPRESSED_SIGNED_RG11_EAC;
        case TextureFormat::ETC2_RGB8:         return GL_COMPRESSED_RGB8_ETC2;
        case TextureFormat::ETC2_SRGB8:        return GL_COMPRESSED_SRGB8_ETC2;
        case TextureFormat::ETC2_RGB8_A1:      return GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
        case TextureFormat::ETC2_SRGB8_A1:     return GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2;
        case TextureFormat::ETC2_EAC_RGBA8:    return GL_COMPRESSED_RGBA8_ETC2_EAC;
        case TextureFormat::ETC2_EAC_SRGBA8:   return GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC;
#else
        case TextureFormat::EAC_R11:
        case TextureFormat::EAC_R11_SIGNED:
        case TextureFormat::EAC_RG11:
        case TextureFormat::EAC_RG11_SIGNED:
        case TextureFormat::ETC2_RGB8:
        case TextureFormat::ETC2_SRGB8:
        case TextureFormat::ETC2_RGB8_A1:
        case TextureFormat::ETC2_SRGB8_A1:
        case TextureFormat::ETC2_EAC_RGBA8:
        case TextureFormat::ETC2_EAC_SRGBA8:
            // this should not happen
            return 0;
#endif

#if defined(GL_EXT_texture_compression_s3tc)
        case TextureFormat::DXT1_RGB:          return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
        case TextureFormat::DXT1_RGBA:         return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        case TextureFormat::DXT3_RGBA:         return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        case TextureFormat::DXT5_RGBA:         return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
#else
        case TextureFormat::DXT1_RGB:
        case TextureFormat::DXT1_RGBA:
        case TextureFormat::DXT3_RGBA:
        case TextureFormat::DXT5_RGBA:
            // this should not happen
            return 0;
#endif

#if defined(GL_EXT_texture_sRGB) || defined(GL_EXT_texture_compression_s3tc_srgb)
        case TextureFormat::DXT1_SRGB:         return GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
        case TextureFormat::DXT1_SRGBA:        return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
        case TextureFormat::DXT3_SRGBA:        return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
        case TextureFormat::DXT5_SRGBA:        return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
#else
        case TextureFormat::DXT1_SRGB:
        case TextureFormat::DXT1_SRGBA:
        case TextureFormat::DXT3_SRGBA:
        case TextureFormat::DXT5_SRGBA:
            // this should not happen
            return 0;
#endif

#if defined(GL_KHR_texture_compression_astc_hdr)
        case TextureFormat::RGBA_ASTC_4x4:     return GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
        case TextureFormat::RGBA_ASTC_5x4:     return GL_COMPRESSED_RGBA_ASTC_5x4_KHR;
        case TextureFormat::RGBA_ASTC_5x5:     return GL_COMPRESSED_RGBA_ASTC_5x5_KHR;
        case TextureFormat::RGBA_ASTC_6x5:     return GL_COMPRESSED_RGBA_ASTC_6x5_KHR;
        case TextureFormat::RGBA_ASTC_6x6:     return GL_COMPRESSED_RGBA_ASTC_6x6_KHR;
        case TextureFormat::RGBA_ASTC_8x5:     return GL_COMPRESSED_RGBA_ASTC_8x5_KHR;
        case TextureFormat::RGBA_ASTC_8x6:     return GL_COMPRESSED_RGBA_ASTC_8x6_KHR;
        case TextureFormat::RGBA_ASTC_8x8:     return GL_COMPRESSED_RGBA_ASTC_8x8_KHR;
        case TextureFormat::RGBA_ASTC_10x5:    return GL_COMPRESSED_RGBA_ASTC_10x5_KHR;
        case TextureFormat::RGBA_ASTC_10x6:    return GL_COMPRESSED_RGBA_ASTC_10x6_KHR;
        case TextureFormat::RGBA_ASTC_10x8:    return GL_COMPRESSED_RGBA_ASTC_10x8_KHR;
        case TextureFormat::RGBA_ASTC_10x10:   return GL_COMPRESSED_RGBA_ASTC_10x10_KHR;
        case TextureFormat::RGBA_ASTC_12x10:   return GL_COMPRESSED_RGBA_ASTC_12x10_KHR;
        case TextureFormat::RGBA_ASTC_12x12:   return GL_COMPRESSED_RGBA_ASTC_12x12_KHR;
        case TextureFormat::SRGB8_ALPHA8_ASTC_4x4:   return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR;
        case TextureFormat::SRGB8_ALPHA8_ASTC_5x4:   return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR;
        case TextureFormat::SRGB8_ALPHA8_ASTC_5x5:   return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR;
        case TextureFormat::SRGB8_ALPHA8_ASTC_6x5:   return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR;
        case TextureFormat::SRGB8_ALPHA8_ASTC_6x6:   return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR;
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x5:   return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR;
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x6:   return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR;
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x8:   return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR;
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x5:  return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR;
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x6:  return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR;
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x8:  return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR;
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x10: return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR;
        case TextureFormat::SRGB8_ALPHA8_ASTC_12x10: return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR;
        case TextureFormat::SRGB8_ALPHA8_ASTC_12x12: return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR;
#else
        case TextureFormat::RGBA_ASTC_4x4:
        case TextureFormat::RGBA_ASTC_5x4:
        case TextureFormat::RGBA_ASTC_5x5:
        case TextureFormat::RGBA_ASTC_6x5:
        case TextureFormat::RGBA_ASTC_6x6:
        case TextureFormat::RGBA_ASTC_8x5:
        case TextureFormat::RGBA_ASTC_8x6:
        case TextureFormat::RGBA_ASTC_8x8:
        case TextureFormat::RGBA_ASTC_10x5:
        case TextureFormat::RGBA_ASTC_10x6:
        case TextureFormat::RGBA_ASTC_10x8:
        case TextureFormat::RGBA_ASTC_10x10:
        case TextureFormat::RGBA_ASTC_12x10:
        case TextureFormat::RGBA_ASTC_12x12:
        case TextureFormat::SRGB8_ALPHA8_ASTC_4x4:
        case TextureFormat::SRGB8_ALPHA8_ASTC_5x4:
        case TextureFormat::SRGB8_ALPHA8_ASTC_5x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_6x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_6x6:
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x6:
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x8:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x5:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x6:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x8:
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x10:
        case TextureFormat::SRGB8_ALPHA8_ASTC_12x10:
        case TextureFormat::SRGB8_ALPHA8_ASTC_12x12:
            // this should not happen
            return 0;
#endif
        case TextureFormat::UNUSED:
            return 0;
    }
}

class unordered_string_set : public std::unordered_set<std::string> {
public:
    bool has(const char* str) {
        return find(std::string(str)) != end();
    }
};

inline unordered_string_set split(const char* spacedList) {
    unordered_string_set set;
    const char* current = spacedList;
    const char* head = current;
    do {
        head = strchr(current, ' ');
        std::string s(current, head ? head - current : strlen(current));
        if (s.length()) {
            set.insert(std::move(s));
        }
        current = head + 1;
    } while (head);
    return set;
}

} // namespace GLUtils
} // namespace filament

#endif // TNT_FILAMENT_DRIVER_GLUTILS_H
