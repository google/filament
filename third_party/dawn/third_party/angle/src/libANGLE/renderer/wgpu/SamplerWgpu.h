//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SamplerWgpu.h:
//    Defines the class interface for SamplerWgpu, implementing SamplerImpl.
//

#ifndef LIBANGLE_RENDERER_WGPU_SAMPLERWGPU_H_
#define LIBANGLE_RENDERER_WGPU_SAMPLERWGPU_H_

#include "libANGLE/renderer/SamplerImpl.h"

namespace rx
{

class SamplerWgpu : public SamplerImpl
{
  public:
    SamplerWgpu(const gl::SamplerState &state);
    ~SamplerWgpu() override;

    angle::Result syncState(const gl::Context *context, const bool dirty) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_SAMPLERWGPU_H_
