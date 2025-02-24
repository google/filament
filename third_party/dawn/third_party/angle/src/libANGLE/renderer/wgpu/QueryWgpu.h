//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// QueryWgpu.h:
//    Defines the class interface for QueryWgpu, implementing QueryImpl.
//

#ifndef LIBANGLE_RENDERER_WGPU_QUERYWGPU_H_
#define LIBANGLE_RENDERER_WGPU_QUERYWGPU_H_

#include "libANGLE/renderer/QueryImpl.h"

namespace rx
{

class QueryWgpu : public QueryImpl
{
  public:
    QueryWgpu(gl::QueryType type);
    ~QueryWgpu() override;

    angle::Result begin(const gl::Context *context) override;
    angle::Result end(const gl::Context *context) override;
    angle::Result queryCounter(const gl::Context *context) override;
    angle::Result getResult(const gl::Context *context, GLint *params) override;
    angle::Result getResult(const gl::Context *context, GLuint *params) override;
    angle::Result getResult(const gl::Context *context, GLint64 *params) override;
    angle::Result getResult(const gl::Context *context, GLuint64 *params) override;
    angle::Result isResultAvailable(const gl::Context *context, bool *available) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_QUERYWGPU_H_
