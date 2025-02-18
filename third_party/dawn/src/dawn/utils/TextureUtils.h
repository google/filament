// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_UTILS_TEXTUREUTILS_H_
#define SRC_DAWN_UTILS_TEXTUREUTILS_H_

#include <webgpu/webgpu_cpp.h>

#include <array>

#include "dawn/common/Assert.h"

namespace dawn::utils {

#ifndef __EMSCRIPTEN__
static constexpr std::array<wgpu::TextureFormat, 101> kAllTextureFormats = {
    wgpu::TextureFormat::R8Unorm,
    wgpu::TextureFormat::R8Snorm,
    wgpu::TextureFormat::R8Uint,
    wgpu::TextureFormat::R8Sint,
    wgpu::TextureFormat::R16Unorm,
    wgpu::TextureFormat::R16Snorm,
    wgpu::TextureFormat::R16Uint,
    wgpu::TextureFormat::R16Sint,
    wgpu::TextureFormat::R16Float,
    wgpu::TextureFormat::RG8Unorm,
    wgpu::TextureFormat::RG8Snorm,
    wgpu::TextureFormat::RG8Uint,
    wgpu::TextureFormat::RG8Sint,
    wgpu::TextureFormat::R32Float,
    wgpu::TextureFormat::R32Uint,
    wgpu::TextureFormat::R32Sint,
    wgpu::TextureFormat::RG16Unorm,
    wgpu::TextureFormat::RG16Snorm,
    wgpu::TextureFormat::RG16Uint,
    wgpu::TextureFormat::RG16Sint,
    wgpu::TextureFormat::RG16Float,
    wgpu::TextureFormat::RGBA8Unorm,
    wgpu::TextureFormat::RGBA8UnormSrgb,
    wgpu::TextureFormat::RGBA8Snorm,
    wgpu::TextureFormat::RGBA8Uint,
    wgpu::TextureFormat::RGBA8Sint,
    wgpu::TextureFormat::BGRA8Unorm,
    wgpu::TextureFormat::BGRA8UnormSrgb,
    wgpu::TextureFormat::RGB10A2Uint,
    wgpu::TextureFormat::RGB10A2Unorm,
    wgpu::TextureFormat::RG11B10Ufloat,
    wgpu::TextureFormat::RGB9E5Ufloat,
    wgpu::TextureFormat::RG32Float,
    wgpu::TextureFormat::RG32Uint,
    wgpu::TextureFormat::RG32Sint,
    wgpu::TextureFormat::RGBA16Unorm,
    wgpu::TextureFormat::RGBA16Snorm,
    wgpu::TextureFormat::RGBA16Uint,
    wgpu::TextureFormat::RGBA16Sint,
    wgpu::TextureFormat::RGBA16Float,
    wgpu::TextureFormat::RGBA32Float,
    wgpu::TextureFormat::RGBA32Uint,
    wgpu::TextureFormat::RGBA32Sint,
    wgpu::TextureFormat::Depth16Unorm,
    wgpu::TextureFormat::Depth32Float,
    wgpu::TextureFormat::Depth24Plus,
    wgpu::TextureFormat::Depth24PlusStencil8,
    wgpu::TextureFormat::Depth32FloatStencil8,
    wgpu::TextureFormat::Stencil8,
    wgpu::TextureFormat::BC1RGBAUnorm,
    wgpu::TextureFormat::BC1RGBAUnormSrgb,
    wgpu::TextureFormat::BC2RGBAUnorm,
    wgpu::TextureFormat::BC2RGBAUnormSrgb,
    wgpu::TextureFormat::BC3RGBAUnorm,
    wgpu::TextureFormat::BC3RGBAUnormSrgb,
    wgpu::TextureFormat::BC4RUnorm,
    wgpu::TextureFormat::BC4RSnorm,
    wgpu::TextureFormat::BC5RGUnorm,
    wgpu::TextureFormat::BC5RGSnorm,
    wgpu::TextureFormat::BC6HRGBUfloat,
    wgpu::TextureFormat::BC6HRGBFloat,
    wgpu::TextureFormat::BC7RGBAUnorm,
    wgpu::TextureFormat::BC7RGBAUnormSrgb,
    wgpu::TextureFormat::ETC2RGB8Unorm,
    wgpu::TextureFormat::ETC2RGB8UnormSrgb,
    wgpu::TextureFormat::ETC2RGB8A1Unorm,
    wgpu::TextureFormat::ETC2RGB8A1UnormSrgb,
    wgpu::TextureFormat::ETC2RGBA8Unorm,
    wgpu::TextureFormat::ETC2RGBA8UnormSrgb,
    wgpu::TextureFormat::EACR11Unorm,
    wgpu::TextureFormat::EACR11Snorm,
    wgpu::TextureFormat::EACRG11Unorm,
    wgpu::TextureFormat::EACRG11Snorm,
    wgpu::TextureFormat::ASTC4x4Unorm,
    wgpu::TextureFormat::ASTC4x4UnormSrgb,
    wgpu::TextureFormat::ASTC5x4Unorm,
    wgpu::TextureFormat::ASTC5x4UnormSrgb,
    wgpu::TextureFormat::ASTC5x5Unorm,
    wgpu::TextureFormat::ASTC5x5UnormSrgb,
    wgpu::TextureFormat::ASTC6x5Unorm,
    wgpu::TextureFormat::ASTC6x5UnormSrgb,
    wgpu::TextureFormat::ASTC6x6Unorm,
    wgpu::TextureFormat::ASTC6x6UnormSrgb,
    wgpu::TextureFormat::ASTC8x5Unorm,
    wgpu::TextureFormat::ASTC8x5UnormSrgb,
    wgpu::TextureFormat::ASTC8x6Unorm,
    wgpu::TextureFormat::ASTC8x6UnormSrgb,
    wgpu::TextureFormat::ASTC8x8Unorm,
    wgpu::TextureFormat::ASTC8x8UnormSrgb,
    wgpu::TextureFormat::ASTC10x5Unorm,
    wgpu::TextureFormat::ASTC10x5UnormSrgb,
    wgpu::TextureFormat::ASTC10x6Unorm,
    wgpu::TextureFormat::ASTC10x6UnormSrgb,
    wgpu::TextureFormat::ASTC10x8Unorm,
    wgpu::TextureFormat::ASTC10x8UnormSrgb,
    wgpu::TextureFormat::ASTC10x10Unorm,
    wgpu::TextureFormat::ASTC10x10UnormSrgb,
    wgpu::TextureFormat::ASTC12x10Unorm,
    wgpu::TextureFormat::ASTC12x10UnormSrgb,
    wgpu::TextureFormat::ASTC12x12Unorm,
    wgpu::TextureFormat::ASTC12x12UnormSrgb};
#endif  // __EMSCRIPTEN__

static constexpr std::array<wgpu::TextureFormat, 41> kFormatsInCoreSpec = {
    wgpu::TextureFormat::R8Unorm,
    wgpu::TextureFormat::R8Snorm,
    wgpu::TextureFormat::R8Uint,
    wgpu::TextureFormat::R8Sint,
    wgpu::TextureFormat::R16Uint,
    wgpu::TextureFormat::R16Sint,
    wgpu::TextureFormat::R16Float,
    wgpu::TextureFormat::RG8Unorm,
    wgpu::TextureFormat::RG8Snorm,
    wgpu::TextureFormat::RG8Uint,
    wgpu::TextureFormat::RG8Sint,
    wgpu::TextureFormat::R32Float,
    wgpu::TextureFormat::R32Uint,
    wgpu::TextureFormat::R32Sint,
    wgpu::TextureFormat::RG16Uint,
    wgpu::TextureFormat::RG16Sint,
    wgpu::TextureFormat::RG16Float,
    wgpu::TextureFormat::RGBA8Unorm,
    wgpu::TextureFormat::RGBA8UnormSrgb,
    wgpu::TextureFormat::RGBA8Snorm,
    wgpu::TextureFormat::RGBA8Uint,
    wgpu::TextureFormat::RGBA8Sint,
    wgpu::TextureFormat::BGRA8Unorm,
    wgpu::TextureFormat::BGRA8UnormSrgb,
    wgpu::TextureFormat::RGB10A2Uint,
    wgpu::TextureFormat::RGB10A2Unorm,
    wgpu::TextureFormat::RG11B10Ufloat,
    wgpu::TextureFormat::RGB9E5Ufloat,
    wgpu::TextureFormat::RG32Float,
    wgpu::TextureFormat::RG32Uint,
    wgpu::TextureFormat::RG32Sint,
    wgpu::TextureFormat::RGBA16Uint,
    wgpu::TextureFormat::RGBA16Sint,
    wgpu::TextureFormat::RGBA16Float,
    wgpu::TextureFormat::RGBA32Float,
    wgpu::TextureFormat::RGBA32Uint,
    wgpu::TextureFormat::RGBA32Sint,
    wgpu::TextureFormat::Depth16Unorm,
    wgpu::TextureFormat::Depth32Float,
    wgpu::TextureFormat::Depth24Plus,
    wgpu::TextureFormat::Depth24PlusStencil8,
};

static constexpr std::array<wgpu::TextureFormat, 14> kBCFormats = {
    wgpu::TextureFormat::BC1RGBAUnorm,  wgpu::TextureFormat::BC1RGBAUnormSrgb,
    wgpu::TextureFormat::BC2RGBAUnorm,  wgpu::TextureFormat::BC2RGBAUnormSrgb,
    wgpu::TextureFormat::BC3RGBAUnorm,  wgpu::TextureFormat::BC3RGBAUnormSrgb,
    wgpu::TextureFormat::BC4RUnorm,     wgpu::TextureFormat::BC4RSnorm,
    wgpu::TextureFormat::BC5RGUnorm,    wgpu::TextureFormat::BC5RGSnorm,
    wgpu::TextureFormat::BC6HRGBUfloat, wgpu::TextureFormat::BC6HRGBFloat,
    wgpu::TextureFormat::BC7RGBAUnorm,  wgpu::TextureFormat::BC7RGBAUnormSrgb};

static constexpr std::array<wgpu::TextureFormat, 10> kETC2Formats = {
    wgpu::TextureFormat::ETC2RGB8Unorm,   wgpu::TextureFormat::ETC2RGB8UnormSrgb,
    wgpu::TextureFormat::ETC2RGB8A1Unorm, wgpu::TextureFormat::ETC2RGB8A1UnormSrgb,
    wgpu::TextureFormat::ETC2RGBA8Unorm,  wgpu::TextureFormat::ETC2RGBA8UnormSrgb,
    wgpu::TextureFormat::EACR11Unorm,     wgpu::TextureFormat::EACR11Snorm,
    wgpu::TextureFormat::EACRG11Unorm,    wgpu::TextureFormat::EACRG11Snorm};

static constexpr std::array<wgpu::TextureFormat, 28> kASTCFormats = {
    wgpu::TextureFormat::ASTC4x4Unorm,   wgpu::TextureFormat::ASTC4x4UnormSrgb,
    wgpu::TextureFormat::ASTC5x4Unorm,   wgpu::TextureFormat::ASTC5x4UnormSrgb,
    wgpu::TextureFormat::ASTC5x5Unorm,   wgpu::TextureFormat::ASTC5x5UnormSrgb,
    wgpu::TextureFormat::ASTC6x5Unorm,   wgpu::TextureFormat::ASTC6x5UnormSrgb,
    wgpu::TextureFormat::ASTC6x6Unorm,   wgpu::TextureFormat::ASTC6x6UnormSrgb,
    wgpu::TextureFormat::ASTC8x5Unorm,   wgpu::TextureFormat::ASTC8x5UnormSrgb,
    wgpu::TextureFormat::ASTC8x6Unorm,   wgpu::TextureFormat::ASTC8x6UnormSrgb,
    wgpu::TextureFormat::ASTC8x8Unorm,   wgpu::TextureFormat::ASTC8x8UnormSrgb,
    wgpu::TextureFormat::ASTC10x5Unorm,  wgpu::TextureFormat::ASTC10x5UnormSrgb,
    wgpu::TextureFormat::ASTC10x6Unorm,  wgpu::TextureFormat::ASTC10x6UnormSrgb,
    wgpu::TextureFormat::ASTC10x8Unorm,  wgpu::TextureFormat::ASTC10x8UnormSrgb,
    wgpu::TextureFormat::ASTC10x10Unorm, wgpu::TextureFormat::ASTC10x10UnormSrgb,
    wgpu::TextureFormat::ASTC12x10Unorm, wgpu::TextureFormat::ASTC12x10UnormSrgb,
    wgpu::TextureFormat::ASTC12x12Unorm, wgpu::TextureFormat::ASTC12x12UnormSrgb,
};

static constexpr std::array<wgpu::TextureFormat, 52> kCompressedFormats = {
    wgpu::TextureFormat::BC1RGBAUnorm,    wgpu::TextureFormat::BC1RGBAUnormSrgb,
    wgpu::TextureFormat::BC2RGBAUnorm,    wgpu::TextureFormat::BC2RGBAUnormSrgb,
    wgpu::TextureFormat::BC3RGBAUnorm,    wgpu::TextureFormat::BC3RGBAUnormSrgb,
    wgpu::TextureFormat::BC4RUnorm,       wgpu::TextureFormat::BC4RSnorm,
    wgpu::TextureFormat::BC5RGUnorm,      wgpu::TextureFormat::BC5RGSnorm,
    wgpu::TextureFormat::BC6HRGBUfloat,   wgpu::TextureFormat::BC6HRGBFloat,
    wgpu::TextureFormat::BC7RGBAUnorm,    wgpu::TextureFormat::BC7RGBAUnormSrgb,
    wgpu::TextureFormat::ETC2RGB8Unorm,   wgpu::TextureFormat::ETC2RGB8UnormSrgb,
    wgpu::TextureFormat::ETC2RGB8A1Unorm, wgpu::TextureFormat::ETC2RGB8A1UnormSrgb,
    wgpu::TextureFormat::ETC2RGBA8Unorm,  wgpu::TextureFormat::ETC2RGBA8UnormSrgb,
    wgpu::TextureFormat::EACR11Unorm,     wgpu::TextureFormat::EACR11Snorm,
    wgpu::TextureFormat::EACRG11Unorm,    wgpu::TextureFormat::EACRG11Snorm,
    wgpu::TextureFormat::ASTC4x4Unorm,    wgpu::TextureFormat::ASTC4x4UnormSrgb,
    wgpu::TextureFormat::ASTC5x4Unorm,    wgpu::TextureFormat::ASTC5x4UnormSrgb,
    wgpu::TextureFormat::ASTC5x5Unorm,    wgpu::TextureFormat::ASTC5x5UnormSrgb,
    wgpu::TextureFormat::ASTC6x5Unorm,    wgpu::TextureFormat::ASTC6x5UnormSrgb,
    wgpu::TextureFormat::ASTC6x6Unorm,    wgpu::TextureFormat::ASTC6x6UnormSrgb,
    wgpu::TextureFormat::ASTC8x5Unorm,    wgpu::TextureFormat::ASTC8x5UnormSrgb,
    wgpu::TextureFormat::ASTC8x6Unorm,    wgpu::TextureFormat::ASTC8x6UnormSrgb,
    wgpu::TextureFormat::ASTC8x8Unorm,    wgpu::TextureFormat::ASTC8x8UnormSrgb,
    wgpu::TextureFormat::ASTC10x5Unorm,   wgpu::TextureFormat::ASTC10x5UnormSrgb,
    wgpu::TextureFormat::ASTC10x6Unorm,   wgpu::TextureFormat::ASTC10x6UnormSrgb,
    wgpu::TextureFormat::ASTC10x8Unorm,   wgpu::TextureFormat::ASTC10x8UnormSrgb,
    wgpu::TextureFormat::ASTC10x10Unorm,  wgpu::TextureFormat::ASTC10x10UnormSrgb,
    wgpu::TextureFormat::ASTC12x10Unorm,  wgpu::TextureFormat::ASTC12x10UnormSrgb,
    wgpu::TextureFormat::ASTC12x12Unorm,  wgpu::TextureFormat::ASTC12x12UnormSrgb};
static_assert(kCompressedFormats.size() ==
                  kBCFormats.size() + kETC2Formats.size() + kASTCFormats.size(),
              "Number of compressed format must equal number of BC, ETC2, and ASTC formats.");

#ifndef __EMSCRIPTEN__
static constexpr std::array<wgpu::TextureFormat, 6> kNorm16Formats = {
    wgpu::TextureFormat::R16Unorm, wgpu::TextureFormat::RG16Unorm, wgpu::TextureFormat::RGBA16Unorm,
    wgpu::TextureFormat::R16Snorm, wgpu::TextureFormat::RG16Snorm, wgpu::TextureFormat::RGBA16Snorm,
};
#endif  // __EMSCRIPTEN__

static constexpr std::array<wgpu::TextureFormat, 5> kDepthFormats = {
    wgpu::TextureFormat::Depth16Unorm,         wgpu::TextureFormat::Depth32Float,
    wgpu::TextureFormat::Depth24Plus,          wgpu::TextureFormat::Depth24PlusStencil8,
    wgpu::TextureFormat::Depth32FloatStencil8,
};
static constexpr std::array<wgpu::TextureFormat, 3> kStencilFormats = {
    wgpu::TextureFormat::Depth24PlusStencil8,
    wgpu::TextureFormat::Depth32FloatStencil8,
    wgpu::TextureFormat::Stencil8,
};
static constexpr std::array<wgpu::TextureFormat, 2> kDepthAndStencilFormats = {
    wgpu::TextureFormat::Depth24PlusStencil8,
    wgpu::TextureFormat::Depth32FloatStencil8,
};

class SubsamplingFactor {
  public:
    constexpr SubsamplingFactor(uint32_t horizontal, uint32_t vertical)
        : horizontalFactor(horizontal), verticalFactor(vertical) {}

