//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SwapChain11.h: Defines a back-end specific class for the D3D11 swap chain.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_SWAPCHAIN11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_SWAPCHAIN11_H_

#include "common/angleutils.h"
#include "libANGLE/renderer/d3d/SwapChainD3D.h"
#include "libANGLE/renderer/d3d/d3d11/RenderTarget11.h"

namespace rx
{
class Renderer11;
class NativeWindow11;

class SwapChain11 final : public SwapChainD3D
{
  public:
    SwapChain11(Renderer11 *renderer,
                NativeWindow11 *nativeWindow,
                HANDLE shareHandle,
                IUnknown *d3dTexture,
                GLenum backBufferFormat,
                GLenum depthBufferFormat,
                EGLint orientation,
                EGLint samples);
    ~SwapChain11() override;

    EGLint resize(DisplayD3D *displayD3D, EGLint backbufferWidth, EGLint backbufferHeight) override;
    EGLint reset(DisplayD3D *displayD3D,
                 EGLint backbufferWidth,
                 EGLint backbufferHeight,
                 EGLint swapInterval) override;
    EGLint swapRect(DisplayD3D *displayD3D,
                    EGLint x,
                    EGLint y,
                    EGLint width,
                    EGLint height) override;
    void recreate() override;

    RenderTargetD3D *getColorRenderTarget() override;
    RenderTargetD3D *getDepthStencilRenderTarget() override;

    const TextureHelper11 &getOffscreenTexture();
    const d3d11::RenderTargetView &getRenderTarget();
    angle::Result getRenderTargetShaderResource(d3d::Context *context,
                                                const d3d11::SharedSRV **outSRV);

    const TextureHelper11 &getDepthStencilTexture();
    const d3d11::DepthStencilView &getDepthStencil();
    const d3d11::SharedSRV &getDepthStencilShaderResource();

    EGLint getWidth() const { return mWidth; }
    EGLint getHeight() const { return mHeight; }
    void *getKeyedMutex() override;
    EGLint getSamples() const { return mEGLSamples; }

    egl::Error getSyncValues(EGLuint64KHR *ust, EGLuint64KHR *msc, EGLuint64KHR *sbc) override;

  private:
    void release();
    angle::Result initPassThroughResources(DisplayD3D *displayD3D);

    void releaseOffscreenColorBuffer();
    void releaseOffscreenDepthBuffer();
    EGLint resetOffscreenBuffers(DisplayD3D *displayD3D, int backbufferWidth, int backbufferHeight);
    EGLint resetOffscreenColorBuffer(DisplayD3D *displayD3D,
                                     int backbufferWidth,
                                     int backbufferHeight);
    EGLint resetOffscreenDepthBuffer(DisplayD3D *displayD3D,
                                     int backbufferWidth,
                                     int backbufferHeight);

    DXGI_FORMAT getSwapChainNativeFormat() const;

    EGLint copyOffscreenToBackbuffer(DisplayD3D *displayD3D,
                                     EGLint x,
                                     EGLint y,
                                     EGLint width,
                                     EGLint height);
    EGLint present(DisplayD3D *displayD3D, EGLint x, EGLint y, EGLint width, EGLint height);
    UINT getD3DSamples() const;

    Renderer11 *mRenderer;
    EGLint mWidth;
    EGLint mHeight;
    const EGLint mOrientation;
    bool mAppCreatedShareHandle;
    unsigned int mSwapInterval;
    bool mPassThroughResourcesInit;

    NativeWindow11 *mNativeWindow;  // Handler for the Window that the surface is created for.

    bool mFirstSwap;
    IDXGISwapChain *mSwapChain;
    IDXGISwapChain1 *mSwapChain1;
    IDXGIKeyedMutex *mKeyedMutex;

    TextureHelper11 mBackBufferTexture;
    d3d11::RenderTargetView mBackBufferRTView;
    d3d11::SharedSRV mBackBufferSRView;

    const bool mNeedsOffscreenTexture;
    TextureHelper11 mOffscreenTexture;
    d3d11::RenderTargetView mOffscreenRTView;
    d3d11::SharedSRV mOffscreenSRView;
    bool mNeedsOffscreenTextureCopy;
    TextureHelper11 mOffscreenTextureCopyForSRV;

    TextureHelper11 mDepthStencilTexture;
    d3d11::DepthStencilView mDepthStencilDSView;
    d3d11::SharedSRV mDepthStencilSRView;

    d3d11::Buffer mQuadVB;
    d3d11::SamplerState mPassThroughSampler;
    d3d11::InputLayout mPassThroughIL;
    d3d11::VertexShader mPassThroughVS;
    d3d11::PixelShader mPassThroughOrResolvePS;
    d3d11::RasterizerState mPassThroughRS;

    SurfaceRenderTarget11 mColorRenderTarget;
    SurfaceRenderTarget11 mDepthStencilRenderTarget;

    EGLint mEGLSamples;
    LONGLONG mQPCFrequency;
};

}  // namespace rx
#endif  // LIBANGLE_RENDERER_D3D_D3D11_SWAPCHAIN11_H_
