// Copyright 2023 The Dawn & Tint Authors
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

#include "dawn/native/d3d/UtilsD3D.h"

#include <utility>

#include "dawn/native/Device.h"

namespace dawn::native::d3d {

ResultOrError<std::wstring> ConvertStringToWstring(std::string_view s) {
    size_t len = s.length();
    if (len == 0) {
        return std::wstring();
    }
    int numChars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), len, nullptr, 0);
    if (numChars == 0) {
        return DAWN_INTERNAL_ERROR("Failed to convert string to wide string");
    }
    std::wstring result;
    result.resize(numChars);
    int numConvertedChars =
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), len, &result[0], numChars);
    if (numConvertedChars != numChars) {
        return DAWN_INTERNAL_ERROR("Failed to convert string to wide string");
    }
    return std::move(result);
}

bool IsTypeless(DXGI_FORMAT format) {
    // List generated from <dxgiformat.h>
    switch (format) {
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case DXGI_FORMAT_R32G32B32_TYPELESS:
        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case DXGI_FORMAT_R32G32_TYPELESS:
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R16G16_TYPELESS:
        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_R24G8_TYPELESS:
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        case DXGI_FORMAT_R8G8_TYPELESS:
        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_R8_TYPELESS:
        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
        case DXGI_FORMAT_BC6H_TYPELESS:
        case DXGI_FORMAT_BC7_TYPELESS:
            return true;
        default:
            return false;
    }
}

bool IsDepthStencil(DXGI_FORMAT format) {
    switch (format) {
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            return true;
        default:
            return false;
    }
}

