//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SamplerWgpu.cpp:
//    Implements the class methods for SamplerWgpu.
//

#include "libANGLE/renderer/wgpu/SamplerWgpu.h"

#include "common/debug.h"

namespace rx
{

SamplerWgpu::SamplerWgpu(const gl::SamplerState &state) : SamplerImpl(state) {}

SamplerWgpu::~SamplerWgpu() {}

angle::Result SamplerWgpu::syncState(const gl::Context *context, const bool dirty)
{
    return angle::Result::Continue;
}

}  // namespace rx
