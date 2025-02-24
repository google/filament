//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// d3d_format: Describes a D3D9 format. Used by the D3D9 and GL back-ends.

#include "libANGLE/renderer/d3d_format.h"

using namespace angle;

namespace rx
{
namespace d3d9
{
namespace
{
constexpr D3DFORMAT D3DFMT_INTZ = ((D3DFORMAT)(MAKEFOURCC('I', 'N', 'T', 'Z')));
constexpr D3DFORMAT D3DFMT_NULL = ((D3DFORMAT)(MAKEFOURCC('N', 'U', 'L', 'L')));
}  // anonymous namespace

D3DFormat::D3DFormat()
    : pixelBytes(0),
      blockWidth(0),
      blockHeight(0),
      redBits(0),
      greenBits(0),
      blueBits(0),
      alphaBits(0),
      luminanceBits(0),
      depthBits(0),
      stencilBits(0),
      formatID(angle::FormatID::NONE)
{}

D3DFormat::D3DFormat(GLuint bits,
                     GLuint blockWidth,
                     GLuint blockHeight,
                     GLuint redBits,
                     GLuint greenBits,
                     GLuint blueBits,
                     GLuint alphaBits,
                     GLuint lumBits,
                     GLuint depthBits,
                     GLuint stencilBits,
                     FormatID formatID)
    : pixelBytes(bits / 8),
      blockWidth(blockWidth),
      blockHeight(blockHeight),
      redBits(redBits),
      greenBits(greenBits),
      blueBits(blueBits),
      alphaBits(alphaBits),
      luminanceBits(lumBits),
      depthBits(depthBits),
      stencilBits(stencilBits),
      formatID(formatID)
{}

const D3DFormat &GetD3DFormatInfo(D3DFORMAT format)
{
    if (format == D3DFMT_NULL)
    {
        static const D3DFormat info(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, FormatID::NONE);
        return info;
    }

    if (format == D3DFMT_INTZ)
    {
        static const D3DFormat info(32, 1, 1, 0, 0, 0, 0, 0, 24, 8, FormatID::D24_UNORM_S8_UINT);
        return info;
    }

    switch (format)
    {
        case D3DFMT_UNKNOWN:
        {
            static const D3DFormat info(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, FormatID::NONE);
            return info;
        }

        case D3DFMT_L8:
        {
            static const D3DFormat info(8, 1, 1, 0, 0, 0, 0, 8, 0, 0, FormatID::L8_UNORM);
            return info;
        }
        case D3DFMT_A8:
        {
            static const D3DFormat info(8, 1, 1, 0, 0, 0, 8, 0, 0, 0, FormatID::A8_UNORM);
            return info;
        }
        case D3DFMT_A8L8:
        {
            static const D3DFormat info(16, 1, 1, 0, 0, 0, 8, 8, 0, 0, FormatID::L8A8_UNORM);
            return info;
        }
        case D3DFMT_A4L4:
        {
            static const D3DFormat info(8, 1, 1, 0, 0, 0, 4, 4, 0, 0, FormatID::L4A4_UNORM);
            return info;
        }

        case D3DFMT_A4R4G4B4:
        {
            static const D3DFormat info(16, 1, 1, 4, 4, 4, 4, 0, 0, 0, FormatID::B4G4R4A4_UNORM);
            return info;
        }
        case D3DFMT_A1R5G5B5:
        {
            static const D3DFormat info(16, 1, 1, 5, 5, 5, 1, 0, 0, 0, FormatID::B5G5R5A1_UNORM);
            return info;
        }
        case D3DFMT_R5G6B5:
        {
            static const D3DFormat info(16, 1, 1, 5, 6, 5, 0, 0, 0, 0, FormatID::R5G6B5_UNORM);
            return info;
        }
        case D3DFMT_X8R8G8B8:
        {
            static const D3DFormat info(32, 1, 1, 8, 8, 8, 0, 0, 0, 0, FormatID::B8G8R8X8_UNORM);
            return info;
        }
        case D3DFMT_A8R8G8B8:
        {
            static const D3DFormat info(32, 1, 1, 8, 8, 8, 8, 0, 0, 0, FormatID::B8G8R8A8_UNORM);
            return info;
        }

        case D3DFMT_R16F:
        {
            static const D3DFormat info(16, 1, 1, 16, 0, 0, 0, 0, 0, 0, FormatID::R16_FLOAT);
            return info;
        }
        case D3DFMT_G16R16F:
        {
            static const D3DFormat info(32, 1, 1, 16, 16, 0, 0, 0, 0, 0, FormatID::R16G16_FLOAT);
            return info;
        }
        case D3DFMT_A16B16G16R16F:
        {
            static const D3DFormat info(64, 1, 1, 16, 16, 16, 16, 0, 0, 0,
                                        FormatID::R16G16B16A16_FLOAT);
            return info;
        }
        case D3DFMT_R32F:
        {
            static const D3DFormat info(32, 1, 1, 32, 0, 0, 0, 0, 0, 0, FormatID::R32_FLOAT);
            return info;
        }
        case D3DFMT_G32R32F:
        {
            static const D3DFormat info(64, 1, 1, 32, 32, 0, 0, 0, 0, 0, FormatID::R32G32_FLOAT);
            return info;
        }
        case D3DFMT_A32B32G32R32F:
        {
            static const D3DFormat info(128, 1, 1, 32, 32, 32, 32, 0, 0, 0,
                                        FormatID::R32G32B32A32_FLOAT);
            return info;
        }

        case D3DFMT_D16:
        {
            static const D3DFormat info(16, 1, 1, 0, 0, 0, 0, 0, 16, 0, FormatID::D16_UNORM);
            return info;
        }
        case D3DFMT_D24S8:
        {
            static const D3DFormat info(32, 1, 1, 0, 0, 0, 0, 0, 24, 8,
                                        FormatID::D24_UNORM_S8_UINT);
            return info;
        }
        case D3DFMT_D24X8:
        {
            static const D3DFormat info(32, 1, 1, 0, 0, 0, 0, 0, 24, 0, FormatID::D16_UNORM);
            return info;
        }
        case D3DFMT_D32:
        {
            static const D3DFormat info(32, 1, 1, 0, 0, 0, 0, 0, 32, 0, FormatID::D32_UNORM);
            return info;
        }

        case D3DFMT_DXT1:
        {
            static const D3DFormat info(64, 4, 4, 0, 0, 0, 0, 0, 0, 0,
                                        FormatID::BC1_RGBA_UNORM_BLOCK);
            return info;
        }
        case D3DFMT_DXT3:
        {
            static const D3DFormat info(128, 4, 4, 0, 0, 0, 0, 0, 0, 0,
                                        FormatID::BC2_RGBA_UNORM_BLOCK);
            return info;
        }
        case D3DFMT_DXT5:
        {
            static const D3DFormat info(128, 4, 4, 0, 0, 0, 0, 0, 0, 0,
                                        FormatID::BC3_RGBA_UNORM_BLOCK);
            return info;
        }

        default:
        {
            static const D3DFormat defaultInfo;
            return defaultInfo;
        }
    }
}
}  // namespace d3d9
}  // namespace rx
