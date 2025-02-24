//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RenderTarget11.h: Defines a DX11-specific wrapper for ID3D11View pointers
// retained by Renderbuffers.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_RENDERTARGET11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_RENDERTARGET11_H_

#include "libANGLE/renderer/d3d/RenderTargetD3D.h"
#include "libANGLE/renderer/d3d/d3d11/ResourceManager11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"
#include "libANGLE/renderer/d3d/d3d11/texture_format_table.h"

namespace rx
{
class SwapChain11;
class Renderer11;

class RenderTarget11 : public RenderTargetD3D
{
  public:
    RenderTarget11(const d3d11::Format &formatSet);
    ~RenderTarget11() override;

    virtual const TextureHelper11 &getTexture() const                                = 0;
    virtual const d3d11::RenderTargetView &getRenderTargetView() const               = 0;
    virtual const d3d11::DepthStencilView &getDepthStencilView() const               = 0;
    virtual angle::Result getShaderResourceView(const gl::Context *context,
                                                const d3d11::SharedSRV **outSRV)     = 0;
    virtual angle::Result getBlitShaderResourceView(const gl::Context *context,
                                                    const d3d11::SharedSRV **outSRV) = 0;

    virtual unsigned int getSubresourceIndex() const = 0;

    const d3d11::Format &getFormatSet() const { return mFormatSet; }

  protected:
    const d3d11::Format &mFormatSet;
};

class TextureRenderTarget11 : public RenderTarget11
{
  public:
    // TextureRenderTarget11 takes ownership of any D3D11 resources it is given and will AddRef them
    TextureRenderTarget11(d3d11::RenderTargetView &&rtv,
                          const TextureHelper11 &resource,
                          const d3d11::SharedSRV &srv,
                          const d3d11::SharedSRV &blitSRV,
                          GLenum internalFormat,
                          const d3d11::Format &formatSet,
                          GLsizei width,
                          GLsizei height,
                          GLsizei depth,
                          GLsizei samples);
    TextureRenderTarget11(d3d11::DepthStencilView &&dsv,
                          const TextureHelper11 &resource,
                          const d3d11::SharedSRV &srv,
                          GLenum internalFormat,
                          const d3d11::Format &formatSet,
                          GLsizei width,
                          GLsizei height,
                          GLsizei depth,
                          GLsizei samples);
    ~TextureRenderTarget11() override;

    GLsizei getWidth() const override;
    GLsizei getHeight() const override;
    GLsizei getDepth() const override;
    GLenum getInternalFormat() const override;
    GLsizei getSamples() const override;

    const TextureHelper11 &getTexture() const override;
    const d3d11::RenderTargetView &getRenderTargetView() const override;
    const d3d11::DepthStencilView &getDepthStencilView() const override;
    angle::Result getShaderResourceView(const gl::Context *context,
                                        const d3d11::SharedSRV **outSRV) override;
    angle::Result getBlitShaderResourceView(const gl::Context *context,
                                            const d3d11::SharedSRV **outSRV) override;

    unsigned int getSubresourceIndex() const override;

  private:
    GLsizei mWidth;
    GLsizei mHeight;
    GLsizei mDepth;
    GLenum mInternalFormat;
    GLsizei mSamples;

    unsigned int mSubresourceIndex;
    TextureHelper11 mTexture;
    d3d11::RenderTargetView mRenderTarget;
    d3d11::DepthStencilView mDepthStencil;
    d3d11::SharedSRV mShaderResource;

    // Shader resource view to use with internal blit shaders. Not set for depth/stencil render
    // targets.
    d3d11::SharedSRV mBlitShaderResource;
};

class SurfaceRenderTarget11 : public RenderTarget11
{
  public:
    SurfaceRenderTarget11(SwapChain11 *swapChain, Renderer11 *renderer, bool depth);
    ~SurfaceRenderTarget11() override;

    GLsizei getWidth() const override;
    GLsizei getHeight() const override;
    GLsizei getDepth() const override;
    GLenum getInternalFormat() const override;
    GLsizei getSamples() const override;

    const TextureHelper11 &getTexture() const override;
    const d3d11::RenderTargetView &getRenderTargetView() const override;
    const d3d11::DepthStencilView &getDepthStencilView() const override;
    angle::Result getShaderResourceView(const gl::Context *context,
                                        const d3d11::SharedSRV **outSRV) override;
    angle::Result getBlitShaderResourceView(const gl::Context *context,
                                            const d3d11::SharedSRV **outSRV) override;

    unsigned int getSubresourceIndex() const override;

  private:
    SwapChain11 *mSwapChain;
    bool mDepth;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_RENDERTARGET11_H_
