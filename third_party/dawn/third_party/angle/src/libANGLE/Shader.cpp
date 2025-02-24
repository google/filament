//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Shader.cpp: Implements the gl::Shader class and its  derived classes
// VertexShader and FragmentShader. Implements GL shader objects and related
// functionality. [OpenGL ES 2.0.24] section 2.10 page 24 and section 3.8 page 84.

#include "libANGLE/Shader.h"

#include <functional>
#include <sstream>

#include "GLSLANG/ShaderLang.h"
#include "common/angle_version_info.h"
#include "common/string_utils.h"
#include "common/system_utils.h"
#include "common/utilities.h"
#include "libANGLE/Caps.h"
#include "libANGLE/Compiler.h"
#include "libANGLE/Constants.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/MemoryShaderCache.h"
#include "libANGLE/Program.h"
#include "libANGLE/ResourceManager.h"
#include "libANGLE/renderer/GLImplFactory.h"
#include "libANGLE/renderer/ShaderImpl.h"
#include "libANGLE/trace.h"
#include "platform/autogen/FrontendFeatures_autogen.h"

namespace gl
{

namespace
{
constexpr uint32_t kShaderCacheIdentifier = 0x12345678;

// Environment variable (and associated Android property) for the path to read and write shader
// dumps
constexpr char kShaderDumpPathVarName[]       = "ANGLE_SHADER_DUMP_PATH";
constexpr char kEShaderDumpPathPropertyName[] = "debug.angle.shader_dump_path";

size_t ComputeShaderHash(const std::string &mergedSource)
{
    return std::hash<std::string>{}(mergedSource);
}

std::string GetShaderDumpFilePath(size_t shaderHash, const char *suffix)
{
    std::stringstream path;
    std::string shaderDumpDir = GetShaderDumpFileDirectory();
    if (!shaderDumpDir.empty())
    {
        path << shaderDumpDir << "/";
    }
    path << shaderHash << "." << suffix;

    return path.str();
}

class CompileTask final : public angle::Closure
{
  public:
    // Translate and compile
    CompileTask(const angle::FrontendFeatures &frontendFeatures,
                ShHandle compilerHandle,
                ShShaderOutput outputType,
                const ShCompileOptions &options,
                const std::string &source,
                size_t sourceHash,
                const SharedCompiledShaderState &compiledState,
                size_t maxComputeWorkGroupInvocations,
                size_t maxComputeSharedMemory,
                std::shared_ptr<rx::ShaderTranslateTask> &&translateTask)
        : mFrontendFeatures(frontendFeatures),
          mMaxComputeWorkGroupInvocations(maxComputeWorkGroupInvocations),
          mMaxComputeSharedMemory(maxComputeSharedMemory),
          mCompilerHandle(compilerHandle),
          mOutputType(outputType),
          mOptions(options),
          mSource(source),
          mSourceHash(sourceHash),
          mCompiledState(compiledState),
          mTranslateTask(std::move(translateTask))
    {}

    // Load from binary
    CompileTask(const angle::FrontendFeatures &frontendFeatures,
                const SharedCompiledShaderState &compiledState,
                std::shared_ptr<rx::ShaderTranslateTask> &&translateTask)
        : mFrontendFeatures(frontendFeatures),
          mCompiledState(compiledState),
          mTranslateTask(std::move(translateTask))
    {}
    ~CompileTask() override = default;

    void operator()() override { mResult = compileImpl(); }

    angle::Result getResult()
    {
        // Note: this function is called from WaitCompileJobUnlocked(), and must therefore be
        // thread-safe if the linkJobIsThreadSafe feature is enabled.  Without linkJobIsThreadSafe,
        // the call will end up done in the main thread, which is the case for the GL backend (which
        // happens to be the only backend that actually does anything in getResult).
        //
        // Consequently, this function must not _write_ to anything, e.g. by trying to cache the
        // result of |mTranslateTask->getResult()|.
        ANGLE_TRY(mResult);
        ANGLE_TRY(mTranslateTask->getResult(mInfoLog));

        return angle::Result::Continue;
    }

    bool isCompilingInternally() { return mTranslateTask->isCompilingInternally(); }

