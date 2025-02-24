//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RenderbufferWgpu.h:
//    Defines the class interface for RenderbufferWgpu, implementing RenderbufferImpl.
//

#ifndef LIBANGLE_RENDERER_WGPU_RENDERBUFFERWGPU_H_
#define LIBANGLE_RENDERER_WGPU_RENDERBUFFERWGPU_H_

#include "libANGLE/renderer/RenderbufferImpl.h"

namespace rx
{

class RenderbufferWgpu : public RenderbufferImpl
{
  public:
    RenderbufferWgpu(const gl::RenderbufferState &state);
    ~RenderbufferWgpu() override;

    angle::Result setStorage(const gl::Context *context,
                             GLenum internalformat,
                             GLsizei width,
                             GLsizei height) override;
    angle::Result setStorageMultisample(const gl::Context *context,
                                        GLsizei samples,
                                        GLenum internalformat,
                                        GLsizei width,
                                        GLsizei height,
                                        gl::MultisamplingMode mode) override;

    angle::Result setStorageEGLImageTarget(const gl::Context *context, egl::Image *image) override;

    angle::Result initializeContents(const gl::Context *context,
                                     GLenum binding,
                                     const gl::ImageIndex &imageIndex) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_RENDERBUFFERWGPU_H_
