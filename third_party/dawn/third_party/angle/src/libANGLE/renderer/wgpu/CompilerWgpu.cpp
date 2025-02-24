//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CompilerWgpu.cpp:
//    Implements the class methods for CompilerWgpu.
//

#include "libANGLE/renderer/wgpu/CompilerWgpu.h"

#include "common/debug.h"

namespace rx
{

CompilerWgpu::CompilerWgpu() : CompilerImpl() {}

CompilerWgpu::~CompilerWgpu() {}

ShShaderOutput CompilerWgpu::getTranslatorOutputType() const
{
    return SH_WGSL_OUTPUT;
}

}  // namespace rx
