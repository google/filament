//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RenderTarget9.h: Defines a D3D9-specific wrapper for IDirect3DSurface9 pointers
// retained by Renderbuffers.

#ifndef LIBANGLE_RENDERER_D3D_D3D9_RENDERTARGET9_H_
#define LIBANGLE_RENDERER_D3D_D3D9_RENDERTARGET9_H_

#include "libANGLE/renderer/d3d/RenderTargetD3D.h"

namespace rx
{
class Renderer9;
class SwapChain9;

class RenderTarget9 : public RenderTargetD3D
{
  public:
    RenderTarget9() {}
    ~RenderTarget9() override {}
    // Retrieve the texture that backs this render target, may be null for swap chain render
    // targets.
    virtual IDirect3DBaseTexture9 *getTexture() const = 0;
    virtual size_t getTextureLevel() const            = 0;

    virtual IDirect3DSurface9 *getSurface() const = 0;

    virtual D3DFORMAT getD3DFormat() const = 0;
};

class TextureRenderTarget9 : public RenderTarget9
{
  public:
    TextureRenderTarget9(IDirect3DBaseTexture9 *texture,
                         size_t textureLevel,
                         IDirect3DSurface9 *surface,
                         GLenum internalFormat,
                         GLsizei width,
                         GLsizei height,
                         GLsizei depth,
                         GLsizei samples);
    ~TextureRenderTarget9() override;

    GLsizei getWidth() const override;
    GLsizei getHeight() const override;
    GLsizei getDepth() const override;
    GLenum getInternalFormat() const override;
    GLsizei getSamples() const override;

    IDirect3DBaseTexture9 *getTexture() const override;
    size_t getTextureLevel() const override;
    IDirect3DSurface9 *getSurface() const override;

    D3DFORMAT getD3DFormat() const override;

  private:
    GLsizei mWidth;
    GLsizei mHeight;
    GLsizei mDepth;
    GLenum mInternalFormat;
    D3DFORMAT mD3DFormat;
    GLsizei mSamples;

    IDirect3DBaseTexture9 *mTexture;
    size_t mTextureLevel;
    IDirect3DSurface9 *mRenderTarget;
};

class SurfaceRenderTarget9 : public RenderTarget9
{
  public:
    SurfaceRenderTarget9(SwapChain9 *swapChain, bool depth);
    ~SurfaceRenderTarget9() override;

    GLsizei getWidth() const override;
    GLsizei getHeight() const override;
    GLsizei getDepth() const override;
    GLenum getInternalFormat() const override;
    GLsizei getSamples() const override;

    IDirect3DBaseTexture9 *getTexture() const override;
    size_t getTextureLevel() const override;
    IDirect3DSurface9 *getSurface() const override;

    D3DFORMAT getD3DFormat() const override;

  private:
    SwapChain9 *mSwapChain;
    bool mDepth;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D9_RENDERTARGET9_H_
