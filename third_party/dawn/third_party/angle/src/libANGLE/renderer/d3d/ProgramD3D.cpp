//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramD3D.cpp: Defines the rx::ProgramD3D class which implements rx::ProgramImpl.

#include "libANGLE/renderer/d3d/ProgramD3D.h"

#include "common/MemoryBuffer.h"
#include "common/bitset_utils.h"
#include "common/string_utils.h"
#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/Program.h"
#include "libANGLE/ProgramLinkedResources.h"
#include "libANGLE/Uniform.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/features.h"
#include "libANGLE/queryconversions.h"
#include "libANGLE/renderer/ContextImpl.h"
#include "libANGLE/renderer/d3d/ContextD3D.h"
#include "libANGLE/renderer/d3d/ShaderD3D.h"
#include "libANGLE/renderer/d3d/ShaderExecutableD3D.h"
#include "libANGLE/renderer/d3d/VertexDataManager.h"
#include "libANGLE/renderer/renderer_utils.h"
#include "libANGLE/trace.h"

using namespace angle;

namespace rx
{
namespace
{
bool HasFlatInterpolationVarying(const std::vector<sh::ShaderVariable> &varyings)
{
    // Note: this assumes nested structs can only be packed with one interpolation.
    for (const auto &varying : varyings)
    {
        if (varying.interpolation == sh::INTERPOLATION_FLAT)
        {
            return true;
        }
    }

    return false;
}

bool FindFlatInterpolationVaryingPerShader(const gl::SharedCompiledShaderState &shader)
{
    ASSERT(shader);
    switch (shader->shaderType)
    {
        case gl::ShaderType::Vertex:
            return HasFlatInterpolationVarying(shader->outputVaryings);
        case gl::ShaderType::Fragment:
            return HasFlatInterpolationVarying(shader->inputVaryings);
        case gl::ShaderType::Geometry:
            return HasFlatInterpolationVarying(shader->inputVaryings) ||
                   HasFlatInterpolationVarying(shader->outputVaryings);
        default:
            UNREACHABLE();
            return false;
    }
}

bool FindFlatInterpolationVarying(const gl::ShaderMap<gl::SharedCompiledShaderState> &shaders)
{
    for (gl::ShaderType shaderType : gl::kAllGraphicsShaderTypes)
    {
        const gl::SharedCompiledShaderState &shader = shaders[shaderType];
        if (!shader)
        {
            continue;
        }

        if (FindFlatInterpolationVaryingPerShader(shader))
        {
            return true;
        }
    }

    return false;
}

class HLSLBlockLayoutEncoderFactory : public gl::CustomBlockLayoutEncoderFactory
{
  public:
    sh::BlockLayoutEncoder *makeEncoder() override
    {
        return new sh::HLSLBlockEncoder(sh::HLSLBlockEncoder::ENCODE_PACKED, false);
    }
};

// GetExecutableTask class
class GetExecutableTask : public LinkSubTask, public d3d::Context
{
  public:
    GetExecutableTask(ProgramD3D *program, const SharedCompiledShaderStateD3D &shader)
        : mProgram(program), mExecutable(program->getExecutable()), mShader(shader)
    {}
    ~GetExecutableTask() override = default;

    angle::Result getResult(const gl::Context *context, gl::InfoLog &infoLog) override
    {
        ASSERT((mResult == angle::Result::Continue) == (mStoredHR == S_OK));

        ANGLE_TRY(checkTask(context, infoLog));

        // Append debug info
        if (mShader && mShaderExecutable != nullptr)
        {
            mShader->appendDebugInfo(mShaderExecutable->getDebugInfo());
        }

        return angle::Result::Continue;
    }

    void handleResult(HRESULT hr,
                      const char *message,
                      const char *file,
                      const char *function,
                      unsigned int line) override
    {
        mStoredHR       = hr;
        mStoredMessage  = message;
        mStoredFile     = file;
        mStoredFunction = function;
        mStoredLine     = line;
    }

  protected:
    void popError(d3d::Context *context)
    {
        ASSERT(mStoredFile);
        ASSERT(mStoredFunction);
        context->handleResult(mStoredHR, mStoredMessage.c_str(), mStoredFile, mStoredFunction,
                              mStoredLine);
    }

