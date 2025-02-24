//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Program.cpp: Implements the gl::Program class. Implements GL program objects
// and related functionality. [OpenGL ES 2.0.24] section 2.10.3 page 28.

#include "libANGLE/Program.h"

#include <algorithm>
#include <utility>

#include "common/angle_version_info.h"
#include "common/bitset_utils.h"
#include "common/debug.h"
#include "common/platform.h"
#include "common/platform_helpers.h"
#include "common/string_utils.h"
#include "common/utilities.h"
#include "compiler/translator/blocklayout.h"
#include "libANGLE/Context.h"
#include "libANGLE/ErrorStrings.h"
#include "libANGLE/MemoryProgramCache.h"
#include "libANGLE/ProgramLinkedResources.h"
#include "libANGLE/ResourceManager.h"
#include "libANGLE/Uniform.h"
#include "libANGLE/VaryingPacking.h"
#include "libANGLE/Version.h"
#include "libANGLE/capture/FrameCapture.h"
#include "libANGLE/features.h"
#include "libANGLE/histogram_macros.h"
#include "libANGLE/queryconversions.h"
#include "libANGLE/renderer/ContextImpl.h"
#include "libANGLE/renderer/GLImplFactory.h"
#include "libANGLE/renderer/ProgramImpl.h"
#include "libANGLE/trace.h"
#include "platform/PlatformMethods.h"
#include "platform/autogen/FrontendFeatures_autogen.h"

namespace gl
{

namespace
{
void InitUniformBlockLinker(const ProgramState &state, UniformBlockLinker *blockLinker)
{
    for (ShaderType shaderType : AllShaderTypes())
    {
        const SharedCompiledShaderState &shader = state.getAttachedShader(shaderType);
        if (shader)
        {
            blockLinker->addShaderBlocks(shaderType, &shader->uniformBlocks);
        }
    }
}

void InitShaderStorageBlockLinker(const ProgramState &state, ShaderStorageBlockLinker *blockLinker)
{
    for (ShaderType shaderType : AllShaderTypes())
    {
        const SharedCompiledShaderState &shader = state.getAttachedShader(shaderType);
        if (shader)
        {
            blockLinker->addShaderBlocks(shaderType, &shader->shaderStorageBlocks);
        }
    }
}

// Provides a mechanism to access the result of asynchronous linking.
class LinkEvent : angle::NonCopyable
{
  public:
    virtual ~LinkEvent() {}

    // Please be aware that these methods may be called under a gl::Context other
    // than the one where the LinkEvent was created.
    //
    // Waits until the linking is actually done. Returns true if the linking
    // succeeded, false otherwise.
    virtual angle::Result wait(const Context *context) = 0;
    // Peeks whether the linking is still ongoing.
    virtual bool isLinking() = 0;
};

// Wraps an already done linking.
class LinkEventDone final : public LinkEvent
{
  public:
    LinkEventDone(angle::Result result) : mResult(result) {}
    angle::Result wait(const Context *context) override { return mResult; }
    bool isLinking() override { return false; }

  private:
    angle::Result mResult;
};

void ScheduleSubTasks(const std::shared_ptr<angle::WorkerThreadPool> &workerThreadPool,
                      std::vector<std::shared_ptr<rx::LinkSubTask>> &tasks,
                      std::vector<std::shared_ptr<angle::WaitableEvent>> *eventsOut)
{
    eventsOut->reserve(tasks.size());
    for (const std::shared_ptr<rx::LinkSubTask> &subTask : tasks)
    {
        eventsOut->push_back(workerThreadPool->postWorkerTask(subTask));
    }
}
}  // anonymous namespace

const char *GetLinkMismatchErrorString(LinkMismatchError linkError)
{
    switch (linkError)
    {
        case LinkMismatchError::TYPE_MISMATCH:
            return "Type";
        case LinkMismatchError::ARRAYNESS_MISMATCH:
            return "Array-ness";
        case LinkMismatchError::ARRAY_SIZE_MISMATCH:
            return "Array size";
        case LinkMismatchError::PRECISION_MISMATCH:
            return "Precision";
        case LinkMismatchError::STRUCT_NAME_MISMATCH:
            return "Structure name";
        case LinkMismatchError::FIELD_NUMBER_MISMATCH:
            return "Field number";
        case LinkMismatchError::FIELD_NAME_MISMATCH:
            return "Field name";

        case LinkMismatchError::INTERPOLATION_TYPE_MISMATCH:
            return "Interpolation type";
        case LinkMismatchError::INVARIANCE_MISMATCH:
            return "Invariance";

        case LinkMismatchError::BINDING_MISMATCH:
            return "Binding layout qualifier";
        case LinkMismatchError::LOCATION_MISMATCH:
            return "Location layout qualifier";
        case LinkMismatchError::OFFSET_MISMATCH:
            return "Offset layout qualifier";
        case LinkMismatchError::INSTANCE_NAME_MISMATCH:
            return "Instance name qualifier";
        case LinkMismatchError::FORMAT_MISMATCH:
            return "Format qualifier";

        case LinkMismatchError::LAYOUT_QUALIFIER_MISMATCH:
            return "Layout qualifier";
        case LinkMismatchError::MATRIX_PACKING_MISMATCH:
            return "Matrix Packing";

        case LinkMismatchError::FIELD_LOCATION_MISMATCH:
            return "Field location";
        case LinkMismatchError::FIELD_STRUCT_NAME_MISMATCH:
            return "Field structure name";
        default:
            UNREACHABLE();
            return "";
    }
}

template <typename T>
void UpdateInterfaceVariable(std::vector<T> *block, const sh::ShaderVariable &var)
{
    if (!var.isStruct())
    {
        block->emplace_back(var);
        block->back().resetEffectiveLocation();
    }

    for (const sh::ShaderVariable &field : var.fields)
    {
        ASSERT(!var.name.empty() || var.isShaderIOBlock);

        // Shader I/O block naming is similar to UBOs and SSBOs:
        //
        //     in Block
        //     {
        //         type field;  // produces "field"
        //     };
        //
        //     in Block2
        //     {
        //         type field;  // produces "Block2.field"
        //     } block2;
        //
        const std::string &baseName = var.isShaderIOBlock ? var.structOrBlockName : var.name;
        const std::string prefix    = var.name.empty() ? "" : baseName + ".";

        if (!field.isStruct())
        {
            sh::ShaderVariable fieldCopy = field;
            fieldCopy.updateEffectiveLocation(var);
            fieldCopy.name = prefix + field.name;
            block->emplace_back(fieldCopy);
        }

        for (const sh::ShaderVariable &nested : field.fields)
        {
            sh::ShaderVariable nestedCopy = nested;
            nestedCopy.updateEffectiveLocation(field);
            nestedCopy.name = prefix + field.name + "." + nested.name;
            block->emplace_back(nestedCopy);
        }
    }
}

// Saves the linking context for later use in resolveLink().
struct Program::LinkingState
{
    LinkingVariables linkingVariables;
    ProgramLinkedResources resources;
    std::unique_ptr<LinkEvent> linkEvent;
    bool linkingFromBinary;
};

const char *const g_fakepath = "C:\\fakepath";

// InfoLog implementation.
InfoLog::InfoLog() : mLazyStream(nullptr) {}

InfoLog::~InfoLog() {}

size_t InfoLog::getLength() const
{
    if (!mLazyStream)
    {
        return 0;
    }

    const std::string &logString = mLazyStream->str();
    return logString.empty() ? 0 : logString.length() + 1;
}

void InfoLog::getLog(GLsizei bufSize, GLsizei *length, char *infoLog) const
{
    size_t index = 0;

    if (bufSize > 0)
    {
        const std::string logString(str());

        if (!logString.empty())
        {
            index = std::min(static_cast<size_t>(bufSize) - 1, logString.length());
            memcpy(infoLog, logString.c_str(), index);
        }

        infoLog[index] = '\0';
    }

    if (length)
    {
        *length = static_cast<GLsizei>(index);
    }
}

// append a sanitized message to the program info log.
// The D3D compiler includes a fake file path in some of the warning or error
// messages, so lets remove all occurrences of this fake file path from the log.
void InfoLog::appendSanitized(const char *message)
{
    ensureInitialized();

    std::string msg(message);

    size_t found;
    do
    {
        found = msg.find(g_fakepath);
        if (found != std::string::npos)
        {
            msg.erase(found, strlen(g_fakepath));
        }
    } while (found != std::string::npos);

    if (!msg.empty())
    {
        *mLazyStream << message << std::endl;
    }
}

void InfoLog::reset()
{
    if (mLazyStream)
    {
        mLazyStream.reset(nullptr);
    }
}

bool InfoLog::empty() const
{
    if (!mLazyStream)
    {
        return true;
    }

    return mLazyStream->rdbuf()->in_avail() == 0;
}

void LogLinkMismatch(InfoLog &infoLog,
                     const std::string &variableName,
                     const char *variableType,
                     LinkMismatchError linkError,
                     const std::string &mismatchedStructOrBlockFieldName,
                     ShaderType shaderType1,
                     ShaderType shaderType2)
{
    std::ostringstream stream;
    stream << GetLinkMismatchErrorString(linkError) << "s of " << variableType << " '"
           << variableName;

    if (!mismatchedStructOrBlockFieldName.empty())
    {
        stream << "' member '" << variableName << "." << mismatchedStructOrBlockFieldName;
    }

    stream << "' differ between " << GetShaderTypeString(shaderType1) << " and "
           << GetShaderTypeString(shaderType2) << " shaders.";

    infoLog << stream.str();
}

bool IsActiveInterfaceBlock(const sh::InterfaceBlock &interfaceBlock)
{
    // Only 'packed' blocks are allowed to be considered inactive.
    return interfaceBlock.active || interfaceBlock.layout != sh::BLOCKLAYOUT_PACKED;
}

// VariableLocation implementation.
VariableLocation::VariableLocation() : index(kUnused), arrayIndex(0), ignored(false) {}

VariableLocation::VariableLocation(unsigned int arrayIndexIn, unsigned int index)
    : index(index), ignored(false)
{
    ASSERT(arrayIndex != GL_INVALID_INDEX);
    SetBitField(arrayIndex, arrayIndexIn);
}

// ProgramBindings implementation.
ProgramBindings::ProgramBindings() {}

ProgramBindings::~ProgramBindings() {}

void ProgramBindings::bindLocation(GLuint index, const std::string &name)
{
    mBindings[name] = index;
}

int ProgramBindings::getBindingByName(const std::string &name) const
{
    auto iter = mBindings.find(name);
    return (iter != mBindings.end()) ? iter->second : -1;
}

template <typename T>
int ProgramBindings::getBinding(const T &variable) const
{
    return getBindingByName(variable.name);
}

ProgramBindings::const_iterator ProgramBindings::begin() const
{
    return mBindings.begin();
}

ProgramBindings::const_iterator ProgramBindings::end() const
{
    return mBindings.end();
}

std::map<std::string, GLuint> ProgramBindings::getStableIterationMap() const
{
    return std::map<std::string, GLuint>(mBindings.begin(), mBindings.end());
}

// ProgramAliasedBindings implementation.
ProgramAliasedBindings::ProgramAliasedBindings() {}

ProgramAliasedBindings::~ProgramAliasedBindings() {}

void ProgramAliasedBindings::bindLocation(GLuint index, const std::string &name)
{
    mBindings[name] = ProgramBinding(index);

    // EXT_blend_func_extended spec: "If it specifies the base name of an array,
    // it identifies the resources associated with the first element of the array."
    //
    // Normalize array bindings so that "name" and "name[0]" map to the same entry.
    // If this binding is of the form "name[0]", then mark the "name" binding as
    // aliased but do not update it yet in case "name" is not actually an array.
    size_t nameLengthWithoutArrayIndex;
    unsigned int arrayIndex = ParseArrayIndex(name, &nameLengthWithoutArrayIndex);
    if (arrayIndex == 0)
    {
        std::string baseName = name.substr(0u, nameLengthWithoutArrayIndex);
        auto iter            = mBindings.find(baseName);
        if (iter != mBindings.end())
        {
            iter->second.aliased = true;
        }
    }
}

int ProgramAliasedBindings::getBindingByName(const std::string &name) const
{
    auto iter = mBindings.find(name);
    return (iter != mBindings.end()) ? iter->second.location : -1;
}

int ProgramAliasedBindings::getBindingByLocation(GLuint location) const
{
    for (const auto &iter : mBindings)
    {
        if (iter.second.location == location)
        {
            return iter.second.location;
        }
    }
    return -1;
}

template <typename T>
int ProgramAliasedBindings::getBinding(const T &variable) const
{
    const std::string &name = variable.name;

    // Check with the normalized array name if applicable.
    if (variable.isArray())
    {
        size_t nameLengthWithoutArrayIndex;
        unsigned int arrayIndex = ParseArrayIndex(name, &nameLengthWithoutArrayIndex);
        if (arrayIndex == 0)
        {
            std::string baseName = name.substr(0u, nameLengthWithoutArrayIndex);
            auto iter            = mBindings.find(baseName);
            // If "name" exists and is not aliased, that means it was modified more
            // recently than its "name[0]" form and should be used instead of that.
            if (iter != mBindings.end() && !iter->second.aliased)
            {
                return iter->second.location;
            }
        }
        else if (arrayIndex == GL_INVALID_INDEX)
        {
            auto iter = mBindings.find(variable.name);
            // If "name" exists and is not aliased, that means it was modified more
            // recently than its "name[0]" form and should be used instead of that.
            if (iter != mBindings.end() && !iter->second.aliased)
            {
                return iter->second.location;
            }
            // The base name was aliased, so use the name with the array notation.
            return getBindingByName(name + "[0]");
        }
    }

    return getBindingByName(name);
}
template int ProgramAliasedBindings::getBinding<UsedUniform>(const UsedUniform &variable) const;
template int ProgramAliasedBindings::getBinding<ProgramOutput>(const ProgramOutput &variable) const;
template int ProgramAliasedBindings::getBinding<sh::ShaderVariable>(
    const sh::ShaderVariable &variable) const;

ProgramAliasedBindings::const_iterator ProgramAliasedBindings::begin() const
{
    return mBindings.begin();
}

ProgramAliasedBindings::const_iterator ProgramAliasedBindings::end() const
{
    return mBindings.end();
}

std::map<std::string, ProgramBinding> ProgramAliasedBindings::getStableIterationMap() const
{
    return std::map<std::string, ProgramBinding>(mBindings.begin(), mBindings.end());
}

// ProgramState implementation.
ProgramState::ProgramState(rx::GLImplFactory *factory)
    : mLabel(),
      mAttachedShaders{},
      mTransformFeedbackBufferMode(GL_INTERLEAVED_ATTRIBS),
      mBinaryRetrieveableHint(false),
      mSeparable(false),
      mExecutable(new ProgramExecutable(factory, &mInfoLog))
{}

ProgramState::~ProgramState()
{
    ASSERT(!hasAnyAttachedShader());
}

const std::string &ProgramState::getLabel()
{
    return mLabel;
}

SharedCompiledShaderState ProgramState::getAttachedShader(ShaderType shaderType) const
{
    ASSERT(shaderType != ShaderType::InvalidEnum);
    return mAttachedShaders[shaderType];
}

bool ProgramState::hasAnyAttachedShader() const
{
    for (const SharedCompiledShaderState &shader : mAttachedShaders)
    {
        if (shader)
        {
            return true;
        }
    }
    return false;
}

ShaderType ProgramState::getAttachedTransformFeedbackStage() const
{
    if (mAttachedShaders[ShaderType::Geometry])
    {
        return ShaderType::Geometry;
    }
    if (mAttachedShaders[ShaderType::TessEvaluation])
    {
        return ShaderType::TessEvaluation;
    }
    return ShaderType::Vertex;
}

// The common portion of parallel link and load jobs
class Program::MainLinkLoadTask : public angle::Closure
{
  public:
    MainLinkLoadTask(const std::shared_ptr<angle::WorkerThreadPool> &subTaskWorkerPool,
                     ProgramState *state,
                     std::shared_ptr<rx::LinkTask> &&linkTask)
        : mSubTaskWorkerPool(subTaskWorkerPool), mState(*state), mLinkTask(std::move(linkTask))
    {
        ASSERT(subTaskWorkerPool.get());
    }
    ~MainLinkLoadTask() override = default;

