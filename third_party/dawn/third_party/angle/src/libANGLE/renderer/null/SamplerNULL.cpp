//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SamplerNULL.cpp:
//    Implements the class methods for SamplerNULL.
//

#include "libANGLE/renderer/null/SamplerNULL.h"

#include "common/debug.h"

namespace rx
{

SamplerNULL::SamplerNULL(const gl::SamplerState &state) : SamplerImpl(state) {}

SamplerNULL::~SamplerNULL() {}

angle::Result SamplerNULL::syncState(const gl::Context *context, const bool dirty)
{
    return angle::Result::Continue;
}

}  // namespace rx