DXGI_FORMAT DXGITypelessTextureFormat(const DeviceBase* device, wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::R8Unorm:
        case wgpu::TextureFormat::R8Snorm:
        case wgpu::TextureFormat::R8Uint:
        case wgpu::TextureFormat::R8Sint:
            return DXGI_FORMAT_R8_TYPELESS;

        case wgpu::TextureFormat::R16Unorm:
        case wgpu::TextureFormat::R16Snorm:
        case wgpu::TextureFormat::R16Uint:
        case wgpu::TextureFormat::R16Sint:
        case wgpu::TextureFormat::R16Float:
        case wgpu::TextureFormat::Depth16Unorm:
            return DXGI_FORMAT_R16_TYPELESS;

        case wgpu::TextureFormat::RG8Unorm:
        case wgpu::TextureFormat::RG8Snorm:
        case wgpu::TextureFormat::RG8Uint:
        case wgpu::TextureFormat::RG8Sint:
            return DXGI_FORMAT_R8G8_TYPELESS;

        case wgpu::TextureFormat::R32Uint:
        case wgpu::TextureFormat::R32Sint:
        case wgpu::TextureFormat::R32Float:
            return DXGI_FORMAT_R32_TYPELESS;

        case wgpu::TextureFormat::RG16Unorm:
        case wgpu::TextureFormat::RG16Snorm:
        case wgpu::TextureFormat::RG16Uint:
        case wgpu::TextureFormat::RG16Sint:
        case wgpu::TextureFormat::RG16Float:
            return DXGI_FORMAT_R16G16_TYPELESS;

        case wgpu::TextureFormat::RGBA8Unorm:
        case wgpu::TextureFormat::RGBA8UnormSrgb:
        case wgpu::TextureFormat::RGBA8Snorm:
        case wgpu::TextureFormat::RGBA8Uint:
        case wgpu::TextureFormat::RGBA8Sint:
            return DXGI_FORMAT_R8G8B8A8_TYPELESS;

        case wgpu::TextureFormat::BGRA8Unorm:
        case wgpu::TextureFormat::BGRA8UnormSrgb:
            return DXGI_FORMAT_B8G8R8A8_TYPELESS;

        case wgpu::TextureFormat::RGB10A2Uint:
        case wgpu::TextureFormat::RGB10A2Unorm:
            return DXGI_FORMAT_R10G10B10A2_TYPELESS;

        case wgpu::TextureFormat::RG11B10Ufloat:
            return DXGI_FORMAT_R11G11B10_FLOAT;
        case wgpu::TextureFormat::RGB9E5Ufloat:
            return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;

        case wgpu::TextureFormat::RG32Uint:
        case wgpu::TextureFormat::RG32Sint:
        case wgpu::TextureFormat::RG32Float:
            return DXGI_FORMAT_R32G32_TYPELESS;

        case wgpu::TextureFormat::RGBA16Unorm:
        case wgpu::TextureFormat::RGBA16Snorm:
        case wgpu::TextureFormat::RGBA16Uint:
        case wgpu::TextureFormat::RGBA16Sint:
        case wgpu::TextureFormat::RGBA16Float:
            return DXGI_FORMAT_R16G16B16A16_TYPELESS;

        case wgpu::TextureFormat::RGBA32Uint:
        case wgpu::TextureFormat::RGBA32Sint:
        case wgpu::TextureFormat::RGBA32Float:
            return DXGI_FORMAT_R32G32B32A32_TYPELESS;

        case wgpu::TextureFormat::Depth32Float:
        case wgpu::TextureFormat::Depth24Plus:
            return DXGI_FORMAT_R32_TYPELESS;

        // DXGI_FORMAT_D24_UNORM_S8_UINT is the smallest format supported on D3D12 that has stencil,
        // for which the typeless equivalent is DXGI_FORMAT_R24G8_TYPELESS.
        case wgpu::TextureFormat::Stencil8:
            return DXGI_FORMAT_R24G8_TYPELESS;

        case wgpu::TextureFormat::Depth24PlusStencil8:
            return device->IsToggleEnabled(Toggle::UsePackedDepth24UnormStencil8Format)
                       ? DXGI_FORMAT_R24G8_TYPELESS
                       : DXGI_FORMAT_R32G8X24_TYPELESS;

        case wgpu::TextureFormat::Depth32FloatStencil8:
            return DXGI_FORMAT_R32G8X24_TYPELESS;

        case wgpu::TextureFormat::BC1RGBAUnorm:
        case wgpu::TextureFormat::BC1RGBAUnormSrgb:
            return DXGI_FORMAT_BC1_TYPELESS;

        case wgpu::TextureFormat::BC2RGBAUnorm:
        case wgpu::TextureFormat::BC2RGBAUnormSrgb:
            return DXGI_FORMAT_BC2_TYPELESS;

        case wgpu::TextureFormat::BC3RGBAUnorm:
        case wgpu::TextureFormat::BC3RGBAUnormSrgb:
            return DXGI_FORMAT_BC3_TYPELESS;

        case wgpu::TextureFormat::BC4RSnorm:
        case wgpu::TextureFormat::BC4RUnorm:
            return DXGI_FORMAT_BC4_TYPELESS;

        case wgpu::TextureFormat::BC5RGSnorm:
        case wgpu::TextureFormat::BC5RGUnorm:
            return DXGI_FORMAT_BC5_TYPELESS;

        case wgpu::TextureFormat::BC6HRGBFloat:
        case wgpu::TextureFormat::BC6HRGBUfloat:
            return DXGI_FORMAT_BC6H_TYPELESS;

        case wgpu::TextureFormat::BC7RGBAUnorm:
        case wgpu::TextureFormat::BC7RGBAUnormSrgb:
            return DXGI_FORMAT_BC7_TYPELESS;

        case wgpu::TextureFormat::ETC2RGB8Unorm:
        case wgpu::TextureFormat::ETC2RGB8UnormSrgb:
        case wgpu::TextureFormat::ETC2RGB8A1Unorm:
        case wgpu::TextureFormat::ETC2RGB8A1UnormSrgb:
        case wgpu::TextureFormat::ETC2RGBA8Unorm:
        case wgpu::TextureFormat::ETC2RGBA8UnormSrgb:
        case wgpu::TextureFormat::EACR11Unorm:
        case wgpu::TextureFormat::EACR11Snorm:
        case wgpu::TextureFormat::EACRG11Unorm:
        case wgpu::TextureFormat::EACRG11Snorm:

        case wgpu::TextureFormat::ASTC4x4Unorm:
        case wgpu::TextureFormat::ASTC4x4UnormSrgb:
        case wgpu::TextureFormat::ASTC5x4Unorm:
        case wgpu::TextureFormat::ASTC5x4UnormSrgb:
        case wgpu::TextureFormat::ASTC5x5Unorm:
        case wgpu::TextureFormat::ASTC5x5UnormSrgb:
        case wgpu::TextureFormat::ASTC6x5Unorm:
        case wgpu::TextureFormat::ASTC6x5UnormSrgb:
        case wgpu::TextureFormat::ASTC6x6Unorm:
        case wgpu::TextureFormat::ASTC6x6UnormSrgb:
        case wgpu::TextureFormat::ASTC8x5Unorm:
        case wgpu::TextureFormat::ASTC8x5UnormSrgb:
        case wgpu::TextureFormat::ASTC8x6Unorm:
        case wgpu::TextureFormat::ASTC8x6UnormSrgb:
        case wgpu::TextureFormat::ASTC8x8Unorm:
        case wgpu::TextureFormat::ASTC8x8UnormSrgb:
        case wgpu::TextureFormat::ASTC10x5Unorm:
        case wgpu::TextureFormat::ASTC10x5UnormSrgb:
        case wgpu::TextureFormat::ASTC10x6Unorm:
        case wgpu::TextureFormat::ASTC10x6UnormSrgb:
        case wgpu::TextureFormat::ASTC10x8Unorm:
        case wgpu::TextureFormat::ASTC10x8UnormSrgb:
        case wgpu::TextureFormat::ASTC10x10Unorm:
        case wgpu::TextureFormat::ASTC10x10UnormSrgb:
        case wgpu::TextureFormat::ASTC12x10Unorm:
        case wgpu::TextureFormat::ASTC12x10UnormSrgb:
        case wgpu::TextureFormat::ASTC12x12Unorm:
        case wgpu::TextureFormat::ASTC12x12UnormSrgb:

        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar422Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar444Unorm:
        case wgpu::TextureFormat::R8BG8A8Triplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm:
        case wgpu::TextureFormat::External:
        case wgpu::TextureFormat::Undefined:
            DAWN_UNREACHABLE();
    }
}

