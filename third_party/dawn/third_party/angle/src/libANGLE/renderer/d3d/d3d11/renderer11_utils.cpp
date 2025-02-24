//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// renderer11_utils.cpp: Conversion functions and other utility routines
// specific to the D3D11 renderer.

#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"

#include <algorithm>

#include "common/debug.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/Context.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/Program.h"
#include "libANGLE/State.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/BufferD3D.h"
#include "libANGLE/renderer/d3d/FramebufferD3D.h"
#include "libANGLE/renderer/d3d/d3d11/Context11.h"
#include "libANGLE/renderer/d3d/d3d11/RenderTarget11.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"
#include "libANGLE/renderer/d3d/d3d11/texture_format_table.h"
#include "libANGLE/renderer/driver_utils.h"
#include "libANGLE/renderer/dxgi_support_table.h"
#include "platform/PlatformMethods.h"
#include "platform/autogen/FeaturesD3D_autogen.h"

namespace rx
{

namespace d3d11_gl
{
namespace
{
// TODO(xinghua.cao@intel.com): Get a more accurate limit.
static D3D_FEATURE_LEVEL kMinimumFeatureLevelForES31 = D3D_FEATURE_LEVEL_11_0;

// Helper functor for querying DXGI support. Saves passing the parameters repeatedly.
class DXGISupportHelper : angle::NonCopyable
{
  public:
    DXGISupportHelper(ID3D11Device *device, D3D_FEATURE_LEVEL featureLevel)
        : mDevice(device), mFeatureLevel(featureLevel)
    {}

    bool query(DXGI_FORMAT dxgiFormat, UINT supportMask)
    {
        if (dxgiFormat == DXGI_FORMAT_UNKNOWN)
            return false;

        auto dxgiSupport = d3d11::GetDXGISupport(dxgiFormat, mFeatureLevel);

        UINT supportedBits = dxgiSupport.alwaysSupportedFlags;

        if ((dxgiSupport.optionallySupportedFlags & supportMask) != 0)
        {
            UINT formatSupport;
            if (SUCCEEDED(mDevice->CheckFormatSupport(dxgiFormat, &formatSupport)))
            {
                supportedBits |= (formatSupport & supportMask);
            }
            else
            {
                // TODO(jmadill): find out why we fail this call sometimes in FL9_3
                // ERR() << "Error checking format support for format 0x" << std::hex << dxgiFormat;
            }
        }

        return ((supportedBits & supportMask) == supportMask);
    }

  private:
    ID3D11Device *mDevice;
    D3D_FEATURE_LEVEL mFeatureLevel;
};

gl::TextureCaps GenerateTextureFormatCaps(gl::Version maxClientVersion,
                                          GLenum internalFormat,
                                          ID3D11Device *device,
                                          const Renderer11DeviceCaps &renderer11DeviceCaps)
{
    gl::TextureCaps textureCaps;

    DXGISupportHelper support(device, renderer11DeviceCaps.featureLevel);
    const d3d11::Format &formatInfo = d3d11::Format::Get(internalFormat, renderer11DeviceCaps);

    const gl::InternalFormat &internalFormatInfo = gl::GetSizedInternalFormatInfo(internalFormat);

    UINT texSupportMask = D3D11_FORMAT_SUPPORT_TEXTURE2D;
    if (internalFormatInfo.depthBits == 0 && internalFormatInfo.stencilBits == 0)
    {
        texSupportMask |= D3D11_FORMAT_SUPPORT_TEXTURECUBE;
        if (maxClientVersion.major > 2)
        {
            texSupportMask |= D3D11_FORMAT_SUPPORT_TEXTURE3D;
        }
    }

    textureCaps.texturable = support.query(formatInfo.texFormat, texSupportMask);
    textureCaps.filterable =
        support.query(formatInfo.srvFormat, D3D11_FORMAT_SUPPORT_SHADER_SAMPLE);
    textureCaps.textureAttachment =
        (support.query(formatInfo.rtvFormat, D3D11_FORMAT_SUPPORT_RENDER_TARGET)) ||
        (support.query(formatInfo.dsvFormat, D3D11_FORMAT_SUPPORT_DEPTH_STENCIL));
    textureCaps.renderbuffer = textureCaps.textureAttachment;
    textureCaps.blendable    = textureCaps.renderbuffer;

    DXGI_FORMAT renderFormat = DXGI_FORMAT_UNKNOWN;
    if (formatInfo.dsvFormat != DXGI_FORMAT_UNKNOWN)
    {
        renderFormat = formatInfo.dsvFormat;
    }
    else if (formatInfo.rtvFormat != DXGI_FORMAT_UNKNOWN)
    {
        renderFormat = formatInfo.rtvFormat;
    }
    if (renderFormat != DXGI_FORMAT_UNKNOWN &&
        support.query(renderFormat, D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET))
    {
        // Assume 1x
        textureCaps.sampleCounts.insert(1);

        for (unsigned int sampleCount = 2; sampleCount <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT;
             sampleCount *= 2)
        {
            UINT qualityCount = 0;
            if (SUCCEEDED(device->CheckMultisampleQualityLevels(renderFormat, sampleCount,
                                                                &qualityCount)))
            {
                // Assume we always support lower sample counts
                if (qualityCount == 0)
                {
                    break;
                }
                textureCaps.sampleCounts.insert(sampleCount);
            }
        }
    }

    return textureCaps;
}

bool GetNPOTTextureSupport(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return true;

        // From http://msdn.microsoft.com/en-us/library/windows/desktop/ff476876.aspx
        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return false;

        default:
            UNREACHABLE();
            return false;
    }
}

float GetMaximumAnisotropy(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_MAX_MAXANISOTROPY;

        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return D3D10_MAX_MAXANISOTROPY;

        // From http://msdn.microsoft.com/en-us/library/windows/desktop/ff476876.aspx
        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
            return 16;

        case D3D_FEATURE_LEVEL_9_1:
            return D3D_FL9_1_DEFAULT_MAX_ANISOTROPY;

        default:
            UNREACHABLE();
            return 0;
    }
}

bool GetOcclusionQuerySupport(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return true;

        // From http://msdn.microsoft.com/en-us/library/windows/desktop/ff476150.aspx
        // ID3D11Device::CreateQuery
        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
            return true;
        case D3D_FEATURE_LEVEL_9_1:
            return false;

        default:
            UNREACHABLE();
            return false;
    }
}

bool GetEventQuerySupport(D3D_FEATURE_LEVEL featureLevel)
{
    // From http://msdn.microsoft.com/en-us/library/windows/desktop/ff476150.aspx
    // ID3D11Device::CreateQuery

    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return true;

        default:
            UNREACHABLE();
            return false;
    }
}

bool GetInstancingSupport(D3D_FEATURE_LEVEL featureLevel)
{
    // From http://msdn.microsoft.com/en-us/library/windows/desktop/ff476150.aspx
    // ID3D11Device::CreateInputLayout

    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return true;

        // Feature Level 9_3 supports instancing, but slot 0 in the input layout must not be
        // instanced.
        // D3D9 has a similar restriction, where stream 0 must not be instanced.
        // This restriction can be worked around by remapping any non-instanced slot to slot
        // 0.
        // This works because HLSL uses shader semantics to match the vertex inputs to the
        // elements in the input layout, rather than the slots.
        // Note that we only support instancing via ANGLE_instanced_array on 9_3, since 9_3
        // doesn't support OpenGL ES 3.0
        case D3D_FEATURE_LEVEL_9_3:
            return true;

        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return false;

        default:
            UNREACHABLE();
            return false;
    }
}

bool GetFramebufferMultisampleSupport(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return true;

        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return false;

        default:
            UNREACHABLE();
            return false;
    }
}

bool GetFramebufferBlitSupport(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return true;

        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return false;

        default:
            UNREACHABLE();
            return false;
    }
}

bool GetDerivativeInstructionSupport(D3D_FEATURE_LEVEL featureLevel)
{
    // http://msdn.microsoft.com/en-us/library/windows/desktop/bb509588.aspx states that
    // shader model
    // ps_2_x is required for the ddx (and other derivative functions).

    // http://msdn.microsoft.com/en-us/library/windows/desktop/ff476876.aspx states that
    // feature level
    // 9.3 supports shader model ps_2_x.

    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
        case D3D_FEATURE_LEVEL_9_3:
            return true;
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return false;

        default:
            UNREACHABLE();
            return false;
    }
}

bool GetShaderTextureLODSupport(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return true;

        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return false;

        default:
            UNREACHABLE();
            return false;
    }
}

int GetMaximumSimultaneousRenderTargets(D3D_FEATURE_LEVEL featureLevel)
{
    // From http://msdn.microsoft.com/en-us/library/windows/desktop/ff476150.aspx
    // ID3D11Device::CreateInputLayout

    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;

        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return D3D10_SIMULTANEOUS_RENDER_TARGET_COUNT;

        case D3D_FEATURE_LEVEL_9_3:
            return D3D_FL9_3_SIMULTANEOUS_RENDER_TARGET_COUNT;
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return D3D_FL9_1_SIMULTANEOUS_RENDER_TARGET_COUNT;

        default:
            UNREACHABLE();
            return 0;
    }
}

int GetMaximum2DTextureSize(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;

        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION;

        case D3D_FEATURE_LEVEL_9_3:
            return D3D_FL9_3_REQ_TEXTURE2D_U_OR_V_DIMENSION;
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return D3D_FL9_1_REQ_TEXTURE2D_U_OR_V_DIMENSION;

        default:
            UNREACHABLE();
            return 0;
    }
}

int GetMaximumCubeMapTextureSize(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_REQ_TEXTURECUBE_DIMENSION;

        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return D3D10_REQ_TEXTURECUBE_DIMENSION;

        case D3D_FEATURE_LEVEL_9_3:
            return D3D_FL9_3_REQ_TEXTURECUBE_DIMENSION;
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return D3D_FL9_1_REQ_TEXTURECUBE_DIMENSION;

        default:
            UNREACHABLE();
            return 0;
    }
}

int GetMaximum2DTextureArraySize(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;

        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return D3D10_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;

        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return 0;

        default:
            UNREACHABLE();
            return 0;
    }
}

int GetMaximum3DTextureSize(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;

        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return D3D10_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;

        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return D3D_FL9_1_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;

        default:
            UNREACHABLE();
            return 0;
    }
}

int GetMaximumViewportSize(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_VIEWPORT_BOUNDS_MAX;

        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return D3D10_VIEWPORT_BOUNDS_MAX;

        // No constants for D3D11 Feature Level 9 viewport size limits, use the maximum
        // texture sizes
        case D3D_FEATURE_LEVEL_9_3:
            return D3D_FL9_3_REQ_TEXTURE2D_U_OR_V_DIMENSION;
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return D3D_FL9_1_REQ_TEXTURE2D_U_OR_V_DIMENSION;

        default:
            UNREACHABLE();
            return 0;
    }
}

int GetMaximumDrawIndexedIndexCount(D3D_FEATURE_LEVEL featureLevel)
{
    // D3D11 allows up to 2^32 elements, but we report max signed int for convenience since
    // that's what's
    // returned from glGetInteger
    static_assert(D3D11_REQ_DRAWINDEXED_INDEX_COUNT_2_TO_EXP == 32,
                  "Unexpected D3D11 constant value.");
    static_assert(D3D10_REQ_DRAWINDEXED_INDEX_COUNT_2_TO_EXP == 32,
                  "Unexpected D3D11 constant value.");

    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return std::numeric_limits<GLint>::max();

        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
            return D3D_FL9_2_IA_PRIMITIVE_MAX_COUNT;
        case D3D_FEATURE_LEVEL_9_1:
            return D3D_FL9_1_IA_PRIMITIVE_MAX_COUNT;

        default:
            UNREACHABLE();
            return 0;
    }
}