    angle::Result checkTask(const gl::Context *context, gl::InfoLog &infoLog)
    {
        // Forward any logs
        if (!mInfoLog.empty())
        {
            infoLog << mInfoLog.str();
        }

        // Forward any errors
        if (mResult != angle::Result::Continue)
        {
            ContextD3D *contextD3D = GetImplAs<ContextD3D>(context);
            popError(contextD3D);
            return angle::Result::Stop;
        }

        return angle::Result::Continue;
    }

    ProgramD3D *mProgram              = nullptr;
    ProgramExecutableD3D *mExecutable = nullptr;
    angle::Result mResult             = angle::Result::Continue;
    gl::InfoLog mInfoLog;
    ShaderExecutableD3D *mShaderExecutable = nullptr;
    SharedCompiledShaderStateD3D mShader;

    // Error handling
    HRESULT mStoredHR = S_OK;
    std::string mStoredMessage;
    const char *mStoredFile     = nullptr;
    const char *mStoredFunction = nullptr;
    unsigned int mStoredLine    = 0;
};
}  // anonymous namespace

// ProgramD3DMetadata Implementation
ProgramD3DMetadata::ProgramD3DMetadata(
    RendererD3D *renderer,
    const gl::SharedCompiledShaderState &fragmentShader,
    const gl::ShaderMap<SharedCompiledShaderStateD3D> &attachedShaders,
    int shaderVersion)
    : mRendererMajorShaderModel(renderer->getMajorShaderModel()),
      mShaderModelSuffix(renderer->getShaderModelSuffix()),
      mUsesViewScale(renderer->presentPathFastEnabled()),
      mCanSelectViewInVertexShader(renderer->canSelectViewInVertexShader()),
      mFragmentShader(fragmentShader),
      mAttachedShaders(attachedShaders),
      mShaderVersion(shaderVersion)
{}

ProgramD3DMetadata::~ProgramD3DMetadata() = default;

int ProgramD3DMetadata::getRendererMajorShaderModel() const
{
    return mRendererMajorShaderModel;
}

bool ProgramD3DMetadata::usesBroadcast(const gl::Version &clientVersion) const
{
    const SharedCompiledShaderStateD3D &shader = mAttachedShaders[gl::ShaderType::Fragment];
    return shader && shader->usesFragColor && shader->usesMultipleRenderTargets &&
           clientVersion.major < 3;
}

bool ProgramD3DMetadata::usesSecondaryColor() const
{
    const SharedCompiledShaderStateD3D &shader = mAttachedShaders[gl::ShaderType::Fragment];
    return shader && shader->usesSecondaryColor;
}

FragDepthUsage ProgramD3DMetadata::getFragDepthUsage() const
{
    const SharedCompiledShaderStateD3D &shader = mAttachedShaders[gl::ShaderType::Fragment];
    return shader ? shader->fragDepthUsage : FragDepthUsage::Unused;
}

bool ProgramD3DMetadata::usesPointCoord() const
{
    const SharedCompiledShaderStateD3D &shader = mAttachedShaders[gl::ShaderType::Fragment];
    return shader && shader->usesPointCoord;
}

bool ProgramD3DMetadata::usesFragCoord() const
{
    const SharedCompiledShaderStateD3D &shader = mAttachedShaders[gl::ShaderType::Fragment];
    return shader && shader->usesFragCoord;
}

bool ProgramD3DMetadata::usesPointSize() const
{
    const SharedCompiledShaderStateD3D &shader = mAttachedShaders[gl::ShaderType::Vertex];
    return shader && shader->usesPointSize;
}

bool ProgramD3DMetadata::usesInsertedPointCoordValue() const
{
    return usesPointCoord() && mRendererMajorShaderModel >= 4;
}

bool ProgramD3DMetadata::usesViewScale() const
{
    return mUsesViewScale;
}

bool ProgramD3DMetadata::hasMultiviewEnabled() const
{
    const SharedCompiledShaderStateD3D &shader = mAttachedShaders[gl::ShaderType::Vertex];
    return shader && shader->hasMultiviewEnabled;
}

bool ProgramD3DMetadata::usesVertexID() const
{
    const SharedCompiledShaderStateD3D &shader = mAttachedShaders[gl::ShaderType::Vertex];
    return shader && shader->usesVertexID;
}

bool ProgramD3DMetadata::usesViewID() const
{
    const SharedCompiledShaderStateD3D &shader = mAttachedShaders[gl::ShaderType::Fragment];
    return shader && shader->usesViewID;
}

bool ProgramD3DMetadata::canSelectViewInVertexShader() const
{
    return mCanSelectViewInVertexShader;
}

bool ProgramD3DMetadata::addsPointCoordToVertexShader() const
{
    // With a geometry shader, the app can render triangles or lines and reference
    // gl_PointCoord in the fragment shader, requiring us to provide a placeholder value. For
    // simplicity, we always add this to the vertex shader when the fragment shader
    // references gl_PointCoord, even if we could skip it in the geometry shader.
    return usesInsertedPointCoordValue();
}

bool ProgramD3DMetadata::usesTransformFeedbackGLPosition() const
{
    // gl_Position only needs to be outputted from the vertex shader if transform feedback is
    // active. This isn't supported on D3D11 Feature Level 9_3, so we don't output gl_Position from
    // the vertex shader in this case. This saves us 1 output vector.
    return !(mRendererMajorShaderModel >= 4 && mShaderModelSuffix != "");
}

bool ProgramD3DMetadata::usesSystemValuePointSize() const
{
    return usesPointSize();
}

bool ProgramD3DMetadata::usesMultipleFragmentOuts() const
{
    const SharedCompiledShaderStateD3D &shader = mAttachedShaders[gl::ShaderType::Fragment];
    return shader && shader->usesMultipleRenderTargets;
}

bool ProgramD3DMetadata::usesCustomOutVars() const
{
    return mShaderVersion >= 300;
}

bool ProgramD3DMetadata::usesSampleMask() const
{
    const SharedCompiledShaderStateD3D &shader = mAttachedShaders[gl::ShaderType::Fragment];
    return shader && shader->usesSampleMask;
}

const gl::SharedCompiledShaderState &ProgramD3DMetadata::getFragmentShader() const
{
    return mFragmentShader;
}

uint8_t ProgramD3DMetadata::getClipDistanceArraySize() const
{
    const SharedCompiledShaderStateD3D &shader = mAttachedShaders[gl::ShaderType::Vertex];
    return shader ? shader->clipDistanceSize : 0;
}

uint8_t ProgramD3DMetadata::getCullDistanceArraySize() const
{
    const SharedCompiledShaderStateD3D &shader = mAttachedShaders[gl::ShaderType::Vertex];
    return shader ? shader->cullDistanceSize : 0;
}

// ProgramD3D Implementation

class ProgramD3D::GetVertexExecutableTask : public GetExecutableTask
{
  public:
    GetVertexExecutableTask(ProgramD3D *program, const SharedCompiledShaderStateD3D &shader)
        : GetExecutableTask(program, shader)
    {}
    ~GetVertexExecutableTask() override = default;

