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

#ifndef TNT_FILAMENT_BACKEND_OPENGL_GLUTILS_H
#define TNT_FILAMENT_BACKEND_OPENGL_GLUTILS_H

#include <utils/debug.h>
#include <utils/ostream.h>

#include <backend/DriverEnums.h>

#include <string_view>
#include <unordered_set>

#include <stddef.h>
#include <stdint.h>

#include "gl_headers.h"

namespace filament::backend::GLUtils {

std::string_view getGLErrorString(GLenum error) noexcept;
GLenum checkGLError(const char* function, size_t line) noexcept;
void assertGLError(const char* function, size_t line) noexcept;

std::string_view getFramebufferStatusString(GLenum err) noexcept;
GLenum checkFramebufferStatus(GLenum target, const char* function, size_t line) noexcept;
void assertFramebufferStatus(GLenum target, const char* function, size_t line) noexcept;

#ifdef NDEBUG
#   define CHECK_GL_ERROR()
#   define CHECK_GL_ERROR_NON_FATAL()
#   define CHECK_GL_FRAMEBUFFER_STATUS(target)
#else
#   define CHECK_GL_ERROR() { GLUtils::assertGLError(__func__, __LINE__); }
#   define CHECK_GL_ERROR_NON_FATAL() { GLUtils::checkGLError(__func__, __LINE__); }
#   define CHECK_GL_FRAMEBUFFER_STATUS(target) { GLUtils::checkFramebufferStatus( target, __func__, __LINE__); }
#endif

constexpr GLuint getComponentCount(ElementType const type) noexcept {
    using ElementType = ElementType;
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
    // should never happen
    return 1;
}

// ------------------------------------------------------------------------------------------------
// Our enums to GLenum conversions
// ------------------------------------------------------------------------------------------------

constexpr GLbitfield getAttachmentBitfield(TargetBufferFlags const flags) noexcept {
    GLbitfield mask = 0;
    if (any(flags & TargetBufferFlags::COLOR_ALL)) {
        mask |= GLbitfield(GL_COLOR_BUFFER_BIT);
    }
    if (any(flags & TargetBufferFlags::DEPTH)) {
        mask |= GLbitfield(GL_DEPTH_BUFFER_BIT);
    }
    if (any(flags & TargetBufferFlags::STENCIL)) {
        mask |= GLbitfield(GL_STENCIL_BUFFER_BIT);
    }
    return mask;
}

constexpr GLenum getBufferUsage(BufferUsage const usage) noexcept {
    switch (usage) {
        case BufferUsage::STATIC:
            return GL_STATIC_DRAW;
        default:
            return GL_DYNAMIC_DRAW;
    }
}

constexpr GLenum getBufferBindingType(BufferObjectBinding const bindingType) noexcept {
    switch (bindingType) {
        case BufferObjectBinding::VERTEX:
            return GL_ARRAY_BUFFER;
        case BufferObjectBinding::UNIFORM:
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
            return GL_UNIFORM_BUFFER;
#else
            utils::panic(__func__, __FILE__, __LINE__, "UNIFORM not supported");
            return 0x8A11;
#endif
        case BufferObjectBinding::SHADER_STORAGE:
#ifdef BACKEND_OPENGL_LEVEL_GLES31
            return GL_SHADER_STORAGE_BUFFER;
#else
            utils::panic(__func__, __FILE__, __LINE__, "SHADER_STORAGE not supported");
            return 0x90D2; // just to return something
#endif
    }
    // should never happen
    return GL_ARRAY_BUFFER;
}

constexpr GLboolean getNormalization(bool const normalized) noexcept {
    return GLboolean(normalized ? GL_TRUE : GL_FALSE);
}

constexpr GLenum getComponentType(ElementType const type) noexcept {
    using ElementType = ElementType;
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
            // on ES2 we should never end-up here
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
            return GL_HALF_FLOAT;
#else
            return GL_HALF_FLOAT_OES;
#endif
    }
    // should never happen
    return GL_INT;
}

