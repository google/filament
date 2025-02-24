//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ShaderD3D.cpp: Defines the rx::ShaderD3D class which implements rx::ShaderImpl.

#include "libANGLE/renderer/d3d/ShaderD3D.h"

#include "common/system_utils.h"
#include "common/utilities.h"
#include "libANGLE/Caps.h"
#include "libANGLE/Compiler.h"
#include "libANGLE/Context.h"
#include "libANGLE/Shader.h"
#include "libANGLE/features.h"
#include "libANGLE/renderer/ContextImpl.h"
#include "libANGLE/renderer/d3d/ProgramD3D.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"
#include "libANGLE/trace.h"

namespace rx
{
namespace
{
const std::map<std::string, unsigned int> &GetUniformRegisterMap(
    const std::map<std::string, unsigned int> *uniformRegisterMap)
{
    ASSERT(uniformRegisterMap);
    return *uniformRegisterMap;
}

const std::set<std::string> &GetSlowCompilingUniformBlockSet(
    const std::set<std::string> *slowCompilingUniformBlockSet)
{
    ASSERT(slowCompilingUniformBlockSet);
    return *slowCompilingUniformBlockSet;
}

const std::set<std::string> &GetUsedImage2DFunctionNames(
    const std::set<std::string> *usedImage2DFunctionNames)
{
    ASSERT(usedImage2DFunctionNames);
    return *usedImage2DFunctionNames;
}

class ShaderTranslateTaskD3D final : public ShaderTranslateTask
{
  public:
    ShaderTranslateTaskD3D(const SharedCompiledShaderStateD3D &shader, std::string &&sourcePath)
        : mSourcePath(std::move(sourcePath)), mShader(shader)
    {}
    ~ShaderTranslateTaskD3D() override = default;

    bool translate(ShHandle compiler,
                   const ShCompileOptions &options,
                   const std::string &source) override
    {
        ANGLE_TRACE_EVENT1("gpu.angle", "ShaderTranslateTaskD3D::run", "source", source);
        angle::FixedVector<const char *, 2> srcStrings;
        if (!mSourcePath.empty())
        {
            srcStrings.push_back(mSourcePath.c_str());
        }
        srcStrings.push_back(source.c_str());

        return sh::Compile(compiler, srcStrings.data(), srcStrings.size(), options);
    }

    void postTranslate(ShHandle compiler, const gl::CompiledShaderState &compiledState) override
    {
        const std::string &translatedSource = compiledState.translatedSource;
        CompiledShaderStateD3D *state       = mShader.get();

        // Note: We shouldn't need to cache this.
        state->compilerOutputType = sh::GetShaderOutputType(compiler);

        state->usesMultipleRenderTargets =
            translatedSource.find("GL_USES_MRT") != std::string::npos;
        state->usesFragColor = translatedSource.find("GL_USES_FRAG_COLOR") != std::string::npos;
        state->usesFragData  = translatedSource.find("GL_USES_FRAG_DATA") != std::string::npos;
        state->usesSecondaryColor =
            translatedSource.find("GL_USES_SECONDARY_COLOR") != std::string::npos;
        state->usesFragCoord   = translatedSource.find("GL_USES_FRAG_COORD") != std::string::npos;
        state->usesFrontFacing = translatedSource.find("GL_USES_FRONT_FACING") != std::string::npos;
        state->usesSampleID    = translatedSource.find("GL_USES_SAMPLE_ID") != std::string::npos;
        state->usesSamplePosition =
            translatedSource.find("GL_USES_SAMPLE_POSITION") != std::string::npos;
        state->usesSampleMaskIn =
            translatedSource.find("GL_USES_SAMPLE_MASK_IN") != std::string::npos;
        state->usesSampleMask =
            translatedSource.find("GL_USES_SAMPLE_MASK_OUT") != std::string::npos;
        state->usesHelperInvocation =
            translatedSource.find("GL_USES_HELPER_INVOCATION") != std::string::npos;
        state->usesPointSize  = translatedSource.find("GL_USES_POINT_SIZE") != std::string::npos;
        state->usesPointCoord = translatedSource.find("GL_USES_POINT_COORD") != std::string::npos;
        state->usesDepthRange = translatedSource.find("GL_USES_DEPTH_RANGE") != std::string::npos;
        state->hasMultiviewEnabled =
            translatedSource.find("GL_MULTIVIEW_ENABLED") != std::string::npos;
        state->usesVertexID = translatedSource.find("GL_USES_VERTEX_ID") != std::string::npos;
        state->usesViewID   = translatedSource.find("GL_USES_VIEW_ID") != std::string::npos;
        state->usesDiscardRewriting =
            translatedSource.find("ANGLE_USES_DISCARD_REWRITING") != std::string::npos;
        state->usesNestedBreak =
            translatedSource.find("ANGLE_USES_NESTED_BREAK") != std::string::npos;
        state->requiresIEEEStrictCompiling =
            translatedSource.find("ANGLE_REQUIRES_IEEE_STRICT_COMPILING") != std::string::npos;

        if (translatedSource.find("GL_USES_FRAG_DEPTH_GREATER") != std::string::npos)
        {
            state->fragDepthUsage = FragDepthUsage::Greater;
        }
        else if (translatedSource.find("GL_USES_FRAG_DEPTH_LESS") != std::string::npos)
        {
            state->fragDepthUsage = FragDepthUsage::Less;
        }
        else if (translatedSource.find("GL_USES_FRAG_DEPTH") != std::string::npos)
        {
            state->fragDepthUsage = FragDepthUsage::Any;
        }
        state->clipDistanceSize   = sh::GetClipDistanceArraySize(compiler);
        state->cullDistanceSize   = sh::GetCullDistanceArraySize(compiler);
        state->uniformRegisterMap = GetUniformRegisterMap(sh::GetUniformRegisterMap(compiler));
        state->readonlyImage2DRegisterIndex = sh::GetReadonlyImage2DRegisterIndex(compiler);
        state->image2DRegisterIndex         = sh::GetImage2DRegisterIndex(compiler);
        state->usedImage2DFunctionNames =
            GetUsedImage2DFunctionNames(sh::GetUsedImage2DFunctionNames(compiler));

        for (const sh::InterfaceBlock &interfaceBlock : compiledState.uniformBlocks)
        {
            if (interfaceBlock.active)
            {
                unsigned int index = static_cast<unsigned int>(-1);
                bool blockRegisterResult =
                    sh::GetUniformBlockRegister(compiler, interfaceBlock.name, &index);
                ASSERT(blockRegisterResult);
                bool useStructuredBuffer =
                    sh::ShouldUniformBlockUseStructuredBuffer(compiler, interfaceBlock.name);

                state->uniformBlockRegisterMap[interfaceBlock.name] = index;
                state->uniformBlockUseStructuredBufferMap[interfaceBlock.name] =
                    useStructuredBuffer;
            }
        }

        state->slowCompilingUniformBlockSet =
            GetSlowCompilingUniformBlockSet(sh::GetSlowCompilingUniformBlockSet(compiler));

        for (const sh::InterfaceBlock &interfaceBlock : compiledState.shaderStorageBlocks)
        {
            if (interfaceBlock.active)
            {
                unsigned int index = static_cast<unsigned int>(-1);
                bool blockRegisterResult =
                    sh::GetShaderStorageBlockRegister(compiler, interfaceBlock.name, &index);
                ASSERT(blockRegisterResult);

                state->shaderStorageBlockRegisterMap[interfaceBlock.name] = index;
            }
        }

        state->debugInfo +=
            "// INITIAL HLSL BEGIN\n\n" + translatedSource + "\n// INITIAL HLSL END\n\n\n";
    }