    void operator()() override
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "GetVertexExecutableTask::run");

        mExecutable->updateCachedImage2DBindLayoutFromShader(gl::ShaderType::Vertex);
        mResult = mExecutable->getVertexExecutableForCachedInputLayout(
            this, mProgram->mRenderer, &mShaderExecutable, &mInfoLog);
    }
};

class ProgramD3D::GetPixelExecutableTask : public GetExecutableTask
{
  public:
    GetPixelExecutableTask(ProgramD3D *program, const SharedCompiledShaderStateD3D &shader)
        : GetExecutableTask(program, shader)
    {}
    ~GetPixelExecutableTask() override = default;

    void operator()() override
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "GetPixelExecutableTask::run");
        if (!mShader)
        {
            return;
        }

        mExecutable->updateCachedOutputLayoutFromShader();
        mExecutable->updateCachedImage2DBindLayoutFromShader(gl::ShaderType::Fragment);
        mResult = mExecutable->getPixelExecutableForCachedOutputLayout(
            this, mProgram->mRenderer, &mShaderExecutable, &mInfoLog);
    }
};

class ProgramD3D::GetGeometryExecutableTask : public GetExecutableTask
{
  public:
    GetGeometryExecutableTask(ProgramD3D *program,
                              const SharedCompiledShaderStateD3D &shader,
                              const gl::Caps &caps,
                              gl::ProvokingVertexConvention provokingVertex)
        : GetExecutableTask(program, shader), mCaps(caps), mProvokingVertex(provokingVertex)
    {}
    ~GetGeometryExecutableTask() override = default;

