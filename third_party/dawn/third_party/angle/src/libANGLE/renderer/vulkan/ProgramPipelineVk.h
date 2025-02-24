//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramPipelineVk.h:
//    Defines the class interface for ProgramPipelineVk, implementing ProgramPipelineImpl.
//

#ifndef LIBANGLE_RENDERER_VULKAN_PROGRAMPIPELINEVK_H_
#define LIBANGLE_RENDERER_VULKAN_PROGRAMPIPELINEVK_H_

#include "libANGLE/renderer/ProgramPipelineImpl.h"

#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/ProgramExecutableVk.h"
#include "libANGLE/renderer/vulkan/ProgramVk.h"

namespace rx
{

class ProgramPipelineVk : public ProgramPipelineImpl
{
  public:
    ProgramPipelineVk(const gl::ProgramPipelineState &state);
    ~ProgramPipelineVk() override;

    void destroy(const gl::Context *context) override;
    void reset(ContextVk *contextVk);

    const ProgramExecutableVk *getExecutable() const
    {
        return vk::GetImpl(&mState.getExecutable());
    }
    ProgramExecutableVk *getExecutable() { return vk::GetImpl(&mState.getExecutable()); }

    angle::Result link(const gl::Context *glContext,
                       const gl::ProgramMergedVaryings &mergedVaryings,
                       const gl::ProgramVaryingPacking &varyingPacking) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_PROGRAMPIPELINEVK_H_
