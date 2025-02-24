//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ContextWGL.h: Context class for GL on Windows.  Wraps a RendererWGL.

#ifndef LIBANGLE_RENDERER_GL_WGL_CONTEXTWGL_H_
#define LIBANGLE_RENDERER_GL_WGL_CONTEXTWGL_H_

#include "libANGLE/renderer/gl/ContextGL.h"
#include "libANGLE/renderer/gl/wgl/RendererWGL.h"

namespace rx
{
class ContextWGL : public ContextGL
{
  public:
    ContextWGL(const gl::State &state,
               gl::ErrorSet *errorSet,
               const std::shared_ptr<RendererWGL> &renderer);
    ~ContextWGL() override;

    HGLRC getContext() const;

  private:
    std::shared_ptr<RendererWGL> mRendererWGL;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_WGL_RENDERERWGL_H_
