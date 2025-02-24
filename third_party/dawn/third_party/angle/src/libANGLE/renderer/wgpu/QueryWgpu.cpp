//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// QueryWgpu.cpp:
//    Implements the class methods for QueryWgpu.
//

#include "libANGLE/renderer/wgpu/QueryWgpu.h"

#include "common/debug.h"

namespace rx
{

QueryWgpu::QueryWgpu(gl::QueryType type) : QueryImpl(type) {}

QueryWgpu::~QueryWgpu() {}

angle::Result QueryWgpu::begin(const gl::Context *context)
{
    return angle::Result::Continue;
}

angle::Result QueryWgpu::end(const gl::Context *context)
{
    return angle::Result::Continue;
}

angle::Result QueryWgpu::queryCounter(const gl::Context *context)
{
    return angle::Result::Continue;
}

angle::Result QueryWgpu::getResult(const gl::Context *context, GLint *params)
{
    *params = 0;
    return angle::Result::Continue;
}

angle::Result QueryWgpu::getResult(const gl::Context *context, GLuint *params)
{
    *params = 0;
    return angle::Result::Continue;
}

angle::Result QueryWgpu::getResult(const gl::Context *context, GLint64 *params)
{
    *params = 0;
    return angle::Result::Continue;
}

angle::Result QueryWgpu::getResult(const gl::Context *context, GLuint64 *params)
{
    *params = 0;
    return angle::Result::Continue;
}

angle::Result QueryWgpu::isResultAvailable(const gl::Context *context, bool *available)
{
    *available = true;
    return angle::Result::Continue;
}

}  // namespace rx
