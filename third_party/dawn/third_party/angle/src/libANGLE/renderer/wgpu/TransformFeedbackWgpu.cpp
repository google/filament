//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TransformFeedbackWgpu.cpp:
//    Implements the class methods for TransformFeedbackWgpu.
//

#include "libANGLE/renderer/wgpu/TransformFeedbackWgpu.h"

#include "common/debug.h"

namespace rx
{

TransformFeedbackWgpu::TransformFeedbackWgpu(const gl::TransformFeedbackState &state)
    : TransformFeedbackImpl(state)
{}

TransformFeedbackWgpu::~TransformFeedbackWgpu() {}

angle::Result TransformFeedbackWgpu::begin(const gl::Context *context,
                                           gl::PrimitiveMode primitiveMode)
{
    return angle::Result::Continue;
}

angle::Result TransformFeedbackWgpu::end(const gl::Context *context)
{
    return angle::Result::Continue;
}

angle::Result TransformFeedbackWgpu::pause(const gl::Context *context)
{
    return angle::Result::Continue;
}

angle::Result TransformFeedbackWgpu::resume(const gl::Context *context)
{
    return angle::Result::Continue;
}

angle::Result TransformFeedbackWgpu::bindIndexedBuffer(
    const gl::Context *context,
    size_t index,
    const gl::OffsetBindingPointer<gl::Buffer> &binding)
{
    return angle::Result::Continue;
}

}  // namespace rx
