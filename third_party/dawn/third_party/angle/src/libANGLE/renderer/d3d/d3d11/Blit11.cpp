//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Blit11.cpp: Texture copy utility class.

#include "libANGLE/renderer/d3d/d3d11/Blit11.h"

#include <float.h>

#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/d3d11/Context11.h"
#include "libANGLE/renderer/d3d/d3d11/RenderTarget11.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"
#include "libANGLE/renderer/d3d/d3d11/texture_format_table.h"
#include "libANGLE/trace.h"

namespace rx
{

namespace
{

// Include inline shaders in the anonymous namespace to make sure no symbols are exported
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthrough2d11vs.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughdepth2d11ps.h"

#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthrough3d11gs.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthrough3d11vs.h"

#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/resolvedepth11_ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/resolvedepthstencil11_ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/resolvedepthstencil11_vs.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/resolvestencil11_ps.h"

#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/swizzlef2darrayps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/swizzlef2dps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/swizzlef3dps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/swizzlei2darrayps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/swizzlei2dps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/swizzlei3dps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/swizzleui2darrayps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/swizzleui2dps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/swizzleui3dps.h"

void StretchedBlitNearest_RowByRow(const gl::Box &sourceArea,
                                   const gl::Box &destArea,
                                   const gl::Rectangle &clippedDestArea,
                                   const gl::Extents &sourceSize,
                                   unsigned int sourceRowPitch,
                                   unsigned int destRowPitch,
                                   size_t pixelSize,
                                   const uint8_t *sourceData,
                                   uint8_t *destData)
{
    int srcHeightSubOne = (sourceArea.height - 1);
    size_t copySize     = pixelSize * clippedDestArea.width;
    size_t srcOffset    = sourceArea.x * pixelSize;
    size_t destOffset   = clippedDestArea.x * pixelSize;

    for (int y = clippedDestArea.y; y < clippedDestArea.y + clippedDestArea.height; y++)
    {
        // TODO: Fix divide by zero when height == 1. http://anglebug.com/42264628
        float yPerc = static_cast<float>(y - destArea.y) / (destArea.height - 1);

        // Interpolate using the original source rectangle to determine which row to sample from
        // while clamping to the edges
        unsigned int readRow = static_cast<unsigned int>(
            gl::clamp(sourceArea.y + floor(yPerc * srcHeightSubOne + 0.5f), 0, srcHeightSubOne));
        unsigned int writeRow = y;

        const uint8_t *sourceRow = sourceData + readRow * sourceRowPitch + srcOffset;
        uint8_t *destRow         = destData + writeRow * destRowPitch + destOffset;
        memcpy(destRow, sourceRow, copySize);
    }
}

void StretchedBlitNearest_PixelByPixel(const gl::Box &sourceArea,
                                       const gl::Box &destArea,
                                       const gl::Rectangle &clippedDestArea,
                                       const gl::Extents &sourceSize,
                                       unsigned int sourceRowPitch,
                                       unsigned int destRowPitch,
                                       ptrdiff_t readOffset,
                                       ptrdiff_t writeOffset,
                                       size_t copySize,
                                       size_t srcPixelStride,
                                       size_t destPixelStride,
                                       const uint8_t *sourceData,
                                       uint8_t *destData)
{
    auto xMax = clippedDestArea.x + clippedDestArea.width;
    auto yMax = clippedDestArea.y + clippedDestArea.height;

    for (int writeRow = clippedDestArea.y; writeRow < yMax; writeRow++)
    {
        // Interpolate using the original source rectangle to determine which row to sample from
        // while clamping to the edges
        float yPerc    = static_cast<float>(writeRow - destArea.y) / (destArea.height - 1);
        float yRounded = floor(yPerc * (sourceArea.height - 1) + 0.5f);
        unsigned int readRow =
            static_cast<unsigned int>(gl::clamp(sourceArea.y + yRounded, 0, sourceSize.height - 1));

        for (int writeColumn = clippedDestArea.x; writeColumn < xMax; writeColumn++)
        {
            // Interpolate the original source rectangle to determine which column to sample
            // from while clamping to the edges
            float xPerc    = static_cast<float>(writeColumn - destArea.x) / (destArea.width - 1);
            float xRounded = floor(xPerc * (sourceArea.width - 1) + 0.5f);
            unsigned int readColumn = static_cast<unsigned int>(
                gl::clamp(sourceArea.x + xRounded, 0, sourceSize.width - 1));

            const uint8_t *sourcePixel =
                sourceData + readRow * sourceRowPitch + readColumn * srcPixelStride + readOffset;

            uint8_t *destPixel =
                destData + writeRow * destRowPitch + writeColumn * destPixelStride + writeOffset;

            memcpy(destPixel, sourcePixel, copySize);
        }
    }
}

void StretchedBlitNearest(const gl::Box &sourceArea,
                          const gl::Box &destArea,
                          const gl::Rectangle &clipRect,
                          const gl::Extents &sourceSize,
                          unsigned int sourceRowPitch,
                          unsigned int destRowPitch,
                          ptrdiff_t readOffset,
                          ptrdiff_t writeOffset,
                          size_t copySize,
                          size_t srcPixelStride,
                          size_t destPixelStride,
                          const uint8_t *sourceData,
                          uint8_t *destData)
{
    gl::Rectangle clippedDestArea(destArea.x, destArea.y, destArea.width, destArea.height);
    if (!gl::ClipRectangle(clippedDestArea, clipRect, &clippedDestArea))
    {
        return;
    }

    // Determine if entire rows can be copied at once instead of each individual pixel. There
    // must be no out of bounds lookups, whole rows copies, and no scale.
    if (sourceArea.width == clippedDestArea.width && sourceArea.x >= 0 &&
        sourceArea.x + sourceArea.width <= sourceSize.width && copySize == srcPixelStride &&
        copySize == destPixelStride)
    {
        StretchedBlitNearest_RowByRow(sourceArea, destArea, clippedDestArea, sourceSize,
                                      sourceRowPitch, destRowPitch, srcPixelStride, sourceData,
                                      destData);
    }
    else
    {
        StretchedBlitNearest_PixelByPixel(sourceArea, destArea, clippedDestArea, sourceSize,
                                          sourceRowPitch, destRowPitch, readOffset, writeOffset,
                                          copySize, srcPixelStride, destPixelStride, sourceData,
                                          destData);
    }
}

using DepthStencilLoader = void(const float *, uint8_t *);

void LoadDepth16(const float *source, uint8_t *dest)
{
    uint32_t convertedDepth = gl::floatToNormalized<16, uint32_t>(source[0]);
    memcpy(dest, &convertedDepth, 2u);
}

void LoadDepth24(const float *source, uint8_t *dest)
{
    uint32_t convertedDepth = gl::floatToNormalized<24, uint32_t>(source[0]);
    memcpy(dest, &convertedDepth, 3u);
}

void LoadStencilHelper(const float *source, uint8_t *dest)
{
    uint32_t convertedStencil = gl::getShiftedData<8, 0>(static_cast<uint32_t>(source[1]));
    memcpy(dest, &convertedStencil, 1u);
}

void LoadStencil8(const float *source, uint8_t *dest)
{
    // STENCIL_INDEX8 is implemented with D24S8, with the depth bits unused. Writes zero for safety.
    float zero = 0.0f;
    LoadDepth24(&zero, &dest[0]);
    LoadStencilHelper(source, &dest[3]);
}

void LoadDepth24Stencil8(const float *source, uint8_t *dest)
{
    LoadDepth24(source, &dest[0]);
    LoadStencilHelper(source, &dest[3]);
}

void LoadDepth32F(const float *source, uint8_t *dest)
{
    memcpy(dest, source, sizeof(float));
}

void LoadDepth32FStencil8(const float *source, uint8_t *dest)
{
    LoadDepth32F(source, &dest[0]);
    LoadStencilHelper(source, &dest[4]);
}

template <DepthStencilLoader loader>
void CopyDepthStencil(const gl::Box &sourceArea,
                      const gl::Box &destArea,
                      const gl::Rectangle &clippedDestArea,
                      const gl::Extents &sourceSize,
                      unsigned int sourceRowPitch,
                      unsigned int destRowPitch,
                      ptrdiff_t readOffset,
                      ptrdiff_t writeOffset,
                      size_t copySize,
                      size_t srcPixelStride,
                      size_t destPixelStride,
                      const uint8_t *sourceData,
                      uint8_t *destData)
{
    // No stretching or subregions are supported, only full blits.
    ASSERT(sourceArea == destArea);
    ASSERT(sourceSize.width == sourceArea.width && sourceSize.height == sourceArea.height &&
           sourceSize.depth == 1);
    ASSERT(clippedDestArea.width == sourceSize.width &&
           clippedDestArea.height == sourceSize.height);
    ASSERT(readOffset == 0 && writeOffset == 0);
    ASSERT(destArea.x == 0 && destArea.y == 0);

    for (int row = 0; row < destArea.height; ++row)
    {
        for (int column = 0; column < destArea.width; ++column)
        {
            ptrdiff_t offset         = row * sourceRowPitch + column * srcPixelStride;
            const float *sourcePixel = reinterpret_cast<const float *>(sourceData + offset);

            uint8_t *destPixel = destData + row * destRowPitch + column * destPixelStride;

            loader(sourcePixel, destPixel);
        }
    }
}

void Depth32FStencil8ToDepth32F(const float *source, float *dest)
{
    *dest = *source;
}

void Depth24Stencil8ToDepth32F(const uint32_t *source, float *dest)
{
    uint32_t normDepth = source[0] & 0x00FFFFFF;
    float floatDepth   = gl::normalizedToFloat<24>(normDepth);
    *dest              = floatDepth;
}

void BlitD24S8ToD32F(const gl::Box &sourceArea,
                     const gl::Box &destArea,
                     const gl::Rectangle &clippedDestArea,
                     const gl::Extents &sourceSize,
                     unsigned int sourceRowPitch,
                     unsigned int destRowPitch,
                     ptrdiff_t readOffset,
                     ptrdiff_t writeOffset,
                     size_t copySize,
                     size_t srcPixelStride,
                     size_t destPixelStride,
                     const uint8_t *sourceData,
                     uint8_t *destData)
{
    // No stretching or subregions are supported, only full blits.
    ASSERT(sourceArea == destArea);
    ASSERT(sourceSize.width == sourceArea.width && sourceSize.height == sourceArea.height &&
           sourceSize.depth == 1);
    ASSERT(clippedDestArea.width == sourceSize.width &&
           clippedDestArea.height == sourceSize.height);
    ASSERT(readOffset == 0 && writeOffset == 0);
    ASSERT(destArea.x == 0 && destArea.y == 0);

    for (int row = 0; row < destArea.height; ++row)
    {
        for (int column = 0; column < destArea.width; ++column)
        {
            ptrdiff_t offset            = row * sourceRowPitch + column * srcPixelStride;
            const uint32_t *sourcePixel = reinterpret_cast<const uint32_t *>(sourceData + offset);

            float *destPixel =
                reinterpret_cast<float *>(destData + row * destRowPitch + column * destPixelStride);

            Depth24Stencil8ToDepth32F(sourcePixel, destPixel);
        }
    }
}

void BlitD32FS8ToD32F(const gl::Box &sourceArea,
                      const gl::Box &destArea,
                      const gl::Rectangle &clippedDestArea,
                      const gl::Extents &sourceSize,
                      unsigned int sourceRowPitch,
                      unsigned int destRowPitch,
                      ptrdiff_t readOffset,
                      ptrdiff_t writeOffset,
                      size_t copySize,
                      size_t srcPixelStride,
                      size_t destPixelStride,
                      const uint8_t *sourceData,
                      uint8_t *destData)
{
    // No stretching or subregions are supported, only full blits.
    ASSERT(sourceArea == destArea);
    ASSERT(sourceSize.width == sourceArea.width && sourceSize.height == sourceArea.height &&
           sourceSize.depth == 1);
    ASSERT(clippedDestArea.width == sourceSize.width &&
           clippedDestArea.height == sourceSize.height);
    ASSERT(readOffset == 0 && writeOffset == 0);
    ASSERT(destArea.x == 0 && destArea.y == 0);

    for (int row = 0; row < destArea.height; ++row)
    {
        for (int column = 0; column < destArea.width; ++column)
        {
            ptrdiff_t offset         = row * sourceRowPitch + column * srcPixelStride;
            const float *sourcePixel = reinterpret_cast<const float *>(sourceData + offset);
            float *destPixel =
                reinterpret_cast<float *>(destData + row * destRowPitch + column * destPixelStride);

            Depth32FStencil8ToDepth32F(sourcePixel, destPixel);
        }
    }
}

Blit11::BlitConvertFunction *GetCopyDepthStencilFunction(GLenum internalFormat)
{
    switch (internalFormat)
    {
        case GL_DEPTH_COMPONENT16:
            return &CopyDepthStencil<LoadDepth16>;
        case GL_DEPTH_COMPONENT24:
            return &CopyDepthStencil<LoadDepth24>;
        case GL_DEPTH_COMPONENT32F:
            return &CopyDepthStencil<LoadDepth32F>;
        case GL_STENCIL_INDEX8:
            return &CopyDepthStencil<LoadStencil8>;
        case GL_DEPTH24_STENCIL8:
            return &CopyDepthStencil<LoadDepth24Stencil8>;
        case GL_DEPTH32F_STENCIL8:
            return &CopyDepthStencil<LoadDepth32FStencil8>;
        default:
            UNREACHABLE();
            return nullptr;
    }
}

inline void GenerateVertexCoords(const gl::Box &sourceArea,
                                 const gl::Extents &sourceSize,
                                 const gl::Box &destArea,
                                 const gl::Extents &destSize,
                                 float *x1,
                                 float *y1,
                                 float *x2,
                                 float *y2,
                                 float *u1,
                                 float *v1,
                                 float *u2,
                                 float *v2)
{
    *x1 = (destArea.x / float(destSize.width)) * 2.0f - 1.0f;
    *y1 = ((destSize.height - destArea.y - destArea.height) / float(destSize.height)) * 2.0f - 1.0f;
    *x2 = ((destArea.x + destArea.width) / float(destSize.width)) * 2.0f - 1.0f;
    *y2 = ((destSize.height - destArea.y) / float(destSize.height)) * 2.0f - 1.0f;

    *u1 = sourceArea.x / float(sourceSize.width);
    *v1 = sourceArea.y / float(sourceSize.height);
    *u2 = (sourceArea.x + sourceArea.width) / float(sourceSize.width);
    *v2 = (sourceArea.y + sourceArea.height) / float(sourceSize.height);
}

void Write2DVertices(const gl::Box &sourceArea,
                     const gl::Extents &sourceSize,
                     const gl::Box &destArea,
                     const gl::Extents &destSize,
                     void *outVertices,
                     unsigned int *outStride,
                     unsigned int *outVertexCount,
                     D3D11_PRIMITIVE_TOPOLOGY *outTopology)
{
    float x1, y1, x2, y2, u1, v1, u2, v2;
    GenerateVertexCoords(sourceArea, sourceSize, destArea, destSize, &x1, &y1, &x2, &y2, &u1, &v1,
                         &u2, &v2);

    d3d11::PositionTexCoordVertex *vertices =
        static_cast<d3d11::PositionTexCoordVertex *>(outVertices);

    d3d11::SetPositionTexCoordVertex(&vertices[0], x1, y1, u1, v2);
    d3d11::SetPositionTexCoordVertex(&vertices[1], x1, y2, u1, v1);
    d3d11::SetPositionTexCoordVertex(&vertices[2], x2, y1, u2, v2);
    d3d11::SetPositionTexCoordVertex(&vertices[3], x2, y2, u2, v1);

    *outStride      = sizeof(d3d11::PositionTexCoordVertex);
    *outVertexCount = 4;
    *outTopology    = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
}

void Write3DVertices(const gl::Box &sourceArea,
                     const gl::Extents &sourceSize,
                     const gl::Box &destArea,
                     const gl::Extents &destSize,
                     void *outVertices,
                     unsigned int *outStride,
                     unsigned int *outVertexCount,
                     D3D11_PRIMITIVE_TOPOLOGY *outTopology)
{
    ASSERT(sourceSize.depth > 0 && destSize.depth > 0);

    float x1, y1, x2, y2, u1, v1, u2, v2;
    GenerateVertexCoords(sourceArea, sourceSize, destArea, destSize, &x1, &y1, &x2, &y2, &u1, &v1,
                         &u2, &v2);

    d3d11::PositionLayerTexCoord3DVertex *vertices =
        static_cast<d3d11::PositionLayerTexCoord3DVertex *>(outVertices);

    for (int i = 0; i < destSize.depth; i++)
    {
        float readDepth = (float)i / std::max(destSize.depth - 1, 1);

        d3d11::SetPositionLayerTexCoord3DVertex(&vertices[i * 6 + 0], x1, y1, i, u1, v2, readDepth);
        d3d11::SetPositionLayerTexCoord3DVertex(&vertices[i * 6 + 1], x1, y2, i, u1, v1, readDepth);
        d3d11::SetPositionLayerTexCoord3DVertex(&vertices[i * 6 + 2], x2, y1, i, u2, v2, readDepth);

        d3d11::SetPositionLayerTexCoord3DVertex(&vertices[i * 6 + 3], x1, y2, i, u1, v1, readDepth);
        d3d11::SetPositionLayerTexCoord3DVertex(&vertices[i * 6 + 4], x2, y2, i, u2, v1, readDepth);
        d3d11::SetPositionLayerTexCoord3DVertex(&vertices[i * 6 + 5], x2, y1, i, u2, v2, readDepth);
    }

    *outStride      = sizeof(d3d11::PositionLayerTexCoord3DVertex);
    *outVertexCount = destSize.depth * 6;
    *outTopology    = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

unsigned int GetSwizzleIndex(GLenum swizzle)
{
    unsigned int colorIndex = 0;

    switch (swizzle)
    {
        case GL_RED:
            colorIndex = 0;
            break;
        case GL_GREEN:
            colorIndex = 1;
            break;
        case GL_BLUE:
            colorIndex = 2;
            break;
        case GL_ALPHA:
            colorIndex = 3;
            break;
        case GL_ZERO:
            colorIndex = 4;
            break;
        case GL_ONE:
            colorIndex = 5;
            break;
        default:
            UNREACHABLE();
            break;
    }

    return colorIndex;
}

D3D11_BLEND_DESC GetAlphaMaskBlendStateDesc()
{
    D3D11_BLEND_DESC desc;
    memset(&desc, 0, sizeof(desc));
    desc.RenderTarget[0].BlendEnable           = TRUE;
    desc.RenderTarget[0].SrcBlend              = D3D11_BLEND_ONE;
    desc.RenderTarget[0].DestBlend             = D3D11_BLEND_ZERO;
    desc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ZERO;
    desc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
    desc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED |
                                                 D3D11_COLOR_WRITE_ENABLE_GREEN |
                                                 D3D11_COLOR_WRITE_ENABLE_BLUE;
    return desc;
}

D3D11_INPUT_ELEMENT_DESC quad2DLayout[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0},
};

D3D11_INPUT_ELEMENT_DESC quad3DLayout[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"LAYER", 0, DXGI_FORMAT_R32_UINT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
};

DXGI_FORMAT GetStencilSRVFormat(const d3d11::Format &formatSet)
{
    switch (formatSet.texFormat)
    {
        case DXGI_FORMAT_R32G8X24_TYPELESS:
            return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
        case DXGI_FORMAT_R24G8_TYPELESS:
            return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
        default:
            UNREACHABLE();
            return DXGI_FORMAT_UNKNOWN;
    }
}

}  // namespace

#include "libANGLE/renderer/d3d/d3d11/Blit11Helper_autogen.inc"

Blit11::Shader::Shader() = default;

Blit11::Shader::Shader(Shader &&other) = default;

Blit11::Shader::~Shader() = default;

Blit11::Shader &Blit11::Shader::operator=(Blit11::Shader &&other) = default;

Blit11::Blit11(Renderer11 *renderer)
    : mRenderer(renderer),
      mResourcesInitialized(false),
      mVertexBuffer(),
      mPointSampler(),
      mLinearSampler(),
      mScissorEnabledRasterizerState(),
      mScissorDisabledRasterizerState(),
      mDepthStencilState(),
      mQuad2DIL(quad2DLayout,
                ArraySize(quad2DLayout),
                g_VS_Passthrough2D,
                ArraySize(g_VS_Passthrough2D),
                "Blit11 2D input layout"),
      mQuad2DVS(g_VS_Passthrough2D, ArraySize(g_VS_Passthrough2D), "Blit11 2D vertex shader"),
      mDepthPS(g_PS_PassthroughDepth2D,
               ArraySize(g_PS_PassthroughDepth2D),
               "Blit11 2D depth pixel shader"),
      mQuad3DIL(quad3DLayout,
                ArraySize(quad3DLayout),
                g_VS_Passthrough3D,
                ArraySize(g_VS_Passthrough3D),
                "Blit11 3D input layout"),
      mQuad3DVS(g_VS_Passthrough3D, ArraySize(g_VS_Passthrough3D), "Blit11 3D vertex shader"),
      mQuad3DGS(g_GS_Passthrough3D, ArraySize(g_GS_Passthrough3D), "Blit11 3D geometry shader"),
      mAlphaMaskBlendState(GetAlphaMaskBlendStateDesc(), "Blit11 Alpha Mask Blend"),
      mSwizzleCB(),
      mResolveDepthStencilVS(g_VS_ResolveDepthStencil,
                             ArraySize(g_VS_ResolveDepthStencil),
                             "Blit11::mResolveDepthStencilVS"),
      mResolveDepthPS(g_PS_ResolveDepth, ArraySize(g_PS_ResolveDepth), "Blit11::mResolveDepthPS"),
      mResolveDepthStencilPS(g_PS_ResolveDepthStencil,
                             ArraySize(g_PS_ResolveDepthStencil),
                             "Blit11::mResolveDepthStencilPS"),
      mResolveStencilPS(g_PS_ResolveStencil,
                        ArraySize(g_PS_ResolveStencil),
                        "Blit11::mResolveStencilPS"),
      mStencilSRV(),
      mResolvedDepthStencilRTView()
{}

Blit11::~Blit11() {}

angle::Result Blit11::initResources(const gl::Context *context)
{
    if (mResourcesInitialized)
    {
        return angle::Result::Continue;
    }

    ANGLE_TRACE_EVENT0("gpu.angle", "Blit11::initResources");

    D3D11_BUFFER_DESC vbDesc;
    vbDesc.ByteWidth =
        static_cast<unsigned int>(std::max(sizeof(d3d11::PositionLayerTexCoord3DVertex),
                                           sizeof(d3d11::PositionTexCoordVertex)) *
                                  6 * mRenderer->getNativeCaps().max3DTextureSize);
    vbDesc.Usage               = D3D11_USAGE_DYNAMIC;
    vbDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
    vbDesc.MiscFlags           = 0;
    vbDesc.StructureByteStride = 0;

    Context11 *context11 = GetImplAs<Context11>(context);

    ANGLE_TRY(mRenderer->allocateResource(context11, vbDesc, &mVertexBuffer));
    mVertexBuffer.setInternalName("Blit11VertexBuffer");

    D3D11_SAMPLER_DESC pointSamplerDesc;
    pointSamplerDesc.Filter         = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
    pointSamplerDesc.AddressU       = D3D11_TEXTURE_ADDRESS_CLAMP;
    pointSamplerDesc.AddressV       = D3D11_TEXTURE_ADDRESS_CLAMP;
    pointSamplerDesc.AddressW       = D3D11_TEXTURE_ADDRESS_CLAMP;
    pointSamplerDesc.MipLODBias     = 0.0f;
    pointSamplerDesc.MaxAnisotropy  = 0;
    pointSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    pointSamplerDesc.BorderColor[0] = 0.0f;
    pointSamplerDesc.BorderColor[1] = 0.0f;
    pointSamplerDesc.BorderColor[2] = 0.0f;
    pointSamplerDesc.BorderColor[3] = 0.0f;
    pointSamplerDesc.MinLOD         = 0.0f;
    pointSamplerDesc.MaxLOD         = FLT_MAX;

    ANGLE_TRY(mRenderer->allocateResource(context11, pointSamplerDesc, &mPointSampler));
    mPointSampler.setInternalName("Blit11PointSampler");

    D3D11_SAMPLER_DESC linearSamplerDesc;
    linearSamplerDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    linearSamplerDesc.AddressU       = D3D11_TEXTURE_ADDRESS_CLAMP;
    linearSamplerDesc.AddressV       = D3D11_TEXTURE_ADDRESS_CLAMP;
    linearSamplerDesc.AddressW       = D3D11_TEXTURE_ADDRESS_CLAMP;
    linearSamplerDesc.MipLODBias     = 0.0f;
    linearSamplerDesc.MaxAnisotropy  = 0;
    linearSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    linearSamplerDesc.BorderColor[0] = 0.0f;
    linearSamplerDesc.BorderColor[1] = 0.0f;
    linearSamplerDesc.BorderColor[2] = 0.0f;
    linearSamplerDesc.BorderColor[3] = 0.0f;
    linearSamplerDesc.MinLOD         = 0.0f;
    linearSamplerDesc.MaxLOD         = FLT_MAX;

    ANGLE_TRY(mRenderer->allocateResource(context11, linearSamplerDesc, &mLinearSampler));
    mLinearSampler.setInternalName("Blit11LinearSampler");

    // Use a rasterizer state that will not cull so that inverted quads will not be culled
    D3D11_RASTERIZER_DESC rasterDesc;
    rasterDesc.FillMode              = D3D11_FILL_SOLID;
    rasterDesc.CullMode              = D3D11_CULL_NONE;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthBias             = 0;
    rasterDesc.SlopeScaledDepthBias  = 0.0f;
    rasterDesc.DepthBiasClamp        = 0.0f;
    rasterDesc.DepthClipEnable       = TRUE;
    rasterDesc.MultisampleEnable     = FALSE;
    rasterDesc.AntialiasedLineEnable = FALSE;

    rasterDesc.ScissorEnable = TRUE;
    ANGLE_TRY(mRenderer->allocateResource(context11, rasterDesc, &mScissorEnabledRasterizerState));
    mScissorEnabledRasterizerState.setInternalName("Blit11ScissoringRasterizerState");

    rasterDesc.ScissorEnable = FALSE;
    ANGLE_TRY(mRenderer->allocateResource(context11, rasterDesc, &mScissorDisabledRasterizerState));
    mScissorDisabledRasterizerState.setInternalName("Blit11NoScissoringRasterizerState");

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    depthStencilDesc.DepthEnable                  = TRUE;
    depthStencilDesc.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc                    = D3D11_COMPARISON_ALWAYS;
    depthStencilDesc.StencilEnable                = FALSE;
    depthStencilDesc.StencilReadMask              = D3D11_DEFAULT_STENCIL_READ_MASK;
    depthStencilDesc.StencilWriteMask             = D3D11_DEFAULT_STENCIL_WRITE_MASK;
    depthStencilDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
    depthStencilDesc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc         = D3D11_COMPARISON_ALWAYS;

    ANGLE_TRY(mRenderer->allocateResource(context11, depthStencilDesc, &mDepthStencilState));
    mDepthStencilState.setInternalName("Blit11DepthStencilState");

    D3D11_BUFFER_DESC swizzleBufferDesc;
    swizzleBufferDesc.ByteWidth           = sizeof(unsigned int) * 4;
    swizzleBufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
    swizzleBufferDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
    swizzleBufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
    swizzleBufferDesc.MiscFlags           = 0;
    swizzleBufferDesc.StructureByteStride = 0;

    ANGLE_TRY(mRenderer->allocateResource(context11, swizzleBufferDesc, &mSwizzleCB));
    mSwizzleCB.setInternalName("Blit11SwizzleConstantBuffer");

    mResourcesInitialized = true;

    return angle::Result::Continue;
}

// static
Blit11::SwizzleShaderType Blit11::GetSwizzleShaderType(GLenum type,
                                                       D3D11_SRV_DIMENSION dimensionality)
{
    switch (dimensionality)
    {
        case D3D11_SRV_DIMENSION_TEXTURE2D:
            switch (type)
            {
                case GL_FLOAT:
                    return SWIZZLESHADER_2D_FLOAT;
                case GL_UNSIGNED_INT:
                    return SWIZZLESHADER_2D_UINT;
                case GL_INT:
                    return SWIZZLESHADER_2D_INT;
                default:
                    UNREACHABLE();
                    return SWIZZLESHADER_INVALID;
            }
        case D3D11_SRV_DIMENSION_TEXTURECUBE:
            switch (type)
            {
                case GL_FLOAT:
                    return SWIZZLESHADER_CUBE_FLOAT;
                case GL_UNSIGNED_INT:
                    return SWIZZLESHADER_CUBE_UINT;
                case GL_INT:
                    return SWIZZLESHADER_CUBE_INT;
                default:
                    UNREACHABLE();
                    return SWIZZLESHADER_INVALID;
            }
        case D3D11_SRV_DIMENSION_TEXTURE3D:
            switch (type)
            {
                case GL_FLOAT:
                    return SWIZZLESHADER_3D_FLOAT;
                case GL_UNSIGNED_INT:
                    return SWIZZLESHADER_3D_UINT;
                case GL_INT:
                    return SWIZZLESHADER_3D_INT;
                default:
                    UNREACHABLE();
                    return SWIZZLESHADER_INVALID;
            }
        case D3D11_SRV_DIMENSION_TEXTURE2DARRAY:
            switch (type)
            {
                case GL_FLOAT:
                    return SWIZZLESHADER_ARRAY_FLOAT;
                case GL_UNSIGNED_INT:
                    return SWIZZLESHADER_ARRAY_UINT;
                case GL_INT:
                    return SWIZZLESHADER_ARRAY_INT;
                default:
                    UNREACHABLE();
                    return SWIZZLESHADER_INVALID;
            }
        default:
            UNREACHABLE();
            return SWIZZLESHADER_INVALID;
    }
}

angle::Result Blit11::getShaderSupport(const gl::Context *context,
                                       const Shader &shader,
                                       Blit11::ShaderSupport *supportOut)
{

    Context11 *context11 = GetImplAs<Context11>(context);

    switch (shader.dimension)
    {
        case SHADER_2D:
        {
            ANGLE_TRY(mQuad2DIL.resolve(context11, mRenderer));
            ANGLE_TRY(mQuad2DVS.resolve(context11, mRenderer));
            supportOut->inputLayout         = &mQuad2DIL.getObj();
            supportOut->vertexShader        = &mQuad2DVS.getObj();
            supportOut->geometryShader      = nullptr;
            supportOut->vertexWriteFunction = Write2DVertices;
            break;
        }
        case SHADER_3D:
        case SHADER_2DARRAY:
        {
            ANGLE_TRY(mQuad3DIL.resolve(context11, mRenderer));
            ANGLE_TRY(mQuad3DVS.resolve(context11, mRenderer));
            ANGLE_TRY(mQuad3DGS.resolve(context11, mRenderer));
            supportOut->inputLayout         = &mQuad3DIL.getObj();
            supportOut->vertexShader        = &mQuad3DVS.getObj();
            supportOut->geometryShader      = &mQuad3DGS.getObj();
            supportOut->vertexWriteFunction = Write3DVertices;
            break;
        }
        default:
            UNREACHABLE();
    }

    return angle::Result::Continue;
}

angle::Result Blit11::swizzleTexture(const gl::Context *context,
                                     const d3d11::SharedSRV &source,
                                     const d3d11::RenderTargetView &dest,
                                     const gl::Extents &size,
                                     const gl::SwizzleState &swizzleTarget)
{
    ANGLE_TRY(initResources(context));

    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

    D3D11_SHADER_RESOURCE_VIEW_DESC sourceSRVDesc;
    source.get()->GetDesc(&sourceSRVDesc);

    GLenum componentType = d3d11::GetComponentType(sourceSRVDesc.Format);
    if (componentType == GL_NONE)
    {
        // We're swizzling the depth component of a depth-stencil texture.
        switch (sourceSRVDesc.Format)
        {
            case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
                componentType = GL_UNSIGNED_NORMALIZED;
                break;
            case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
                componentType = GL_FLOAT;
                break;
            default:
                UNREACHABLE();
                break;
        }
    }

    GLenum shaderType = GL_NONE;
    switch (componentType)
    {
        case GL_UNSIGNED_NORMALIZED:
        case GL_SIGNED_NORMALIZED:
        case GL_FLOAT:
            shaderType = GL_FLOAT;
            break;
        case GL_INT:
            shaderType = GL_INT;
            break;
        case GL_UNSIGNED_INT:
            shaderType = GL_UNSIGNED_INT;
            break;
        default:
            UNREACHABLE();
            break;
    }

    const Shader *shader = nullptr;
    ANGLE_TRY(getSwizzleShader(context, shaderType, sourceSRVDesc.ViewDimension, &shader));

    // Set vertices
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ANGLE_TRY(mRenderer->mapResource(context, mVertexBuffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0,
                                     &mappedResource));

    ShaderSupport support;
    ANGLE_TRY(getShaderSupport(context, *shader, &support));

    UINT stride    = 0;
    UINT drawCount = 0;
    D3D11_PRIMITIVE_TOPOLOGY topology;

    gl::Box area(0, 0, 0, size.width, size.height, size.depth);
    support.vertexWriteFunction(area, size, area, size, mappedResource.pData, &stride, &drawCount,
                                &topology);

    deviceContext->Unmap(mVertexBuffer.get(), 0);

    // Set constant buffer
    ANGLE_TRY(mRenderer->mapResource(context, mSwizzleCB.get(), 0, D3D11_MAP_WRITE_DISCARD, 0,
                                     &mappedResource));

    unsigned int *swizzleIndices = static_cast<unsigned int *>(mappedResource.pData);
    swizzleIndices[0]            = GetSwizzleIndex(swizzleTarget.swizzleRed);
    swizzleIndices[1]            = GetSwizzleIndex(swizzleTarget.swizzleGreen);
    swizzleIndices[2]            = GetSwizzleIndex(swizzleTarget.swizzleBlue);
    swizzleIndices[3]            = GetSwizzleIndex(swizzleTarget.swizzleAlpha);

    deviceContext->Unmap(mSwizzleCB.get(), 0);

    StateManager11 *stateManager = mRenderer->getStateManager();

    // Apply vertex buffer
    stateManager->setSingleVertexBuffer(&mVertexBuffer, stride, 0);

    // Apply constant buffer
    stateManager->setPixelConstantBuffer(0, &mSwizzleCB);

    // Apply state
    stateManager->setSimpleBlendState(nullptr);
    stateManager->setDepthStencilState(nullptr, 0xFFFFFFFF);
    stateManager->setRasterizerState(&mScissorDisabledRasterizerState);

    // Apply shaders
    stateManager->setInputLayout(support.inputLayout);
    stateManager->setPrimitiveTopology(topology);

    stateManager->setDrawShaders(support.vertexShader, support.geometryShader,
                                 &shader->pixelShader);

    // Apply render target
    stateManager->setRenderTarget(dest.get(), nullptr);

    // Set the viewport
    stateManager->setSimpleViewport(size);

    // Apply textures and sampler
    stateManager->setSimplePixelTextureAndSampler(source, mPointSampler);

    // Draw the quad
    deviceContext->Draw(drawCount, 0);

    return angle::Result::Continue;
}

angle::Result Blit11::copyTexture(const gl::Context *context,
                                  const d3d11::SharedSRV &source,
                                  const gl::Box &sourceArea,
                                  const gl::Extents &sourceSize,
                                  GLenum sourceFormat,
                                  const d3d11::RenderTargetView &dest,
                                  const gl::Box &destArea,
                                  const gl::Extents &destSize,
                                  const gl::Rectangle *scissor,
                                  GLenum destFormat,
                                  GLenum destTypeForDownsampling,
                                  GLenum filter,
                                  bool maskOffAlpha,
                                  bool unpackPremultiplyAlpha,
                                  bool unpackUnmultiplyAlpha)
{
    ANGLE_TRY(initResources(context));

    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

    // Determine if the source format is a signed integer format, the destFormat will already
    // be GL_XXXX_INTEGER but it does not tell us if it is signed or unsigned.
    D3D11_SHADER_RESOURCE_VIEW_DESC sourceSRVDesc;
    source.get()->GetDesc(&sourceSRVDesc);

    GLenum componentType = d3d11::GetComponentType(sourceSRVDesc.Format);

    ASSERT(componentType != GL_NONE);
    bool isSrcSigned = (componentType == GL_INT);

    D3D11_RENDER_TARGET_VIEW_DESC destRTVDesc;
    dest.get()->GetDesc(&destRTVDesc);

    GLenum destComponentType = d3d11::GetComponentType(destRTVDesc.Format);

    ASSERT(componentType != GL_NONE);
    bool isDestSigned = (destComponentType == GL_INT);

    ShaderDimension dimension = SHADER_INVALID;

    switch (sourceSRVDesc.ViewDimension)
    {
        case D3D11_SRV_DIMENSION_TEXTURE2D:
            dimension = SHADER_2D;
            break;
        case D3D11_SRV_DIMENSION_TEXTURE3D:
            dimension = SHADER_3D;
            break;
        case D3D11_SRV_DIMENSION_TEXTURE2DARRAY:
            dimension = SHADER_2DARRAY;
            break;
        default:
            UNREACHABLE();
    }

    const Shader *shader = nullptr;

    ANGLE_TRY(getBlitShader(context, destFormat, sourceFormat, isSrcSigned, isDestSigned,
                            unpackPremultiplyAlpha, unpackUnmultiplyAlpha, destTypeForDownsampling,
                            dimension, &shader));

    ShaderSupport support;
    ANGLE_TRY(getShaderSupport(context, *shader, &support));

    // Set vertices
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ANGLE_TRY(mRenderer->mapResource(context, mVertexBuffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0,
                                     &mappedResource));

    UINT stride    = 0;
    UINT drawCount = 0;
    D3D11_PRIMITIVE_TOPOLOGY topology;

    support.vertexWriteFunction(sourceArea, sourceSize, destArea, destSize, mappedResource.pData,
                                &stride, &drawCount, &topology);

    deviceContext->Unmap(mVertexBuffer.get(), 0);

    StateManager11 *stateManager = mRenderer->getStateManager();

    // Apply vertex buffer
    stateManager->setSingleVertexBuffer(&mVertexBuffer, stride, 0);

    // Apply state
    if (maskOffAlpha)
    {
        ANGLE_TRY(mAlphaMaskBlendState.resolve(GetImplAs<Context11>(context), mRenderer));
        stateManager->setSimpleBlendState(&mAlphaMaskBlendState.getObj());
    }
    else
    {
        stateManager->setSimpleBlendState(nullptr);
    }
    stateManager->setDepthStencilState(nullptr, 0xFFFFFFFF);

    if (scissor)
    {
        stateManager->setSimpleScissorRect(*scissor);
        stateManager->setRasterizerState(&mScissorEnabledRasterizerState);
    }
    else
    {
        stateManager->setRasterizerState(&mScissorDisabledRasterizerState);
    }

    // Apply shaders
    stateManager->setInputLayout(support.inputLayout);
    stateManager->setPrimitiveTopology(topology);

    stateManager->setDrawShaders(support.vertexShader, support.geometryShader,
                                 &shader->pixelShader);

    // Apply render target
    stateManager->setRenderTarget(dest.get(), nullptr);

    // Set the viewport
    stateManager->setSimpleViewport(destSize);

    // Apply texture and sampler
    switch (filter)
    {
        case GL_NEAREST:
            stateManager->setSimplePixelTextureAndSampler(source, mPointSampler);
            break;
        case GL_LINEAR:
            stateManager->setSimplePixelTextureAndSampler(source, mLinearSampler);
            break;

        default:
            UNREACHABLE();
            ANGLE_TRY_HR(GetImplAs<Context11>(context), E_FAIL,
                         "Internal error, unknown blit filter mode.");
    }

    // Draw the quad
    deviceContext->Draw(drawCount, 0);

    return angle::Result::Continue;
}

angle::Result Blit11::copyStencil(const gl::Context *context,
                                  const TextureHelper11 &source,
                                  unsigned int sourceSubresource,
                                  const gl::Box &sourceArea,
                                  const gl::Extents &sourceSize,
                                  const TextureHelper11 &dest,
                                  unsigned int destSubresource,
                                  const gl::Box &destArea,
                                  const gl::Extents &destSize,
                                  const gl::Rectangle *scissor)
{
    return copyDepthStencilImpl(context, source, sourceSubresource, sourceArea, sourceSize, dest,
                                destSubresource, destArea, destSize, scissor, true);
}

angle::Result Blit11::copyDepth(const gl::Context *context,
                                const d3d11::SharedSRV &source,
                                const gl::Box &sourceArea,
                                const gl::Extents &sourceSize,
                                const d3d11::DepthStencilView &dest,
                                const gl::Box &destArea,
                                const gl::Extents &destSize,
                                const gl::Rectangle *scissor)
{
    ANGLE_TRY(initResources(context));

    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

    // Set vertices
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ANGLE_TRY(mRenderer->mapResource(context, mVertexBuffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0,
                                     &mappedResource));

    UINT stride    = 0;
    UINT drawCount = 0;
    D3D11_PRIMITIVE_TOPOLOGY topology;

    Write2DVertices(sourceArea, sourceSize, destArea, destSize, mappedResource.pData, &stride,
                    &drawCount, &topology);

    deviceContext->Unmap(mVertexBuffer.get(), 0);

    StateManager11 *stateManager = mRenderer->getStateManager();

    // Apply vertex buffer
    stateManager->setSingleVertexBuffer(&mVertexBuffer, stride, 0);

    // Apply state
    stateManager->setSimpleBlendState(nullptr);
    stateManager->setDepthStencilState(&mDepthStencilState, 0xFFFFFFFF);

    if (scissor)
    {
        stateManager->setSimpleScissorRect(*scissor);
        stateManager->setRasterizerState(&mScissorEnabledRasterizerState);
    }
    else
    {
        stateManager->setRasterizerState(&mScissorDisabledRasterizerState);
    }

    Context11 *context11 = GetImplAs<Context11>(context);

    ANGLE_TRY(mQuad2DIL.resolve(context11, mRenderer));
    ANGLE_TRY(mQuad2DVS.resolve(context11, mRenderer));
    ANGLE_TRY(mDepthPS.resolve(context11, mRenderer));

    // Apply shaders
    stateManager->setInputLayout(&mQuad2DIL.getObj());
    stateManager->setPrimitiveTopology(topology);

    stateManager->setDrawShaders(&mQuad2DVS.getObj(), nullptr, &mDepthPS.getObj());

    // Apply render target
    stateManager->setRenderTarget(nullptr, dest.get());

    // Set the viewport
    stateManager->setSimpleViewport(destSize);

    // Apply texture and sampler
    stateManager->setSimplePixelTextureAndSampler(source, mPointSampler);

    // Draw the quad
    deviceContext->Draw(drawCount, 0);

    return angle::Result::Continue;
}

angle::Result Blit11::copyDepthStencil(const gl::Context *context,
                                       const TextureHelper11 &source,
                                       unsigned int sourceSubresource,
                                       const gl::Box &sourceArea,
                                       const gl::Extents &sourceSize,
                                       const TextureHelper11 &dest,
                                       unsigned int destSubresource,
                                       const gl::Box &destArea,
                                       const gl::Extents &destSize,
                                       const gl::Rectangle *scissor)
{
    return copyDepthStencilImpl(context, source, sourceSubresource, sourceArea, sourceSize, dest,
                                destSubresource, destArea, destSize, scissor, false);
}

angle::Result Blit11::copyDepthStencilImpl(const gl::Context *context,
                                           const TextureHelper11 &source,
                                           unsigned int sourceSubresource,
                                           const gl::Box &sourceArea,
                                           const gl::Extents &sourceSize,
                                           const TextureHelper11 &dest,
                                           unsigned int destSubresource,
                                           const gl::Box &destArea,
                                           const gl::Extents &destSize,
                                           const gl::Rectangle *scissor,
                                           bool stencilOnly)
{
    auto srcDXGIFormat         = source.getFormat();
    const auto &srcSizeInfo    = d3d11::GetDXGIFormatSizeInfo(srcDXGIFormat);
    unsigned int srcPixelSize  = srcSizeInfo.pixelBytes;
    unsigned int copyOffset    = 0;
    unsigned int copySize      = srcPixelSize;
    auto destDXGIFormat        = dest.getFormat();
    const auto &destSizeInfo   = d3d11::GetDXGIFormatSizeInfo(destDXGIFormat);
    unsigned int destPixelSize = destSizeInfo.pixelBytes;

    ASSERT(srcDXGIFormat == destDXGIFormat || destDXGIFormat == DXGI_FORMAT_R32_TYPELESS);

    if (stencilOnly)
    {
        const auto &srcFormat = source.getFormatSet().format();

        // Stencil channel should be right after the depth channel. Some views to depth/stencil
        // resources have red channel for depth, in which case the depth channel bit width is in
        // redBits.
        ASSERT((srcFormat.redBits != 0) != (srcFormat.depthBits != 0));
        GLuint depthBits = srcFormat.redBits + srcFormat.depthBits;
        // Known formats have either 24 or 32 bits of depth.
        ASSERT(depthBits == 24 || depthBits == 32);
        copyOffset = depthBits / 8;

        // Stencil is assumed to be 8-bit - currently this is true for all possible formats.
        copySize = 1;
    }

    if (srcDXGIFormat != destDXGIFormat)
    {
        if (srcDXGIFormat == DXGI_FORMAT_R24G8_TYPELESS)
        {
            ASSERT(sourceArea == destArea && sourceSize == destSize && scissor == nullptr);
            return copyAndConvert(context, source, sourceSubresource, sourceArea, sourceSize, dest,
                                  destSubresource, destArea, destSize, scissor, copyOffset,
                                  copyOffset, copySize, srcPixelSize, destPixelSize,
                                  BlitD24S8ToD32F);
        }
        ASSERT(srcDXGIFormat == DXGI_FORMAT_R32G8X24_TYPELESS);
        return copyAndConvert(context, source, sourceSubresource, sourceArea, sourceSize, dest,
                              destSubresource, destArea, destSize, scissor, copyOffset, copyOffset,
                              copySize, srcPixelSize, destPixelSize, BlitD32FS8ToD32F);
    }

    return copyAndConvert(context, source, sourceSubresource, sourceArea, sourceSize, dest,
                          destSubresource, destArea, destSize, scissor, copyOffset, copyOffset,
                          copySize, srcPixelSize, destPixelSize, StretchedBlitNearest);
}

angle::Result Blit11::copyAndConvertImpl(const gl::Context *context,
                                         const TextureHelper11 &source,
                                         unsigned int sourceSubresource,
                                         const gl::Box &sourceArea,
                                         const gl::Extents &sourceSize,
                                         const TextureHelper11 &destStaging,
                                         const gl::Box &destArea,
                                         const gl::Extents &destSize,
                                         const gl::Rectangle *scissor,
                                         size_t readOffset,
                                         size_t writeOffset,
                                         size_t copySize,
                                         size_t srcPixelStride,
                                         size_t destPixelStride,
                                         BlitConvertFunction *convertFunction)
{
    ANGLE_TRY(initResources(context));

    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

    TextureHelper11 sourceStaging;
    ANGLE_TRY(mRenderer->createStagingTexture(context, ResourceType::Texture2D,
                                              source.getFormatSet(), sourceSize,
                                              StagingAccess::READ, &sourceStaging));

    deviceContext->CopySubresourceRegion(sourceStaging.get(), 0, 0, 0, 0, source.get(),
                                         sourceSubresource, nullptr);

    D3D11_MAPPED_SUBRESOURCE sourceMapping;
    ANGLE_TRY(
        mRenderer->mapResource(context, sourceStaging.get(), 0, D3D11_MAP_READ, 0, &sourceMapping));

    D3D11_MAPPED_SUBRESOURCE destMapping;
    angle::Result error =
        mRenderer->mapResource(context, destStaging.get(), 0, D3D11_MAP_WRITE, 0, &destMapping);
    if (error == angle::Result::Stop)
    {
        deviceContext->Unmap(sourceStaging.get(), 0);
        return error;
    }

    // Clip dest area to the destination size
    gl::Rectangle clipRect = gl::Rectangle(0, 0, destSize.width, destSize.height);

    // Clip dest area to the scissor
    if (scissor)
    {
        if (!gl::ClipRectangle(clipRect, *scissor, &clipRect))
        {
            return angle::Result::Continue;
        }
    }

    convertFunction(sourceArea, destArea, clipRect, sourceSize, sourceMapping.RowPitch,
                    destMapping.RowPitch, readOffset, writeOffset, copySize, srcPixelStride,
                    destPixelStride, static_cast<const uint8_t *>(sourceMapping.pData),
                    static_cast<uint8_t *>(destMapping.pData));

    deviceContext->Unmap(sourceStaging.get(), 0);
    deviceContext->Unmap(destStaging.get(), 0);

    return angle::Result::Continue;
}

angle::Result Blit11::copyAndConvert(const gl::Context *context,
                                     const TextureHelper11 &source,
                                     unsigned int sourceSubresource,
                                     const gl::Box &sourceArea,
                                     const gl::Extents &sourceSize,
                                     const TextureHelper11 &dest,
                                     unsigned int destSubresource,
                                     const gl::Box &destArea,
                                     const gl::Extents &destSize,
                                     const gl::Rectangle *scissor,
                                     size_t readOffset,
                                     size_t writeOffset,
                                     size_t copySize,
                                     size_t srcPixelStride,
                                     size_t destPixelStride,
                                     BlitConvertFunction *convertFunction)
{
    ANGLE_TRY(initResources(context));

    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

    // HACK: Create the destination staging buffer as a read/write texture so
    // ID3D11DevicContext::UpdateSubresource can be called
    //       using it's mapped data as a source
    TextureHelper11 destStaging;
    ANGLE_TRY(mRenderer->createStagingTexture(context, ResourceType::Texture2D, dest.getFormatSet(),
                                              destSize, StagingAccess::READ_WRITE, &destStaging));

    deviceContext->CopySubresourceRegion(destStaging.get(), 0, 0, 0, 0, dest.get(), destSubresource,
                                         nullptr);

    ANGLE_TRY(copyAndConvertImpl(context, source, sourceSubresource, sourceArea, sourceSize,
                                 destStaging, destArea, destSize, scissor, readOffset, writeOffset,
                                 copySize, srcPixelStride, destPixelStride, convertFunction));

    // Work around timeouts/TDRs in older NVIDIA drivers.
    if (mRenderer->getFeatures().depthStencilBlitExtraCopy.enabled)
    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        ANGLE_TRY(
            mRenderer->mapResource(context, destStaging.get(), 0, D3D11_MAP_READ, 0, &mapped));
        deviceContext->UpdateSubresource(dest.get(), destSubresource, nullptr, mapped.pData,
                                         mapped.RowPitch, mapped.DepthPitch);
        deviceContext->Unmap(destStaging.get(), 0);
    }
    else
    {
        deviceContext->CopySubresourceRegion(dest.get(), destSubresource, 0, 0, 0,
                                             destStaging.get(), 0, nullptr);
    }

