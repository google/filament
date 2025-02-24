//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RenderTarget11.cpp: Implements a DX11-specific wrapper for ID3D11View pointers
// retained by Renderbuffers.

#include "libANGLE/renderer/d3d/d3d11/RenderTarget11.h"

#include "libANGLE/Context.h"
#include "libANGLE/renderer/d3d/d3d11/Context11.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/SwapChain11.h"
#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"
#include "libANGLE/renderer/d3d/d3d11/texture_format_table.h"

namespace rx
{

namespace
{
bool GetTextureProperties(ID3D11Resource *resource, unsigned int *mipLevels, unsigned int *samples)
{
    ID3D11Texture1D *texture1D = d3d11::DynamicCastComObject<ID3D11Texture1D>(resource);
    if (texture1D)
    {
        D3D11_TEXTURE1D_DESC texDesc;
        texture1D->GetDesc(&texDesc);
        SafeRelease(texture1D);

        *mipLevels = texDesc.MipLevels;
        *samples   = 0;

        return true;
    }

    ID3D11Texture2D *texture2D = d3d11::DynamicCastComObject<ID3D11Texture2D>(resource);
    if (texture2D)
    {
        D3D11_TEXTURE2D_DESC texDesc;
        texture2D->GetDesc(&texDesc);
        SafeRelease(texture2D);

        *mipLevels = texDesc.MipLevels;
        *samples   = texDesc.SampleDesc.Count > 1 ? texDesc.SampleDesc.Count : 0;

        return true;
    }

    ID3D11Texture3D *texture3D = d3d11::DynamicCastComObject<ID3D11Texture3D>(resource);
    if (texture3D)
    {
        D3D11_TEXTURE3D_DESC texDesc;
        texture3D->GetDesc(&texDesc);
        SafeRelease(texture3D);

        *mipLevels = texDesc.MipLevels;
        *samples   = 0;

        return true;
    }

    return false;
}

unsigned int GetRTVSubresourceIndex(ID3D11Resource *resource, ID3D11RenderTargetView *view)
{
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    view->GetDesc(&rtvDesc);

    unsigned int mipSlice   = 0;
    unsigned int arraySlice = 0;

    switch (rtvDesc.ViewDimension)
    {
        case D3D11_RTV_DIMENSION_TEXTURE1D:
            mipSlice   = rtvDesc.Texture1D.MipSlice;
            arraySlice = 0;
            break;

        case D3D11_RTV_DIMENSION_TEXTURE1DARRAY:
            mipSlice   = rtvDesc.Texture1DArray.MipSlice;
            arraySlice = rtvDesc.Texture1DArray.FirstArraySlice;
            break;

        case D3D11_RTV_DIMENSION_TEXTURE2D:
            mipSlice   = rtvDesc.Texture2D.MipSlice;
            arraySlice = 0;
            break;

        case D3D11_RTV_DIMENSION_TEXTURE2DARRAY:
            mipSlice   = rtvDesc.Texture2DArray.MipSlice;
            arraySlice = rtvDesc.Texture2DArray.FirstArraySlice;
            break;

        case D3D11_RTV_DIMENSION_TEXTURE2DMS:
            mipSlice   = 0;
            arraySlice = 0;
            break;

        case D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY:
            mipSlice   = 0;
            arraySlice = rtvDesc.Texture2DMSArray.FirstArraySlice;
            break;

        case D3D11_RTV_DIMENSION_TEXTURE3D:
            mipSlice   = rtvDesc.Texture3D.MipSlice;
            arraySlice = 0;
            break;

        case D3D11_RTV_DIMENSION_UNKNOWN:
        case D3D11_RTV_DIMENSION_BUFFER:
            UNIMPLEMENTED();
            break;

        default:
            UNREACHABLE();
            break;
    }

    unsigned int mipLevels, samples;
    GetTextureProperties(resource, &mipLevels, &samples);

    return D3D11CalcSubresource(mipSlice, arraySlice, mipLevels);
}

unsigned int GetDSVSubresourceIndex(ID3D11Resource *resource, ID3D11DepthStencilView *view)
{
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    view->GetDesc(&dsvDesc);

    unsigned int mipSlice   = 0;
    unsigned int arraySlice = 0;

    switch (dsvDesc.ViewDimension)
    {
        case D3D11_DSV_DIMENSION_TEXTURE1D:
            mipSlice   = dsvDesc.Texture1D.MipSlice;
            arraySlice = 0;
            break;

        case D3D11_DSV_DIMENSION_TEXTURE1DARRAY:
            mipSlice   = dsvDesc.Texture1DArray.MipSlice;
            arraySlice = dsvDesc.Texture1DArray.FirstArraySlice;
            break;

        case D3D11_DSV_DIMENSION_TEXTURE2D:
            mipSlice   = dsvDesc.Texture2D.MipSlice;
            arraySlice = 0;
            break;

        case D3D11_DSV_DIMENSION_TEXTURE2DARRAY:
            mipSlice   = dsvDesc.Texture2DArray.MipSlice;
            arraySlice = dsvDesc.Texture2DArray.FirstArraySlice;
            break;

        case D3D11_DSV_DIMENSION_TEXTURE2DMS:
            mipSlice   = 0;
            arraySlice = 0;
            break;

        case D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY:
            mipSlice   = 0;
            arraySlice = dsvDesc.Texture2DMSArray.FirstArraySlice;
            break;

        case D3D11_DSV_DIMENSION_UNKNOWN:
            UNIMPLEMENTED();
            break;

        default:
            UNREACHABLE();
            break;
    }

    unsigned int mipLevels, samples;
    GetTextureProperties(resource, &mipLevels, &samples);

    return D3D11CalcSubresource(mipSlice, arraySlice, mipLevels);
}

GLenum GetSurfaceRTFormat(bool depth, SwapChain11 *swapChain)
{
    return (depth ? swapChain->getDepthBufferInternalFormat()
                  : swapChain->getRenderTargetInternalFormat());
}

const d3d11::Format &GetSurfaceFormatSet(bool depth, SwapChain11 *swapChain, Renderer11 *renderer)
{
    return d3d11::Format::Get(GetSurfaceRTFormat(depth, swapChain),
                              renderer->getRenderer11DeviceCaps());
}

}  // anonymous namespace

RenderTarget11::RenderTarget11(const d3d11::Format &formatSet) : mFormatSet(formatSet) {}

RenderTarget11::~RenderTarget11() {}

TextureRenderTarget11::TextureRenderTarget11(d3d11::RenderTargetView &&rtv,
                                             const TextureHelper11 &resource,
                                             const d3d11::SharedSRV &srv,
                                             const d3d11::SharedSRV &blitSRV,
                                             GLenum internalFormat,
                                             const d3d11::Format &formatSet,
                                             GLsizei width,
                                             GLsizei height,
                                             GLsizei depth,
                                             GLsizei samples)
    : RenderTarget11(formatSet),
      mWidth(width),
      mHeight(height),
      mDepth(depth),
      mInternalFormat(internalFormat),
      mSamples(samples),
      mSubresourceIndex(0),
      mTexture(resource),
      mRenderTarget(std::move(rtv)),
      mDepthStencil(),
      mShaderResource(srv.makeCopy()),
      mBlitShaderResource(blitSRV.makeCopy())
{
    if (mRenderTarget.valid() && mTexture.valid())
    {
        mSubresourceIndex = GetRTVSubresourceIndex(mTexture.get(), mRenderTarget.get());
    }
    ASSERT(mFormatSet.formatID != angle::FormatID::NONE || mWidth == 0 || mHeight == 0);
}

TextureRenderTarget11::TextureRenderTarget11(d3d11::DepthStencilView &&dsv,
                                             const TextureHelper11 &resource,
                                             const d3d11::SharedSRV &srv,
                                             GLenum internalFormat,
                                             const d3d11::Format &formatSet,
                                             GLsizei width,
                                             GLsizei height,
                                             GLsizei depth,
                                             GLsizei samples)
    : RenderTarget11(formatSet),
      mWidth(width),
      mHeight(height),
      mDepth(depth),
      mInternalFormat(internalFormat),
      mSamples(samples),
      mSubresourceIndex(0),
      mTexture(resource),
      mRenderTarget(),
      mDepthStencil(std::move(dsv)),
      mShaderResource(srv.makeCopy()),
      mBlitShaderResource()
{
    if (mDepthStencil.valid() && mTexture.valid())
    {
        mSubresourceIndex = GetDSVSubresourceIndex(mTexture.get(), mDepthStencil.get());
    }
    ASSERT(mFormatSet.formatID != angle::FormatID::NONE || mWidth == 0 || mHeight == 0);
}

TextureRenderTarget11::~TextureRenderTarget11() {}

const TextureHelper11 &TextureRenderTarget11::getTexture() const
{
    return mTexture;
}

const d3d11::RenderTargetView &TextureRenderTarget11::getRenderTargetView() const
{
    return mRenderTarget;
}

const d3d11::DepthStencilView &TextureRenderTarget11::getDepthStencilView() const
{
    return mDepthStencil;
}

angle::Result TextureRenderTarget11::getShaderResourceView(const gl::Context *context,
                                                           const d3d11::SharedSRV **outSRV)
{
    *outSRV = &mShaderResource;
    return angle::Result::Continue;
}

angle::Result TextureRenderTarget11::getBlitShaderResourceView(const gl::Context *context,
                                                               const d3d11::SharedSRV **outSRV)
{
    *outSRV = &mBlitShaderResource;
    return angle::Result::Continue;
}

GLsizei TextureRenderTarget11::getWidth() const
{
    return mWidth;
}

GLsizei TextureRenderTarget11::getHeight() const
{
    return mHeight;
}

GLsizei TextureRenderTarget11::getDepth() const
{
    return mDepth;
}

GLenum TextureRenderTarget11::getInternalFormat() const
{
    return mInternalFormat;
}

GLsizei TextureRenderTarget11::getSamples() const
{
    return mSamples;
}

unsigned int TextureRenderTarget11::getSubresourceIndex() const
{
    return mSubresourceIndex;
}

SurfaceRenderTarget11::SurfaceRenderTarget11(SwapChain11 *swapChain,
                                             Renderer11 *renderer,
                                             bool depth)
    : RenderTarget11(GetSurfaceFormatSet(depth, swapChain, renderer)),
      mSwapChain(swapChain),
      mDepth(depth)
{
    ASSERT(mSwapChain);
}

SurfaceRenderTarget11::~SurfaceRenderTarget11() {}

GLsizei SurfaceRenderTarget11::getWidth() const
{
    return mSwapChain->getWidth();
}

GLsizei SurfaceRenderTarget11::getHeight() const
{
    return mSwapChain->getHeight();
}

GLsizei SurfaceRenderTarget11::getDepth() const
{
    return 1;
}

GLenum SurfaceRenderTarget11::getInternalFormat() const
{
    return GetSurfaceRTFormat(mDepth, mSwapChain);
}

GLsizei SurfaceRenderTarget11::getSamples() const
{
    return mSwapChain->getSamples();
}

const TextureHelper11 &SurfaceRenderTarget11::getTexture() const
{
    return (mDepth ? mSwapChain->getDepthStencilTexture() : mSwapChain->getOffscreenTexture());
}

const d3d11::RenderTargetView &SurfaceRenderTarget11::getRenderTargetView() const
{
    ASSERT(!mDepth);
    return mSwapChain->getRenderTarget();
}

const d3d11::DepthStencilView &SurfaceRenderTarget11::getDepthStencilView() const
{
    ASSERT(mDepth);
    return mSwapChain->getDepthStencil();
}

angle::Result SurfaceRenderTarget11::getShaderResourceView(const gl::Context *context,
                                                           const d3d11::SharedSRV **outSRV)
{
    if (mDepth)
    {
        *outSRV = &mSwapChain->getDepthStencilShaderResource();
    }
    else
    {
        ANGLE_TRY(mSwapChain->getRenderTargetShaderResource(GetImplAs<Context11>(context), outSRV));
    }
    return angle::Result::Continue;
}

angle::Result SurfaceRenderTarget11::getBlitShaderResourceView(const gl::Context *context,
                                                               const d3d11::SharedSRV **outSRV)
{
    // The SurfaceRenderTargetView format should always be such that the normal SRV works for blits.
    return getShaderResourceView(context, outSRV);
}

unsigned int SurfaceRenderTarget11::getSubresourceIndex() const
{
    return 0;
}

}  // namespace rx
