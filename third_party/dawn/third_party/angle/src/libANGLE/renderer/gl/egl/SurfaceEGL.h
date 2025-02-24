//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SurfaceEGL.h: common interface for EGL surfaces

#ifndef LIBANGLE_RENDERER_GL_EGL_SURFACEEGL_H_
#define LIBANGLE_RENDERER_GL_EGL_SURFACEEGL_H_

#include <EGL/egl.h>

#include "libANGLE/renderer/gl/SurfaceGL.h"
#include "libANGLE/renderer/gl/egl/FunctionsEGL.h"

namespace rx
{

class SurfaceEGL : public SurfaceGL
{
  public:
    SurfaceEGL(const egl::SurfaceState &state, const FunctionsEGL *egl, EGLConfig config);
    ~SurfaceEGL() override;

    egl::Error makeCurrent(const gl::Context *context) override;
    egl::Error swap(const gl::Context *context) override;
    egl::Error swapWithDamage(const gl::Context *context,
                              const EGLint *rects,
                              EGLint n_rects) override;
    egl::Error postSubBuffer(const gl::Context *context,
                             EGLint x,
                             EGLint y,
                             EGLint width,
                             EGLint height) override;
    egl::Error setPresentationTime(EGLnsecsANDROID time) override;
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

    void setTimestampsEnabled(bool enabled) override;
    egl::SupportedCompositorTimings getSupportedCompositorTimings() const override;
    egl::Error getCompositorTiming(EGLint numTimestamps,
                                   const EGLint *names,
                                   EGLnsecsANDROID *values) const override;
    egl::Error getNextFrameId(EGLuint64KHR *frameId) const override;
    egl::SupportedTimestamps getSupportedTimestamps() const override;
    egl::Error getFrameTimestamps(EGLuint64KHR frameId,
                                  EGLint numTimestamps,
                                  const EGLint *timestamps,
                                  EGLnsecsANDROID *values) const override;

    EGLSurface getSurface() const;
    virtual bool isExternal() const;

  protected:
    const FunctionsEGL *mEGL;
    EGLConfig mConfig;
    EGLSurface mSurface;

  private:
    bool mHasSwapBuffersWithDamage;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_EGL_SURFACEEGL_H_