#define UNCOMPRESSED_COLOR_FORMATS(X)                                       \
    X(wgpu::TextureFormat::R8Unorm, DXGI_FORMAT_R8_UNORM)                   \
    X(wgpu::TextureFormat::R8Snorm, DXGI_FORMAT_R8_SNORM)                   \
    X(wgpu::TextureFormat::R8Uint, DXGI_FORMAT_R8_UINT)                     \
    X(wgpu::TextureFormat::R8Sint, DXGI_FORMAT_R8_SINT)                     \
                                                                            \
    X(wgpu::TextureFormat::R16Unorm, DXGI_FORMAT_R16_UNORM)                 \
    X(wgpu::TextureFormat::R16Snorm, DXGI_FORMAT_R16_SNORM)                 \
    X(wgpu::TextureFormat::R16Uint, DXGI_FORMAT_R16_UINT)                   \
    X(wgpu::TextureFormat::R16Sint, DXGI_FORMAT_R16_SINT)                   \
    X(wgpu::TextureFormat::R16Float, DXGI_FORMAT_R16_FLOAT)                 \
    X(wgpu::TextureFormat::RG8Unorm, DXGI_FORMAT_R8G8_UNORM)                \
    X(wgpu::TextureFormat::RG8Snorm, DXGI_FORMAT_R8G8_SNORM)                \
    X(wgpu::TextureFormat::RG8Uint, DXGI_FORMAT_R8G8_UINT)                  \
    X(wgpu::TextureFormat::RG8Sint, DXGI_FORMAT_R8G8_SINT)                  \
                                                                            \
    X(wgpu::TextureFormat::R32Uint, DXGI_FORMAT_R32_UINT)                   \
    X(wgpu::TextureFormat::R32Sint, DXGI_FORMAT_R32_SINT)                   \
    X(wgpu::TextureFormat::R32Float, DXGI_FORMAT_R32_FLOAT)                 \
    X(wgpu::TextureFormat::RG16Unorm, DXGI_FORMAT_R16G16_UNORM)             \
    X(wgpu::TextureFormat::RG16Snorm, DXGI_FORMAT_R16G16_SNORM)             \
    X(wgpu::TextureFormat::RG16Uint, DXGI_FORMAT_R16G16_UINT)               \
    X(wgpu::TextureFormat::RG16Sint, DXGI_FORMAT_R16G16_SINT)               \
    X(wgpu::TextureFormat::RG16Float, DXGI_FORMAT_R16G16_FLOAT)             \
    X(wgpu::TextureFormat::RGBA8Unorm, DXGI_FORMAT_R8G8B8A8_UNORM)          \
    X(wgpu::TextureFormat::RGBA8UnormSrgb, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB) \
    X(wgpu::TextureFormat::RGBA8Snorm, DXGI_FORMAT_R8G8B8A8_SNORM)          \
    X(wgpu::TextureFormat::RGBA8Uint, DXGI_FORMAT_R8G8B8A8_UINT)            \
    X(wgpu::TextureFormat::RGBA8Sint, DXGI_FORMAT_R8G8B8A8_SINT)            \
    X(wgpu::TextureFormat::BGRA8Unorm, DXGI_FORMAT_B8G8R8A8_UNORM)          \
    X(wgpu::TextureFormat::BGRA8UnormSrgb, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB) \
    X(wgpu::TextureFormat::RGB10A2Uint, DXGI_FORMAT_R10G10B10A2_UINT)       \
    X(wgpu::TextureFormat::RGB10A2Unorm, DXGI_FORMAT_R10G10B10A2_UNORM)     \
    X(wgpu::TextureFormat::RG11B10Ufloat, DXGI_FORMAT_R11G11B10_FLOAT)      \
    X(wgpu::TextureFormat::RGB9E5Ufloat, DXGI_FORMAT_R9G9B9E5_SHAREDEXP)    \
                                                                            \
    X(wgpu::TextureFormat::RG32Uint, DXGI_FORMAT_R32G32_UINT)               \
    X(wgpu::TextureFormat::RG32Sint, DXGI_FORMAT_R32G32_SINT)               \
    X(wgpu::TextureFormat::RG32Float, DXGI_FORMAT_R32G32_FLOAT)             \
    X(wgpu::TextureFormat::RGBA16Unorm, DXGI_FORMAT_R16G16B16A16_UNORM)     \
    X(wgpu::TextureFormat::RGBA16Snorm, DXGI_FORMAT_R16G16B16A16_SNORM)     \
    X(wgpu::TextureFormat::RGBA16Uint, DXGI_FORMAT_R16G16B16A16_UINT)       \
    X(wgpu::TextureFormat::RGBA16Sint, DXGI_FORMAT_R16G16B16A16_SINT)       \
    X(wgpu::TextureFormat::RGBA16Float, DXGI_FORMAT_R16G16B16A16_FLOAT)     \
                                                                            \
    X(wgpu::TextureFormat::RGBA32Uint, DXGI_FORMAT_R32G32B32A32_UINT)       \
    X(wgpu::TextureFormat::RGBA32Sint, DXGI_FORMAT_R32G32B32A32_SINT)       \
    X(wgpu::TextureFormat::RGBA32Float, DXGI_FORMAT_R32G32B32A32_FLOAT)     \
                                                                            \
    X(wgpu::TextureFormat::R8BG8Biplanar420Unorm, DXGI_FORMAT_NV12)         \
    X(wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm, DXGI_FORMAT_P010)

