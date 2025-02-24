//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ContextCGL:
//   Mac-specific subclass of ContextGL.
//

#ifndef LIBANGLE_RENDERER_GL_CGL_CONTEXTCGL_H_
#define LIBANGLE_RENDERER_GL_CGL_CONTEXTCGL_H_

#include "libANGLE/renderer/gl/ContextGL.h"
#include "libANGLE/renderer/gl/RendererGL.h"

namespace rx
{
class DisplayCGL;

class ContextCGL : public ContextGL
{
  public:
    ContextCGL(DisplayCGL *display,
               const gl::State &state,
               gl::ErrorSet *errorSet,
               const std::shared_ptr<RendererGL> &renderer,
               bool usesDiscreteGPU);

    void onDestroy(const gl::Context *context) override;

    egl::Error releaseHighPowerGPU(gl::Context *context) override;
    egl::Error reacquireHighPowerGPU(gl::Context *context) override;

  private:
    bool mUsesDiscreteGpu;
    bool mReleasedDiscreteGpu;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_CGL_CONTEXTCGL_H_