    void operator()() override
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "GetGeometryExecutableTask::run");
        // Auto-generate the geometry shader here, if we expect to be using point rendering in
        // D3D11.
        if (mExecutable->usesGeometryShader(mProgram->mRenderer, mProvokingVertex,
                                            gl::PrimitiveMode::Points))
        {
            mResult = mExecutable->getGeometryExecutableForPrimitiveType(
                this, mProgram->mRenderer, mCaps, mProvokingVertex, gl::PrimitiveMode::Points,
                &mShaderExecutable, &mInfoLog);
        }
    }

  private:
    const gl::Caps &mCaps;
    gl::ProvokingVertexConvention mProvokingVertex;
};

class ProgramD3D::GetComputeExecutableTask : public GetExecutableTask
{
  public:
    GetComputeExecutableTask(ProgramD3D *program, const SharedCompiledShaderStateD3D &shader)
        : GetExecutableTask(program, shader)
    {}
    ~GetComputeExecutableTask() override = default;

    void operator()() override
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "GetComputeExecutableTask::run");

        mExecutable->updateCachedImage2DBindLayoutFromShader(gl::ShaderType::Compute);
        mResult = mExecutable->getComputeExecutableForImage2DBindLayout(
            this, mProgram->mRenderer, &mShaderExecutable, &mInfoLog);
    }
};

class ProgramD3D::LinkLoadTaskD3D : public d3d::Context, public LinkTask
{
  public:
    LinkLoadTaskD3D(ProgramD3D *program) : mProgram(program), mExecutable(program->getExecutable())
    {
        ASSERT(mProgram);
    }
    ~LinkLoadTaskD3D() override = default;

    angle::Result getResult(const gl::Context *context, gl::InfoLog &infoLog) override
    {
        if (mStoredHR != S_OK)
        {
            GetImplAs<ContextD3D>(context)->handleResult(mStoredHR, mStoredMessage.c_str(),
                                                         mStoredFile, mStoredFunction, mStoredLine);
            return angle::Result::Stop;
        }

        return angle::Result::Continue;
    }

    void handleResult(HRESULT hr,
                      const char *message,
                      const char *file,
                      const char *function,
                      unsigned int line) override
    {
        mStoredHR       = hr;
        mStoredMessage  = message;
        mStoredFile     = file;
        mStoredFunction = function;
        mStoredLine     = line;
    }

  protected:
    // The front-end ensures that the program is not accessed while linking, so it is safe to
    // direclty access the state from a potentially parallel job.
    ProgramD3D *mProgram;
    ProgramExecutableD3D *mExecutable = nullptr;

    // Error handling
    HRESULT mStoredHR = S_OK;
    std::string mStoredMessage;
    const char *mStoredFile     = nullptr;
    const char *mStoredFunction = nullptr;
    unsigned int mStoredLine    = 0;
};

class ProgramD3D::LinkTaskD3D final : public LinkLoadTaskD3D
{
  public:
    LinkTaskD3D(const gl::Version &clientVersion,
                const gl::Caps &caps,
                ProgramD3D *program,
                gl::ProvokingVertexConvention provokingVertex)
        : LinkLoadTaskD3D(program),
          mClientVersion(clientVersion),
          mCaps(caps),
          mProvokingVertex(provokingVertex)
    {}
    ~LinkTaskD3D() override = default;

    void link(const gl::ProgramLinkedResources &resources,
              const gl::ProgramMergedVaryings &mergedVaryings,
              std::vector<std::shared_ptr<LinkSubTask>> *linkSubTasksOut,
              std::vector<std::shared_ptr<LinkSubTask>> *postLinkSubTasksOut) override;

  private:
    const gl::Version mClientVersion;
    const gl::Caps &mCaps;
    const gl::ProvokingVertexConvention mProvokingVertex;
};