DXGI_FORMAT DXGITextureFormat(const DeviceBase* device, wgpu::TextureFormat format) {
    switch (format) {
#define X(wgpuFormat, dxgiFormat) \
    case wgpuFormat:              \
        return dxgiFormat;
        UNCOMPRESSED_COLOR_FORMATS(X)
#undef X

        case wgpu::TextureFormat::Depth16Unorm:
            return DXGI_FORMAT_D16_UNORM;
        case wgpu::TextureFormat::Depth32Float:
        case wgpu::TextureFormat::Depth24Plus:
            return DXGI_FORMAT_D32_FLOAT;
        // DXGI_FORMAT_D24_UNORM_S8_UINT is the smallest format supported on D3D12 that has stencil.
        case wgpu::TextureFormat::Stencil8:
            return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case wgpu::TextureFormat::Depth24PlusStencil8:
            return device->IsToggleEnabled(Toggle::UsePackedDepth24UnormStencil8Format)
                       ? DXGI_FORMAT_D24_UNORM_S8_UINT
                       : DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        case wgpu::TextureFormat::Depth32FloatStencil8:
            return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

        case wgpu::TextureFormat::BC1RGBAUnorm:
            return DXGI_FORMAT_BC1_UNORM;
        case wgpu::TextureFormat::BC1RGBAUnormSrgb:
            return DXGI_FORMAT_BC1_UNORM_SRGB;
        case wgpu::TextureFormat::BC2RGBAUnorm:
            return DXGI_FORMAT_BC2_UNORM;
        case wgpu::TextureFormat::BC2RGBAUnormSrgb:
            return DXGI_FORMAT_BC2_UNORM_SRGB;
        case wgpu::TextureFormat::BC3RGBAUnorm:
            return DXGI_FORMAT_BC3_UNORM;
        case wgpu::TextureFormat::BC3RGBAUnormSrgb:
            return DXGI_FORMAT_BC3_UNORM_SRGB;
        case wgpu::TextureFormat::BC4RSnorm:
            return DXGI_FORMAT_BC4_SNORM;
        case wgpu::TextureFormat::BC4RUnorm:
            return DXGI_FORMAT_BC4_UNORM;
        case wgpu::TextureFormat::BC5RGSnorm:
            return DXGI_FORMAT_BC5_SNORM;
        case wgpu::TextureFormat::BC5RGUnorm:
            return DXGI_FORMAT_BC5_UNORM;
        case wgpu::TextureFormat::BC6HRGBFloat:
            return DXGI_FORMAT_BC6H_SF16;
        case wgpu::TextureFormat::BC6HRGBUfloat:
            return DXGI_FORMAT_BC6H_UF16;
        case wgpu::TextureFormat::BC7RGBAUnorm:
            return DXGI_FORMAT_BC7_UNORM;
        case wgpu::TextureFormat::BC7RGBAUnormSrgb:
            return DXGI_FORMAT_BC7_UNORM_SRGB;

        case wgpu::TextureFormat::ETC2RGB8Unorm:
        case wgpu::TextureFormat::ETC2RGB8UnormSrgb:
        case wgpu::TextureFormat::ETC2RGB8A1Unorm:
        case wgpu::TextureFormat::ETC2RGB8A1UnormSrgb:
        case wgpu::TextureFormat::ETC2RGBA8Unorm:
        case wgpu::TextureFormat::ETC2RGBA8UnormSrgb:
        case wgpu::TextureFormat::EACR11Unorm:
        case wgpu::TextureFormat::EACR11Snorm:
        case wgpu::TextureFormat::EACRG11Unorm:
        case wgpu::TextureFormat::EACRG11Snorm:

        case wgpu::TextureFormat::ASTC4x4Unorm:
        case wgpu::TextureFormat::ASTC4x4UnormSrgb:
        case wgpu::TextureFormat::ASTC5x4Unorm:
        case wgpu::TextureFormat::ASTC5x4UnormSrgb:
        case wgpu::TextureFormat::ASTC5x5Unorm:
        case wgpu::TextureFormat::ASTC5x5UnormSrgb:
        case wgpu::TextureFormat::ASTC6x5Unorm:
        case wgpu::TextureFormat::ASTC6x5UnormSrgb:
        case wgpu::TextureFormat::ASTC6x6Unorm:
        case wgpu::TextureFormat::ASTC6x6UnormSrgb:
        case wgpu::TextureFormat::ASTC8x5Unorm:
        case wgpu::TextureFormat::ASTC8x5UnormSrgb:
        case wgpu::TextureFormat::ASTC8x6Unorm:
        case wgpu::TextureFormat::ASTC8x6UnormSrgb:
        case wgpu::TextureFormat::ASTC8x8Unorm:
        case wgpu::TextureFormat::ASTC8x8UnormSrgb:
        case wgpu::TextureFormat::ASTC10x5Unorm:
        case wgpu::TextureFormat::ASTC10x5UnormSrgb:
        case wgpu::TextureFormat::ASTC10x6Unorm:
        case wgpu::TextureFormat::ASTC10x6UnormSrgb:
        case wgpu::TextureFormat::ASTC10x8Unorm:
        case wgpu::TextureFormat::ASTC10x8UnormSrgb:
        case wgpu::TextureFormat::ASTC10x10Unorm:
        case wgpu::TextureFormat::ASTC10x10UnormSrgb:
        case wgpu::TextureFormat::ASTC12x10Unorm:
        case wgpu::TextureFormat::ASTC12x10UnormSrgb:
        case wgpu::TextureFormat::ASTC12x12Unorm:
        case wgpu::TextureFormat::ASTC12x12UnormSrgb:
        case wgpu::TextureFormat::R8BG8Biplanar422Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar444Unorm:
        case wgpu::TextureFormat::R8BG8A8Triplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm:
        case wgpu::TextureFormat::External:

        case wgpu::TextureFormat::Undefined:
            DAWN_UNREACHABLE();
    }
}