    return angle::Result::Continue;
}

angle::Result Blit11::addBlitShaderToMap(const gl::Context *context,
                                         BlitShaderType blitShaderType,
                                         ShaderDimension dimension,
                                         const ShaderData &shaderData,
                                         const char *name)
{
    ASSERT(mBlitShaderMap.find(blitShaderType) == mBlitShaderMap.end());

    d3d11::PixelShader ps;
    ANGLE_TRY(mRenderer->allocateResource(GetImplAs<Context11>(context), shaderData, &ps));
    ps.setInternalName(name);

    Shader shader;
    shader.dimension   = dimension;
    shader.pixelShader = std::move(ps);

    mBlitShaderMap[blitShaderType] = std::move(shader);
    return angle::Result::Continue;
}

angle::Result Blit11::addSwizzleShaderToMap(const gl::Context *context,
                                            SwizzleShaderType swizzleShaderType,
                                            ShaderDimension dimension,
                                            const ShaderData &shaderData,
                                            const char *name)
{
    ASSERT(mSwizzleShaderMap.find(swizzleShaderType) == mSwizzleShaderMap.end());

    d3d11::PixelShader ps;
    ANGLE_TRY(mRenderer->allocateResource(GetImplAs<Context11>(context), shaderData, &ps));
    ps.setInternalName(name);

    Shader shader;
    shader.dimension   = dimension;
    shader.pixelShader = std::move(ps);

    mSwizzleShaderMap[swizzleShaderType] = std::move(shader);
    return angle::Result::Continue;
}