    SubsamplingFactor(const SubsamplingFactor&) = delete;
    SubsamplingFactor& operator=(const SubsamplingFactor&) = delete;

    const uint32_t horizontalFactor = 1;
    const uint32_t verticalFactor = 1;
};

bool TextureFormatSupportsStorageTexture(wgpu::TextureFormat format,
                                         const wgpu::Device& device,
                                         bool isCompatibilityMode);
bool TextureFormatSupportsReadWriteStorageTexture(wgpu::TextureFormat format);

bool IsBCTextureFormat(wgpu::TextureFormat textureFormat);
bool IsETC2TextureFormat(wgpu::TextureFormat textureFormat);
bool IsASTCTextureFormat(wgpu::TextureFormat textureFormat);
bool IsCompressedTextureFormat(wgpu::TextureFormat textureFormat);

bool IsDepthOnlyFormat(wgpu::TextureFormat textureFormat);
bool IsStencilOnlyFormat(wgpu::TextureFormat textureFormat);
bool IsDepthOrStencilFormat(wgpu::TextureFormat textureFormat);

bool IsRenderableFormat(const wgpu::Device& device, wgpu::TextureFormat textureFormat);

bool TextureFormatSupportsMultisampling(const wgpu::Device& device,
                                        wgpu::TextureFormat textureFormat,
                                        bool isCompatibilityMode);
bool TextureFormatSupportsResolveTarget(const wgpu::Device& device,
                                        wgpu::TextureFormat textureFormat);

#ifndef __EMSCRIPTEN__
bool IsUnorm16TextureFormat(wgpu::TextureFormat textureFormat);
bool IsSnorm16TextureFormat(wgpu::TextureFormat textureFormat);

bool IsMultiPlanarFormat(wgpu::TextureFormat textureFormat);
uint32_t GetMultiPlaneTextureBitDepth(wgpu::TextureFormat textureFormat);
uint32_t GetMultiPlaneTextureNumPlanes(wgpu::TextureFormat textureFormat);
uint32_t GetMultiPlaneTextureBytesPerElement(wgpu::TextureFormat textureFormat, size_t plane);
SubsamplingFactor GetMultiPlaneTextureSubsamplingFactor(wgpu::TextureFormat textureFormat,
                                                        size_t plane);
#endif  // __EMSCRIPTEN__

uint32_t GetTexelBlockSizeInBytes(wgpu::TextureFormat textureFormat);
uint32_t GetTextureFormatBlockWidth(wgpu::TextureFormat textureFormat);
uint32_t GetTextureFormatBlockHeight(wgpu::TextureFormat textureFormat);

const char* GetWGSLColorTextureComponentType(wgpu::TextureFormat textureFormat);
const char* GetWGSLImageFormatQualifier(wgpu::TextureFormat textureFormat);
uint32_t GetTextureComponentCount(wgpu::TextureFormat textureFormat);

wgpu::TextureDimension ViewDimensionToTextureDimension(const wgpu::TextureViewDimension dimension);
}  // namespace dawn::utils

#endif  // SRC_DAWN_UTILS_TEXTUREUTILS_H_