    std::string &&getInfoLog() { return std::move(mInfoLog); }

  private:
    angle::Result compileImpl();
    angle::Result postTranslate();

    // Global constants that are safe to access by the worker thread
    const angle::FrontendFeatures &mFrontendFeatures;
    size_t mMaxComputeWorkGroupInvocations = 0;
    size_t mMaxComputeSharedMemory         = 0;

    // Access to the compile information.  Note that the compiler instance is kept alive until
    // resolveCompile.
    ShHandle mCompilerHandle = 0;
    ShShaderOutput mOutputType;
    ShCompileOptions mOptions;
    const std::string mSource;
    size_t mSourceHash = 0;
    SharedCompiledShaderState mCompiledState;

    std::shared_ptr<rx::ShaderTranslateTask> mTranslateTask;
    angle::Result mResult;
    std::string mInfoLog;
};

class CompileEvent final
{
  public:
    CompileEvent(const std::shared_ptr<CompileTask> &compileTask,
                 const std::shared_ptr<angle::WaitableEvent> &waitEvent)
        : mCompileTask(compileTask), mWaitableEvent(waitEvent)
    {}
    ~CompileEvent() = default;

    angle::Result wait()
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "CompileEvent::wait");

        mWaitableEvent->wait();

        return mCompileTask->getResult();
    }
    bool isCompiling()
    {
        return !mWaitableEvent->isReady() || mCompileTask->isCompilingInternally();
    }

    std::string &&getInfoLog() { return std::move(mCompileTask->getInfoLog()); }

  private:
    std::shared_ptr<CompileTask> mCompileTask;
    std::shared_ptr<angle::WaitableEvent> mWaitableEvent;
};

angle::Result CompileTask::compileImpl()
{
    if (mCompilerHandle)
    {
        // Compiling from source

        // Call the translator and get the info log
        bool result = mTranslateTask->translate(mCompilerHandle, mOptions, mSource);
        mInfoLog    = sh::GetInfoLog(mCompilerHandle);
        if (!result)
        {
            return angle::Result::Stop;
        }

        // Process the translation results itself; gather compilation info, substitute the shader if
        // being overriden, etc.
        return postTranslate();
    }
    else
    {
        // Loading from binary
        mTranslateTask->load(*mCompiledState.get());
        return angle::Result::Continue;
    }
}