void Blit11::clearShaderMap()
{
    mBlitShaderMap.clear();
    mSwizzleShaderMap.clear();
}

Blit11::BlitShaderOperation Blit11::getBlitShaderOperation(GLenum destinationFormat,
                                                           GLenum sourceFormat,
                                                           bool isSrcSigned,
                                                           bool isDestSigned,
                                                           bool unpackPremultiplyAlpha,
                                                           bool unpackUnmultiplyAlpha,
                                                           GLenum destTypeForDownsampling)
{
    bool floatToIntBlit =
        !gl::IsIntegerFormat(sourceFormat) && gl::IsIntegerFormat(destinationFormat);

    if (isSrcSigned)
    {
        ASSERT(!unpackPremultiplyAlpha && !unpackUnmultiplyAlpha);
        switch (destinationFormat)
        {
            case GL_RGBA_INTEGER:
                return RGBAI;
            case GL_RGB_INTEGER:
                return RGBI;
            case GL_RG_INTEGER:
                return RGI;
            case GL_RED_INTEGER:
                return RI;
            default:
                UNREACHABLE();
                return OPERATION_INVALID;
        }
    }
    else if (isDestSigned)
    {
        ASSERT(floatToIntBlit);

        switch (destinationFormat)
        {
            case GL_RGBA_INTEGER:
                if (unpackPremultiplyAlpha == unpackUnmultiplyAlpha)
                {
                    return RGBAF_TOI;
                }
                return unpackPremultiplyAlpha ? RGBAF_TOI_PREMULTIPLY : RGBAF_TOI_UNMULTIPLY;
            case GL_RGB_INTEGER:
            case GL_RG_INTEGER:
            case GL_RED_INTEGER:
                if (unpackPremultiplyAlpha == unpackUnmultiplyAlpha)
                {
                    return RGBF_TOI;
                }
                return unpackPremultiplyAlpha ? RGBF_TOI_PREMULTIPLY : RGBF_TOI_UNMULTIPLY;
            default:
                UNREACHABLE();
                return OPERATION_INVALID;
        }
    }
    else
    {
        // Check for the downsample formats first
        switch (destTypeForDownsampling)
        {
            case GL_UNSIGNED_SHORT_4_4_4_4:
                ASSERT(destinationFormat == GL_RGBA && !floatToIntBlit);
                if (unpackPremultiplyAlpha == unpackUnmultiplyAlpha)
                {
                    return RGBAF_4444;
                }
                else if (unpackPremultiplyAlpha)
                {
                    return RGBAF_4444_PREMULTIPLY;
                }
                else
                {
                    return RGBAF_4444_UNMULTIPLY;
                }

            case GL_UNSIGNED_SHORT_5_6_5:
                ASSERT(destinationFormat == GL_RGB && !floatToIntBlit);
                if (unpackPremultiplyAlpha == unpackUnmultiplyAlpha)
                {
                    return RGBF_565;
                }
                else
                {
                    return unpackPremultiplyAlpha ? RGBF_565_PREMULTIPLY : RGBF_565_UNMULTIPLY;
                }
            case GL_UNSIGNED_SHORT_5_5_5_1:
                if (unpackPremultiplyAlpha == unpackUnmultiplyAlpha)
                {
                    return RGBAF_5551;
                }
                else
                {
                    return unpackPremultiplyAlpha ? RGBAF_5551_PREMULTIPLY : RGBAF_5551_UNMULTIPLY;
                }

            default:
                // By default, use the regular passthrough/multiply/unmultiply shaders.  The above
                // shaders are only needed for some emulated texture formats.
                break;
        }

        if (unpackPremultiplyAlpha != unpackUnmultiplyAlpha || floatToIntBlit)
        {
            switch (destinationFormat)
            {
                case GL_RGBA:
                case GL_BGRA_EXT:
                    ASSERT(!floatToIntBlit);
                    return unpackPremultiplyAlpha ? RGBAF_PREMULTIPLY : RGBAF_UNMULTIPLY;
                case GL_RGB:
                case GL_RG:
                case GL_RED:
                    if (unpackPremultiplyAlpha == unpackUnmultiplyAlpha)
                    {
                        return RGBF_TOUI;
                    }
                    else
                    {
                        return unpackPremultiplyAlpha ? RGBF_PREMULTIPLY : RGBF_UNMULTIPLY;
                    }
                case GL_RGBA_INTEGER:
                    if (unpackPremultiplyAlpha == unpackUnmultiplyAlpha)
                    {
                        return RGBAF_TOUI;
                    }
                    else
                    {
                        return unpackPremultiplyAlpha ? RGBAF_TOUI_PREMULTIPLY
                                                      : RGBAF_TOUI_UNMULTIPLY;
                    }
                case GL_RGB_INTEGER:
                case GL_RG_INTEGER:
                case GL_RED_INTEGER:
                    if (unpackPremultiplyAlpha == unpackUnmultiplyAlpha)
                    {
                        return RGBF_TOUI;
                    }
                    else
                    {
                        return unpackPremultiplyAlpha ? RGBF_TOUI_PREMULTIPLY
                                                      : RGBF_TOUI_UNMULTIPLY;
                    }
                case GL_LUMINANCE:
                    ASSERT(!floatToIntBlit);
                    return unpackPremultiplyAlpha ? LUMAF_PREMULTIPLY : LUMAF_UNMULTIPLY;

                case GL_LUMINANCE_ALPHA:
                    ASSERT(!floatToIntBlit);
                    return unpackPremultiplyAlpha ? LUMAALPHAF_PREMULTIPLY : LUMAALPHAF_UNMULTIPLY;
                case GL_ALPHA:
                    return ALPHA;
                default:
                    UNREACHABLE();
                    return OPERATION_INVALID;
            }
        }
        else
        {
            switch (destinationFormat)
            {
                case GL_RGBA:
                    return RGBAF;
                case GL_RGBA_INTEGER:
                    return RGBAUI;
                case GL_BGRA_EXT:
                    return BGRAF;
                case GL_RGB:
                    return RGBF;
                case GL_RGB_INTEGER:
                    return RGBUI;
                case GL_RG:
                    return RGF;
                case GL_RG_INTEGER:
                    return RGUI;
                case GL_RED:
                    return RF;
                case GL_RED_INTEGER:
                    return RUI;
                case GL_ALPHA:
                    return ALPHA;
                case GL_LUMINANCE:
                    return LUMA;
                case GL_LUMINANCE_ALPHA:
                    return LUMAALPHA;
                default:
                    UNREACHABLE();
                    return OPERATION_INVALID;
            }
        }
    }
}

