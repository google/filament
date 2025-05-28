/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DRIVER_METALENUMS_H
#define TNT_FILAMENT_DRIVER_METALENUMS_H

#include "private/backend/Driver.h"

#include <Metal/Metal.h>

#include <utils/Panic.h>

#include <Availability.h>

namespace filament {
namespace backend {

struct MetalContext;

constexpr inline MTLCompareFunction getMetalCompareFunction(RasterState::DepthFunc func)
        noexcept {
    switch (func) {
        case RasterState::DepthFunc::LE:
            return MTLCompareFunctionLessEqual;
        case RasterState::DepthFunc::GE:
            return MTLCompareFunctionGreaterEqual;
        case RasterState::DepthFunc::L:
            return MTLCompareFunctionLess;
        case RasterState::DepthFunc::G:
            return MTLCompareFunctionGreater;
        case RasterState::DepthFunc::E:
            return MTLCompareFunctionEqual;
        case RasterState::DepthFunc::NE:
            return MTLCompareFunctionNotEqual;
        case RasterState::DepthFunc::A:
            return MTLCompareFunctionAlways;
        case RasterState::DepthFunc::N:
            return MTLCompareFunctionNever;
    }
}

constexpr inline MTLStencilOperation getMetalStencilOperation(StencilOperation op) {
    switch (op) {
        case StencilOperation::KEEP: return MTLStencilOperationKeep;
        case StencilOperation::ZERO: return MTLStencilOperationZero;
        case StencilOperation::REPLACE: return MTLStencilOperationReplace;
        case StencilOperation::INCR: return MTLStencilOperationIncrementClamp;
        case StencilOperation::INCR_WRAP: return MTLStencilOperationIncrementWrap;
        case StencilOperation::DECR: return MTLStencilOperationDecrementClamp;
        case StencilOperation::DECR_WRAP: return MTLStencilOperationDecrementWrap;
        case StencilOperation::INVERT: return MTLStencilOperationInvert;
    }
}

constexpr inline MTLIndexType getIndexType(size_t elementSize) noexcept {
    if (elementSize == 2) {
        return MTLIndexTypeUInt16;
    } else if (elementSize == 4) {
        return MTLIndexTypeUInt32;
    }
    assert_invariant(false);
    return MTLIndexTypeUInt16;
}

constexpr inline MTLVertexFormat getMetalFormat(ElementType type, bool normalized) noexcept {
    if (normalized) {
        switch (type) {
            // Single Component Types
#if MAC_OS_X_VERSION_MAX_ALLOWED > 101300 || __IPHONE_OS_VERSION_MAX_ALLOWED > 110000
            case ElementType::BYTE: return MTLVertexFormatCharNormalized;
            case ElementType::UBYTE: return MTLVertexFormatUCharNormalized;
            case ElementType::SHORT: return MTLVertexFormatShortNormalized;
            case ElementType::USHORT: return MTLVertexFormatUShortNormalized;
#endif
            // Two Component Types
            case ElementType::BYTE2: return MTLVertexFormatChar2Normalized;
            case ElementType::UBYTE2: return MTLVertexFormatUChar2Normalized;
            case ElementType::SHORT2: return MTLVertexFormatShort2Normalized;
            case ElementType::USHORT2: return MTLVertexFormatUShort2Normalized;
            // Three Component Types
            case ElementType::BYTE3: return MTLVertexFormatChar3Normalized;
            case ElementType::UBYTE3: return MTLVertexFormatUChar3Normalized;
            case ElementType::SHORT3: return MTLVertexFormatShort3Normalized;
            case ElementType::USHORT3: return MTLVertexFormatUShort3Normalized;
            // Four Component Types
            case ElementType::BYTE4: return MTLVertexFormatChar4Normalized;
            case ElementType::UBYTE4: return MTLVertexFormatUChar4Normalized;
            case ElementType::SHORT4: return MTLVertexFormatShort4Normalized;
            case ElementType::USHORT4: return MTLVertexFormatUShort4Normalized;
            default:
                FILAMENT_CHECK_POSTCONDITION(false) << "Normalized format does not exist.";
                return MTLVertexFormatInvalid;
        }
    }
    switch (type) {
        // Single Component Types
#if MAC_OS_X_VERSION_MAX_ALLOWED > 101300 || __IPHONE_OS_VERSION_MAX_ALLOWED > 110000
        case ElementType::BYTE: return MTLVertexFormatChar;
        case ElementType::UBYTE: return MTLVertexFormatUChar;
        case ElementType::SHORT: return MTLVertexFormatShort;
        case ElementType::USHORT: return MTLVertexFormatUShort;
        case ElementType::HALF: return MTLVertexFormatHalf;
#endif
        case ElementType::INT: return MTLVertexFormatInt;
        case ElementType::UINT: return MTLVertexFormatUInt;
        case ElementType::FLOAT: return MTLVertexFormatFloat;
        // Two Component Types
        case ElementType::BYTE2: return MTLVertexFormatChar2;
        case ElementType::UBYTE2: return MTLVertexFormatUChar2;
        case ElementType::SHORT2: return MTLVertexFormatShort2;
        case ElementType::USHORT2: return MTLVertexFormatUShort2;
        case ElementType::HALF2: return MTLVertexFormatHalf2;
        case ElementType::FLOAT2: return MTLVertexFormatFloat2;
        // Three Component Types
        case ElementType::BYTE3: return MTLVertexFormatChar3;
        case ElementType::UBYTE3: return MTLVertexFormatUChar3;
        case ElementType::SHORT3: return MTLVertexFormatShort3;
        case ElementType::USHORT3: return MTLVertexFormatUShort3;
        case ElementType::HALF3: return MTLVertexFormatHalf3;
        case ElementType::FLOAT3: return MTLVertexFormatFloat3;
        // Four Component Types
        case ElementType::BYTE4: return MTLVertexFormatChar4;
        case ElementType::UBYTE4: return MTLVertexFormatUChar4;
        case ElementType::SHORT4: return MTLVertexFormatShort4;
        case ElementType::USHORT4: return MTLVertexFormatUShort4;
        case ElementType::HALF4: return MTLVertexFormatHalf4;
        case ElementType::FLOAT4: return MTLVertexFormatFloat4;
    }
    return MTLVertexFormatInvalid;
}

MTLPixelFormat getMetalFormat(MetalContext* context, TextureFormat format) noexcept;

// Converts PixelBufferDescriptor format + type pair into a MTLPixelFormat.
inline MTLPixelFormat getMetalFormat(PixelDataFormat format, PixelDataType type) {
    if (type == PixelDataType::UINT_2_10_10_10_REV) return MTLPixelFormatRGB10A2Unorm;
    if (type == PixelDataType::UINT_10F_11F_11F_REV) return MTLPixelFormatRG11B10Float;
    if (@available(macOS 11, *)) {
        if (type == PixelDataType::USHORT_565) return MTLPixelFormatB5G6R5Unorm;
    }

    #define CONVERT(FORMAT, TYPE, MTL) \
    if (PixelDataFormat::FORMAT == format && PixelDataType::TYPE == type)  return MTLPixelFormat ## MTL;

    CONVERT(R, UBYTE, R8Unorm);
    CONVERT(R, BYTE, R8Snorm);
    CONVERT(R_INTEGER, UBYTE, R8Uint);
    CONVERT(R_INTEGER, BYTE, R8Sint);
    CONVERT(RG, UBYTE, RG8Unorm);
    CONVERT(RG, BYTE, RG8Snorm);
    CONVERT(RG_INTEGER, UBYTE, RG8Uint);
    CONVERT(RG_INTEGER, BYTE, RG8Sint);
    CONVERT(RGBA, UBYTE, RGBA8Unorm);
    CONVERT(RGBA, BYTE, RGBA8Snorm);
    CONVERT(RGBA_INTEGER, UBYTE, RGBA8Uint);
    CONVERT(RGBA_INTEGER, BYTE, RGBA8Sint);
    CONVERT(R_INTEGER, USHORT, R16Uint);
    CONVERT(R_INTEGER, SHORT, R16Sint);
    CONVERT(R, HALF, R16Float);
    CONVERT(RG_INTEGER, USHORT, RG16Uint);
    CONVERT(RG_INTEGER, SHORT, RG16Sint);
    CONVERT(RG, HALF, RG16Float);
    CONVERT(RGBA_INTEGER, USHORT, RGBA16Uint);
    CONVERT(RGBA_INTEGER, SHORT, RGBA16Sint);
    CONVERT(RGBA, HALF, RGBA16Float);
    CONVERT(R_INTEGER, UINT, R32Uint);
    CONVERT(R_INTEGER, INT, R32Sint);
    CONVERT(R, FLOAT, R32Float);
    CONVERT(RG_INTEGER, UINT, RG32Uint);
    CONVERT(RG_INTEGER, INT, RG32Sint);
    CONVERT(RG, FLOAT, RG32Float);
    CONVERT(RGBA_INTEGER, UINT, RGBA32Uint);
    CONVERT(RGBA_INTEGER, INT, RGBA32Sint);
    CONVERT(RGBA, FLOAT, RGBA32Float);
    CONVERT(DEPTH_COMPONENT, FLOAT, Depth32Float);
    #undef CONVERT

    return MTLPixelFormatInvalid;
}

inline MTLPixelFormat getMetalFormatLinear(MTLPixelFormat format) {
    switch (format) {
        case MTLPixelFormatR8Unorm_sRGB: return MTLPixelFormatR8Unorm;
        case MTLPixelFormatRG8Unorm_sRGB: return MTLPixelFormatRG8Unorm;
        case MTLPixelFormatRGBA8Unorm_sRGB: return MTLPixelFormatRGBA8Unorm;
        case MTLPixelFormatBGRA8Unorm_sRGB: return MTLPixelFormatBGRA8Unorm;
#if !defined(FILAMENT_IOS)
        case MTLPixelFormatBC1_RGBA_sRGB: return MTLPixelFormatBC1_RGBA;
        case MTLPixelFormatBC2_RGBA_sRGB: return MTLPixelFormatBC2_RGBA;
        case MTLPixelFormatBC3_RGBA_sRGB: return MTLPixelFormatBC3_RGBA;
        case MTLPixelFormatBC7_RGBAUnorm_sRGB: return MTLPixelFormatBC7_RGBAUnorm;
#endif
        default: break;
    }
    if (@available(macOS 11, *)) {
        switch (format) {
            case MTLPixelFormatASTC_4x4_sRGB: return MTLPixelFormatASTC_4x4_LDR;
            case MTLPixelFormatASTC_5x4_sRGB: return MTLPixelFormatASTC_5x4_LDR;
            case MTLPixelFormatASTC_5x5_sRGB: return MTLPixelFormatASTC_5x5_LDR;
            case MTLPixelFormatASTC_6x5_sRGB: return MTLPixelFormatASTC_6x5_LDR;
            case MTLPixelFormatASTC_6x6_sRGB: return MTLPixelFormatASTC_6x6_LDR;
            case MTLPixelFormatASTC_8x5_sRGB: return MTLPixelFormatASTC_8x5_LDR;
            case MTLPixelFormatASTC_8x6_sRGB: return MTLPixelFormatASTC_8x6_LDR;
            case MTLPixelFormatASTC_8x8_sRGB: return MTLPixelFormatASTC_8x8_LDR;
            case MTLPixelFormatASTC_10x5_sRGB: return MTLPixelFormatASTC_10x5_LDR;
            case MTLPixelFormatASTC_10x6_sRGB: return MTLPixelFormatASTC_10x6_LDR;
            case MTLPixelFormatASTC_10x8_sRGB: return MTLPixelFormatASTC_10x8_LDR;
            case MTLPixelFormatASTC_10x10_sRGB: return MTLPixelFormatASTC_10x10_LDR;
            case MTLPixelFormatASTC_12x10_sRGB: return MTLPixelFormatASTC_12x10_LDR;
            case MTLPixelFormatASTC_12x12_sRGB: return MTLPixelFormatASTC_12x12_LDR;
            case MTLPixelFormatPVRTC_RGB_2BPP_sRGB: return MTLPixelFormatPVRTC_RGB_2BPP;
            case MTLPixelFormatPVRTC_RGB_4BPP_sRGB: return MTLPixelFormatPVRTC_RGB_4BPP;
            case MTLPixelFormatPVRTC_RGBA_2BPP_sRGB: return MTLPixelFormatPVRTC_RGBA_2BPP;
            case MTLPixelFormatPVRTC_RGBA_4BPP_sRGB: return MTLPixelFormatPVRTC_RGBA_4BPP;
            case MTLPixelFormatEAC_RGBA8_sRGB: return MTLPixelFormatEAC_RGBA8;
            case MTLPixelFormatETC2_RGB8_sRGB: return MTLPixelFormatETC2_RGB8;
            case MTLPixelFormatETC2_RGB8A1_sRGB: return MTLPixelFormatETC2_RGB8A1;
            case MTLPixelFormatBGR10_XR_sRGB: return MTLPixelFormatBGR10_XR;
            case MTLPixelFormatBGRA10_XR_sRGB: return MTLPixelFormatBGRA10_XR;
            default: break;
        }
    }
    return format;
}

constexpr inline bool isMetalFormatInteger(MTLPixelFormat format) {
    switch (format) {
        case MTLPixelFormatR8Uint:
        case MTLPixelFormatR8Sint:
        case MTLPixelFormatR16Uint:
        case MTLPixelFormatR16Sint:
        case MTLPixelFormatRG8Uint:
        case MTLPixelFormatRG8Sint:
        case MTLPixelFormatR32Uint:
        case MTLPixelFormatR32Sint:
        case MTLPixelFormatRG16Uint:
        case MTLPixelFormatRG16Sint:
        case MTLPixelFormatRGBA8Uint:
        case MTLPixelFormatRGBA8Sint:
        case MTLPixelFormatRGB10A2Uint:
        case MTLPixelFormatRG32Uint:
        case MTLPixelFormatRG32Sint:
        case MTLPixelFormatRGBA16Uint:
        case MTLPixelFormatRGBA16Sint:
        case MTLPixelFormatRGBA32Uint:
        case MTLPixelFormatRGBA32Sint:
            return true;

        default:
            return false;
    }
}

constexpr inline bool isMetalFormatStencil(MTLPixelFormat format) {
    switch (format) {
        case MTLPixelFormatStencil8:
        case MTLPixelFormatDepth32Float_Stencil8:
        case MTLPixelFormatX32_Stencil8:
#if !defined(FILAMENT_IOS)
        case MTLPixelFormatDepth24Unorm_Stencil8:
        case MTLPixelFormatX24_Stencil8:
#endif
            return true;

        default:
            return false;
    }
}

constexpr inline MTLTextureType getMetalType(SamplerType target) {
    switch (target) {
        case SamplerType::SAMPLER_2D:
        case SamplerType::SAMPLER_EXTERNAL:
            return MTLTextureType2D;
        case SamplerType::SAMPLER_2D_ARRAY:
            return MTLTextureType2DArray;
        case SamplerType::SAMPLER_CUBEMAP:
            return MTLTextureTypeCube;
        case SamplerType::SAMPLER_3D:
            return MTLTextureType3D;
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            return MTLTextureTypeCubeArray;
    }
}

constexpr inline MTLBlendOperation getMetalBlendOperation(BlendEquation equation) noexcept {
    switch (equation) {
        case BlendEquation::ADD: return MTLBlendOperationAdd;
        case BlendEquation::SUBTRACT: return MTLBlendOperationSubtract;
        case BlendEquation::REVERSE_SUBTRACT: return MTLBlendOperationReverseSubtract;
        case BlendEquation::MIN: return MTLBlendOperationMin;
        case BlendEquation::MAX: return MTLBlendOperationMax;
    }
}

constexpr inline MTLBlendFactor getMetalBlendFactor(BlendFunction function) noexcept {
    switch (function) {
        case BlendFunction::ZERO: return MTLBlendFactorZero;
        case BlendFunction::ONE: return MTLBlendFactorOne;
        case BlendFunction::SRC_COLOR: return MTLBlendFactorSourceColor;
        case BlendFunction::ONE_MINUS_SRC_COLOR: return MTLBlendFactorOneMinusSourceColor;
        case BlendFunction::DST_COLOR: return MTLBlendFactorDestinationColor;
        case BlendFunction::ONE_MINUS_DST_COLOR: return MTLBlendFactorOneMinusDestinationColor;
        case BlendFunction::SRC_ALPHA: return MTLBlendFactorSourceAlpha;
        case BlendFunction::ONE_MINUS_SRC_ALPHA: return MTLBlendFactorOneMinusSourceAlpha;
        case BlendFunction::DST_ALPHA: return MTLBlendFactorDestinationAlpha;
        case BlendFunction::ONE_MINUS_DST_ALPHA: return MTLBlendFactorOneMinusDestinationAlpha;
        case BlendFunction::SRC_ALPHA_SATURATE: return MTLBlendFactorSourceAlphaSaturated;
    }
}

constexpr inline MTLCullMode getMetalCullMode(CullingMode cullMode) noexcept {
    switch (cullMode) {
        case CullingMode::NONE: return MTLCullModeNone;
        case CullingMode::FRONT: return MTLCullModeFront;
        case CullingMode::BACK: return MTLCullModeBack;
        case CullingMode::FRONT_AND_BACK:
            FILAMENT_CHECK_POSTCONDITION(false)
                    << "FRONT_AND_BACK culling is not supported in Metal.";
    }
}

constexpr inline MTLPrimitiveType getMetalPrimitiveType(PrimitiveType type) noexcept {
    switch (type) {
        case PrimitiveType::POINTS: return MTLPrimitiveTypePoint;
        case PrimitiveType::LINES: return MTLPrimitiveTypeLine;
        case PrimitiveType::LINE_STRIP: return MTLPrimitiveTypeLineStrip;
        case PrimitiveType::TRIANGLES: return MTLPrimitiveTypeTriangle;
        case PrimitiveType::TRIANGLE_STRIP: return MTLPrimitiveTypeTriangleStrip;
    }
}

constexpr inline MTLSamplerMinMagFilter getFilter(SamplerMinFilter filter) noexcept {
    switch (filter) {
        case SamplerMinFilter::NEAREST:
        case SamplerMinFilter::NEAREST_MIPMAP_NEAREST:
        case SamplerMinFilter::NEAREST_MIPMAP_LINEAR:
            return MTLSamplerMinMagFilterLinear;
        case SamplerMinFilter::LINEAR_MIPMAP_NEAREST:
        case SamplerMinFilter::LINEAR:
        case SamplerMinFilter::LINEAR_MIPMAP_LINEAR:
            return MTLSamplerMinMagFilterLinear;
    }
}

constexpr inline MTLSamplerMinMagFilter getFilter(SamplerMagFilter filter) noexcept {
    switch (filter) {
        case SamplerMagFilter::NEAREST:
            return MTLSamplerMinMagFilterLinear;
        case SamplerMagFilter::LINEAR:
            return MTLSamplerMinMagFilterLinear;
    }
}

constexpr inline MTLSamplerMipFilter getMipFilter(SamplerMinFilter filter) noexcept {
    switch (filter) {
        case SamplerMinFilter::NEAREST:
        case SamplerMinFilter::LINEAR:
            return MTLSamplerMipFilterLinear;
        case SamplerMinFilter::NEAREST_MIPMAP_NEAREST:
        case SamplerMinFilter::LINEAR_MIPMAP_NEAREST:
            return MTLSamplerMipFilterLinear;
        case SamplerMinFilter::NEAREST_MIPMAP_LINEAR:
        case SamplerMinFilter::LINEAR_MIPMAP_LINEAR:
            return MTLSamplerMipFilterLinear;
    }
}

constexpr inline MTLSamplerAddressMode getAddressMode(SamplerWrapMode wrapMode) noexcept {
    switch (wrapMode) {
        case SamplerWrapMode::CLAMP_TO_EDGE:
            return MTLSamplerAddressModeClampToEdge;
        case SamplerWrapMode::REPEAT:
            return MTLSamplerAddressModeRepeat;
        case SamplerWrapMode::MIRRORED_REPEAT:
            return MTLSamplerAddressModeMirrorRepeat;
    }
}

constexpr inline MTLCompareFunction getCompareFunction(SamplerCompareFunc compareFunc) noexcept {
    switch (compareFunc) {
        case SamplerCompareFunc::LE:
            return MTLCompareFunctionLessEqual;
        case SamplerCompareFunc::GE:
            return MTLCompareFunctionGreaterEqual;
        case SamplerCompareFunc::L:
            return MTLCompareFunctionLess;
        case SamplerCompareFunc::G:
            return MTLCompareFunctionGreater;
        case SamplerCompareFunc::E:
            return MTLCompareFunctionEqual;
        case SamplerCompareFunc::NE:
            return MTLCompareFunctionNotEqual;
        case SamplerCompareFunc::A:
            return MTLCompareFunctionAlways;
        case SamplerCompareFunc::N:
            return MTLCompareFunctionNever;
    }
}

API_AVAILABLE(ios(13.0))
constexpr inline MTLTextureSwizzle getSwizzle(TextureSwizzle swizzle) {
    switch (swizzle) {
        case TextureSwizzle::SUBSTITUTE_ZERO:
            return MTLTextureSwizzleZero;
        case TextureSwizzle::SUBSTITUTE_ONE:
            return MTLTextureSwizzleOne;
        case TextureSwizzle::CHANNEL_0:
            return MTLTextureSwizzleRed;
        case TextureSwizzle::CHANNEL_1:
            return MTLTextureSwizzleGreen;
        case TextureSwizzle::CHANNEL_2:
            return MTLTextureSwizzleBlue;
        case TextureSwizzle::CHANNEL_3:
            return MTLTextureSwizzleAlpha;
    }
}

API_AVAILABLE(ios(13.0))
inline MTLTextureSwizzleChannels getSwizzleChannels(TextureSwizzle r, TextureSwizzle g, TextureSwizzle b,
        TextureSwizzle a) {
    return MTLTextureSwizzleChannelsMake(getSwizzle(r), getSwizzle(g), getSwizzle(b),
            getSwizzle(a));
}

} // namespace backend
} // namespace filament

#endif //TNT_FILAMENT_DRIVER_METALENUMS_H
