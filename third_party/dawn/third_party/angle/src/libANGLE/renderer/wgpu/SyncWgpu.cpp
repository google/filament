//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SyncWgpu.cpp:
//    Implements the class methods for SyncWgpu.
//

#include "libANGLE/renderer/wgpu/SyncWgpu.h"

#include "common/debug.h"

namespace rx
{

SyncWgpu::SyncWgpu() : SyncImpl() {}

SyncWgpu::~SyncWgpu() {}

angle::Result SyncWgpu::set(const gl::Context *context, GLenum condition, GLbitfield flags)
{
    return angle::Result::Continue;
}

angle::Result SyncWgpu::clientWait(const gl::Context *context,
                                   GLbitfield flags,
                                   GLuint64 timeout,
                                   GLenum *outResult)
{
    *outResult = GL_ALREADY_SIGNALED;
    return angle::Result::Continue;
}

angle::Result SyncWgpu::serverWait(const gl::Context *context, GLbitfield flags, GLuint64 timeout)
{
    return angle::Result::Continue;
}

angle::Result SyncWgpu::getStatus(const gl::Context *context, GLint *outResult)
{
    *outResult = GL_SIGNALED;
    return angle::Result::Continue;
}

}  // namespace rx
