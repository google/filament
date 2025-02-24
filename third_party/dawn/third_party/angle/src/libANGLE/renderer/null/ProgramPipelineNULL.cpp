//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramPipelineNULL.cpp:
//    Implements the class methods for ProgramPipelineNULL.
//

#include "libANGLE/renderer/null/ProgramPipelineNULL.h"

namespace rx
{

ProgramPipelineNULL::ProgramPipelineNULL(const gl::ProgramPipelineState &state)
    : ProgramPipelineImpl(state)
{}

ProgramPipelineNULL::~ProgramPipelineNULL() {}

}  // namespace rx
