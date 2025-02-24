//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RenderTarget9.cpp: Implements a D3D9-specific wrapper for IDirect3DSurface9
// pointers retained by renderbuffers.

#include "libANGLE/renderer/d3d/d3d9/RenderTarget9.h"
#include "libANGLE/renderer/d3d/d3d9/Renderer9.h"
#include "libANGLE/renderer/d3d/d3d9/SwapChain9.h"
#include "libANGLE/renderer/d3d/d3d9/formatutils9.h"
#include "libANGLE/renderer/d3d/d3d9/renderer9_utils.h"

namespace rx
{

// TODO: AddRef the incoming surface to take ownership instead of expecting that its ref is being
// given.
TextureRenderTarget9::TextureRenderTarget9(IDirect3DBaseTexture9 *texture,
                                           size_t textureLevel,
                                           IDirect3DSurface9 *surface,
                                           GLenum internalFormat,
                                           GLsizei width,
                                           GLsizei height,
                                           GLsizei depth,
                                           GLsizei samples)
    : mWidth(width),
      mHeight(height),
      mDepth(depth),
      mInternalFormat(internalFormat),
      mD3DFormat(D3DFMT_UNKNOWN),
      mSamples(samples),
      mTexture(texture),
      mTextureLevel(textureLevel),
      mRenderTarget(surface)
{
    ASSERT(mDepth == 1);

    if (mRenderTarget)
    {
        D3DSURFACE_DESC description;
        mRenderTarget->GetDesc(&description);
        mD3DFormat = description.Format;
    }
}

TextureRenderTarget9::~TextureRenderTarget9()
{
    SafeRelease(mTexture);
    SafeRelease(mRenderTarget);
}

GLsizei TextureRenderTarget9::getWidth() const
{
    return mWidth;
}

GLsizei TextureRenderTarget9::getHeight() const
{
    return mHeight;
}

GLsizei TextureRenderTarget9::getDepth() const
{
    return mDepth;
}

GLenum TextureRenderTarget9::getInternalFormat() const
{
    return mInternalFormat;
}

GLsizei TextureRenderTarget9::getSamples() const
{
    return mSamples;
}

IDirect3DBaseTexture9 *TextureRenderTarget9::getTexture() const
{
    return mTexture;
}

size_t TextureRenderTarget9::getTextureLevel() const
{
    return mTextureLevel;
}

IDirect3DSurface9 *TextureRenderTarget9::getSurface() const
{
    // Caller is responsible for releasing the returned surface reference.
    // TODO: remove the AddRef to match RenderTarget11
    if (mRenderTarget)
    {
        mRenderTarget->AddRef();
    }

    return mRenderTarget;
}

D3DFORMAT TextureRenderTarget9::getD3DFormat() const
{
    return mD3DFormat;
}

SurfaceRenderTarget9::SurfaceRenderTarget9(SwapChain9 *swapChain, bool depth)
    : mSwapChain(swapChain), mDepth(depth)
{}

SurfaceRenderTarget9::~SurfaceRenderTarget9() {}

GLsizei SurfaceRenderTarget9::getWidth() const
{
    return mSwapChain->getWidth();
}

GLsizei SurfaceRenderTarget9::getHeight() const
{
    return mSwapChain->getHeight();
}

GLsizei SurfaceRenderTarget9::getDepth() const
{
    return 1;
}

GLenum SurfaceRenderTarget9::getInternalFormat() const
{
    return (mDepth ? mSwapChain->getDepthBufferInternalFormat()
                   : mSwapChain->getRenderTargetInternalFormat());
}

GLsizei SurfaceRenderTarget9::getSamples() const
{
    // Our EGL surfaces do not support multisampling.
    return 0;
}

IDirect3DSurface9 *SurfaceRenderTarget9::getSurface() const
{
    return (mDepth ? mSwapChain->getDepthStencil() : mSwapChain->getRenderTarget());
}

IDirect3DBaseTexture9 *SurfaceRenderTarget9::getTexture() const
{
    return (mDepth ? nullptr : mSwapChain->getOffscreenTexture());
}

size_t SurfaceRenderTarget9::getTextureLevel() const
{
    return 0;
}

D3DFORMAT SurfaceRenderTarget9::getD3DFormat() const
{
    return d3d9::GetTextureFormatInfo(getInternalFormat()).texFormat;
}

}  // namespace rx
