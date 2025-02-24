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

#include "dawn/utils/TextureUtils.h"

namespace dawn::utils {
bool TextureFormatSupportsStorageTexture(wgpu::TextureFormat format,
                                         const wgpu::Device& device,
                                         bool isCompatibilityMode) {
    switch (format) {
        case wgpu::TextureFormat::R32Uint:
        case wgpu::TextureFormat::R32Sint:
        case wgpu::TextureFormat::R32Float:
        case wgpu::TextureFormat::RGBA8Unorm:
        case wgpu::TextureFormat::RGBA8Snorm:
        case wgpu::TextureFormat::RGBA8Uint:
        case wgpu::TextureFormat::RGBA8Sint:
        case wgpu::TextureFormat::RGBA16Uint:
        case wgpu::TextureFormat::RGBA16Sint:
        case wgpu::TextureFormat::RGBA16Float:
        case wgpu::TextureFormat::RGBA32Uint:
        case wgpu::TextureFormat::RGBA32Sint:
        case wgpu::TextureFormat::RGBA32Float:
            return true;
            // 32-bit RG* formats are unsupported in Compatibility mode.
        case wgpu::TextureFormat::RG32Uint:
        case wgpu::TextureFormat::RG32Sint:
        case wgpu::TextureFormat::RG32Float:
            return !isCompatibilityMode;
        case wgpu::TextureFormat::BGRA8Unorm:
            return device.HasFeature(wgpu::FeatureName::BGRA8UnormStorage);

        default:
            return false;
    }
}

bool IsBCTextureFormat(wgpu::TextureFormat textureFormat) {
    switch (textureFormat) {
        case wgpu::TextureFormat::BC1RGBAUnorm:
        case wgpu::TextureFormat::BC1RGBAUnormSrgb:
        case wgpu::TextureFormat::BC4RUnorm:
        case wgpu::TextureFormat::BC4RSnorm:
        case wgpu::TextureFormat::BC2RGBAUnorm:
        case wgpu::TextureFormat::BC2RGBAUnormSrgb:
        case wgpu::TextureFormat::BC3RGBAUnorm:
        case wgpu::TextureFormat::BC3RGBAUnormSrgb:
        case wgpu::TextureFormat::BC5RGUnorm:
        case wgpu::TextureFormat::BC5RGSnorm:
        case wgpu::TextureFormat::BC6HRGBUfloat:
        case wgpu::TextureFormat::BC6HRGBFloat:
        case wgpu::TextureFormat::BC7RGBAUnorm:
        case wgpu::TextureFormat::BC7RGBAUnormSrgb:
            return true;

        default:
            return false;
    }
}

bool IsETC2TextureFormat(wgpu::TextureFormat textureFormat) {
    switch (textureFormat) {
        case wgpu::TextureFormat::ETC2RGB8Unorm:
        case wgpu::TextureFormat::ETC2RGB8UnormSrgb:
        case wgpu::TextureFormat::ETC2RGB8A1Unorm:
        case wgpu::TextureFormat::ETC2RGB8A1UnormSrgb:
        case wgpu::TextureFormat::EACR11Unorm:
        case wgpu::TextureFormat::EACR11Snorm:
        case wgpu::TextureFormat::ETC2RGBA8Unorm:
        case wgpu::TextureFormat::ETC2RGBA8UnormSrgb:
        case wgpu::TextureFormat::EACRG11Unorm:
        case wgpu::TextureFormat::EACRG11Snorm:
            return true;

        default:
            return false;
    }
}

bool IsASTCTextureFormat(wgpu::TextureFormat textureFormat) {
    switch (textureFormat) {
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
            return true;

        default:
            return false;
    }
}

bool IsCompressedTextureFormat(wgpu::TextureFormat textureFormat) {
    return IsASTCTextureFormat(textureFormat) || IsBCTextureFormat(textureFormat) ||
           IsETC2TextureFormat(textureFormat);
}

bool IsDepthOnlyFormat(wgpu::TextureFormat textureFormat) {
    switch (textureFormat) {
        case wgpu::TextureFormat::Depth16Unorm:
        case wgpu::TextureFormat::Depth24Plus:
        case wgpu::TextureFormat::Depth32Float:
            return true;
        default:
            return false;
    }
}

bool IsDepthOrStencilFormat(wgpu::TextureFormat textureFormat) {
    switch (textureFormat) {
        case wgpu::TextureFormat::Depth16Unorm:
        case wgpu::TextureFormat::Depth24Plus:
        case wgpu::TextureFormat::Depth32Float:
        case wgpu::TextureFormat::Depth24PlusStencil8:
        case wgpu::TextureFormat::Depth32FloatStencil8:
        case wgpu::TextureFormat::Stencil8:
            return true;
        default:
            return false;
    }
}

bool IsRenderableFormat(const wgpu::Device& device, wgpu::TextureFormat textureFormat) {
    if (IsBCTextureFormat(textureFormat) || IsETC2TextureFormat(textureFormat) ||
        IsASTCTextureFormat(textureFormat)) {
        return false;
    }

#ifndef __EMSCRIPTEN__
    if (IsUnorm16TextureFormat(textureFormat)) {
        return device.HasFeature(wgpu::FeatureName::Unorm16TextureFormats);
    }

    if (IsSnorm16TextureFormat(textureFormat)) {
        return device.HasFeature(wgpu::FeatureName::Snorm16TextureFormats);
    }
#endif  // __EMSCRIPTEN__

    switch (textureFormat) {
        case wgpu::TextureFormat::RGB9E5Ufloat:
        case wgpu::TextureFormat::R8Snorm:
        case wgpu::TextureFormat::RG8Snorm:
        case wgpu::TextureFormat::RGBA8Snorm:
            return false;

        case wgpu::TextureFormat::RG11B10Ufloat:
            return device.HasFeature(wgpu::FeatureName::RG11B10UfloatRenderable);

        default:
            return true;
    }
}

bool TextureFormatSupportsMultisampling(const wgpu::Device& device,
                                        wgpu::TextureFormat textureFormat,
                                        bool isCompatibilityMode) {
    if (IsBCTextureFormat(textureFormat) || IsETC2TextureFormat(textureFormat) ||
        IsASTCTextureFormat(textureFormat)) {
        return false;
    }

#ifndef __EMSCRIPTEN__
    if (IsUnorm16TextureFormat(textureFormat)) {
        return device.HasFeature(wgpu::FeatureName::Unorm16TextureFormats);
    }

    if (IsSnorm16TextureFormat(textureFormat)) {
        return device.HasFeature(wgpu::FeatureName::Snorm16TextureFormats);
    }
#endif  // __EMSCRIPTEN__

    switch (textureFormat) {
        case wgpu::TextureFormat::R32Uint:
        case wgpu::TextureFormat::R32Sint:
        case wgpu::TextureFormat::RG32Uint:
        case wgpu::TextureFormat::RG32Sint:
        case wgpu::TextureFormat::RG32Float:
        case wgpu::TextureFormat::RGBA32Uint:
        case wgpu::TextureFormat::RGBA32Sint:
        case wgpu::TextureFormat::RGBA32Float:
        case wgpu::TextureFormat::RGB9E5Ufloat:
        case wgpu::TextureFormat::R8Snorm:
        case wgpu::TextureFormat::RG8Snorm:
        case wgpu::TextureFormat::RGBA8Snorm:
            return false;

        case wgpu::TextureFormat::RG11B10Ufloat:
            return device.HasFeature(wgpu::FeatureName::RG11B10UfloatRenderable);

        case wgpu::TextureFormat::RGBA16Float:
        case wgpu::TextureFormat::R32Float:
            return !isCompatibilityMode;

        default:
            return true;
    }
}

bool TextureFormatSupportsResolveTarget(const wgpu::Device& device,
                                        wgpu::TextureFormat textureFormat) {
    switch (textureFormat) {
        case wgpu::TextureFormat::R8Unorm:
        case wgpu::TextureFormat::RG8Unorm:
        case wgpu::TextureFormat::RGBA8Unorm:
        case wgpu::TextureFormat::RGBA8UnormSrgb:
        case wgpu::TextureFormat::BGRA8Unorm:
        case wgpu::TextureFormat::BGRA8UnormSrgb:
        case wgpu::TextureFormat::R16Float:
        case wgpu::TextureFormat::RG16Float:
        case wgpu::TextureFormat::RGBA16Float:
        case wgpu::TextureFormat::RGB10A2Unorm:
            return true;

#ifndef __EMSCRIPTEN__
        case wgpu::TextureFormat::R16Unorm:
        case wgpu::TextureFormat::R16Snorm:
        case wgpu::TextureFormat::RG16Unorm:
        case wgpu::TextureFormat::RG16Snorm:
        case wgpu::TextureFormat::RGBA16Unorm:
        case wgpu::TextureFormat::RGBA16Snorm:
            return device.HasFeature(wgpu::FeatureName::Unorm16TextureFormats);
#endif  // __EMSCRIPTEN__

        default:
            return false;
    }
}

bool TextureFormatSupportsReadWriteStorageTexture(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::R32Float:
        case wgpu::TextureFormat::R32Sint:
        case wgpu::TextureFormat::R32Uint:
            return true;
        default:
            return false;
    }
}

