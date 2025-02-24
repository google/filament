//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramMtl.mm:
//    Implements the class methods for ProgramMtl.
//

#include "libANGLE/renderer/metal/ProgramMtl.h"

#include <TargetConditionals.h>

#include <sstream>

#include "common/WorkerThread.h"
#include "common/debug.h"
#include "common/system_utils.h"

#include "libANGLE/Context.h"
#include "libANGLE/ProgramLinkedResources.h"
#include "libANGLE/renderer/metal/CompilerMtl.h"
#include "libANGLE/renderer/metal/ContextMtl.h"
#include "libANGLE/renderer/metal/DisplayMtl.h"
#include "libANGLE/renderer/metal/blocklayoutMetal.h"
#include "libANGLE/renderer/metal/mtl_msl_utils.h"
#include "libANGLE/renderer/metal/mtl_utils.h"
#include "libANGLE/renderer/metal/renderermtl_utils.h"
#include "libANGLE/renderer/renderer_utils.h"
#include "libANGLE/trace.h"

namespace rx
{

namespace
{
inline std::map<std::string, std::string> GetDefaultSubstitutionDictionary()
{
    return {};
}

class Std140BlockLayoutEncoderFactory : public gl::CustomBlockLayoutEncoderFactory
{
  public:
    sh::BlockLayoutEncoder *makeEncoder() override { return new sh::Std140BlockEncoder(); }
};

class CompileMslTask final : public LinkSubTask, public mtl::Context
{
  public:
    CompileMslTask(DisplayMtl *displayMtl,
                   mtl::TranslatedShaderInfo *translatedMslInfo,
                   const std::map<std::string, std::string> &substitutionMacros)
        : mtl::Context(displayMtl),
          mTranslatedMslInfo(translatedMslInfo),
          mSubstitutionMacros(substitutionMacros)
    {}
    ~CompileMslTask() override = default;

    void operator()() override
    {
        mResult = CreateMslShaderLib(this, mInfoLog, mTranslatedMslInfo, mSubstitutionMacros);
    }

    angle::Result getResult(const gl::Context *context, gl::InfoLog &infoLog) override
    {
        if (!mInfoLog.empty())
        {
            infoLog << mInfoLog.str();
        }
        if (mErrorCode != GL_NO_ERROR)
        {
            mtl::GetImpl(context)->handleError(mErrorCode, mErrorMessage.c_str(), mErrorFile,
                                               mErrorFunction, mErrorLine);
            return angle::Result::Stop;
        }
        return mResult;
    }

    // override mtl::ErrorHandler
    void handleError(GLenum glErrorCode,
                     const char *message,
                     const char *file,
                     const char *function,
                     unsigned int line) override
    {
        mErrorCode     = glErrorCode;
        mErrorMessage  = message;
        mErrorFile     = file;
        mErrorFunction = function;
        mErrorLine     = line;
    }

  private:
    mtl::TranslatedShaderInfo *mTranslatedMslInfo;
    std::map<std::string, std::string> mSubstitutionMacros;
    angle::Result mResult = angle::Result::Continue;
    gl::InfoLog mInfoLog;
    GLenum mErrorCode = GL_NO_ERROR;
    std::string mErrorMessage;
    const char *mErrorFile     = nullptr;
    const char *mErrorFunction = nullptr;
    unsigned int mErrorLine    = 0;
};

}  // namespace

class ProgramMtl::LinkTaskMtl final : public LinkTask, public mtl::Context
{
  public:
    LinkTaskMtl(DisplayMtl *displayMtl, ProgramMtl *program)
        : mtl::Context(displayMtl), mProgram(program)
    {}
    ~LinkTaskMtl() override = default;

    void link(const gl::ProgramLinkedResources &resources,
              const gl::ProgramMergedVaryings &mergedVaryings,
              std::vector<std::shared_ptr<LinkSubTask>> *linkSubTasksOut,
              std::vector<std::shared_ptr<LinkSubTask>> *postLinkSubTasksOut) override
    {
        ASSERT(linkSubTasksOut && linkSubTasksOut->empty());
        ASSERT(postLinkSubTasksOut && postLinkSubTasksOut->empty());

        mResult = mProgram->linkJobImpl(this, resources, linkSubTasksOut);
        return;
    }

