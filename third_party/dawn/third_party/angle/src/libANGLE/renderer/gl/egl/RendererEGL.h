//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RendererEGL.h: Renderer class for GL on Android/ChromeOS.  Owns an EGL context object.

#ifndef LIBANGLE_RENDERER_GL_EGL_RENDEREREGL_H_
#define LIBANGLE_RENDERER_GL_EGL_RENDEREREGL_H_

#include "libANGLE/renderer/gl/RendererGL.h"

namespace rx
{
class DisplayEGL;

class RendererEGL : public RendererGL
{
  public:
    RendererEGL(std::unique_ptr<FunctionsGL> functionsGL,
                const egl::AttributeMap &attribMap,
                DisplayEGL *display,
                EGLContext context,
                bool isExternalContext);
    ~RendererEGL() override;

    EGLContext getContext() const;

  private:
    DisplayEGL *mDisplay;
    EGLContext mContext;
    const bool mIsExternalContext;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_EGL_RENDEREREGL_H_
