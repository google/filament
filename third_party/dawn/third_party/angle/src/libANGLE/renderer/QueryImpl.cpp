//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// QueryImpl.cpp: Defines the abstract rx::QueryImpl classes.

#include "libANGLE/renderer/QueryImpl.h"

namespace rx
{

void QueryImpl::onDestroy(const gl::Context *context) {}

angle::Result QueryImpl::onLabelUpdate(const gl::Context *context)
{
    return angle::Result::Continue;
}

}  // namespace rx
