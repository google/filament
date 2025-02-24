// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/dawn/node/binding/Converter.h"

#include <cassert>
#include <sstream>

#include "src/dawn/node/binding/GPUBuffer.h"
#include "src/dawn/node/binding/GPUPipelineLayout.h"
#include "src/dawn/node/binding/GPUQuerySet.h"
#include "src/dawn/node/binding/GPUSampler.h"
#include "src/dawn/node/binding/GPUShaderModule.h"
#include "src/dawn/node/binding/GPUTexture.h"
#include "src/dawn/node/binding/GPUTextureView.h"
#include "src/dawn/node/utils/Debug.h"

namespace wgpu::binding {

Converter::~Converter() {
    for (auto& free : free_) {
        free();
    }
}

bool Converter::HasFeature(wgpu::FeatureName feature) {
    // Not all uses of the converter will have a device (for example for adapter-related
    // conversions).
    assert(device.Get() != nullptr);
    return device.HasFeature(feature);
}

bool Converter::Convert(wgpu::Extent3D& out, const interop::GPUExtent3D& in) {
    out = {};
    if (auto* dict = std::get_if<interop::GPUExtent3DDict>(&in)) {
        out.depthOrArrayLayers = dict->depthOrArrayLayers;
        out.width = dict->width;
        out.height = dict->height;
        return true;
    }
    if (auto* vec = std::get_if<std::vector<interop::GPUIntegerCoordinate>>(&in)) {
        switch (vec->size()) {
            default:
            case 3:
                out.depthOrArrayLayers = (*vec)[2];
            case 2:  // fallthrough
                out.height = (*vec)[1];
            case 1:  // fallthrough
                out.width = (*vec)[0];
                return true;
            case 0:
                break;
        }
    }
    return Throw("invalid value for GPUExtent3D");
}

bool Converter::Convert(wgpu::Origin3D& out, const interop::GPUOrigin3DDict& in) {
    out = {};
    out.x = in.x;
    out.y = in.y;
    out.z = in.z;
    return true;
}

bool Converter::Convert(wgpu::Color& out, const interop::GPUColor& in) {
    out = {};
    if (auto* dict = std::get_if<interop::GPUColorDict>(&in)) {
        out.r = dict->r;
        out.g = dict->g;
        out.b = dict->b;
        out.a = dict->a;
        return true;
    }
    if (auto* vec = std::get_if<std::vector<double>>(&in)) {
        switch (vec->size()) {
            default:
            case 4:
                out.a = (*vec)[3];
            case 3:  // fallthrough
                out.b = (*vec)[2];
            case 2:  // fallthrough
                out.g = (*vec)[1];
            case 1:  // fallthrough
                out.r = (*vec)[0];
                return true;
            case 0:
                break;
        }
    }
    return Throw("invalid value for GPUColor");
}

bool Converter::Convert(wgpu::Origin3D& out, const std::vector<interop::GPUIntegerCoordinate>& in) {
    out = {};
    switch (in.size()) {
        default:
        case 3:
            out.z = in[2];
        case 2:  // fallthrough
            out.y = in[1];
        case 1:  // fallthrough
            out.x = in[0];
        case 0:
            break;
    }
    return true;
}

bool Converter::Convert(wgpu::TextureAspect& out, const interop::GPUTextureAspect& in) {
    out = wgpu::TextureAspect::All;
    switch (in) {
        case interop::GPUTextureAspect::kAll:
            out = wgpu::TextureAspect::All;
            return true;
        case interop::GPUTextureAspect::kStencilOnly:
            out = wgpu::TextureAspect::StencilOnly;
            return true;
        case interop::GPUTextureAspect::kDepthOnly:
            out = wgpu::TextureAspect::DepthOnly;
            return true;
    }
    return Throw("invalid value for GPUTextureAspect");
}

bool Converter::Convert(wgpu::TexelCopyTextureInfo& out,
                        const interop::GPUTexelCopyTextureInfo& in) {
    out = {};
    return Convert(out.texture, in.texture) && Convert(out.mipLevel, in.mipLevel) &&
           Convert(out.origin, in.origin) && Convert(out.aspect, in.aspect);
}

bool Converter::Convert(wgpu::TexelCopyBufferInfo& out, const interop::GPUTexelCopyBufferInfo& in) {
    out = {};
    out.buffer = *in.buffer.As<GPUBuffer>();
    return Convert(out.layout.offset, in.offset) &&
           Convert(out.layout.bytesPerRow, in.bytesPerRow) &&
           Convert(out.layout.rowsPerImage, in.rowsPerImage);
}

bool Converter::Convert(BufferSource& out, interop::BufferSource in) {
    out = {};
    if (auto* view = std::get_if<interop::ArrayBufferView>(&in)) {
        std::visit(
            [&](auto&& v) {
                auto arr = v.ArrayBuffer();
                out.data = static_cast<uint8_t*>(arr.Data()) + v.ByteOffset();
                out.size = v.ByteLength();
                out.bytesPerElement = v.ElementSize();
            },
            *view);
        return true;
    }
    if (auto* arr = std::get_if<interop::ArrayBuffer>(&in)) {
        out.data = arr->Data();
        out.size = arr->ByteLength();
        out.bytesPerElement = 1;
        return true;
    }
    return Throw("invalid value for BufferSource");
}

bool Converter::Convert(wgpu::TexelCopyBufferLayout& out,
                        const interop::GPUTexelCopyBufferLayout& in) {
    out = {};
    return Convert(out.bytesPerRow, in.bytesPerRow) && Convert(out.offset, in.offset) &&
           Convert(out.rowsPerImage, in.rowsPerImage);
}

bool Converter::Convert(wgpu::TextureFormat& out, const interop::GPUTextureFormat& in) {
    out = wgpu::TextureFormat::Undefined;
    wgpu::FeatureName requiredFeature = wgpu::FeatureName(0u);
    switch (in) {
        case interop::GPUTextureFormat::kR8Unorm:
            out = wgpu::TextureFormat::R8Unorm;
            return true;
        case interop::GPUTextureFormat::kR8Snorm:
            out = wgpu::TextureFormat::R8Snorm;
            return true;
        case interop::GPUTextureFormat::kR8Uint:
            out = wgpu::TextureFormat::R8Uint;
            return true;
        case interop::GPUTextureFormat::kR8Sint:
            out = wgpu::TextureFormat::R8Sint;
            return true;
        case interop::GPUTextureFormat::kR16Uint:
            out = wgpu::TextureFormat::R16Uint;
            return true;
        case interop::GPUTextureFormat::kR16Sint:
            out = wgpu::TextureFormat::R16Sint;
            return true;
        case interop::GPUTextureFormat::kR16Float:
            out = wgpu::TextureFormat::R16Float;
            return true;
        case interop::GPUTextureFormat::kRg8Unorm:
            out = wgpu::TextureFormat::RG8Unorm;
            return true;
        case interop::GPUTextureFormat::kRg8Snorm:
            out = wgpu::TextureFormat::RG8Snorm;
            return true;
        case interop::GPUTextureFormat::kRg8Uint:
            out = wgpu::TextureFormat::RG8Uint;
            return true;
        case interop::GPUTextureFormat::kRg8Sint:
            out = wgpu::TextureFormat::RG8Sint;
            return true;
        case interop::GPUTextureFormat::kR32Uint:
            out = wgpu::TextureFormat::R32Uint;
            return true;
        case interop::GPUTextureFormat::kR32Sint:
            out = wgpu::TextureFormat::R32Sint;
            return true;
        case interop::GPUTextureFormat::kR32Float:
            out = wgpu::TextureFormat::R32Float;
            return true;
        case interop::GPUTextureFormat::kRg16Uint:
            out = wgpu::TextureFormat::RG16Uint;
            return true;
        case interop::GPUTextureFormat::kRg16Sint:
            out = wgpu::TextureFormat::RG16Sint;
            return true;
        case interop::GPUTextureFormat::kRg16Float:
            out = wgpu::TextureFormat::RG16Float;
            return true;
        case interop::GPUTextureFormat::kRgba8Unorm:
            out = wgpu::TextureFormat::RGBA8Unorm;
            return true;
        case interop::GPUTextureFormat::kRgba8UnormSrgb:
            out = wgpu::TextureFormat::RGBA8UnormSrgb;
            return true;
        case interop::GPUTextureFormat::kRgba8Snorm:
            out = wgpu::TextureFormat::RGBA8Snorm;
            return true;
        case interop::GPUTextureFormat::kRgba8Uint:
            out = wgpu::TextureFormat::RGBA8Uint;
            return true;
        case interop::GPUTextureFormat::kRgba8Sint:
            out = wgpu::TextureFormat::RGBA8Sint;
            return true;
        case interop::GPUTextureFormat::kBgra8Unorm:
            out = wgpu::TextureFormat::BGRA8Unorm;
            return true;
        case interop::GPUTextureFormat::kBgra8UnormSrgb:
            out = wgpu::TextureFormat::BGRA8UnormSrgb;
            return true;
        case interop::GPUTextureFormat::kRgb9E5Ufloat:
            out = wgpu::TextureFormat::RGB9E5Ufloat;
            return true;
        case interop::GPUTextureFormat::kRgb10A2Uint:
            out = wgpu::TextureFormat::RGB10A2Uint;
            return true;
        case interop::GPUTextureFormat::kRgb10A2Unorm:
            out = wgpu::TextureFormat::RGB10A2Unorm;
            return true;
        case interop::GPUTextureFormat::kRg11B10Ufloat:
            out = wgpu::TextureFormat::RG11B10Ufloat;
            return true;
        case interop::GPUTextureFormat::kRg32Uint:
            out = wgpu::TextureFormat::RG32Uint;
            return true;
        case interop::GPUTextureFormat::kRg32Sint:
            out = wgpu::TextureFormat::RG32Sint;
            return true;
        case interop::GPUTextureFormat::kRg32Float:
            out = wgpu::TextureFormat::RG32Float;
            return true;
        case interop::GPUTextureFormat::kRgba16Uint:
            out = wgpu::TextureFormat::RGBA16Uint;
            return true;
        case interop::GPUTextureFormat::kRgba16Sint:
            out = wgpu::TextureFormat::RGBA16Sint;
            return true;
        case interop::GPUTextureFormat::kRgba16Float:
            out = wgpu::TextureFormat::RGBA16Float;
            return true;
        case interop::GPUTextureFormat::kRgba32Uint:
            out = wgpu::TextureFormat::RGBA32Uint;
            return true;
        case interop::GPUTextureFormat::kRgba32Sint:
            out = wgpu::TextureFormat::RGBA32Sint;
            return true;
        case interop::GPUTextureFormat::kRgba32Float:
            out = wgpu::TextureFormat::RGBA32Float;
            return true;
        case interop::GPUTextureFormat::kStencil8:
            out = wgpu::TextureFormat::Stencil8;
            return true;
        case interop::GPUTextureFormat::kDepth16Unorm:
            out = wgpu::TextureFormat::Depth16Unorm;
            return true;
        case interop::GPUTextureFormat::kDepth24Plus:
            out = wgpu::TextureFormat::Depth24Plus;
            return true;
        case interop::GPUTextureFormat::kDepth24PlusStencil8:
            out = wgpu::TextureFormat::Depth24PlusStencil8;
            return true;
        case interop::GPUTextureFormat::kDepth32Float:
            out = wgpu::TextureFormat::Depth32Float;
            return true;

        case interop::GPUTextureFormat::kDepth32FloatStencil8:
            out = wgpu::TextureFormat::Depth32FloatStencil8;
            requiredFeature = wgpu::FeatureName::Depth32FloatStencil8;
            break;
        case interop::GPUTextureFormat::kBc1RgbaUnorm:
            out = wgpu::TextureFormat::BC1RGBAUnorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc1RgbaUnormSrgb:
            out = wgpu::TextureFormat::BC1RGBAUnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc2RgbaUnorm:
            out = wgpu::TextureFormat::BC2RGBAUnorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc2RgbaUnormSrgb:
            out = wgpu::TextureFormat::BC2RGBAUnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc3RgbaUnorm:
            out = wgpu::TextureFormat::BC3RGBAUnorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc3RgbaUnormSrgb:
            out = wgpu::TextureFormat::BC3RGBAUnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc4RUnorm:
            out = wgpu::TextureFormat::BC4RUnorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc4RSnorm:
            out = wgpu::TextureFormat::BC4RSnorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc5RgUnorm:
            out = wgpu::TextureFormat::BC5RGUnorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc5RgSnorm:
            out = wgpu::TextureFormat::BC5RGSnorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc6HRgbUfloat:
            out = wgpu::TextureFormat::BC6HRGBUfloat;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc6HRgbFloat:
            out = wgpu::TextureFormat::BC6HRGBFloat;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc7RgbaUnorm:
            out = wgpu::TextureFormat::BC7RGBAUnorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc7RgbaUnormSrgb:
            out = wgpu::TextureFormat::BC7RGBAUnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kEtc2Rgb8Unorm:
            out = wgpu::TextureFormat::ETC2RGB8Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionETC2;
            break;
        case interop::GPUTextureFormat::kEtc2Rgb8UnormSrgb:
            out = wgpu::TextureFormat::ETC2RGB8UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionETC2;
            break;
        case interop::GPUTextureFormat::kEtc2Rgb8A1Unorm:
            out = wgpu::TextureFormat::ETC2RGB8A1Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionETC2;
            break;
        case interop::GPUTextureFormat::kEtc2Rgb8A1UnormSrgb:
            out = wgpu::TextureFormat::ETC2RGB8A1UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionETC2;
            break;
        case interop::GPUTextureFormat::kEtc2Rgba8Unorm:
            out = wgpu::TextureFormat::ETC2RGBA8Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionETC2;
            break;
        case interop::GPUTextureFormat::kEtc2Rgba8UnormSrgb:
            out = wgpu::TextureFormat::ETC2RGBA8UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionETC2;
            break;
        case interop::GPUTextureFormat::kEacR11Unorm:
            out = wgpu::TextureFormat::EACR11Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionETC2;
            break;
        case interop::GPUTextureFormat::kEacR11Snorm:
            out = wgpu::TextureFormat::EACR11Snorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionETC2;
            break;
        case interop::GPUTextureFormat::kEacRg11Unorm:
            out = wgpu::TextureFormat::EACRG11Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionETC2;
            break;
        case interop::GPUTextureFormat::kEacRg11Snorm:
            out = wgpu::TextureFormat::EACRG11Snorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionETC2;
            break;
        case interop::GPUTextureFormat::kAstc4X4Unorm:
            out = wgpu::TextureFormat::ASTC4x4Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc4X4UnormSrgb:
            out = wgpu::TextureFormat::ASTC4x4UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc5X4Unorm:
            out = wgpu::TextureFormat::ASTC5x4Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc5X4UnormSrgb:
            out = wgpu::TextureFormat::ASTC5x4UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc5X5Unorm:
            out = wgpu::TextureFormat::ASTC5x5Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc5X5UnormSrgb:
            out = wgpu::TextureFormat::ASTC5x5UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc6X5Unorm:
            out = wgpu::TextureFormat::ASTC6x5Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc6X5UnormSrgb:
            out = wgpu::TextureFormat::ASTC6x5UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc6X6Unorm:
            out = wgpu::TextureFormat::ASTC6x6Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc6X6UnormSrgb:
            out = wgpu::TextureFormat::ASTC6x6UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc8X5Unorm:
            out = wgpu::TextureFormat::ASTC8x5Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc8X5UnormSrgb:
            out = wgpu::TextureFormat::ASTC8x5UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc8X6Unorm:
            out = wgpu::TextureFormat::ASTC8x6Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc8X6UnormSrgb:
            out = wgpu::TextureFormat::ASTC8x6UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc8X8Unorm:
            out = wgpu::TextureFormat::ASTC8x8Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc8X8UnormSrgb:
            out = wgpu::TextureFormat::ASTC8x8UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc10X5Unorm:
            out = wgpu::TextureFormat::ASTC10x5Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc10X5UnormSrgb:
            out = wgpu::TextureFormat::ASTC10x5UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc10X6Unorm:
            out = wgpu::TextureFormat::ASTC10x6Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc10X6UnormSrgb:
            out = wgpu::TextureFormat::ASTC10x6UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc10X8Unorm:
            out = wgpu::TextureFormat::ASTC10x8Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc10X8UnormSrgb:
            out = wgpu::TextureFormat::ASTC10x8UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc10X10Unorm:
            out = wgpu::TextureFormat::ASTC10x10Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc10X10UnormSrgb:
            out = wgpu::TextureFormat::ASTC10x10UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc12X10Unorm:
            out = wgpu::TextureFormat::ASTC12x10Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc12X10UnormSrgb:
            out = wgpu::TextureFormat::ASTC12x10UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc12X12Unorm:
            out = wgpu::TextureFormat::ASTC12x12Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc12X12UnormSrgb:
            out = wgpu::TextureFormat::ASTC12x12UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;

        default:
            std::stringstream err;
            err << "unknown GPUTextureFormat(" << static_cast<int>(in) << ")";
            return Throw(err.str());
    }

    assert(requiredFeature != wgpu::FeatureName(0u));
    if (!HasFeature(requiredFeature)) {
        std::stringstream err;
        err << "" << out << " requires feature '" << requiredFeature << "'";
        return Throw(Napi::TypeError::New(env, err.str()));
    }

    return true;
}

bool Converter::Convert(interop::GPUTextureFormat& out, wgpu::TextureFormat in) {
#define CASE(WGPU, INTEROP)                       \
    case wgpu::TextureFormat::WGPU:               \
        out = interop::GPUTextureFormat::INTEROP; \
        return true

    switch (in) {
        CASE(ASTC10x10Unorm, kAstc10X10Unorm);
        CASE(ASTC10x10UnormSrgb, kAstc10X10UnormSrgb);
        CASE(ASTC10x5Unorm, kAstc10X5Unorm);
        CASE(ASTC10x5UnormSrgb, kAstc10X5UnormSrgb);
        CASE(ASTC10x6Unorm, kAstc10X6Unorm);
        CASE(ASTC10x6UnormSrgb, kAstc10X6UnormSrgb);
        CASE(ASTC10x8Unorm, kAstc10X8Unorm);
        CASE(ASTC10x8UnormSrgb, kAstc10X8UnormSrgb);
        CASE(ASTC12x10Unorm, kAstc12X10Unorm);
        CASE(ASTC12x10UnormSrgb, kAstc12X10UnormSrgb);
        CASE(ASTC12x12Unorm, kAstc12X12Unorm);
        CASE(ASTC12x12UnormSrgb, kAstc12X12UnormSrgb);
        CASE(ASTC4x4Unorm, kAstc4X4Unorm);
        CASE(ASTC4x4UnormSrgb, kAstc4X4UnormSrgb);
        CASE(ASTC5x4Unorm, kAstc5X4Unorm);
        CASE(ASTC5x4UnormSrgb, kAstc5X4UnormSrgb);
        CASE(ASTC5x5Unorm, kAstc5X5Unorm);
        CASE(ASTC5x5UnormSrgb, kAstc5X5UnormSrgb);
        CASE(ASTC6x5Unorm, kAstc6X5Unorm);
        CASE(ASTC6x5UnormSrgb, kAstc6X5UnormSrgb);
        CASE(ASTC6x6Unorm, kAstc6X6Unorm);
        CASE(ASTC6x6UnormSrgb, kAstc6X6UnormSrgb);
        CASE(ASTC8x5Unorm, kAstc8X5Unorm);
        CASE(ASTC8x5UnormSrgb, kAstc8X5UnormSrgb);
        CASE(ASTC8x6Unorm, kAstc8X6Unorm);
        CASE(ASTC8x6UnormSrgb, kAstc8X6UnormSrgb);
        CASE(ASTC8x8Unorm, kAstc8X8Unorm);
        CASE(ASTC8x8UnormSrgb, kAstc8X8UnormSrgb);
        CASE(BC1RGBAUnorm, kBc1RgbaUnorm);
        CASE(BC1RGBAUnormSrgb, kBc1RgbaUnormSrgb);
        CASE(BC2RGBAUnorm, kBc2RgbaUnorm);
        CASE(BC2RGBAUnormSrgb, kBc2RgbaUnormSrgb);
        CASE(BC3RGBAUnorm, kBc3RgbaUnorm);
        CASE(BC3RGBAUnormSrgb, kBc3RgbaUnormSrgb);
        CASE(BC4RSnorm, kBc4RSnorm);
        CASE(BC4RUnorm, kBc4RUnorm);
        CASE(BC5RGSnorm, kBc5RgSnorm);
        CASE(BC5RGUnorm, kBc5RgUnorm);
        CASE(BC6HRGBFloat, kBc6HRgbFloat);
        CASE(BC6HRGBUfloat, kBc6HRgbUfloat);
        CASE(BC7RGBAUnorm, kBc7RgbaUnorm);
        CASE(BC7RGBAUnormSrgb, kBc7RgbaUnormSrgb);
        CASE(BGRA8Unorm, kBgra8Unorm);
        CASE(BGRA8UnormSrgb, kBgra8UnormSrgb);
        CASE(Depth16Unorm, kDepth16Unorm);
        CASE(Depth24Plus, kDepth24Plus);
        CASE(Depth24PlusStencil8, kDepth24PlusStencil8);
        CASE(Depth32Float, kDepth32Float);
        CASE(Depth32FloatStencil8, kDepth32FloatStencil8);
        CASE(EACR11Snorm, kEacR11Snorm);
        CASE(EACR11Unorm, kEacR11Unorm);
        CASE(EACRG11Snorm, kEacRg11Snorm);
        CASE(EACRG11Unorm, kEacRg11Unorm);
        CASE(ETC2RGB8A1Unorm, kEtc2Rgb8A1Unorm);
        CASE(ETC2RGB8A1UnormSrgb, kEtc2Rgb8A1UnormSrgb);
        CASE(ETC2RGB8Unorm, kEtc2Rgb8Unorm);
        CASE(ETC2RGB8UnormSrgb, kEtc2Rgb8UnormSrgb);
        CASE(ETC2RGBA8Unorm, kEtc2Rgba8Unorm);
        CASE(ETC2RGBA8UnormSrgb, kEtc2Rgba8UnormSrgb);
        CASE(R16Float, kR16Float);
        CASE(R16Sint, kR16Sint);
        CASE(R16Uint, kR16Uint);
        CASE(R32Float, kR32Float);
        CASE(R32Sint, kR32Sint);
        CASE(R32Uint, kR32Uint);
        CASE(R8Sint, kR8Sint);
        CASE(R8Snorm, kR8Snorm);
        CASE(R8Uint, kR8Uint);
        CASE(R8Unorm, kR8Unorm);
        CASE(RG11B10Ufloat, kRg11B10Ufloat);
        CASE(RG16Float, kRg16Float);
        CASE(RG16Sint, kRg16Sint);
        CASE(RG16Uint, kRg16Uint);
        CASE(RG32Float, kRg32Float);
        CASE(RG32Sint, kRg32Sint);
        CASE(RG32Uint, kRg32Uint);
        CASE(RG8Sint, kRg8Sint);
        CASE(RG8Snorm, kRg8Snorm);
        CASE(RG8Uint, kRg8Uint);
        CASE(RG8Unorm, kRg8Unorm);
        CASE(RGB10A2Uint, kRgb10A2Uint);
        CASE(RGB10A2Unorm, kRgb10A2Unorm);
        CASE(RGB9E5Ufloat, kRgb9E5Ufloat);
        CASE(RGBA16Float, kRgba16Float);
        CASE(RGBA16Sint, kRgba16Sint);
        CASE(RGBA16Uint, kRgba16Uint);
        CASE(RGBA32Float, kRgba32Float);
        CASE(RGBA32Sint, kRgba32Sint);
        CASE(RGBA32Uint, kRgba32Uint);
        CASE(RGBA8Sint, kRgba8Sint);
        CASE(RGBA8Snorm, kRgba8Snorm);
        CASE(RGBA8Uint, kRgba8Uint);
        CASE(RGBA8Unorm, kRgba8Unorm);
        CASE(RGBA8UnormSrgb, kRgba8UnormSrgb);
        CASE(Stencil8, kStencil8);
#undef CASE

        case wgpu::TextureFormat::R16Snorm:
        case wgpu::TextureFormat::R16Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar422Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar444Unorm:
        case wgpu::TextureFormat::R8BG8A8Triplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm:
        case wgpu::TextureFormat::RG16Snorm:
        case wgpu::TextureFormat::RG16Unorm:
        case wgpu::TextureFormat::RGBA16Snorm:
        case wgpu::TextureFormat::RGBA16Unorm:
        case wgpu::TextureFormat::External:

        case wgpu::TextureFormat::Undefined:
            return false;
    }

    return false;
}

bool Converter::Convert(wgpu::TextureUsage& out, const interop::GPUTextureUsageFlags& in) {
    out = static_cast<wgpu::TextureUsage>(in.value);
    return true;
}

bool Converter::Convert(interop::GPUTextureUsageFlags& out, wgpu::TextureUsage in) {
    out = interop::GPUTextureUsageFlags(static_cast<uint32_t>(in));
    return true;
}

bool Converter::Convert(wgpu::ColorWriteMask& out, const interop::GPUColorWriteFlags& in) {
    out = static_cast<wgpu::ColorWriteMask>(in.value);
    return true;
}

bool Converter::Convert(wgpu::BufferUsage& out, const interop::GPUBufferUsageFlags& in) {
    out = static_cast<wgpu::BufferUsage>(in.value);
    return true;
}

bool Converter::Convert(interop::GPUBufferUsageFlags& out, wgpu::BufferUsage in) {
    out = interop::GPUBufferUsageFlags(static_cast<uint32_t>(in));
    return true;
}

bool Converter::Convert(wgpu::MapMode& out, const interop::GPUMapModeFlags& in) {
    out = static_cast<wgpu::MapMode>(in.value);
    return true;
}

bool Converter::Convert(wgpu::ShaderStage& out, const interop::GPUShaderStageFlags& in) {
    out = static_cast<wgpu::ShaderStage>(in.value);
    return true;
}

bool Converter::Convert(wgpu::TextureDimension& out, const interop::GPUTextureDimension& in) {
    out = wgpu::TextureDimension::e1D;
    switch (in) {
        case interop::GPUTextureDimension::k1D:
            out = wgpu::TextureDimension::e1D;
            return true;
        case interop::GPUTextureDimension::k2D:
            out = wgpu::TextureDimension::e2D;
            return true;
        case interop::GPUTextureDimension::k3D:
            out = wgpu::TextureDimension::e3D;
            return true;
    }
    return Throw("invalid value for GPUTextureDimension");
}

bool Converter::Convert(interop::GPUTextureDimension& out, wgpu::TextureDimension in) {
    switch (in) {
        case wgpu::TextureDimension::e1D:
            out = interop::GPUTextureDimension::k1D;
            return true;
        case wgpu::TextureDimension::e2D:
            out = interop::GPUTextureDimension::k2D;
            return true;
        case wgpu::TextureDimension::e3D:
            out = interop::GPUTextureDimension::k3D;
            return true;
        case wgpu::TextureDimension::Undefined:
            break;
    }
    return false;
}

bool Converter::Convert(wgpu::TextureViewDimension& out,
                        const interop::GPUTextureViewDimension& in) {
    out = wgpu::TextureViewDimension::Undefined;
    switch (in) {
        case interop::GPUTextureViewDimension::k1D:
            out = wgpu::TextureViewDimension::e1D;
            return true;
        case interop::GPUTextureViewDimension::k2D:
            out = wgpu::TextureViewDimension::e2D;
            return true;
        case interop::GPUTextureViewDimension::k2DArray:
            out = wgpu::TextureViewDimension::e2DArray;
            return true;
        case interop::GPUTextureViewDimension::kCube:
            out = wgpu::TextureViewDimension::Cube;
            return true;
        case interop::GPUTextureViewDimension::kCubeArray:
            out = wgpu::TextureViewDimension::CubeArray;
            return true;
        case interop::GPUTextureViewDimension::k3D:
            out = wgpu::TextureViewDimension::e3D;
            return true;
        default:
            break;
    }
    return Throw("invalid value for GPUTextureViewDimension");
}

bool Converter::Convert(wgpu::ComputeState& out, const interop::GPUProgrammableStage& in) {
    out = {};
    out.module = *in.module.As<GPUShaderModule>();

    // Replace nulls in the entryPoint name with another character that's disallowed in WGSL
    // identifiers. This is so that using "main\0" doesn't match an entryPoint named "main".
    out.entryPoint = in.entryPoint ? ConvertStringReplacingNull(in.entryPoint.value()) : nullptr;

    return Convert(out.constants, out.constantCount, in.constants);
}

bool Converter::Convert(wgpu::ConstantEntry& out,
                        const std::string& in_name,
                        wgpu::interop::GPUPipelineConstantValue in_value) {
    // Replace nulls in the key with another character that's disallowed in WGSL identifiers.
    // This is so that using "c\0" doesn't match a constant named "c".
    out.key = ConvertStringReplacingNull(in_name);

    return Convert(out.value, in_value);
}

bool Converter::Convert(wgpu::BlendComponent& out, const interop::GPUBlendComponent& in) {
    out = {};
    return Convert(out.operation, in.operation) && Convert(out.dstFactor, in.dstFactor) &&
           Convert(out.srcFactor, in.srcFactor);
}

bool Converter::Convert(wgpu::BlendFactor& out, const interop::GPUBlendFactor& in) {
    out = wgpu::BlendFactor::Zero;
    wgpu::FeatureName requiredFeature = wgpu::FeatureName(0u);
    switch (in) {
        case interop::GPUBlendFactor::kZero:
            out = wgpu::BlendFactor::Zero;
            return true;
        case interop::GPUBlendFactor::kOne:
            out = wgpu::BlendFactor::One;
            return true;
        case interop::GPUBlendFactor::kSrc:
            out = wgpu::BlendFactor::Src;
            return true;
        case interop::GPUBlendFactor::kOneMinusSrc:
            out = wgpu::BlendFactor::OneMinusSrc;
            return true;
        case interop::GPUBlendFactor::kSrcAlpha:
            out = wgpu::BlendFactor::SrcAlpha;
            return true;
        case interop::GPUBlendFactor::kOneMinusSrcAlpha:
            out = wgpu::BlendFactor::OneMinusSrcAlpha;
            return true;
        case interop::GPUBlendFactor::kDst:
            out = wgpu::BlendFactor::Dst;
            return true;
        case interop::GPUBlendFactor::kOneMinusDst:
            out = wgpu::BlendFactor::OneMinusDst;
            return true;
        case interop::GPUBlendFactor::kDstAlpha:
            out = wgpu::BlendFactor::DstAlpha;
            return true;
        case interop::GPUBlendFactor::kOneMinusDstAlpha:
            out = wgpu::BlendFactor::OneMinusDstAlpha;
            return true;
        case interop::GPUBlendFactor::kSrcAlphaSaturated:
            out = wgpu::BlendFactor::SrcAlphaSaturated;
            return true;
        case interop::GPUBlendFactor::kConstant:
            out = wgpu::BlendFactor::Constant;
            return true;
        case interop::GPUBlendFactor::kOneMinusConstant:
            out = wgpu::BlendFactor::OneMinusConstant;
            return true;
        case interop::GPUBlendFactor::kSrc1:
            out = wgpu::BlendFactor::Src1;
            requiredFeature = wgpu::FeatureName::DualSourceBlending;
            return true;
        case interop::GPUBlendFactor::kOneMinusSrc1:
            out = wgpu::BlendFactor::OneMinusSrc1;
            requiredFeature = wgpu::FeatureName::DualSourceBlending;
            return true;
        case interop::GPUBlendFactor::kSrc1Alpha:
            out = wgpu::BlendFactor::Src1Alpha;
            requiredFeature = wgpu::FeatureName::DualSourceBlending;
            return true;
        case interop::GPUBlendFactor::kOneMinusSrc1Alpha:
            out = wgpu::BlendFactor::OneMinusSrc1Alpha;
            requiredFeature = wgpu::FeatureName::DualSourceBlending;
            break;

        default:
            std::stringstream err;
            err << "unknown GPUBlendFactor(" << static_cast<int>(in) << ")";
            return Throw(err.str());
    }

    assert(requiredFeature != wgpu::FeatureName(0u));
    if (!HasFeature(requiredFeature)) {
        std::stringstream err;
        err << "" << out << " requires feature '" << requiredFeature << "'";
        return Throw(Napi::TypeError::New(env, err.str()));
    }

    return true;
}

bool Converter::Convert(wgpu::BlendOperation& out, const interop::GPUBlendOperation& in) {
    out = wgpu::BlendOperation::Add;
    switch (in) {
        case interop::GPUBlendOperation::kAdd:
            out = wgpu::BlendOperation::Add;
            return true;
        case interop::GPUBlendOperation::kSubtract:
            out = wgpu::BlendOperation::Subtract;
            return true;
        case interop::GPUBlendOperation::kReverseSubtract:
            out = wgpu::BlendOperation::ReverseSubtract;
            return true;
        case interop::GPUBlendOperation::kMin:
            out = wgpu::BlendOperation::Min;
            return true;
        case interop::GPUBlendOperation::kMax:
            out = wgpu::BlendOperation::Max;
            return true;
        default:
            break;
    }
    return Throw("invalid value for GPUBlendOperation");
}

bool Converter::Convert(wgpu::BlendState& out, const interop::GPUBlendState& in) {
    out = {};
    return Convert(out.alpha, in.alpha) && Convert(out.color, in.color);
}

bool Converter::Convert(wgpu::PrimitiveState& out, const interop::GPUPrimitiveState& in) {
    out = {};

    return Convert(out.topology, in.topology) &&
           Convert(out.stripIndexFormat, in.stripIndexFormat) &&
           Convert(out.frontFace, in.frontFace) && Convert(out.cullMode, in.cullMode) &&
           Convert(out.unclippedDepth, in.unclippedDepth);
}

bool Converter::Convert(wgpu::ColorTargetState& out, const interop::GPUColorTargetState& in) {
    out = {};
    return Convert(out.format, in.format) && Convert(out.blend, in.blend) &&
           Convert(out.writeMask, in.writeMask);
}

bool Converter::Convert(wgpu::DepthStencilState& out, const interop::GPUDepthStencilState& in) {
    out = {};

    return Convert(out.format, in.format) && Convert(out.depthWriteEnabled, in.depthWriteEnabled) &&
           Convert(out.depthCompare, in.depthCompare) &&
           Convert(out.stencilFront, in.stencilFront) && Convert(out.stencilBack, in.stencilBack) &&
           Convert(out.stencilReadMask, in.stencilReadMask) &&
           Convert(out.stencilWriteMask, in.stencilWriteMask) &&
           Convert(out.depthBias, in.depthBias) &&
           Convert(out.depthBiasSlopeScale, in.depthBiasSlopeScale) &&
           Convert(out.depthBiasClamp, in.depthBiasClamp);
}

bool Converter::Convert(wgpu::MultisampleState& out, const interop::GPUMultisampleState& in) {
    out = {};
    return Convert(out.count, in.count) && Convert(out.mask, in.mask) &&
           Convert(out.alphaToCoverageEnabled, in.alphaToCoverageEnabled);
}

bool Converter::Convert(wgpu::FragmentState& out, const interop::GPUFragmentState& in) {
    out = {};

    // Replace nulls in the entryPoint name with another character that's disallowed in WGSL
    // identifiers. This is so that using "main\0" doesn't match an entryPoint named "main".
    out.entryPoint = in.entryPoint ? ConvertStringReplacingNull(in.entryPoint.value()) : nullptr;

    return Convert(out.targets, out.targetCount, in.targets) &&  //
           Convert(out.module, in.module) &&                     //
           Convert(out.constants, out.constantCount, in.constants);
}

bool Converter::Convert(wgpu::PrimitiveTopology& out, const interop::GPUPrimitiveTopology& in) {
    out = wgpu::PrimitiveTopology::LineList;
    switch (in) {
        case interop::GPUPrimitiveTopology::kPointList:
            out = wgpu::PrimitiveTopology::PointList;
            return true;
        case interop::GPUPrimitiveTopology::kLineList:
            out = wgpu::PrimitiveTopology::LineList;
            return true;
        case interop::GPUPrimitiveTopology::kLineStrip:
            out = wgpu::PrimitiveTopology::LineStrip;
            return true;
        case interop::GPUPrimitiveTopology::kTriangleList:
            out = wgpu::PrimitiveTopology::TriangleList;
            return true;
        case interop::GPUPrimitiveTopology::kTriangleStrip:
            out = wgpu::PrimitiveTopology::TriangleStrip;
            return true;
    }
    return Throw("invalid value for GPUPrimitiveTopology");
}

bool Converter::Convert(wgpu::FrontFace& out, const interop::GPUFrontFace& in) {
    out = wgpu::FrontFace::CW;
    switch (in) {
        case interop::GPUFrontFace::kCw:
            out = wgpu::FrontFace::CW;
            return true;
        case interop::GPUFrontFace::kCcw:
            out = wgpu::FrontFace::CCW;
            return true;
    }
    return Throw("invalid value for GPUFrontFace");
}

bool Converter::Convert(wgpu::CullMode& out, const interop::GPUCullMode& in) {
    out = wgpu::CullMode::None;
    switch (in) {
        case interop::GPUCullMode::kNone:
            out = wgpu::CullMode::None;
            return true;
        case interop::GPUCullMode::kFront:
            out = wgpu::CullMode::Front;
            return true;
        case interop::GPUCullMode::kBack:
            out = wgpu::CullMode::Back;
            return true;
    }
    return Throw("invalid value for GPUCullMode");
}

bool Converter::Convert(wgpu::CompareFunction& out, const interop::GPUCompareFunction& in) {
    out = wgpu::CompareFunction::Undefined;
    switch (in) {
        case interop::GPUCompareFunction::kNever:
            out = wgpu::CompareFunction::Never;
            return true;
        case interop::GPUCompareFunction::kLess:
            out = wgpu::CompareFunction::Less;
            return true;
        case interop::GPUCompareFunction::kLessEqual:
            out = wgpu::CompareFunction::LessEqual;
            return true;
        case interop::GPUCompareFunction::kGreater:
            out = wgpu::CompareFunction::Greater;
            return true;
        case interop::GPUCompareFunction::kGreaterEqual:
            out = wgpu::CompareFunction::GreaterEqual;
            return true;
        case interop::GPUCompareFunction::kEqual:
            out = wgpu::CompareFunction::Equal;
            return true;
        case interop::GPUCompareFunction::kNotEqual:
            out = wgpu::CompareFunction::NotEqual;
            return true;
        case interop::GPUCompareFunction::kAlways:
            out = wgpu::CompareFunction::Always;
            return true;
    }
    return Throw("invalid value for GPUCompareFunction");
}

bool Converter::Convert(wgpu::IndexFormat& out, const interop::GPUIndexFormat& in) {
    out = wgpu::IndexFormat::Undefined;
    switch (in) {
        case interop::GPUIndexFormat::kUint16:
            out = wgpu::IndexFormat::Uint16;
            return true;
        case interop::GPUIndexFormat::kUint32:
            out = wgpu::IndexFormat::Uint32;
            return true;
    }
    return Throw("invalid value for GPUIndexFormat");
}

bool Converter::Convert(wgpu::StencilOperation& out, const interop::GPUStencilOperation& in) {
    out = wgpu::StencilOperation::Zero;
    switch (in) {
        case interop::GPUStencilOperation::kKeep:
            out = wgpu::StencilOperation::Keep;
            return true;
        case interop::GPUStencilOperation::kZero:
            out = wgpu::StencilOperation::Zero;
            return true;
        case interop::GPUStencilOperation::kReplace:
            out = wgpu::StencilOperation::Replace;
            return true;
        case interop::GPUStencilOperation::kInvert:
            out = wgpu::StencilOperation::Invert;
            return true;
        case interop::GPUStencilOperation::kIncrementClamp:
            out = wgpu::StencilOperation::IncrementClamp;
            return true;
        case interop::GPUStencilOperation::kDecrementClamp:
            out = wgpu::StencilOperation::DecrementClamp;
            return true;
        case interop::GPUStencilOperation::kIncrementWrap:
            out = wgpu::StencilOperation::IncrementWrap;
            return true;
        case interop::GPUStencilOperation::kDecrementWrap:
            out = wgpu::StencilOperation::DecrementWrap;
            return true;
    }
    return Throw("invalid value for GPUStencilOperation");
}

bool Converter::Convert(wgpu::StencilFaceState& out, const interop::GPUStencilFaceState& in) {
    return Convert(out.compare, in.compare) && Convert(out.failOp, in.failOp) &&
           Convert(out.depthFailOp, in.depthFailOp) && Convert(out.passOp, in.passOp);
}

bool Converter::Convert(wgpu::VertexBufferLayout& out, const interop::GPUVertexBufferLayout& in) {
    out = {};
    return Convert(out.attributes, out.attributeCount, in.attributes) &&
           Convert(out.arrayStride, in.arrayStride) && Convert(out.stepMode, in.stepMode);
}

bool Converter::Convert(wgpu::VertexState& out, const interop::GPUVertexState& in) {
    out = {};

    // Replace nulls in the entryPoint name with another character that's disallowed in WGSL
    // identifiers. This is so that using "main\0" doesn't match an entryPoint named "main".
    out.entryPoint = in.entryPoint ? ConvertStringReplacingNull(in.entryPoint.value()) : nullptr;

    wgpu::VertexBufferLayout* outBuffers = nullptr;
    if (!Convert(out.module, in.module) ||                    //
        !Convert(outBuffers, out.bufferCount, in.buffers) ||  //
        !Convert(out.constants, out.constantCount, in.constants)) {
        return false;
    }

    // Patch up the unused vertex buffer layouts to use wgpu::VertexStepMode::Undefined.
    // The converter for optional value will have put the default value of wgpu::VertexBufferLayout
    // that has wgpu::VertexStepMode::Vertex.
    out.buffers = outBuffers;
    for (size_t i = 0; i < in.buffers.size(); i++) {
        if (!in.buffers[i].has_value()) {
            outBuffers[i].stepMode = wgpu::VertexStepMode::Undefined;
        }
    }

    return true;
}

bool Converter::Convert(wgpu::VertexStepMode& out, const interop::GPUVertexStepMode& in) {
    out = wgpu::VertexStepMode::Instance;
    switch (in) {
        case interop::GPUVertexStepMode::kInstance:
            out = wgpu::VertexStepMode::Instance;
            return true;
        case interop::GPUVertexStepMode::kVertex:
            out = wgpu::VertexStepMode::Vertex;
            return true;
        default:
            break;
    }
    return Throw("invalid value for GPUVertexStepMode");
}

bool Converter::Convert(wgpu::VertexAttribute& out, const interop::GPUVertexAttribute& in) {
    return Convert(out.format, in.format) && Convert(out.offset, in.offset) &&
           Convert(out.shaderLocation, in.shaderLocation);
}

bool Converter::Convert(wgpu::VertexFormat& out, const interop::GPUVertexFormat& in) {
    switch (in) {
        case interop::GPUVertexFormat::kUint8:
            out = wgpu::VertexFormat::Uint8;
            return true;
        case interop::GPUVertexFormat::kUint8X2:
            out = wgpu::VertexFormat::Uint8x2;
            return true;
        case interop::GPUVertexFormat::kUint8X4:
            out = wgpu::VertexFormat::Uint8x4;
            return true;
        case interop::GPUVertexFormat::kSint8:
            out = wgpu::VertexFormat::Sint8;
            return true;
        case interop::GPUVertexFormat::kSint8X2:
            out = wgpu::VertexFormat::Sint8x2;
            return true;
        case interop::GPUVertexFormat::kSint8X4:
            out = wgpu::VertexFormat::Sint8x4;
            return true;
        case interop::GPUVertexFormat::kUnorm8:
            out = wgpu::VertexFormat::Unorm8;
            return true;
        case interop::GPUVertexFormat::kUnorm8X2:
            out = wgpu::VertexFormat::Unorm8x2;
            return true;
        case interop::GPUVertexFormat::kUnorm8X4:
            out = wgpu::VertexFormat::Unorm8x4;
            return true;
        case interop::GPUVertexFormat::kSnorm8:
            out = wgpu::VertexFormat::Snorm8;
            return true;
        case interop::GPUVertexFormat::kSnorm8X2:
            out = wgpu::VertexFormat::Snorm8x2;
            return true;
        case interop::GPUVertexFormat::kSnorm8X4:
            out = wgpu::VertexFormat::Snorm8x4;
            return true;
        case interop::GPUVertexFormat::kUint16:
            out = wgpu::VertexFormat::Uint16;
            return true;
        case interop::GPUVertexFormat::kUint16X2:
            out = wgpu::VertexFormat::Uint16x2;
            return true;
        case interop::GPUVertexFormat::kUint16X4:
            out = wgpu::VertexFormat::Uint16x4;
            return true;
        case interop::GPUVertexFormat::kSint16:
            out = wgpu::VertexFormat::Sint16;
            return true;
        case interop::GPUVertexFormat::kSint16X2:
            out = wgpu::VertexFormat::Sint16x2;
            return true;
        case interop::GPUVertexFormat::kSint16X4:
            out = wgpu::VertexFormat::Sint16x4;
            return true;
        case interop::GPUVertexFormat::kUnorm16:
            out = wgpu::VertexFormat::Unorm16;
            return true;
        case interop::GPUVertexFormat::kUnorm16X2:
            out = wgpu::VertexFormat::Unorm16x2;
            return true;
        case interop::GPUVertexFormat::kUnorm16X4:
            out = wgpu::VertexFormat::Unorm16x4;
            return true;
        case interop::GPUVertexFormat::kSnorm16:
            out = wgpu::VertexFormat::Snorm16;
            return true;
        case interop::GPUVertexFormat::kSnorm16X2:
            out = wgpu::VertexFormat::Snorm16x2;
            return true;
        case interop::GPUVertexFormat::kSnorm16X4:
            out = wgpu::VertexFormat::Snorm16x4;
            return true;
        case interop::GPUVertexFormat::kFloat16:
            out = wgpu::VertexFormat::Float16;
            return true;
        case interop::GPUVertexFormat::kFloat16X2:
            out = wgpu::VertexFormat::Float16x2;
            return true;
        case interop::GPUVertexFormat::kFloat16X4:
            out = wgpu::VertexFormat::Float16x4;
            return true;
        case interop::GPUVertexFormat::kFloat32:
            out = wgpu::VertexFormat::Float32;
            return true;
        case interop::GPUVertexFormat::kFloat32X2:
            out = wgpu::VertexFormat::Float32x2;
            return true;
        case interop::GPUVertexFormat::kFloat32X3:
            out = wgpu::VertexFormat::Float32x3;
            return true;
        case interop::GPUVertexFormat::kFloat32X4:
            out = wgpu::VertexFormat::Float32x4;
            return true;
        case interop::GPUVertexFormat::kUint32:
            out = wgpu::VertexFormat::Uint32;
            return true;
        case interop::GPUVertexFormat::kUint32X2:
            out = wgpu::VertexFormat::Uint32x2;
            return true;
        case interop::GPUVertexFormat::kUint32X3:
            out = wgpu::VertexFormat::Uint32x3;
            return true;
        case interop::GPUVertexFormat::kUint32X4:
            out = wgpu::VertexFormat::Uint32x4;
            return true;
        case interop::GPUVertexFormat::kSint32:
            out = wgpu::VertexFormat::Sint32;
            return true;
        case interop::GPUVertexFormat::kSint32X2:
            out = wgpu::VertexFormat::Sint32x2;
            return true;
        case interop::GPUVertexFormat::kSint32X3:
            out = wgpu::VertexFormat::Sint32x3;
            return true;
        case interop::GPUVertexFormat::kSint32X4:
            out = wgpu::VertexFormat::Sint32x4;
            return true;
        case interop::GPUVertexFormat::kUnorm1010102:
            out = wgpu::VertexFormat::Unorm10_10_10_2;
            return true;
        case interop::GPUVertexFormat::kUnorm8X4Bgra:
            out = wgpu::VertexFormat::Unorm8x4BGRA;
            return true;
        default:
            break;
    }
    return Throw("invalid value for GPUVertexFormat");
}

bool Converter::Convert(wgpu::RenderPassColorAttachment& out,
                        const interop::GPURenderPassColorAttachment& in) {
    out = {};
    return Convert(out.view, in.view) &&                    //
           Convert(out.depthSlice, in.depthSlice) &&        //
           Convert(out.resolveTarget, in.resolveTarget) &&  //
           Convert(out.clearValue, in.clearValue) &&        //
           Convert(out.loadOp, in.loadOp) &&                //
           Convert(out.storeOp, in.storeOp);
}

bool Converter::Convert(wgpu::RenderPassDepthStencilAttachment& out,
                        const interop::GPURenderPassDepthStencilAttachment& in) {
    out = {};
    return Convert(out.view, in.view) &&                            //
           Convert(out.depthClearValue, in.depthClearValue) &&      //
           Convert(out.depthLoadOp, in.depthLoadOp) &&              //
           Convert(out.depthStoreOp, in.depthStoreOp) &&            //
           Convert(out.depthReadOnly, in.depthReadOnly) &&          //
           Convert(out.stencilClearValue, in.stencilClearValue) &&  //
           Convert(out.stencilLoadOp, in.stencilLoadOp) &&          //
           Convert(out.stencilStoreOp, in.stencilStoreOp) &&        //
           Convert(out.stencilReadOnly, in.stencilReadOnly);
}

bool Converter::Convert(wgpu::PassTimestampWrites& out,
                        const interop::GPURenderPassTimestampWrites& in) {
    return Convert(out.querySet, in.querySet) &&                                    //
           Convert(out.beginningOfPassWriteIndex, in.beginningOfPassWriteIndex) &&  //
           Convert(out.endOfPassWriteIndex, in.endOfPassWriteIndex);
}

bool Converter::Convert(wgpu::PassTimestampWrites& out,
                        const interop::GPUComputePassTimestampWrites& in) {
    return Convert(out.querySet, in.querySet) &&                                    //
           Convert(out.beginningOfPassWriteIndex, in.beginningOfPassWriteIndex) &&  //
           Convert(out.endOfPassWriteIndex, in.endOfPassWriteIndex);
}

bool Converter::Convert(wgpu::LoadOp& out, const interop::GPULoadOp& in) {
    out = wgpu::LoadOp::Clear;
    switch (in) {
        case interop::GPULoadOp::kLoad:
            out = wgpu::LoadOp::Load;
            return true;
        case interop::GPULoadOp::kClear:
            out = wgpu::LoadOp::Clear;
            return true;
    }
    return Throw("invalid value for GPULoadOp");
}

bool Converter::Convert(wgpu::StoreOp& out, const interop::GPUStoreOp& in) {
    out = wgpu::StoreOp::Store;
    switch (in) {
        case interop::GPUStoreOp::kStore:
            out = wgpu::StoreOp::Store;
            return true;
        case interop::GPUStoreOp::kDiscard:
            out = wgpu::StoreOp::Discard;
            return true;
    }
    return Throw("invalid value for GPUStoreOp");
}

bool Converter::Convert(wgpu::BindGroupEntry& out, const interop::GPUBindGroupEntry& in) {
    out = {};
    if (!Convert(out.binding, in.binding)) {
        return false;
    }

    if (auto* res = std::get_if<interop::Interface<interop::GPUSampler>>(&in.resource)) {
        return Convert(out.sampler, *res);
    }
    if (auto* res = std::get_if<interop::Interface<interop::GPUTextureView>>(&in.resource)) {
        return Convert(out.textureView, *res);
    }
    if (auto* res = std::get_if<interop::GPUBufferBinding>(&in.resource)) {
        auto buffer = res->buffer.As<GPUBuffer>();
        out.size = wgpu::kWholeSize;
        if (!buffer || !Convert(out.offset, res->offset) || !Convert(out.size, res->size)) {
            return false;
        }
        out.buffer = *buffer;
        return true;
    }
    if (auto* res = std::get_if<interop::Interface<interop::GPUExternalTexture>>(&in.resource)) {
        // TODO(crbug.com/dawn/1129): External textures
        UNIMPLEMENTED(env, {});
    }
    return Throw("invalid value for GPUBindGroupEntry.resource");
}

bool Converter::Convert(wgpu::BindGroupLayoutEntry& out,
                        const interop::GPUBindGroupLayoutEntry& in) {
    // TODO(crbug.com/dawn/1129): External textures
    return Convert(out.binding, in.binding) && Convert(out.visibility, in.visibility) &&
           Convert(out.buffer, in.buffer) && Convert(out.sampler, in.sampler) &&
           Convert(out.texture, in.texture) && Convert(out.storageTexture, in.storageTexture);
}

bool Converter::Convert(wgpu::BufferBindingLayout& out, const interop::GPUBufferBindingLayout& in) {
    return Convert(out.type, in.type) && Convert(out.hasDynamicOffset, in.hasDynamicOffset) &&
           Convert(out.minBindingSize, in.minBindingSize);
}

bool Converter::Convert(wgpu::SamplerBindingLayout& out,
                        const interop::GPUSamplerBindingLayout& in) {
    return Convert(out.type, in.type);
}

bool Converter::Convert(wgpu::TextureBindingLayout& out,
                        const interop::GPUTextureBindingLayout& in) {
    return Convert(out.sampleType, in.sampleType) && Convert(out.viewDimension, in.viewDimension) &&
           Convert(out.multisampled, in.multisampled);
}

bool Converter::Convert(wgpu::StorageTextureBindingLayout& out,
                        const interop::GPUStorageTextureBindingLayout& in) {
    return Convert(out.access, in.access) && Convert(out.format, in.format) &&
           Convert(out.viewDimension, in.viewDimension);
}

bool Converter::Convert(wgpu::BufferBindingType& out, const interop::GPUBufferBindingType& in) {
    out = wgpu::BufferBindingType::BindingNotUsed;
    switch (in) {
        case interop::GPUBufferBindingType::kUniform:
            out = wgpu::BufferBindingType::Uniform;
            return true;
        case interop::GPUBufferBindingType::kStorage:
            out = wgpu::BufferBindingType::Storage;
            return true;
        case interop::GPUBufferBindingType::kReadOnlyStorage:
            out = wgpu::BufferBindingType::ReadOnlyStorage;
            return true;
    }
    return Throw("invalid value for GPUBufferBindingType");
}

bool Converter::Convert(wgpu::TextureSampleType& out, const interop::GPUTextureSampleType& in) {
    out = wgpu::TextureSampleType::BindingNotUsed;
    switch (in) {
        case interop::GPUTextureSampleType::kFloat:
            out = wgpu::TextureSampleType::Float;
            return true;
        case interop::GPUTextureSampleType::kUnfilterableFloat:
            out = wgpu::TextureSampleType::UnfilterableFloat;
            return true;
        case interop::GPUTextureSampleType::kDepth:
            out = wgpu::TextureSampleType::Depth;
            return true;
        case interop::GPUTextureSampleType::kSint:
            out = wgpu::TextureSampleType::Sint;
            return true;
        case interop::GPUTextureSampleType::kUint:
            out = wgpu::TextureSampleType::Uint;
            return true;
    }
    return Throw("invalid value for GPUTextureSampleType");
}

bool Converter::Convert(wgpu::SamplerBindingType& out, const interop::GPUSamplerBindingType& in) {
    out = wgpu::SamplerBindingType::BindingNotUsed;
    switch (in) {
        case interop::GPUSamplerBindingType::kFiltering:
            out = wgpu::SamplerBindingType::Filtering;
            return true;
        case interop::GPUSamplerBindingType::kNonFiltering:
            out = wgpu::SamplerBindingType::NonFiltering;
            return true;
        case interop::GPUSamplerBindingType::kComparison:
            out = wgpu::SamplerBindingType::Comparison;
            return true;
    }
    return Throw("invalid value for GPUSamplerBindingType");
}

bool Converter::Convert(wgpu::StorageTextureAccess& out,
                        const interop::GPUStorageTextureAccess& in) {
    out = wgpu::StorageTextureAccess::BindingNotUsed;
    switch (in) {
        case interop::GPUStorageTextureAccess::kWriteOnly:
            out = wgpu::StorageTextureAccess::WriteOnly;
            return true;
        case interop::GPUStorageTextureAccess::kReadOnly:
            out = wgpu::StorageTextureAccess::ReadOnly;
            return true;
        case interop::GPUStorageTextureAccess::kReadWrite:
            out = wgpu::StorageTextureAccess::ReadWrite;
            return true;
    }
    return Throw("invalid value for GPUStorageTextureAccess");
}

bool Converter::Convert(wgpu::QueryType& out, const interop::GPUQueryType& in) {
    out = wgpu::QueryType::Occlusion;
    switch (in) {
        case interop::GPUQueryType::kOcclusion:
            out = wgpu::QueryType::Occlusion;
            return true;
        case interop::GPUQueryType::kTimestamp:
            if (HasFeature(wgpu::FeatureName::TimestampQuery)) {
                out = wgpu::QueryType::Timestamp;
                return true;
            } else {
                return Throw(Napi::TypeError::New(env, "invalid value for GPUQueryType"));
            }
    }
    return Throw("invalid value for GPUQueryType");
}

bool Converter::Convert(wgpu::FeatureName& out, interop::GPUFeatureName in) {
    switch (in) {
        case interop::GPUFeatureName::kTextureCompressionBc:
            out = wgpu::FeatureName::TextureCompressionBC;
            return true;
        case interop::GPUFeatureName::kTextureCompressionEtc2:
            out = wgpu::FeatureName::TextureCompressionETC2;
            return true;
        case interop::GPUFeatureName::kTextureCompressionAstc:
            out = wgpu::FeatureName::TextureCompressionASTC;
            return true;
        case interop::GPUFeatureName::kTimestampQuery:
            out = wgpu::FeatureName::TimestampQuery;
            return true;
        case interop::GPUFeatureName::kDepth32FloatStencil8:
            out = wgpu::FeatureName::Depth32FloatStencil8;
            return true;
        case interop::GPUFeatureName::kDepthClipControl:
            out = wgpu::FeatureName::DepthClipControl;
            return true;
        case interop::GPUFeatureName::kIndirectFirstInstance:
            out = wgpu::FeatureName::IndirectFirstInstance;
            return true;
        case interop::GPUFeatureName::kShaderF16:
            out = wgpu::FeatureName::ShaderF16;
            return true;
        case interop::GPUFeatureName::kRg11B10UfloatRenderable:
            out = wgpu::FeatureName::RG11B10UfloatRenderable;
            return true;
        case interop::GPUFeatureName::kBgra8UnormStorage:
            out = wgpu::FeatureName::BGRA8UnormStorage;
            return true;
        case interop::GPUFeatureName::kFloat32Filterable:
            out = wgpu::FeatureName::Float32Filterable;
            return true;
        case interop::GPUFeatureName::kFloat32Blendable:
            out = wgpu::FeatureName::Float32Blendable;
            return true;
        case interop::GPUFeatureName::kSubgroups:
            out = wgpu::FeatureName::Subgroups;
            return true;
        case interop::GPUFeatureName::kMultiDrawIndirect:
            out = wgpu::FeatureName::MultiDrawIndirect;
            return true;
        case interop::GPUFeatureName::kDualSourceBlending:
            out = wgpu::FeatureName::DualSourceBlending;
            return true;
        case interop::GPUFeatureName::kChromiumExperimentalImmediateData:
            out = wgpu::FeatureName::ChromiumExperimentalImmediateData;
            return true;
        case interop::GPUFeatureName::kClipDistances:
            out = wgpu::FeatureName::ClipDistances;
            return true;
        case interop::GPUFeatureName::kTextureCompressionAstcSliced3D:
        case interop::GPUFeatureName::kTextureCompressionBcSliced3D:
            return false;
    }
    return false;
}

bool Converter::Convert(interop::GPUFeatureName& out, wgpu::FeatureName in) {
#define CASE(WGPU, INTEROP)                     \
    case wgpu::FeatureName::WGPU:               \
        out = interop::GPUFeatureName::INTEROP; \
        return true

    switch (in) {
        CASE(BGRA8UnormStorage, kBgra8UnormStorage);
        CASE(Depth32FloatStencil8, kDepth32FloatStencil8);
        CASE(DepthClipControl, kDepthClipControl);
        CASE(Float32Filterable, kFloat32Filterable);
        CASE(Float32Blendable, kFloat32Blendable);
        CASE(IndirectFirstInstance, kIndirectFirstInstance);
        CASE(RG11B10UfloatRenderable, kRg11B10UfloatRenderable);
        CASE(ShaderF16, kShaderF16);
        CASE(TextureCompressionASTC, kTextureCompressionAstc);
        CASE(TextureCompressionBC, kTextureCompressionBc);
        CASE(TextureCompressionETC2, kTextureCompressionEtc2);
        CASE(TimestampQuery, kTimestampQuery);
        CASE(Subgroups, kSubgroups);
        CASE(MultiDrawIndirect, kMultiDrawIndirect);
        CASE(DualSourceBlending, kDualSourceBlending);
        CASE(ClipDistances, kClipDistances);
        CASE(ChromiumExperimentalImmediateData, kChromiumExperimentalImmediateData);

#undef CASE

        case wgpu::FeatureName::AdapterPropertiesD3D:
        case wgpu::FeatureName::AdapterPropertiesMemoryHeaps:
        case wgpu::FeatureName::AdapterPropertiesVk:
        case wgpu::FeatureName::ANGLETextureSharing:
        case wgpu::FeatureName::BufferMapExtendedUsages:
        case wgpu::FeatureName::ChromiumExperimentalTimestampQueryInsidePasses:
        case wgpu::FeatureName::D3D11MultithreadProtected:
        case wgpu::FeatureName::DawnInternalUsages:
        case wgpu::FeatureName::DawnMultiPlanarFormats:
        case wgpu::FeatureName::DawnNative:
        case wgpu::FeatureName::DawnDrmFormatCapabilities:
        case wgpu::FeatureName::DawnFormatCapabilities:
        case wgpu::FeatureName::FramebufferFetch:
        case wgpu::FeatureName::HostMappedPointer:
        case wgpu::FeatureName::ImplicitDeviceSynchronization:
        case wgpu::FeatureName::MSAARenderToSingleSampled:
        case wgpu::FeatureName::MultiPlanarFormatExtendedUsages:
        case wgpu::FeatureName::MultiPlanarFormatP010:
        case wgpu::FeatureName::MultiPlanarFormatP210:
        case wgpu::FeatureName::MultiPlanarFormatP410:
        case wgpu::FeatureName::MultiPlanarFormatNv16:
        case wgpu::FeatureName::MultiPlanarFormatNv24:
        case wgpu::FeatureName::MultiPlanarFormatNv12a:
        case wgpu::FeatureName::MultiPlanarRenderTargets:
        case wgpu::FeatureName::Norm16TextureFormats:
        case wgpu::FeatureName::Unorm16TextureFormats:
        case wgpu::FeatureName::Snorm16TextureFormats:
        case wgpu::FeatureName::PixelLocalStorageCoherent:
        case wgpu::FeatureName::PixelLocalStorageNonCoherent:
        case wgpu::FeatureName::R8UnormStorage:
        case wgpu::FeatureName::ShaderModuleCompilationOptions:
        case wgpu::FeatureName::SharedBufferMemoryD3D12Resource:
        case wgpu::FeatureName::SharedFenceDXGISharedHandle:
        case wgpu::FeatureName::SharedFenceMTLSharedEvent:
        case wgpu::FeatureName::SharedFenceVkSemaphoreOpaqueFD:
        case wgpu::FeatureName::SharedFenceSyncFD:
        case wgpu::FeatureName::SharedFenceVkSemaphoreZirconHandle:
        case wgpu::FeatureName::SharedTextureMemoryAHardwareBuffer:
        case wgpu::FeatureName::SharedTextureMemoryD3D11Texture2D:
        case wgpu::FeatureName::SharedTextureMemoryDmaBuf:
        case wgpu::FeatureName::SharedTextureMemoryDXGISharedHandle:
        case wgpu::FeatureName::SharedTextureMemoryEGLImage:
        case wgpu::FeatureName::SharedTextureMemoryIOSurface:
        case wgpu::FeatureName::SharedTextureMemoryOpaqueFD:
        case wgpu::FeatureName::SharedTextureMemoryVkDedicatedAllocation:
        case wgpu::FeatureName::SharedTextureMemoryZirconHandle:
        case wgpu::FeatureName::StaticSamplers:
        case wgpu::FeatureName::SubgroupsF16:
        case wgpu::FeatureName::TransientAttachments:
        case wgpu::FeatureName::YCbCrVulkanSamplers:
        case wgpu::FeatureName::DawnLoadResolveTexture:
        case wgpu::FeatureName::DawnPartialLoadResolveTexture:
        case wgpu::FeatureName::DawnTexelCopyBufferRowAlignment:
        case wgpu::FeatureName::FlexibleTextureViews:
            return false;
    }
    return false;
}

bool Converter::Convert(wgpu::WGSLLanguageFeatureName& out, interop::WGSLLanguageFeatureName in) {
    switch (in) {
        case interop::WGSLLanguageFeatureName::kReadonlyAndReadwriteStorageTextures:
            out = wgpu::WGSLLanguageFeatureName::ReadonlyAndReadwriteStorageTextures;
            return true;
        case interop::WGSLLanguageFeatureName::kPacked4X8IntegerDotProduct:
            out = wgpu::WGSLLanguageFeatureName::Packed4x8IntegerDotProduct;
            return true;
        case interop::WGSLLanguageFeatureName::kUnrestrictedPointerParameters:
            out = wgpu::WGSLLanguageFeatureName::UnrestrictedPointerParameters;
            return true;
        case interop::WGSLLanguageFeatureName::kPointerCompositeAccess:
            out = wgpu::WGSLLanguageFeatureName::PointerCompositeAccess;
            return true;
        case interop::WGSLLanguageFeatureName::kSizedBindingArray:
            out = wgpu::WGSLLanguageFeatureName::SizedBindingArray;
            return true;
    }
    return false;
}

bool Converter::Convert(interop::WGSLLanguageFeatureName& out, wgpu::WGSLLanguageFeatureName in) {
    switch (in) {
        case wgpu::WGSLLanguageFeatureName::ReadonlyAndReadwriteStorageTextures:
            out = interop::WGSLLanguageFeatureName::kReadonlyAndReadwriteStorageTextures;
            return true;
        case wgpu::WGSLLanguageFeatureName::Packed4x8IntegerDotProduct:
            out = interop::WGSLLanguageFeatureName::kPacked4X8IntegerDotProduct;
            return true;
        case wgpu::WGSLLanguageFeatureName::UnrestrictedPointerParameters:
            out = interop::WGSLLanguageFeatureName::kUnrestrictedPointerParameters;
            return true;
        case wgpu::WGSLLanguageFeatureName::PointerCompositeAccess:
            out = interop::WGSLLanguageFeatureName::kPointerCompositeAccess;
            return true;
        case wgpu::WGSLLanguageFeatureName::SizedBindingArray:
            out = interop::WGSLLanguageFeatureName::kSizedBindingArray;
            return true;

        case wgpu::WGSLLanguageFeatureName::ChromiumTestingUnimplemented:
        case wgpu::WGSLLanguageFeatureName::ChromiumTestingUnsafeExperimental:
        case wgpu::WGSLLanguageFeatureName::ChromiumTestingExperimental:
        case wgpu::WGSLLanguageFeatureName::ChromiumTestingShippedWithKillswitch:
        case wgpu::WGSLLanguageFeatureName::ChromiumTestingShipped:
            return false;
    }
    return false;
}
bool Converter::Convert(interop::GPUQueryType& out, wgpu::QueryType in) {
    switch (in) {
        case wgpu::QueryType::Occlusion:
            out = interop::GPUQueryType::kOcclusion;
            return true;
        case wgpu::QueryType::Timestamp:
            out = interop::GPUQueryType::kTimestamp;
            return true;
    }
    return false;
}

bool Converter::Convert(interop::GPUBufferMapState& out, wgpu::BufferMapState in) {
    switch (in) {
        case wgpu::BufferMapState::Unmapped:
            out = interop::GPUBufferMapState::kUnmapped;
            return true;
        case wgpu::BufferMapState::Pending:
            out = interop::GPUBufferMapState::kPending;
            return true;
        case wgpu::BufferMapState::Mapped:
            out = interop::GPUBufferMapState::kMapped;
            return true;
    }
    return false;
}

bool Converter::Convert(wgpu::AddressMode& out, const interop::GPUAddressMode& in) {
    out = wgpu::AddressMode::Repeat;
    switch (in) {
        case interop::GPUAddressMode::kClampToEdge:
            out = wgpu::AddressMode::ClampToEdge;
            return true;
        case interop::GPUAddressMode::kRepeat:
            out = wgpu::AddressMode::Repeat;
            return true;
        case interop::GPUAddressMode::kMirrorRepeat:
            out = wgpu::AddressMode::MirrorRepeat;
            return true;
    }
    return Throw("invalid value for GPUAddressMode");
}

bool Converter::Convert(wgpu::FilterMode& out, const interop::GPUFilterMode& in) {
    out = wgpu::FilterMode::Nearest;
    switch (in) {
        case interop::GPUFilterMode::kNearest:
            out = wgpu::FilterMode::Nearest;
            return true;
        case interop::GPUFilterMode::kLinear:
            out = wgpu::FilterMode::Linear;
            return true;
    }
    return Throw("invalid value for GPUFilterMode");
}

bool Converter::Convert(wgpu::MipmapFilterMode& out, const interop::GPUMipmapFilterMode& in) {
    out = wgpu::MipmapFilterMode::Nearest;
    switch (in) {
        case interop::GPUMipmapFilterMode::kNearest:
            out = wgpu::MipmapFilterMode::Nearest;
            return true;
        case interop::GPUMipmapFilterMode::kLinear:
            out = wgpu::MipmapFilterMode::Linear;
            return true;
    }
    return Throw("invalid value for GPUMipmapFilterMode");
}

bool Converter::Convert(wgpu::ComputePipelineDescriptor& out,
                        const interop::GPUComputePipelineDescriptor& in) {
    return Convert(out.label, in.label) &&    //
           Convert(out.layout, in.layout) &&  //
           Convert(out.compute, in.compute);
}

bool Converter::Convert(wgpu::RenderPipelineDescriptor& out,
                        const interop::GPURenderPipelineDescriptor& in) {
    return Convert(out.label, in.label) &&                //
           Convert(out.layout, in.layout) &&              //
           Convert(out.vertex, in.vertex) &&              //
           Convert(out.primitive, in.primitive) &&        //
           Convert(out.depthStencil, in.depthStencil) &&  //
           Convert(out.multisample, in.multisample) &&    //
           Convert(out.fragment, in.fragment);
}

bool Converter::Convert(wgpu::PipelineLayout& out, const interop::GPUAutoLayoutMode& in) {
    out = nullptr;
    return true;
}

bool Converter::Convert(wgpu::Bool& out, const bool& in) {
    out = in;
    return true;
}

bool Converter::Convert(wgpu::OptionalBool& out, const std::optional<bool>& in) {
    out = in;
    return true;
}

char* Converter::ConvertStringReplacingNull(std::string_view in) {
    char* out = Allocate<char>(in.size() + 1);
    out[in.size()] = '\0';

    for (size_t i = 0; i < in.size(); i++) {
        if (in[i] == '\0') {
            out[i] = '#';
        } else {
            out[i] = in[i];
        }
    }

    return out;
}

bool Converter::Throw(std::string&& message) {
    return Throw(Napi::Error::New(env, message));
}

bool Converter::Throw(Napi::Error&& error) {
    error.ThrowAsJavaScriptException();
    return false;
}

std::string CopyLabel(StringView label) {
    if (label.data == nullptr) {
        return {};
    }
    size_t length = label.length == WGPU_STRLEN ? std::strlen(label.data) : label.length;
    return {label.data, length};
}

}  // namespace wgpu::binding