angle::Result CompileTask::postTranslate()
{
    const bool isBinaryOutput = mOutputType == SH_SPIRV_VULKAN_OUTPUT;
    mCompiledState->buildCompiledShaderState(mCompilerHandle, isBinaryOutput);

    ASSERT(!mCompiledState->translatedSource.empty() || !mCompiledState->compiledBinary.empty());

    // Validation checks for compute shaders
    if (mCompiledState->shaderType == ShaderType::Compute && mCompiledState->localSize.isDeclared())
    {
        angle::CheckedNumeric<size_t> checked_local_size_product(mCompiledState->localSize[0]);
        checked_local_size_product *= mCompiledState->localSize[1];
        checked_local_size_product *= mCompiledState->localSize[2];

        if (!checked_local_size_product.IsValid() ||
            checked_local_size_product.ValueOrDie() > mMaxComputeWorkGroupInvocations)
        {
            mInfoLog +=
                "\nThe total number of invocations within a work group exceeds "
                "MAX_COMPUTE_WORK_GROUP_INVOCATIONS.";
            return angle::Result::Stop;
        }
    }

    unsigned int sharedMemSize = sh::GetShaderSharedMemorySize(mCompilerHandle);
    if (sharedMemSize > mMaxComputeSharedMemory)
    {
        mInfoLog += "\nShared memory size exceeds GL_MAX_COMPUTE_SHARED_MEMORY_SIZE";
        return angle::Result::Stop;
    }

    bool substitutedTranslatedShader = false;
    const char *suffix               = "translated";
    if (mFrontendFeatures.enableTranslatedShaderSubstitution.enabled)
    {
        // To support reading/writing compiled binaries (SPIR-V representation), need more file
        // input/output facilities, and figure out the byte ordering of writing the 32-bit words to
        // disk.
        if (isBinaryOutput)
        {
            INFO() << "Can not substitute compiled binary (SPIR-V) shaders yet";
        }
        else
        {
            std::string substituteShaderPath = GetShaderDumpFilePath(mSourceHash, suffix);

            std::string substituteShader;
            if (angle::ReadFileToString(substituteShaderPath, &substituteShader))
            {
                mCompiledState->translatedSource = std::move(substituteShader);
                substitutedTranslatedShader      = true;
                INFO() << "Translated shader substitute found, loading from "
                       << substituteShaderPath;
            }
        }
    }

    // Only dump translated shaders that have not been previously substituted. It would write the
    // same data back to the file.
    if (mFrontendFeatures.dumpTranslatedShaders.enabled && !substitutedTranslatedShader)
    {
        if (isBinaryOutput)
        {
            INFO() << "Can not dump compiled binary (SPIR-V) shaders yet";
        }
        else
        {
            std::string dumpFile = GetShaderDumpFilePath(mSourceHash, suffix);

            const std::string &translatedSource = mCompiledState->translatedSource;
            writeFile(dumpFile.c_str(), translatedSource.c_str(), translatedSource.length());
            INFO() << "Dumped translated source: " << dumpFile;
        }
    }

#if defined(ANGLE_ENABLE_ASSERTS)
    if (!isBinaryOutput)
    {
        // Suffix the translated shader with commented out un-translated shader.
        // Useful in diagnostics tools which capture the shader source.
        std::ostringstream shaderStream;
        shaderStream << "\n";
        shaderStream << "// GLSL\n";
        shaderStream << "//\n";

        std::istringstream inputSourceStream(mSource);
        std::string line;
        while (std::getline(inputSourceStream, line))
        {
            // Remove null characters from the source line
            line.erase(std::remove(line.begin(), line.end(), '\0'), line.end());

            shaderStream << "// " << line;

            // glslang complains if a comment ends with backslash
            if (!line.empty() && line.back() == '\\')
            {
                shaderStream << "\\";
            }

            shaderStream << std::endl;
        }
        mCompiledState->translatedSource += shaderStream.str();
    }
#endif  // defined(ANGLE_ENABLE_ASSERTS)

    // Let the backend process the result of the compilation.  For the GL backend, this means
    // kicking off compilation internally.  Some of the other backends fill in their internal
    // "compiled state" at this point.
    mTranslateTask->postTranslate(mCompilerHandle, *mCompiledState.get());

    return angle::Result::Continue;
}

template <typename T>
void AppendHashValue(angle::base::SecureHashAlgorithm &hasher, T value)
{
    static_assert(std::is_fundamental<T>::value || std::is_enum<T>::value);
    hasher.Update(&value, sizeof(T));
}

angle::JobThreadSafety GetTranslateTaskThreadSafety(const Context *context)
{
    // The GL backend relies on the driver's internal parallel compilation, and thus does not use a
    // thread to compile.  A front-end feature selects whether the single-threaded pool must be
    // used.
    return context->getFrontendFeatures().compileJobIsThreadSafe.enabled
               ? angle::JobThreadSafety::Safe
               : angle::JobThreadSafety::Unsafe;
}

}  // anonymous namespace

const char *GetShaderTypeString(ShaderType type)
{
    switch (type)
    {
        case ShaderType::Vertex:
            return "VERTEX";

        case ShaderType::Fragment:
            return "FRAGMENT";

        case ShaderType::Compute:
            return "COMPUTE";

        case ShaderType::Geometry:
            return "GEOMETRY";

        case ShaderType::TessControl:
            return "TESS_CONTROL";

        case ShaderType::TessEvaluation:
            return "TESS_EVALUATION";

        default:
            UNREACHABLE();
            return "";
    }
}

std::string GetShaderDumpFileDirectory()
{
    // Check the environment variable for the path to save and read shader dump files.
    std::string environmentVariableDumpDir =
        angle::GetAndSetEnvironmentVarOrUnCachedAndroidProperty(kShaderDumpPathVarName,
                                                                kEShaderDumpPathPropertyName);
    if (!environmentVariableDumpDir.empty() && environmentVariableDumpDir.compare("0") != 0)
    {
        return environmentVariableDumpDir;
    }

    // Fall back to the temp dir. If that doesn't exist, use the current working directory.
    return angle::GetTempDirectory().valueOr("");
}

