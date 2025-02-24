//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramPipelineWgpu.h:
//    Defines the class interface for ProgramPipelineWgpu, implementing ProgramPipelineImpl.
//

#ifndef LIBANGLE_RENDERER_WGPU_PROGRAMPIPELINEWGPU_H_
#define LIBANGLE_RENDERER_WGPU_PROGRAMPIPELINEWGPU_H_

#include "libANGLE/renderer/ProgramPipelineImpl.h"

namespace rx
{

class ProgramPipelineWgpu : public ProgramPipelineImpl
{
  public:
    ProgramPipelineWgpu(const gl::ProgramPipelineState &state);
    ~ProgramPipelineWgpu() override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_PROGRAMPIPELINEWGPU_H_
