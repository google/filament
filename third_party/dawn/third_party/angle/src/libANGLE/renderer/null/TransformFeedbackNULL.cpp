//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TransformFeedbackNULL.cpp:
//    Implements the class methods for TransformFeedbackNULL.
//

#include "libANGLE/renderer/null/TransformFeedbackNULL.h"

#include "common/debug.h"

namespace rx
{

TransformFeedbackNULL::TransformFeedbackNULL(const gl::TransformFeedbackState &state)
    : TransformFeedbackImpl(state)
{}

TransformFeedbackNULL::~TransformFeedbackNULL() {}

angle::Result TransformFeedbackNULL::begin(const gl::Context *context,
                                           gl::PrimitiveMode primitiveMode)
{
    return angle::Result::Continue;
}

angle::Result TransformFeedbackNULL::end(const gl::Context *context)
{
    return angle::Result::Continue;
}

angle::Result TransformFeedbackNULL::pause(const gl::Context *context)
{
    return angle::Result::Continue;
}

angle::Result TransformFeedbackNULL::resume(const gl::Context *context)
{
    return angle::Result::Continue;
}

angle::Result TransformFeedbackNULL::bindIndexedBuffer(
    const gl::Context *context,
    size_t index,
    const gl::OffsetBindingPointer<gl::Buffer> &binding)
{
    return angle::Result::Continue;
}

}  // namespace rx
