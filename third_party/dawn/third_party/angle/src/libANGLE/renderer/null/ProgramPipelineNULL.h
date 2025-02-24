//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramPipelineNULL.h:
//    Defines the class interface for ProgramPipelineNULL, implementing ProgramPipelineImpl.
//

#ifndef LIBANGLE_RENDERER_NULL_PROGRAMPIPELINENULL_H_
#define LIBANGLE_RENDERER_NULL_PROGRAMPIPELINENULL_H_

#include "libANGLE/renderer/ProgramPipelineImpl.h"

namespace rx
{

class ProgramPipelineNULL : public ProgramPipelineImpl
{
  public:
    ProgramPipelineNULL(const gl::ProgramPipelineState &state);
    ~ProgramPipelineNULL() override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_NULL_PROGRAMPIPELINENULL_H_
