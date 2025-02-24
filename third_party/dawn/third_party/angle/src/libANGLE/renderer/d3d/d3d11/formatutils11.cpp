//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// formatutils11.cpp: Queries for GL image formats and their translations to D3D11
// formats.

#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"

#include "image_util/copyimage.h"
#include "image_util/generatemip.h"
#include "image_util/loadimage.h"

#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/copyvertex.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"
#include "libANGLE/renderer/d3d/d3d11/texture_format_table.h"
#include "libANGLE/renderer/dxgi_support_table.h"

namespace rx
{

namespace d3d11
{

bool SupportsMipGen(DXGI_FORMAT dxgiFormat, D3D_FEATURE_LEVEL featureLevel)
{
    const auto &support = GetDXGISupport(dxgiFormat, featureLevel);
    ASSERT((support.optionallySupportedFlags & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN) == 0);
    return ((support.alwaysSupportedFlags & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN) != 0);
}

bool IsSupportedMultiplanarFormat(DXGI_FORMAT dxgiFormat)
{
    return dxgiFormat == DXGI_FORMAT_NV12 || dxgiFormat == DXGI_FORMAT_P010 ||
           dxgiFormat == DXGI_FORMAT_P016;
}

const Format &GetYUVPlaneFormat(DXGI_FORMAT dxgiFormat, int plane)
{
    static constexpr Format nv12Plane0Info(
        GL_R8, angle::FormatID::R8_UNORM, DXGI_FORMAT_NV12, DXGI_FORMAT_R8_UNORM,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R8_UNORM,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R8_TYPELESS, GL_RGBA8, nullptr);

    static constexpr Format nv12Plane1Info(
        GL_RG8, angle::FormatID::R8G8_UNORM, DXGI_FORMAT_NV12, DXGI_FORMAT_R8G8_UNORM,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R8G8_UNORM,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R8G8_TYPELESS, GL_RGBA8, nullptr);

    static constexpr Format p010Plane0Info(
        GL_R16_EXT, angle::FormatID::R16_UNORM, DXGI_FORMAT_P010, DXGI_FORMAT_R16_UNORM,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R16_UNORM,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R16_TYPELESS, GL_RGBA16_EXT, nullptr);

    static constexpr Format p010Plane1Info(
        GL_RG16_EXT, angle::FormatID::R16G16_UNORM, DXGI_FORMAT_P010, DXGI_FORMAT_R16G16_UNORM,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_UNKNOWN,
        DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN,
        DXGI_FORMAT_R16G16_TYPELESS, GL_RGBA16_EXT, nullptr);

    static constexpr Format p016Plane0Info(
        GL_R16_EXT, angle::FormatID::R16_UNORM, DXGI_FORMAT_P016, DXGI_FORMAT_R16_UNORM,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R16_UNORM,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R16_TYPELESS, GL_RGBA16_EXT, nullptr);

    static constexpr Format p016Plane1Info(
        GL_RG16_EXT, angle::FormatID::R16G16_UNORM, DXGI_FORMAT_P016, DXGI_FORMAT_R16G16_UNORM,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_UNKNOWN,
        DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN,
        DXGI_FORMAT_R16G16_TYPELESS, GL_RGBA16_EXT, nullptr);

    ASSERT(IsSupportedMultiplanarFormat(dxgiFormat));
    if (plane < 0 || plane > 1)
    {
        ERR() << "Invalid client buffer texture plane: " << plane;
        static constexpr Format defaultInfo;
        return defaultInfo;
    }

    switch (dxgiFormat)
    {
        case DXGI_FORMAT_NV12:
            return plane == 0 ? nv12Plane0Info : nv12Plane1Info;
        case DXGI_FORMAT_P010:
            return plane == 0 ? p010Plane0Info : p010Plane1Info;
        case DXGI_FORMAT_P016:
            return plane == 0 ? p016Plane0Info : p016Plane1Info;
        default:
            ERR() << "Not supported multiplanar format: " << dxgiFormat;
    }
    static constexpr Format defaultInfo;
    return defaultInfo;
}

DXGIFormatSize::DXGIFormatSize(GLuint pixelBits, GLuint blockWidth, GLuint blockHeight)
    : pixelBytes(pixelBits / 8), blockWidth(blockWidth), blockHeight(blockHeight)
{}

const DXGIFormatSize &GetDXGIFormatSizeInfo(DXGI_FORMAT format)
{
    static const DXGIFormatSize sizeUnknown(0, 0, 0);
    static const DXGIFormatSize size128(128, 1, 1);
    static const DXGIFormatSize size96(96, 1, 1);
    static const DXGIFormatSize size64(64, 1, 1);
    static const DXGIFormatSize size32(32, 1, 1);
    static const DXGIFormatSize size16(16, 1, 1);
    static const DXGIFormatSize size8(8, 1, 1);
    static const DXGIFormatSize sizeBC1(64, 4, 4);
    static const DXGIFormatSize sizeBC2(128, 4, 4);
    static const DXGIFormatSize sizeBC3(128, 4, 4);
    static const DXGIFormatSize sizeBC4(64, 4, 4);
    static const DXGIFormatSize sizeBC5(128, 4, 4);
    static const DXGIFormatSize sizeBC6H(128, 4, 4);
    static const DXGIFormatSize sizeBC7(128, 4, 4);
    switch (format)
    {
        case DXGI_FORMAT_UNKNOWN:
            return sizeUnknown;
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
            return size128;
        case DXGI_FORMAT_R32G32B32_TYPELESS:
        case DXGI_FORMAT_R32G32B32_FLOAT:
        case DXGI_FORMAT_R32G32B32_UINT:
        case DXGI_FORMAT_R32G32B32_SINT:
            return size96;
        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        case DXGI_FORMAT_R32G32_TYPELESS:
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_R32G32_UINT:
        case DXGI_FORMAT_R32G32_SINT:
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            return size64;
        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UINT:
        case DXGI_FORMAT_R11G11B10_FLOAT:
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
        case DXGI_FORMAT_R16G16_TYPELESS:
        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_R16G16_UINT:
        case DXGI_FORMAT_R16G16_SNORM:
        case DXGI_FORMAT_R16G16_SINT:
        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT:
        case DXGI_FORMAT_R32_FLOAT:
        case DXGI_FORMAT_R32_UINT:
        case DXGI_FORMAT_R32_SINT:
        case DXGI_FORMAT_R24G8_TYPELESS:
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
            return size32;
        case DXGI_FORMAT_R8G8_TYPELESS:
        case DXGI_FORMAT_R8G8_UNORM:
        case DXGI_FORMAT_R8G8_UINT:
        case DXGI_FORMAT_R8G8_SNORM:
        case DXGI_FORMAT_R8G8_SINT:
        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_R16_FLOAT:
        case DXGI_FORMAT_D16_UNORM:
        case DXGI_FORMAT_R16_UNORM:
        case DXGI_FORMAT_R16_UINT:
        case DXGI_FORMAT_R16_SNORM:
        case DXGI_FORMAT_R16_SINT:
            return size16;
        case DXGI_FORMAT_R8_TYPELESS:
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8_UINT:
        case DXGI_FORMAT_R8_SNORM:
        case DXGI_FORMAT_R8_SINT:
        case DXGI_FORMAT_A8_UNORM:
            return size8;
        case DXGI_FORMAT_R1_UNORM:
            UNREACHABLE();
            return sizeUnknown;
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
            return size32;
        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
            return sizeBC1;
        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
            return sizeBC2;
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
            return sizeBC3;
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM:
            return sizeBC4;
        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
            return sizeBC5;
        case DXGI_FORMAT_B5G6R5_UNORM:
        case DXGI_FORMAT_B5G5R5A1_UNORM:
            return size16;
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B8G8R8X8_UNORM:
        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
            return size32;
        case DXGI_FORMAT_BC6H_TYPELESS:
        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC6H_SF16:
            return sizeBC6H;
        case DXGI_FORMAT_BC7_TYPELESS:
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            return sizeBC7;
        case DXGI_FORMAT_AYUV:
        case DXGI_FORMAT_Y410:
        case DXGI_FORMAT_Y416:
        case DXGI_FORMAT_NV12:
        case DXGI_FORMAT_P010:
        case DXGI_FORMAT_P016:
        case DXGI_FORMAT_420_OPAQUE:
        case DXGI_FORMAT_YUY2:
        case DXGI_FORMAT_Y210:
        case DXGI_FORMAT_Y216:
        case DXGI_FORMAT_NV11:
        case DXGI_FORMAT_AI44:
        case DXGI_FORMAT_IA44:
        case DXGI_FORMAT_P8:
        case DXGI_FORMAT_A8P8:
            UNREACHABLE();
            return sizeUnknown;
        case DXGI_FORMAT_B4G4R4A4_UNORM:
            return size16;
        default:
            UNREACHABLE();
            return sizeUnknown;
    }
}

constexpr VertexFormat::VertexFormat()
    : conversionType(VERTEX_CONVERT_NONE), nativeFormat(DXGI_FORMAT_UNKNOWN), copyFunction(nullptr)
{}

constexpr VertexFormat::VertexFormat(VertexConversionType conversionTypeIn,
                                     DXGI_FORMAT nativeFormatIn,
                                     VertexCopyFunction copyFunctionIn)
    : conversionType(conversionTypeIn), nativeFormat(nativeFormatIn), copyFunction(copyFunctionIn)
{}

const VertexFormat *GetVertexFormatInfo_FL_9_3(angle::FormatID vertexFormatID)
{
    // D3D11 Feature Level 9_3 doesn't support as many formats for vertex buffer resource as Feature
    // Level 10_0+.
    // http://msdn.microsoft.com/en-us/library/windows/desktop/ff471324(v=vs.85).aspx

    switch (vertexFormatID)
    {
        // GL_BYTE -- unnormalized
        case angle::FormatID::R8_SSCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_BOTH, DXGI_FORMAT_R16G16_SINT,
                                               &Copy8SintTo16SintVertexData<1, 2>);
            return &info;
        }
        case angle::FormatID::R8G8_SSCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_BOTH, DXGI_FORMAT_R16G16_SINT,
                                               &Copy8SintTo16SintVertexData<2, 2>);
            return &info;
        }
        case angle::FormatID::R8G8B8_SSCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_BOTH, DXGI_FORMAT_R16G16B16A16_SINT,
                                               &Copy8SintTo16SintVertexData<3, 4>);
            return &info;
        }
        case angle::FormatID::R8G8B8A8_SSCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_BOTH, DXGI_FORMAT_R16G16B16A16_SINT,
                                               &Copy8SintTo16SintVertexData<4, 4>);
            return &info;
        }

        // GL_BYTE -- normalized
        case angle::FormatID::R8_SNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R16G16_SNORM,
                                               &Copy8SnormTo16SnormVertexData<1, 2>);
            return &info;
        }
        case angle::FormatID::R8G8_SNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R16G16_SNORM,
                                               &Copy8SnormTo16SnormVertexData<2, 2>);
            return &info;
        }
        case angle::FormatID::R8G8B8_SNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R16G16B16A16_SNORM,
                                               &Copy8SnormTo16SnormVertexData<3, 4>);
            return &info;
        }
        case angle::FormatID::R8G8B8A8_SNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R16G16B16A16_SNORM,
                                               &Copy8SnormTo16SnormVertexData<4, 4>);
            return &info;
        }

        // GL_UNSIGNED_BYTE -- un-normalized
        // NOTE: 3 and 4 component unnormalized GL_UNSIGNED_BYTE should use the default format
        // table.
        case angle::FormatID::R8_USCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_BOTH, DXGI_FORMAT_R8G8B8A8_UINT,
                                               &CopyNativeVertexData<GLubyte, 1, 4, 1>);
            return &info;
        }
        case angle::FormatID::R8G8_USCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_BOTH, DXGI_FORMAT_R8G8B8A8_UINT,
                                               &CopyNativeVertexData<GLubyte, 2, 4, 1>);
            return &info;
        }

        // GL_UNSIGNED_BYTE -- normalized
        // NOTE: 3 and 4 component normalized GL_UNSIGNED_BYTE should use the default format table.

        // GL_UNSIGNED_BYTE -- normalized
        case angle::FormatID::R8_UNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R8G8B8A8_UNORM,
                                               &CopyNativeVertexData<GLubyte, 1, 4, UINT8_MAX>);
            return &info;
        }
        case angle::FormatID::R8G8_UNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R8G8B8A8_UNORM,
                                               &CopyNativeVertexData<GLubyte, 2, 4, UINT8_MAX>);
            return &info;
        }

        // GL_SHORT -- un-normalized
        // NOTE: 2, 3 and 4 component unnormalized GL_SHORT should use the default format table.
        case angle::FormatID::R16_SSCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_BOTH, DXGI_FORMAT_R16G16_SINT,
                                               &CopyNativeVertexData<GLshort, 1, 2, 0>);
            return &info;
        }

        // GL_SHORT -- normalized
        // NOTE: 2, 3 and 4 component normalized GL_SHORT should use the default format table.
        case angle::FormatID::R16_SNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R16G16_SNORM,
                                               &CopyNativeVertexData<GLshort, 1, 2, 0>);
            return &info;
        }

        // GL_UNSIGNED_SHORT -- un-normalized
        case angle::FormatID::R16_USCALED:
        {
            static constexpr VertexFormat info(
                VERTEX_CONVERT_CPU, DXGI_FORMAT_R32G32_FLOAT,
                &CopyToFloatVertexData<GLushort, 1, 2, false, false>);
            return &info;
        }
        case angle::FormatID::R16G16_USCALED:
        {
            static constexpr VertexFormat info(
                VERTEX_CONVERT_CPU, DXGI_FORMAT_R32G32_FLOAT,
                &CopyToFloatVertexData<GLushort, 2, 2, false, false>);
            return &info;
        }
        case angle::FormatID::R16G16B16_USCALED:
        {
            static constexpr VertexFormat info(
                VERTEX_CONVERT_CPU, DXGI_FORMAT_R32G32B32_FLOAT,
                &CopyToFloatVertexData<GLushort, 3, 3, false, false>);
            return &info;
        }
        case angle::FormatID::R16G16B16A16_USCALED:
        {
            static constexpr VertexFormat info(
                VERTEX_CONVERT_CPU, DXGI_FORMAT_R32G32B32A32_FLOAT,
                &CopyToFloatVertexData<GLushort, 4, 4, false, false>);
            return &info;
        }

        // GL_UNSIGNED_SHORT -- normalized
        case angle::FormatID::R16_UNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R32G32_FLOAT,
                                               &CopyToFloatVertexData<GLushort, 1, 2, true, false>);
            return &info;
        }
        case angle::FormatID::R16G16_UNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R32G32_FLOAT,
                                               &CopyToFloatVertexData<GLushort, 2, 2, true, false>);
            return &info;
        }
        case angle::FormatID::R16G16B16_UNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R32G32B32_FLOAT,
                                               &CopyToFloatVertexData<GLushort, 3, 3, true, false>);
            return &info;
        }
        case angle::FormatID::R16G16B16A16_UNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R32G32B32A32_FLOAT,
                                               &CopyToFloatVertexData<GLushort, 4, 4, true, false>);
            return &info;
        }

        // GL_FIXED
        // TODO: Add test to verify that this works correctly.
        // NOTE: 2, 3 and 4 component GL_FIXED should use the default format table.
        case angle::FormatID::R32_FIXED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R32G32_FLOAT,
                                               &Copy32FixedTo32FVertexData<1, 2>);
            return &info;
        }

        // GL_FLOAT
        // TODO: Add test to verify that this works correctly.
        // NOTE: 2, 3 and 4 component GL_FLOAT should use the default format table.
        case angle::FormatID::R32_FLOAT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R32G32_FLOAT,
                                               &CopyNativeVertexData<GLfloat, 1, 2, 0>);
            return &info;
        }

        default:
            return nullptr;
    }
}

