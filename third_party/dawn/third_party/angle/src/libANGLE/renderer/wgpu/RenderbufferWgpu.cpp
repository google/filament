//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RenderbufferWgpu.cpp:
//    Implements the class methods for RenderbufferWgpu.
//

#include "libANGLE/renderer/wgpu/RenderbufferWgpu.h"

#include "common/debug.h"

namespace rx
{

RenderbufferWgpu::RenderbufferWgpu(const gl::RenderbufferState &state) : RenderbufferImpl(state) {}

RenderbufferWgpu::~RenderbufferWgpu() {}

angle::Result RenderbufferWgpu::setStorage(const gl::Context *context,
                                           GLenum internalformat,
                                           GLsizei width,
                                           GLsizei height)
{
    return angle::Result::Continue;
}

angle::Result RenderbufferWgpu::setStorageMultisample(const gl::Context *context,
                                                      GLsizei samples,
                                                      GLenum internalformat,
                                                      GLsizei width,
                                                      GLsizei height,
                                                      gl::MultisamplingMode mode)
{
    return angle::Result::Continue;
}

angle::Result RenderbufferWgpu::setStorageEGLImageTarget(const gl::Context *context,
                                                         egl::Image *image)
{
    return angle::Result::Continue;
}

angle::Result RenderbufferWgpu::initializeContents(const gl::Context *context,
                                                   GLenum binding,
                                                   const gl::ImageIndex &imageIndex)
{
    return angle::Result::Continue;
}

}  // namespace rx