angle::Result Blit11::getBlitShader(const gl::Context *context,
                                    GLenum destFormat,
                                    GLenum sourceFormat,
                                    bool isSrcSigned,
                                    bool isDestSigned,
                                    bool unpackPremultiplyAlpha,
                                    bool unpackUnmultiplyAlpha,
                                    GLenum destTypeForDownsampling,
                                    ShaderDimension dimension,
                                    const Shader **shader)
{
    BlitShaderOperation blitShaderOperation = OPERATION_INVALID;

    blitShaderOperation = getBlitShaderOperation(destFormat, sourceFormat, isSrcSigned,
                                                 isDestSigned, unpackPremultiplyAlpha,
                                                 unpackUnmultiplyAlpha, destTypeForDownsampling);

    BlitShaderType blitShaderType = BLITSHADER_INVALID;

    blitShaderType = getBlitShaderType(blitShaderOperation, dimension);

    ANGLE_CHECK_HR(GetImplAs<Context11>(context), blitShaderType != BLITSHADER_INVALID,
                   "Internal blit shader type mismatch", E_FAIL);

    auto blitShaderIt = mBlitShaderMap.find(blitShaderType);
    if (blitShaderIt != mBlitShaderMap.end())
    {
        *shader = &blitShaderIt->second;
        return angle::Result::Continue;
    }

    ASSERT(dimension == SHADER_2D || mRenderer->isES3Capable());

    ANGLE_TRY(mapBlitShader(context, blitShaderType));

    blitShaderIt = mBlitShaderMap.find(blitShaderType);
    ASSERT(blitShaderIt != mBlitShaderMap.end());
    *shader = &blitShaderIt->second;
    return angle::Result::Continue;
}

