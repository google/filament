//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageNULL.cpp:
//    Implements the class methods for ImageNULL.
//

#include "libANGLE/renderer/null/ImageNULL.h"

#include "common/debug.h"

namespace rx
{

ImageNULL::ImageNULL(const egl::ImageState &state) : ImageImpl(state) {}

ImageNULL::~ImageNULL() {}

egl::Error ImageNULL::initialize(const egl::Display *display)
{
    return egl::NoError();
}

angle::Result ImageNULL::orphan(const gl::Context *context, egl::ImageSibling *sibling)
{
    return angle::Result::Continue;
}

}  // namespace rx