    angle::Result getResult(const gl::Context *context, gl::InfoLog &infoLog) override
    {
        if (mErrorCode != GL_NO_ERROR)
        {
            mtl::GetImpl(context)->handleError(mErrorCode, mErrorMessage.c_str(), mErrorFile,
                                               mErrorFunction, mErrorLine);
            return angle::Result::Stop;
        }

        return mResult;
    }

    // override mtl::ErrorHandler
    void handleError(GLenum glErrorCode,
                     const char *message,
                     const char *file,
                     const char *function,
                     unsigned int line) override
    {
        mErrorCode     = glErrorCode;
        mErrorMessage  = message;
        mErrorFile     = file;
        mErrorFunction = function;
        mErrorLine     = line;
    }

  private:
    ProgramMtl *mProgram;
    angle::Result mResult = angle::Result::Continue;

    GLenum mErrorCode          = GL_NO_ERROR;
    std::string mErrorMessage;
    const char *mErrorFile     = nullptr;
    const char *mErrorFunction = nullptr;
    unsigned int mErrorLine    = 0;
};

class ProgramMtl::LoadTaskMtl final : public LinkTask
{
  public:
    LoadTaskMtl(std::vector<std::shared_ptr<LinkSubTask>> &&subTasks)
        : mSubTasks(std::move(subTasks))
    {}
    ~LoadTaskMtl() override = default;

    void load(std::vector<std::shared_ptr<LinkSubTask>> *linkSubTasksOut,
              std::vector<std::shared_ptr<LinkSubTask>> *postLinkSubTasksOut) override
    {
        ASSERT(linkSubTasksOut && linkSubTasksOut->empty());
        ASSERT(postLinkSubTasksOut && postLinkSubTasksOut->empty());

        *linkSubTasksOut = mSubTasks;
        return;
    }

    angle::Result getResult(const gl::Context *context, gl::InfoLog &infoLog) override
    {
        return angle::Result::Continue;
    }

  private:
    std::vector<std::shared_ptr<LinkSubTask>> mSubTasks;
};

// ProgramArgumentBufferEncoderMtl implementation
void ProgramArgumentBufferEncoderMtl::reset(ContextMtl *contextMtl)
{
    metalArgBufferEncoder = nil;
    bufferPool.destroy(contextMtl);
}

// ProgramShaderObjVariantMtl implementation
void ProgramShaderObjVariantMtl::reset(ContextMtl *contextMtl)
{
    metalShader = nil;

    uboArgBufferEncoder.reset(contextMtl);

    translatedSrcInfo = nullptr;
}

// ProgramMtl implementation
ProgramMtl::ProgramMtl(const gl::ProgramState &state) : ProgramImpl(state) {}

ProgramMtl::~ProgramMtl() = default;

void ProgramMtl::destroy(const gl::Context *context)
{
    getExecutable()->reset(mtl::GetImpl(context));
}

angle::Result ProgramMtl::load(const gl::Context *context,
                               gl::BinaryInputStream *stream,
                               std::shared_ptr<LinkTask> *loadTaskOut,
                               egl::CacheGetResult *resultOut)
{

    ContextMtl *contextMtl = mtl::GetImpl(context);
    // NOTE(hqle): No transform feedbacks for now, since we only support ES 2.0 atm

    ANGLE_TRY(getExecutable()->load(contextMtl, stream));

    // TODO: parallelize the above too.  http://anglebug.com/41488637
    std::vector<std::shared_ptr<LinkSubTask>> subTasks;

    ANGLE_TRY(compileMslShaderLibs(contextMtl, &subTasks));

    *loadTaskOut = std::shared_ptr<LinkTask>(new LoadTaskMtl(std::move(subTasks)));
    *resultOut   = egl::CacheGetResult::Success;

    return angle::Result::Continue;
}

void ProgramMtl::save(const gl::Context *context, gl::BinaryOutputStream *stream)
{
    getExecutable()->save(stream);
}

void ProgramMtl::setBinaryRetrievableHint(bool retrievable) {}

void ProgramMtl::setSeparable(bool separable)
{
    UNIMPLEMENTED();
}

void ProgramMtl::prepareForLink(const gl::ShaderMap<ShaderImpl *> &shaders)
{
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        mAttachedShaders[shaderType].reset();

        if (shaders[shaderType] != nullptr)
        {
            const ShaderMtl *shaderMtl   = GetAs<ShaderMtl>(shaders[shaderType]);
            mAttachedShaders[shaderType] = shaderMtl->getCompiledState();
        }
    }
}

