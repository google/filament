//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TransformFeedbackNULL.h:
//    Defines the class interface for TransformFeedbackNULL, implementing TransformFeedbackImpl.
//

#ifndef LIBANGLE_RENDERER_NULL_TRANSFORMFEEDBACKNULL_H_
#define LIBANGLE_RENDERER_NULL_TRANSFORMFEEDBACKNULL_H_

#include "libANGLE/renderer/TransformFeedbackImpl.h"

namespace rx
{

class TransformFeedbackNULL : public TransformFeedbackImpl
{
  public:
    TransformFeedbackNULL(const gl::TransformFeedbackState &state);
    ~TransformFeedbackNULL() override;

    angle::Result begin(const gl::Context *context, gl::PrimitiveMode primitiveMode) override;
    angle::Result end(const gl::Context *context) override;
    angle::Result pause(const gl::Context *context) override;
    angle::Result resume(const gl::Context *context) override;

    angle::Result bindIndexedBuffer(const gl::Context *context,
                                    size_t index,
                                    const gl::OffsetBindingPointer<gl::Buffer> &binding) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_NULL_TRANSFORMFEEDBACKNULL_H_