angle::Result Blit11::getSwizzleShader(const gl::Context *context,
                                       GLenum type,
                                       D3D11_SRV_DIMENSION viewDimension,
                                       const Shader **shader)
{
    SwizzleShaderType swizzleShaderType = GetSwizzleShaderType(type, viewDimension);

    ANGLE_CHECK_HR(GetImplAs<Context11>(context), swizzleShaderType != SWIZZLESHADER_INVALID,
                   "Swizzle shader type not found", E_FAIL);

    auto swizzleShaderIt = mSwizzleShaderMap.find(swizzleShaderType);
    if (swizzleShaderIt != mSwizzleShaderMap.end())
    {
        *shader = &swizzleShaderIt->second;
        return angle::Result::Continue;
    }

    // Swizzling shaders (OpenGL ES 3+)
    ASSERT(mRenderer->isES3Capable());

    switch (swizzleShaderType)
    {
        case SWIZZLESHADER_2D_FLOAT:
            ANGLE_TRY(addSwizzleShaderToMap(context, swizzleShaderType, SHADER_2D,
                                            ShaderData(g_PS_SwizzleF2D),
                                            "Blit11 2D F swizzle pixel shader"));
            break;
        case SWIZZLESHADER_2D_UINT:
            ANGLE_TRY(addSwizzleShaderToMap(context, swizzleShaderType, SHADER_2D,
                                            ShaderData(g_PS_SwizzleUI2D),
                                            "Blit11 2D UI swizzle pixel shader"));
            break;
        case SWIZZLESHADER_2D_INT:
            ANGLE_TRY(addSwizzleShaderToMap(context, swizzleShaderType, SHADER_2D,
                                            ShaderData(g_PS_SwizzleI2D),
                                            "Blit11 2D I swizzle pixel shader"));
            break;
        case SWIZZLESHADER_CUBE_FLOAT:
            ANGLE_TRY(addSwizzleShaderToMap(context, swizzleShaderType, SHADER_3D,
                                            ShaderData(g_PS_SwizzleF2DArray),
                                            "Blit11 2D Cube F swizzle pixel shader"));
            break;
        case SWIZZLESHADER_CUBE_UINT:
            ANGLE_TRY(addSwizzleShaderToMap(context, swizzleShaderType, SHADER_3D,
                                            ShaderData(g_PS_SwizzleUI2DArray),
                                            "Blit11 2D Cube UI swizzle pixel shader"));
            break;
        case SWIZZLESHADER_CUBE_INT:
            ANGLE_TRY(addSwizzleShaderToMap(context, swizzleShaderType, SHADER_3D,
                                            ShaderData(g_PS_SwizzleI2DArray),
                                            "Blit11 2D Cube I swizzle pixel shader"));
            break;
        case SWIZZLESHADER_3D_FLOAT:
            ANGLE_TRY(addSwizzleShaderToMap(context, swizzleShaderType, SHADER_3D,
                                            ShaderData(g_PS_SwizzleF3D),
                                            "Blit11 3D F swizzle pixel shader"));
            break;
        case SWIZZLESHADER_3D_UINT:
            ANGLE_TRY(addSwizzleShaderToMap(context, swizzleShaderType, SHADER_3D,
                                            ShaderData(g_PS_SwizzleUI3D),
                                            "Blit11 3D UI swizzle pixel shader"));
            break;
        case SWIZZLESHADER_3D_INT:
            ANGLE_TRY(addSwizzleShaderToMap(context, swizzleShaderType, SHADER_3D,
                                            ShaderData(g_PS_SwizzleI3D),
                                            "Blit11 3D I swizzle pixel shader"));
            break;
        case SWIZZLESHADER_ARRAY_FLOAT:
            ANGLE_TRY(addSwizzleShaderToMap(context, swizzleShaderType, SHADER_3D,
                                            ShaderData(g_PS_SwizzleF2DArray),
                                            "Blit11 2D Array F swizzle pixel shader"));
            break;
        case SWIZZLESHADER_ARRAY_UINT:
            ANGLE_TRY(addSwizzleShaderToMap(context, swizzleShaderType, SHADER_3D,
                                            ShaderData(g_PS_SwizzleUI2DArray),
                                            "Blit11 2D Array UI swizzle pixel shader"));
            break;
        case SWIZZLESHADER_ARRAY_INT:
            ANGLE_TRY(addSwizzleShaderToMap(context, swizzleShaderType, SHADER_3D,
                                            ShaderData(g_PS_SwizzleI2DArray),
                                            "Blit11 2D Array I swizzle pixel shader"));
            break;
        default:
            ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    }

    swizzleShaderIt = mSwizzleShaderMap.find(swizzleShaderType);
    ASSERT(swizzleShaderIt != mSwizzleShaderMap.end());
    *shader = &swizzleShaderIt->second;
    return angle::Result::Continue;
}