constexpr GLenum getTextureTargetNotExternal(SamplerType const target) noexcept {
    switch (target) {
        case SamplerType::SAMPLER_2D:
            return GL_TEXTURE_2D;
        case SamplerType::SAMPLER_3D:
            return GL_TEXTURE_3D;
        case SamplerType::SAMPLER_2D_ARRAY:
            return GL_TEXTURE_2D_ARRAY;
        case SamplerType::SAMPLER_CUBEMAP:
            return GL_TEXTURE_CUBE_MAP;
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            return GL_TEXTURE_CUBE_MAP_ARRAY;
        case SamplerType::SAMPLER_EXTERNAL:
            // we should never be here
            return GL_TEXTURE_2D;
    }
    // should never happen
    return GL_TEXTURE_2D;
}

constexpr GLenum getCubemapTarget(uint16_t const layer) noexcept {
    assert_invariant(layer <= 5);
    return GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer;
}

constexpr GLenum getWrapMode(SamplerWrapMode const mode) noexcept {
    using SamplerWrapMode = SamplerWrapMode;
    switch (mode) {
        case SamplerWrapMode::REPEAT:
            return GL_REPEAT;
        case SamplerWrapMode::CLAMP_TO_EDGE:
            return GL_CLAMP_TO_EDGE;
        case SamplerWrapMode::MIRRORED_REPEAT:
            return GL_MIRRORED_REPEAT;
    }
    // should never happen
    return GL_CLAMP_TO_EDGE;
}

constexpr GLenum getTextureFilter(SamplerMinFilter filter) noexcept {
    using SamplerMinFilter = SamplerMinFilter;
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
    // should never happen
    return GL_NEAREST;
}

constexpr GLenum getTextureFilter(SamplerMagFilter filter) noexcept {
    return GL_NEAREST + GLenum(filter);
}


constexpr GLenum getBlendEquationMode(BlendEquation const mode) noexcept {
    using BlendEquation = BlendEquation;
    switch (mode) {
        case BlendEquation::ADD:               return GL_FUNC_ADD;
        case BlendEquation::SUBTRACT:          return GL_FUNC_SUBTRACT;
        case BlendEquation::REVERSE_SUBTRACT:  return GL_FUNC_REVERSE_SUBTRACT;
        case BlendEquation::MIN:               return GL_MIN;
        case BlendEquation::MAX:               return GL_MAX;
    }
    // should never happen
    return GL_FUNC_ADD;
}

constexpr GLenum getBlendFunctionMode(BlendFunction const mode) noexcept {
    using BlendFunction = BlendFunction;
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
    // should never happen
    return GL_ONE;
}

constexpr GLenum getCompareFunc(SamplerCompareFunc const func) noexcept {
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
    // should never happen
    return GL_LEQUAL;
}

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
constexpr GLenum getTextureCompareMode(SamplerCompareMode const mode) noexcept {
    return mode == SamplerCompareMode::NONE ?
           GL_NONE : GL_COMPARE_REF_TO_TEXTURE;
}

constexpr GLenum getTextureCompareFunc(SamplerCompareFunc const func) noexcept {
    return getCompareFunc(func);
}
#endif

constexpr GLenum getDepthFunc(SamplerCompareFunc const func) noexcept {
    return getCompareFunc(func);
}

constexpr GLenum getStencilFunc(SamplerCompareFunc const func) noexcept {
    return getCompareFunc(func);
}

constexpr GLenum getStencilOp(StencilOperation const op) noexcept {
    switch (op) {
        case StencilOperation::KEEP:        return GL_KEEP;
        case StencilOperation::ZERO:        return GL_ZERO;
        case StencilOperation::REPLACE:     return GL_REPLACE;
        case StencilOperation::INCR:        return GL_INCR;
        case StencilOperation::INCR_WRAP:   return GL_INCR_WRAP;
        case StencilOperation::DECR:        return GL_DECR;
        case StencilOperation::DECR_WRAP:   return GL_DECR_WRAP;
        case StencilOperation::INVERT:      return GL_INVERT;
    }
    // should never happen
    return GL_KEEP;
}