int GetMaximumDrawVertexCount(D3D_FEATURE_LEVEL featureLevel)
{
    // D3D11 allows up to 2^32 elements, but we report max signed int for convenience since
    // that's what's
    // returned from glGetInteger
    static_assert(D3D11_REQ_DRAW_VERTEX_COUNT_2_TO_EXP == 32, "Unexpected D3D11 constant value.");
    static_assert(D3D10_REQ_DRAW_VERTEX_COUNT_2_TO_EXP == 32, "Unexpected D3D11 constant value.");

    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return std::numeric_limits<GLint>::max();

        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
            return D3D_FL9_2_IA_PRIMITIVE_MAX_COUNT;
        case D3D_FEATURE_LEVEL_9_1:
            return D3D_FL9_1_IA_PRIMITIVE_MAX_COUNT;

        default:
            UNREACHABLE();
            return 0;
    }
}

int GetMaximumVertexInputSlots(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_STANDARD_VERTEX_ELEMENT_COUNT;

        case D3D_FEATURE_LEVEL_10_1:
            return D3D10_1_STANDARD_VERTEX_ELEMENT_COUNT;
        case D3D_FEATURE_LEVEL_10_0:
            return D3D10_STANDARD_VERTEX_ELEMENT_COUNT;

        // From http://http://msdn.microsoft.com/en-us/library/windows/desktop/ff476876.aspx
        // "Max Input Slots"
        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return 16;

        default:
            UNREACHABLE();
            return 0;
    }
}

int GetMaximumVertexUniformVectors(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT;

        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return D3D10_REQ_CONSTANT_BUFFER_ELEMENT_COUNT;

        // From http://msdn.microsoft.com/en-us/library/windows/desktop/ff476149.aspx
        // ID3D11DeviceContext::VSSetConstantBuffers
        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return 255 - d3d11_gl::GetReservedVertexUniformVectors(featureLevel);

        default:
            UNREACHABLE();
            return 0;
    }
}

int GetMaximumVertexUniformBlocks(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT -
                   d3d11::RESERVED_CONSTANT_BUFFER_SLOT_COUNT;

        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return D3D10_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT -
                   d3d11::RESERVED_CONSTANT_BUFFER_SLOT_COUNT;

        // Uniform blocks not supported on D3D11 Feature Level 9
        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return 0;

        default:
            UNREACHABLE();
            return 0;
    }
}

int GetReservedVertexOutputVectors(D3D_FEATURE_LEVEL featureLevel)
{
    // According to The OpenGL ES Shading Language specifications
    // (Language Version 1.00 section 10.16, Language Version 3.10 section 12.21)
    // built-in special variables (e.g. gl_FragCoord, or gl_PointCoord)
    // which are statically used in the shader should be included in the variable packing
    // algorithm.
    // Therefore, we should not reserve output vectors for them.

    switch (featureLevel)
    {
        // We must reserve one output vector for dx_Position.
        // We also reserve one for gl_Position, which we unconditionally output on Feature
        // Levels 10_0+,
        // even if it's unused in the shader (e.g. for transform feedback). TODO: This could
        // be improved.
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return 2;

        // Just reserve dx_Position on Feature Level 9, since we don't ever need to output
        // gl_Position.
        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return 1;

        default:
            UNREACHABLE();
            return 0;
    }
}

int GetMaximumVertexOutputVectors(D3D_FEATURE_LEVEL featureLevel)
{
    static_assert(gl::IMPLEMENTATION_MAX_VARYING_VECTORS == D3D11_VS_OUTPUT_REGISTER_COUNT,
                  "Unexpected D3D11 constant value.");

    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_VS_OUTPUT_REGISTER_COUNT - GetReservedVertexOutputVectors(featureLevel);

        case D3D_FEATURE_LEVEL_10_1:
            return D3D10_1_VS_OUTPUT_REGISTER_COUNT - GetReservedVertexOutputVectors(featureLevel);
        case D3D_FEATURE_LEVEL_10_0:
            return D3D10_VS_OUTPUT_REGISTER_COUNT - GetReservedVertexOutputVectors(featureLevel);

        // Use Shader Model 2.X limits
        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return 8 - GetReservedVertexOutputVectors(featureLevel);

        default:
            UNREACHABLE();
            return 0;
    }
}

int GetMaximumVertexTextureUnits(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;

        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return D3D10_COMMONSHADER_SAMPLER_SLOT_COUNT;

        // Vertex textures not supported on D3D11 Feature Level 9 according to
        // http://msdn.microsoft.com/en-us/library/windows/desktop/ff476149.aspx
        // ID3D11DeviceContext::VSSetSamplers and ID3D11DeviceContext::VSSetShaderResources
        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return 0;

        default:
            UNREACHABLE();
            return 0;
    }
}

int GetMaximumPixelUniformVectors(D3D_FEATURE_LEVEL featureLevel)
{
    // TODO(geofflang): Remove hard-coded limit once the gl-uniform-arrays test can pass
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return 1024;  // D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT;

        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return 1024;  // D3D10_REQ_CONSTANT_BUFFER_ELEMENT_COUNT;

        // From http://msdn.microsoft.com/en-us/library/windows/desktop/ff476149.aspx
        // ID3D11DeviceContext::PSSetConstantBuffers
        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return 32 - d3d11_gl::GetReservedFragmentUniformVectors(featureLevel);

        default:
            UNREACHABLE();
            return 0;
    }
}

int GetMaximumPixelUniformBlocks(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT -
                   d3d11::RESERVED_CONSTANT_BUFFER_SLOT_COUNT;

        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return D3D10_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT -
                   d3d11::RESERVED_CONSTANT_BUFFER_SLOT_COUNT;

        // Uniform blocks not supported on D3D11 Feature Level 9
        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return 0;

        default:
            UNREACHABLE();
            return 0;
    }
}

int GetMaximumPixelInputVectors(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_PS_INPUT_REGISTER_COUNT - GetReservedVertexOutputVectors(featureLevel);

        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return D3D10_PS_INPUT_REGISTER_COUNT - GetReservedVertexOutputVectors(featureLevel);

        // Use Shader Model 2.X limits
        case D3D_FEATURE_LEVEL_9_3:
            return 8 - GetReservedVertexOutputVectors(featureLevel);
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return 8 - GetReservedVertexOutputVectors(featureLevel);

        default:
            UNREACHABLE();
            return 0;
    }
}

int GetMaximumPixelTextureUnits(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;

        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return D3D10_COMMONSHADER_SAMPLER_SLOT_COUNT;

        // http://msdn.microsoft.com/en-us/library/windows/desktop/ff476149.aspx
        // ID3D11DeviceContext::PSSetShaderResources
        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return 16;

        default:
            UNREACHABLE();
            return 0;
    }
}

std::array<GLint, 3> GetMaxComputeWorkGroupCount(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return {{D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION,
                     D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION,
                     D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION}};
        default:
            return {{0, 0, 0}};
    }
}

std::array<GLint, 3> GetMaxComputeWorkGroupSize(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return {{D3D11_CS_THREAD_GROUP_MAX_X, D3D11_CS_THREAD_GROUP_MAX_Y,
                     D3D11_CS_THREAD_GROUP_MAX_Z}};
        default:
            return {{0, 0, 0}};
    }
}

int GetMaxComputeWorkGroupInvocations(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_CS_THREAD_GROUP_MAX_THREADS_PER_GROUP;
        default:
            return 0;
    }
}

int GetMaxComputeSharedMemorySize(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        // In D3D11 the maximum total size of all variables with the groupshared storage class is
        // 32kb.
        // https://docs.microsoft.com/en-us/windows/desktop/direct3dhlsl/dx-graphics-hlsl-variable-syntax
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return 32768;
        default:
            return 0;
    }
}

int GetMaximumComputeUniformVectors(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT;
        default:
            return 0;
    }
}

int GetMaximumComputeUniformBlocks(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT -
                   d3d11::RESERVED_CONSTANT_BUFFER_SLOT_COUNT;
        default:
            return 0;
    }
}

int GetMaximumComputeTextureUnits(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
        default:
            return 0;
    }
}

void SetUAVRelatedResourceLimits(D3D_FEATURE_LEVEL featureLevel, gl::Caps *caps)
{
    ASSERT(caps);

    GLuint reservedUAVsForAtomicCounterBuffers = 0u;

    // For pixel shaders, the render targets and unordered access views share the same resource
    // slots when being written out.
    // https://msdn.microsoft.com/en-us/library/windows/desktop/ff476465(v=vs.85).aspx
    GLuint maxNumRTVsAndUAVs = 0u;

    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
            // Currently we allocate 4 UAV slots for atomic counter buffers on feature level 11_1.
            reservedUAVsForAtomicCounterBuffers = 4u;
            maxNumRTVsAndUAVs                   = D3D11_1_UAV_SLOT_COUNT;
            break;
        case D3D_FEATURE_LEVEL_11_0:
            // Currently we allocate 1 UAV slot for atomic counter buffers on feature level 11_0.
            reservedUAVsForAtomicCounterBuffers = 1u;
            maxNumRTVsAndUAVs                   = D3D11_PS_CS_UAV_REGISTER_COUNT;
            break;
        default:
            return;
    }

    // Set limits on atomic counter buffers in fragment shaders and compute shaders.
    caps->maxCombinedAtomicCounterBuffers = reservedUAVsForAtomicCounterBuffers;
    caps->maxShaderAtomicCounterBuffers[gl::ShaderType::Compute] =
        reservedUAVsForAtomicCounterBuffers;
    caps->maxShaderAtomicCounterBuffers[gl::ShaderType::Fragment] =
        reservedUAVsForAtomicCounterBuffers;
    caps->maxAtomicCounterBufferBindings = reservedUAVsForAtomicCounterBuffers;

    // Setting MAX_COMPUTE_ATOMIC_COUNTERS to a conservative number of 1024 * the number of UAV
    // reserved for atomic counters. It could theoretically be set to max buffer size / 4 but that
    // number could cause problems.
    caps->maxCombinedAtomicCounters = reservedUAVsForAtomicCounterBuffers * 1024;
    caps->maxShaderAtomicCounters[gl::ShaderType::Compute] = caps->maxCombinedAtomicCounters;

    // See
    // https://docs.microsoft.com/en-us/windows/desktop/direct3d11/overviews-direct3d-11-resources-limits
    // Resource size (in MB) for any of the preceding resources is min(max(128,0.25f * (amount of
    // dedicated VRAM)), 2048) MB. So we set it to 128MB to keep same with GL backend.
    GLint maxResourceSize = D3D11_REQ_RESOURCE_SIZE_IN_MEGABYTES_EXPRESSION_A_TERM * 1024 * 1024;
    caps->maxShaderStorageBlockSize = maxResourceSize;

    caps->maxAtomicCounterBufferSize = maxResourceSize;

    // Allocate the remaining slots for images and shader storage blocks.
    // The maximum number of fragment shader outputs depends on the current context version, so we
    // will not set it here. See comments in Context11::initialize().
    caps->maxCombinedShaderOutputResources =
        maxNumRTVsAndUAVs - reservedUAVsForAtomicCounterBuffers;

    // Set limits on images and shader storage blocks in fragment shaders and compute shaders.
    caps->maxCombinedShaderStorageBlocks                   = caps->maxCombinedShaderOutputResources;
    caps->maxShaderStorageBlocks[gl::ShaderType::Compute]  = caps->maxCombinedShaderOutputResources;
    caps->maxShaderStorageBlocks[gl::ShaderType::Fragment] = caps->maxCombinedShaderOutputResources;
    caps->maxShaderStorageBufferBindings                   = caps->maxCombinedShaderOutputResources;

    caps->maxImageUnits                                    = caps->maxCombinedShaderOutputResources;
    caps->maxCombinedImageUniforms                         = caps->maxCombinedShaderOutputResources;
    caps->maxShaderImageUniforms[gl::ShaderType::Compute]  = caps->maxCombinedShaderOutputResources;
    caps->maxShaderImageUniforms[gl::ShaderType::Fragment] = caps->maxCombinedShaderOutputResources;

    // On feature level 11_1, UAVs are also available in vertex shaders and geometry shaders.
    if (featureLevel == D3D_FEATURE_LEVEL_11_1)
    {
        caps->maxShaderAtomicCounterBuffers[gl::ShaderType::Vertex] =
            caps->maxCombinedAtomicCounterBuffers;
        caps->maxShaderAtomicCounterBuffers[gl::ShaderType::Geometry] =
            caps->maxCombinedAtomicCounterBuffers;

        caps->maxShaderImageUniforms[gl::ShaderType::Vertex] =
            caps->maxCombinedShaderOutputResources;
        caps->maxShaderStorageBlocks[gl::ShaderType::Vertex] =
            caps->maxCombinedShaderOutputResources;
        caps->maxShaderImageUniforms[gl::ShaderType::Geometry] =
            caps->maxCombinedShaderOutputResources;
        caps->maxShaderStorageBlocks[gl::ShaderType::Geometry] =
            caps->maxCombinedShaderOutputResources;
    }
}