void ProgramD3D::LinkTaskD3D::link(const gl::ProgramLinkedResources &resources,
                                   const gl::ProgramMergedVaryings &mergedVaryings,
                                   std::vector<std::shared_ptr<LinkSubTask>> *linkSubTasksOut,
                                   std::vector<std::shared_ptr<LinkSubTask>> *postLinkSubTasksOut)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "LinkTaskD3D::link");

    ASSERT(linkSubTasksOut && linkSubTasksOut->empty());
    ASSERT(postLinkSubTasksOut && postLinkSubTasksOut->empty());

    angle::Result result =
        mProgram->linkJobImpl(this, mCaps, mClientVersion, resources, mergedVaryings);
    ASSERT((result == angle::Result::Continue) == (mStoredHR == S_OK));

    if (result != angle::Result::Continue)
    {
        return;
    }

    // Create the subtasks
    if (mExecutable->hasShaderStage(gl::ShaderType::Compute))
    {
        linkSubTasksOut->push_back(std::make_shared<GetComputeExecutableTask>(
            mProgram, mProgram->getAttachedShader(gl::ShaderType::Compute)));
    }
    else
    {
        // Geometry shaders are currently only used internally, so there is no corresponding shader
        // object at the interface level. For now the geometry shader debug info is prepended to the
        // vertex shader.
        linkSubTasksOut->push_back(std::make_shared<GetVertexExecutableTask>(
            mProgram, mProgram->getAttachedShader(gl::ShaderType::Vertex)));
        linkSubTasksOut->push_back(std::make_shared<GetPixelExecutableTask>(
            mProgram, mProgram->getAttachedShader(gl::ShaderType::Fragment)));
        linkSubTasksOut->push_back(std::make_shared<GetGeometryExecutableTask>(
            mProgram, mProgram->getAttachedShader(gl::ShaderType::Vertex), mCaps,
            mProvokingVertex));
    }
}

class ProgramD3D::LoadTaskD3D final : public LinkLoadTaskD3D
{
  public:
    LoadTaskD3D(ProgramD3D *program, angle::MemoryBuffer &&streamData)
        : LinkLoadTaskD3D(program), mStreamData(std::move(streamData))
    {}
    ~LoadTaskD3D() override = default;

    void load(std::vector<std::shared_ptr<LinkSubTask>> *linkSubTasksOut,
              std::vector<std::shared_ptr<LinkSubTask>> *postLinkSubTasksOut) override
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "LoadTaskD3D::load");

        ASSERT(linkSubTasksOut && linkSubTasksOut->empty());
        ASSERT(postLinkSubTasksOut && postLinkSubTasksOut->empty());

        gl::BinaryInputStream stream(mStreamData.data(), mStreamData.size());
        mResult = mExecutable->loadBinaryShaderExecutables(this, mProgram->mRenderer, &stream);

        return;
    }

    angle::Result getResult(const gl::Context *context, gl::InfoLog &infoLog) override
    {
        ANGLE_TRY(LinkLoadTaskD3D::getResult(context, infoLog));
        return mResult;
    }

  private:
    angle::MemoryBuffer mStreamData;
    angle::Result mResult = angle::Result::Continue;
};

ProgramD3D::ProgramD3D(const gl::ProgramState &state, RendererD3D *renderer)
    : ProgramImpl(state), mRenderer(renderer)
{}

ProgramD3D::~ProgramD3D() = default;

void ProgramD3D::destroy(const gl::Context *context)
{
    getExecutable()->reset();
}

angle::Result ProgramD3D::load(const gl::Context *context,
                               gl::BinaryInputStream *stream,
                               std::shared_ptr<LinkTask> *loadTaskOut,
                               egl::CacheGetResult *resultOut)
{
    if (!getExecutable()->load(context, mRenderer, stream))
    {
        mState.getExecutable().getInfoLog()
            << "Invalid program binary, device configuration has changed.";
        return angle::Result::Continue;
    }

    // Copy the remaining data from the stream locally so that the client can't modify it when
    // loading off thread.
    angle::MemoryBuffer streamData;
    const size_t dataSize = stream->remainingSize();
    if (!streamData.resize(dataSize))
    {
        mState.getExecutable().getInfoLog()
            << "Failed to copy program binary data to local buffer.";
        return angle::Result::Stop;
    }
    memcpy(streamData.data(), stream->data() + stream->offset(), dataSize);

    // Note: pretty much all the above can also be moved to the task
    *loadTaskOut = std::shared_ptr<LinkTask>(new LoadTaskD3D(this, std::move(streamData)));
    *resultOut   = egl::CacheGetResult::Success;

    return angle::Result::Continue;
}

