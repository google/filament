//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramPipeline11.h:
//    Defines the class interface for ProgramPipeline11, implementing ProgramPipelineImpl.
//

#ifndef LIBANGLE_RENDERER_D3D_D3D11_PROGRAMPIPELINE11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_PROGRAMPIPELINE11_H_

#include "libANGLE/renderer/ProgramPipelineImpl.h"

namespace rx
{

class ProgramPipeline11 : public ProgramPipelineImpl
{
  public:
    ProgramPipeline11(const gl::ProgramPipelineState &state);
    ~ProgramPipeline11() override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_PROGRAMPIPELINE11_H_