int GetMinimumTexelOffset(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_COMMONSHADER_TEXEL_OFFSET_MAX_NEGATIVE;

        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return D3D10_COMMONSHADER_TEXEL_OFFSET_MAX_NEGATIVE;

        // Sampling functions with offsets are not available below shader model 4.0.
        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return 0;

        default:
            UNREACHABLE();
            return 0;
    }
}

int GetMaximumTexelOffset(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_COMMONSHADER_TEXEL_OFFSET_MAX_POSITIVE;
        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return D3D11_COMMONSHADER_TEXEL_OFFSET_MAX_POSITIVE;

        // Sampling functions with offsets are not available below shader model 4.0.
        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return 0;

        default:
            UNREACHABLE();
            return 0;
    }
}

int GetMinimumTextureGatherOffset(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        // https://docs.microsoft.com/en-us/windows/desktop/direct3dhlsl/gather4-po--sm5---asm-
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return -32;

        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return 0;

        default:
            UNREACHABLE();
            return 0;
    }
}

int GetMaximumTextureGatherOffset(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        // https://docs.microsoft.com/en-us/windows/desktop/direct3dhlsl/gather4-po--sm5---asm-
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return 31;

        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return 0;

        default:
            UNREACHABLE();
            return 0;
    }
}

size_t GetMaximumConstantBufferSize(D3D_FEATURE_LEVEL featureLevel)
{
    // Returns a size_t despite the limit being a GLuint64 because size_t is the maximum
    // size of
    // any buffer that could be allocated.

    const size_t bytesPerComponent = 4 * sizeof(float);

    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * bytesPerComponent;

        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return D3D10_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * bytesPerComponent;

        // Limits from http://msdn.microsoft.com/en-us/library/windows/desktop/ff476501.aspx
        // remarks section
        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return 4096 * bytesPerComponent;

        default:
            UNREACHABLE();
            return 0;
    }
}

int GetMaximumStreamOutputBuffers(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_SO_BUFFER_SLOT_COUNT;

        case D3D_FEATURE_LEVEL_10_1:
            return D3D10_1_SO_BUFFER_SLOT_COUNT;
        case D3D_FEATURE_LEVEL_10_0:
            return D3D10_SO_BUFFER_SLOT_COUNT;

        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return 0;

        default:
            UNREACHABLE();
            return 0;
    }
}

int GetMaximumStreamOutputInterleavedComponents(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:

        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return GetMaximumVertexOutputVectors(featureLevel) * 4;

        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return 0;

        default:
            UNREACHABLE();
            return 0;
    }
}

int GetMaximumStreamOutputSeparateComponents(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return GetMaximumStreamOutputInterleavedComponents(featureLevel) /
                   GetMaximumStreamOutputBuffers(featureLevel);

        // D3D 10 and 10.1 only allow one output per output slot if an output slot other
        // than zero is used.
        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return 4;

        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return 0;

        default:
            UNREACHABLE();
            return 0;
    }
}

int GetMaximumRenderToBufferWindowSize(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_REQ_RENDER_TO_BUFFER_WINDOW_WIDTH;
        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return D3D10_REQ_RENDER_TO_BUFFER_WINDOW_WIDTH;

        // REQ_RENDER_TO_BUFFER_WINDOW_WIDTH not supported on D3D11 Feature Level 9,
        // use the maximum texture sizes
        case D3D_FEATURE_LEVEL_9_3:
            return D3D_FL9_3_REQ_TEXTURE2D_U_OR_V_DIMENSION;
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return D3D_FL9_1_REQ_TEXTURE2D_U_OR_V_DIMENSION;

        default:
            UNREACHABLE();
            return 0;
    }
}

angle::VersionTriple GetIntelDriverVersion(const Optional<LARGE_INTEGER> driverVersion)
{
    if (!driverVersion.valid())
    {
        return angle::VersionTriple(0, 0, 0);
    }

    DWORD lowPart = driverVersion.value().LowPart;

    // Please refer the details at
    // http://www.intel.com/content/www/us/en/support/graphics-drivers/000005654.html.
    return angle::VersionTriple(HIWORD(lowPart), LOWORD(lowPart), 0);
}

}  // anonymous namespace

unsigned int GetReservedVertexUniformVectors(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return 0;

        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return 3;  // dx_ViewAdjust, dx_ViewCoords and dx_ViewScale

        default:
            UNREACHABLE();
            return 0;
    }
}

unsigned int GetReservedFragmentUniformVectors(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return 0;

        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return 4;  // dx_ViewCoords, dx_DepthFront, dx_DepthRange, dx_FragCoordOffset

        default:
            UNREACHABLE();
            return 0;
    }
}

gl::Version GetMaximumClientVersion(const Renderer11DeviceCaps &caps)
{
    switch (caps.featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return gl::Version(3, 1);
        case D3D_FEATURE_LEVEL_10_1:
            return gl::Version(3, 0);

        case D3D_FEATURE_LEVEL_10_0:
            if (caps.allowES3OnFL10_0)
            {
                return gl::Version(3, 0);
            }
            else
            {
                return gl::Version(2, 0);
            }
        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return gl::Version(2, 0);

        default:
            UNREACHABLE();
            return gl::Version(0, 0);
    }
}

D3D_FEATURE_LEVEL GetMinimumFeatureLevelForES31()
{
    return kMinimumFeatureLevelForES31;
}

unsigned int GetMaxViewportAndScissorRectanglesPerPipeline(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return 1;
        default:
            UNREACHABLE();
            return 0;
    }
}

bool IsMultiviewSupported(D3D_FEATURE_LEVEL featureLevel)
{
    // The multiview extensions can always be supported in D3D11 through geometry shaders.
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            return true;
        default:
            return false;
    }
}

int GetMaxSampleMaskWords(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        // D3D10+ only allows 1 sample mask.
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            return 1;
        case D3D_FEATURE_LEVEL_9_3:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            return 0;
        default:
            UNREACHABLE();
            return 0;
    }
}

bool HasTextureBufferSupport(ID3D11Device *device, const Renderer11DeviceCaps &renderer11DeviceCaps)
{
    if (renderer11DeviceCaps.featureLevel < D3D_FEATURE_LEVEL_11_0)
        return false;

    if (!renderer11DeviceCaps.supportsTypedUAVLoadAdditionalFormats)
        return false;

    // https://docs.microsoft.com/en-us/windows/win32/direct3d12/typed-unordered-access-view-loads
    // we don't need to check the typed store. from the spec,
    // https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm#FormatList
    // all the following format support typed stored.
    // According to spec,
    // https://www.khronos.org/registry/OpenGL-Refpages/es3/html/glBindImageTexture.xhtml the
    // required image unit format are GL_RGBA32F, GL_RGBA32UI, GL_RGBA32I, GL_RGBA16F, GL_RGBA16UI,
    // GL_RGBA16I, GL_RGBA8, GL_RGBAUI, GL_RGBA8I, GL_RGBA8_SNORM, GL_R32F, GL_R32UI, GL_R32I,
    const std::array<DXGI_FORMAT, 2> &optionalFormats = {
        DXGI_FORMAT_R32G32B32A32_FLOAT,  // test for GL_RGBA32(UIF), GL_RGBA16(UIF),
                                         // GL_RGBA8(UIUnorm)
        DXGI_FORMAT_R8G8B8A8_SNORM,      // test for GL_RGBA8_SNORM,
    };

    for (DXGI_FORMAT dxgiFormat : optionalFormats)
    {
        D3D11_FEATURE_DATA_FORMAT_SUPPORT FormatSupport = {dxgiFormat, 0};
        if (!SUCCEEDED(device->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT, &FormatSupport,
                                                   sizeof(FormatSupport))))
        {
            WARN() << "Error checking typed load support for format 0x" << std::hex << dxgiFormat;
            return false;
        }
        if ((FormatSupport.OutFormatSupport & D3D11_FORMAT_SUPPORT2_UAV_TYPED_LOAD) == 0)
            return false;
    }
    return true;
}

