//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WindowSurfaceEGL.h: EGL implementation of egl::Surface for windows

#ifndef LIBANGLE_RENDERER_GL_EGL_WINDOWSURFACEEGL_H_
#define LIBANGLE_RENDERER_GL_EGL_WINDOWSURFACEEGL_H_

#include "libANGLE/renderer/gl/egl/SurfaceEGL.h"

namespace rx
{

class WindowSurfaceEGL : public SurfaceEGL
{
  public:
    WindowSurfaceEGL(const egl::SurfaceState &state,
                     const FunctionsEGL *egl,
                     EGLConfig config,
                     EGLNativeWindowType window);
    ~WindowSurfaceEGL() override;

    egl::Error initialize(const egl::Display *display) override;

    egl::Error getBufferAge(const gl::Context *context, EGLint *age) override;

  private:
    EGLNativeWindowType mWindow;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_EGL_WINDOWSURFACEEGL_H_
