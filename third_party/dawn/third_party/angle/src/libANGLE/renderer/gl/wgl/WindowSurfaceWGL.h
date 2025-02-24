//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WindowSurfaceWGL.h: WGL implementation of egl::Surface for windows

#ifndef LIBANGLE_RENDERER_GL_WGL_WINDOWSURFACEWGL_H_
#define LIBANGLE_RENDERER_GL_WGL_WINDOWSURFACEWGL_H_

#include "libANGLE/renderer/gl/wgl/SurfaceWGL.h"

#include <GL/wglext.h>

namespace rx
{

class FunctionsWGL;

class WindowSurfaceWGL : public SurfaceWGL
{
  public:
    WindowSurfaceWGL(const egl::SurfaceState &state,
                     EGLNativeWindowType window,
                     int pixelFormat,
                     const FunctionsWGL *functions,
                     EGLint orientation);
    ~WindowSurfaceWGL() override;

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

  private:
    int mPixelFormat;

    HWND mWindow;
    HDC mDeviceContext;

    const FunctionsWGL *mFunctionsWGL;

    EGLint mSwapBehavior;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_WGL_WINDOWSURFACEWGL_H_