void GenerateCaps(ID3D11Device *device,
                  ID3D11DeviceContext *deviceContext,
                  const Renderer11DeviceCaps &renderer11DeviceCaps,
                  const angle::FeaturesD3D &features,
                  const char *description,
                  gl::Caps *caps,
                  gl::TextureCapsMap *textureCapsMap,
                  gl::Extensions *extensions,
                  gl::Limitations *limitations,
                  ShPixelLocalStorageOptions *plsOptions)
{
    const D3D_FEATURE_LEVEL featureLevel = renderer11DeviceCaps.featureLevel;
    const gl::FormatSet &allFormats      = gl::GetAllSizedInternalFormats();
    for (GLenum internalFormat : allFormats)
    {
        gl::TextureCaps textureCaps =
            GenerateTextureFormatCaps(GetMaximumClientVersion(renderer11DeviceCaps), internalFormat,
                                      device, renderer11DeviceCaps);
        textureCapsMap->insert(internalFormat, textureCaps);
    }

    // GL core feature limits
    // Reserve MAX_UINT for D3D11's primitive restart.
    caps->maxElementIndex  = static_cast<GLint64>(std::numeric_limits<unsigned int>::max() - 1);
    caps->max3DTextureSize = GetMaximum3DTextureSize(featureLevel);
    caps->max2DTextureSize = GetMaximum2DTextureSize(featureLevel);
    caps->maxCubeMapTextureSize = GetMaximumCubeMapTextureSize(featureLevel);
    caps->maxArrayTextureLayers = GetMaximum2DTextureArraySize(featureLevel);

    // Unimplemented, set to minimum required
    caps->maxLODBias = 2.0f;

    // No specific limits on render target size, maximum 2D texture size is equivalent
    caps->maxRenderbufferSize = caps->max2DTextureSize;

    // Maximum draw buffers and color attachments are the same, max color attachments could
    // eventually be increased to 16
    caps->maxDrawBuffers      = GetMaximumSimultaneousRenderTargets(featureLevel);
    caps->maxColorAttachments = GetMaximumSimultaneousRenderTargets(featureLevel);

    // D3D11 has the same limit for viewport width and height
    caps->maxViewportWidth  = GetMaximumViewportSize(featureLevel);
    caps->maxViewportHeight = caps->maxViewportWidth;

    // Choose a reasonable maximum, enforced in the shader.
    caps->minAliasedPointSize = 1.0f;
    caps->maxAliasedPointSize = 1024.0f;

    // Wide lines not supported
    caps->minAliasedLineWidth = 1.0f;
    caps->maxAliasedLineWidth = 1.0f;

    // Primitive count limits
    caps->maxElementsIndices  = GetMaximumDrawIndexedIndexCount(featureLevel);
    caps->maxElementsVertices = GetMaximumDrawVertexCount(featureLevel);

    // Program and shader binary formats (no supported shader binary formats)
    caps->programBinaryFormats.push_back(GL_PROGRAM_BINARY_ANGLE);

    caps->vertexHighpFloat.setIEEEFloat();
    caps->vertexMediumpFloat.setIEEEFloat();
    caps->vertexLowpFloat.setIEEEFloat();
    caps->fragmentHighpFloat.setIEEEFloat();
    caps->fragmentMediumpFloat.setIEEEFloat();
    caps->fragmentLowpFloat.setIEEEFloat();

    // 32-bit integers are natively supported
    caps->vertexHighpInt.setTwosComplementInt(32);
    caps->vertexMediumpInt.setTwosComplementInt(32);
    caps->vertexLowpInt.setTwosComplementInt(32);
    caps->fragmentHighpInt.setTwosComplementInt(32);
    caps->fragmentMediumpInt.setTwosComplementInt(32);
    caps->fragmentLowpInt.setTwosComplementInt(32);

    // We do not wait for server fence objects internally, so report a max timeout of zero.
    caps->maxServerWaitTimeout = 0;

    // Vertex shader limits
    caps->maxVertexAttributes     = GetMaximumVertexInputSlots(featureLevel);
    caps->maxVertexUniformVectors = GetMaximumVertexUniformVectors(featureLevel);
    if (features.skipVSConstantRegisterZero.enabled)
    {
        caps->maxVertexUniformVectors -= 1;
    }
    caps->maxShaderUniformComponents[gl::ShaderType::Vertex] = caps->maxVertexUniformVectors * 4;
    caps->maxShaderUniformBlocks[gl::ShaderType::Vertex] =
        GetMaximumVertexUniformBlocks(featureLevel);
    caps->maxVertexOutputComponents = GetMaximumVertexOutputVectors(featureLevel) * 4;
    caps->maxShaderTextureImageUnits[gl::ShaderType::Vertex] =
        GetMaximumVertexTextureUnits(featureLevel);

    // Vertex Attribute Bindings are emulated on D3D11.
    caps->maxVertexAttribBindings = caps->maxVertexAttributes;
    // Experimental testing confirmed there is no explicit limit on maximum buffer offset in D3D11.
    caps->maxVertexAttribRelativeOffset = std::numeric_limits<GLint>::max();
    // Experimental testing confirmed 2048 is the maximum stride that D3D11 can support on all
    // platforms.
    caps->maxVertexAttribStride = 2048;

    // Fragment shader limits
    caps->maxFragmentUniformVectors = GetMaximumPixelUniformVectors(featureLevel);
    caps->maxShaderUniformComponents[gl::ShaderType::Fragment] =
        caps->maxFragmentUniformVectors * 4;
    caps->maxShaderUniformBlocks[gl::ShaderType::Fragment] =
        GetMaximumPixelUniformBlocks(featureLevel);
    caps->maxFragmentInputComponents = GetMaximumPixelInputVectors(featureLevel) * 4;
    caps->maxShaderTextureImageUnits[gl::ShaderType::Fragment] =
        GetMaximumPixelTextureUnits(featureLevel);
    caps->minProgramTexelOffset = GetMinimumTexelOffset(featureLevel);
    caps->maxProgramTexelOffset = GetMaximumTexelOffset(featureLevel);

    // Compute shader limits
    caps->maxComputeWorkGroupCount       = GetMaxComputeWorkGroupCount(featureLevel);
    caps->maxComputeWorkGroupSize        = GetMaxComputeWorkGroupSize(featureLevel);
    caps->maxComputeWorkGroupInvocations = GetMaxComputeWorkGroupInvocations(featureLevel);
    caps->maxComputeSharedMemorySize     = GetMaxComputeSharedMemorySize(featureLevel);
    caps->maxShaderUniformComponents[gl::ShaderType::Compute] =
        GetMaximumComputeUniformVectors(featureLevel) * 4;
    caps->maxShaderUniformBlocks[gl::ShaderType::Compute] =
        GetMaximumComputeUniformBlocks(featureLevel);
    caps->maxShaderTextureImageUnits[gl::ShaderType::Compute] =
        GetMaximumComputeTextureUnits(featureLevel);

    SetUAVRelatedResourceLimits(featureLevel, caps);

    // Aggregate shader limits
    caps->maxUniformBufferBindings = caps->maxShaderUniformBlocks[gl::ShaderType::Vertex] +
                                     caps->maxShaderUniformBlocks[gl::ShaderType::Fragment];
    caps->maxUniformBlockSize = static_cast<GLuint64>(GetMaximumConstantBufferSize(featureLevel));

    // TODO(oetuaho): Get a more accurate limit. For now using the minimum requirement for GLES 3.1.
    caps->maxUniformLocations = 1024;

    // With DirectX 11.1, constant buffer offset and size must be a multiple of 16 constants of 16
    // bytes each.
    // https://msdn.microsoft.com/en-us/library/windows/desktop/hh404649%28v=vs.85%29.aspx
    // With DirectX 11.0, we emulate UBO offsets using copies of ranges of the UBO however
    // we still keep the same alignment as 11.1 for consistency.
    caps->uniformBufferOffsetAlignment = 256;

    caps->maxCombinedUniformBlocks = caps->maxShaderUniformBlocks[gl::ShaderType::Vertex] +
                                     caps->maxShaderUniformBlocks[gl::ShaderType::Fragment];

    // A shader storage block will be translated to a structure in HLSL. So We reference the HLSL
    // structure packing rules
    // https://msdn.microsoft.com/en-us/library/windows/desktop/bb509632(v=vs.85).aspx. The
    // resulting size of any structure will always be evenly divisible by sizeof(four-component
    // vector).
    caps->shaderStorageBufferOffsetAlignment = 16;

    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        caps->maxCombinedShaderUniformComponents[shaderType] =
            static_cast<GLint64>(caps->maxShaderUniformBlocks[shaderType]) *
                static_cast<GLint64>(caps->maxUniformBlockSize / 4) +
            static_cast<GLint64>(caps->maxShaderUniformComponents[shaderType]);
    }

    caps->maxVaryingComponents         = GetMaximumVertexOutputVectors(featureLevel) * 4;
    caps->maxVaryingVectors            = GetMaximumVertexOutputVectors(featureLevel);
    caps->maxCombinedTextureImageUnits = caps->maxShaderTextureImageUnits[gl::ShaderType::Vertex] +
                                         caps->maxShaderTextureImageUnits[gl::ShaderType::Fragment];

    // Transform feedback limits
    caps->maxTransformFeedbackInterleavedComponents =
        GetMaximumStreamOutputInterleavedComponents(featureLevel);
    caps->maxTransformFeedbackSeparateAttributes = GetMaximumStreamOutputBuffers(featureLevel);
    caps->maxTransformFeedbackSeparateComponents =
        GetMaximumStreamOutputSeparateComponents(featureLevel);

    // Defer the computation of multisample limits to Context::updateCaps() where max*Samples values
    // are determined according to available sample counts for each individual format.
    caps->maxSamples             = std::numeric_limits<GLint>::max();
    caps->maxColorTextureSamples = std::numeric_limits<GLint>::max();
    caps->maxDepthTextureSamples = std::numeric_limits<GLint>::max();
    caps->maxIntegerSamples      = std::numeric_limits<GLint>::max();

    // Sample mask words limits
    caps->maxSampleMaskWords = GetMaxSampleMaskWords(featureLevel);

    // Framebuffer limits
    caps->maxFramebufferSamples = std::numeric_limits<GLint>::max();
    caps->maxFramebufferWidth   = GetMaximumRenderToBufferWindowSize(featureLevel);
    caps->maxFramebufferHeight  = caps->maxFramebufferWidth;

    // Texture gather offset limits
    caps->minProgramTextureGatherOffset = GetMinimumTextureGatherOffset(featureLevel);
    caps->maxProgramTextureGatherOffset = GetMaximumTextureGatherOffset(featureLevel);

    caps->maxTextureAnisotropy        = GetMaximumAnisotropy(featureLevel);
    caps->queryCounterBitsTimeElapsed = 64;

    caps->queryCounterBitsTimestamp = features.enableTimestampQueries.enabled ? 64 : 0;

    caps->maxDualSourceDrawBuffers = 1;

    // GL extension support
    extensions->setTextureExtensionSupport(*textureCapsMap);

    // Explicitly disable GL_OES_compressed_ETC1_RGB8_texture because it's emulated and never
    // becomes core. WebGL doesn't want to expose it unless there is native support.
    extensions->compressedETC1RGB8TextureOES    = false;
    extensions->compressedETC1RGB8SubTextureEXT = false;

    extensions->elementIndexUintOES         = true;
    extensions->getProgramBinaryOES         = true;
    extensions->rgb8Rgba8OES                = true;
    extensions->readFormatBgraEXT           = true;
    extensions->pixelBufferObjectNV         = true;
    extensions->mapbufferOES                = true;
    extensions->mapBufferRangeEXT           = true;
    extensions->textureNpotOES              = GetNPOTTextureSupport(featureLevel);
    extensions->drawBuffersEXT              = GetMaximumSimultaneousRenderTargets(featureLevel) > 1;
    extensions->drawBuffersIndexedEXT       = (featureLevel >= D3D_FEATURE_LEVEL_10_1);
    extensions->drawBuffersIndexedOES       = extensions->drawBuffersIndexedEXT;
    extensions->textureStorageEXT           = true;
    extensions->textureFilterAnisotropicEXT = true;
    extensions->occlusionQueryBooleanEXT    = GetOcclusionQuerySupport(featureLevel);
    extensions->fenceNV                     = GetEventQuerySupport(featureLevel);
    extensions->disjointTimerQueryEXT       = true;
    extensions->robustnessEXT               = true;
    extensions->robustnessKHR               = true;
    // Direct3D guarantees to return zero for any resource that is accessed out of bounds.
    // See https://msdn.microsoft.com/en-us/library/windows/desktop/ff476332(v=vs.85).aspx
    // and https://msdn.microsoft.com/en-us/library/windows/desktop/ff476900(v=vs.85).aspx
    extensions->robustBufferAccessBehaviorKHR = true;
    extensions->blendMinmaxEXT                = true;
    // https://docs.microsoft.com/en-us/windows/desktop/direct3ddxgi/format-support-for-direct3d-11-0-feature-level-hardware
    extensions->floatBlendEXT               = true;
    extensions->framebufferBlitANGLE        = GetFramebufferBlitSupport(featureLevel);
    extensions->framebufferBlitNV           = extensions->framebufferBlitANGLE;
    extensions->framebufferMultisampleANGLE = GetFramebufferMultisampleSupport(featureLevel);
    extensions->instancedArraysANGLE        = GetInstancingSupport(featureLevel);
    extensions->instancedArraysEXT          = GetInstancingSupport(featureLevel);
    extensions->packReverseRowOrderANGLE    = true;
    extensions->standardDerivativesOES      = GetDerivativeInstructionSupport(featureLevel);
    extensions->shaderTextureLodEXT         = GetShaderTextureLODSupport(featureLevel);
    extensions->fragDepthEXT                = true;
    extensions->conservativeDepthEXT        = (featureLevel >= D3D_FEATURE_LEVEL_11_0);
    extensions->polygonModeANGLE            = true;
    extensions->polygonOffsetClampEXT       = (featureLevel >= D3D_FEATURE_LEVEL_10_0);
    extensions->depthClampEXT               = true;
    extensions->stencilTexturingANGLE       = (featureLevel >= D3D_FEATURE_LEVEL_10_1);
    extensions->multiviewOVR                = IsMultiviewSupported(featureLevel);
    extensions->multiview2OVR               = IsMultiviewSupported(featureLevel);
    if (extensions->multiviewOVR || extensions->multiview2OVR)
    {
        caps->maxViews = std::min(static_cast<GLuint>(GetMaximum2DTextureArraySize(featureLevel)),
                                  GetMaxViewportAndScissorRectanglesPerPipeline(featureLevel));
    }
    extensions->textureUsageANGLE = true;  // This could be false since it has no effect in D3D11
    extensions->discardFramebufferEXT               = true;
    extensions->translatedShaderSourceANGLE         = true;
    extensions->fboRenderMipmapOES                  = true;
    extensions->debugMarkerEXT                      = true;
    extensions->EGLImageOES                         = true;
    extensions->EGLImageExternalOES                 = true;
    extensions->EGLImageExternalWrapModesEXT        = true;
    extensions->EGLImageExternalEssl3OES            = true;
    extensions->EGLStreamConsumerExternalNV         = true;
    extensions->unpackSubimageEXT                   = true;
    extensions->packSubimageNV                      = true;
    extensions->lossyEtcDecodeANGLE                 = true;
    extensions->syncQueryCHROMIUM                   = GetEventQuerySupport(featureLevel);
    extensions->copyTextureCHROMIUM                 = true;
    extensions->copyCompressedTextureCHROMIUM       = true;
    extensions->textureStorageMultisample2dArrayOES = true;
    extensions->textureMirrorClampToEdgeEXT         = true;
    extensions->shaderNoperspectiveInterpolationNV  = (featureLevel >= D3D_FEATURE_LEVEL_10_0);
    extensions->sampleVariablesOES                  = (featureLevel >= D3D_FEATURE_LEVEL_11_0);
    extensions->shaderMultisampleInterpolationOES   = (featureLevel >= D3D_FEATURE_LEVEL_11_0);
    if (extensions->shaderMultisampleInterpolationOES)
    {
        caps->subPixelInterpolationOffsetBits = 4;
        caps->minInterpolationOffset          = -0.5f;
        caps->maxInterpolationOffset          = +0.4375f;  // +0.5 - (2 ^ -4)
    }
    extensions->multiviewMultisampleANGLE =
        ((extensions->multiviewOVR || extensions->multiview2OVR) &&
         extensions->textureStorageMultisample2dArrayOES);
    extensions->copyTexture3dANGLE      = true;
    extensions->textureBorderClampEXT   = true;
    extensions->textureBorderClampOES   = true;
    extensions->multiDrawIndirectEXT    = true;
    extensions->textureMultisampleANGLE = true;
    extensions->provokingVertexANGLE    = true;
    extensions->blendFuncExtendedEXT    = true;
    // http://anglebug.com/42263500
    extensions->texture3DOES                             = false;
    extensions->baseInstanceEXT                          = true;
    extensions->baseVertexBaseInstanceANGLE              = true;
    extensions->baseVertexBaseInstanceShaderBuiltinANGLE = true;
    extensions->drawElementsBaseVertexOES                = true;
    extensions->drawElementsBaseVertexEXT                = true;
    if (!strstr(description, "Adreno"))
    {
        extensions->multisampledRenderToTextureEXT = true;
    }
    extensions->videoTextureWEBGL = true;

    // D3D11 cannot support reading depth texture as a luminance texture.
    // It treats it as a red-channel-only texture.
    extensions->depthTextureOES = false;

    // readPixels on depth & stencil not working with D3D11 backend.
    extensions->readDepthNV         = false;
    extensions->readStencilNV       = false;
    extensions->depthBufferFloat2NV = false;

    // GL_EXT_clip_control
    extensions->clipControlEXT = (featureLevel >= D3D_FEATURE_LEVEL_10_0);

    // GL_APPLE_clip_distance / GL_EXT_clip_cull_distance / GL_ANGLE_clip_cull_distance
    extensions->clipDistanceAPPLE         = true;
    extensions->clipCullDistanceEXT       = true;
    extensions->clipCullDistanceANGLE     = true;
    caps->maxClipDistances                = D3D11_CLIP_OR_CULL_DISTANCE_COUNT;
    caps->maxCullDistances                = D3D11_CLIP_OR_CULL_DISTANCE_COUNT;
    caps->maxCombinedClipAndCullDistances = D3D11_CLIP_OR_CULL_DISTANCE_COUNT;

    // GL_KHR_parallel_shader_compile
    extensions->parallelShaderCompileKHR = true;

    // GL_EXT_texture_buffer
    extensions->textureBufferEXT = HasTextureBufferSupport(device, renderer11DeviceCaps);

    // GL_OES_texture_buffer
    extensions->textureBufferOES = extensions->textureBufferEXT;

    // ANGLE_shader_pixel_local_storage -- fragment shader UAVs appear in D3D 11.0.
    if (featureLevel >= D3D_FEATURE_LEVEL_11_0)
    {
        extensions->shaderPixelLocalStorageANGLE = true;
        plsOptions->type                         = ShPixelLocalStorageType::ImageLoadStore;
        if (renderer11DeviceCaps.supportsRasterizerOrderViews)
        {
            extensions->shaderPixelLocalStorageCoherentANGLE = true;
            plsOptions->fragmentSyncType = ShFragmentSynchronizationType::RasterizerOrderViews_D3D;
        }
        // TODO(anglebug.com/40096838): If we add RG* support to pixel local storage, these are
        // *NOT* in the set of common formats, so we need to query support for each individualy:
        // https://learn.microsoft.com/en-us/windows/win32/direct3d11/typed-unordered-access-view-loads
        plsOptions->supportsNativeRGBA8ImageFormats =
            renderer11DeviceCaps.supportsUAVLoadStoreCommonFormats;
    }

    // D3D11 Feature Level 10_0+ uses SV_IsFrontFace in HLSL to emulate gl_FrontFacing.
    // D3D11 Feature Level 9_3 doesn't support SV_IsFrontFace, and has no equivalent, so can't
    // support gl_FrontFacing.
    limitations->noFrontFacingSupport = (featureLevel <= D3D_FEATURE_LEVEL_9_3);

    // D3D11 Feature Level 9_3 doesn't support alpha-to-coverage
    limitations->noSampleAlphaToCoverageSupport = (featureLevel <= D3D_FEATURE_LEVEL_9_3);

    // D3D11 has no concept of separate masks and refs for front and back faces in the depth stencil
    // state.
    limitations->noSeparateStencilRefsAndMasks = true;

    // D3D11 cannot support constant color and alpha blend funcs together
    limitations->noSimultaneousConstantColorAndAlphaBlendFunc = true;

    // D3D11 does not support multiple transform feedback outputs writing to the same buffer.
    limitations->noDoubleBoundTransformFeedbackBuffers = true;

    // D3D11 does not support vertex attribute aliasing
    limitations->noVertexAttributeAliasing = true;

    // D3D11 does not support compressed textures where the base mip level is not a multiple of 4
    limitations->compressedBaseMipLevelMultipleOfFour = true;

    if (extensions->textureBufferAny())
    {
        caps->maxTextureBufferSize = 1 << D3D11_REQ_BUFFER_RESOURCE_TEXEL_COUNT_2_TO_EXP;
        // this maybe touble for RGB32 format.
        caps->textureBufferOffsetAlignment = 16;
    }