void ProgramD3D::save(const gl::Context *context, gl::BinaryOutputStream *stream)
{
    getExecutable()->save(context, mRenderer, stream);
}

void ProgramD3D::setBinaryRetrievableHint(bool /* retrievable */) {}

void ProgramD3D::setSeparable(bool /* separable */) {}

void ProgramD3D::prepareForLink(const gl::ShaderMap<ShaderImpl *> &shaders)
{
    ProgramExecutableD3D *executableD3D = getExecutable();

    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        executableD3D->mAttachedShaders[shaderType].reset();

        if (shaders[shaderType] != nullptr)
        {
            const ShaderD3D *shaderD3D                  = GetAs<ShaderD3D>(shaders[shaderType]);
            executableD3D->mAttachedShaders[shaderType] = shaderD3D->getCompiledState();
        }
    }
}

angle::Result ProgramD3D::link(const gl::Context *context, std::shared_ptr<LinkTask> *linkTaskOut)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "ProgramD3D::link");
    const gl::Version &clientVersion = context->getClientVersion();
    const gl::Caps &caps             = context->getCaps();

    // Ensure the compiler is initialized to avoid race conditions.
    ANGLE_TRY(mRenderer->ensureHLSLCompilerInitialized(GetImplAs<ContextD3D>(context)));

    *linkTaskOut = std::shared_ptr<LinkTask>(
        new LinkTaskD3D(clientVersion, caps, this, context->getState().getProvokingVertex()));

    return angle::Result::Continue;
}

