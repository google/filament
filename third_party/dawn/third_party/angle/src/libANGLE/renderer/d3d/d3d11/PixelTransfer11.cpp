//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PixelTransfer11.cpp:
//   Implementation for buffer-to-texture and texture-to-buffer copies.
//   Used to implement pixel transfers from unpack and to pack buffers.
//

#include "libANGLE/renderer/d3d/d3d11/PixelTransfer11.h"

#include "libANGLE/Buffer.h"
#include "libANGLE/Context.h"
#include "libANGLE/Texture.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/d3d11/Buffer11.h"
#include "libANGLE/renderer/d3d/d3d11/Context11.h"
#include "libANGLE/renderer/d3d/d3d11/RenderTarget11.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/TextureStorage11.h"
#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"
#include "libANGLE/renderer/d3d/d3d11/texture_format_table.h"
#include "libANGLE/renderer/serial_utils.h"

// Precompiled shaders
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/buffertotexture11_gs.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/buffertotexture11_ps_4f.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/buffertotexture11_ps_4i.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/buffertotexture11_ps_4ui.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/buffertotexture11_vs.h"

namespace rx
{

PixelTransfer11::PixelTransfer11(Renderer11 *renderer)
    : mRenderer(renderer),
      mResourcesLoaded(false),
      mBufferToTextureVS(),
      mBufferToTextureGS(),
      mParamsConstantBuffer(),
      mCopyRasterizerState(),
      mCopyDepthStencilState()
{}

PixelTransfer11::~PixelTransfer11() {}

angle::Result PixelTransfer11::loadResources(const gl::Context *context)
{
    if (mResourcesLoaded)
    {
        return angle::Result::Continue;
    }

    D3D11_RASTERIZER_DESC rasterDesc;
    rasterDesc.FillMode              = D3D11_FILL_SOLID;
    rasterDesc.CullMode              = D3D11_CULL_NONE;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthBias             = 0;
    rasterDesc.SlopeScaledDepthBias  = 0.0f;
    rasterDesc.DepthBiasClamp        = 0.0f;
    rasterDesc.DepthClipEnable       = TRUE;
    rasterDesc.ScissorEnable         = FALSE;
    rasterDesc.MultisampleEnable     = FALSE;
    rasterDesc.AntialiasedLineEnable = FALSE;

    Context11 *context11 = GetImplAs<Context11>(context);

    ANGLE_TRY(mRenderer->allocateResource(context11, rasterDesc, &mCopyRasterizerState));

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    depthStencilDesc.DepthEnable                  = true;
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

    ANGLE_TRY(mRenderer->allocateResource(context11, depthStencilDesc, &mCopyDepthStencilState));

    D3D11_BUFFER_DESC constantBufferDesc   = {};
    constantBufferDesc.ByteWidth           = roundUpPow2<UINT>(sizeof(CopyShaderParams), 32u);
    constantBufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
    constantBufferDesc.MiscFlags           = 0;
    constantBufferDesc.StructureByteStride = 0;

    ANGLE_TRY(mRenderer->allocateResource(context11, constantBufferDesc, &mParamsConstantBuffer));
    mParamsConstantBuffer.setInternalName("PixelTransfer11ConstantBuffer");

    // init shaders
    ANGLE_TRY(mRenderer->allocateResource(context11, ShaderData(g_VS_BufferToTexture),
                                          &mBufferToTextureVS));
    mBufferToTextureVS.setInternalName("BufferToTextureVS");

    ANGLE_TRY(mRenderer->allocateResource(context11, ShaderData(g_GS_BufferToTexture),
                                          &mBufferToTextureGS));
    mBufferToTextureGS.setInternalName("BufferToTextureGS");

    ANGLE_TRY(buildShaderMap(context));

    StructZero(&mParamsData);

    mResourcesLoaded = true;

    return angle::Result::Continue;
}

void PixelTransfer11::setBufferToTextureCopyParams(const gl::Box &destArea,
                                                   const gl::Extents &destSize,
                                                   GLenum internalFormat,
                                                   const gl::PixelUnpackState &unpack,
                                                   unsigned int offset,
                                                   CopyShaderParams *parametersOut)
{
    StructZero(parametersOut);

    float texelCenterX = 0.5f / static_cast<float>(destSize.width);
    float texelCenterY = 0.5f / static_cast<float>(destSize.height);

    unsigned int bytesPerPixel  = gl::GetSizedInternalFormatInfo(internalFormat).pixelBytes;
    unsigned int alignmentBytes = static_cast<unsigned int>(unpack.alignment);
    unsigned int alignmentPixels =
        (alignmentBytes <= bytesPerPixel ? 1 : alignmentBytes / bytesPerPixel);

    parametersOut->FirstPixelOffset = offset / bytesPerPixel;
    parametersOut->PixelsPerRow =
        static_cast<unsigned int>((unpack.rowLength > 0) ? unpack.rowLength : destArea.width);
    parametersOut->RowStride    = roundUp(parametersOut->PixelsPerRow, alignmentPixels);
    parametersOut->RowsPerSlice = static_cast<unsigned int>(destArea.height);
    parametersOut->PositionOffset[0] =
        texelCenterX + (destArea.x / float(destSize.width)) * 2.0f - 1.0f;
    parametersOut->PositionOffset[1] =
        texelCenterY + ((destSize.height - destArea.y - 1) / float(destSize.height)) * 2.0f - 1.0f;
    parametersOut->PositionScale[0] = 2.0f / static_cast<float>(destSize.width);
    parametersOut->PositionScale[1] = -2.0f / static_cast<float>(destSize.height);
    parametersOut->FirstSlice       = destArea.z;
}

angle::Result PixelTransfer11::copyBufferToTexture(const gl::Context *context,
                                                   const gl::PixelUnpackState &unpack,
                                                   gl::Buffer *unpackBuffer,
                                                   unsigned int offset,
                                                   RenderTargetD3D *destRenderTarget,
                                                   GLenum destinationFormat,
                                                   GLenum sourcePixelsType,
                                                   const gl::Box &destArea)
{
    ASSERT(unpackBuffer);

    ANGLE_TRY(loadResources(context));

    gl::Extents destSize = destRenderTarget->getExtents();

    ASSERT(destArea.x >= 0 && destArea.x + destArea.width <= destSize.width && destArea.y >= 0 &&
           destArea.y + destArea.height <= destSize.height && destArea.z >= 0 &&
           destArea.z + destArea.depth <= destSize.depth);

    ASSERT(mRenderer->supportsFastCopyBufferToTexture(destinationFormat));

    const d3d11::PixelShader *pixelShader = findBufferToTexturePS(destinationFormat);
    ASSERT(pixelShader);

    // The SRV must be in the proper read format, which may be different from the destination format
    // EG: for half float data, we can load full precision floats with implicit conversion
    GLenum unsizedFormat = gl::GetUnsizedFormat(destinationFormat);
    const gl::InternalFormat &sourceglFormatInfo =
        gl::GetInternalFormatInfo(unsizedFormat, sourcePixelsType);

    const d3d11::Format &sourceFormatInfo = d3d11::Format::Get(
        sourceglFormatInfo.sizedInternalFormat, mRenderer->getRenderer11DeviceCaps());
    DXGI_FORMAT srvFormat = sourceFormatInfo.srvFormat;
    ASSERT(srvFormat != DXGI_FORMAT_UNKNOWN);
    Buffer11 *bufferStorage11                  = GetAs<Buffer11>(unpackBuffer->getImplementation());
    const d3d11::ShaderResourceView *bufferSRV = nullptr;
    ANGLE_TRY(bufferStorage11->getSRV(context, srvFormat, &bufferSRV));
    ASSERT(bufferSRV != nullptr);

    const d3d11::RenderTargetView &textureRTV =
        GetAs<RenderTarget11>(destRenderTarget)->getRenderTargetView();
    ASSERT(textureRTV.valid());

    CopyShaderParams shaderParams;
    setBufferToTextureCopyParams(destArea, destSize, sourceglFormatInfo.sizedInternalFormat, unpack,
                                 offset, &shaderParams);

    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

    // Are we doing a 2D or 3D copy?
    const auto *geometryShader   = ((destSize.depth > 1) ? &mBufferToTextureGS : nullptr);
    StateManager11 *stateManager = mRenderer->getStateManager();

    stateManager->setDrawShaders(&mBufferToTextureVS, geometryShader, pixelShader);
    stateManager->setShaderResource(gl::ShaderType::Fragment, 0, bufferSRV);
    stateManager->setInputLayout(nullptr);
    stateManager->setPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    stateManager->setSingleVertexBuffer(nullptr, 0, 0);
    stateManager->setSimpleBlendState(nullptr);
    stateManager->setDepthStencilState(&mCopyDepthStencilState, 0xFFFFFFFF);
    stateManager->setRasterizerState(&mCopyRasterizerState);

    stateManager->setRenderTarget(textureRTV.get(), nullptr);

    if (!StructEquals(mParamsData, shaderParams))
    {
        d3d11::SetBufferData(deviceContext, mParamsConstantBuffer.get(), shaderParams);
        mParamsData = shaderParams;
    }

    stateManager->setVertexConstantBuffer(0, &mParamsConstantBuffer);

    // Set the viewport
    stateManager->setSimpleViewport(destSize);

    UINT numPixels = (shaderParams.PixelsPerRow * destArea.height * destArea.depth);
    deviceContext->Draw(numPixels, 0);

    return angle::Result::Continue;
}

angle::Result PixelTransfer11::buildShaderMap(const gl::Context *context)
{
    d3d11::PixelShader bufferToTextureFloat;
    d3d11::PixelShader bufferToTextureInt;
    d3d11::PixelShader bufferToTextureUint;

    Context11 *context11 = GetImplAs<Context11>(context);

    ANGLE_TRY(mRenderer->allocateResource(context11, ShaderData(g_PS_BufferToTexture_4F),
                                          &bufferToTextureFloat));
    ANGLE_TRY(mRenderer->allocateResource(context11, ShaderData(g_PS_BufferToTexture_4I),
                                          &bufferToTextureInt));
    ANGLE_TRY(mRenderer->allocateResource(context11, ShaderData(g_PS_BufferToTexture_4UI),
                                          &bufferToTextureUint));

    bufferToTextureFloat.setInternalName("BufferToTextureRGBA.ps");
    bufferToTextureInt.setInternalName("BufferToTextureRGBA-I.ps");
    bufferToTextureUint.setInternalName("BufferToTextureRGBA-UI.ps");

    mBufferToTexturePSMap[GL_FLOAT]        = std::move(bufferToTextureFloat);
    mBufferToTexturePSMap[GL_INT]          = std::move(bufferToTextureInt);
    mBufferToTexturePSMap[GL_UNSIGNED_INT] = std::move(bufferToTextureUint);

    return angle::Result::Continue;
}

const d3d11::PixelShader *PixelTransfer11::findBufferToTexturePS(GLenum internalFormat) const
{
    GLenum componentType = gl::GetSizedInternalFormatInfo(internalFormat).componentType;
    if (componentType == GL_SIGNED_NORMALIZED || componentType == GL_UNSIGNED_NORMALIZED)
    {
        componentType = GL_FLOAT;
    }

    auto shaderMapIt = mBufferToTexturePSMap.find(componentType);
    return (shaderMapIt == mBufferToTexturePSMap.end() ? nullptr : &shaderMapIt->second);
}

}  // namespace rx