angle::Result ProgramMtl::link(const gl::Context *context, std::shared_ptr<LinkTask> *linkTaskOut)
{
    DisplayMtl *displayMtl = mtl::GetImpl(context)->getDisplay();

    *linkTaskOut = std::shared_ptr<LinkTask>(new LinkTaskMtl(displayMtl, this));
    return angle::Result::Continue;
}

angle::Result ProgramMtl::linkJobImpl(mtl::Context *context,
                                      const gl::ProgramLinkedResources &resources,
                                      std::vector<std::shared_ptr<LinkSubTask>> *subTasksOut)
{
    ProgramExecutableMtl *executableMtl = getExecutable();

    // Link resources before calling GetShaderSource to make sure they are ready for the set/binding
    // assignment done in that function.
    linkResources(resources);

    ANGLE_TRY(executableMtl->initDefaultUniformBlocks(context, mState.getAttachedShaders()));
    executableMtl->linkUpdateHasFlatAttributes(mState.getAttachedShader(gl::ShaderType::Vertex));

    gl::ShaderMap<std::string> shaderSources;
    mtl::MSLGetShaderSource(mState, resources, &shaderSources);

    ANGLE_TRY(mtl::MTLGetMSL(context->getDisplay()->getFeatures(), mState.getExecutable(),
                             shaderSources, mAttachedShaders,
                             &executableMtl->mMslShaderTranslateInfo));
    executableMtl->mMslXfbOnlyVertexShaderInfo =
        executableMtl->mMslShaderTranslateInfo[gl::ShaderType::Vertex];

    return compileMslShaderLibs(context, subTasksOut);
}

angle::Result ProgramMtl::compileMslShaderLibs(
    mtl::Context *context,
    std::vector<std::shared_ptr<LinkSubTask>> *subTasksOut)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "ProgramMtl::compileMslShaderLibs");
    gl::InfoLog &infoLog = mState.getExecutable().getInfoLog();

    DisplayMtl *displayMtl              = context->getDisplay();
    ProgramExecutableMtl *executableMtl = getExecutable();
    bool asyncCompile = displayMtl->getFeatures().enableParallelMtlLibraryCompilation.enabled;
    mtl::LibraryCache &libraryCache = displayMtl->getLibraryCache();

    for (gl::ShaderType shaderType : gl::kAllGLES2ShaderTypes)
    {
        mtl::TranslatedShaderInfo *translateInfo =
            &executableMtl->mMslShaderTranslateInfo[shaderType];
        std::map<std::string, std::string> macros = GetDefaultSubstitutionDictionary();
        const bool disableFastMath = displayMtl->getFeatures().intelDisableFastMath.enabled ||
                                     translateInfo->hasIsnanOrIsinf;
        const bool usesInvariance = translateInfo->hasInvariant;

        // Check if the shader is already in the cache and use it instead of spawning a new thread
        translateInfo->metalLibrary = libraryCache.get(translateInfo->metalShaderSource, macros,
                                                       disableFastMath, usesInvariance);

        if (!translateInfo->metalLibrary)
        {
            if (asyncCompile)
            {
                subTasksOut->emplace_back(new CompileMslTask(displayMtl, translateInfo, macros));
            }
            else
            {
                ANGLE_TRY(CreateMslShaderLib(context, infoLog, translateInfo, macros));
            }
        }
    }

    return angle::Result::Continue;
}

void ProgramMtl::linkResources(const gl::ProgramLinkedResources &resources)
{
    Std140BlockLayoutEncoderFactory std140EncoderFactory;
    gl::ProgramLinkedResourcesLinker linker(&std140EncoderFactory);

    linker.linkResources(mState, resources);
}

GLboolean ProgramMtl::validate(const gl::Caps &caps)
{
    // No-op. The spec is very vague about the behavior of validation.
    return GL_TRUE;
}

}  // namespace rx
