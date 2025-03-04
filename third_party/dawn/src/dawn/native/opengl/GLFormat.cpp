// Copyright 2019 The Dawn & Tint Authors
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

#include "dawn/native/opengl/GLFormat.h"

namespace dawn::native::opengl {

namespace {
GLenum GetBGRAInternalFormat(const OpenGLFunctions& gl) {
    if (gl.IsGLExtensionSupported("GL_EXT_texture_format_BGRA8888") ||
        gl.IsGLExtensionSupported("GL_APPLE_texture_format_BGRA8888")) {
        return GL_BGRA8_EXT;
    } else {
        // Desktop GL will swizzle to/from RGBA8 for BGRA formats.
        return GL_RGBA8;
    }
}

GLenum GetStencil8InternalFormat(const OpenGLFunctions& gl) {
    if (gl.GetVersion().IsDesktop() || gl.IsAtLeastGLES(3, 2) ||
        gl.IsGLExtensionSupported("GL_OES_texture_stencil8")) {
        return GL_STENCIL_INDEX8;
    }
    return GL_DEPTH24_STENCIL8;
}

bool FormatSupportsTexStorage(const OpenGLFunctions& gl, GLenum internalFormat) {
    if (internalFormat == GL_BGRA8_EXT && gl.GetVersion().IsES() &&
        !gl.IsGLExtensionSupported("GL_EXT_texture_storage")) {
        // GL_BGRA8_EXT is only valid for glTextureStorage if GL_EXT_texture_storage is present. The
        // core glTextureStorage added in ES 3.0 does not add this format.
        return false;
    }

    return true;
}

}  // namespace

GLFormatTable BuildGLFormatTable(const OpenGLFunctions& gl) {
    GLFormatTable table;

    using Type = GLFormat::ComponentType;

    auto AddFormat = [&table, &gl](wgpu::TextureFormat dawnFormat, GLenum internalFormat,
                                   GLenum format, GLenum type, Type componentType) {
        FormatIndex index = ComputeFormatIndex(dawnFormat);
        DAWN_ASSERT(index < table.size());

        table[index].internalFormat = internalFormat;
        table[index].format = format;
        table[index].type = type;
        table[index].componentType = componentType;
        table[index].isSupportedOnBackend = true;
        table[index].isSupportedForTextureStorage = FormatSupportsTexStorage(gl, internalFormat);
    };

    // It's dangerous to go alone, take this:
    //
    //     [ANGLE's formatutils.cpp]
    //     [ANGLE's formatutilsgl.cpp]
    //
    // The format tables in these files are extremely complete and the best reference on GL
    // format support, enums, etc.

    // clang-format off

    // 1 byte color formats
    AddFormat(wgpu::TextureFormat::R8Unorm, GL_R8, GL_RED, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::R8Snorm, GL_R8_SNORM, GL_RED, GL_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::R8Uint, GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE, Type::Uint);
    AddFormat(wgpu::TextureFormat::R8Sint, GL_R8I, GL_RED_INTEGER, GL_BYTE, Type::Int);

    // 2 bytes color formats
    AddFormat(wgpu::TextureFormat::R16Unorm, GL_R16, GL_RED, GL_UNSIGNED_SHORT, Type::Float);
    AddFormat(wgpu::TextureFormat::R16Snorm, GL_R16_SNORM, GL_RED, GL_SHORT, Type::Float);
    AddFormat(wgpu::TextureFormat::R16Uint, GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT, Type::Uint);
    AddFormat(wgpu::TextureFormat::R16Sint, GL_R16I, GL_RED_INTEGER, GL_SHORT, Type::Int);
    AddFormat(wgpu::TextureFormat::R16Float, GL_R16F, GL_RED, GL_HALF_FLOAT, Type::Float);
    AddFormat(wgpu::TextureFormat::RG8Unorm, GL_RG8, GL_RG, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::RG8Snorm, GL_RG8_SNORM, GL_RG, GL_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::RG8Uint, GL_RG8UI, GL_RG_INTEGER, GL_UNSIGNED_BYTE, Type::Uint);
    AddFormat(wgpu::TextureFormat::RG8Sint, GL_RG8I, GL_RG_INTEGER, GL_BYTE, Type::Int);

    // 4 bytes color formats
    AddFormat(wgpu::TextureFormat::R32Uint, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, Type::Uint);
    AddFormat(wgpu::TextureFormat::R32Sint, GL_R32I, GL_RED_INTEGER, GL_INT, Type::Int);
    AddFormat(wgpu::TextureFormat::R32Float, GL_R32F, GL_RED, GL_FLOAT, Type::Float);
    AddFormat(wgpu::TextureFormat::RG16Unorm, GL_RG16, GL_RG, GL_UNSIGNED_SHORT, Type::Float);
    AddFormat(wgpu::TextureFormat::RG16Snorm, GL_RG16_SNORM, GL_RG, GL_SHORT, Type::Float);
    AddFormat(wgpu::TextureFormat::RG16Uint, GL_RG16UI, GL_RG_INTEGER, GL_UNSIGNED_SHORT, Type::Uint);
    AddFormat(wgpu::TextureFormat::RG16Sint, GL_RG16I, GL_RG_INTEGER, GL_SHORT, Type::Int);
    AddFormat(wgpu::TextureFormat::RG16Float, GL_RG16F, GL_RG, GL_HALF_FLOAT, Type::Float);
    AddFormat(wgpu::TextureFormat::RGBA8Unorm, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::RGBA8UnormSrgb, GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::RGBA8Snorm, GL_RGBA8_SNORM, GL_RGBA, GL_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::RGBA8Uint, GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, Type::Uint);
    AddFormat(wgpu::TextureFormat::RGBA8Sint, GL_RGBA8I, GL_RGBA_INTEGER, GL_BYTE, Type::Int);

    AddFormat(wgpu::TextureFormat::BGRA8Unorm, GetBGRAInternalFormat(gl), GL_BGRA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::RGB10A2Uint, GL_RGB10_A2UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT_2_10_10_10_REV, Type::Uint);
    AddFormat(wgpu::TextureFormat::RGB10A2Unorm, GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV, Type::Float);
    AddFormat(wgpu::TextureFormat::RG11B10Ufloat, GL_R11F_G11F_B10F, GL_RGB, GL_UNSIGNED_INT_10F_11F_11F_REV, Type::Float);
    AddFormat(wgpu::TextureFormat::RGB9E5Ufloat, GL_RGB9_E5, GL_RGB, GL_UNSIGNED_INT_5_9_9_9_REV, Type::Float);

    // 8 bytes color formats
    AddFormat(wgpu::TextureFormat::RG32Uint, GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT, Type::Uint);
    AddFormat(wgpu::TextureFormat::RG32Sint, GL_RG32I, GL_RG_INTEGER, GL_INT, Type::Int);
    AddFormat(wgpu::TextureFormat::RG32Float, GL_RG32F, GL_RG, GL_FLOAT, Type::Float);
    AddFormat(wgpu::TextureFormat::RGBA16Unorm, GL_RGBA16, GL_RGBA, GL_UNSIGNED_SHORT, Type::Float);
    AddFormat(wgpu::TextureFormat::RGBA16Snorm, GL_RGBA16_SNORM, GL_RGBA, GL_SHORT, Type::Float);
    AddFormat(wgpu::TextureFormat::RGBA16Uint, GL_RGBA16UI, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT, Type::Uint);
    AddFormat(wgpu::TextureFormat::RGBA16Sint, GL_RGBA16I, GL_RGBA_INTEGER, GL_SHORT, Type::Int);
    AddFormat(wgpu::TextureFormat::RGBA16Float, GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, Type::Float);

    // 16 bytes color formats
    AddFormat(wgpu::TextureFormat::RGBA32Uint, GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT, Type::Uint);
    AddFormat(wgpu::TextureFormat::RGBA32Sint, GL_RGBA32I, GL_RGBA_INTEGER, GL_INT, Type::Int);
    AddFormat(wgpu::TextureFormat::RGBA32Float, GL_RGBA32F, GL_RGBA, GL_FLOAT, Type::Float);

    // Depth stencil formats
    AddFormat(wgpu::TextureFormat::Depth32Float, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, Type::DepthStencil);
    AddFormat(wgpu::TextureFormat::Depth24Plus, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, Type::DepthStencil);
    AddFormat(wgpu::TextureFormat::Depth24PlusStencil8, GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, Type::DepthStencil);
    AddFormat(wgpu::TextureFormat::Depth16Unorm, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, Type::DepthStencil);

    // Internal format for stencil8 can be either GL_STENCIL_INDEX8 or GL_DEPTH24_STENCIL8
    GLenum internalFormatForStencil8 = GetStencil8InternalFormat(gl);
    DAWN_ASSERT(internalFormatForStencil8 == GL_STENCIL_INDEX8 || internalFormatForStencil8 == GL_DEPTH24_STENCIL8);
    bool useStencilIndex8 = internalFormatForStencil8 == GL_STENCIL_INDEX8;
    AddFormat(wgpu::TextureFormat::Stencil8, internalFormatForStencil8, useStencilIndex8 ? GL_STENCIL : GL_DEPTH_STENCIL, useStencilIndex8 ? GL_UNSIGNED_BYTE : GL_UNSIGNED_INT_24_8, Type::DepthStencil);

    // Block compressed formats
    AddFormat(wgpu::TextureFormat::BC1RGBAUnorm, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::BC1RGBAUnormSrgb, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::BC2RGBAUnorm, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::BC2RGBAUnormSrgb, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::BC3RGBAUnorm, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::BC3RGBAUnormSrgb, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::BC4RSnorm, GL_COMPRESSED_SIGNED_RED_RGTC1, GL_RED, GL_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::BC4RUnorm, GL_COMPRESSED_RED_RGTC1, GL_RED, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::BC5RGSnorm, GL_COMPRESSED_SIGNED_RG_RGTC2, GL_RG, GL_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::BC5RGUnorm, GL_COMPRESSED_RG_RGTC2, GL_RG, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::BC6HRGBFloat, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT, GL_RGB, GL_HALF_FLOAT, Type::Float);
    AddFormat(wgpu::TextureFormat::BC6HRGBUfloat, GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT, GL_RGB, GL_HALF_FLOAT, Type::Float);
    AddFormat(wgpu::TextureFormat::BC7RGBAUnorm, GL_COMPRESSED_RGBA_BPTC_UNORM, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::BC7RGBAUnormSrgb, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);

    // ASTC compressed formats
    AddFormat(wgpu::TextureFormat::ASTC4x4Unorm, GL_COMPRESSED_RGBA_ASTC_4x4, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC5x4Unorm, GL_COMPRESSED_RGBA_ASTC_5x4, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC5x5Unorm, GL_COMPRESSED_RGBA_ASTC_5x5, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC6x5Unorm, GL_COMPRESSED_RGBA_ASTC_6x5, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC6x6Unorm, GL_COMPRESSED_RGBA_ASTC_6x6, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC8x5Unorm, GL_COMPRESSED_RGBA_ASTC_8x5, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC8x6Unorm, GL_COMPRESSED_RGBA_ASTC_8x6, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC8x8Unorm, GL_COMPRESSED_RGBA_ASTC_8x8, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC10x5Unorm, GL_COMPRESSED_RGBA_ASTC_10x5, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC10x6Unorm, GL_COMPRESSED_RGBA_ASTC_10x6, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC10x8Unorm, GL_COMPRESSED_RGBA_ASTC_10x8, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC10x10Unorm, GL_COMPRESSED_RGBA_ASTC_10x10, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC12x10Unorm, GL_COMPRESSED_RGBA_ASTC_12x10, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC12x12Unorm, GL_COMPRESSED_RGBA_ASTC_12x12, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);

    AddFormat(wgpu::TextureFormat::ASTC4x4UnormSrgb, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC5x4UnormSrgb, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC5x5UnormSrgb, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC6x5UnormSrgb, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC6x6UnormSrgb, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC8x5UnormSrgb, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC8x6UnormSrgb, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC8x8UnormSrgb, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC10x5UnormSrgb, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC10x6UnormSrgb, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC10x8UnormSrgb, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC10x10UnormSrgb, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC12x10UnormSrgb, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ASTC12x12UnormSrgb, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);

    // ETC compressed formats
    AddFormat(wgpu::TextureFormat::EACR11Unorm, GL_COMPRESSED_R11_EAC, GL_RED, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::EACR11Snorm, GL_COMPRESSED_SIGNED_R11_EAC, GL_RED, GL_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::EACRG11Unorm, GL_COMPRESSED_RG11_EAC, GL_RG, GL_UNSIGNED_SHORT, Type::Float);
    AddFormat(wgpu::TextureFormat::EACRG11Snorm, GL_COMPRESSED_SIGNED_RG11_EAC, GL_RG, GL_SHORT, Type::Float);
    AddFormat(wgpu::TextureFormat::ETC2RGB8Unorm, GL_COMPRESSED_RGB8_ETC2, GL_RGB, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ETC2RGB8UnormSrgb, GL_COMPRESSED_SRGB8_ETC2, GL_RGB, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ETC2RGB8A1Unorm, GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ETC2RGB8A1UnormSrgb, GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ETC2RGBA8Unorm, GL_COMPRESSED_RGBA8_ETC2_EAC, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
    AddFormat(wgpu::TextureFormat::ETC2RGBA8UnormSrgb, GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);

    // clang-format on

    return table;
}

}  // namespace dawn::native::opengl
