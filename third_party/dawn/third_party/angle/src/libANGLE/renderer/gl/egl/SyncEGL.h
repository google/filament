//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SyncEGL.h: Defines the rx::SyncEGL class, the EGL implementation of EGL sync objects.

#ifndef LIBANGLE_RENDERER_GL_EGL_SYNCEGL_H_
#define LIBANGLE_RENDERER_GL_EGL_SYNCEGL_H_

#include "libANGLE/renderer/EGLSyncImpl.h"

namespace egl
{
class AttributeMap;
}

namespace rx
{

class FunctionsEGL;

class SyncEGL final : public EGLSyncImpl
{
  public:
    SyncEGL(const FunctionsEGL *egl);
    ~SyncEGL() override;

    void onDestroy(const egl::Display *display) override;

    egl::Error initialize(const egl::Display *display,
                          const gl::Context *context,
                          EGLenum type,
                          const egl::AttributeMap &attribs) override;
    egl::Error clientWait(const egl::Display *display,
                          const gl::Context *context,
                          EGLint flags,
                          EGLTime timeout,
                          EGLint *outResult) override;
    egl::Error serverWait(const egl::Display *display,
                          const gl::Context *context,
                          EGLint flags) override;
    egl::Error getStatus(const egl::Display *display, EGLint *outStatus) override;

    egl::Error dupNativeFenceFD(const egl::Display *display, EGLint *result) const override;

  private:
    const FunctionsEGL *mEGL;

    EGLSync mSync;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_EGL_IMAGEEGL_H_
