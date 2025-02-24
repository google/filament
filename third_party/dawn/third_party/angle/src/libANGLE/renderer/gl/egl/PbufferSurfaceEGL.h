//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PbufferSurfaceEGL.h: EGL implementation of egl::Surface for pbuffers

#ifndef LIBANGLE_RENDERER_GL_EGL_PBUFFERSURFACEEGL_H_
#define LIBANGLE_RENDERER_GL_EGL_PBUFFERSURFACEEGL_H_

#include <EGL/egl.h>
#include <vector>

#include "libANGLE/renderer/gl/egl/SurfaceEGL.h"

namespace rx
{

class PbufferSurfaceEGL : public SurfaceEGL
{
  public:
    PbufferSurfaceEGL(const egl::SurfaceState &state, const FunctionsEGL *egl, EGLConfig config);
    ~PbufferSurfaceEGL() override;

    egl::Error initialize(const egl::Display *display) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_EGL_PBUFFERSURFACEEGL_H_
