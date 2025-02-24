//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramPipelineImpl.cpp: Defines the abstract rx::ProgramPipelineImpl class.

#include "libANGLE/renderer/ProgramPipelineImpl.h"

namespace rx
{

angle::Result ProgramPipelineImpl::link(const gl::Context *context,
                                        const gl::ProgramMergedVaryings &mergedVaryings,
                                        const gl::ProgramVaryingPacking &varyingPacking)
{
    return angle::Result::Continue;
}

angle::Result ProgramPipelineImpl::onLabelUpdate(const gl::Context *context)
{
    return angle::Result::Continue;
}

}  // namespace rx
