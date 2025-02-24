//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SwapChainD3D.h: Defines a back-end specific class that hides the details of the
// implementation-specific swapchain.

#ifndef LIBANGLE_RENDERER_D3D_SWAPCHAIND3D_H_
#define LIBANGLE_RENDERER_D3D_SWAPCHAIND3D_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>

#include "common/angleutils.h"
#include "common/platform.h"
#include "libANGLE/Error.h"

#if !defined(ANGLE_FORCE_VSYNC_OFF)
#    define ANGLE_FORCE_VSYNC_OFF 0
#endif

namespace gl
{
class Context;
}  // namespace gl

namespace egl
{
class Display;
}  // namespace egl

namespace rx
{
class DisplayD3D;
class RenderTargetD3D;

class SwapChainD3D : angle::NonCopyable
{
  public:
    SwapChainD3D(HANDLE shareHandle,
                 IUnknown *d3dTexture,
                 GLenum backBufferFormat,
                 GLenum depthBufferFormat);
    virtual ~SwapChainD3D();

    virtual EGLint resize(DisplayD3D *displayD3D,
                          EGLint backbufferWidth,
                          EGLint backbufferSize) = 0;
    virtual EGLint reset(DisplayD3D *displayD3D,
                         EGLint backbufferWidth,
                         EGLint backbufferHeight,
                         EGLint swapInterval)    = 0;
    virtual EGLint swapRect(DisplayD3D *displayD3D,
                            EGLint x,
                            EGLint y,
                            EGLint width,
                            EGLint height)       = 0;
    virtual void recreate()                      = 0;

    virtual RenderTargetD3D *getColorRenderTarget()        = 0;
    virtual RenderTargetD3D *getDepthStencilRenderTarget() = 0;

    GLenum getRenderTargetInternalFormat() const { return mOffscreenRenderTargetFormat; }
    GLenum getDepthBufferInternalFormat() const { return mDepthBufferFormat; }

    HANDLE getShareHandle() { return mShareHandle; }
    virtual void *getKeyedMutex() = 0;

    virtual egl::Error getSyncValues(EGLuint64KHR *ust, EGLuint64KHR *msc, EGLuint64KHR *sbc) = 0;

  protected:
    const GLenum mOffscreenRenderTargetFormat;
    const GLenum mDepthBufferFormat;

    HANDLE mShareHandle;
    IUnknown *mD3DTexture;
};

}  // namespace rx
#endif  // LIBANGLE_RENDERER_D3D_SWAPCHAIND3D_H_
