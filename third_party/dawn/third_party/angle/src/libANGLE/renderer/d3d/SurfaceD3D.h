//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SurfaceD3D.h: D3D implementation of an EGL surface

#ifndef LIBANGLE_RENDERER_D3D_SURFACED3D_H_
#define LIBANGLE_RENDERER_D3D_SURFACED3D_H_

#include "libANGLE/renderer/SurfaceImpl.h"
#include "libANGLE/renderer/d3d/NativeWindowD3D.h"

namespace egl
{
class Surface;
}

namespace rx
{
class DisplayD3D;
class SwapChainD3D;
class RendererD3D;

class SurfaceD3D : public SurfaceImpl
{
  public:
    ~SurfaceD3D() override;
    void releaseSwapChain();

    egl::Error initialize(const egl::Display *display) override;

    egl::Error swap(const gl::Context *context) override;
    egl::Error postSubBuffer(const gl::Context *context,
                             EGLint x,
                             EGLint y,
                             EGLint width,
                             EGLint height) override;
    egl::Error querySurfacePointerANGLE(EGLint attribute, void **value) override;
    egl::Error bindTexImage(const gl::Context *context,
                            gl::Texture *texture,
                            EGLint buffer) override;
    egl::Error releaseTexImage(const gl::Context *context, EGLint buffer) override;
    egl::Error getSyncValues(EGLuint64KHR *ust, EGLuint64KHR *msc, EGLuint64KHR *sbc) override;
    egl::Error getMscRate(EGLint *numerator, EGLint *denominator) override;
    void setSwapInterval(const egl::Display *display, EGLint interval) override;
    void setFixedWidth(EGLint width) override;
    void setFixedHeight(EGLint height) override;

    EGLint getWidth() const override;
    EGLint getHeight() const override;

    EGLint isPostSubBufferSupported() const override;
    EGLint getSwapBehavior() const override;

    // D3D implementations
    SwapChainD3D *getSwapChain() const;

    egl::Error resetSwapChain(const egl::Display *display);

    egl::Error checkForOutOfDateSwapChain(DisplayD3D *displayD3D);

    angle::Result getAttachmentRenderTarget(const gl::Context *context,
                                            GLenum binding,
                                            const gl::ImageIndex &imageIndex,
                                            GLsizei samples,
                                            FramebufferAttachmentRenderTarget **rtOut) override;
    angle::Result initializeContents(const gl::Context *context,
                                     GLenum binding,
                                     const gl::ImageIndex &imageIndex) override;

    const angle::Format *getD3DTextureColorFormat() const override;

    egl::Error attachToFramebuffer(const gl::Context *context,
                                   gl::Framebuffer *framebuffer) override;
    egl::Error detachFromFramebuffer(const gl::Context *context,
                                     gl::Framebuffer *framebuffer) override;

  protected:
    SurfaceD3D(const egl::SurfaceState &state,
               RendererD3D *renderer,
               egl::Display *display,
               EGLNativeWindowType window,
               EGLenum buftype,
               EGLClientBuffer clientBuffer,
               const egl::AttributeMap &attribs);

    egl::Error swapRect(DisplayD3D *displayD3D, EGLint x, EGLint y, EGLint width, EGLint height);
    egl::Error resetSwapChain(DisplayD3D *displayD3D, int backbufferWidth, int backbufferHeight);
    egl::Error resizeSwapChain(DisplayD3D *displayD3D, int backbufferWidth, int backbufferHeight);

    RendererD3D *mRenderer;
    egl::Display *mDisplay;

    bool mFixedSize;
    GLint mFixedWidth;
    GLint mFixedHeight;
    GLint mOrientation;

    GLenum mRenderTargetFormat;
    GLenum mDepthStencilFormat;
    const angle::Format *mColorFormat;

    SwapChainD3D *mSwapChain;
    bool mSwapIntervalDirty;

    NativeWindowD3D *mNativeWindow;  // Handler for the Window that the surface is created for.
    EGLint mWidth;
    EGLint mHeight;

    EGLint mSwapInterval;

    HANDLE mShareHandle;
    IUnknown *mD3DTexture;

    EGLenum mBuftype;
};

class WindowSurfaceD3D : public SurfaceD3D
{
  public:
    WindowSurfaceD3D(const egl::SurfaceState &state,
                     RendererD3D *renderer,
                     egl::Display *display,
                     EGLNativeWindowType window,
                     const egl::AttributeMap &attribs);
    ~WindowSurfaceD3D() override;
};

class PbufferSurfaceD3D : public SurfaceD3D
{
  public:
    PbufferSurfaceD3D(const egl::SurfaceState &state,
                      RendererD3D *renderer,
                      egl::Display *display,
                      EGLenum buftype,
                      EGLClientBuffer clientBuffer,
                      const egl::AttributeMap &attribs);
    ~PbufferSurfaceD3D() override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_SURFACED3D_H_