#ifdef ANGLE_ENABLE_WINDOWS_UWP
    // Setting a non-zero divisor on attribute zero doesn't work on certain Windows Phone 8-era
    // devices. We should prevent developers from doing this on ALL Windows Store devices. This will
    // maintain consistency across all Windows devices. We allow non-zero divisors on attribute zero
    // if the Client Version >= 3, since devices affected by this issue don't support ES3+.
    limitations->attributeZeroRequiresZeroDivisorInEXT = true;
#endif
}

}  // namespace d3d11_gl

namespace gl_d3d11
{

D3D11_BLEND ConvertBlendFunc(gl::BlendFactorType glBlend, bool isAlpha)
{
    D3D11_BLEND d3dBlend = D3D11_BLEND_ZERO;

    switch (glBlend)
    {
        case gl::BlendFactorType::Zero:
            d3dBlend = D3D11_BLEND_ZERO;
            break;
        case gl::BlendFactorType::One:
            d3dBlend = D3D11_BLEND_ONE;
            break;
        case gl::BlendFactorType::SrcColor:
            d3dBlend = (isAlpha ? D3D11_BLEND_SRC_ALPHA : D3D11_BLEND_SRC_COLOR);
            break;
        case gl::BlendFactorType::OneMinusSrcColor:
            d3dBlend = (isAlpha ? D3D11_BLEND_INV_SRC_ALPHA : D3D11_BLEND_INV_SRC_COLOR);
            break;
        case gl::BlendFactorType::DstColor:
            d3dBlend = (isAlpha ? D3D11_BLEND_DEST_ALPHA : D3D11_BLEND_DEST_COLOR);
            break;
        case gl::BlendFactorType::OneMinusDstColor:
            d3dBlend = (isAlpha ? D3D11_BLEND_INV_DEST_ALPHA : D3D11_BLEND_INV_DEST_COLOR);
            break;
        case gl::BlendFactorType::SrcAlpha:
            d3dBlend = D3D11_BLEND_SRC_ALPHA;
            break;
        case gl::BlendFactorType::OneMinusSrcAlpha:
            d3dBlend = D3D11_BLEND_INV_SRC_ALPHA;
            break;
        case gl::BlendFactorType::DstAlpha:
            d3dBlend = D3D11_BLEND_DEST_ALPHA;
            break;
        case gl::BlendFactorType::OneMinusDstAlpha:
            d3dBlend = D3D11_BLEND_INV_DEST_ALPHA;
            break;
        case gl::BlendFactorType::ConstantColor:
            d3dBlend = D3D11_BLEND_BLEND_FACTOR;
            break;
        case gl::BlendFactorType::OneMinusConstantColor:
            d3dBlend = D3D11_BLEND_INV_BLEND_FACTOR;
            break;
        case gl::BlendFactorType::ConstantAlpha:
            d3dBlend = D3D11_BLEND_BLEND_FACTOR;
            break;
        case gl::BlendFactorType::OneMinusConstantAlpha:
            d3dBlend = D3D11_BLEND_INV_BLEND_FACTOR;
            break;
        case gl::BlendFactorType::SrcAlphaSaturate:
            d3dBlend = D3D11_BLEND_SRC_ALPHA_SAT;
            break;
        case gl::BlendFactorType::Src1Color:
            d3dBlend = (isAlpha ? D3D11_BLEND_SRC1_ALPHA : D3D11_BLEND_SRC1_COLOR);
            break;
        case gl::BlendFactorType::Src1Alpha:
            d3dBlend = D3D11_BLEND_SRC1_ALPHA;
            break;
        case gl::BlendFactorType::OneMinusSrc1Color:
            d3dBlend = (isAlpha ? D3D11_BLEND_INV_SRC1_ALPHA : D3D11_BLEND_INV_SRC1_COLOR);
            break;
        case gl::BlendFactorType::OneMinusSrc1Alpha:
            d3dBlend = D3D11_BLEND_INV_SRC1_ALPHA;
            break;
        default:
            UNREACHABLE();
    }

    return d3dBlend;
}

D3D11_BLEND_OP ConvertBlendOp(gl::BlendEquationType glBlendOp)
{
    D3D11_BLEND_OP d3dBlendOp = D3D11_BLEND_OP_ADD;

    switch (glBlendOp)
    {
        case gl::BlendEquationType::Add:
            d3dBlendOp = D3D11_BLEND_OP_ADD;
            break;
        case gl::BlendEquationType::Subtract:
            d3dBlendOp = D3D11_BLEND_OP_SUBTRACT;
            break;
        case gl::BlendEquationType::ReverseSubtract:
            d3dBlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
            break;
        case gl::BlendEquationType::Min:
            d3dBlendOp = D3D11_BLEND_OP_MIN;
            break;
        case gl::BlendEquationType::Max:
            d3dBlendOp = D3D11_BLEND_OP_MAX;
            break;
        default:
            UNREACHABLE();
    }

    return d3dBlendOp;
}

UINT8 ConvertColorMask(bool red, bool green, bool blue, bool alpha)
{
    UINT8 mask = 0;
    if (red)
    {
        mask |= D3D11_COLOR_WRITE_ENABLE_RED;
    }
    if (green)
    {
        mask |= D3D11_COLOR_WRITE_ENABLE_GREEN;
    }
    if (blue)
    {
        mask |= D3D11_COLOR_WRITE_ENABLE_BLUE;
    }
    if (alpha)
    {
        mask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;
    }
    return mask;
}

D3D11_CULL_MODE ConvertCullMode(bool cullEnabled, gl::CullFaceMode cullMode)
{
    D3D11_CULL_MODE cull = D3D11_CULL_NONE;

    if (cullEnabled)
    {
        switch (cullMode)
        {
            case gl::CullFaceMode::Front:
                cull = D3D11_CULL_FRONT;
                break;
            case gl::CullFaceMode::Back:
                cull = D3D11_CULL_BACK;
                break;
            case gl::CullFaceMode::FrontAndBack:
                cull = D3D11_CULL_NONE;
                break;
            default:
                UNREACHABLE();
        }
    }
    else
    {
        cull = D3D11_CULL_NONE;
    }

    return cull;
}

D3D11_COMPARISON_FUNC ConvertComparison(GLenum comparison)
{
    D3D11_COMPARISON_FUNC d3dComp = D3D11_COMPARISON_NEVER;
    switch (comparison)
    {
        case GL_NEVER:
            d3dComp = D3D11_COMPARISON_NEVER;
            break;
        case GL_ALWAYS:
            d3dComp = D3D11_COMPARISON_ALWAYS;
            break;
        case GL_LESS:
            d3dComp = D3D11_COMPARISON_LESS;
            break;
        case GL_LEQUAL:
            d3dComp = D3D11_COMPARISON_LESS_EQUAL;
            break;
        case GL_EQUAL:
            d3dComp = D3D11_COMPARISON_EQUAL;
            break;
        case GL_GREATER:
            d3dComp = D3D11_COMPARISON_GREATER;
            break;
        case GL_GEQUAL:
            d3dComp = D3D11_COMPARISON_GREATER_EQUAL;
            break;
        case GL_NOTEQUAL:
            d3dComp = D3D11_COMPARISON_NOT_EQUAL;
            break;
        default:
            UNREACHABLE();
    }

    return d3dComp;
}

D3D11_DEPTH_WRITE_MASK ConvertDepthMask(bool depthWriteEnabled)
{
    return depthWriteEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
}

UINT8 ConvertStencilMask(GLuint stencilmask)
{
    return static_cast<UINT8>(stencilmask);
}

D3D11_STENCIL_OP ConvertStencilOp(GLenum stencilOp)
{
    D3D11_STENCIL_OP d3dStencilOp = D3D11_STENCIL_OP_KEEP;

    switch (stencilOp)
    {
        case GL_ZERO:
            d3dStencilOp = D3D11_STENCIL_OP_ZERO;
            break;
        case GL_KEEP:
            d3dStencilOp = D3D11_STENCIL_OP_KEEP;
            break;
        case GL_REPLACE:
            d3dStencilOp = D3D11_STENCIL_OP_REPLACE;
            break;
        case GL_INCR:
            d3dStencilOp = D3D11_STENCIL_OP_INCR_SAT;
            break;
        case GL_DECR:
            d3dStencilOp = D3D11_STENCIL_OP_DECR_SAT;
            break;
        case GL_INVERT:
            d3dStencilOp = D3D11_STENCIL_OP_INVERT;
            break;
        case GL_INCR_WRAP:
            d3dStencilOp = D3D11_STENCIL_OP_INCR;
            break;
        case GL_DECR_WRAP:
            d3dStencilOp = D3D11_STENCIL_OP_DECR;
            break;
        default:
            UNREACHABLE();
    }

    return d3dStencilOp;
}

D3D11_FILTER ConvertFilter(GLenum minFilter,
                           GLenum magFilter,
                           float maxAnisotropy,
                           GLenum comparisonMode)
{
    bool comparison = comparisonMode != GL_NONE;

    if (maxAnisotropy > 1.0f)
    {
        return D3D11_ENCODE_ANISOTROPIC_FILTER(static_cast<D3D11_COMPARISON_FUNC>(comparison));
    }
    else
    {
        D3D11_FILTER_TYPE dxMin = D3D11_FILTER_TYPE_POINT;
        D3D11_FILTER_TYPE dxMip = D3D11_FILTER_TYPE_POINT;
        switch (minFilter)
        {
            case GL_NEAREST:
                dxMin = D3D11_FILTER_TYPE_POINT;
                dxMip = D3D11_FILTER_TYPE_POINT;
                break;
            case GL_LINEAR:
                dxMin = D3D11_FILTER_TYPE_LINEAR;
                dxMip = D3D11_FILTER_TYPE_POINT;
                break;
            case GL_NEAREST_MIPMAP_NEAREST:
                dxMin = D3D11_FILTER_TYPE_POINT;
                dxMip = D3D11_FILTER_TYPE_POINT;
                break;
            case GL_LINEAR_MIPMAP_NEAREST:
                dxMin = D3D11_FILTER_TYPE_LINEAR;
                dxMip = D3D11_FILTER_TYPE_POINT;
                break;
            case GL_NEAREST_MIPMAP_LINEAR:
                dxMin = D3D11_FILTER_TYPE_POINT;
                dxMip = D3D11_FILTER_TYPE_LINEAR;
                break;
            case GL_LINEAR_MIPMAP_LINEAR:
                dxMin = D3D11_FILTER_TYPE_LINEAR;
                dxMip = D3D11_FILTER_TYPE_LINEAR;
                break;
            default:
                UNREACHABLE();
        }

        D3D11_FILTER_TYPE dxMag = D3D11_FILTER_TYPE_POINT;
        switch (magFilter)
        {
            case GL_NEAREST:
                dxMag = D3D11_FILTER_TYPE_POINT;
                break;
            case GL_LINEAR:
                dxMag = D3D11_FILTER_TYPE_LINEAR;
                break;
            default:
                UNREACHABLE();
        }

        return D3D11_ENCODE_BASIC_FILTER(dxMin, dxMag, dxMip,
                                         static_cast<D3D11_COMPARISON_FUNC>(comparison));
    }
}

D3D11_TEXTURE_ADDRESS_MODE ConvertTextureWrap(GLenum wrap)
{
    switch (wrap)
    {
        case GL_REPEAT:
            return D3D11_TEXTURE_ADDRESS_WRAP;
        case GL_MIRRORED_REPEAT:
            return D3D11_TEXTURE_ADDRESS_MIRROR;
        case GL_CLAMP_TO_EDGE:
            return D3D11_TEXTURE_ADDRESS_CLAMP;
        case GL_CLAMP_TO_BORDER:
            return D3D11_TEXTURE_ADDRESS_BORDER;
        case GL_MIRROR_CLAMP_TO_EDGE_EXT:
            return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
        default:
            UNREACHABLE();
    }

    return D3D11_TEXTURE_ADDRESS_WRAP;
}

UINT ConvertMaxAnisotropy(float maxAnisotropy, D3D_FEATURE_LEVEL featureLevel)
{
    return static_cast<UINT>(std::min(maxAnisotropy, d3d11_gl::GetMaximumAnisotropy(featureLevel)));
}

D3D11_QUERY ConvertQueryType(gl::QueryType type)
{
    switch (type)
    {
        case gl::QueryType::AnySamples:
        case gl::QueryType::AnySamplesConservative:
            return D3D11_QUERY_OCCLUSION;
        case gl::QueryType::TransformFeedbackPrimitivesWritten:
            return D3D11_QUERY_SO_STATISTICS;
        case gl::QueryType::TimeElapsed:
            // Two internal queries are also created for begin/end timestamps
            return D3D11_QUERY_TIMESTAMP_DISJOINT;
        case gl::QueryType::Timestamp:
            // A disjoint query is also created for timestamp
            return D3D11_QUERY_TIMESTAMP_DISJOINT;
        case gl::QueryType::CommandsCompleted:
            return D3D11_QUERY_EVENT;
        default:
            UNREACHABLE();
            return D3D11_QUERY_EVENT;
    }
}

// Get the D3D11 write mask covering all color channels of a given format
UINT8 GetColorMask(const gl::InternalFormat &format)
{
    return ConvertColorMask(format.redBits > 0, format.greenBits > 0, format.blueBits > 0,
                            format.alphaBits > 0);
}

}  // namespace gl_d3d11

