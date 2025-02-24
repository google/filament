//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TransformFeedbackWgpu.h:
//    Defines the class interface for TransformFeedbackWgpu, implementing TransformFeedbackImpl.
//

#ifndef LIBANGLE_RENDERER_WGPU_TRANSFORMFEEDBACKWGPU_H_
#define LIBANGLE_RENDERER_WGPU_TRANSFORMFEEDBACKWGPU_H_

#include "libANGLE/renderer/TransformFeedbackImpl.h"

namespace rx
{

class TransformFeedbackWgpu : public TransformFeedbackImpl
{
  public:
    TransformFeedbackWgpu(const gl::TransformFeedbackState &state);
    ~TransformFeedbackWgpu() override;

    angle::Result begin(const gl::Context *context, gl::PrimitiveMode primitiveMode) override;
    angle::Result end(const gl::Context *context) override;
    angle::Result pause(const gl::Context *context) override;
    angle::Result resume(const gl::Context *context) override;

    angle::Result bindIndexedBuffer(const gl::Context *context,
                                    size_t index,
                                    const gl::OffsetBindingPointer<gl::Buffer> &binding) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_TRANSFORMFEEDBACKWGPU_H_