const VertexFormat &GetVertexFormatInfo(angle::FormatID vertexFormatID,
                                        D3D_FEATURE_LEVEL featureLevel)
{
    if (featureLevel == D3D_FEATURE_LEVEL_9_3)
    {
        const VertexFormat *result = GetVertexFormatInfo_FL_9_3(vertexFormatID);
        if (result)
        {
            return *result;
        }
    }

    switch (vertexFormatID)
    {
        //
        // Float formats
        //

        // GL_BYTE -- un-normalized
        case angle::FormatID::R8_SSCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_GPU, DXGI_FORMAT_R8_SINT,
                                               &CopyNativeVertexData<GLbyte, 1, 1, 0>);
            return info;
        }
        case angle::FormatID::R8G8_SSCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_GPU, DXGI_FORMAT_R8G8_SINT,
                                               &CopyNativeVertexData<GLbyte, 2, 2, 0>);
            return info;
        }
        case angle::FormatID::R8G8B8_SSCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_BOTH, DXGI_FORMAT_R8G8B8A8_SINT,
                                               &CopyNativeVertexData<GLbyte, 3, 4, 1>);
            return info;
        }
        case angle::FormatID::R8G8B8A8_SSCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_GPU, DXGI_FORMAT_R8G8B8A8_SINT,
                                               &CopyNativeVertexData<GLbyte, 4, 4, 0>);
            return info;
        }

        // GL_BYTE -- normalized
        case angle::FormatID::R8_SNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R8_SNORM,
                                               &CopyNativeVertexData<GLbyte, 1, 1, 0>);
            return info;
        }
        case angle::FormatID::R8G8_SNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R8G8_SNORM,
                                               &CopyNativeVertexData<GLbyte, 2, 2, 0>);
            return info;
        }
        case angle::FormatID::R8G8B8_SNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R8G8B8A8_SNORM,
                                               &CopyNativeVertexData<GLbyte, 3, 4, INT8_MAX>);
            return info;
        }
        case angle::FormatID::R8G8B8A8_SNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R8G8B8A8_SNORM,
                                               &CopyNativeVertexData<GLbyte, 4, 4, 0>);
            return info;
        }

        // GL_UNSIGNED_BYTE -- un-normalized
        case angle::FormatID::R8_USCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_GPU, DXGI_FORMAT_R8_UINT,
                                               &CopyNativeVertexData<GLubyte, 1, 1, 0>);
            return info;
        }
        case angle::FormatID::R8G8_USCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_GPU, DXGI_FORMAT_R8G8_UINT,
                                               &CopyNativeVertexData<GLubyte, 2, 2, 0>);
            return info;
        }
        case angle::FormatID::R8G8B8_USCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_BOTH, DXGI_FORMAT_R8G8B8A8_UINT,
                                               &CopyNativeVertexData<GLubyte, 3, 4, 1>);
            return info;
        }
        case angle::FormatID::R8G8B8A8_USCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_GPU, DXGI_FORMAT_R8G8B8A8_UINT,
                                               &CopyNativeVertexData<GLubyte, 4, 4, 0>);
            return info;
        }

        // GL_UNSIGNED_BYTE -- normalized
        case angle::FormatID::R8_UNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R8_UNORM,
                                               &CopyNativeVertexData<GLubyte, 1, 1, 0>);
            return info;
        }
        case angle::FormatID::R8G8_UNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R8G8_UNORM,
                                               &CopyNativeVertexData<GLubyte, 2, 2, 0>);
            return info;
        }
        case angle::FormatID::R8G8B8_UNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R8G8B8A8_UNORM,
                                               &CopyNativeVertexData<GLubyte, 3, 4, UINT8_MAX>);
            return info;
        }
        case angle::FormatID::R8G8B8A8_UNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R8G8B8A8_UNORM,
                                               &CopyNativeVertexData<GLubyte, 4, 4, 0>);
            return info;
        }

        // GL_SHORT -- un-normalized
        case angle::FormatID::R16_SSCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_GPU, DXGI_FORMAT_R16_SINT,
                                               &CopyNativeVertexData<GLshort, 1, 1, 0>);
            return info;
        }
        case angle::FormatID::R16G16_SSCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_GPU, DXGI_FORMAT_R16G16_SINT,
                                               &CopyNativeVertexData<GLshort, 2, 2, 0>);
            return info;
        }
        case angle::FormatID::R16G16B16_SSCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_BOTH, DXGI_FORMAT_R16G16B16A16_SINT,
                                               &CopyNativeVertexData<GLshort, 3, 4, 1>);
            return info;
        }
        case angle::FormatID::R16G16B16A16_SSCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_GPU, DXGI_FORMAT_R16G16B16A16_SINT,
                                               &CopyNativeVertexData<GLshort, 4, 4, 0>);
            return info;
        }

        // GL_SHORT -- normalized
        case angle::FormatID::R16_SNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R16_SNORM,
                                               &CopyNativeVertexData<GLshort, 1, 1, 0>);
            return info;
        }
        case angle::FormatID::R16G16_SNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R16G16_SNORM,
                                               &CopyNativeVertexData<GLshort, 2, 2, 0>);
            return info;
        }
        case angle::FormatID::R16G16B16_SNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R16G16B16A16_SNORM,
                                               &CopyNativeVertexData<GLshort, 3, 4, INT16_MAX>);
            return info;
        }
        case angle::FormatID::R16G16B16A16_SNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R16G16B16A16_SNORM,
                                               &CopyNativeVertexData<GLshort, 4, 4, 0>);
            return info;
        }

        // GL_UNSIGNED_SHORT -- un-normalized
        case angle::FormatID::R16_USCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_GPU, DXGI_FORMAT_R16_UINT,
                                               &CopyNativeVertexData<GLushort, 1, 1, 0>);
            return info;
        }
        case angle::FormatID::R16G16_USCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_GPU, DXGI_FORMAT_R16G16_UINT,
                                               &CopyNativeVertexData<GLushort, 2, 2, 0>);
            return info;
        }
        case angle::FormatID::R16G16B16_USCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_BOTH, DXGI_FORMAT_R16G16B16A16_UINT,
                                               &CopyNativeVertexData<GLushort, 3, 4, 1>);
            return info;
        }
        case angle::FormatID::R16G16B16A16_USCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_GPU, DXGI_FORMAT_R16G16B16A16_UINT,
                                               &CopyNativeVertexData<GLushort, 4, 4, 0>);
            return info;
        }

        // GL_UNSIGNED_SHORT -- normalized
        case angle::FormatID::R16_UNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R16_UNORM,
                                               &CopyNativeVertexData<GLushort, 1, 1, 0>);
            return info;
        }
        case angle::FormatID::R16G16_UNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R16G16_UNORM,
                                               &CopyNativeVertexData<GLushort, 2, 2, 0>);
            return info;
        }
        case angle::FormatID::R16G16B16_UNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R16G16B16A16_UNORM,
                                               &CopyNativeVertexData<GLushort, 3, 4, UINT16_MAX>);
            return info;
        }
        case angle::FormatID::R16G16B16A16_UNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R16G16B16A16_UNORM,
                                               &CopyNativeVertexData<GLushort, 4, 4, 0>);
            return info;
        }

        // GL_INT -- un-normalized
        case angle::FormatID::R32_SSCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_GPU, DXGI_FORMAT_R32_SINT,
                                               &CopyNativeVertexData<GLint, 1, 1, 0>);
            return info;
        }
        case angle::FormatID::R32G32_SSCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_GPU, DXGI_FORMAT_R32G32_SINT,
                                               &CopyNativeVertexData<GLint, 2, 2, 0>);
            return info;
        }
        case angle::FormatID::R32G32B32_SSCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_GPU, DXGI_FORMAT_R32G32B32_SINT,
                                               &CopyNativeVertexData<GLint, 3, 3, 0>);
            return info;
        }
        case angle::FormatID::R32G32B32A32_SSCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_GPU, DXGI_FORMAT_R32G32B32A32_SINT,
                                               &CopyNativeVertexData<GLint, 4, 4, 0>);
            return info;
        }

        // GL_INT -- normalized
        case angle::FormatID::R32_SNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R32_FLOAT,
                                               &CopyToFloatVertexData<GLint, 1, 1, true, false>);
            return info;
        }
        case angle::FormatID::R32G32_SNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R32G32_FLOAT,
                                               &CopyToFloatVertexData<GLint, 2, 2, true, false>);
            return info;
        }
        case angle::FormatID::R32G32B32_SNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R32G32B32_FLOAT,
                                               &CopyToFloatVertexData<GLint, 3, 3, true, false>);
            return info;
        }
        case angle::FormatID::R32G32B32A32_SNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R32G32B32A32_FLOAT,
                                               &CopyToFloatVertexData<GLint, 4, 4, true, false>);
            return info;
        }

        // GL_UNSIGNED_INT -- un-normalized
        case angle::FormatID::R32_USCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_GPU, DXGI_FORMAT_R32_UINT,
                                               &CopyNativeVertexData<GLuint, 1, 1, 0>);
            return info;
        }
        case angle::FormatID::R32G32_USCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_GPU, DXGI_FORMAT_R32G32_UINT,
                                               &CopyNativeVertexData<GLuint, 2, 2, 0>);
            return info;
        }
        case angle::FormatID::R32G32B32_USCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_GPU, DXGI_FORMAT_R32G32B32_UINT,
                                               &CopyNativeVertexData<GLuint, 3, 3, 0>);
            return info;
        }
        case angle::FormatID::R32G32B32A32_USCALED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_GPU, DXGI_FORMAT_R32G32B32A32_UINT,
                                               &CopyNativeVertexData<GLuint, 4, 4, 0>);
            return info;
        }

        // GL_UNSIGNED_INT -- normalized
        case angle::FormatID::R32_UNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R32_FLOAT,
                                               &CopyToFloatVertexData<GLuint, 1, 1, true, false>);
            return info;
        }
        case angle::FormatID::R32G32_UNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R32G32_FLOAT,
                                               &CopyToFloatVertexData<GLuint, 2, 2, true, false>);
            return info;
        }
        case angle::FormatID::R32G32B32_UNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R32G32B32_FLOAT,
                                               &CopyToFloatVertexData<GLuint, 3, 3, true, false>);
            return info;
        }
        case angle::FormatID::R32G32B32A32_UNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R32G32B32A32_FLOAT,
                                               &CopyToFloatVertexData<GLuint, 4, 4, true, false>);
            return info;
        }

        // GL_FIXED
        case angle::FormatID::R32_FIXED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R32_FLOAT,
                                               &Copy32FixedTo32FVertexData<1, 1>);
            return info;
        }
        case angle::FormatID::R32G32_FIXED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R32G32_FLOAT,
                                               &Copy32FixedTo32FVertexData<2, 2>);
            return info;
        }
        case angle::FormatID::R32G32B32_FIXED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R32G32B32_FLOAT,
                                               &Copy32FixedTo32FVertexData<3, 3>);
            return info;
        }
        case angle::FormatID::R32G32B32A32_FIXED:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R32G32B32A32_FLOAT,
                                               &Copy32FixedTo32FVertexData<4, 4>);
            return info;
        }

        // GL_HALF_FLOAT
        case angle::FormatID::R16_FLOAT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R16_FLOAT,
                                               &CopyNativeVertexData<GLhalf, 1, 1, 0>);
            return info;
        }
        case angle::FormatID::R16G16_FLOAT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R16G16_FLOAT,
                                               &CopyNativeVertexData<GLhalf, 2, 2, 0>);
            return info;
        }
        case angle::FormatID::R16G16B16_FLOAT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R16G16B16A16_FLOAT,
                                               &CopyNativeVertexData<GLhalf, 3, 4, gl::Float16One>);
            return info;
        }
        case angle::FormatID::R16G16B16A16_FLOAT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R16G16B16A16_FLOAT,
                                               &CopyNativeVertexData<GLhalf, 4, 4, 0>);
            return info;
        }

        // GL_FLOAT
        case angle::FormatID::R32_FLOAT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R32_FLOAT,
                                               &CopyNativeVertexData<GLfloat, 1, 1, 0>);
            return info;
        }
        case angle::FormatID::R32G32_FLOAT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R32G32_FLOAT,
                                               &CopyNativeVertexData<GLfloat, 2, 2, 0>);
            return info;
        }
        case angle::FormatID::R32G32B32_FLOAT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R32G32B32_FLOAT,
                                               &CopyNativeVertexData<GLfloat, 3, 3, 0>);
            return info;
        }
        case angle::FormatID::R32G32B32A32_FLOAT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R32G32B32A32_FLOAT,
                                               &CopyNativeVertexData<GLfloat, 4, 4, 0>);
            return info;
        }

        // GL_INT_2_10_10_10_REV
        case angle::FormatID::R10G10B10A2_SSCALED:
        {
            static constexpr VertexFormat info(
                VERTEX_CONVERT_CPU, DXGI_FORMAT_R32G32B32A32_FLOAT,
                &CopyXYZ10W2ToXYZWFloatVertexData<true, false, true, false>);
            return info;
        }
        case angle::FormatID::R10G10B10A2_SNORM:
        {
            static constexpr VertexFormat info(
                VERTEX_CONVERT_CPU, DXGI_FORMAT_R32G32B32A32_FLOAT,
                &CopyXYZ10W2ToXYZWFloatVertexData<true, true, true, false>);
            return info;
        }

        // GL_UNSIGNED_INT_2_10_10_10_REV
        case angle::FormatID::R10G10B10A2_USCALED:
        {
            static constexpr VertexFormat info(
                VERTEX_CONVERT_CPU, DXGI_FORMAT_R32G32B32A32_FLOAT,
                &CopyXYZ10W2ToXYZWFloatVertexData<false, false, true, false>);
            return info;
        }
        case angle::FormatID::R10G10B10A2_UNORM:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R10G10B10A2_UNORM,
                                               &CopyNativeVertexData<GLuint, 1, 1, 0>);
            return info;
        }

        //
        // Integer Formats
        //

        // GL_BYTE
        case angle::FormatID::R8_SINT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R8_SINT,
                                               &CopyNativeVertexData<GLbyte, 1, 1, 0>);
            return info;
        }
        case angle::FormatID::R8G8_SINT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R8G8_SINT,
                                               &CopyNativeVertexData<GLbyte, 2, 2, 0>);
            return info;
        }
        case angle::FormatID::R8G8B8_SINT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R8G8B8A8_SINT,
                                               &CopyNativeVertexData<GLbyte, 3, 4, 1>);
            return info;
        }
        case angle::FormatID::R8G8B8A8_SINT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R8G8B8A8_SINT,
                                               &CopyNativeVertexData<GLbyte, 4, 4, 0>);
            return info;
        }

        // GL_UNSIGNED_BYTE
        case angle::FormatID::R8_UINT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R8_UINT,
                                               &CopyNativeVertexData<GLubyte, 1, 1, 0>);
            return info;
        }
        case angle::FormatID::R8G8_UINT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R8G8_UINT,
                                               &CopyNativeVertexData<GLubyte, 2, 2, 0>);
            return info;
        }
        case angle::FormatID::R8G8B8_UINT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R8G8B8A8_UINT,
                                               &CopyNativeVertexData<GLubyte, 3, 4, 1>);
            return info;
        }
        case angle::FormatID::R8G8B8A8_UINT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R8G8B8A8_UINT,
                                               &CopyNativeVertexData<GLubyte, 4, 4, 0>);
            return info;
        }

        // GL_SHORT
        case angle::FormatID::R16_SINT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R16_SINT,
                                               &CopyNativeVertexData<GLshort, 1, 1, 0>);
            return info;
        }
        case angle::FormatID::R16G16_SINT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R16G16_SINT,
                                               &CopyNativeVertexData<GLshort, 2, 2, 0>);
            return info;
        }
        case angle::FormatID::R16G16B16_SINT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R16G16B16A16_SINT,
                                               &CopyNativeVertexData<GLshort, 3, 4, 1>);
            return info;
        }
        case angle::FormatID::R16G16B16A16_SINT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R16G16B16A16_SINT,
                                               &CopyNativeVertexData<GLshort, 4, 4, 0>);
            return info;
        }

        // GL_UNSIGNED_SHORT
        case angle::FormatID::R16_UINT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R16_UINT,
                                               &CopyNativeVertexData<GLushort, 1, 1, 0>);
            return info;
        }
        case angle::FormatID::R16G16_UINT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R16G16_UINT,
                                               &CopyNativeVertexData<GLushort, 2, 2, 0>);
            return info;
        }
        case angle::FormatID::R16G16B16_UINT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_CPU, DXGI_FORMAT_R16G16B16A16_UINT,
                                               &CopyNativeVertexData<GLushort, 3, 4, 1>);
            return info;
        }
        case angle::FormatID::R16G16B16A16_UINT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R16G16B16A16_UINT,
                                               &CopyNativeVertexData<GLushort, 4, 4, 0>);
            return info;
        }

        // GL_INT
        case angle::FormatID::R32_SINT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R32_SINT,
                                               &CopyNativeVertexData<GLint, 1, 1, 0>);
            return info;
        }
        case angle::FormatID::R32G32_SINT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R32G32_SINT,
                                               &CopyNativeVertexData<GLint, 2, 2, 0>);
            return info;
        }
        case angle::FormatID::R32G32B32_SINT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R32G32B32_SINT,
                                               &CopyNativeVertexData<GLint, 3, 3, 0>);
            return info;
        }
        case angle::FormatID::R32G32B32A32_SINT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R32G32B32A32_SINT,
                                               &CopyNativeVertexData<GLint, 4, 4, 0>);
            return info;
        }

        // GL_UNSIGNED_INT
        case angle::FormatID::R32_UINT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R32_SINT,
                                               &CopyNativeVertexData<GLuint, 1, 1, 0>);
            return info;
        }
        case angle::FormatID::R32G32_UINT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R32G32_SINT,
                                               &CopyNativeVertexData<GLuint, 2, 2, 0>);
            return info;
        }
        case angle::FormatID::R32G32B32_UINT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R32G32B32_SINT,
                                               &CopyNativeVertexData<GLuint, 3, 3, 0>);
            return info;
        }
        case angle::FormatID::R32G32B32A32_UINT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R32G32B32A32_SINT,
                                               &CopyNativeVertexData<GLuint, 4, 4, 0>);
            return info;
        }

        // GL_INT_2_10_10_10_REV
        case angle::FormatID::R10G10B10A2_SINT:
        {
            static constexpr VertexFormat info(
                VERTEX_CONVERT_CPU, DXGI_FORMAT_R16G16B16A16_SINT,
                &CopyXYZ10W2ToXYZWFloatVertexData<true, true, false, false>);
            return info;
        }

        // GL_UNSIGNED_INT_2_10_10_10_REV
        case angle::FormatID::R10G10B10A2_UINT:
        {
            static constexpr VertexFormat info(VERTEX_CONVERT_NONE, DXGI_FORMAT_R10G10B10A2_UINT,
                                               &CopyNativeVertexData<GLuint, 1, 1, 0>);
            return info;
        }

        default:
        {
            static constexpr VertexFormat info;
            return info;
        }
    }
}

}  // namespace d3d11

}  // namespace rx