namespace d3d11
{

ANGLED3D11DeviceType GetDeviceType(ID3D11Device *device)
{
    // Note that this function returns an ANGLED3D11DeviceType rather than a D3D_DRIVER_TYPE value,
    // since it is difficult to tell Software and Reference devices apart

    IDXGIDevice *dxgiDevice     = nullptr;
    IDXGIAdapter *dxgiAdapter   = nullptr;
    IDXGIAdapter2 *dxgiAdapter2 = nullptr;

    ANGLED3D11DeviceType retDeviceType = ANGLE_D3D11_DEVICE_TYPE_UNKNOWN;

    HRESULT hr = device->QueryInterface(__uuidof(IDXGIDevice), (void **)&dxgiDevice);
    if (SUCCEEDED(hr))
    {
        hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void **)&dxgiAdapter);
        if (SUCCEEDED(hr))
        {
            std::wstring adapterString;
            HRESULT adapter2hr =
                dxgiAdapter->QueryInterface(__uuidof(dxgiAdapter2), (void **)&dxgiAdapter2);
            if (SUCCEEDED(adapter2hr))
            {
                // On D3D_FEATURE_LEVEL_9_*, IDXGIAdapter::GetDesc returns "Software Adapter"
                // for the description string. Try to use IDXGIAdapter2::GetDesc2 to get the
                // actual hardware values if possible.
                DXGI_ADAPTER_DESC2 adapterDesc2;
                dxgiAdapter2->GetDesc2(&adapterDesc2);
                adapterString = std::wstring(adapterDesc2.Description);
            }
            else
            {
                DXGI_ADAPTER_DESC adapterDesc;
                dxgiAdapter->GetDesc(&adapterDesc);
                adapterString = std::wstring(adapterDesc.Description);
            }

            // Both Reference and Software adapters will be 'Software Adapter'
            const bool isSoftwareDevice =
                (adapterString.find(std::wstring(L"Software Adapter")) != std::string::npos);
            const bool isNullDevice = (adapterString == L"");
            const bool isWARPDevice =
                (adapterString.find(std::wstring(L"Basic Render")) != std::string::npos);

            if (isSoftwareDevice || isNullDevice)
            {
                ASSERT(!isWARPDevice);
                retDeviceType = ANGLE_D3D11_DEVICE_TYPE_SOFTWARE_REF_OR_NULL;
            }
            else if (isWARPDevice)
            {
                retDeviceType = ANGLE_D3D11_DEVICE_TYPE_WARP;
            }
            else
            {
                retDeviceType = ANGLE_D3D11_DEVICE_TYPE_HARDWARE;
            }
        }
    }

    SafeRelease(dxgiDevice);
    SafeRelease(dxgiAdapter);
    SafeRelease(dxgiAdapter2);

    return retDeviceType;
}