    angle::Result getResult(const Context *context)
    {
        InfoLog &infoLog = mState.getExecutable().getInfoLog();

        ANGLE_TRY(mResult);
        ANGLE_TRY(mLinkTask->getResult(context, infoLog));

        for (const std::shared_ptr<rx::LinkSubTask> &task : mSubTasks)
        {
            ANGLE_TRY(task->getResult(context, infoLog));
        }

        return angle::Result::Continue;
    }

    void waitSubTasks() { angle::WaitableEvent::WaitMany(&mSubTaskWaitableEvents); }

    bool areSubTasksLinking()
    {
        if (mLinkTask->isLinkingInternally())
        {
            return true;
        }
        return !angle::WaitableEvent::AllReady(&mSubTaskWaitableEvents);
    }

  protected:
    void scheduleSubTasks(std::vector<std::shared_ptr<rx::LinkSubTask>> &&linkSubTasks,
                          std::vector<std::shared_ptr<rx::LinkSubTask>> &&postLinkSubTasks)
    {
        // Only one of linkSubTasks or postLinkSubTasks should have tasks.  This is because
        // currently, there is no support for ordering them.
        ASSERT(linkSubTasks.empty() || postLinkSubTasks.empty());

        // Schedule link subtasks
        mSubTasks = std::move(linkSubTasks);
        ScheduleSubTasks(mSubTaskWorkerPool, mSubTasks, &mSubTaskWaitableEvents);

        // Schedule post-link subtasks
        mState.mExecutable->mPostLinkSubTasks = std::move(postLinkSubTasks);
        ScheduleSubTasks(mSubTaskWorkerPool, mState.mExecutable->mPostLinkSubTasks,
                         &mState.mExecutable->mPostLinkSubTaskWaitableEvents);

        // No further use for worker pool.  Release it earlier than the destructor (to avoid
        // situations such as http://anglebug.com/42267099)
        mSubTaskWorkerPool.reset();
    }

    std::shared_ptr<angle::WorkerThreadPool> mSubTaskWorkerPool;
    ProgramState &mState;
    std::shared_ptr<rx::LinkTask> mLinkTask;

    // Subtask and wait events
    std::vector<std::shared_ptr<rx::LinkSubTask>> mSubTasks;
    std::vector<std::shared_ptr<angle::WaitableEvent>> mSubTaskWaitableEvents;

    // The result of the front-end portion of the link.  The backend's result is retrieved via
    // mLinkTask->getResult().  The subtask results are retrieved via mSubTasks similarly.
    angle::Result mResult;
};

class Program::MainLinkTask final : public Program::MainLinkLoadTask
{
  public:
    MainLinkTask(const std::shared_ptr<angle::WorkerThreadPool> &subTaskWorkerPool,
                 const Caps &caps,
                 const Limitations &limitations,
                 const Version &clientVersion,
                 bool isWebGL,
                 Program *program,
                 ProgramState *state,
                 LinkingVariables *linkingVariables,
                 ProgramLinkedResources *resources,
                 std::shared_ptr<rx::LinkTask> &&linkTask)
        : MainLinkLoadTask(subTaskWorkerPool, state, std::move(linkTask)),
          mCaps(caps),
          mLimitations(limitations),
          mClientVersion(clientVersion),
          mIsWebGL(isWebGL),
          mProgram(program),
          mLinkingVariables(linkingVariables),
          mResources(resources)
    {}
    ~MainLinkTask() override = default;

    void operator()() override { mResult = linkImpl(); }

  private:
    angle::Result linkImpl();

    // State needed for link
    const Caps &mCaps;
    const Limitations &mLimitations;
    const Version mClientVersion;
    const bool mIsWebGL;
    Program *mProgram;
    LinkingVariables *mLinkingVariables;
    ProgramLinkedResources *mResources;
};

class Program::MainLoadTask final : public Program::MainLinkLoadTask
{
  public:
    MainLoadTask(const std::shared_ptr<angle::WorkerThreadPool> &subTaskWorkerPool,
                 Program *program,
                 ProgramState *state,
                 std::shared_ptr<rx::LinkTask> &&loadTask)
        : MainLinkLoadTask(subTaskWorkerPool, state, std::move(loadTask))
    {}
    ~MainLoadTask() override = default;

    void operator()() override { mResult = loadImpl(); }