constexpr GLenum getFormat(PixelDataFormat const format) noexcept {
    using PixelDataFormat = PixelDataFormat;
    switch (format) {
        case PixelDataFormat::RGB:              return GL_RGB;
        case PixelDataFormat::RGBA:             return GL_RGBA;
        case PixelDataFormat::UNUSED:           return GL_RGBA; // should never happen (used to be rgbm)
        case PixelDataFormat::DEPTH_COMPONENT:  return GL_DEPTH_COMPONENT;
        case PixelDataFormat::ALPHA:            return GL_ALPHA;
        case PixelDataFormat::DEPTH_STENCIL:    return GL_DEPTH_STENCIL;
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        // when context is ES2 we should never end-up here
        case PixelDataFormat::R:                return GL_RED;
        case PixelDataFormat::R_INTEGER:        return GL_RED_INTEGER;
        case PixelDataFormat::RG:               return GL_RG;
        case PixelDataFormat::RG_INTEGER:       return GL_RG_INTEGER;
        case PixelDataFormat::RGB_INTEGER:      return GL_RGB_INTEGER;
        case PixelDataFormat::RGBA_INTEGER:     return GL_RGBA_INTEGER;
#else
        // silence compiler warning in ES2 headers mode
        default: return GL_NONE;
#endif
    }
    // should never happen
    return GL_RGBA;
}

constexpr GLenum getType(PixelDataType const type) noexcept {
    using PixelDataType = PixelDataType;
    switch (type) {
        case PixelDataType::UBYTE:                return GL_UNSIGNED_BYTE;
        case PixelDataType::BYTE:                 return GL_BYTE;
        case PixelDataType::USHORT:               return GL_UNSIGNED_SHORT;
        case PixelDataType::SHORT:                return GL_SHORT;
        case PixelDataType::UINT:                 return GL_UNSIGNED_INT;
        case PixelDataType::INT:                  return GL_INT;
        case PixelDataType::FLOAT:                return GL_FLOAT;
        case PixelDataType::USHORT_565:           return GL_UNSIGNED_SHORT_5_6_5;
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        // when context is ES2 we should never end-up here
        case PixelDataType::HALF:                 return GL_HALF_FLOAT;
        case PixelDataType::UINT_10F_11F_11F_REV: return GL_UNSIGNED_INT_10F_11F_11F_REV;
        case PixelDataType::UINT_2_10_10_10_REV:  return GL_UNSIGNED_INT_2_10_10_10_REV;
        case PixelDataType::COMPRESSED:           return 0; // should never happen
#else
        // silence compiler warning in ES2 headers mode
        default: return GL_NONE;
#endif
    }
    // should never happen
    return GL_UNSIGNED_INT;
}

#if !defined(__EMSCRIPTEN__)  && !defined(FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2)
constexpr GLenum getSwizzleChannel(TextureSwizzle const c) noexcept {
    using TextureSwizzle = TextureSwizzle;
    switch (c) {
        case TextureSwizzle::SUBSTITUTE_ZERO:   return GL_ZERO;
        case TextureSwizzle::SUBSTITUTE_ONE:    return GL_ONE;
        case TextureSwizzle::CHANNEL_0:         return GL_RED;
        case TextureSwizzle::CHANNEL_1:         return GL_GREEN;
        case TextureSwizzle::CHANNEL_2:         return GL_BLUE;
        case TextureSwizzle::CHANNEL_3:         return GL_ALPHA;
    }
    // should never happen
    return GL_RED;
}
#endif