std::string GetShaderDumpFileName(size_t shaderHash)
{
    std::stringstream name;
    name << shaderHash << ".essl";
    return name.str();
}

struct CompileJob
{
    virtual ~CompileJob() = default;
    virtual bool wait() { return compileEvent->wait() == angle::Result::Continue; }

    std::unique_ptr<CompileEvent> compileEvent;
    ShCompilerInstance shCompilerInstance;
};

struct CompileJobDone final : public CompileJob
{
    CompileJobDone(bool compiledIn) : compiled(compiledIn) {}
    bool wait() override { return compiled; }

    bool compiled;
};

ShaderState::ShaderState(ShaderType shaderType)
    : mCompiledState(std::make_shared<CompiledShaderState>(shaderType))
{}

ShaderState::~ShaderState() {}

Shader::Shader(ShaderProgramManager *manager,
               rx::GLImplFactory *implFactory,
               const gl::Limitations &rendererLimitations,
               ShaderType type,
               ShaderProgramID handle)
    : mState(type),
      mImplementation(implFactory->createShader(mState)),
      mRendererLimitations(rendererLimitations),
      mHandle(handle),
      mRefCount(0),
      mDeleteStatus(false),
      mResourceManager(manager)
{
    ASSERT(mImplementation);

    mShaderHash = {0};
}

void Shader::onDestroy(const gl::Context *context)
{
    resolveCompile(context);
    mImplementation->onDestroy(context);
    mBoundCompiler.set(context, nullptr);
    mImplementation.reset(nullptr);
    delete this;
}

Shader::~Shader()
{
    ASSERT(!mImplementation);
}

angle::Result Shader::setLabel(const Context *context, const std::string &label)
{
    mState.mLabel = label;

    if (mImplementation)
    {
        return mImplementation->onLabelUpdate(context);
    }
    return angle::Result::Continue;
}

const std::string &Shader::getLabel() const
{
    return mState.mLabel;
}

ShaderProgramID Shader::getHandle() const
{
    return mHandle;
}

std::string Shader::joinShaderSources(GLsizei count, const char *const *string, const GLint *length)
{
    // Fast path for the most common case.
    if (count == 1)
    {
        if (length == nullptr || length[0] < 0)
            return std::string(string[0]);
        else
            return std::string(string[0], static_cast<size_t>(length[0]));
    }

    // Start with totalLength of 1 to reserve space for the null terminator
    size_t totalLength = 1;

    // First pass, calculate the total length of the joined string
    for (GLsizei i = 0; i < count; ++i)
    {
        if (length == nullptr || length[i] < 0)
            totalLength += std::strlen(string[i]);
        else
            totalLength += static_cast<size_t>(length[i]);
    }

    // Second pass, allocate the string and concatenate each shader source
    // fragment
    std::string joinedString;
    joinedString.reserve(totalLength);
    for (GLsizei i = 0; i < count; ++i)
    {
        if (length == nullptr || length[i] < 0)
            joinedString.append(string[i]);
        else
            joinedString.append(string[i], static_cast<size_t>(length[i]));
    }

    return joinedString;
}

void Shader::setSource(const Context *context,
                       GLsizei count,
                       const char *const *string,
                       const GLint *length)
{
    std::string source = joinShaderSources(count, string, length);

    // Compute the hash based on the original source before any substitutions
    size_t sourceHash = ComputeShaderHash(source);

    const angle::FrontendFeatures &frontendFeatures = context->getFrontendFeatures();

    bool substitutedShader = false;
    const char *suffix     = "essl";
    if (frontendFeatures.enableShaderSubstitution.enabled)
    {
        std::string subsitutionShaderPath = GetShaderDumpFilePath(sourceHash, suffix);

        std::string substituteShader;
        if (angle::ReadFileToString(subsitutionShaderPath, &substituteShader))
        {
            source            = std::move(substituteShader);
            substitutedShader = true;
            INFO() << "Shader substitute found, loading from " << subsitutionShaderPath;
        }
    }

    // Only dump shaders that have not been previously substituted. It would write the same data
    // back to the file.
    if (frontendFeatures.dumpShaderSource.enabled && !substitutedShader)
    {
        std::string dumpFile = GetShaderDumpFilePath(sourceHash, suffix);

        writeFile(dumpFile.c_str(), source.c_str(), source.length());
        INFO() << "Dumped shader source: " << dumpFile;
    }

    mState.mSource     = std::move(source);
    mState.mSourceHash = sourceHash;
}