bool IsStencilOnlyFormat(wgpu::TextureFormat textureFormat) {
    return textureFormat == wgpu::TextureFormat::Stencil8;
}

#ifndef __EMSCRIPTEN__
bool IsUnorm16TextureFormat(wgpu::TextureFormat textureFormat) {
    switch (textureFormat) {
        case wgpu::TextureFormat::R16Unorm:
        case wgpu::TextureFormat::RG16Unorm:
        case wgpu::TextureFormat::RGBA16Unorm:
            return true;
        default:
            return false;
    }
}

bool IsSnorm16TextureFormat(wgpu::TextureFormat textureFormat) {
    switch (textureFormat) {
        case wgpu::TextureFormat::R16Snorm:
        case wgpu::TextureFormat::RG16Snorm:
        case wgpu::TextureFormat::RGBA16Snorm:
            return true;
        default:
            return false;
    }
}

bool IsMultiPlanarFormat(wgpu::TextureFormat textureFormat) {
    switch (textureFormat) {
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar422Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar444Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm:
        case wgpu::TextureFormat::R8BG8A8Triplanar420Unorm:
            return true;
        default:
            return false;
    }
}

uint32_t GetMultiPlaneTextureBitDepth(wgpu::TextureFormat textureFormat) {
    switch (textureFormat) {
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar422Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar444Unorm:
        case wgpu::TextureFormat::R8BG8A8Triplanar420Unorm:
            return 8;
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm:
            return 16;
        default:
            DAWN_UNREACHABLE();
            return 0;
    }
}