void MakeValidSize(bool isImage,
                   DXGI_FORMAT format,
                   GLsizei *requestWidth,
                   GLsizei *requestHeight,
                   int *levelOffset)
{
    const DXGIFormatSize &dxgiFormatInfo = d3d11::GetDXGIFormatSizeInfo(format);
    bool validFormat                     = format != DXGI_FORMAT_UNKNOWN;
    bool validImage                      = isImage && validFormat;

    int upsampleCount = 0;
    // Don't expand the size of full textures that are at least (blockWidth x blockHeight) already.
    if (validImage || *requestWidth < static_cast<GLsizei>(dxgiFormatInfo.blockWidth) ||
        *requestHeight < static_cast<GLsizei>(dxgiFormatInfo.blockHeight))
    {
        while (*requestWidth % dxgiFormatInfo.blockWidth != 0 ||
               *requestHeight % dxgiFormatInfo.blockHeight != 0)
        {
            *requestWidth <<= 1;
            *requestHeight <<= 1;
            upsampleCount++;
        }
    }
    else if (validFormat)
    {
        if (*requestWidth % dxgiFormatInfo.blockWidth != 0)
        {
            *requestWidth = roundUp(*requestWidth, static_cast<GLsizei>(dxgiFormatInfo.blockWidth));
        }

        if (*requestHeight % dxgiFormatInfo.blockHeight != 0)
        {
            *requestHeight =
                roundUp(*requestHeight, static_cast<GLsizei>(dxgiFormatInfo.blockHeight));
        }
    }

    if (levelOffset)
    {
        *levelOffset = upsampleCount;
    }
}

angle::Result GenerateInitialTextureData(
    const gl::Context *context,
    GLint internalFormat,
    const Renderer11DeviceCaps &renderer11DeviceCaps,
    GLuint width,
    GLuint height,
    GLuint depth,
    GLuint mipLevels,
    gl::TexLevelArray<D3D11_SUBRESOURCE_DATA> *outSubresourceData)
{
    const d3d11::Format &d3dFormatInfo = d3d11::Format::Get(internalFormat, renderer11DeviceCaps);
    ASSERT(d3dFormatInfo.dataInitializerFunction != nullptr);

    const d3d11::DXGIFormatSize &dxgiFormatInfo =
        d3d11::GetDXGIFormatSizeInfo(d3dFormatInfo.texFormat);

    using CheckedSize        = angle::CheckedNumeric<size_t>;
    CheckedSize rowPitch     = CheckedSize(dxgiFormatInfo.pixelBytes) * CheckedSize(width);
    CheckedSize depthPitch   = rowPitch * CheckedSize(height);
    CheckedSize maxImageSize = depthPitch * CheckedSize(depth);

    Context11 *context11 = GetImplAs<Context11>(context);
    ANGLE_CHECK_GL_ALLOC(context11, maxImageSize.IsValid());

    angle::MemoryBuffer *scratchBuffer = nullptr;
    ANGLE_CHECK_GL_ALLOC(context11,
                         context->getScratchBuffer(maxImageSize.ValueOrDie(), &scratchBuffer));

    d3dFormatInfo.dataInitializerFunction(width, height, depth, scratchBuffer->data(),
                                          rowPitch.ValueOrDie(), depthPitch.ValueOrDie());

    for (unsigned int i = 0; i < mipLevels; i++)
    {
        unsigned int mipWidth  = std::max(width >> i, 1U);
        unsigned int mipHeight = std::max(height >> i, 1U);

        using CheckedUINT         = angle::CheckedNumeric<UINT>;
        CheckedUINT mipRowPitch   = CheckedUINT(dxgiFormatInfo.pixelBytes) * CheckedUINT(mipWidth);
        CheckedUINT mipDepthPitch = mipRowPitch * CheckedUINT(mipHeight);

        ANGLE_CHECK_GL_ALLOC(context11, mipRowPitch.IsValid() && mipDepthPitch.IsValid());

        outSubresourceData->at(i).pSysMem          = scratchBuffer->data();
        outSubresourceData->at(i).SysMemPitch      = mipRowPitch.ValueOrDie();
        outSubresourceData->at(i).SysMemSlicePitch = mipDepthPitch.ValueOrDie();
    }

    return angle::Result::Continue;
}

UINT GetPrimitiveRestartIndex()
{
    return std::numeric_limits<UINT>::max();
}

void SetPositionTexCoordVertex(PositionTexCoordVertex *vertex, float x, float y, float u, float v)
{
    vertex->x = x;
    vertex->y = y;
    vertex->u = u;
    vertex->v = v;
}

void SetPositionLayerTexCoord3DVertex(PositionLayerTexCoord3DVertex *vertex,
                                      float x,
                                      float y,
                                      unsigned int layer,
                                      float u,
                                      float v,
                                      float s)
{
    vertex->x = x;
    vertex->y = y;
    vertex->l = layer;
    vertex->u = u;
    vertex->v = v;
    vertex->s = s;
}

BlendStateKey::BlendStateKey()
{
    memset(this, 0, sizeof(BlendStateKey));
    blendStateExt = gl::BlendStateExt();
}

BlendStateKey::BlendStateKey(const BlendStateKey &other)
{
    memcpy(this, &other, sizeof(BlendStateKey));
}

bool operator==(const BlendStateKey &a, const BlendStateKey &b)
{
    return memcmp(&a, &b, sizeof(BlendStateKey)) == 0;
}

bool operator!=(const BlendStateKey &a, const BlendStateKey &b)
{
    return !(a == b);
}

RasterizerStateKey::RasterizerStateKey()
{
    memset(this, 0, sizeof(RasterizerStateKey));
}

bool operator==(const RasterizerStateKey &a, const RasterizerStateKey &b)
{
    return memcmp(&a, &b, sizeof(RasterizerStateKey)) == 0;
}

bool operator!=(const RasterizerStateKey &a, const RasterizerStateKey &b)
{
    return !(a == b);
}

HRESULT SetDebugName(ID3D11DeviceChild *resource,
                     const char *internalName,
                     const std::string *khrDebugName)
{
    // Prepend ANGLE to separate names from other components in the same process.
    std::string d3dName = "ANGLE";
    bool sendNameToD3D  = false;
    if (internalName && internalName[0] != '\0')
    {
        d3dName += std::string("_") + internalName;
        sendNameToD3D = true;
    }
    if (khrDebugName && !khrDebugName->empty())
    {
        d3dName += std::string("_") + *khrDebugName;
        sendNameToD3D = true;
    }
    // If both internalName and khrDebugName are empty, avoid sending the string to d3d.
    if (sendNameToD3D)
    {
        return resource->SetPrivateData(WKPDID_D3DDebugObjectName,
                                        static_cast<UINT>(d3dName.size()), d3dName.c_str());
    }
    return S_OK;
}

// Keep this in cpp file where it has visibility of Renderer11.h, otherwise calling
// allocateResource is only compatible with Clang and MSVS, which support calling a
// method on a forward declared class in a template.
template <ResourceType ResourceT>
angle::Result LazyResource<ResourceT>::resolveImpl(d3d::Context *context,
                                                   Renderer11 *renderer,
                                                   const GetDescType<ResourceT> &desc,
                                                   GetInitDataType<ResourceT> *initData,
                                                   const char *name)
{
    if (!mResource.valid())
    {
        ANGLE_TRY(renderer->allocateResource(context, desc, initData, &mResource));
        mResource.setInternalName(name);
    }
    return angle::Result::Continue;
}

template angle::Result LazyResource<ResourceType::BlendState>::resolveImpl(
    d3d::Context *context,
    Renderer11 *renderer,
    const D3D11_BLEND_DESC &desc,
    void *initData,
    const char *name);
template angle::Result LazyResource<ResourceType::ComputeShader>::resolveImpl(
    d3d::Context *context,
    Renderer11 *renderer,
    const ShaderData &desc,
    void *initData,
    const char *name);
template angle::Result LazyResource<ResourceType::GeometryShader>::resolveImpl(
    d3d::Context *context,
    Renderer11 *renderer,
    const ShaderData &desc,
    const std::vector<D3D11_SO_DECLARATION_ENTRY> *initData,
    const char *name);
template angle::Result LazyResource<ResourceType::InputLayout>::resolveImpl(
    d3d::Context *context,
    Renderer11 *renderer,
    const InputElementArray &desc,
    const ShaderData *initData,
    const char *name);
template angle::Result LazyResource<ResourceType::PixelShader>::resolveImpl(d3d::Context *context,
                                                                            Renderer11 *renderer,
                                                                            const ShaderData &desc,
                                                                            void *initData,
                                                                            const char *name);
template angle::Result LazyResource<ResourceType::VertexShader>::resolveImpl(d3d::Context *context,
                                                                             Renderer11 *renderer,
                                                                             const ShaderData &desc,
                                                                             void *initData,
                                                                             const char *name);

LazyInputLayout::LazyInputLayout(const D3D11_INPUT_ELEMENT_DESC *inputDesc,
                                 size_t inputDescLen,
                                 const BYTE *byteCode,
                                 size_t byteCodeLen,
                                 const char *debugName)
    : mInputDesc(inputDesc, inputDescLen), mByteCode(byteCode, byteCodeLen), mDebugName(debugName)
{}

LazyInputLayout::~LazyInputLayout() {}

angle::Result LazyInputLayout::resolve(d3d::Context *context, Renderer11 *renderer)
{
    return resolveImpl(context, renderer, mInputDesc, &mByteCode, mDebugName);
}

LazyBlendState::LazyBlendState(const D3D11_BLEND_DESC &desc, const char *debugName)
    : mDesc(desc), mDebugName(debugName)
{}

angle::Result LazyBlendState::resolve(d3d::Context *context, Renderer11 *renderer)
{
    return resolveImpl(context, renderer, mDesc, nullptr, mDebugName);
}