ResultOrError<wgpu::TextureFormat> FromUncompressedColorDXGITextureFormat(DXGI_FORMAT format) {
    switch (format) {
#define X(wgpuFormat, dxgiFormat) \
    case dxgiFormat:              \
        return wgpuFormat;
        UNCOMPRESSED_COLOR_FORMATS(X)
#undef X

        default:
            return DAWN_VALIDATION_ERROR("Unsupported DXGI format %x", format);
    }
}

#undef UNCOMPRESSED_COLOR_FORMATS

DXGI_FORMAT DXGIVertexFormat(wgpu::VertexFormat format) {
    switch (format) {
        case wgpu::VertexFormat::Uint8:
            return DXGI_FORMAT_R8_UINT;
        case wgpu::VertexFormat::Uint8x2:
            return DXGI_FORMAT_R8G8_UINT;
        case wgpu::VertexFormat::Uint8x4:
            return DXGI_FORMAT_R8G8B8A8_UINT;
        case wgpu::VertexFormat::Sint8:
            return DXGI_FORMAT_R8_SINT;
        case wgpu::VertexFormat::Sint8x2:
            return DXGI_FORMAT_R8G8_SINT;
        case wgpu::VertexFormat::Sint8x4:
            return DXGI_FORMAT_R8G8B8A8_SINT;
        case wgpu::VertexFormat::Unorm8:
            return DXGI_FORMAT_R8_UNORM;
        case wgpu::VertexFormat::Unorm8x2:
            return DXGI_FORMAT_R8G8_UNORM;
        case wgpu::VertexFormat::Unorm8x4:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case wgpu::VertexFormat::Snorm8:
            return DXGI_FORMAT_R8_SNORM;
        case wgpu::VertexFormat::Snorm8x2:
            return DXGI_FORMAT_R8G8_SNORM;
        case wgpu::VertexFormat::Snorm8x4:
            return DXGI_FORMAT_R8G8B8A8_SNORM;
        case wgpu::VertexFormat::Uint16:
            return DXGI_FORMAT_R16_UINT;
        case wgpu::VertexFormat::Uint16x2:
            return DXGI_FORMAT_R16G16_UINT;
        case wgpu::VertexFormat::Uint16x4:
            return DXGI_FORMAT_R16G16B16A16_UINT;
        case wgpu::VertexFormat::Sint16:
            return DXGI_FORMAT_R16_SINT;
        case wgpu::VertexFormat::Sint16x2:
            return DXGI_FORMAT_R16G16_SINT;
        case wgpu::VertexFormat::Sint16x4:
            return DXGI_FORMAT_R16G16B16A16_SINT;
        case wgpu::VertexFormat::Unorm16:
            return DXGI_FORMAT_R16_UNORM;
        case wgpu::VertexFormat::Unorm16x2:
            return DXGI_FORMAT_R16G16_UNORM;
        case wgpu::VertexFormat::Unorm16x4:
            return DXGI_FORMAT_R16G16B16A16_UNORM;
        case wgpu::VertexFormat::Snorm16:
            return DXGI_FORMAT_R16_SNORM;
        case wgpu::VertexFormat::Snorm16x2:
            return DXGI_FORMAT_R16G16_SNORM;
        case wgpu::VertexFormat::Snorm16x4:
            return DXGI_FORMAT_R16G16B16A16_SNORM;
        case wgpu::VertexFormat::Float16:
            return DXGI_FORMAT_R16_FLOAT;
        case wgpu::VertexFormat::Float16x2:
            return DXGI_FORMAT_R16G16_FLOAT;
        case wgpu::VertexFormat::Float16x4:
            return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case wgpu::VertexFormat::Float32:
            return DXGI_FORMAT_R32_FLOAT;
        case wgpu::VertexFormat::Float32x2:
            return DXGI_FORMAT_R32G32_FLOAT;
        case wgpu::VertexFormat::Float32x3:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case wgpu::VertexFormat::Float32x4:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case wgpu::VertexFormat::Uint32:
            return DXGI_FORMAT_R32_UINT;
        case wgpu::VertexFormat::Uint32x2:
            return DXGI_FORMAT_R32G32_UINT;
        case wgpu::VertexFormat::Uint32x3:
            return DXGI_FORMAT_R32G32B32_UINT;
        case wgpu::VertexFormat::Uint32x4:
            return DXGI_FORMAT_R32G32B32A32_UINT;
        case wgpu::VertexFormat::Sint32:
            return DXGI_FORMAT_R32_SINT;
        case wgpu::VertexFormat::Sint32x2:
            return DXGI_FORMAT_R32G32_SINT;
        case wgpu::VertexFormat::Sint32x3:
            return DXGI_FORMAT_R32G32B32_SINT;
        case wgpu::VertexFormat::Sint32x4:
            return DXGI_FORMAT_R32G32B32A32_SINT;
        case wgpu::VertexFormat::Unorm10_10_10_2:
            return DXGI_FORMAT_R10G10B10A2_UNORM;
        case wgpu::VertexFormat::Unorm8x4BGRA:
            return DXGI_FORMAT_B8G8R8A8_UNORM;
        default:
            DAWN_UNREACHABLE();
    }
}

