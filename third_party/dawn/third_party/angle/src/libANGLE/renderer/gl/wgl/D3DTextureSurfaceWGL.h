
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// D3DTextureSurfaceWGL.h: WGL implementation of egl::Surface for D3D texture interop.

#ifndef LIBANGLE_RENDERER_GL_WGL_D3DTEXTIRESURFACEWGL_H_
#define LIBANGLE_RENDERER_GL_WGL_D3DTEXTIRESURFACEWGL_H_

#include "libANGLE/renderer/gl/wgl/SurfaceWGL.h"

#include <GL/wglext.h>

namespace rx
{

class FunctionsGL;
class FunctionsWGL;
class DisplayWGL;
class StateManagerGL;

class D3DTextureSurfaceWGL : public SurfaceWGL
{
  public:
    D3DTextureSurfaceWGL(const egl::SurfaceState &state,
                         StateManagerGL *stateManager,
                         EGLenum buftype,
                         EGLClientBuffer clientBuffer,
                         DisplayWGL *display,
                         HDC deviceContext,
                         ID3D11Device *displayD3D11Device,
                         ID3D11Device1 *displayD3D11Device1,
                         const FunctionsGL *functionsGL,
                         const FunctionsWGL *functionsWGL);
    ~D3DTextureSurfaceWGL() override;

    static egl::Error ValidateD3DTextureClientBuffer(EGLenum buftype,
                                                     EGLClientBuffer clientBuffer,
                                                     ID3D11Device *d3d11Device,
                                                     ID3D11Device1 *d3d11Device1);

    egl::Error initialize(const egl::Display *display) override;
    egl::Error makeCurrent(const gl::Context *context) override;
    egl::Error unMakeCurrent(const gl::Context *context) override;

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
    const angle::Format *getD3DTextureColorFormat() const override;

    egl::Error attachToFramebuffer(const gl::Context *context,
                                   gl::Framebuffer *framebuffer) override;
    egl::Error detachFromFramebuffer(const gl::Context *context,
                                     gl::Framebuffer *framebuffer) override;

  private:
    EGLenum mBuftype;
    EGLClientBuffer mClientBuffer;

    ID3D11Device *mDisplayD3D11Device;
    ID3D11Device1 *mDisplayD3D11Device1;

    DisplayWGL *mDisplay;
    StateManagerGL *mStateManager;
    const FunctionsGL *mFunctionsGL;
    const FunctionsWGL *mFunctionsWGL;

    HDC mDeviceContext;

    size_t mWidth;
    size_t mHeight;

    const angle::Format *mColorFormat;

    HANDLE mDeviceHandle;
    IUnknown *mObject;
    IDXGIKeyedMutex *mKeyedMutex;
    HANDLE mBoundObjectTextureHandle;
    HANDLE mBoundObjectRenderbufferHandle;

    GLuint mFramebufferID;
    GLuint mColorRenderbufferID;
    GLuint mDepthStencilRenderbufferID;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_WGL_D3DTEXTIRESURFACEWGL_H_
