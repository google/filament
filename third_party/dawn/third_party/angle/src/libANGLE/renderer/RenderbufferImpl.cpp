//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RenderbufferImpl.cpp: Defines the abstract rx::RenderbufferImpl class.

#include "libANGLE/renderer/RenderbufferImpl.h"

namespace rx
{

angle::Result RenderbufferImpl::onLabelUpdate(const gl::Context *context)
{
    return angle::Result::Continue;
}

}  // namespace rx