  private:
    angle::Result loadImpl();
};

class Program::MainLinkLoadEvent final : public LinkEvent
{
  public:
    MainLinkLoadEvent(const std::shared_ptr<MainLinkLoadTask> &linkTask,
                      const std::shared_ptr<angle::WaitableEvent> &waitEvent)
        : mLinkTask(linkTask), mWaitableEvent(waitEvent)
    {}
    ~MainLinkLoadEvent() override {}

    angle::Result wait(const Context *context) override
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "Program::MainLinkLoadEvent::wait");

        mWaitableEvent->wait();
        mLinkTask->waitSubTasks();

        return mLinkTask->getResult(context);
    }
    bool isLinking() override
    {
        return !mWaitableEvent->isReady() || mLinkTask->areSubTasksLinking();
    }

  private:
    std::shared_ptr<MainLinkLoadTask> mLinkTask;
    std::shared_ptr<angle::WaitableEvent> mWaitableEvent;
};

angle::Result Program::MainLinkTask::linkImpl()
{
    ProgramMergedVaryings mergedVaryings;

    // Do the front-end portion of the link.
    ANGLE_TRY(mProgram->linkJobImpl(mCaps, mLimitations, mClientVersion, mIsWebGL,
                                    mLinkingVariables, mResources, &mergedVaryings));

    // Next, do the backend portion of the link.  If there are any subtasks to be scheduled, they
    // are collected now.
    std::vector<std::shared_ptr<rx::LinkSubTask>> linkSubTasks;
    std::vector<std::shared_ptr<rx::LinkSubTask>> postLinkSubTasks;
    mLinkTask->link(*mResources, mergedVaryings, &linkSubTasks, &postLinkSubTasks);

    // Must be after backend's link to avoid misleading the linker about input/output variables.
    mState.updateProgramInterfaceInputs();
    mState.updateProgramInterfaceOutputs();

    // Schedule the subtasks
    scheduleSubTasks(std::move(linkSubTasks), std::move(postLinkSubTasks));

    return angle::Result::Continue;
}

angle::Result Program::MainLoadTask::loadImpl()
{
    std::vector<std::shared_ptr<rx::LinkSubTask>> linkSubTasks;
    std::vector<std::shared_ptr<rx::LinkSubTask>> postLinkSubTasks;
    mLinkTask->load(&linkSubTasks, &postLinkSubTasks);

    // Schedule the subtasks
    scheduleSubTasks(std::move(linkSubTasks), std::move(postLinkSubTasks));

    return angle::Result::Continue;
}

Program::Program(rx::GLImplFactory *factory, ShaderProgramManager *manager, ShaderProgramID handle)
    : mSerial(factory->generateSerial()),
      mState(factory),
      mProgram(factory->createProgram(mState)),
      mValidated(false),
      mDeleteStatus(false),
      mIsBinaryCached(true),
      mLinked(false),
      mProgramHash{0},
      mRefCount(0),
      mResourceManager(manager),
      mHandle(handle),
      mAttachedShaders{}
{
    ASSERT(mProgram);

    unlink();
}

Program::~Program()
{
    ASSERT(!mProgram);
}

void Program::onDestroy(const Context *context)
{
    resolveLink(context);
    waitForPostLinkTasks(context);

    for (ShaderType shaderType : AllShaderTypes())
    {
        Shader *shader = getAttachedShader(shaderType);
        if (shader != nullptr)
        {
            shader->release(context);
        }
        mState.mShaderCompileJobs[shaderType].reset();
        mState.mAttachedShaders[shaderType].reset();
        mAttachedShaders[shaderType] = nullptr;
    }

    mProgram->destroy(context);
    UninstallExecutable(context, &mState.mExecutable);

    ASSERT(!mState.hasAnyAttachedShader());
    SafeDelete(mProgram);

    mBinary.clear();

    delete this;
}

ShaderProgramID Program::id() const
{
    return mHandle;
}

angle::Result Program::setLabel(const Context *context, const std::string &label)
{
    ASSERT(!mLinkingState);
    mState.mLabel = label;

    if (mProgram)
    {
        return mProgram->onLabelUpdate(context);
    }
    return angle::Result::Continue;
}

const std::string &Program::getLabel() const
{
    ASSERT(!mLinkingState);
    return mState.mLabel;
}

void Program::attachShader(const Context *context, Shader *shader)
{
    resolveLink(context);

    ShaderType shaderType = shader->getType();
    ASSERT(shaderType != ShaderType::InvalidEnum);

    shader->addRef();
    mAttachedShaders[shaderType] = shader;
}

void Program::detachShader(const Context *context, Shader *shader)
{
    resolveLink(context);

    ShaderType shaderType = shader->getType();
    ASSERT(shaderType != ShaderType::InvalidEnum);

    ASSERT(mAttachedShaders[shaderType] == shader);
    shader->release(context);
    mAttachedShaders[shaderType] = nullptr;
    mState.mShaderCompileJobs[shaderType].reset();
    mState.mAttachedShaders[shaderType].reset();
}

int Program::getAttachedShadersCount() const
{
    ASSERT(!mLinkingState);
    int numAttachedShaders = 0;
    for (const Shader *shader : mAttachedShaders)
    {
        if (shader != nullptr)
        {
            ++numAttachedShaders;
        }
    }

    return numAttachedShaders;
}

Shader *Program::getAttachedShader(ShaderType shaderType) const
{
    return mAttachedShaders[shaderType];
}

void Program::bindAttributeLocation(const Context *context, GLuint index, const char *name)
{
    ASSERT(!mLinkingState);
    mState.mAttributeBindings.bindLocation(index, name);
}

void Program::bindUniformLocation(const Context *context,
                                  UniformLocation location,
                                  const char *name)
{
    ASSERT(!mLinkingState);
    mState.mUniformLocationBindings.bindLocation(location.value, name);
}

void Program::bindFragmentOutputLocation(const Context *context, GLuint index, const char *name)
{
    ASSERT(!mLinkingState);
    mState.mFragmentOutputLocations.bindLocation(index, name);
}

void Program::bindFragmentOutputIndex(const Context *context, GLuint index, const char *name)
{
    ASSERT(!mLinkingState);
    mState.mFragmentOutputIndexes.bindLocation(index, name);
}

void Program::makeNewExecutable(const Context *context)
{
    ASSERT(!mLinkingState);
    waitForPostLinkTasks(context);

    // Unlink the program, but do not clear the validation-related caching yet, since we can still
    // use the previously linked program if linking the shaders fails.
    mLinked = false;

    mLinkingState = std::make_unique<LinkingState>();

    // By default, set the link event as failing.  If link succeeds, it will be replaced by the
    // appropriate event.
    mLinkingState->linkEvent = std::make_unique<LinkEventDone>(angle::Result::Stop);

    InstallExecutable(
        context,
        std::make_shared<ProgramExecutable>(context->getImplementation(), &mState.mInfoLog),
        &mState.mExecutable);
    onStateChange(angle::SubjectMessage::ProgramUnlinked);

    // If caching is disabled, consider it cached!
    mIsBinaryCached = context->getFrontendFeatures().disableProgramCaching.enabled;

    // Start with a clean slate every time a new executable is installed.  Note that the executable
    // binary is not mutable; once linked it remains constant.  When the program changes, a new
    // executable is installed in this function.
    mBinary.clear();
}

void Program::setupExecutableForLink(const Context *context)
{
    // Create a new executable to hold the result of the link.  The previous executable may still be
    // referenced by the contexts the program is current on, and any program pipelines it may be
    // used in.  Once link succeeds, the users of the program are notified to update their
    // executables.
    makeNewExecutable(context);

    // For every attached shader, get the compile job and compiled state.  This is done at link time
    // (instead of earlier, such as attachShader time), because the shader could get recompiled
    // between attach and link.
    //
    // Additionally, make sure the backend is also able to cache the compiled state of its own
    // ShaderImpl objects.
    ShaderMap<rx::ShaderImpl *> shaderImpls = {};
    for (ShaderType shaderType : AllShaderTypes())
    {
        Shader *shader = mAttachedShaders[shaderType];
        SharedCompileJob compileJob;
        SharedCompiledShaderState shaderCompiledState;
        if (shader != nullptr)
        {
            compileJob              = shader->getCompileJob(&shaderCompiledState);
            shaderImpls[shaderType] = shader->getImplementation();
        }
        mState.mShaderCompileJobs[shaderType] = std::move(compileJob);
        mState.mAttachedShaders[shaderType]   = std::move(shaderCompiledState);
    }
    mProgram->prepareForLink(shaderImpls);

    const angle::FrontendFeatures &frontendFeatures = context->getFrontendFeatures();
    if (frontendFeatures.dumpShaderSource.enabled)
    {
        dumpProgramInfo(context);
    }

    // Make sure the executable state is in sync with the program.
    //
    // The transform feedback buffer mode is duplicated in the executable as it is the only
    // link-input that is also needed at draw time.
    //
    // The transform feedback varying names are duplicated because the program pipeline link is not
    // currently able to use the link result of the program directly (and redoes the link, using
    // these names).
    //
    // The isSeparable state is duplicated for convenience; it is used when setting sampler/image
    // uniforms.
    mState.mExecutable->mPod.transformFeedbackBufferMode = mState.mTransformFeedbackBufferMode;
    mState.mExecutable->mTransformFeedbackVaryingNames   = mState.mTransformFeedbackVaryingNames;
    mState.mExecutable->mPod.isSeparable                 = mState.mSeparable;

    mState.mInfoLog.reset();
}