void InitializeFeatures(const Renderer11DeviceCaps &deviceCaps,
                        const DXGI_ADAPTER_DESC &adapterDesc,
                        angle::FeaturesD3D *features)
{
    bool isNvidia          = IsNvidia(adapterDesc.VendorId);
    bool isIntel           = IsIntel(adapterDesc.VendorId);
    bool isSkylake         = false;
    bool isBroadwell       = false;
    bool isHaswell         = false;
    bool isIvyBridge       = false;
    bool isAMD             = IsAMD(adapterDesc.VendorId);
    bool isFeatureLevel9_3 = deviceCaps.featureLevel <= D3D_FEATURE_LEVEL_9_3;

    angle::VersionTriple capsVersion;
    if (isIntel)
    {
        capsVersion = d3d11_gl::GetIntelDriverVersion(deviceCaps.driverVersion);

        isSkylake   = IsSkylake(adapterDesc.DeviceId);
        isBroadwell = IsBroadwell(adapterDesc.DeviceId);
        isHaswell   = IsHaswell(adapterDesc.DeviceId);
        isIvyBridge = IsIvyBridge(adapterDesc.DeviceId);
    }

    if (isNvidia)
    {
        // TODO(jmadill): Narrow problematic driver range.
        bool driverVersionValid = deviceCaps.driverVersion.valid();
        if (driverVersionValid)
        {
            angle::VersionTriple driverVersion(HIWORD(deviceCaps.driverVersion.value().LowPart),
                                               LOWORD(deviceCaps.driverVersion.value().LowPart), 0);

            // Disable the workaround to fix a second driver bug on newer NVIDIA.
            ANGLE_FEATURE_CONDITION(features, depthStencilBlitExtraCopy,
                                    driverVersion < angle::VersionTriple(13, 6881, 0) && isNvidia &&
                                        driverVersionValid);
        }
        else
        {
            ANGLE_FEATURE_CONDITION(features, depthStencilBlitExtraCopy,
                                    isNvidia && !driverVersionValid);
        }
    }

    ANGLE_FEATURE_CONDITION(features, mrtPerfWorkaround, true);
    ANGLE_FEATURE_CONDITION(features, zeroMaxLodWorkaround, isFeatureLevel9_3);
    ANGLE_FEATURE_CONDITION(features, allowES3OnFL100, false);

    // TODO(jmadill): Disable workaround when we have a fixed compiler DLL.
    ANGLE_FEATURE_CONDITION(features, expandIntegerPowExpressions, true);

    ANGLE_FEATURE_CONDITION(features, flushAfterEndingTransformFeedback, isNvidia);
    ANGLE_FEATURE_CONDITION(features, getDimensionsIgnoresBaseLevel, isNvidia);
    ANGLE_FEATURE_CONDITION(features, skipVSConstantRegisterZero, isNvidia);
    ANGLE_FEATURE_CONDITION(features, forceAtomicValueResolution, isNvidia);

    ANGLE_FEATURE_CONDITION(features, preAddTexelFetchOffsets, isIntel);
    ANGLE_FEATURE_CONDITION(features, useSystemMemoryForConstantBuffers, isIntel);

    ANGLE_FEATURE_CONDITION(features, callClearTwice,
                            isIntel && isSkylake && capsVersion >= angle::VersionTriple(16, 0, 0) &&
                                capsVersion < angle::VersionTriple(16, 4771, 0));
    ANGLE_FEATURE_CONDITION(features, emulateIsnanFloat,
                            isIntel && isSkylake && capsVersion >= angle::VersionTriple(16, 0, 0) &&
                                capsVersion < angle::VersionTriple(16, 4542, 0));
    ANGLE_FEATURE_CONDITION(features, rewriteUnaryMinusOperator,
                            isIntel && (isBroadwell || isHaswell) &&
                                capsVersion >= angle::VersionTriple(15, 0, 0) &&
                                capsVersion < angle::VersionTriple(15, 4624, 0));

    ANGLE_FEATURE_CONDITION(features, addMockTextureNoRenderTarget,
                            isIntel && capsVersion >= angle::VersionTriple(16, 0, 0) &&
                                capsVersion < angle::VersionTriple(16, 4815, 0));

    // Haswell/Ivybridge drivers occasionally corrupt (small?) (vertex?) texture data uploads.
    ANGLE_FEATURE_CONDITION(features, setDataFasterThanImageUpload,
                            !(isIvyBridge || isBroadwell || isHaswell));

    ANGLE_FEATURE_CONDITION(features, disableB5G6R5Support,
                            (isIntel && capsVersion >= angle::VersionTriple(15, 0, 0) &&
                             capsVersion < angle::VersionTriple(15, 4539, 0)) ||
                                isAMD);

    // TODO(jmadill): Disable when we have a fixed driver version.
    // The tiny stencil texture workaround involves using CopySubresource or UpdateSubresource on a
    // depth stencil texture.  This is not allowed until feature level 10.1 but since it is not
    // possible to support ES3 on these devices, there is no need for the workaround to begin with
    // (anglebug.com/42260536).
    ANGLE_FEATURE_CONDITION(features, emulateTinyStencilTextures,
                            isAMD && !(deviceCaps.featureLevel < D3D_FEATURE_LEVEL_10_1));

    // If the VPAndRTArrayIndexFromAnyShaderFeedingRasterizer feature is not available, we have to
    // select the viewport / RT array index in the geometry shader.
    ANGLE_FEATURE_CONDITION(features, selectViewInGeometryShader,
                            !deviceCaps.supportsVpRtIndexWriteFromVertexShader);

    // NVidia drivers have no trouble clearing textures without showing corruption.
    // Intel and AMD drivers that have trouble have been blocklisted by Chromium. In the case of
    // Intel, they've been blocklisted to the DX9 runtime.
    ANGLE_FEATURE_CONDITION(features, allowClearForRobustResourceInit, true);

    // Allow translating uniform block to StructuredBuffer on Windows 10. This is targeted
    // to work around a slow fxc compile performance issue with dynamic uniform indexing.
    ANGLE_FEATURE_CONDITION(features, allowTranslateUniformBlockToStructuredBuffer,
                            IsWindows10OrLater());

    // D3D11 Feature Levels 9_3 and below do not support non-constant loop indexing and require
    // additional
    // pre-validation of the shader at compile time to produce a better error message.
    ANGLE_FEATURE_CONDITION(features, supportsNonConstantLoopIndexing, !isFeatureLevel9_3);
}

void InitializeFrontendFeatures(const DXGI_ADAPTER_DESC &adapterDesc,
                                angle::FrontendFeatures *features)
{
    bool isAMD = IsAMD(adapterDesc.VendorId);

    ANGLE_FEATURE_CONDITION(features, forceDepthAttachmentInitOnClear, isAMD);

    // The D3D backend's handling of compile and link is thread-safe
    ANGLE_FEATURE_CONDITION(features, compileJobIsThreadSafe, true);
    ANGLE_FEATURE_CONDITION(features, linkJobIsThreadSafe, true);
}

void InitConstantBufferDesc(D3D11_BUFFER_DESC *constantBufferDescription, size_t byteWidth)
{
    constantBufferDescription->ByteWidth           = static_cast<UINT>(byteWidth);
    constantBufferDescription->Usage               = D3D11_USAGE_DYNAMIC;
    constantBufferDescription->BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDescription->CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
    constantBufferDescription->MiscFlags           = 0;
    constantBufferDescription->StructureByteStride = 0;
}

}  // namespace d3d11

// TextureHelper11 implementation.
TextureHelper11::TextureHelper11() : mFormatSet(nullptr), mSampleCount(0) {}

TextureHelper11::TextureHelper11(TextureHelper11 &&toCopy) : TextureHelper11()
{
    *this = std::move(toCopy);
}

TextureHelper11::TextureHelper11(const TextureHelper11 &other)
    : mFormatSet(other.mFormatSet), mExtents(other.mExtents), mSampleCount(other.mSampleCount)
{
    mData = other.mData;
}

TextureHelper11::~TextureHelper11() {}

void TextureHelper11::getDesc(D3D11_TEXTURE2D_DESC *desc) const
{
    static_cast<ID3D11Texture2D *>(mData->object)->GetDesc(desc);
}

void TextureHelper11::getDesc(D3D11_TEXTURE3D_DESC *desc) const
{
    static_cast<ID3D11Texture3D *>(mData->object)->GetDesc(desc);
}

void TextureHelper11::getDesc(D3D11_BUFFER_DESC *desc) const
{
    static_cast<ID3D11Buffer *>(mData->object)->GetDesc(desc);
}

void TextureHelper11::initDesc(const D3D11_TEXTURE2D_DESC &desc2D)
{
    mData->resourceType = ResourceType::Texture2D;
    mExtents.width      = static_cast<int>(desc2D.Width);
    mExtents.height     = static_cast<int>(desc2D.Height);
    mExtents.depth      = 1;
    mSampleCount        = desc2D.SampleDesc.Count;
}

void TextureHelper11::initDesc(const D3D11_TEXTURE3D_DESC &desc3D)
{
    mData->resourceType = ResourceType::Texture3D;
    mExtents.width      = static_cast<int>(desc3D.Width);
    mExtents.height     = static_cast<int>(desc3D.Height);
    mExtents.depth      = static_cast<int>(desc3D.Depth);
    mSampleCount        = 1;
}

void TextureHelper11::initDesc(const D3D11_BUFFER_DESC &descBuffer)
{
    mData->resourceType = ResourceType::Buffer;
    mExtents.width      = static_cast<int>(descBuffer.ByteWidth);
    mExtents.height     = 1;
    mExtents.depth      = 1;
    mSampleCount        = 1;
}

TextureHelper11 &TextureHelper11::operator=(TextureHelper11 &&other)
{
    std::swap(mData, other.mData);
    std::swap(mExtents, other.mExtents);
    std::swap(mFormatSet, other.mFormatSet);
    std::swap(mSampleCount, other.mSampleCount);
    return *this;
}

TextureHelper11 &TextureHelper11::operator=(const TextureHelper11 &other)
{
    mData        = other.mData;
    mExtents     = other.mExtents;
    mFormatSet   = other.mFormatSet;
    mSampleCount = other.mSampleCount;
    return *this;
}

bool TextureHelper11::operator==(const TextureHelper11 &other) const
{
    return mData->object == other.mData->object;
}

bool TextureHelper11::operator!=(const TextureHelper11 &other) const
{
    return mData->object != other.mData->object;
}

bool UsePresentPathFast(const Renderer11 *renderer,
                        const gl::FramebufferAttachment *framebufferAttachment)
{
    if (framebufferAttachment == nullptr)
    {
        return false;
    }

    return (framebufferAttachment->type() == GL_FRAMEBUFFER_DEFAULT &&
            renderer->presentPathFastEnabled());
}

bool UsePrimitiveRestartWorkaround(bool primitiveRestartFixedIndexEnabled,
                                   gl::DrawElementsType type)
{
    // We should never have to deal with primitive restart workaround issue with GL_UNSIGNED_INT
    // indices, since we restrict it via MAX_ELEMENT_INDEX.
    return (!primitiveRestartFixedIndexEnabled && type == gl::DrawElementsType::UnsignedShort);
}

IndexStorageType ClassifyIndexStorage(const gl::State &glState,
                                      const gl::Buffer *elementArrayBuffer,
                                      gl::DrawElementsType elementType,
                                      gl::DrawElementsType destElementType,
                                      unsigned int offset)
{
    // No buffer bound means we are streaming from a client pointer.
    if (!elementArrayBuffer || !IsOffsetAligned(elementType, offset))
    {
        return IndexStorageType::Dynamic;
    }

    // The buffer can be used directly if the storage supports it and no translation needed.
    BufferD3D *bufferD3D = GetImplAs<BufferD3D>(elementArrayBuffer);
    if (bufferD3D->supportsDirectBinding() && destElementType == elementType)
    {
        return IndexStorageType::Direct;
    }

    // Use a static copy when available.
    StaticIndexBufferInterface *staticBuffer = bufferD3D->getStaticIndexBuffer();
    if (staticBuffer != nullptr)
    {
        return IndexStorageType::Static;
    }

    // Static buffer not available, fall back to streaming.
    return IndexStorageType::Dynamic;
}

bool SwizzleRequired(const gl::TextureState &textureState)
{
    // When sampling stencil, a swizzle is needed to move the stencil channel from G to R.
    return textureState.swizzleRequired() || textureState.isStencilMode();
}

gl::SwizzleState GetEffectiveSwizzle(const gl::TextureState &textureState)
{
    const gl::SwizzleState &swizzle = textureState.getSwizzleState();
    if (textureState.isStencilMode())
    {
        // Per GL semantics, the stencil value should be in the red channel, while D3D11 formats
        // leave stencil in the green channel. So copy the stencil value from green to all
        // components requesting red. Green and blue become zero; alpha becomes one.
        std::unordered_map<GLenum, GLenum> map = {{GL_RED, GL_GREEN}, {GL_GREEN, GL_ZERO},
                                                  {GL_BLUE, GL_ZERO}, {GL_ALPHA, GL_ONE},
                                                  {GL_ZERO, GL_ZERO}, {GL_ONE, GL_ONE}};

        return gl::SwizzleState(map[swizzle.swizzleRed], map[swizzle.swizzleGreen],
                                map[swizzle.swizzleBlue], map[swizzle.swizzleAlpha]);
    }
    return swizzle;
}

}  // namespace rx
