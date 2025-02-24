//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DXGISwapChainWindowSurfaceWGL.h: WGL implementation of egl::Surface for windows using a DXGI
// swapchain.

#ifndef LIBANGLE_RENDERER_GL_WGL_DXGISWAPCHAINSURFACEWGL_H_
#define LIBANGLE_RENDERER_GL_WGL_DXGISWAPCHAINSURFACEWGL_H_

#include "libANGLE/renderer/gl/wgl/SurfaceWGL.h"

#include <GL/wglext.h>

namespace rx
{

class FunctionsGL;
class FunctionsWGL;
class DisplayWGL;
class StateManagerGL;

class DXGISwapChainWindowSurfaceWGL : public SurfaceWGL
{
  public:
    DXGISwapChainWindowSurfaceWGL(const egl::SurfaceState &state,
                                  StateManagerGL *stateManager,
                                  EGLNativeWindowType window,
                                  ID3D11Device *device,
                                  HANDLE deviceHandle,
                                  HDC deviceContext,
                                  const FunctionsGL *functionsGL,
                                  const FunctionsWGL *functionsWGL,
                                  EGLint orientation);
    ~DXGISwapChainWindowSurfaceWGL() override;

    egl::Error initialize(const egl::Display *display) override;
    egl::Error makeCurrent(const gl::Context *context) override;

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
    void setSwapInterval(const egl::Display *display, EGLint interval) override;

    EGLint getWidth() const override;
    EGLint getHeight() const override;

    EGLint isPostSubBufferSupported() const override;
    EGLint getSwapBehavior() const override;
    HDC getDC() const override;

    egl::Error attachToFramebuffer(const gl::Context *context,
                                   gl::Framebuffer *framebuffer) override;
    egl::Error detachFromFramebuffer(const gl::Context *context,
                                     gl::Framebuffer *framebuffer) override;

  private:
    egl::Error setObjectsLocked(bool locked);
    egl::Error checkForResize();

    egl::Error createSwapChain();

    EGLNativeWindowType mWindow;

    StateManagerGL *mStateManager;
    const FunctionsGL *mFunctionsGL;
    const FunctionsWGL *mFunctionsWGL;

    ID3D11Device *mDevice;
    HANDLE mDeviceHandle;

    HDC mWGLDevice;

    DXGI_FORMAT mSwapChainFormat;
    UINT mSwapChainFlags;
    GLenum mDepthBufferFormat;

    bool mFirstSwap;
    IDXGISwapChain *mSwapChain;
    IDXGISwapChain1 *mSwapChain1;

    GLuint mFramebufferID;
    GLuint mColorRenderbufferID;
    HANDLE mRenderbufferBufferHandle;

    GLuint mDepthRenderbufferID;

    GLuint mTextureID;
    HANDLE mTextureHandle;

    size_t mWidth;
    size_t mHeight;

    EGLint mSwapInterval;

    EGLint mOrientation;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_WGL_DXGISWAPCHAINSURFACEWGL_H_
