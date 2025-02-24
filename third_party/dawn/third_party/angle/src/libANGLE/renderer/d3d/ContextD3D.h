//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ContextD3D: Shared common class for Context9 and Context11.

#ifndef LIBANGLE_RENDERER_CONTEXTD3D_H_
#define LIBANGLE_RENDERER_CONTEXTD3D_H_

#include "libANGLE/renderer/ContextImpl.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"

namespace rx
{
class ContextD3D : public ContextImpl, public d3d::Context
{
  public:
    ContextD3D(const gl::State &state, gl::ErrorSet *errorSet) : ContextImpl(state, errorSet) {}
    ~ContextD3D() override {}
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_CONTEXTD3D_H_