uint32_t GetMultiPlaneTextureNumPlanes(wgpu::TextureFormat textureFormat) {
    switch (textureFormat) {
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar422Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar444Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm:
            return 2;
        case wgpu::TextureFormat::R8BG8A8Triplanar420Unorm:
            return 3;
        default:
            DAWN_UNREACHABLE();
            return 0;
    }
}

uint32_t GetMultiPlaneTextureBytesPerElement(wgpu::TextureFormat textureFormat, size_t plane) {
    switch (textureFormat) {
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar422Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar444Unorm:
        case wgpu::TextureFormat::R8BG8A8Triplanar420Unorm:
            return plane == 1 ? 2 : 1;
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm:
            return plane == 1 ? 4 : 2;
        default:
            DAWN_UNREACHABLE();
            return 0;
    }
}

SubsamplingFactor GetMultiPlaneTextureSubsamplingFactor(wgpu::TextureFormat textureFormat,
                                                        size_t plane) {
    switch (textureFormat) {
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R8BG8A8Triplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
            if (plane == 1) {
                return {2, 2};
            }
            return {1, 1};
        case wgpu::TextureFormat::R8BG8Biplanar422Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm:
            if (plane == 1) {
                return {2, 1};
            }
            return {1, 1};
        case wgpu::TextureFormat::R8BG8Biplanar444Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm:
            return {1, 1};
        default:
            DAWN_UNREACHABLE();
            return {0, 0};
    }
}
#endif  // __EMSCRIPTEN__