angle::Result Program::link(const Context *context, angle::JobResultExpectancy resultExpectancy)
{
    auto *platform   = ANGLEPlatformCurrent();
    double startTime = platform->currentTime(platform);

    setupExecutableForLink(context);

    mProgramHash              = {0};
    MemoryProgramCache *cache = (context->getFrontendFeatures().disableProgramCaching.enabled)
                                    ? nullptr
                                    : context->getMemoryProgramCache();

    // TODO: http://anglebug.com/42263141: Enable program caching for separable programs
    if (cache && !isSeparable())
    {
        std::lock_guard<angle::SimpleMutex> cacheLock(context->getProgramCacheMutex());
        egl::CacheGetResult result = egl::CacheGetResult::NotFound;
        ANGLE_TRY(cache->getProgram(context, this, &mProgramHash, &result));

        switch (result)
        {
            case egl::CacheGetResult::Success:
            {
                // No need to care about the compile jobs any more.
                mState.mShaderCompileJobs = {};

                std::scoped_lock lock(mHistogramMutex);
                // Succeeded in loading the binaries in the front-end, back end may still be loading
                // asynchronously
                double delta = platform->currentTime(platform) - startTime;
                int us       = static_cast<int>(delta * 1000'000.0);
                ANGLE_HISTOGRAM_COUNTS("GPU.ANGLE.ProgramCache.ProgramCacheHitTimeUS", us);
                return angle::Result::Continue;
            }
            case egl::CacheGetResult::Rejected:
                // If the program binary was found but rejected, the program executable may be in an
                // inconsistent half-loaded state.  In that case, start over.
                mLinkingState.reset();
                setupExecutableForLink(context);
                break;
            case egl::CacheGetResult::NotFound:
            default:
                break;
        }
    }

    const Caps &caps               = context->getCaps();
    const Limitations &limitations = context->getLimitations();
    const Version &clientVersion   = context->getClientVersion();
    const bool isWebGL             = context->isWebGL();

    // Ask the backend to prepare the link task.
    std::shared_ptr<rx::LinkTask> linkTask;
    ANGLE_TRY(mProgram->link(context, &linkTask));

    std::unique_ptr<LinkingState> linkingState = std::make_unique<LinkingState>();

    // Prepare the main link job
    std::shared_ptr<MainLinkLoadTask> mainLinkTask(new MainLinkTask(
        context->getLinkSubTaskThreadPool(), caps, limitations, clientVersion, isWebGL, this,
        &mState, &linkingState->linkingVariables, &linkingState->resources, std::move(linkTask)));

    // While the subtasks are currently always thread-safe, the main task is not safe on all
    // backends.  A front-end feature selects whether the single-threaded pool must be used.
    const angle::JobThreadSafety threadSafety =
        context->getFrontendFeatures().linkJobIsThreadSafe.enabled ? angle::JobThreadSafety::Safe
                                                                   : angle::JobThreadSafety::Unsafe;
    std::shared_ptr<angle::WaitableEvent> mainLinkEvent =
        context->postCompileLinkTask(mainLinkTask, threadSafety, resultExpectancy);

    mLinkingState                    = std::move(linkingState);
    mLinkingState->linkingFromBinary = false;
    mLinkingState->linkEvent = std::make_unique<MainLinkLoadEvent>(mainLinkTask, mainLinkEvent);

    return angle::Result::Continue;
}

angle::Result Program::linkJobImpl(const Caps &caps,
                                   const Limitations &limitations,
                                   const Version &clientVersion,
                                   bool isWebGL,
                                   LinkingVariables *linkingVariables,
                                   ProgramLinkedResources *resources,
                                   ProgramMergedVaryings *mergedVaryingsOut)
{
    // Cache load failed, fall through to normal linking.
    unlink();

    // Validate we have properly attached shaders after checking the cache.  Since the input to the
    // shaders is part of the cache key, if there was a cache hit, the shaders would have linked
    // correctly.
    if (!linkValidateShaders())
    {
        return angle::Result::Stop;
    }

    linkShaders();

    linkingVariables->initForProgram(mState);
    resources->init(
        &mState.mExecutable->mUniformBlocks, &mState.mExecutable->mUniforms,
        &mState.mExecutable->mUniformNames, &mState.mExecutable->mUniformMappedNames,
        &mState.mExecutable->mShaderStorageBlocks, &mState.mExecutable->mBufferVariables,
        &mState.mExecutable->mAtomicCounterBuffers, &mState.mExecutable->mPixelLocalStorageFormats);

    updateLinkedShaderStages();

    InitUniformBlockLinker(mState, &resources->uniformBlockLinker);
    InitShaderStorageBlockLinker(mState, &resources->shaderStorageBlockLinker);

    if (mState.mAttachedShaders[ShaderType::Compute])
    {
        GLuint combinedImageUniforms = 0;
        if (!linkUniforms(caps, clientVersion, &resources->unusedUniforms, &combinedImageUniforms))
        {
            return angle::Result::Stop;
        }

        GLuint combinedShaderStorageBlocks = 0u;
        if (!LinkValidateProgramInterfaceBlocks(
                caps, clientVersion, isWebGL, mState.mExecutable->getLinkedShaderStages(),
                *resources, mState.mInfoLog, &combinedShaderStorageBlocks))
        {
            return angle::Result::Stop;
        }

        // [OpenGL ES 3.1] Chapter 8.22 Page 203:
        // A link error will be generated if the sum of the number of active image uniforms used in
        // all shaders, the number of active shader storage blocks, and the number of active
        // fragment shader outputs exceeds the implementation-dependent value of
        // MAX_COMBINED_SHADER_OUTPUT_RESOURCES.
        if (combinedImageUniforms + combinedShaderStorageBlocks >
            static_cast<GLuint>(caps.maxCombinedShaderOutputResources))
        {
            mState.mInfoLog
                << "The sum of the number of active image uniforms, active shader storage blocks "
                   "and active fragment shader outputs exceeds "
                   "MAX_COMBINED_SHADER_OUTPUT_RESOURCES ("
                << caps.maxCombinedShaderOutputResources << ")";
            return angle::Result::Stop;
        }
    }
    else
    {
        if (!linkAttributes(caps, limitations, isWebGL))
        {
            return angle::Result::Stop;
        }

        if (!linkVaryings())
        {
            return angle::Result::Stop;
        }

        GLuint combinedImageUniforms = 0;
        if (!linkUniforms(caps, clientVersion, &resources->unusedUniforms, &combinedImageUniforms))
        {
            return angle::Result::Stop;
        }

        GLuint combinedShaderStorageBlocks = 0u;
        if (!LinkValidateProgramInterfaceBlocks(
                caps, clientVersion, isWebGL, mState.mExecutable->getLinkedShaderStages(),
                *resources, mState.mInfoLog, &combinedShaderStorageBlocks))
        {
            return angle::Result::Stop;
        }

        if (!LinkValidateProgramGlobalNames(mState.mInfoLog, getExecutable(), *linkingVariables))
        {
            return angle::Result::Stop;
        }

        const SharedCompiledShaderState &vertexShader = mState.mAttachedShaders[ShaderType::Vertex];
        if (vertexShader)
        {
            mState.mExecutable->mPod.numViews = vertexShader->numViews;
            mState.mExecutable->mPod.hasClipDistance =
                vertexShader->metadataFlags.test(sh::MetadataFlags::HasClipDistance);
            mState.mExecutable->mPod.specConstUsageBits |= vertexShader->specConstUsageBits;
        }

        const SharedCompiledShaderState &fragmentShader =
            mState.mAttachedShaders[ShaderType::Fragment];
        if (fragmentShader)
        {
            ASSERT(mState.mExecutable->mOutputVariables.empty());
            mState.mExecutable->mOutputVariables.reserve(
                fragmentShader->activeOutputVariables.size());
            for (const sh::ShaderVariable &shaderVariable : fragmentShader->activeOutputVariables)
            {
                mState.mExecutable->mOutputVariables.emplace_back(shaderVariable);
            }
            if (!mState.mExecutable->linkValidateOutputVariables(
                    caps, clientVersion, combinedImageUniforms, combinedShaderStorageBlocks,
                    fragmentShader->shaderVersion, mState.mFragmentOutputLocations,
                    mState.mFragmentOutputIndexes))
            {
                return angle::Result::Stop;
            }

            mState.mExecutable->mPod.hasDiscard =
                fragmentShader->metadataFlags.test(sh::MetadataFlags::HasDiscard);
            mState.mExecutable->mPod.enablesPerSampleShading =
                fragmentShader->metadataFlags.test(sh::MetadataFlags::EnablesPerSampleShading);
            mState.mExecutable->mPod.hasDepthInputAttachment =
                fragmentShader->metadataFlags.test(sh::MetadataFlags::HasDepthInputAttachment);
            mState.mExecutable->mPod.hasStencilInputAttachment =
                fragmentShader->metadataFlags.test(sh::MetadataFlags::HasStencilInputAttachment);
            mState.mExecutable->mPod.advancedBlendEquations =
                fragmentShader->advancedBlendEquations;
            mState.mExecutable->mPod.specConstUsageBits |= fragmentShader->specConstUsageBits;

            for (uint32_t index = 0; index < IMPLEMENTATION_MAX_DRAW_BUFFERS; ++index)
            {
                const sh::MetadataFlags flag = static_cast<sh::MetadataFlags>(
                    static_cast<uint32_t>(sh::MetadataFlags::HasInputAttachment0) + index);
                if (fragmentShader->metadataFlags.test(flag))
                {
                    mState.mExecutable->mPod.fragmentInoutIndices.set(index);
                }
            }
        }

        *mergedVaryingsOut = GetMergedVaryingsFromLinkingVariables(*linkingVariables);
        if (!mState.mExecutable->linkMergedVaryings(caps, limitations, clientVersion, isWebGL,
                                                    *mergedVaryingsOut, *linkingVariables,
                                                    &resources->varyingPacking))
        {
            return angle::Result::Stop;
        }
    }

    mState.mExecutable->saveLinkedStateInfo(mState);

    return angle::Result::Continue;
}

bool Program::isLinking() const
{
    return mLinkingState.get() && mLinkingState->linkEvent && mLinkingState->linkEvent->isLinking();
}

bool Program::isBinaryReady(const Context *context)
{
    if (mState.mExecutable->mPostLinkSubTasks.empty())
    {
        // Ensure the program binary is cached, even if the backend waits for post-link tasks
        // without the knowledge of the front-end.
        cacheProgramBinaryIfNotAlready(context);
        return true;
    }

    const bool allPostLinkTasksComplete =
        angle::WaitableEvent::AllReady(&mState.mExecutable->getPostLinkSubTaskWaitableEvents());

    // Once the binary is ready, the |glGetProgramBinary| call will result in
    // |waitForPostLinkTasks| which in turn may internally cache the binary.  However, for the sake
    // of blob cache tests, call |waitForPostLinkTasks| anyway if tasks are already complete.
    if (allPostLinkTasksComplete)
    {
        waitForPostLinkTasks(context);
    }

    return allPostLinkTasksComplete;
}

void Program::resolveLinkImpl(const Context *context)
{
    ASSERT(mLinkingState.get());

    angle::Result result                       = mLinkingState->linkEvent->wait(context);
    mLinked                                    = result == angle::Result::Continue;
    std::unique_ptr<LinkingState> linkingState = std::move(mLinkingState);
    if (!mLinked)
    {
        // If the link fails, the spec allows program queries to either return empty results (all
        // zeros) or whatever parts of the link happened to have been done before the failure:
        //
        // > Implementations may return information on variables and interface blocks that would
        // > have been active had the program been linked successfully.  In cases where the link
        // > failed because the program required too many resources, these commands may help
        // > applications determine why limits were exceeded. However, the information returned in
        // > this case is implementation-dependent and may be incomplete.
        //
        // The above means that it's ok for ANGLE to reset the executable here, but it *may* be
        // helpful to applications if it doesn't.  We do reset it however, the info log should
        // already have enough debug information for the application.
        mState.mExecutable->reset();
        return;
    }

    // According to GLES 3.0/3.1 spec for LinkProgram and UseProgram,
    // Only successfully linked program can replace the executables.
    ASSERT(mLinked);

    // In case of a successful link, it is no longer required for the attached shaders to hold on to
    // the memory they have used. Therefore, the shader compilations are resolved to save memory.
    for (Shader *shader : mAttachedShaders)
    {
        if (shader != nullptr)
        {
            shader->resolveCompile(context);
        }
    }

    // Mark implementation-specific unreferenced uniforms as ignored.
    std::vector<ImageBinding> *imageBindings = getExecutable().getImageBindings();
    mProgram->markUnusedUniformLocations(&mState.mExecutable->mUniformLocations,
                                         &mState.mExecutable->mSamplerBindings, imageBindings);

    // Must be called after markUnusedUniformLocations.
    postResolveLink(context);

    // Notify observers that a new linked executable is available.  If this program is current on a
    // context, the executable is reinstalled.  If it is attached to a PPO, it is installed there
    // and the PPO is marked as needing to be linked again.
    onStateChange(angle::SubjectMessage::ProgramRelinked);

    // Cache the program if:
    //
    // - Not loading from binary, in which case the program is already in the cache.
    // - There are no post link tasks. If there are any, waitForPostLinkTasks will do this
    //   instead.
    //   * Note that serialize() calls waitForPostLinkTasks, so caching the binary here
    //     effectively forces a wait for the post-link tasks.
    //
    if (!linkingState->linkingFromBinary && mState.mExecutable->mPostLinkSubTasks.empty())
    {
        cacheProgramBinaryIfNotAlready(context);
    }
}

void Program::waitForPostLinkTasks(const Context *context)
{
    // No-op if no tasks.
    mState.mExecutable->waitForPostLinkTasks(context);

    // Now that the subtasks are done, cache the binary (this was deferred in resolveLinkImpl).
    cacheProgramBinaryIfNotAlready(context);
}

void Program::updateLinkedShaderStages()
{
    mState.mExecutable->resetLinkedShaderStages();

    for (ShaderType shaderType : AllShaderTypes())
    {
        if (mState.mAttachedShaders[shaderType])
        {
            mState.mExecutable->setLinkedShaderStages(shaderType);
        }
    }
}

void ProgramState::updateActiveSamplers()
{
    mExecutable->mActiveSamplerRefCounts.fill(0);
    mExecutable->updateActiveSamplers(*mExecutable);
}

void ProgramState::updateProgramInterfaceInputs()
{
    const ShaderType firstAttachedShaderType = mExecutable->getFirstLinkedShaderStageType();

    if (firstAttachedShaderType == ShaderType::Vertex)
    {
        // Vertex attributes are already what we need, so nothing to do
        return;
    }

    const SharedCompiledShaderState &shader = getAttachedShader(firstAttachedShaderType);
    ASSERT(shader);

    // Copy over each input varying, since the Shader could go away
    if (shader->shaderType == ShaderType::Compute)
    {
        for (const sh::ShaderVariable &attribute : shader->allAttributes)
        {
            // Compute Shaders have the following built-in input variables.
            //
            // in uvec3 gl_NumWorkGroups;
            // in uvec3 gl_WorkGroupID;
            // in uvec3 gl_LocalInvocationID;
            // in uvec3 gl_GlobalInvocationID;
            // in uint  gl_LocalInvocationIndex;
            // They are all vecs or uints, so no special handling is required.
            mExecutable->mProgramInputs.emplace_back(attribute);
        }
    }
    else
    {
        for (const sh::ShaderVariable &varying : shader->inputVaryings)
        {
            UpdateInterfaceVariable(&mExecutable->mProgramInputs, varying);
        }
    }
}

void ProgramState::updateProgramInterfaceOutputs()
{
    const ShaderType lastAttachedShaderType = mExecutable->getLastLinkedShaderStageType();

    if (lastAttachedShaderType == ShaderType::Fragment)
    {
        // Fragment outputs are already what we need, so nothing to do
        return;
    }
    if (lastAttachedShaderType == ShaderType::Compute)
    {
        // If the program only contains a Compute Shader, then there are no user-defined outputs.
        return;
    }

    const SharedCompiledShaderState &shader = getAttachedShader(lastAttachedShaderType);
    ASSERT(shader);

    // Copy over each output varying, since the Shader could go away
    for (const sh::ShaderVariable &varying : shader->outputVaryings)
    {
        UpdateInterfaceVariable(&mExecutable->mOutputVariables, varying);
    }
}

// Returns the program object to an unlinked state, before re-linking, or at destruction
void Program::unlink()
{
    // There is always a new executable created on link, so the executable is already in a clean
    // state.

    mValidated = false;
}

angle::Result Program::setBinary(const Context *context,
                                 GLenum binaryFormat,
                                 const void *binary,
                                 GLsizei length)
{
    ASSERT(binaryFormat == GL_PROGRAM_BINARY_ANGLE);

    makeNewExecutable(context);

    egl::CacheGetResult result = egl::CacheGetResult::NotFound;
    return loadBinary(context, binary, length, &result);
}

angle::Result Program::loadBinary(const Context *context,
                                  const void *binary,
                                  GLsizei length,
                                  egl::CacheGetResult *resultOut)
{
    *resultOut = egl::CacheGetResult::Rejected;

    ASSERT(mLinkingState);
    unlink();

    BinaryInputStream stream(binary, length);
    if (!deserialize(context, stream))
    {
        return angle::Result::Continue;
    }
    // Currently we require the full shader text to compute the program hash.
    // We could also store the binary in the internal program cache.

    // Initialize the uniform block -> buffer index map based on serialized data.
    mState.mExecutable->initInterfaceBlockBindings();

    // If load does not succeed, we know for sure that the binary is not compatible with the
    // backend.  The loaded binary could have been read from the on-disk shader cache and be
    // corrupted or serialized with different revision and subsystem id than the currently loaded
    // backend.  Returning to the caller results in link happening using the original shader
    // sources.
    std::shared_ptr<rx::LinkTask> loadTask;
    ANGLE_TRY(mProgram->load(context, &stream, &loadTask, resultOut));
    if (*resultOut == egl::CacheGetResult::Rejected)
    {
        return angle::Result::Continue;
    }

    std::unique_ptr<LinkEvent> loadEvent;
    if (loadTask)
    {
        std::shared_ptr<MainLinkLoadTask> mainLoadTask(new MainLoadTask(
            context->getLinkSubTaskThreadPool(), this, &mState, std::move(loadTask)));

        std::shared_ptr<angle::WaitableEvent> mainLoadEvent =
            context->getShaderCompileThreadPool()->postWorkerTask(mainLoadTask);
        loadEvent = std::make_unique<MainLinkLoadEvent>(mainLoadTask, mainLoadEvent);
    }
    else
    {
        loadEvent = std::make_unique<LinkEventDone>(angle::Result::Continue);
    }

    mLinkingState->linkingFromBinary = true;
    mLinkingState->linkEvent         = std::move(loadEvent);

    // Don't attempt to cache the binary that's just loaded
    mIsBinaryCached = true;

    *resultOut = egl::CacheGetResult::Success;

    return angle::Result::Continue;
}

angle::Result Program::getBinary(Context *context,
                                 GLenum *binaryFormat,
                                 void *binary,
                                 GLsizei bufSize,
                                 GLsizei *length)
{
    if (!mState.mBinaryRetrieveableHint)
    {
        ANGLE_PERF_WARNING(
            context->getState().getDebug(), GL_DEBUG_SEVERITY_LOW,
            "Saving program binary without GL_PROGRAM_BINARY_RETRIEVABLE_HINT is suboptimal.");
    }

    ASSERT(!mLinkingState);
    if (binaryFormat)
    {
        *binaryFormat = GL_PROGRAM_BINARY_ANGLE;
    }

    // Serialize the program only if not already done.
    if (mBinary.empty())
    {
        ANGLE_TRY(serialize(context));
    }

    GLsizei streamLength       = static_cast<GLsizei>(mBinary.size());
    const uint8_t *streamState = mBinary.data();

    if (streamLength > bufSize)
    {
        if (length)
        {
            *length = 0;
        }

        // TODO: This should be moved to the validation layer but computing the size of the binary
        // before saving it causes the save to happen twice.  It may be possible to write the binary
        // to a separate buffer, validate sizes and then copy it.
        ANGLE_CHECK(context, false, err::kInsufficientBufferSize, GL_INVALID_OPERATION);
    }

    if (binary)
    {
        char *ptr = reinterpret_cast<char *>(binary);

        memcpy(ptr, streamState, streamLength);
        ptr += streamLength;

        ASSERT(ptr - streamLength == binary);

        // Once the binary is retrieved, assume the application will never need the binary and
        // release the memory.  Note that implicit caching to blob cache is disabled when the
        // GL_PROGRAM_BINARY_RETRIEVABLE_HINT is set.  If that hint is not set, serialization is
        // done twice, which is what the perf warning above is about!
        mBinary.clear();
    }

    if (length)
    {
        *length = streamLength;
    }

    return angle::Result::Continue;
}

GLint Program::getBinaryLength(Context *context)
{
    ASSERT(!mLinkingState);
    if (!mLinked)
    {
        return 0;
    }

    GLint length;
    angle::Result result =
        getBinary(context, nullptr, nullptr, std::numeric_limits<GLint>::max(), &length);
    if (result != angle::Result::Continue)
    {
        return 0;
    }

    return length;
}

void Program::setBinaryRetrievableHint(bool retrievable)
{
    ASSERT(!mLinkingState);
    // TODO(jmadill) : replace with dirty bits
    mProgram->setBinaryRetrievableHint(retrievable);
    mState.mBinaryRetrieveableHint = retrievable;
}

bool Program::getBinaryRetrievableHint() const
{
    ASSERT(!mLinkingState);
    return mState.mBinaryRetrieveableHint;
}

int Program::getInfoLogLength() const
{
    return static_cast<int>(mState.mInfoLog.getLength());
}

void Program::getInfoLog(GLsizei bufSize, GLsizei *length, char *infoLog) const
{
    return mState.mInfoLog.getLog(bufSize, length, infoLog);
}

void Program::setSeparable(const Context *context, bool separable)
{
    ASSERT(!mLinkingState);

    if (isSeparable() != separable)
    {
        mProgram->setSeparable(separable);
        mState.mSeparable = separable;
    }
}

void Program::deleteSelf(const Context *context)
{
    ASSERT(mRefCount == 0 && mDeleteStatus);
    mResourceManager->deleteProgram(context, mHandle);
}

unsigned int Program::getRefCount() const
{
    return mRefCount;
}

void Program::getAttachedShaders(GLsizei maxCount, GLsizei *count, ShaderProgramID *shaders) const
{
    int total = 0;

    for (const Shader *shader : mAttachedShaders)
    {
        if (shader != nullptr && total < maxCount)
        {
            shaders[total] = shader->getHandle();
            ++total;
        }
    }

    if (count)
    {
        *count = total;
    }
}

void Program::flagForDeletion()
{
    ASSERT(!mLinkingState);
    mDeleteStatus = true;
}

bool Program::isFlaggedForDeletion() const
{
    ASSERT(!mLinkingState);
    return mDeleteStatus;
}

void Program::validate(const Caps &caps)
{
    ASSERT(!mLinkingState);
    mState.mInfoLog.reset();

    if (mLinked)
    {
        mValidated = ConvertToBool(mProgram->validate(caps));
    }
    else
    {
        mState.mInfoLog << "Program has not been successfully linked.";
    }
}

bool Program::isValidated() const
{
    ASSERT(!mLinkingState);
    return mValidated;
}

void Program::bindUniformBlock(UniformBlockIndex uniformBlockIndex, GLuint uniformBlockBinding)
{
    ASSERT(!mLinkingState);

    mState.mExecutable->remapUniformBlockBinding(uniformBlockIndex, uniformBlockBinding);

    mProgram->onUniformBlockBinding(uniformBlockIndex);

    onStateChange(
        angle::ProgramUniformBlockBindingUpdatedMessageFromIndex(uniformBlockIndex.value));
}

void Program::setTransformFeedbackVaryings(const Context *context,
                                           GLsizei count,
                                           const GLchar *const *varyings,
                                           GLenum bufferMode)
{
    ASSERT(!mLinkingState);

    mState.mTransformFeedbackVaryingNames.resize(count);
    for (GLsizei i = 0; i < count; i++)
    {
        mState.mTransformFeedbackVaryingNames[i] = varyings[i];
    }

    mState.mTransformFeedbackBufferMode = bufferMode;
}

bool Program::linkValidateShaders()
{
    // Wait for attached shaders to finish compilation.  At this point, they need to be checked
    // whether they successfully compiled.  This information is cached so that all compile jobs can
    // be waited on and their corresponding objects released before the actual check.
    //
    // Note that this function is called from the link job, and is therefore not protected by any
    // locks.
    ShaderBitSet successfullyCompiledShaders;
    for (ShaderType shaderType : AllShaderTypes())
    {
        const SharedCompileJob &compileJob = mState.mShaderCompileJobs[shaderType];
        if (compileJob)
        {
            const bool success = WaitCompileJobUnlocked(compileJob);
            successfullyCompiledShaders.set(shaderType, success);
        }
    }
    mState.mShaderCompileJobs = {};

    const ShaderMap<SharedCompiledShaderState> &shaders = mState.mAttachedShaders;

    bool isComputeShaderAttached  = shaders[ShaderType::Compute].get() != nullptr;
    bool isGraphicsShaderAttached = shaders[ShaderType::Vertex].get() != nullptr ||
                                    shaders[ShaderType::TessControl].get() != nullptr ||
                                    shaders[ShaderType::TessEvaluation].get() != nullptr ||
                                    shaders[ShaderType::Geometry].get() != nullptr ||
                                    shaders[ShaderType::Fragment].get() != nullptr;
    // Check whether we both have a compute and non-compute shaders attached.
    // If there are of both types attached, then linking should fail.
    // OpenGL ES 3.10, 7.3 Program Objects, under LinkProgram
    if (isComputeShaderAttached && isGraphicsShaderAttached)
    {
        mState.mInfoLog << "Both compute and graphics shaders are attached to the same program.";
        return false;
    }

    Optional<int> version;
    for (ShaderType shaderType : kAllGraphicsShaderTypes)
    {
        const SharedCompiledShaderState &shader = shaders[shaderType];
        ASSERT(!shader || shader->shaderType == shaderType);

        if (!shader)
        {
            continue;
        }

        if (!successfullyCompiledShaders.test(shaderType))
        {
            mState.mInfoLog << ShaderTypeToString(shaderType) << " shader is not compiled.";
            return false;
        }

        if (!version.valid())
        {
            version = shader->shaderVersion;
        }
        else if (version != shader->shaderVersion)
        {
            mState.mInfoLog << ShaderTypeToString(shaderType)
                            << " shader version does not match other shader versions.";
            return false;
        }
    }

    if (isComputeShaderAttached)
    {
        ASSERT(shaders[ShaderType::Compute]->shaderType == ShaderType::Compute);

        // GLSL ES 3.10, 4.4.1.1 Compute Shader Inputs
        // If the work group size is not specified, a link time error should occur.
        if (!shaders[ShaderType::Compute]->localSize.isDeclared())
        {
            mState.mInfoLog << "Work group size is not specified.";
            return false;
        }
    }
    else
    {
        if (!isGraphicsShaderAttached)
        {
            mState.mInfoLog << "No compiled shaders.";
            return false;
        }

        bool hasVertex   = shaders[ShaderType::Vertex].get() != nullptr;
        bool hasFragment = shaders[ShaderType::Fragment].get() != nullptr;
        if (!isSeparable() && (!hasVertex || !hasFragment))
        {
            mState.mInfoLog
                << "The program must contain objects to form both a vertex and fragment shader.";
            return false;
        }

        bool hasTessControl    = shaders[ShaderType::TessControl].get() != nullptr;
        bool hasTessEvaluation = shaders[ShaderType::TessEvaluation].get() != nullptr;
        if (!isSeparable() && (hasTessControl != hasTessEvaluation))
        {
            mState.mInfoLog
                << "Tessellation control and evaluation shaders must be specified together.";
            return false;
        }

        const SharedCompiledShaderState &geometryShader = shaders[ShaderType::Geometry];
        if (geometryShader)
        {
            // [GL_EXT_geometry_shader] Chapter 7
            // Linking can fail for a variety of reasons as specified in the OpenGL ES Shading
            // Language Specification, as well as any of the following reasons:
            // * One or more of the shader objects attached to <program> are not compiled
            //   successfully.
            // * The shaders do not use the same shader language version.
            // * <program> contains objects to form a geometry shader, and
            //   - <program> is not separable and contains no objects to form a vertex shader; or
            //   - the input primitive type, output primitive type, or maximum output vertex count
            //     is not specified in the compiled geometry shader object.
            if (!geometryShader->hasValidGeometryShaderInputPrimitiveType())
            {
                mState.mInfoLog << "Input primitive type is not specified in the geometry shader.";
                return false;
            }

            if (!geometryShader->hasValidGeometryShaderOutputPrimitiveType())
            {
                mState.mInfoLog << "Output primitive type is not specified in the geometry shader.";
                return false;
            }

            if (!geometryShader->hasValidGeometryShaderMaxVertices())
            {
                mState.mInfoLog << "'max_vertices' is not specified in the geometry shader.";
                return false;
            }
        }

        const SharedCompiledShaderState &tessControlShader = shaders[ShaderType::TessControl];
        if (tessControlShader)
        {
            int tcsShaderVertices = tessControlShader->tessControlShaderVertices;
            if (tcsShaderVertices == 0)
            {
                // In tessellation control shader, output vertices should be specified at least
                // once.
                // > GLSL ES Version 3.20.6 spec:
                // > 4.4.2. Output Layout Qualifiers
                // > Tessellation Control Outputs
                // > ...
                // > There must be at least one layout qualifier specifying an output patch vertex
                // > count in any program containing a tessellation control shader.
                mState.mInfoLog << "In Tessellation Control Shader, at least one layout qualifier "
                                   "specifying an output patch vertex count must exist.";
                return false;
            }
        }

        const SharedCompiledShaderState &tessEvaluationShader = shaders[ShaderType::TessEvaluation];
        if (tessEvaluationShader)
        {
            GLenum tesPrimitiveMode = tessEvaluationShader->tessGenMode;
            if (tesPrimitiveMode == 0)
            {
                // In tessellation evaluation shader, a primitive mode should be specified at least
                // once.
                // > GLSL ES Version 3.20.6 spec:
                // > 4.4.1. Input Layout Qualifiers
                // > Tessellation Evaluation Inputs
                // > ...
                // > The tessellation evaluation shader object in a program must declare a primitive
                // > mode in its input layout. Declaring vertex spacing, ordering, or point mode
                // > identifiers is optional.
                mState.mInfoLog
                    << "The Tessellation Evaluation Shader object in a program must declare a "
                       "primitive mode in its input layout.";
                return false;
            }
        }
    }

    return true;
}

// Assumes linkValidateShaders() has validated the shaders and caches some values from the shaders.
void Program::linkShaders()
{
    const ShaderMap<SharedCompiledShaderState> &shaders = mState.mAttachedShaders;

    const bool isComputeShaderAttached = shaders[ShaderType::Compute].get() != nullptr;

    if (isComputeShaderAttached)
    {
        mState.mExecutable->mPod.computeShaderLocalSize = shaders[ShaderType::Compute]->localSize;
    }
    else
    {
        const SharedCompiledShaderState &geometryShader = shaders[ShaderType::Geometry];
        if (geometryShader)
        {
            mState.mExecutable->mPod.geometryShaderInputPrimitiveType =
                geometryShader->geometryShaderInputPrimitiveType;
            mState.mExecutable->mPod.geometryShaderOutputPrimitiveType =
                geometryShader->geometryShaderOutputPrimitiveType;
            mState.mExecutable->mPod.geometryShaderMaxVertices =
                geometryShader->geometryShaderMaxVertices;
            mState.mExecutable->mPod.geometryShaderInvocations =
                geometryShader->geometryShaderInvocations;
        }

        const SharedCompiledShaderState &tessControlShader = shaders[ShaderType::TessControl];
        if (tessControlShader)
        {
            int tcsShaderVertices = tessControlShader->tessControlShaderVertices;
            mState.mExecutable->mPod.tessControlShaderVertices = tcsShaderVertices;
        }

        const SharedCompiledShaderState &tessEvaluationShader = shaders[ShaderType::TessEvaluation];
        if (tessEvaluationShader)
        {
            GLenum tesPrimitiveMode = tessEvaluationShader->tessGenMode;

            mState.mExecutable->mPod.tessGenMode        = tesPrimitiveMode;
            mState.mExecutable->mPod.tessGenSpacing     = tessEvaluationShader->tessGenSpacing;
            mState.mExecutable->mPod.tessGenVertexOrder = tessEvaluationShader->tessGenVertexOrder;
            mState.mExecutable->mPod.tessGenPointMode   = tessEvaluationShader->tessGenPointMode;
        }
    }
}

bool Program::linkVaryings()
{
    ShaderType previousShaderType = ShaderType::InvalidEnum;
    for (ShaderType shaderType : kAllGraphicsShaderTypes)
    {
        const SharedCompiledShaderState &currentShader = mState.mAttachedShaders[shaderType];
        if (!currentShader)
        {
            continue;
        }

        if (previousShaderType != ShaderType::InvalidEnum)
        {
            const SharedCompiledShaderState &previousShader =
                mState.mAttachedShaders[previousShaderType];
            const std::vector<sh::ShaderVariable> &outputVaryings = previousShader->outputVaryings;

            if (!LinkValidateShaderInterfaceMatching(
                    outputVaryings, currentShader->inputVaryings, previousShaderType,
                    currentShader->shaderType, previousShader->shaderVersion,
                    currentShader->shaderVersion, isSeparable(), mState.mInfoLog))
            {
                return false;
            }
        }
        previousShaderType = currentShader->shaderType;
    }

    // TODO: http://anglebug.com/42262233 and http://anglebug.com/42262234
    // Need to move logic of validating builtin varyings inside the for-loop above.
    // This is because the built-in symbols `gl_ClipDistance` and `gl_CullDistance`
    // can be redeclared in Geometry or Tessellation shaders as well.
    const SharedCompiledShaderState &vertexShader   = mState.mAttachedShaders[ShaderType::Vertex];
    const SharedCompiledShaderState &fragmentShader = mState.mAttachedShaders[ShaderType::Fragment];
    if (vertexShader && fragmentShader &&
        !LinkValidateBuiltInVaryings(vertexShader->outputVaryings, fragmentShader->inputVaryings,
                                     vertexShader->shaderType, fragmentShader->shaderType,
                                     vertexShader->shaderVersion, fragmentShader->shaderVersion,
                                     mState.mInfoLog))
    {
        return false;
    }

    return true;
}

bool Program::linkUniforms(const Caps &caps,
                           const Version &clientVersion,
                           std::vector<UnusedUniform> *unusedUniformsOutOrNull,
                           GLuint *combinedImageUniformsOut)
{
    // Initialize executable shader map.
    ShaderMap<std::vector<sh::ShaderVariable>> shaderUniforms;
    for (const SharedCompiledShaderState &shader : mState.mAttachedShaders)
    {
        if (shader)
        {
            shaderUniforms[shader->shaderType] = shader->uniforms;
        }
    }

    if (!mState.mExecutable->linkUniforms(caps, shaderUniforms, mState.mUniformLocationBindings,
                                          combinedImageUniformsOut, unusedUniformsOutOrNull))
    {
        return false;
    }

    if (clientVersion >= Version(3, 1))
    {
        GLint locationSize = static_cast<GLint>(mState.mExecutable->getUniformLocations().size());

        if (locationSize > caps.maxUniformLocations)
        {
            mState.mInfoLog << "Exceeded maximum uniform location size";
            return false;
        }
    }

    return true;
}

// Assigns locations to all attributes (except built-ins) from the bindings and program locations.
bool Program::linkAttributes(const Caps &caps,
                             const Limitations &limitations,
                             bool webglCompatibility)
{
    int shaderVersion          = -1;
    unsigned int usedLocations = 0;

    const SharedCompiledShaderState &vertexShader = mState.getAttachedShader(ShaderType::Vertex);

    if (!vertexShader)
    {
        // No vertex shader, so no attributes, so nothing to do
        return true;
    }

    // In GLSL ES 3.00.6, aliasing checks should be done with all declared attributes -
    // see GLSL ES 3.00.6 section 12.46. Inactive attributes will be pruned after
    // aliasing checks.
    // In GLSL ES 1.00.17 we only do aliasing checks for active attributes.
    shaderVersion = vertexShader->shaderVersion;
    const std::vector<sh::ShaderVariable> &shaderAttributes =
        shaderVersion >= 300 ? vertexShader->allAttributes : vertexShader->activeAttributes;

    ASSERT(mState.mExecutable->mProgramInputs.empty());
    mState.mExecutable->mProgramInputs.reserve(shaderAttributes.size());

    GLuint maxAttribs = static_cast<GLuint>(caps.maxVertexAttributes);
    std::vector<ProgramInput *> usedAttribMap(maxAttribs, nullptr);

    for (const sh::ShaderVariable &shaderAttribute : shaderAttributes)
    {
        // GLSL ES 3.10 January 2016 section 4.3.4: Vertex shader inputs can't be arrays or
        // structures, so we don't need to worry about adjusting their names or generating entries
        // for each member/element (unlike uniforms for example).
        ASSERT(!shaderAttribute.isArray() && !shaderAttribute.isStruct());

        mState.mExecutable->mProgramInputs.emplace_back(shaderAttribute);

        // Assign locations to attributes that have a binding location and check for attribute
        // aliasing.
        ProgramInput &attribute = mState.mExecutable->mProgramInputs.back();
        int bindingLocation     = mState.mAttributeBindings.getBinding(attribute);
        if (attribute.getLocation() == -1 && bindingLocation != -1)
        {
            attribute.setLocation(bindingLocation);
        }

        if (attribute.getLocation() != -1)
        {
            // Location is set by glBindAttribLocation or by location layout qualifier
            const int regs = VariableRegisterCount(attribute.getType());

            if (static_cast<GLuint>(regs + attribute.getLocation()) > maxAttribs)
            {
                mState.mInfoLog << "Attribute (" << attribute.name << ") at location "
                                << attribute.getLocation() << " is too big to fit";

                return false;
            }

            for (int reg = 0; reg < regs; reg++)
            {
                const int regLocation         = attribute.getLocation() + reg;
                ProgramInput *linkedAttribute = usedAttribMap[regLocation];

                // In GLSL ES 3.00.6 and in WebGL, attribute aliasing produces a link error.
                // In non-WebGL GLSL ES 1.00.17, attribute aliasing is allowed with some
                // restrictions - see GLSL ES 1.00.17 section 2.10.4, but ANGLE currently has a bug.
                // In D3D 9 and 11, aliasing is not supported, so check a limitation.
                if (linkedAttribute)
                {
                    if (shaderVersion >= 300 || webglCompatibility ||
                        limitations.noVertexAttributeAliasing)
                    {
                        mState.mInfoLog << "Attribute '" << attribute.name
                                        << "' aliases attribute '" << linkedAttribute->name
                                        << "' at location " << regLocation;
                        return false;
                    }
                }
                else
                {
                    usedAttribMap[regLocation] = &attribute;
                }

                usedLocations |= 1 << regLocation;
            }
        }
    }

    // Assign locations to attributes that don't have a binding location.
    for (ProgramInput &attribute : mState.mExecutable->mProgramInputs)
    {
        // Not set by glBindAttribLocation or by location layout qualifier
        if (attribute.getLocation() == -1)
        {
            int regs           = VariableRegisterCount(attribute.getType());
            int availableIndex = AllocateFirstFreeBits(&usedLocations, regs, maxAttribs);

            if (availableIndex == -1 || static_cast<GLuint>(availableIndex + regs) > maxAttribs)
            {
                mState.mInfoLog << "Too many attributes (" << attribute.name << ")";
                return false;
            }

            attribute.setLocation(availableIndex);
        }
    }

    ASSERT(mState.mExecutable->mPod.attributesTypeMask.none());
    ASSERT(mState.mExecutable->mPod.attributesMask.none());

    // Prune inactive attributes. This step is only needed on shaderVersion >= 300 since on earlier
    // shader versions we're only processing active attributes to begin with.
    if (shaderVersion >= 300)
    {
        for (auto attributeIter = mState.mExecutable->getProgramInputs().begin();
             attributeIter != mState.mExecutable->getProgramInputs().end();)
        {
            if (attributeIter->isActive())
            {
                ++attributeIter;
            }
            else
            {
                attributeIter = mState.mExecutable->mProgramInputs.erase(attributeIter);
            }
        }
    }

    for (const ProgramInput &attribute : mState.mExecutable->getProgramInputs())
    {
        ASSERT(attribute.isActive());
        ASSERT(attribute.getLocation() != -1);
        unsigned int regs = static_cast<unsigned int>(VariableRegisterCount(attribute.getType()));

        unsigned int location = static_cast<unsigned int>(attribute.getLocation());
        for (unsigned int r = 0; r < regs; r++)
        {
            // Built-in active program inputs don't have a bound attribute.
            if (!attribute.isBuiltIn())
            {
                mState.mExecutable->mPod.activeAttribLocationsMask.set(location);
                mState.mExecutable->mPod.maxActiveAttribLocation =
                    std::max(mState.mExecutable->mPod.maxActiveAttribLocation, location + 1);

                ComponentType componentType =
                    GLenumToComponentType(VariableComponentType(attribute.getType()));

                SetComponentTypeMask(componentType, location,
                                     &mState.mExecutable->mPod.attributesTypeMask);
                mState.mExecutable->mPod.attributesMask.set(location);

                location++;
            }
        }
    }

    return true;
}

angle::Result Program::serialize(const Context *context)
{
    // In typical applications, the binary should already be empty here.  However, in unusual
    // situations this may not be true.  In particular, if the application doesn't set
    // GL_PROGRAM_BINARY_RETRIEVABLE_HINT, gets the program length but doesn't get the binary, the
    // cached binary remains until the program is destroyed or the program is bound (both causing
    // |waitForPostLinkTasks()| to cache the program in the blob cache).
    if (!mBinary.empty())
    {
        return angle::Result::Continue;
    }

    BinaryOutputStream stream;

    stream.writeBytes(
        reinterpret_cast<const unsigned char *>(angle::GetANGLEShaderProgramVersion()),
        angle::GetANGLEShaderProgramVersionHashSize());

    stream.writeBool(angle::Is64Bit());

    stream.writeInt(angle::GetANGLESHVersion());

    stream.writeString(context->getRendererString());

    // nullptr context is supported when computing binary length.
    if (context)
    {
        stream.writeInt(context->getClientVersion().major);
        stream.writeInt(context->getClientVersion().minor);
    }
    else
    {
        stream.writeInt(2);
        stream.writeInt(0);
    }

    // mSeparable must be before mExecutable->save(), since it uses the value.
    stream.writeBool(mState.mSeparable);
    stream.writeInt(mState.mTransformFeedbackBufferMode);

    stream.writeInt(mState.mTransformFeedbackVaryingNames.size());
    for (const std::string &name : mState.mTransformFeedbackVaryingNames)
    {
        stream.writeString(name);
    }

    mState.mExecutable->save(&stream);

    // Warn the app layer if saving a binary with unsupported transform feedback.
    if (!mState.mExecutable->getLinkedTransformFeedbackVaryings().empty() &&
        context->getFrontendFeatures().disableProgramCachingForTransformFeedback.enabled)
    {
        ANGLE_PERF_WARNING(context->getState().getDebug(), GL_DEBUG_SEVERITY_LOW,
                           "Saving program binary with transform feedback, which is not supported "
                           "on this driver.");
    }

    if (context->getShareGroup()->getFrameCaptureShared()->enabled())
    {
        // Serialize the source for each stage for re-use during capture
        for (ShaderType shaderType : mState.mExecutable->getLinkedShaderStages())
        {
            Shader *shader = getAttachedShader(shaderType);
            if (shader)
            {
                stream.writeString(shader->getSourceString());
            }
            else
            {
                // If we don't have an attached shader, which would occur if this program was
                // created via glProgramBinary, pull from our cached copy
                const angle::ProgramSources &cachedLinkedSources =
                    context->getShareGroup()->getFrameCaptureShared()->getProgramSources(id());
                const std::string &cachedSourceString = cachedLinkedSources[shaderType];
                ASSERT(!cachedSourceString.empty());
                stream.writeString(cachedSourceString.c_str());
            }
        }
    }

    mProgram->save(context, &stream);
    ASSERT(mState.mExecutable->mPostLinkSubTasks.empty());

    if (!mBinary.resize(stream.length()))
    {
        ANGLE_PERF_WARNING(context->getState().getDebug(), GL_DEBUG_SEVERITY_LOW,
                           "Failed to allocate enough memory to serialize a program. (%zu bytes)",
                           stream.length());
        return angle::Result::Stop;
    }
    memcpy(mBinary.data(), stream.data(), stream.length());
    return angle::Result::Continue;
}

bool Program::deserialize(const Context *context, BinaryInputStream &stream)
{
    std::vector<uint8_t> angleShaderProgramVersionString(
        angle::GetANGLEShaderProgramVersionHashSize(), 0);
    stream.readBytes(angleShaderProgramVersionString.data(),
                     angleShaderProgramVersionString.size());
    if (memcmp(angleShaderProgramVersionString.data(), angle::GetANGLEShaderProgramVersion(),
               angleShaderProgramVersionString.size()) != 0)
    {
        mState.mInfoLog << "Invalid program binary version.";
        return false;
    }

    bool binaryIs64Bit = stream.readBool();
    if (binaryIs64Bit != angle::Is64Bit())
    {
        mState.mInfoLog << "cannot load program binaries across CPU architectures.";
        return false;
    }

    int angleSHVersion = stream.readInt<int>();
    if (angleSHVersion != angle::GetANGLESHVersion())
    {
        mState.mInfoLog << "cannot load program binaries across different angle sh version.";
        return false;
    }

    std::string rendererString = stream.readString();
    if (rendererString != context->getRendererString())
    {
        mState.mInfoLog << "Cannot load program binary due to changed renderer string.";
        return false;
    }

    int majorVersion = stream.readInt<int>();
    int minorVersion = stream.readInt<int>();
    if (majorVersion != context->getClientMajorVersion() ||
        minorVersion != context->getClientMinorVersion())
    {
        mState.mInfoLog << "Cannot load program binaries across different ES context versions.";
        return false;
    }

    mState.mSeparable                   = stream.readBool();
    mState.mTransformFeedbackBufferMode = stream.readInt<GLenum>();

    mState.mTransformFeedbackVaryingNames.resize(stream.readInt<size_t>());
    for (std::string &name : mState.mTransformFeedbackVaryingNames)
    {
        name = stream.readString();
    }

    // mSeparable must be before mExecutable->load(), since it uses the value.  This state is
    // duplicated in the executable for convenience.
    mState.mExecutable->mPod.isSeparable = mState.mSeparable;
    mState.mExecutable->load(&stream);

    static_assert(static_cast<unsigned long>(ShaderType::EnumCount) <= sizeof(unsigned long) * 8,
                  "Too many shader types");

    // Reject programs that use transform feedback varyings if the hardware cannot support them.
    if (mState.mExecutable->getLinkedTransformFeedbackVaryings().size() > 0 &&
        context->getFrontendFeatures().disableProgramCachingForTransformFeedback.enabled)
    {
        mState.mInfoLog << "Current driver does not support transform feedback in binary programs.";
        return false;
    }

    if (!mState.mAttachedShaders[ShaderType::Compute])
    {
        mState.mExecutable->updateTransformFeedbackStrides();
        mState.mExecutable->mTransformFeedbackVaryingNames = mState.mTransformFeedbackVaryingNames;
    }

    if (context->getShareGroup()->getFrameCaptureShared()->enabled())
    {
        // Extract the source for each stage from the program binary
        angle::ProgramSources sources;

        for (ShaderType shaderType : mState.mExecutable->getLinkedShaderStages())
        {
            std::string shaderSource = stream.readString();
            ASSERT(shaderSource.length() > 0);
            sources[shaderType] = std::move(shaderSource);
        }

        // Store it for use during mid-execution capture
        context->getShareGroup()->getFrameCaptureShared()->setProgramSources(id(),
                                                                             std::move(sources));
    }

    return true;
}

void Program::postResolveLink(const Context *context)
{
    mState.updateActiveSamplers();
    mState.mExecutable->mActiveImageShaderBits.fill({});
    mState.mExecutable->updateActiveImages(getExecutable());

    mState.mExecutable->initInterfaceBlockBindings();
    mState.mExecutable->setUniformValuesFromBindingQualifiers();

    if (context->getExtensions().multiDrawANGLE)
    {
        mState.mExecutable->mPod.drawIDLocation =
            mState.mExecutable->getUniformLocation("gl_DrawID").value;
    }

    if (context->getExtensions().baseVertexBaseInstanceShaderBuiltinANGLE)
    {
        mState.mExecutable->mPod.baseVertexLocation =
            mState.mExecutable->getUniformLocation("gl_BaseVertex").value;
        mState.mExecutable->mPod.baseInstanceLocation =
            mState.mExecutable->getUniformLocation("gl_BaseInstance").value;
    }
}

void Program::cacheProgramBinaryIfNotAlready(const Context *context)
{
    // If program caching is disabled, we already consider the binary cached.
    ASSERT(!context->getFrontendFeatures().disableProgramCaching.enabled || mIsBinaryCached);
    if (!mLinked || mIsBinaryCached || mState.mBinaryRetrieveableHint)
    {
        // Program caching is disabled, the program is yet to be linked, it's already cached, or the
        // application has specified that it prefers to cache the program binary itself.
        return;
    }

    // No post-link tasks should be pending.
    ASSERT(mState.mExecutable->mPostLinkSubTasks.empty());

    // Save to the program cache.
    std::lock_guard<angle::SimpleMutex> cacheLock(context->getProgramCacheMutex());
    MemoryProgramCache *cache = context->getMemoryProgramCache();
    // TODO: http://anglebug.com/42263141: Enable program caching for separable programs
    if (cache && !isSeparable() &&
        (mState.mExecutable->mLinkedTransformFeedbackVaryings.empty() ||
         !context->getFrontendFeatures().disableProgramCachingForTransformFeedback.enabled))
    {
        if (cache->putProgram(mProgramHash, context, this) == angle::Result::Stop)
        {
            // Don't fail linking if putting the program binary into the cache fails, the program is
            // still usable.
            ANGLE_PERF_WARNING(context->getState().getDebug(), GL_DEBUG_SEVERITY_LOW,
                               "Failed to save linked program to memory program cache.");
        }

        // Drop the binary; the application didn't specify that it wants to retrieve the binary.  If
        // it did, we wouldn't be implicitly caching it.
        mBinary.clear();
    }

    mIsBinaryCached = true;
}

void Program::dumpProgramInfo(const Context *context) const
{
    std::stringstream dumpStream;
    for (ShaderType shaderType : angle::AllEnums<ShaderType>())
    {
        Shader *shader = getAttachedShader(shaderType);
        if (shader)
        {
            dumpStream << shader->getType() << ": "
                       << GetShaderDumpFileName(shader->getSourceHash()) << std::endl;
        }
    }

    std::string dump = dumpStream.str();
    size_t dumpHash  = std::hash<std::string>{}(dump);

    std::stringstream pathStream;
    std::string shaderDumpDir = GetShaderDumpFileDirectory();
    if (!shaderDumpDir.empty())
    {
        pathStream << shaderDumpDir << "/";
    }
    pathStream << dumpHash << ".program";
    std::string path = pathStream.str();

    writeFile(path.c_str(), dump.c_str(), dump.length());
    INFO() << "Dumped program: " << path;
}
}  // namespace gl