int Shader::getInfoLogLength(const Context *context)
{
    resolveCompile(context);
    if (mInfoLog.empty())
    {
        return 0;
    }

    return (static_cast<int>(mInfoLog.length()) + 1);
}

void Shader::getInfoLog(const Context *context, GLsizei bufSize, GLsizei *length, char *infoLog)
{
    resolveCompile(context);

    int index = 0;

    if (bufSize > 0)
    {
        index = std::min(bufSize - 1, static_cast<GLsizei>(mInfoLog.length()));
        memcpy(infoLog, mInfoLog.c_str(), index);

        infoLog[index] = '\0';
    }

    if (length)
    {
        *length = index;
    }
}

int Shader::getSourceLength() const
{
    return mState.mSource.empty() ? 0 : (static_cast<int>(mState.mSource.length()) + 1);
}

int Shader::getTranslatedSourceLength(const Context *context)
{
    resolveCompile(context);

    if (mState.mCompiledState->translatedSource.empty())
    {
        return 0;
    }

    return static_cast<int>(mState.mCompiledState->translatedSource.length()) + 1;
}

int Shader::getTranslatedSourceWithDebugInfoLength(const Context *context)
{
    resolveCompile(context);

    const std::string &debugInfo = mImplementation->getDebugInfo();
    if (debugInfo.empty())
    {
        return 0;
    }

    return (static_cast<int>(debugInfo.length()) + 1);
}

// static
void Shader::GetSourceImpl(const std::string &source,
                           GLsizei bufSize,
                           GLsizei *length,
                           char *buffer)
{
    int index = 0;

    if (bufSize > 0)
    {
        index = std::min(bufSize - 1, static_cast<GLsizei>(source.length()));
        memcpy(buffer, source.c_str(), index);

        buffer[index] = '\0';
    }

    if (length)
    {
        *length = index;
    }
}

void Shader::getSource(GLsizei bufSize, GLsizei *length, char *buffer) const
{
    GetSourceImpl(mState.mSource, bufSize, length, buffer);
}

void Shader::getTranslatedSource(const Context *context,
                                 GLsizei bufSize,
                                 GLsizei *length,
                                 char *buffer)
{
    GetSourceImpl(getTranslatedSource(context), bufSize, length, buffer);
}

const std::string &Shader::getTranslatedSource(const Context *context)
{
    resolveCompile(context);
    return mState.mCompiledState->translatedSource;
}

size_t Shader::getSourceHash() const
{
    return mState.mSourceHash;
}

void Shader::getTranslatedSourceWithDebugInfo(const Context *context,
                                              GLsizei bufSize,
                                              GLsizei *length,
                                              char *buffer)
{
    resolveCompile(context);
    const std::string &debugInfo = mImplementation->getDebugInfo();
    GetSourceImpl(debugInfo, bufSize, length, buffer);
}