uint32_t GetTexelBlockSizeInBytes(wgpu::TextureFormat textureFormat) {
    switch (textureFormat) {
        case wgpu::TextureFormat::R8Unorm:
        case wgpu::TextureFormat::R8Snorm:
        case wgpu::TextureFormat::R8Uint:
        case wgpu::TextureFormat::R8Sint:
        case wgpu::TextureFormat::Stencil8:
            return 1u;

        case wgpu::TextureFormat::R16Uint:
        case wgpu::TextureFormat::R16Sint:
        case wgpu::TextureFormat::R16Float:
        case wgpu::TextureFormat::RG8Unorm:
        case wgpu::TextureFormat::RG8Snorm:
        case wgpu::TextureFormat::RG8Uint:
        case wgpu::TextureFormat::RG8Sint:
            return 2u;

        case wgpu::TextureFormat::R32Float:
        case wgpu::TextureFormat::R32Uint:
        case wgpu::TextureFormat::R32Sint:
        case wgpu::TextureFormat::RG16Uint:
        case wgpu::TextureFormat::RG16Sint:
        case wgpu::TextureFormat::RG16Float:
        case wgpu::TextureFormat::RGBA8Unorm:
        case wgpu::TextureFormat::RGBA8UnormSrgb:
        case wgpu::TextureFormat::RGBA8Snorm:
        case wgpu::TextureFormat::RGBA8Uint:
        case wgpu::TextureFormat::RGBA8Sint:
        case wgpu::TextureFormat::BGRA8Unorm:
        case wgpu::TextureFormat::BGRA8UnormSrgb:
        case wgpu::TextureFormat::RGB10A2Uint:
        case wgpu::TextureFormat::RGB10A2Unorm:
        case wgpu::TextureFormat::RG11B10Ufloat:
        case wgpu::TextureFormat::RGB9E5Ufloat:
            return 4u;

        case wgpu::TextureFormat::RG32Float:
        case wgpu::TextureFormat::RG32Uint:
        case wgpu::TextureFormat::RG32Sint:
        case wgpu::TextureFormat::RGBA16Uint:
        case wgpu::TextureFormat::RGBA16Sint:
        case wgpu::TextureFormat::RGBA16Float:
            return 8u;

        case wgpu::TextureFormat::RGBA32Float:
        case wgpu::TextureFormat::RGBA32Uint:
        case wgpu::TextureFormat::RGBA32Sint:
            return 16u;

        case wgpu::TextureFormat::Depth16Unorm:
            return 2u;

        case wgpu::TextureFormat::Depth24Plus:
        case wgpu::TextureFormat::Depth32Float:
            return 4u;

        case wgpu::TextureFormat::BC1RGBAUnorm:
        case wgpu::TextureFormat::BC1RGBAUnormSrgb:
        case wgpu::TextureFormat::BC4RUnorm:
        case wgpu::TextureFormat::BC4RSnorm:
            return 8u;

        case wgpu::TextureFormat::BC2RGBAUnorm:
        case wgpu::TextureFormat::BC2RGBAUnormSrgb:
        case wgpu::TextureFormat::BC3RGBAUnorm:
        case wgpu::TextureFormat::BC3RGBAUnormSrgb:
        case wgpu::TextureFormat::BC5RGUnorm:
        case wgpu::TextureFormat::BC5RGSnorm:
        case wgpu::TextureFormat::BC6HRGBUfloat:
        case wgpu::TextureFormat::BC6HRGBFloat:
        case wgpu::TextureFormat::BC7RGBAUnorm:
        case wgpu::TextureFormat::BC7RGBAUnormSrgb:
            return 16u;

        case wgpu::TextureFormat::ETC2RGB8Unorm:
        case wgpu::TextureFormat::ETC2RGB8UnormSrgb:
        case wgpu::TextureFormat::ETC2RGB8A1Unorm:
        case wgpu::TextureFormat::ETC2RGB8A1UnormSrgb:
        case wgpu::TextureFormat::EACR11Unorm:
        case wgpu::TextureFormat::EACR11Snorm:
            return 8u;

        case wgpu::TextureFormat::ETC2RGBA8Unorm:
        case wgpu::TextureFormat::ETC2RGBA8UnormSrgb:
        case wgpu::TextureFormat::EACRG11Unorm:
        case wgpu::TextureFormat::EACRG11Snorm:
            return 16u;

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
            return 16u;

        case wgpu::TextureFormat::Depth24PlusStencil8:
        case wgpu::TextureFormat::Depth32FloatStencil8:
            break;

#ifndef __EMSCRIPTEN__
        // Unorm and Snorm 16 formats.
        case wgpu::TextureFormat::R16Unorm:
        case wgpu::TextureFormat::R16Snorm:
            return 2u;
        case wgpu::TextureFormat::RG16Unorm:
        case wgpu::TextureFormat::RG16Snorm:
            return 4u;
        case wgpu::TextureFormat::RGBA16Unorm:
        case wgpu::TextureFormat::RGBA16Snorm:
            return 8u;

        // Block size of a multi-planar format depends on aspect.
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar422Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar444Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm:
        case wgpu::TextureFormat::R8BG8A8Triplanar420Unorm:
        case wgpu::TextureFormat::External:
#endif  // __EMSCRIPTEN__

        case wgpu::TextureFormat::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

uint32_t GetTextureFormatBlockWidth(wgpu::TextureFormat textureFormat) {
    switch (textureFormat) {
        case wgpu::TextureFormat::R8Unorm:
        case wgpu::TextureFormat::R8Snorm:
        case wgpu::TextureFormat::R8Uint:
        case wgpu::TextureFormat::R8Sint:
        case wgpu::TextureFormat::R16Uint:
        case wgpu::TextureFormat::R16Sint:
        case wgpu::TextureFormat::R16Float:
        case wgpu::TextureFormat::RG8Unorm:
        case wgpu::TextureFormat::RG8Snorm:
        case wgpu::TextureFormat::RG8Uint:
        case wgpu::TextureFormat::RG8Sint:
        case wgpu::TextureFormat::R32Float:
        case wgpu::TextureFormat::R32Uint:
        case wgpu::TextureFormat::R32Sint:
        case wgpu::TextureFormat::RG16Uint:
        case wgpu::TextureFormat::RG16Sint:
        case wgpu::TextureFormat::RG16Float:
        case wgpu::TextureFormat::RGBA8Unorm:
        case wgpu::TextureFormat::RGBA8UnormSrgb:
        case wgpu::TextureFormat::RGBA8Snorm:
        case wgpu::TextureFormat::RGBA8Uint:
        case wgpu::TextureFormat::RGBA8Sint:
        case wgpu::TextureFormat::BGRA8Unorm:
        case wgpu::TextureFormat::BGRA8UnormSrgb:
        case wgpu::TextureFormat::RGB10A2Uint:
        case wgpu::TextureFormat::RGB10A2Unorm:
        case wgpu::TextureFormat::RG11B10Ufloat:
        case wgpu::TextureFormat::RGB9E5Ufloat:
        case wgpu::TextureFormat::RG32Float:
        case wgpu::TextureFormat::RG32Uint:
        case wgpu::TextureFormat::RG32Sint:
        case wgpu::TextureFormat::RGBA16Uint:
        case wgpu::TextureFormat::RGBA16Sint:
        case wgpu::TextureFormat::RGBA16Float:
        case wgpu::TextureFormat::RGBA32Float:
        case wgpu::TextureFormat::RGBA32Uint:
        case wgpu::TextureFormat::RGBA32Sint:
        case wgpu::TextureFormat::Depth32Float:
        case wgpu::TextureFormat::Depth24Plus:
        case wgpu::TextureFormat::Depth24PlusStencil8:
        case wgpu::TextureFormat::Depth16Unorm:
        case wgpu::TextureFormat::Depth32FloatStencil8:
        case wgpu::TextureFormat::Stencil8:
            return 1u;

        case wgpu::TextureFormat::BC1RGBAUnorm:
        case wgpu::TextureFormat::BC1RGBAUnormSrgb:
        case wgpu::TextureFormat::BC4RUnorm:
        case wgpu::TextureFormat::BC4RSnorm:
        case wgpu::TextureFormat::BC2RGBAUnorm:
        case wgpu::TextureFormat::BC2RGBAUnormSrgb:
        case wgpu::TextureFormat::BC3RGBAUnorm:
        case wgpu::TextureFormat::BC3RGBAUnormSrgb:
        case wgpu::TextureFormat::BC5RGUnorm:
        case wgpu::TextureFormat::BC5RGSnorm:
        case wgpu::TextureFormat::BC6HRGBUfloat:
        case wgpu::TextureFormat::BC6HRGBFloat:
        case wgpu::TextureFormat::BC7RGBAUnorm:
        case wgpu::TextureFormat::BC7RGBAUnormSrgb:
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
            return 4u;

        case wgpu::TextureFormat::ASTC4x4Unorm:
        case wgpu::TextureFormat::ASTC4x4UnormSrgb:
            return 4u;
        case wgpu::TextureFormat::ASTC5x4Unorm:
        case wgpu::TextureFormat::ASTC5x4UnormSrgb:
        case wgpu::TextureFormat::ASTC5x5Unorm:
        case wgpu::TextureFormat::ASTC5x5UnormSrgb:
            return 5u;
        case wgpu::TextureFormat::ASTC6x5Unorm:
        case wgpu::TextureFormat::ASTC6x5UnormSrgb:
        case wgpu::TextureFormat::ASTC6x6Unorm:
        case wgpu::TextureFormat::ASTC6x6UnormSrgb:
            return 6u;
        case wgpu::TextureFormat::ASTC8x5Unorm:
        case wgpu::TextureFormat::ASTC8x5UnormSrgb:
        case wgpu::TextureFormat::ASTC8x6Unorm:
        case wgpu::TextureFormat::ASTC8x6UnormSrgb:
        case wgpu::TextureFormat::ASTC8x8Unorm:
        case wgpu::TextureFormat::ASTC8x8UnormSrgb:
            return 8u;
        case wgpu::TextureFormat::ASTC10x5Unorm:
        case wgpu::TextureFormat::ASTC10x5UnormSrgb:
        case wgpu::TextureFormat::ASTC10x6Unorm:
        case wgpu::TextureFormat::ASTC10x6UnormSrgb:
        case wgpu::TextureFormat::ASTC10x8Unorm:
        case wgpu::TextureFormat::ASTC10x8UnormSrgb:
        case wgpu::TextureFormat::ASTC10x10Unorm:
        case wgpu::TextureFormat::ASTC10x10UnormSrgb:
            return 10u;
        case wgpu::TextureFormat::ASTC12x10Unorm:
        case wgpu::TextureFormat::ASTC12x10UnormSrgb:
        case wgpu::TextureFormat::ASTC12x12Unorm:
        case wgpu::TextureFormat::ASTC12x12UnormSrgb:
            return 12u;

#ifndef __EMSCRIPTEN__
        // Unorm and Snorm 16 formats.
        case wgpu::TextureFormat::R16Unorm:
        case wgpu::TextureFormat::R16Snorm:
        case wgpu::TextureFormat::RG16Unorm:
        case wgpu::TextureFormat::RG16Snorm:
        case wgpu::TextureFormat::RGBA16Unorm:
        case wgpu::TextureFormat::RGBA16Snorm:
            return 1u;

        // Block size of a multi-planar format depends on aspect.
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar422Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar444Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm:
        case wgpu::TextureFormat::R8BG8A8Triplanar420Unorm:
        case wgpu::TextureFormat::External:
#endif  // __EMSCRIPTEN__

        case wgpu::TextureFormat::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

uint32_t GetTextureFormatBlockHeight(wgpu::TextureFormat textureFormat) {
    switch (textureFormat) {
        case wgpu::TextureFormat::R8Unorm:
        case wgpu::TextureFormat::R8Snorm:
        case wgpu::TextureFormat::R8Uint:
        case wgpu::TextureFormat::R8Sint:
        case wgpu::TextureFormat::R16Uint:
        case wgpu::TextureFormat::R16Sint:
        case wgpu::TextureFormat::R16Float:
        case wgpu::TextureFormat::RG8Unorm:
        case wgpu::TextureFormat::RG8Snorm:
        case wgpu::TextureFormat::RG8Uint:
        case wgpu::TextureFormat::RG8Sint:
        case wgpu::TextureFormat::R32Float:
        case wgpu::TextureFormat::R32Uint:
        case wgpu::TextureFormat::R32Sint:
        case wgpu::TextureFormat::RG16Uint:
        case wgpu::TextureFormat::RG16Sint:
        case wgpu::TextureFormat::RG16Float:
        case wgpu::TextureFormat::RGBA8Unorm:
        case wgpu::TextureFormat::RGBA8UnormSrgb:
        case wgpu::TextureFormat::RGBA8Snorm:
        case wgpu::TextureFormat::RGBA8Uint:
        case wgpu::TextureFormat::RGBA8Sint:
        case wgpu::TextureFormat::BGRA8Unorm:
        case wgpu::TextureFormat::BGRA8UnormSrgb:
        case wgpu::TextureFormat::RGB10A2Uint:
        case wgpu::TextureFormat::RGB10A2Unorm:
        case wgpu::TextureFormat::RG11B10Ufloat:
        case wgpu::TextureFormat::RGB9E5Ufloat:
        case wgpu::TextureFormat::RG32Float:
        case wgpu::TextureFormat::RG32Uint:
        case wgpu::TextureFormat::RG32Sint:
        case wgpu::TextureFormat::RGBA16Uint:
        case wgpu::TextureFormat::RGBA16Sint:
        case wgpu::TextureFormat::RGBA16Float:
        case wgpu::TextureFormat::RGBA32Float:
        case wgpu::TextureFormat::RGBA32Uint:
        case wgpu::TextureFormat::RGBA32Sint:
        case wgpu::TextureFormat::Depth32Float:
        case wgpu::TextureFormat::Depth24Plus:
        case wgpu::TextureFormat::Depth24PlusStencil8:
        case wgpu::TextureFormat::Depth16Unorm:
        case wgpu::TextureFormat::Depth32FloatStencil8:
        case wgpu::TextureFormat::Stencil8:
            return 1u;

        case wgpu::TextureFormat::BC1RGBAUnorm:
        case wgpu::TextureFormat::BC1RGBAUnormSrgb:
        case wgpu::TextureFormat::BC4RUnorm:
        case wgpu::TextureFormat::BC4RSnorm:
        case wgpu::TextureFormat::BC2RGBAUnorm:
        case wgpu::TextureFormat::BC2RGBAUnormSrgb:
        case wgpu::TextureFormat::BC3RGBAUnorm:
        case wgpu::TextureFormat::BC3RGBAUnormSrgb:
        case wgpu::TextureFormat::BC5RGUnorm:
        case wgpu::TextureFormat::BC5RGSnorm:
        case wgpu::TextureFormat::BC6HRGBUfloat:
        case wgpu::TextureFormat::BC6HRGBFloat:
        case wgpu::TextureFormat::BC7RGBAUnorm:
        case wgpu::TextureFormat::BC7RGBAUnormSrgb:
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
            return 4u;

        case wgpu::TextureFormat::ASTC4x4Unorm:
        case wgpu::TextureFormat::ASTC4x4UnormSrgb:
        case wgpu::TextureFormat::ASTC5x4Unorm:
        case wgpu::TextureFormat::ASTC5x4UnormSrgb:
            return 4u;
        case wgpu::TextureFormat::ASTC5x5Unorm:
        case wgpu::TextureFormat::ASTC5x5UnormSrgb:
        case wgpu::TextureFormat::ASTC6x5Unorm:
        case wgpu::TextureFormat::ASTC6x5UnormSrgb:
        case wgpu::TextureFormat::ASTC8x5Unorm:
        case wgpu::TextureFormat::ASTC8x5UnormSrgb:
        case wgpu::TextureFormat::ASTC10x5Unorm:
        case wgpu::TextureFormat::ASTC10x5UnormSrgb:
            return 5u;
        case wgpu::TextureFormat::ASTC6x6Unorm:
        case wgpu::TextureFormat::ASTC6x6UnormSrgb:
        case wgpu::TextureFormat::ASTC8x6Unorm:
        case wgpu::TextureFormat::ASTC8x6UnormSrgb:
        case wgpu::TextureFormat::ASTC10x6Unorm:
        case wgpu::TextureFormat::ASTC10x6UnormSrgb:
            return 6u;
        case wgpu::TextureFormat::ASTC8x8Unorm:
        case wgpu::TextureFormat::ASTC8x8UnormSrgb:
        case wgpu::TextureFormat::ASTC10x8Unorm:
        case wgpu::TextureFormat::ASTC10x8UnormSrgb:
            return 8u;
        case wgpu::TextureFormat::ASTC10x10Unorm:
        case wgpu::TextureFormat::ASTC10x10UnormSrgb:
        case wgpu::TextureFormat::ASTC12x10Unorm:
        case wgpu::TextureFormat::ASTC12x10UnormSrgb:
            return 10u;
        case wgpu::TextureFormat::ASTC12x12Unorm:
        case wgpu::TextureFormat::ASTC12x12UnormSrgb:
            return 12u;

#ifndef __EMSCRIPTEN__
        // Unorm and Snorm 16 formats.
        case wgpu::TextureFormat::R16Unorm:
        case wgpu::TextureFormat::R16Snorm:
        case wgpu::TextureFormat::RG16Unorm:
        case wgpu::TextureFormat::RG16Snorm:
        case wgpu::TextureFormat::RGBA16Unorm:
        case wgpu::TextureFormat::RGBA16Snorm:
            return 1u;

        // Block size of a multi-planar format depends on aspect.
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar422Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar444Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm:
        case wgpu::TextureFormat::R8BG8A8Triplanar420Unorm:
        case wgpu::TextureFormat::External:
#endif  // __EMSCRIPTEN__

        case wgpu::TextureFormat::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

const char* GetWGSLColorTextureComponentType(wgpu::TextureFormat textureFormat) {
    switch (textureFormat) {
        case wgpu::TextureFormat::R8Unorm:
        case wgpu::TextureFormat::R8Snorm:
        case wgpu::TextureFormat::R16Float:
        case wgpu::TextureFormat::RG8Unorm:
        case wgpu::TextureFormat::RG8Snorm:
        case wgpu::TextureFormat::R32Float:
        case wgpu::TextureFormat::RG16Float:
        case wgpu::TextureFormat::RGBA8Unorm:
        case wgpu::TextureFormat::RGBA8Snorm:
        case wgpu::TextureFormat::RGB10A2Unorm:
        case wgpu::TextureFormat::RG11B10Ufloat:
        case wgpu::TextureFormat::RGB9E5Ufloat:
        case wgpu::TextureFormat::RG32Float:
        case wgpu::TextureFormat::RGBA16Float:
        case wgpu::TextureFormat::RGBA32Float:
        case wgpu::TextureFormat::BGRA8Unorm:
        case wgpu::TextureFormat::BGRA8UnormSrgb:
        case wgpu::TextureFormat::RGBA8UnormSrgb:
            return "f32";

        case wgpu::TextureFormat::R8Uint:
        case wgpu::TextureFormat::R16Uint:
        case wgpu::TextureFormat::RG8Uint:
        case wgpu::TextureFormat::R32Uint:
        case wgpu::TextureFormat::RG16Uint:
        case wgpu::TextureFormat::RGBA8Uint:
        case wgpu::TextureFormat::RG32Uint:
        case wgpu::TextureFormat::RGB10A2Uint:
        case wgpu::TextureFormat::RGBA16Uint:
        case wgpu::TextureFormat::RGBA32Uint:
            return "u32";

        case wgpu::TextureFormat::R8Sint:
        case wgpu::TextureFormat::R16Sint:
        case wgpu::TextureFormat::RG8Sint:
        case wgpu::TextureFormat::R32Sint:
        case wgpu::TextureFormat::RG16Sint:
        case wgpu::TextureFormat::RGBA8Sint:
        case wgpu::TextureFormat::RG32Sint:
        case wgpu::TextureFormat::RGBA16Sint:
        case wgpu::TextureFormat::RGBA32Sint:
            return "i32";

#ifndef __EMSCRIPTEN__
        // Unorm and Snorm 16 formats.
        case wgpu::TextureFormat::R16Unorm:
        case wgpu::TextureFormat::R16Snorm:
        case wgpu::TextureFormat::RG16Unorm:
        case wgpu::TextureFormat::RG16Snorm:
        case wgpu::TextureFormat::RGBA16Unorm:
        case wgpu::TextureFormat::RGBA16Snorm:
            return "f32";
#endif  // __EMSCRIPTEN__

        default:
            DAWN_UNREACHABLE();
    }
}

uint32_t GetTextureComponentCount(wgpu::TextureFormat textureFormat) {
    switch (textureFormat) {
        case wgpu::TextureFormat::R8Unorm:
        case wgpu::TextureFormat::R8Snorm:
        case wgpu::TextureFormat::R8Uint:
        case wgpu::TextureFormat::R8Sint:
        case wgpu::TextureFormat::R16Uint:
        case wgpu::TextureFormat::R16Sint:
        case wgpu::TextureFormat::R16Float:
        case wgpu::TextureFormat::R32Float:
        case wgpu::TextureFormat::R32Uint:
        case wgpu::TextureFormat::R32Sint:
        case wgpu::TextureFormat::Depth16Unorm:
        case wgpu::TextureFormat::Depth24Plus:
        case wgpu::TextureFormat::Depth32Float:
        case wgpu::TextureFormat::Stencil8:
            return 1u;
        case wgpu::TextureFormat::RG8Unorm:
        case wgpu::TextureFormat::RG8Snorm:
        case wgpu::TextureFormat::RG8Uint:
        case wgpu::TextureFormat::RG8Sint:
        case wgpu::TextureFormat::RG16Uint:
        case wgpu::TextureFormat::RG16Sint:
        case wgpu::TextureFormat::RG16Float:
        case wgpu::TextureFormat::RG32Float:
        case wgpu::TextureFormat::RG32Uint:
        case wgpu::TextureFormat::RG32Sint:
            return 2u;
        case wgpu::TextureFormat::RGB9E5Ufloat:
        case wgpu::TextureFormat::RG11B10Ufloat:
            return 3u;
        case wgpu::TextureFormat::RGBA8Unorm:
        case wgpu::TextureFormat::RGBA8UnormSrgb:
        case wgpu::TextureFormat::RGBA8Snorm:
        case wgpu::TextureFormat::RGBA8Uint:
        case wgpu::TextureFormat::RGBA8Sint:
        case wgpu::TextureFormat::BGRA8Unorm:
        case wgpu::TextureFormat::BGRA8UnormSrgb:
        case wgpu::TextureFormat::RGB10A2Uint:
        case wgpu::TextureFormat::RGB10A2Unorm:
        case wgpu::TextureFormat::RGBA16Uint:
        case wgpu::TextureFormat::RGBA16Sint:
        case wgpu::TextureFormat::RGBA16Float:
        case wgpu::TextureFormat::RGBA32Float:
        case wgpu::TextureFormat::RGBA32Uint:
        case wgpu::TextureFormat::RGBA32Sint:
            return 4u;

#ifndef __EMSCRIPTEN__
        // Unorm and Snorm 16 formats.
        case wgpu::TextureFormat::R16Unorm:
        case wgpu::TextureFormat::R16Snorm:
            return 1u;
        case wgpu::TextureFormat::RG16Unorm:
        case wgpu::TextureFormat::RG16Snorm:
            return 2u;
        case wgpu::TextureFormat::RGBA16Unorm:
        case wgpu::TextureFormat::RGBA16Snorm:
            return 4u;
#endif  // __EMSCRIPTEN__

        default:
            // Compressed foramts, depth-stencil combined formats, multi-planar formats are
            // unreachable.
            break;
    }
    DAWN_UNREACHABLE();
}

const char* GetWGSLImageFormatQualifier(wgpu::TextureFormat textureFormat) {
    switch (textureFormat) {
        case wgpu::TextureFormat::RGBA8Unorm:
            return "rgba8unorm";
        case wgpu::TextureFormat::RGBA8Snorm:
            return "rgba8snorm";
        case wgpu::TextureFormat::RGBA8Uint:
            return "rgba8uint";
        case wgpu::TextureFormat::RGBA8Sint:
            return "rgba8sint";
        case wgpu::TextureFormat::BGRA8Unorm:
            return "bgra8unorm";
        case wgpu::TextureFormat::RGBA16Uint:
            return "rgba16uint";
        case wgpu::TextureFormat::RGBA16Sint:
            return "rgba16sint";
        case wgpu::TextureFormat::RGBA16Float:
            return "rgba16float";
        case wgpu::TextureFormat::R32Uint:
            return "r32uint";
        case wgpu::TextureFormat::R32Sint:
            return "r32sint";
        case wgpu::TextureFormat::R32Float:
            return "r32float";
        case wgpu::TextureFormat::RG32Uint:
            return "rg32uint";
        case wgpu::TextureFormat::RG32Sint:
            return "rg32sint";
        case wgpu::TextureFormat::RG32Float:
            return "rg32float";
        case wgpu::TextureFormat::RGBA32Uint:
            return "rgba32uint";
        case wgpu::TextureFormat::RGBA32Sint:
            return "rgba32sint";
        case wgpu::TextureFormat::RGBA32Float:
            return "rgba32float";
        // For Chromium Internal Graphite
        case wgpu::TextureFormat::R8Unorm:
            return "r8unorm";

#ifndef __EMSCRIPTEN__
        // Unorm and Snorm 16 formats.
        case wgpu::TextureFormat::RGBA16Unorm:
            return "rgba16unorm";
        case wgpu::TextureFormat::RGBA16Snorm:
            return "rgba16snorm";
#endif  // __EMSCRIPTEN__

        default:
            DAWN_UNREACHABLE();
    }
}

wgpu::TextureDimension ViewDimensionToTextureDimension(const wgpu::TextureViewDimension dimension) {
    switch (dimension) {
        case wgpu::TextureViewDimension::e2D:
        case wgpu::TextureViewDimension::e2DArray:
        case wgpu::TextureViewDimension::Cube:
        case wgpu::TextureViewDimension::CubeArray:
            return wgpu::TextureDimension::e2D;
        case wgpu::TextureViewDimension::e3D:
            return wgpu::TextureDimension::e3D;
        case wgpu::TextureViewDimension::e1D:
            return wgpu::TextureDimension::e1D;
        default:
            DAWN_UNREACHABLE();
            break;
    }
}

}  // namespace dawn::utils
