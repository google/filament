//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderVk.cpp:
//    Implements the class methods for ShaderVk.
//

#include "libANGLE/renderer/vulkan/ShaderVk.h"

#include "common/debug.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"

namespace rx
{
ShaderVk::ShaderVk(const gl::ShaderState &state) : ShaderImpl(state) {}

ShaderVk::~ShaderVk() {}

std::shared_ptr<ShaderTranslateTask> ShaderVk::compile(const gl::Context *context,
                                                       ShCompileOptions *options)
{
    ContextVk *contextVk = vk::GetImpl(context);

    if (context->isWebGL())
    {
        // Only WebGL requires initialization of local variables, others don't.
        // Extra initialization in spirv shader may affect performance.
        options->initializeUninitializedLocals = true;

        // WebGL shaders may contain OOB array accesses which in turn cause undefined behavior,
        // which may result in security issues. See https://crbug.com/1189110.
        options->clampIndirectArrayBounds = true;

        if (mState.getShaderType() != gl::ShaderType::Compute)
        {
            options->initOutputVariables = true;
        }
    }

    if (contextVk->getFeatures().supportsSPIRV14.enabled)
    {
        options->emitSPIRV14 = true;
    }

    if (contextVk->getFeatures().retainSPIRVDebugInfo.enabled)
    {
        options->outputDebugInfo = true;
    }

    // robustBufferAccess on Vulkan doesn't support bound check on shader local variables
    // but the GL_EXT_robustness does support.
    // Enable the flag clampIndirectArrayBounds to ensure out of bounds local variable writes in
    // shaders are protected when the context has GL_EXT_robustness enabled
    if (contextVk->getShareGroup()->hasAnyContextWithRobustness())
    {
        options->clampIndirectArrayBounds = true;
    }

    if (contextVk->getFeatures().clampPointSize.enabled)
    {
        options->clampPointSize = true;
    }

    if (contextVk->getFeatures().emulateAdvancedBlendEquations.enabled)
    {
        options->addAdvancedBlendEquationsEmulation = true;
    }

    if (!contextVk->getFeatures().enablePrecisionQualifiers.enabled)
    {
        options->ignorePrecisionQualifiers = true;
    }

    if (contextVk->getFeatures().forceFragmentShaderPrecisionHighpToMediump.enabled)
    {
        options->forceShaderPrecisionHighpToMediump = true;
    }

    // Let compiler use specialized constant for pre-rotation.
    if (!contextVk->getFeatures().preferDriverUniformOverSpecConst.enabled)
    {
        options->useSpecializationConstant = true;
    }

    if (contextVk->getFeatures().clampFragDepth.enabled)
    {
        options->clampFragDepth = true;
    }

    if (!contextVk->getFeatures().supportsDepthClipControl.enabled)
    {
        options->addVulkanDepthCorrection = true;
    }

    if (contextVk->getFeatures().supportsTransformFeedbackExtension.enabled)
    {
        options->addVulkanXfbExtensionSupportCode = true;
    }
    else if (mState.getShaderType() == gl::ShaderType::Vertex &&
             contextVk->getFeatures().emulateTransformFeedback.enabled)
    {
        options->addVulkanXfbEmulationSupportCode = true;
    }

    if (contextVk->getFeatures().roundOutputAfterDithering.enabled)
    {
        options->roundOutputAfterDithering = true;
    }

    if (contextVk->getFeatures().appendAliasedMemoryDecorations.enabled)
    {
        options->aliasedUnlessRestrict = true;
    }

    if (contextVk->getFeatures().explicitlyCastMediumpFloatTo16Bit.enabled)
    {
        options->castMediumpFloatTo16Bit = true;
    }

    if (contextVk->getExtensions().shaderPixelLocalStorageANGLE)
    {
        options->pls = contextVk->getNativePixelLocalStorageOptions();
    }

    if (contextVk->getFeatures().avoidOpSelectWithMismatchingRelaxedPrecision.enabled)
    {
        options->avoidOpSelectWithMismatchingRelaxedPrecision = true;
    }

    if (contextVk->getFeatures().wrapSwitchInIfTrue.enabled)
    {
        options->wrapSwitchInIfTrue = true;
    }

    if (contextVk->getFeatures().emulateR32fImageAtomicExchange.enabled)
    {
        options->emulateR32fImageAtomicExchange = true;
    }

    // The Vulkan backend needs no post-processing of the translated shader.
    return std::shared_ptr<ShaderTranslateTask>(new ShaderTranslateTask);
}

std::shared_ptr<ShaderTranslateTask> ShaderVk::load(const gl::Context *context,
                                                    gl::BinaryInputStream *stream)
{
    return std::shared_ptr<ShaderTranslateTask>(new ShaderTranslateTask);
}

std::string ShaderVk::getDebugInfo() const
{
    const sh::BinaryBlob &spirv = mState.getCompiledState()->compiledBinary;
    if (spirv.empty())
    {
        return "";
    }

    std::ostringstream blob;
    if (!mState.getCompiledState()->inputVaryings.empty())
    {
        blob << "Inputs:";
        for (const sh::ShaderVariable &var : mState.getCompiledState()->inputVaryings)
        {
            blob << " " << var.name;
        }
        blob << std::endl;
    }
    if (!mState.getCompiledState()->activeAttributes.empty())
    {
        blob << "Inputs:";
        for (const sh::ShaderVariable &var : mState.getCompiledState()->activeAttributes)
        {
            blob << " " << var.name;
        }
        blob << std::endl;
    }
    if (!mState.getCompiledState()->outputVaryings.empty())
    {
        blob << "Outputs:";
        for (const sh::ShaderVariable &var : mState.getCompiledState()->outputVaryings)
        {
            blob << " " << var.name;
        }
        blob << std::endl;
    }
    if (!mState.getCompiledState()->activeOutputVariables.empty())
    {
        blob << "Outputs:";
        for (const sh::ShaderVariable &var : mState.getCompiledState()->activeOutputVariables)
        {
            blob << " " << var.name;
        }
        blob << std::endl;
    }
    if (!mState.getCompiledState()->uniforms.empty())
    {
        blob << "Uniforms:";
        for (const sh::ShaderVariable &var : mState.getCompiledState()->uniforms)
        {
            blob << " " << var.name;
        }
        blob << std::endl;
    }
    if (!mState.getCompiledState()->uniformBlocks.empty())
    {
        blob << "Uniform blocks:";
        for (const sh::InterfaceBlock &block : mState.getCompiledState()->uniformBlocks)
        {
            blob << " " << block.name;
        }
        blob << std::endl;
    }
    if (!mState.getCompiledState()->shaderStorageBlocks.empty())
    {
        blob << "Storage blocks:";
        for (const sh::InterfaceBlock &block : mState.getCompiledState()->shaderStorageBlocks)
        {
            blob << " " << block.name;
        }
        blob << std::endl;
    }

    blob << R"(
Paste the following SPIR-V binary in https://www.khronos.org/spir/visualizer/ or pass to a recent build of `spirv-dis` (optionally with `--comment --nested-indent`)

Setting the environment variable ANGLE_FEATURE_OVERRIDES_ENABLED=retainSPIRVDebugInfo will retain debug info

)";

    constexpr size_t kIndicesPerRow = 10;
    size_t rowOffset                = 0;
    for (size_t index = 0; index < spirv.size(); ++index, ++rowOffset)
    {
        if (rowOffset == kIndicesPerRow)
        {
            blob << std::endl;
            rowOffset = 0;
        }
        blob << "0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex
             << spirv[index] << ",";
    }

    return blob.str();
}

}  // namespace rx