void Shader::compile(const Context *context, angle::JobResultExpectancy resultExpectancy)
{
    resolveCompile(context);

    // Create a new compiled shader state.  If any programs are currently linking using this shader,
    // they would use the old compiled state, and this shader is free to recompile in the meantime.
    mState.mCompiledState = std::make_shared<CompiledShaderState>(mState.getShaderType());

    mInfoLog.clear();

    ShCompileOptions options = {};
    options.objectCode       = true;
    options.emulateGLDrawID  = true;

    // Add default options to WebGL shaders to prevent unexpected behavior during
    // compilation.
    if (context->isWebGL())
    {
        options.initGLPosition             = true;
        options.limitCallStackDepth        = true;
        options.limitExpressionComplexity  = true;
        options.enforcePackingRestrictions = true;
        options.initSharedVariables        = true;

        if (context->getFrontendFeatures().rejectWebglShadersWithUndefinedBehavior.enabled)
        {
            options.rejectWebglShadersWithUndefinedBehavior = true;
        }
    }
    else
    {
        // Per https://github.com/KhronosGroup/WebGL/pull/3278 gl_BaseVertex/gl_BaseInstance are
        // removed from WebGL
        options.emulateGLBaseVertexBaseInstance = true;
    }

    if (context->getFrontendFeatures().forceInitShaderVariables.enabled)
    {
        options.initOutputVariables           = true;
        options.initializeUninitializedLocals = true;
    }

#if defined(ANGLE_ENABLE_ASSERTS)
    options.validateAST = true;
#endif

    // Find a shader in Blob Cache
    Compiler *compiler = context->getCompiler();
    setShaderKey(context, options, compiler->getShaderOutputType(),
                 compiler->getBuiltInResources());
    ASSERT(!mShaderHash.empty());
    MemoryShaderCache *shaderCache = context->getMemoryShaderCache();
    if (shaderCache != nullptr)
    {
        egl::CacheGetResult result =
            shaderCache->getShader(context, this, mShaderHash, resultExpectancy);
        switch (result)
        {
            case egl::CacheGetResult::Success:
                return;
            case egl::CacheGetResult::Rejected:
                // Reset the state
                mState.mCompiledState =
                    std::make_shared<CompiledShaderState>(mState.getShaderType());
                break;
            case egl::CacheGetResult::NotFound:
            default:
                break;
        }
    }

    mBoundCompiler.set(context, compiler);
    ASSERT(mBoundCompiler.get());

    ShCompilerInstance compilerInstance = mBoundCompiler->getInstance(mState.getShaderType());
    ShHandle compilerHandle             = compilerInstance.getHandle();
    ASSERT(compilerHandle);

    // Cache load failed, fall through normal compiling.
    mState.mCompileStatus = CompileStatus::COMPILE_REQUESTED;

    // Ask the backend to prepare the translate task
    std::shared_ptr<rx::ShaderTranslateTask> translateTask =
        mImplementation->compile(context, &options);

    // Prepare the complete compile task
    const size_t maxComputeWorkGroupInvocations =
        static_cast<size_t>(context->getCaps().maxComputeWorkGroupInvocations);
    const size_t maxComputeSharedMemory = context->getCaps().maxComputeSharedMemorySize;

    std::shared_ptr<CompileTask> compileTask(
        new CompileTask(context->getFrontendFeatures(), compilerInstance.getHandle(),
                        compilerInstance.getShaderOutputType(), options, mState.mSource,
                        mState.mSourceHash, mState.mCompiledState, maxComputeWorkGroupInvocations,
                        maxComputeSharedMemory, std::move(translateTask)));

    // The GL backend relies on the driver's internal parallel compilation, and thus does not use a
    // thread to compile.  A front-end feature selects whether the single-threaded pool must be
    // used.
    const angle::JobThreadSafety threadSafety =
        context->getFrontendFeatures().compileJobIsThreadSafe.enabled
            ? angle::JobThreadSafety::Safe
            : angle::JobThreadSafety::Unsafe;
    std::shared_ptr<angle::WaitableEvent> compileEvent =
        context->postCompileLinkTask(compileTask, threadSafety, resultExpectancy);

    mCompileJob                     = std::make_shared<CompileJob>();
    mCompileJob->shCompilerInstance = std::move(compilerInstance);
    mCompileJob->compileEvent       = std::make_unique<CompileEvent>(compileTask, compileEvent);
}