angle::Result Blit11::resolveDepth(const gl::Context *context,
                                   RenderTarget11 *depth,
                                   TextureHelper11 *textureOut)
{
    ANGLE_TRY(initResources(context));

    // Multisampled depth stencil SRVs are not available in feature level 10.0
    ASSERT(mRenderer->getRenderer11DeviceCaps().featureLevel > D3D_FEATURE_LEVEL_10_0);

    const auto &extents = depth->getExtents();
    auto *deviceContext = mRenderer->getDeviceContext();
    auto *stateManager  = mRenderer->getStateManager();

    ANGLE_TRY(initResolveDepthOnly(context, depth->getFormatSet(), extents));

    Context11 *context11 = GetImplAs<Context11>(context);

    ANGLE_TRY(mResolveDepthStencilVS.resolve(context11, mRenderer));
    ANGLE_TRY(mResolveDepthPS.resolve(context11, mRenderer));

    // Apply the necessary state changes to the D3D11 immediate device context.
    stateManager->setInputLayout(nullptr);
    stateManager->setPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    stateManager->setDrawShaders(&mResolveDepthStencilVS.getObj(), nullptr,
                                 &mResolveDepthPS.getObj());
    stateManager->setRasterizerState(nullptr);
    stateManager->setDepthStencilState(&mDepthStencilState, 0xFFFFFFFF);
    stateManager->setRenderTargets(nullptr, 0, mResolvedDepthDSView.get());
    stateManager->setSimpleBlendState(nullptr);
    stateManager->setSimpleViewport(extents);

    // Set the viewport
    const d3d11::SharedSRV *srv;
    ANGLE_TRY(depth->getShaderResourceView(context, &srv));

    stateManager->setShaderResourceShared(gl::ShaderType::Fragment, 0, srv);

    // Trigger the blit on the GPU.
    deviceContext->Draw(6, 0);

    *textureOut = mResolvedDepth;
    return angle::Result::Continue;
}

