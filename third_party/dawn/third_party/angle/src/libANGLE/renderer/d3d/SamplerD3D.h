//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SamplerD3D.h: Defines the rx::SamplerD3D class, an implementation of SamplerImpl.

#ifndef LIBANGLE_RENDERER_D3D_SAMPLERD3D_H_
#define LIBANGLE_RENDERER_D3D_SAMPLERD3D_H_

#include "libANGLE/renderer/SamplerImpl.h"

namespace rx
{

class SamplerD3D : public SamplerImpl
{
  public:
    SamplerD3D(const gl::SamplerState &state) : SamplerImpl(state) {}
    ~SamplerD3D() override {}

    angle::Result syncState(const gl::Context *context, const bool dirtyBits) override;
};

inline angle::Result SamplerD3D::syncState(const gl::Context *context, const bool dirtyBits)
{
    return angle::Result::Continue;
}

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_SAMPLERD3D_H_
