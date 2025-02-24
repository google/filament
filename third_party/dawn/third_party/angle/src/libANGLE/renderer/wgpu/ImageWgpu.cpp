//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageWgpu.cpp:
//    Implements the class methods for ImageWgpu.
//

#include "libANGLE/renderer/wgpu/ImageWgpu.h"

#include "common/debug.h"

namespace rx
{

ImageWgpu::ImageWgpu(const egl::ImageState &state) : ImageImpl(state) {}

ImageWgpu::~ImageWgpu() {}

egl::Error ImageWgpu::initialize(const egl::Display *display)
{
    return egl::NoError();
}

angle::Result ImageWgpu::orphan(const gl::Context *context, egl::ImageSibling *sibling)
{
    return angle::Result::Continue;
}

}  // namespace rx