constexpr GLenum getCullingMode(CullingMode const mode) noexcept {
    switch (mode) {
        case CullingMode::NONE:
            // should never happen
            return GL_FRONT_AND_BACK;
        case CullingMode::FRONT:
            return GL_FRONT;
        case CullingMode::BACK:
            return GL_BACK;
        case CullingMode::FRONT_AND_BACK:
            return GL_FRONT_AND_BACK;
    }
    // should never happen
    return GL_FRONT_AND_BACK;
}

// ES2 supported internal formats for texturing and how they  map to a format/type
constexpr std::pair<GLenum, GLenum> textureFormatToFormatAndType(
        TextureFormat const format) noexcept {
    switch (format) {
        case TextureFormat::R8:         return { 0x1909 /*GL_LUMINANCE*/, GL_UNSIGNED_BYTE };
        case TextureFormat::RGB8:       return { GL_RGB,                  GL_UNSIGNED_BYTE };
        case TextureFormat::SRGB8:      return { GL_RGB,                  GL_UNSIGNED_BYTE };
        case TextureFormat::RGBA8:      return { GL_RGBA,                 GL_UNSIGNED_BYTE };
        case TextureFormat::SRGB8_A8:   return { GL_RGBA,                 GL_UNSIGNED_BYTE };
        case TextureFormat::RGB565:     return { GL_RGB,                  GL_UNSIGNED_SHORT_5_6_5 };
        case TextureFormat::RGB5_A1:    return { GL_RGBA,                 GL_UNSIGNED_SHORT_5_5_5_1 };
        case TextureFormat::RGBA4:      return { GL_RGBA,                 GL_UNSIGNED_SHORT_4_4_4_4 };
        case TextureFormat::DEPTH16:    return { GL_DEPTH_COMPONENT,      GL_UNSIGNED_SHORT };
        case TextureFormat::DEPTH24:    return { GL_DEPTH_COMPONENT,      GL_UNSIGNED_INT };
        case TextureFormat::DEPTH24_STENCIL8:
                                        return { GL_DEPTH24_STENCIL8,     GL_UNSIGNED_INT_24_8 };
        default:                        return { GL_NONE,                 GL_NONE };
    }
}

