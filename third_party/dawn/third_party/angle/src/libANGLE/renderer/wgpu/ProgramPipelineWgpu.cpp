//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramPipelineWgpu.cpp:
//    Implements the class methods for ProgramPipelineWgpu.
//

#include "libANGLE/renderer/wgpu/ProgramPipelineWgpu.h"

namespace rx
{

ProgramPipelineWgpu::ProgramPipelineWgpu(const gl::ProgramPipelineState &state)
    : ProgramPipelineImpl(state)
{}

ProgramPipelineWgpu::~ProgramPipelineWgpu() {}

}  // namespace rx