void Shader::resolveCompile(const Context *context)
{
    if (!mState.compilePending())
    {
        return;
    }

    ASSERT(mCompileJob.get());
    mState.mCompileStatus = CompileStatus::IS_RESOLVING;

    const bool success    = WaitCompileJobUnlocked(mCompileJob);
    mInfoLog              = std::move(mCompileJob->compileEvent->getInfoLog());
    mState.mCompileStatus = success ? CompileStatus::COMPILED : CompileStatus::NOT_COMPILED;

    if (mCompileJob->shCompilerInstance.getHandle())
    {
        // Only save this shader to the cache if it was a compile from source (not load from binary)
        if (success)
        {
            MemoryShaderCache *shaderCache = context->getMemoryShaderCache();
            if (shaderCache != nullptr)
            {
                // Save to the shader cache.
                if (shaderCache->putShader(context, mShaderHash, this) != angle::Result::Continue)
                {
                    ANGLE_PERF_WARNING(context->getState().getDebug(), GL_DEBUG_SEVERITY_LOW,
                                       "Failed to save compiled shader to memory shader cache.");
                }
            }
        }

        mBoundCompiler->putInstance(std::move(mCompileJob->shCompilerInstance));
    }
    mCompileJob.reset();
}

void Shader::addRef()
{
    mRefCount++;
}

void Shader::release(const Context *context)
{
    mRefCount--;

    if (mRefCount == 0 && mDeleteStatus)
    {
        mResourceManager->deleteShader(context, mHandle);
    }
}

unsigned int Shader::getRefCount() const
{
    return mRefCount;
}

bool Shader::isFlaggedForDeletion() const
{
    return mDeleteStatus;
}

void Shader::flagForDeletion()
{
    mDeleteStatus = true;
}

bool Shader::isCompiled(const Context *context)
{
    resolveCompile(context);
    return mState.mCompileStatus == CompileStatus::COMPILED;
}

bool Shader::isCompleted()
{
    return !mState.compilePending() || !mCompileJob->compileEvent->isCompiling();
}

SharedCompileJob Shader::getCompileJob(SharedCompiledShaderState *compiledStateOut)
{
    // mState.mCompiledState is the same as the one in the current compile job, because this call is
    // made during link which expects to pick up the currently compiled (or pending compilation)
    // state.
    *compiledStateOut = mState.mCompiledState;

    if (mCompileJob)
    {
        ASSERT(mState.compilePending());
        return mCompileJob;
    }

    ASSERT(!mState.compilePending());
    ASSERT(mState.mCompileStatus == CompileStatus::COMPILED ||
           mState.mCompileStatus == CompileStatus::NOT_COMPILED);

    return std::make_shared<CompileJobDone>(mState.mCompileStatus == CompileStatus::COMPILED);
}

angle::Result Shader::serialize(const Context *context, angle::MemoryBuffer *binaryOut) const
{
    BinaryOutputStream stream;

    stream.writeInt(kShaderCacheIdentifier);
    mState.mCompiledState->serialize(stream);

    ASSERT(binaryOut);
    if (!binaryOut->resize(stream.length()))
    {
        ANGLE_PERF_WARNING(context->getState().getDebug(), GL_DEBUG_SEVERITY_LOW,
                           "Failed to allocate enough memory to serialize a shader. (%zu bytes)",
                           stream.length());
        return angle::Result::Stop;
    }

    memcpy(binaryOut->data(), stream.data(), stream.length());

    return angle::Result::Continue;
}

bool Shader::deserialize(BinaryInputStream &stream)
{
    mState.mCompiledState->deserialize(stream);

    if (stream.error())
    {
        // Error while deserializing binary stream
        return false;
    }

    // Note: Currently, shader binaries are only supported on backends that don't happen to have any
    // additional state used at link time.  If other backends implement this functionality, this
    // function should call into the backend object to deserialize their part.

    return true;
}

bool Shader::loadBinary(const Context *context,
                        const void *binary,
                        GLsizei length,
                        angle::JobResultExpectancy resultExpectancy)
{
    return loadBinaryImpl(context, binary, length, resultExpectancy, false);
}

bool Shader::loadShaderBinary(const Context *context,
                              const void *binary,
                              GLsizei length,
                              angle::JobResultExpectancy resultExpectancy)
{
    return loadBinaryImpl(context, binary, length, resultExpectancy, true);
}