angle::Result ProgramD3D::linkJobImpl(d3d::Context *context,
                                      const gl::Caps &caps,
                                      const gl::Version &clientVersion,
                                      const gl::ProgramLinkedResources &resources,
                                      const gl::ProgramMergedVaryings &mergedVaryings)
{
    ProgramExecutableD3D *executableD3D = getExecutable();

    const gl::SharedCompiledShaderState &computeShader =
        mState.getAttachedShader(gl::ShaderType::Compute);
    if (computeShader)
    {
        const gl::SharedCompiledShaderState &shader =
            mState.getAttachedShader(gl::ShaderType::Compute);
        executableD3D->mShaderHLSL[gl::ShaderType::Compute] = shader->translatedSource;

        executableD3D->mShaderSamplers[gl::ShaderType::Compute].resize(
            caps.maxShaderTextureImageUnits[gl::ShaderType::Compute]);
        executableD3D->mImages[gl::ShaderType::Compute].resize(caps.maxImageUnits);
        executableD3D->mReadonlyImages[gl::ShaderType::Compute].resize(caps.maxImageUnits);

        executableD3D->mShaderUniformsDirty.set(gl::ShaderType::Compute);

        linkResources(resources);

        for (const sh::ShaderVariable &uniform : computeShader->uniforms)
        {
            if (gl::IsImageType(uniform.type) && gl::IsImage2DType(uniform.type))
            {
                executableD3D->mImage2DUniforms[gl::ShaderType::Compute].push_back(uniform);
            }
        }

        executableD3D->defineUniformsAndAssignRegisters(mRenderer, mState.getAttachedShaders());

        return angle::Result::Continue;
    }

    for (gl::ShaderType shaderType : gl::kAllGraphicsShaderTypes)
    {
        const gl::SharedCompiledShaderState &shader = mState.getAttachedShader(shaderType);
        if (shader)
        {
            executableD3D->mAttachedShaders[shaderType]->generateWorkarounds(
                &executableD3D->mShaderWorkarounds[shaderType]);
            executableD3D->mShaderSamplers[shaderType].resize(
                caps.maxShaderTextureImageUnits[shaderType]);
            executableD3D->mImages[shaderType].resize(caps.maxImageUnits);
            executableD3D->mReadonlyImages[shaderType].resize(caps.maxImageUnits);

            executableD3D->mShaderUniformsDirty.set(shaderType);

            for (const sh::ShaderVariable &uniform : shader->uniforms)
            {
                if (gl::IsImageType(uniform.type) && gl::IsImage2DType(uniform.type))
                {
                    executableD3D->mImage2DUniforms[shaderType].push_back(uniform);
                }
            }

            for (const std::string &slowBlock :
                 executableD3D->mAttachedShaders[shaderType]->slowCompilingUniformBlockSet)
            {
                WARN() << "Uniform block '" << slowBlock << "' will be slow to compile. "
                       << "See UniformBlockToStructuredBufferTranslation.md "
                       << "(https://shorturl.at/drFY7) for details.";
            }
        }
    }

    if (mRenderer->getNativeLimitations().noFrontFacingSupport)
    {
        const SharedCompiledShaderStateD3D &fragmentShader =
            executableD3D->mAttachedShaders[gl::ShaderType::Fragment];
        if (fragmentShader && fragmentShader->usesFrontFacing)
        {
            mState.getExecutable().getInfoLog()
                << "The current renderer doesn't support gl_FrontFacing";
            // Fail compilation
            ANGLE_CHECK_HR(context, false, "gl_FrontFacing not supported", E_NOTIMPL);
        }
    }

    const gl::VaryingPacking &varyingPacking =
        resources.varyingPacking.getOutputPacking(gl::ShaderType::Vertex);

    ProgramD3DMetadata metadata(mRenderer, mState.getAttachedShader(gl::ShaderType::Fragment),
                                executableD3D->mAttachedShaders,
                                mState.getAttachedShader(gl::ShaderType::Vertex)->shaderVersion);
    BuiltinVaryingsD3D builtins(metadata, varyingPacking);

    DynamicHLSL::GenerateShaderLinkHLSL(mRenderer, caps, mState.getAttachedShaders(),
                                        executableD3D->mAttachedShaders, metadata, varyingPacking,
                                        builtins, &executableD3D->mShaderHLSL);

    executableD3D->mUsesPointSize =
        executableD3D->mAttachedShaders[gl::ShaderType::Vertex] &&
        executableD3D->mAttachedShaders[gl::ShaderType::Vertex]->usesPointSize;
    DynamicHLSL::GetPixelShaderOutputKey(mRenderer, caps, clientVersion, mState.getExecutable(),
                                         metadata, &executableD3D->mPixelShaderKey);
    executableD3D->mFragDepthUsage      = metadata.getFragDepthUsage();
    executableD3D->mUsesSampleMask      = metadata.usesSampleMask();
    executableD3D->mUsesVertexID        = metadata.usesVertexID();
    executableD3D->mUsesViewID          = metadata.usesViewID();
    executableD3D->mHasMultiviewEnabled = metadata.hasMultiviewEnabled();

    // Cache if we use flat shading
    executableD3D->mUsesFlatInterpolation =
        FindFlatInterpolationVarying(mState.getAttachedShaders());

    if (mRenderer->getMajorShaderModel() >= 4)
    {
        executableD3D->mGeometryShaderPreamble = DynamicHLSL::GenerateGeometryShaderPreamble(
            mRenderer, varyingPacking, builtins, executableD3D->mHasMultiviewEnabled,
            metadata.canSelectViewInVertexShader());
    }

    executableD3D->initAttribLocationsToD3DSemantic(
        mState.getAttachedShader(gl::ShaderType::Vertex));

    executableD3D->defineUniformsAndAssignRegisters(mRenderer, mState.getAttachedShaders());

    executableD3D->gatherTransformFeedbackVaryings(mRenderer, varyingPacking,
                                                   mState.getTransformFeedbackVaryingNames(),
                                                   builtins[gl::ShaderType::Vertex]);

    linkResources(resources);

    if (mState.getAttachedShader(gl::ShaderType::Vertex))
    {
        executableD3D->updateCachedInputLayoutFromShader(
            mRenderer, mState.getAttachedShader(gl::ShaderType::Vertex));
    }

    return angle::Result::Continue;
}

GLboolean ProgramD3D::validate(const gl::Caps & /*caps*/)
{
    // TODO(jmadill): Do something useful here?
    return GL_TRUE;
}

void ProgramD3D::linkResources(const gl::ProgramLinkedResources &resources)
{
    HLSLBlockLayoutEncoderFactory hlslEncoderFactory;
    gl::ProgramLinkedResourcesLinker linker(&hlslEncoderFactory);

    linker.linkResources(mState, resources);

    ProgramExecutableD3D *executableD3D = getExecutable();

    executableD3D->initializeUniformBlocks();
    executableD3D->initializeShaderStorageBlocks(mState.getAttachedShaders());
}

}  // namespace rx
