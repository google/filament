//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TransformFeedbackImpl_mock.h: Defines a mock of the TransformFeedbackImpl class.

#ifndef LIBANGLE_RENDERER_TRANSFORMFEEDBACKIMPLMOCK_H_
#define LIBANGLE_RENDERER_TRANSFORMFEEDBACKIMPLMOCK_H_

#include "gmock/gmock.h"

#include "libANGLE/renderer/TransformFeedbackImpl.h"

namespace rx
{

class MockTransformFeedbackImpl : public TransformFeedbackImpl
{
  public:
    MockTransformFeedbackImpl(const gl::TransformFeedbackState &state)
        : TransformFeedbackImpl(state)
    {}
    ~MockTransformFeedbackImpl() { destructor(); }

    MOCK_METHOD2(begin, angle::Result(const gl::Context *, gl::PrimitiveMode));
    MOCK_METHOD1(end, angle::Result(const gl::Context *));
    MOCK_METHOD1(pause, angle::Result(const gl::Context *));
    MOCK_METHOD1(resume, angle::Result(const gl::Context *));

    MOCK_METHOD3(bindIndexedBuffer,
                 angle::Result(const gl::Context *,
                               size_t,
                               const gl::OffsetBindingPointer<gl::Buffer> &));

    MOCK_METHOD0(destructor, void());
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_TRANSFORMFEEDBACKIMPLMOCK_H_
