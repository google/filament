//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FenceNVWgpu.cpp:
//    Implements the class methods for FenceNVWgpu.
//

#include "libANGLE/renderer/wgpu/FenceNVWgpu.h"

#include "common/debug.h"

namespace rx
{

FenceNVWgpu::FenceNVWgpu() : FenceNVImpl() {}

FenceNVWgpu::~FenceNVWgpu() {}

angle::Result FenceNVWgpu::set(const gl::Context *context, GLenum condition)
{
    return angle::Result::Continue;
}

angle::Result FenceNVWgpu::test(const gl::Context *context, GLboolean *outFinished)
{
    *outFinished = GL_TRUE;
    return angle::Result::Continue;
}

angle::Result FenceNVWgpu::finish(const gl::Context *context)
{
    return angle::Result::Continue;
}

}  // namespace rx
