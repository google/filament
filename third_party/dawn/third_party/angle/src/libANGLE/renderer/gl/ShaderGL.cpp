//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ShaderGL.cpp: Implements the class methods for ShaderGL.

#include "libANGLE/renderer/gl/ShaderGL.h"

#include "common/debug.h"
#include "libANGLE/Compiler.h"
#include "libANGLE/Context.h"
#include "libANGLE/renderer/ContextImpl.h"
#include "libANGLE/renderer/gl/ContextGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/trace.h"
#include "platform/autogen/FeaturesGL_autogen.h"

#include <iostream>

namespace rx
{
namespace
{
class ShaderTranslateTaskGL final : public ShaderTranslateTask
{
  public:
    ShaderTranslateTaskGL(const FunctionsGL *functions,
                          GLuint shaderID,
                          bool hasNativeParallelCompile)
        : mFunctions(functions),
          mShaderID(shaderID),
          mHasNativeParallelCompile(hasNativeParallelCompile)
    {}
    ~ShaderTranslateTaskGL() override = default;

    void postTranslate(ShHandle compiler, const gl::CompiledShaderState &compiledState) override
    {
        startCompile(compiledState);
    }

    void load(const gl::CompiledShaderState &compiledState) override
    {
        startCompile(compiledState);
    }

    bool isCompilingInternally() override
    {
        if (!mHasNativeParallelCompile)
        {
            return false;
        }

        GLint status = GL_FALSE;
        mFunctions->getShaderiv(mShaderID, GL_COMPLETION_STATUS, &status);
        return status != GL_TRUE;
    }

    angle::Result getResult(std::string &infoLog) override
    {
        // Check for compile errors from the native driver
        GLint compileStatus = GL_FALSE;
        mFunctions->getShaderiv(mShaderID, GL_COMPILE_STATUS, &compileStatus);
        if (compileStatus != GL_FALSE)
        {
            return angle::Result::Continue;
        }

        // Compilation failed, put the error into the info log
        GLint infoLogLength = 0;
        mFunctions->getShaderiv(mShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);

        // Info log length includes the null terminator, so 1 means that the info log is an empty
        // string.
        if (infoLogLength > 1)
        {
            std::vector<char> buf(infoLogLength);
            mFunctions->getShaderInfoLog(mShaderID, infoLogLength, nullptr, &buf[0]);

            infoLog += buf.data();
        }
        else
        {
            WARN() << std::endl << "Shader compilation failed with no info log.";
        }

        return angle::Result::Stop;
    }

  private:
    void startCompile(const gl::CompiledShaderState &compiledState)
    {
        const char *source = compiledState.translatedSource.c_str();
        mFunctions->shaderSource(mShaderID, 1, &source, nullptr);
        mFunctions->compileShader(mShaderID);
    }