// clang loses it on this one, and generates a huge jump table when
// inlined. So we don't  mark it as inline (only constexpr) which solves the problem,
// strangely, when not inlined, clang simply generates an array lookup.
constexpr /* inline */ GLenum getInternalFormat(TextureFormat const format) noexcept {
    switch (format) {

        /* Formats supported by our ES2 implementations */

        // 8-bits per element
        case TextureFormat::STENCIL8:          return GL_STENCIL_INDEX8;

        // 16-bits per element
        case TextureFormat::RGB565:            return GL_RGB565;
        case TextureFormat::RGB5_A1:           return GL_RGB5_A1;
        case TextureFormat::RGBA4:             return GL_RGBA4;
        case TextureFormat::DEPTH16:           return GL_DEPTH_COMPONENT16;

        // 24-bits per element
        case TextureFormat::RGB8:              return GL_RGB8;
        case TextureFormat::DEPTH24:           return GL_DEPTH_COMPONENT24;

        // 32-bits per element
        case TextureFormat::RGBA8:             return GL_RGBA8;
        case TextureFormat::DEPTH24_STENCIL8:  return GL_DEPTH24_STENCIL8;

        /* Formats not supported by our ES2 implementations */

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        // 8-bits per element
        case TextureFormat::R8:                return GL_R8;
        case TextureFormat::R8_SNORM:          return GL_R8_SNORM;
        case TextureFormat::R8UI:              return GL_R8UI;
        case TextureFormat::R8I:               return GL_R8I;

        // 16-bits per element
        case TextureFormat::R16F:              return GL_R16F;
        case TextureFormat::R16UI:             return GL_R16UI;
        case TextureFormat::R16I:              return GL_R16I;
        case TextureFormat::RG8:               return GL_RG8;
        case TextureFormat::RG8_SNORM:         return GL_RG8_SNORM;
        case TextureFormat::RG8UI:             return GL_RG8UI;
        case TextureFormat::RG8I:              return GL_RG8I;

        // 24-bits per element
        case TextureFormat::SRGB8:             return GL_SRGB8;
        case TextureFormat::RGB8_SNORM:        return GL_RGB8_SNORM;
        case TextureFormat::RGB8UI:            return GL_RGB8UI;
        case TextureFormat::RGB8I:             return GL_RGB8I;

        // 32-bits per element
        case TextureFormat::R32F:              return GL_R32F;
        case TextureFormat::R32UI:             return GL_R32UI;
        case TextureFormat::R32I:              return GL_R32I;
        case TextureFormat::RG16F:             return GL_RG16F;
        case TextureFormat::RG16UI:            return GL_RG16UI;
        case TextureFormat::RG16I:             return GL_RG16I;
        case TextureFormat::R11F_G11F_B10F:    return GL_R11F_G11F_B10F;
        case TextureFormat::RGB9_E5:           return GL_RGB9_E5;
        case TextureFormat::SRGB8_A8:          return GL_SRGB8_ALPHA8;
        case TextureFormat::RGBA8_SNORM:       return GL_RGBA8_SNORM;
        case TextureFormat::RGB10_A2:          return GL_RGB10_A2;
        case TextureFormat::RGBA8UI:           return GL_RGBA8UI;
        case TextureFormat::RGBA8I:            return GL_RGBA8I;
        case TextureFormat::DEPTH32F:          return GL_DEPTH_COMPONENT32F;
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
#else
        default:
            // this is just to squash the IDE warning about not having all cases when in
            // ES2 header mode.
            return 0;
#endif

        // compressed formats
#if defined(GL_ES_VERSION_3_0) || defined(BACKEND_OPENGL_VERSION_GL) || defined(GL_ARB_ES3_compatibility)
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

#if defined(GL_EXT_texture_compression_rgtc)
        case TextureFormat::RED_RGTC1:              return GL_COMPRESSED_RED_RGTC1_EXT;
        case TextureFormat::SIGNED_RED_RGTC1:       return GL_COMPRESSED_SIGNED_RED_RGTC1_EXT;
        case TextureFormat::RED_GREEN_RGTC2:        return GL_COMPRESSED_RED_GREEN_RGTC2_EXT;
        case TextureFormat::SIGNED_RED_GREEN_RGTC2: return GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT;
#else
        case TextureFormat::RED_RGTC1:
        case TextureFormat::SIGNED_RED_RGTC1:
        case TextureFormat::RED_GREEN_RGTC2:
        case TextureFormat::SIGNED_RED_GREEN_RGTC2:
            // this should not happen
            return 0;
#endif

#if defined(GL_EXT_texture_compression_bptc)
        case TextureFormat::RGB_BPTC_SIGNED_FLOAT:      return GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT;
        case TextureFormat::RGB_BPTC_UNSIGNED_FLOAT:    return GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT;
        case TextureFormat::RGBA_BPTC_UNORM:            return GL_COMPRESSED_RGBA_BPTC_UNORM_EXT;
        case TextureFormat::SRGB_ALPHA_BPTC_UNORM:      return GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT;
#else
        case TextureFormat::RGB_BPTC_SIGNED_FLOAT:
        case TextureFormat::RGB_BPTC_UNSIGNED_FLOAT:
        case TextureFormat::RGBA_BPTC_UNORM:
        case TextureFormat::SRGB_ALPHA_BPTC_UNORM:
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

class unordered_string_set : public std::unordered_set<std::string_view> {
public:
    bool has(std::string_view str) const noexcept;
};

unordered_string_set split(const char* extensions) noexcept;

} // namespace filament::backend::GLUtils


#endif // TNT_FILAMENT_BACKEND_OPENGL_GLUTILS_H