angle::Result Blit11::initResolveDepthOnly(const gl::Context *context,
                                           const d3d11::Format &format,
                                           const gl::Extents &extents)
{
    if (mResolvedDepth.valid() && extents == mResolvedDepth.getExtents() &&
        format.texFormat == mResolvedDepth.getFormat())
    {
        return angle::Result::Continue;
    }

    D3D11_TEXTURE2D_DESC textureDesc;
    textureDesc.Width              = extents.width;
    textureDesc.Height             = extents.height;
    textureDesc.MipLevels          = 1;
    textureDesc.ArraySize          = 1;
    textureDesc.Format             = format.texFormat;
    textureDesc.SampleDesc.Count   = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage              = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags     = 0;
    textureDesc.MiscFlags          = 0;

    Context11 *context11 = GetImplAs<Context11>(context);

    ANGLE_TRY(mRenderer->allocateTexture(context11, textureDesc, format, &mResolvedDepth));
    mResolvedDepth.setInternalName("Blit11::mResolvedDepth");

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags              = 0;
    dsvDesc.Format             = format.dsvFormat;
    dsvDesc.Texture2D.MipSlice = 0;
    dsvDesc.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;

    ANGLE_TRY(mRenderer->allocateResource(context11, dsvDesc, mResolvedDepth.get(),
                                          &mResolvedDepthDSView));
    mResolvedDepthDSView.setInternalName("Blit11::mResolvedDepthDSView");

    // Possibly D3D11 bug or undefined behaviour: Clear the DSV so that our first render
    // works as expected. Otherwise the results of the first use seem to be incorrect.
    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();
    deviceContext->ClearDepthStencilView(mResolvedDepthDSView.get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    return angle::Result::Continue;
}

angle::Result Blit11::initResolveDepthStencil(const gl::Context *context,
                                              const gl::Extents &extents)
{
    // Check if we need to recreate depth stencil view
    if (mResolvedDepthStencil.valid() && extents == mResolvedDepthStencil.getExtents())
    {
        ASSERT(mResolvedDepthStencil.getFormat() == DXGI_FORMAT_R32G32_FLOAT);
        return angle::Result::Continue;
    }

    if (mResolvedDepthStencil.valid())
    {
        releaseResolveDepthStencilResources();
    }

    const auto &formatSet = d3d11::Format::Get(GL_RG32F, mRenderer->getRenderer11DeviceCaps());

    D3D11_TEXTURE2D_DESC textureDesc;
    textureDesc.Width              = extents.width;
    textureDesc.Height             = extents.height;
    textureDesc.MipLevels          = 1;
    textureDesc.ArraySize          = 1;
    textureDesc.Format             = formatSet.texFormat;
    textureDesc.SampleDesc.Count   = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage              = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags          = D3D11_BIND_RENDER_TARGET;
    textureDesc.CPUAccessFlags     = 0;
    textureDesc.MiscFlags          = 0;

    Context11 *context11 = GetImplAs<Context11>(context);

    ANGLE_TRY(
        mRenderer->allocateTexture(context11, textureDesc, formatSet, &mResolvedDepthStencil));
    mResolvedDepthStencil.setInternalName("Blit11::mResolvedDepthStencil");

    ANGLE_TRY(mRenderer->allocateResourceNoDesc(context11, mResolvedDepthStencil.get(),
                                                &mResolvedDepthStencilRTView));
    mResolvedDepthStencilRTView.setInternalName("Blit11::mResolvedDepthStencilRTView");

    return angle::Result::Continue;
}

angle::Result Blit11::resolveStencil(const gl::Context *context,
                                     RenderTarget11 *depthStencil,
                                     bool alsoDepth,
                                     TextureHelper11 *textureOut)
{
    ANGLE_TRY(initResources(context));

    // Multisampled depth stencil SRVs are not available in feature level 10.0
    ASSERT(mRenderer->getRenderer11DeviceCaps().featureLevel > D3D_FEATURE_LEVEL_10_0);

    const auto &extents = depthStencil->getExtents();

    ANGLE_TRY(initResolveDepthStencil(context, extents));

    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();
    auto *stateManager                 = mRenderer->getStateManager();
    ID3D11Resource *stencilResource    = depthStencil->getTexture().get();

    // Check if we need to re-create the stencil SRV.
    if (mStencilSRV.valid())
    {
        ID3D11Resource *priorResource = nullptr;
        mStencilSRV.get()->GetResource(&priorResource);

        if (stencilResource != priorResource)
        {
            mStencilSRV.reset();
        }

        SafeRelease(priorResource);
    }

    Context11 *context11 = GetImplAs<Context11>(context);

    if (!mStencilSRV.valid())
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srViewDesc;
        srViewDesc.Format        = GetStencilSRVFormat(depthStencil->getFormatSet());
        srViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;

        ANGLE_TRY(
            mRenderer->allocateResource(context11, srViewDesc, stencilResource, &mStencilSRV));
        mStencilSRV.setInternalName("Blit11::mStencilSRV");
    }

    // Notify the Renderer that all state should be invalidated.
    ANGLE_TRY(mResolveDepthStencilVS.resolve(context11, mRenderer));

    // Resolving the depth buffer works by sampling the depth in the shader using a SRV, then
    // writing to the resolved depth buffer using SV_Depth. We can't use this method for stencil
    // because SV_StencilRef isn't supported until HLSL 5.1/D3D11.3.
    const d3d11::PixelShader *pixelShader = nullptr;
    if (alsoDepth)
    {
        ANGLE_TRY(mResolveDepthStencilPS.resolve(context11, mRenderer));
        pixelShader = &mResolveDepthStencilPS.getObj();
    }
    else
    {
        ANGLE_TRY(mResolveStencilPS.resolve(context11, mRenderer));
        pixelShader = &mResolveStencilPS.getObj();
    }

    // Apply the necessary state changes to the D3D11 immediate device context.
    stateManager->setInputLayout(nullptr);
    stateManager->setPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    stateManager->setDrawShaders(&mResolveDepthStencilVS.getObj(), nullptr, pixelShader);
    stateManager->setRasterizerState(nullptr);
    stateManager->setDepthStencilState(nullptr, 0xFFFFFFFF);
    stateManager->setRenderTarget(mResolvedDepthStencilRTView.get(), nullptr);
    stateManager->setSimpleBlendState(nullptr);

    // Set the viewport
    stateManager->setSimpleViewport(extents);
    const d3d11::SharedSRV *srv;
    ANGLE_TRY(depthStencil->getShaderResourceView(context, &srv));
    stateManager->setShaderResourceShared(gl::ShaderType::Fragment, 0, srv);
    stateManager->setShaderResource(gl::ShaderType::Fragment, 1, &mStencilSRV);

    // Trigger the blit on the GPU.
    deviceContext->Draw(6, 0);

    gl::Box copyBox(0, 0, 0, extents.width, extents.height, 1);

    ANGLE_TRY(mRenderer->createStagingTexture(context, ResourceType::Texture2D,
                                              depthStencil->getFormatSet(), extents,
                                              StagingAccess::READ_WRITE, textureOut));

    const auto &copyFunction = GetCopyDepthStencilFunction(depthStencil->getInternalFormat());
    const auto &dsFormatSet  = depthStencil->getFormatSet();
    const auto &dsDxgiInfo   = d3d11::GetDXGIFormatSizeInfo(dsFormatSet.texFormat);

    ANGLE_TRY(copyAndConvertImpl(context, mResolvedDepthStencil, 0, copyBox, extents, *textureOut,
                                 copyBox, extents, nullptr, 0, 0, 0, 8u, dsDxgiInfo.pixelBytes,
                                 copyFunction));

    return angle::Result::Continue;
}

void Blit11::releaseResolveDepthStencilResources()
{
    mStencilSRV.reset();
    mResolvedDepthStencilRTView.reset();
}

}  // namespace rx
