//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TransformFeedbackImpl.cpp: Defines the abstract rx::TransformFeedbackImpl class.

#include "libANGLE/renderer/TransformFeedbackImpl.h"

namespace rx
{

angle::Result TransformFeedbackImpl::onLabelUpdate(const gl::Context *context)
{
    return angle::Result::Continue;
}

}  // namespace rx