DXGI_FORMAT D3DShaderResourceViewFormat(const DeviceBase* device,
                                        const Format& textureFormat,
                                        const Format& viewFormat,
                                        Aspect aspects) {
    DAWN_ASSERT(aspects != Aspect::None);
    if (!HasZeroOrOneBits(aspects)) {
        // A single aspect is not selected. The texture view must not be sampled.
        return DXGI_FORMAT_UNKNOWN;
    }
    // Note that this will configure the SRV descriptor to reinterpret the texture allocated as
    // TYPELESS as a single-plane shader-accessible view.
    DXGI_FORMAT srvFormat = DXGITextureFormat(device, viewFormat.format);
    if (textureFormat.HasDepthOrStencil()) {
        // Depth-stencil formats must be mapped to compatible shader-accessible view format.
        switch (DXGITextureFormat(device, textureFormat.format)) {
            case DXGI_FORMAT_D32_FLOAT:
                srvFormat = DXGI_FORMAT_R32_FLOAT;
                break;
            case DXGI_FORMAT_D16_UNORM:
                srvFormat = DXGI_FORMAT_R16_UNORM;
                break;
            case DXGI_FORMAT_D24_UNORM_S8_UINT: {
                switch (aspects) {
                    case Aspect::Depth:
                        srvFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
                        break;
                    case Aspect::Stencil:
                        srvFormat = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
                        break;
                    default:
                        DAWN_UNREACHABLE();
                        break;
                }
                break;
            }
            case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: {
                switch (aspects) {
                    case Aspect::Depth:
                        srvFormat = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
                        break;
                    case Aspect::Stencil:
                        srvFormat = DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
                        break;
                    default:
                        DAWN_UNREACHABLE();
                        break;
                }
                break;
            }
            default:
                DAWN_UNREACHABLE();
                break;
        }
    }
    return srvFormat;
}

}  // namespace dawn::native::d3d