bool Shader::loadBinaryImpl(const Context *context,
                            const void *binary,
                            GLsizei length,
                            angle::JobResultExpectancy resultExpectancy,
                            bool generatedWithOfflineCompiler)
{
    BinaryInputStream stream(binary, length);

    mState.mCompiledState = std::make_shared<CompiledShaderState>(mState.getShaderType());

    // Shader binaries generated with offline compiler have additional fields
    if (generatedWithOfflineCompiler)
    {
        // Load binary from a glShaderBinary call.
        // Validation layer should have already verified that the shader program version and shader
        // type match
        std::vector<uint8_t> commitString(angle::GetANGLEShaderProgramVersionHashSize(), 0);
        stream.readBytes(commitString.data(), commitString.size());
        ASSERT(memcmp(commitString.data(), angle::GetANGLEShaderProgramVersion(),
                      commitString.size()) == 0);

        gl::ShaderType shaderType;
        stream.readEnum(&shaderType);
        ASSERT(mState.getShaderType() == shaderType);

        // Get fields needed to generate the key for memory caches.
        ShShaderOutput outputType;
        stream.readEnum<ShShaderOutput>(&outputType);

        // Get the shader's source string.
        mState.mSource = stream.readString();

        // In the absence of element-by-element serialize/deserialize functions, read
        // ShCompileOptions and ShBuiltInResources as raw binary blobs.
        ShCompileOptions compileOptions;
        stream.readBytes(reinterpret_cast<uint8_t *>(&compileOptions), sizeof(ShCompileOptions));

        ShBuiltInResources resources;
        stream.readBytes(reinterpret_cast<uint8_t *>(&resources), sizeof(ShBuiltInResources));

        setShaderKey(context, compileOptions, outputType, resources);
    }
    else
    {
        // Load binary from shader cache.
        if (stream.readInt<uint32_t>() != kShaderCacheIdentifier)
        {
            return false;
        }
    }

    if (!deserialize(stream))
    {
        return false;
    }

    mState.mCompileStatus = CompileStatus::COMPILE_REQUESTED;

    // Ask the backend to prepare the translate task
    std::shared_ptr<rx::ShaderTranslateTask> translateTask =
        mImplementation->load(context, &stream);

    std::shared_ptr<CompileTask> compileTask(new CompileTask(
        context->getFrontendFeatures(), mState.mCompiledState, std::move(translateTask)));

    const angle::JobThreadSafety threadSafety = GetTranslateTaskThreadSafety(context);
    std::shared_ptr<angle::WaitableEvent> compileEvent =
        context->postCompileLinkTask(compileTask, threadSafety, resultExpectancy);

    mCompileJob               = std::make_shared<CompileJob>();
    mCompileJob->compileEvent = std::make_unique<CompileEvent>(compileTask, compileEvent);

    return true;
}

void Shader::setShaderKey(const Context *context,
                          const ShCompileOptions &compileOptions,
                          const ShShaderOutput &outputType,
                          const ShBuiltInResources &resources)
{
    // Compute shader key.
    angle::base::SecureHashAlgorithm hasher;
    hasher.Init();

    // Start with the shader type and source.
    AppendHashValue(hasher, mState.getShaderType());
    hasher.Update(mState.getSource().c_str(), mState.getSource().length());

    // Include the shader program version hash.
    hasher.Update(angle::GetANGLEShaderProgramVersion(),
                  angle::GetANGLEShaderProgramVersionHashSize());

    AppendHashValue(hasher, Compiler::SelectShaderSpec(context->getState()));
    AppendHashValue(hasher, outputType);
    hasher.Update(reinterpret_cast<const uint8_t *>(&compileOptions), sizeof(compileOptions));

    // Include the ShBuiltInResources, which represent the extensions and constants used by the
    // shader.
    hasher.Update(reinterpret_cast<const uint8_t *>(&resources), sizeof(resources));

    // Call the secure SHA hashing function.
    hasher.Final();
    memcpy(mShaderHash.data(), hasher.Digest(), angle::base::kSHA1Length);
}

bool WaitCompileJobUnlocked(const SharedCompileJob &compileJob)
{
    // Simply wait for the job and return whether it succeeded.  Do nothing more as this can be
    // called from multiple threads.  Caching of the shader results and compiler clean up will be
    // done in resolveCompile() when the main thread happens to call it.
    return compileJob->wait();
}
}  // namespace gl