    const FunctionsGL *mFunctions;
    GLuint mShaderID;
    bool mHasNativeParallelCompile;
};
}  // anonymous namespace

ShaderGL::ShaderGL(const gl::ShaderState &data, GLuint shaderID)
    : ShaderImpl(data), mShaderID(shaderID)
{}

ShaderGL::~ShaderGL()
{
    ASSERT(mShaderID == 0);
}

void ShaderGL::onDestroy(const gl::Context *context)
{
    const FunctionsGL *functions = GetFunctionsGL(context);

    functions->deleteShader(mShaderID);
    mShaderID = 0;
}

std::shared_ptr<ShaderTranslateTask> ShaderGL::compile(const gl::Context *context,
                                                       ShCompileOptions *options)
{
    ContextGL *contextGL         = GetImplAs<ContextGL>(context);
    const FunctionsGL *functions = GetFunctionsGL(context);

    options->initGLPosition = true;

    bool isWebGL = context->isWebGL();
    if (isWebGL && mState.getShaderType() != gl::ShaderType::Compute)
    {
        options->initOutputVariables = true;
    }

    if (isWebGL && !context->getState().getEnableFeature(GL_TEXTURE_RECTANGLE_ANGLE))
    {
        options->disableARBTextureRectangle = true;
    }

    const angle::FeaturesGL &features = GetFeaturesGL(context);

    if (features.initFragmentOutputVariables.enabled)
    {
        options->initFragmentOutputVariables = true;
    }

    if (features.doWhileGLSLCausesGPUHang.enabled)
    {
        options->rewriteDoWhileLoops = true;
    }

    if (features.emulateAbsIntFunction.enabled)
    {
        options->emulateAbsIntFunction = true;
    }

    if (features.addAndTrueToLoopCondition.enabled)
    {
        options->addAndTrueToLoopCondition = true;
    }

    if (features.emulateIsnanFloat.enabled)
    {
        options->emulateIsnanFloatFunction = true;
    }

    if (features.emulateAtan2Float.enabled)
    {
        options->emulateAtan2FloatFunction = true;
    }

    if (features.useUnusedBlocksWithStandardOrSharedLayout.enabled)
    {
        options->useUnusedStandardSharedBlocks = true;
    }

    if (features.removeInvariantAndCentroidForESSL3.enabled)
    {
        options->removeInvariantAndCentroidForESSL3 = true;
    }

    if (features.rewriteFloatUnaryMinusOperator.enabled)
    {
        options->rewriteFloatUnaryMinusOperator = true;
    }

    if (!features.dontInitializeUninitializedLocals.enabled)
    {
        options->initializeUninitializedLocals = true;
    }

    if (features.clampPointSize.enabled)
    {
        options->clampPointSize = true;
    }

    if (features.dontUseLoopsToInitializeVariables.enabled)
    {
        options->dontUseLoopsToInitializeVariables = true;
    }

    if (features.clampFragDepth.enabled)
    {
        options->clampFragDepth = true;
    }

    if (features.rewriteRepeatedAssignToSwizzled.enabled)
    {
        options->rewriteRepeatedAssignToSwizzled = true;
    }

    if (features.preTransformTextureCubeGradDerivatives.enabled)
    {
        options->preTransformTextureCubeGradDerivatives = true;
    }

    if (contextGL->getMultiviewImplementationType() ==
        MultiviewImplementationTypeGL::NV_VIEWPORT_ARRAY2)
    {
        options->initializeBuiltinsForInstancedMultiview = true;
        options->selectViewInNvGLSLVertexShader          = true;
    }

    if (features.clampArrayAccess.enabled || isWebGL)
    {
        options->clampIndirectArrayBounds = true;
    }

    if (features.vertexIDDoesNotIncludeBaseVertex.enabled)
    {
        options->addBaseVertexToVertexID = true;
    }

    if (features.unfoldShortCircuits.enabled)
    {
        options->unfoldShortCircuit = true;
    }

    if (features.removeDynamicIndexingOfSwizzledVector.enabled)
    {
        options->removeDynamicIndexingOfSwizzledVector = true;
    }

    if (features.preAddTexelFetchOffsets.enabled)
    {
        options->rewriteTexelFetchOffsetToTexelFetch = true;
    }

    if (features.regenerateStructNames.enabled)
    {
        options->regenerateStructNames = true;
    }

    if (features.rewriteRowMajorMatrices.enabled)
    {
        options->rewriteRowMajorMatrices = true;
    }

    if (features.passHighpToPackUnormSnormBuiltins.enabled)
    {
        options->passHighpToPackUnormSnormBuiltins = true;
    }

    if (features.emulateClipDistanceState.enabled)
    {
        options->emulateClipDistanceState = true;
    }

    if (features.emulateClipOrigin.enabled)
    {
        options->emulateClipOrigin = true;
    }

    if (features.scalarizeVecAndMatConstructorArgs.enabled)
    {
        options->scalarizeVecAndMatConstructorArgs = true;
    }

    if (features.explicitFragmentLocations.enabled)
    {
        options->explicitFragmentLocations = true;
    }

    if (contextGL->getNativeExtensions().shaderPixelLocalStorageANGLE)
    {
        options->pls = contextGL->getNativePixelLocalStorageOptions();
    }

    return std::shared_ptr<ShaderTranslateTask>(
        new ShaderTranslateTaskGL(functions, mShaderID, contextGL->hasNativeParallelCompile()));
}

std::shared_ptr<ShaderTranslateTask> ShaderGL::load(const gl::Context *context,
                                                    gl::BinaryInputStream *stream)
{
    ContextGL *contextGL         = GetImplAs<ContextGL>(context);
    const FunctionsGL *functions = GetFunctionsGL(context);

    return std::shared_ptr<ShaderTranslateTask>(
        new ShaderTranslateTaskGL(functions, mShaderID, contextGL->hasNativeParallelCompile()));
}

std::string ShaderGL::getDebugInfo() const
{
    return mState.getCompiledState()->translatedSource;
}

GLuint ShaderGL::getShaderID() const
{
    return mShaderID;
}

}  // namespace rx
