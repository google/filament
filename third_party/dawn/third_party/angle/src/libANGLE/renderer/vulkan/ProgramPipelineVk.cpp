//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramPipelineVk.cpp:
//    Implements the class methods for ProgramPipelineVk.
//

#include "libANGLE/renderer/vulkan/ProgramPipelineVk.h"

namespace rx
{

ProgramPipelineVk::ProgramPipelineVk(const gl::ProgramPipelineState &state)
    : ProgramPipelineImpl(state)
{}

ProgramPipelineVk::~ProgramPipelineVk() {}

void ProgramPipelineVk::destroy(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);
    reset(contextVk);
}

void ProgramPipelineVk::reset(ContextVk *contextVk)
{
    getExecutable()->reset(contextVk);
}

angle::Result ProgramPipelineVk::link(const gl::Context *glContext,
                                      const gl::ProgramMergedVaryings &mergedVaryings,
                                      const gl::ProgramVaryingPacking &varyingPacking)
{
    ContextVk *contextVk                      = vk::GetImpl(glContext);
    vk::Renderer *renderer                    = contextVk->getRenderer();
    const gl::ProgramExecutable &glExecutable = mState.getExecutable();
    ProgramExecutableVk *executableVk         = vk::GetImpl(&glExecutable);
    SpvSourceOptions options                  = SpvCreateSourceOptions(contextVk->getFeatures(),
                                                                       renderer->getMaxColorInputAttachmentCount());
    SpvProgramInterfaceInfo spvProgramInterfaceInfo = {};

    reset(contextVk);
    executableVk->clearVariableInfoMap();

    // Now that the program pipeline has all of the programs attached, the various descriptor
    // set/binding locations need to be re-assigned to their correct values.
    const gl::ShaderType linkedTransformFeedbackStage =
        glExecutable.getLinkedTransformFeedbackStage();

    // This should be done before assigning varying locations. Otherwise, we can encounter shader
    // interface mismatching problems when the transform feedback stage is not the vertex stage.
    if (options.supportsTransformFeedbackExtension)
    {
        for (const gl::ShaderType shaderType : glExecutable.getLinkedShaderStages())
        {
            const gl::SharedProgramExecutable &glShaderExecutable =
                mState.getShaderProgramExecutable(shaderType);
            if (glShaderExecutable && gl::ShaderTypeSupportsTransformFeedback(shaderType))
            {
                const bool isTransformFeedbackStage =
                    shaderType == linkedTransformFeedbackStage &&
                    !glShaderExecutable->getLinkedTransformFeedbackVaryings().empty();

                SpvAssignTransformFeedbackLocations(
                    shaderType, *glShaderExecutable.get(), isTransformFeedbackStage,
                    &spvProgramInterfaceInfo, &executableVk->mVariableInfoMap);
            }
        }
    }

    executableVk->mOriginalShaderInfo.clear();

    SpvAssignLocations(options, glExecutable, varyingPacking, linkedTransformFeedbackStage,
                       &spvProgramInterfaceInfo, &executableVk->mVariableInfoMap);

    for (const gl::ShaderType shaderType : glExecutable.getLinkedShaderStages())
    {
        const gl::SharedProgramExecutable &glShaderExecutable =
            mState.getShaderProgramExecutable(shaderType);
        ProgramExecutableVk *programExecutableVk = vk::GetImpl(glShaderExecutable.get());
        executableVk->mDefaultUniformBlocks[shaderType] =
            programExecutableVk->getSharedDefaultUniformBlock(shaderType);

        executableVk->mOriginalShaderInfo.initShaderFromProgram(
            shaderType, programExecutableVk->mOriginalShaderInfo);
    }

    executableVk->setAllDefaultUniformsDirty();

    if (contextVk->getFeatures().varyingsRequireMatchingPrecisionInSpirv.enabled &&
        contextVk->getFeatures().enablePrecisionQualifiers.enabled)
    {
        executableVk->resolvePrecisionMismatch(mergedVaryings);
    }

    executableVk->resetLayout(contextVk);
    ANGLE_TRY(executableVk->createPipelineLayout(contextVk, &contextVk->getPipelineLayoutCache(),
                                                 &contextVk->getDescriptorSetLayoutCache(),
                                                 nullptr));
    ANGLE_TRY(executableVk->initializeDescriptorPools(contextVk,
                                                      &contextVk->getDescriptorSetLayoutCache(),
                                                      &contextVk->getMetaDescriptorPools()));

    angle::Result result = angle::Result::Continue;

    if (contextVk->getFeatures().warmUpPipelineCacheAtLink.enabled)
    {
        ANGLE_TRY(executableVk->warmUpPipelineCache(renderer, contextVk->pipelineRobustness(),
                                                    contextVk->pipelineProtectedAccess()));
    }

    return result;
}  // namespace rx

}  // namespace rx
