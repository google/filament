//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SurfaceWGL.h: Base class for WGL surface types

#ifndef LIBANGLE_RENDERER_GL_WGL_SURFACEWGL_H_
#define LIBANGLE_RENDERER_GL_WGL_SURFACEWGL_H_

#include "libANGLE/renderer/gl/SurfaceGL.h"

namespace rx
{
class SurfaceWGL : public SurfaceGL
{
  public:
    SurfaceWGL(const egl::SurfaceState &state) : SurfaceGL(state) {}

    ~SurfaceWGL() override {}

    virtual HDC getDC() const = 0;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_WGL_SURFACEWGL_H_