  private:
    std::string mSourcePath;
    SharedCompiledShaderStateD3D mShader;
};
}  // anonymous namespace

CompiledShaderStateD3D::CompiledShaderStateD3D()
    : compilerOutputType(SH_ESSL_OUTPUT),
      usesMultipleRenderTargets(false),
      usesFragColor(false),
      usesFragData(false),
      usesSecondaryColor(false),
      usesFragCoord(false),
      usesFrontFacing(false),
      usesHelperInvocation(false),
      usesPointSize(false),
      usesPointCoord(false),
      usesDepthRange(false),
      usesSampleID(false),
      usesSamplePosition(false),
      usesSampleMaskIn(false),
      usesSampleMask(false),
      hasMultiviewEnabled(false),
      usesVertexID(false),
      usesViewID(false),
      usesDiscardRewriting(false),
      usesNestedBreak(false),
      requiresIEEEStrictCompiling(false),
      fragDepthUsage(FragDepthUsage::Unused),
      clipDistanceSize(0),
      cullDistanceSize(0),
      readonlyImage2DRegisterIndex(0),
      image2DRegisterIndex(0)
{}

CompiledShaderStateD3D::~CompiledShaderStateD3D() = default;

ShaderD3D::ShaderD3D(const gl::ShaderState &state, RendererD3D *renderer)
    : ShaderImpl(state), mRenderer(renderer)
{}

ShaderD3D::~ShaderD3D() {}

std::string ShaderD3D::getDebugInfo() const
{
    if (!mCompiledState || mCompiledState->debugInfo.empty())
    {
        return "";
    }

    return mCompiledState->debugInfo + std::string("\n// ") +
           gl::GetShaderTypeString(mState.getShaderType()) + " SHADER END\n";
}

void CompiledShaderStateD3D::generateWorkarounds(CompilerWorkaroundsD3D *workarounds) const
{
    if (usesDiscardRewriting)
    {
        // ANGLE issue 486:
        // Work-around a D3D9 compiler bug that presents itself when using conditional discard, by
        // disabling optimization
        workarounds->skipOptimization = true;
    }
    else if (usesNestedBreak)
    {
        // ANGLE issue 603:
        // Work-around a D3D9 compiler bug that presents itself when using break in a nested loop,
        // by maximizing optimization We want to keep the use of
        // ANGLE_D3D_WORKAROUND_MAX_OPTIMIZATION minimal to prevent hangs, so usesDiscard takes
        // precedence
        workarounds->useMaxOptimization = true;
    }

    if (requiresIEEEStrictCompiling)
    {
        // IEEE Strictness for D3D compiler needs to be enabled for NaNs to work.
        workarounds->enableIEEEStrictness = true;
    }
}

unsigned int CompiledShaderStateD3D::getUniformRegister(const std::string &uniformName) const
{
    ASSERT(uniformRegisterMap.count(uniformName) > 0);
    return uniformRegisterMap.find(uniformName)->second;
}

unsigned int CompiledShaderStateD3D::getUniformBlockRegister(const std::string &blockName) const
{
    ASSERT(uniformBlockRegisterMap.count(blockName) > 0);
    return uniformBlockRegisterMap.find(blockName)->second;
}

bool CompiledShaderStateD3D::shouldUniformBlockUseStructuredBuffer(
    const std::string &blockName) const
{
    ASSERT(uniformBlockUseStructuredBufferMap.count(blockName) > 0);
    return uniformBlockUseStructuredBufferMap.find(blockName)->second;
}

unsigned int CompiledShaderStateD3D::getShaderStorageBlockRegister(
    const std::string &blockName) const
{
    ASSERT(shaderStorageBlockRegisterMap.count(blockName) > 0);
    return shaderStorageBlockRegisterMap.find(blockName)->second;
}

bool CompiledShaderStateD3D::useImage2DFunction(const std::string &functionName) const
{
    if (usedImage2DFunctionNames.empty())
    {
        return false;
    }

    return usedImage2DFunctionNames.find(functionName) != usedImage2DFunctionNames.end();
}

const std::set<std::string> &CompiledShaderStateD3D::getSlowCompilingUniformBlockSet() const
{
    return slowCompilingUniformBlockSet;
}

std::shared_ptr<ShaderTranslateTask> ShaderD3D::compile(const gl::Context *context,
                                                        ShCompileOptions *options)
{
    // Create a new compiled shader state.  Currently running program link jobs will use the
    // previous state.
    mCompiledState = std::make_shared<CompiledShaderStateD3D>();

    std::string sourcePath;

    const angle::FeaturesD3D &features = mRenderer->getFeatures();
    const gl::Extensions &extensions   = mRenderer->getNativeExtensions();

    const std::string &source = mState.getSource();

#if !defined(ANGLE_ENABLE_WINDOWS_UWP)
    if (gl::DebugAnnotationsActive(context))
    {
        sourcePath = angle::CreateTemporaryFile().value();
        writeFile(sourcePath.c_str(), source.c_str(), source.length());
        options->lineDirectives = true;
        options->sourcePath     = true;
    }
#endif

    if (features.expandIntegerPowExpressions.enabled)
    {
        options->expandSelectHLSLIntegerPowExpressions = true;
    }

    if (features.getDimensionsIgnoresBaseLevel.enabled)
    {
        options->HLSLGetDimensionsIgnoresBaseLevel = true;
    }

    if (features.preAddTexelFetchOffsets.enabled)
    {
        options->rewriteTexelFetchOffsetToTexelFetch = true;
    }
    if (features.rewriteUnaryMinusOperator.enabled)
    {
        options->rewriteIntegerUnaryMinusOperator = true;
    }
    if (features.emulateIsnanFloat.enabled)
    {
        options->emulateIsnanFloatFunction = true;
    }
    if (features.skipVSConstantRegisterZero.enabled &&
        mState.getShaderType() == gl::ShaderType::Vertex)
    {
        options->skipD3DConstantRegisterZero = true;
    }
    if (features.forceAtomicValueResolution.enabled)
    {
        options->forceAtomicValueResolution = true;
    }
    if (features.allowTranslateUniformBlockToStructuredBuffer.enabled)
    {
        options->allowTranslateUniformBlockToStructuredBuffer = true;
    }
    if (extensions.multiviewOVR || extensions.multiview2OVR)
    {
        options->initializeBuiltinsForInstancedMultiview = true;
    }
    if (extensions.shaderPixelLocalStorageANGLE)
    {
        options->pls = mRenderer->getNativePixelLocalStorageOptions();
    }

    // D3D11 Feature Level 9_3 and below do not support non-constant loop indexes in fragment
    // shaders.  Shader compilation will fail.  To provide a better error message we can instruct
    // the compiler to pre-validate.
    if (!features.supportsNonConstantLoopIndexing.enabled)
    {
        options->validateLoopIndexing = true;
    }

    // The D3D translations are not currently validation-error-free
    options->validateAST = false;

    return std::shared_ptr<ShaderTranslateTask>(
        new ShaderTranslateTaskD3D(mCompiledState, std::move(sourcePath)));
}

std::shared_ptr<ShaderTranslateTask> ShaderD3D::load(const gl::Context *context,
                                                     gl::BinaryInputStream *stream)
{
    UNREACHABLE();
    return std::shared_ptr<ShaderTranslateTask>(new ShaderTranslateTask);
}

bool CompiledShaderStateD3D::hasUniform(const std::string &name) const
{
    return uniformRegisterMap.find(name) != uniformRegisterMap.end();
}

}  // namespace rx
