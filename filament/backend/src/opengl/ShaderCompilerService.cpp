/*
 * Copyright (C) 2023 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ShaderCompilerService.h"

#include "BlobCacheKey.h"
#include "CallbackManager.h"
#include "CompilerThreadPool.h"
#include "OpenGLBlobCache.h"
#include "OpenGLDriver.h"

#include <iterator>
#include <private/backend/BackendUtils.h>

#include <backend/DriverEnums.h>
#include <backend/Program.h>

#include <utils/compiler.h>
#include <utils/CString.h>
#include <utils/debug.h>
#include <utils/FixedCapacityVector.h>
#include <utils/JobSystem.h>
#include <utils/ostream.h>
#include <utils/Panic.h>
#include <utils/Systrace.h>

#include <absl/log/log.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <mutex>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <variant>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace filament::backend {

using namespace utils;

// ------------------------------------------------------------------------------------------------

static std::string to_string(bool const b) { return b ? "true" : "false"; }
static std::string to_string(int const i) { return std::to_string(i); }
static std::string to_string(float const f) { return "float(" + std::to_string(f) + ")"; }

static void logCompilationError(ShaderStage shaderType, const char* name, GLuint shaderId,
        CString const& sourceCode) noexcept;
static void logProgramLinkError(char const* name, GLuint program) noexcept;

static void process_GOOGLE_cpp_style_line_directive(OpenGLContext const& context, char* source,
        size_t len) noexcept;
static void process_OVR_multiview2(OpenGLContext const& context, int32_t eyeCount, char* source,
        size_t len) noexcept;
static std::string_view process_ARB_shading_language_packing(OpenGLContext& context) noexcept;
static std::array<std::string_view, 3> splitShaderSource(std::string_view source) noexcept;

// ------------------------------------------------------------------------------------------------

struct ShaderCompilerService::OpenGLProgramToken : ProgramToken {
    ~OpenGLProgramToken() override;

    OpenGLProgramToken(ShaderCompilerService& compiler, CString const& name) noexcept
            : compiler(compiler), name(name), handle(compiler.issueCallbackHandle()) {
    }

    ShaderCompilerService& compiler;
    CString const& name;
    FixedCapacityVector<std::pair<CString, uint8_t>> attributes;
    shaders_source_t shaderSourceCode;
    void* user = nullptr;
    struct {
        shaders_t shaders{};
        GLuint program = 0;
    } gl; // 12 bytes

    // Used in THREAD_POOL mode. The job from ThreadPool should call this when the token is ready to
    // be used. It sends a signal to the engine thread being blocked upon the `wait` call, so that
    // the engine thread resumes its processing with the token.
    void signal() noexcept {
        std::unique_lock const l(lock);
        signaled = true;
        cond.notify_one();
    }

    // Used in THREAD_POOL mode. The engine thread should call this before accessing token's fields.
    // This may block until the token is ready to be used.
    void wait() const noexcept {
        std::unique_lock l(lock);
        cond.wait(l, [this] { return signaled; });
    }

    CallbackManager::Handle handle{};
    BlobCacheKey key;

    // Used for the `THREAD_POOL` mode.
    mutable Mutex lock;
    mutable Condition cond;
    bool signaled = false;
};

ShaderCompilerService::OpenGLProgramToken::~OpenGLProgramToken() {
    compiler.submitCallbackHandle(handle);
}

/* static */ void ShaderCompilerService::setUserData(const program_token_t& token,
        void* user) noexcept {
    token->user = user;
}

/* static */ void* ShaderCompilerService::getUserData(const program_token_t& token) noexcept {
    return token->user;
}

// ------------------------------------------------------------------------------------------------

ShaderCompilerService::ShaderCompilerService(OpenGLDriver& driver)
        : mDriver(driver),
          mBlobCache(driver.getContext()),
          mCallbackManager(driver) {
}

ShaderCompilerService::~ShaderCompilerService() noexcept = default;

bool ShaderCompilerService::isParallelShaderCompileSupported() const noexcept {
    assert_invariant(mMode != Mode::UNDEFINED);
    return mMode != Mode::SYNCHRONOUS;
}

void ShaderCompilerService::init() noexcept {
    if (UTILS_UNLIKELY(mDriver.getDriverConfig().disableParallelShaderCompile)) {
        // user disabled parallel shader compile
        mMode = Mode::SYNCHRONOUS;
        return;
    }

    // Here we decide which mode we'll be using. We always prefer our own thread-pool if
    // that mode is available because, we have no control on how the compilation queues are
    // handled if done by the driver (so at the very least we'd need to decode this per-driver).
    // In theory, using Mode::ASYNCHRONOUS (a.k.a. KHR_parallel_shader_compile) could be more
    // efficient, since it doesn't require shared contexts.
    // In practice, we already know that with ANGLE, Mode::ASYNCHRONOUS can cause very long
    // pauses at glDraw() time in the situation where glLinkProgram() has been emitted, but has
    // other programs ahead of it in ANGLE's queue.
    if (mDriver.mPlatform.isExtraContextSupported()) {
        // our thread-pool if possible
        mMode = Mode::THREAD_POOL;
    } else if (mDriver.getContext().ext.KHR_parallel_shader_compile) {
        // if not, async shader compilation and link if the driver supports it
        mMode = Mode::ASYNCHRONOUS;
    } else {
        // fallback to synchronous shader compilation
        mMode = Mode::SYNCHRONOUS;
    }

    if (mMode == Mode::THREAD_POOL) {
        // - on Adreno there is a single compiler object. We can't use a pool > 1
        //   also glProgramBinary blocks if other threads are compiling.
        // - on Mali shader compilation can be multithreaded, but program linking happens on
        //   a single service thread, so we don't bother using more than one thread either.
        // - on PowerVR shader compilation and linking can be multithreaded.
        //   How many threads should we use?
        // - on macOS (M1 MacBook Pro/Ventura) there is global lock around all GL APIs when using
        //   a shared context, so parallel shader compilation yields no benefit.
        // - on windows/linux we could use more threads, tbd.

        // By default, we use one thread at the same priority as the gl thread. This is the
        // safest choice that avoids priority inversions.
        uint32_t poolSize = 1;
        JobSystem::Priority priority = JobSystem::Priority::DISPLAY;

        auto const& renderer = mDriver.getContext().state.renderer;
        // Some drivers support parallel shader compilation well, so we use N
        // threads, we can use lower priority threads here because urgent compilations
        // will most likely happen on the main gl thread. Using too many thread can
        // increase memory pressure significantly.
        if (UTILS_UNLIKELY(strstr(renderer, "PowerVR"))) {
            // For PowerVR, it's unclear what the cost of extra shared contexts is, so we only
            // use 2 to be safe.
            poolSize = 2;
            priority = JobSystem::Priority::BACKGROUND;
        } else if (UTILS_UNLIKELY(strstr(renderer, "ANGLE"))) {
            // Angle shared contexts are not expensive once we have two.
            poolSize = (std::thread::hardware_concurrency() + 1) / 2;
            priority = JobSystem::Priority::BACKGROUND;
        }

        mShaderCompilerThreadCount = poolSize;
        mCompilerThreadPool.init(mShaderCompilerThreadCount,
                [&platform = mDriver.mPlatform, priority] {
                    // give the thread a name
                    JobSystem::setThreadName("CompilerThreadPool");
                    // run at a slightly lower priority than other filament threads
                    JobSystem::setThreadPriority(priority);
                    // create a gl context current to this thread
                    platform.createContext(true);
                },
                [&platform = mDriver.mPlatform] {
                    // release context and thread state
                    platform.releaseContext();
                });
    }
}

void ShaderCompilerService::terminate() noexcept {
    // Finally stop the thread pool immediately. Pending jobs will be discarded. We guarantee by
    // construction that nobody is waiting on a token (because waiting is only done on the main
    // backend thread, and if we're here, we're on the backend main thread).
    mCompilerThreadPool.terminate();

    mRunAtNextTickOps.clear();

    // We could have some pending callbacks here, we need to execute them.
    // This is equivalent to calling cancelTickOp() on all active tokens.
    mCallbackManager.terminate();
}

ShaderCompilerService::program_token_t ShaderCompilerService::createProgram(
        CString const& name, Program&& program) {
    auto& gl = mDriver.getContext();

    // Create a token. A callback condition (handle) is internally created upon token creation.
    auto token = std::make_shared<OpenGLProgramToken>(*this, name);
    if (UTILS_UNLIKELY(gl.isES2())) {
        token->attributes = std::move(program.getAttributes());
    }

    // Try retrieving the cached program blob if available.
    token->gl.program = mBlobCache.retrieve(&token->key, mDriver.mPlatform, program);
    if (token->gl.program) {
        return token;
    }

    // Initiate program compilation.
    CompilerPriorityQueue const priorityQueue = program.getPriorityQueue();
    switch (mMode) {
        case Mode::THREAD_POOL: {
            mCompilerThreadPool.queue(priorityQueue, token,
                    [this, &gl, program = std::move(program), token]() mutable {
                        compileShaders(gl, std::move(program.getShadersSource()),
                                program.getSpecializationConstants(), program.isMultiview(), token);
                        linkProgram(gl, token);
                        // Now `token->gl.program` must be populated, so we signal the completion
                        // of the linking. We don't need to check the result of the program here
                        // because it'll be done in the engine thread.
                        token->signal();
                        // We try caching the program blob after sending the signal. This allows us
                        // to unblock the engine thread as soon as the token is ready while
                        // performing an expensive caching operation still in the pool.
                        tryCachingProgram(mBlobCache, mDriver.mPlatform, token);
                    });
            break;
        }

        case Mode::SYNCHRONOUS:
        case Mode::ASYNCHRONOUS: {
            compileShaders(gl, std::move(program.getShadersSource()),
                    program.getSpecializationConstants(), program.isMultiview(), token);

            runAtNextTick(priorityQueue, token, [this, token](Job const&) {
                assert_invariant(mMode != Mode::THREAD_POOL);
                if (mMode == Mode::ASYNCHRONOUS) {
                    // Check link completion if link was initiated.
                    if (token->gl.program) {
                        return isLinkCompleted(token);
                    }
                    // Link hasn't been initiated, then check compile completion.
                    if (!isCompileCompleted(token)) {
                        return false;
                    }
                }
                if (!token->gl.program) {
                    linkProgram(mDriver.getContext(), token);
                    if (mMode == Mode::ASYNCHRONOUS) {
                        return false;// Wait until the link finishes.
                    }
                }
                return true;
            });
            break;
        }

        case Mode::UNDEFINED: {
            assert_invariant(false);
        }
    }

    return token;
}

GLuint ShaderCompilerService::getProgram(program_token_t& token) {
    GLuint const program = initialize(token);
    assert_invariant(token == nullptr);
#if !FILAMENT_ENABLE_MATDBG
    assert_invariant(program);
#endif
    return program;
}

/*
 * Cancel program compilation. This function is responsible for cleaning up the ongoing
 * compilation & link process. If the process is already completed by calling `initialize(token)`,
 * this function is not called.
 */
/* static */ void ShaderCompilerService::terminate(program_token_t& token) {

    assert_invariant(token);// This function should be called when the token is still alive.

    if (token->compiler.mMode == Mode::THREAD_POOL) {
        auto const job = token->compiler.mCompilerThreadPool.dequeue(token);
        if (!job) {
            // It's likely that the job was already completed. But it may be still being
            // executed at this moment. Just try waiting for it to avoid a race.
            token->wait();
        }
    }

    cleanupProgramAndShaders(token);

    // Cleanup the token.
    token->compiler.cancelTickOp(token);
    token = nullptr;// This will submit a callback condition (handle) to the callback manager.
}

void ShaderCompilerService::tick() {
    // we don't need to run executeTickOps() if we're using the thread-pool
    if (UTILS_UNLIKELY(mMode != Mode::THREAD_POOL)) {
        executeTickOps();
    }
}

CallbackManager::Handle ShaderCompilerService::issueCallbackHandle() const noexcept {
    return mCallbackManager.get();
}

void ShaderCompilerService::submitCallbackHandle(CallbackManager::Handle handle) noexcept {
    mCallbackManager.put(handle);
}

void ShaderCompilerService::notifyWhenAllProgramsAreReady(
        CallbackHandler* handler, CallbackHandler::Callback const callback, void* user) {
    if (callback) {
        mCallbackManager.setCallback(handler, callback, user);
    }
}

// ------------------------------------------------------------------------------------------------

GLuint ShaderCompilerService::initialize(program_token_t& token) {

    SYSTRACE_CALL();

    assert_invariant(token);// This function should be called when the token is still alive.

    ensureTokenIsReady(token);
    assert_invariant(token->gl.program);

    // Check status of program linking. If it failed, errors will be logged.
    bool const linked = checkLinkStatusAndCleanupShaders(token);

    // We panic if it failed to create the program.
    FILAMENT_CHECK_POSTCONDITION(linked)
            << "OpenGL program " << token->name.c_str_safe() << " failed to link or compile";

    // The program is successfully created. Try caching the program blob. In the THREAD_POOL mode,
    // caching is performed in the pool.
    if (mMode != Mode::THREAD_POOL) {
        tryCachingProgram(mBlobCache, mDriver.mPlatform, token);
    }

    GLuint const program = token->gl.program;

    // Cleanup the token.
    token->compiler.cancelTickOp(token);
    token = nullptr;// This will submit a callback condition (handle) to the callback manager.

    return program;
}

void ShaderCompilerService::ensureTokenIsReady(program_token_t const& token) {
    if (token->gl.program) {
        return;// It's ready.
    }

    switch (mMode) {
        case Mode::THREAD_POOL: {
            // We need this program right now, make sure the job is finished.
            if (auto job = mCompilerThreadPool.dequeue(token)) {
                job();// The job hasn't started yet, so execute it now.
            }

            // This may block if the job was already taken by a thread ahead of the `dequeue`
            // above and currently being executed. Otherwise, the job must have already been
            // completed by this point from either the code above or the other thread.
            token->wait();
            break;
        }

        case Mode::ASYNCHRONOUS: {
            // Technically the shader compilation may not have finished yet. To deal with the case,
            // ideally, we should wait here until the compilation is finished. However, for now, we
            // just log warnings here instead of repeatedly checking compile status. If this turns
            // out to be a real issue later, we would need to consider doing the canonical way.
            if (!isCompileCompleted(token)) {
                LOG(WARNING)
                        << "Shader compilation for OpenGL program " << token->name.c_str_safe()
                        << " is not completed yet. The following program link may not succeed.";
            }

            linkProgram(mDriver.getContext(), token);
            break;
        }

        case Mode::SYNCHRONOUS: {
            // We must not have called the TickOp yet until now. Call now to have
            // `token->gl.program` ready to use.
            tick();
            break;
        }

        case Mode::UNDEFINED: {
            assert_invariant(false);
        }
    }
}

// ------------------------------------------------------------------------------------------------

void ShaderCompilerService::runAtNextTick(CompilerPriorityQueue priority,
        program_token_t const& token, Job job) noexcept {
    // insert items in order of priority and at the end of the range
    auto& ops = mRunAtNextTickOps;
    auto const pos = std::lower_bound(ops.begin(), ops.end(), priority,
            [](ContainerType const& lhs, CompilerPriorityQueue const priorityQueue) {
                return std::get<0>(lhs) < priorityQueue;
            });
    ops.emplace(pos, priority, token, std::move(job));

    SYSTRACE_CONTEXT();
    SYSTRACE_VALUE32("ShaderCompilerService Jobs", mRunAtNextTickOps.size());
}

bool ShaderCompilerService::cancelTickOp(program_token_t const& token) noexcept {
    // We do a linear search here, but this is rare, and we know the list is pretty small.
    auto& ops = mRunAtNextTickOps;
    auto const pos = std::find_if(ops.begin(), ops.end(), [&](const auto& item) {
        return std::get<1>(item) == token;
    });
    if (pos != ops.end()) {
        ops.erase(pos);
        return true;
    }
    SYSTRACE_CONTEXT();
    SYSTRACE_VALUE32("ShaderCompilerService Jobs", ops.size());
    return false;
}

void ShaderCompilerService::executeTickOps() noexcept {
    auto& ops = mRunAtNextTickOps;
    auto it = ops.begin();
    while (it != ops.end()) {
        Job const& job = std::get<2>(*it);
        bool const remove = job.fn(job);
        if (remove) {
            it = ops.erase(it);
        } else {
            ++it;
        }
    }
    SYSTRACE_CONTEXT();
    SYSTRACE_VALUE32("ShaderCompilerService Jobs", ops.size());
}

/* static */ void ShaderCompilerService::compileShaders(OpenGLContext& context,
        Program::ShaderSource shadersSource,
        FixedCapacityVector<Program::SpecializationConstant> const& specializationConstants,
        bool multiview, program_token_t const& token) noexcept {
    SYSTRACE_CALL();

    auto const appendSpecConstantString = +[](std::string& s, Program::SpecializationConstant const& sc) {
        s += "#define SPIRV_CROSS_CONSTANT_ID_" + std::to_string(sc.id) + ' ';
        s += std::visit([](auto&& arg) { return to_string(arg); }, sc.value);
        s += '\n';
        return s;
    };

    std::string specializationConstantString;
    int32_t numViews = 2;
    for (auto const& sc: specializationConstants) {
        appendSpecConstantString(specializationConstantString, sc);
        if (sc.id == 8) {
            // This constant must match
            // ReservedSpecializationConstants::CONFIG_STEREO_EYE_COUNT
            // which we can't use here because it's defined in EngineEnums.h.
            // (we're breaking layering here, but it's for the good cause).
            numViews = std::get<int32_t>(sc.value);
        }
    }
    if (!specializationConstantString.empty()) {
        specializationConstantString += '\n';
    }

    // build all shaders
    UTILS_NOUNROLL
    for (size_t i = 0; i < Program::SHADER_TYPE_COUNT; i++) {
        const ShaderStage stage = static_cast<ShaderStage>(i);
        GLenum glShaderType{};
        switch (stage) {
            case ShaderStage::VERTEX:
                glShaderType = GL_VERTEX_SHADER;
                break;
            case ShaderStage::FRAGMENT:
                glShaderType = GL_FRAGMENT_SHADER;
                break;
            case ShaderStage::COMPUTE:
#if defined(BACKEND_OPENGL_LEVEL_GLES31)
                glShaderType = GL_COMPUTE_SHADER;
#else
                continue;
#endif
                break;
        }

        if (UTILS_LIKELY(!shadersSource[i].empty())) {
            Program::ShaderBlob& shader = shadersSource[i];
            char* shader_src = reinterpret_cast<char*>(shader.data());
            size_t const shader_len = shader.size();

            // remove GOOGLE_cpp_style_line_directive
            process_GOOGLE_cpp_style_line_directive(context, shader_src, shader_len);

            // replace the value of layout(num_views = X) for multiview extension
            if (multiview && stage == ShaderStage::VERTEX) {
                process_OVR_multiview2(context, numViews, shader_src, shader_len);
            }

            // add support for ARB_shading_language_packing if needed
            auto const packingFunctions = process_ARB_shading_language_packing(context);

            // split shader source, so we can insert the specialization constants and the packing
            // functions
            auto [version, prolog, body] = splitShaderSource({ shader_src, shader_len });

            // enable ESSL 3.10 if available
            if (context.isAtLeastGLES<3, 1>()) {
                version = "#version 310 es\n";
            }

            std::array<std::string_view, 5> sources = {
                version, prolog, specializationConstantString, packingFunctions,
                { body.data(), body.size() - 1 }// null-terminated
            };

            // Some of the sources may be zero-length. Remove them as to avoid passing lengths of
            // zero to glShaderSource(). glShaderSource should work with lengths of zero, but some
            // drivers instead interpret zero as a sentinel for a null-terminated string.
            auto const partitionPoint = std::stable_partition(sources.begin(), sources.end(),
                    [](std::string_view s) { return !s.empty(); });
            size_t const count = std::distance(sources.begin(), partitionPoint);

            std::array<const char*, 5> shaderStrings;
            std::array<GLint, 5> lengths;
            for (size_t j = 0; j < count; j++) {
                shaderStrings[j] = sources[j].data();
                lengths[j] = GLint(sources[j].size());
            }

            GLuint const shaderId = glCreateShader(glShaderType);
            glShaderSource(shaderId, GLsizei(count), shaderStrings.data(), lengths.data());
            glCompileShader(shaderId);
#ifndef NDEBUG
            // for debugging we return the original shader source (without the modifications we
            // made here), otherwise the line numbers wouldn't match.
            token->shaderSourceCode[i] = { shader_src, shader_len };
#endif
            token->gl.shaders[i] = shaderId;
        }
    }
}

/* static */ bool ShaderCompilerService::isCompileCompleted(program_token_t const& token) noexcept {
    GLenum param = GL_COMPLETION_STATUS;
    if (UTILS_UNLIKELY(token->compiler.mMode != Mode::ASYNCHRONOUS)) {
        param = GL_COMPILE_STATUS;
    }

    for (auto shader: token->gl.shaders) {
        if (!shader) {
            continue;
        }
        GLint status;
        glGetShaderiv(shader, param, &status);
        if (status == GL_FALSE) {
            return false;
        }
    }
    return true;
}

/* static */ void ShaderCompilerService::checkCompileStatus(program_token_t const& token) noexcept {
    SYSTRACE_CALL();

    UTILS_NOUNROLL
    for (size_t i = 0; i < Program::SHADER_TYPE_COUNT; i++) {
        const GLuint shader = token->gl.shaders[i];
        if (!shader) {
            continue;// We're not using this shader stage.
        }
        // GL_COMPILE_STATUS may block until the compilation is completed.
        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (UTILS_LIKELY(status == GL_TRUE)) {
            continue;// Succeeded in compilation.
        }
        // Something went wrong. Log the error message.
        const ShaderStage type = static_cast<ShaderStage>(i);
        logCompilationError(type, token->name.c_str_safe(), shader, token->shaderSourceCode[i]);
    }
}

/* static */ void ShaderCompilerService::linkProgram(OpenGLContext const& context,
        program_token_t const& token) noexcept {
    SYSTRACE_CALL();

    // Shader compilation should be completed by now. Check the status and log errors on failure.
    checkCompileStatus(token);

    // Link program
    GLuint const program = glCreateProgram();
    for (auto const shader: token->gl.shaders) {
        if (shader) {
            glAttachShader(program, shader);
        }
    }
    if (UTILS_UNLIKELY(context.isES2())) {
        for (auto const& [name, loc]: token->attributes) {
            glBindAttribLocation(program, loc, name.c_str());
        }
    }
    glLinkProgram(program);
    token->gl.program = program;
}

/* static */ bool ShaderCompilerService::isLinkCompleted(program_token_t const& token) noexcept {
    assert_invariant(token->gl.program);

    GLenum param = GL_COMPLETION_STATUS;
    if (UTILS_UNLIKELY(token->compiler.mMode != Mode::ASYNCHRONOUS)) {
        param = GL_LINK_STATUS;
    }

    GLint status;
    glGetProgramiv(token->gl.program, param, &status);
    return (status == GL_TRUE);
}

/* static */ bool ShaderCompilerService::checkLinkStatusAndCleanupShaders(
        program_token_t const& token) noexcept {
    SYSTRACE_CALL();
    assert_invariant(token->gl.program);

    bool linked = true;
    GLint status;
    // GL_LINK_STATUS may block until the link is completed.
    glGetProgramiv(token->gl.program, GL_LINK_STATUS, &status);
    if (UTILS_UNLIKELY(status != GL_TRUE)) {
        // Something went wrong. Log the error message.
        logProgramLinkError(token->name.c_str_safe(), token->gl.program);
        linked = false;
    }
    // No need to keep the shaders around regardless of the result of the program linking.
    UTILS_NOUNROLL
    for (GLuint& shader: token->gl.shaders) {
        if (shader) {
            glDetachShader(token->gl.program, shader);
            glDeleteShader(shader);
            shader = 0;
        }
    }
    return linked;
}

/* static */ void ShaderCompilerService::tryCachingProgram(OpenGLBlobCache& cache,
        OpenGLPlatform& platform, program_token_t const& token) noexcept {
    if (!token->key || !token->gl.program) {
        return; // Invalid params
    }
    GLint status = GL_FALSE;
    glGetProgramiv(token->gl.program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        return;// Link failure
    }

    cache.insert(platform, token->key, token->gl.program);
}

/* static */ void ShaderCompilerService::cleanupProgramAndShaders(
        program_token_t const& token) noexcept {
    for (GLuint& shader: token->gl.shaders) {
        if (!shader) {
            continue;
        }
        if (token->gl.program) {
            glDetachShader(token->gl.program, shader);
        }
        glDeleteShader(shader);
        shader = 0;
    }
    if (token->gl.program) {
        glDeleteProgram(token->gl.program);
        token->gl.program = 0;
    }
}

// ------------------------------------------------------------------------------------------------

UTILS_NOINLINE
/* static */ void logCompilationError(ShaderStage shaderType, const char* name,
        GLuint const shaderId, UTILS_UNUSED_IN_RELEASE CString const& sourceCode) noexcept {

    { // scope for the temporary string storage
        auto to_string = [](ShaderStage type) -> const char* {
            switch (type) {
                case ShaderStage::VERTEX:
                    return "vertex";
                case ShaderStage::FRAGMENT:
                    return "fragment";
                case ShaderStage::COMPUTE:
                    return "compute";
            }
            return "unknown";
        };

        GLint length = 0;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);

        CString infoLog(length);
        glGetShaderInfoLog(shaderId, length, nullptr, infoLog.data());

        LOG(ERROR) << "Compilation error in " << to_string(shaderType) << " shader \"" << name
                   << "\":";
        LOG(ERROR) << "\"" << infoLog.c_str() << "\"";
    }

#ifndef NDEBUG
    std::string_view const shader{ sourceCode.data(), sourceCode.size() };
    size_t lc = 1;
    size_t start = 0;
    std::string line;
    while (true) {
        size_t const end = shader.find('\n', start);
        if (end == std::string::npos) {
            line = shader.substr(start);
        } else {
            line = shader.substr(start, end - start);
        }
        LOG(ERROR) << lc++ << ":   " << line.c_str();
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
    LOG(ERROR) << "";
#endif
}

UTILS_NOINLINE
/* static */ void logProgramLinkError(char const* name, GLuint program) noexcept {
    GLint length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

    CString infoLog(length);
    glGetProgramInfoLog(program, length, nullptr, infoLog.data());

    LOG(ERROR) << "Link error in \"" << name << "\":\n" << "\"" << infoLog.c_str() << "\"";
}

// If usages of the Google-style line directive are present, remove them, as some
// drivers don't allow the quotation marks. This source modification happens in-place.
/* static */ void process_GOOGLE_cpp_style_line_directive(OpenGLContext const& context,
        char* source, size_t len) noexcept {
    if (!context.ext.GOOGLE_cpp_style_line_directive) {
        if (UTILS_UNLIKELY(requestsGoogleLineDirectivesExtension({ source, len }))) {
            removeGoogleLineDirectives(source, len);// length is unaffected
        }
    }
}

// Look up the `source` to replace the number of eyes for multiview with the given number. This is
// necessary for OpenGL because OpenGL relies on the number specified in shader files to determine
// the number of views, which is assumed as a single digit, for multiview.
// This source modification happens in-place.
/* static */ void process_OVR_multiview2(OpenGLContext const& context, int32_t const eyeCount,
    char* source, size_t const len) noexcept {
    // We don't use regular expression in favor of performance.
    if (context.ext.OVR_multiview2) {
        const std::string_view shader{ source, len };
        constexpr std::string_view layout = "layout";
        constexpr std::string_view num_views = "num_views";
        size_t found = 0;
        while (true) {
            found = shader.find(layout, found);
            if (found == std::string_view::npos) {
                break;
            }
            found = shader.find_first_not_of(' ', found + layout.size());
            if (found == std::string_view::npos || shader[found] != '(') {
                continue;
            }
            found = shader.find_first_not_of(' ', found + 1);
            if (found == std::string_view::npos) {
                continue;
            }
            if (shader.compare(found, num_views.size(), num_views) != 0) {
                continue;
            }
            found = shader.find_first_not_of(' ', found + num_views.size());
            if (found == std::string_view::npos || shader[found] != '=') {
                continue;
            }
            found = shader.find_first_not_of(' ', found + 1);
            if (found == std::string_view::npos) {
                continue;
            }
            // We assume the value should be one-digit number.
            assert_invariant(eyeCount < 10);
            assert_invariant(!::isdigit(source[found + 1]));
            source[found] = '0' + eyeCount;
            break;
        }
    }
}

// Tragically, OpenGL 4.1 doesn't support unpackHalf2x16 (appeared in 4.2) and
// macOS doesn't support GL_ARB_shading_language_packing
// Also GLES3.0 didn't have the full set of packing/unpacking functions
/* static */ std::string_view process_ARB_shading_language_packing(
        OpenGLContext& context) noexcept {
    using namespace std::literals;
#ifdef BACKEND_OPENGL_VERSION_GL
    if (!context.isAtLeastGL<4, 2>() && !context.ext.ARB_shading_language_packing) {
        return R"(

// these don't handle denormals, NaNs or inf
float u16tofp32(highp uint v) {
    v <<= 16u;
    highp uint s = v & 0x80000000u;
    highp uint n = v & 0x7FFFFFFFu;
    highp uint nz = (n == 0u) ? 0u : 0xFFFFFFFFu;
    return uintBitsToFloat(s | ((((n >> 3u) + (0x70u << 23u))) & nz));
}
vec2 unpackHalf2x16(highp uint v) {
    return vec2(u16tofp32(v&0xFFFFu), u16tofp32(v>>16u));
}
uint fp32tou16(float val) {
    uint f32 = floatBitsToUint(val);
    uint f16 = 0u;
    uint sign = (f32 >> 16u) & 0x8000u;
    int exponent = int((f32 >> 23u) & 0xFFu) - 127;
    uint mantissa = f32 & 0x007FFFFFu;
    if (exponent > 15) {
        f16 = sign | (0x1Fu << 10u);
    } else if (exponent > -15) {
        exponent += 15;
        mantissa >>= 13;
        f16 = sign | uint(exponent << 10) | mantissa;
    } else {
        f16 = sign;
    }
    return f16;
}
highp uint packHalf2x16(vec2 v) {
    highp uint x = fp32tou16(v.x);
    highp uint y = fp32tou16(v.y);
    return (y << 16u) | x;
}
highp uint packUnorm4x8(mediump vec4 v) {
    v = round(clamp(v, 0.0, 1.0) * 255.0);
    highp uint a = uint(v.x);
    highp uint b = uint(v.y) <<  8;
    highp uint c = uint(v.z) << 16;
    highp uint d = uint(v.w) << 24;
    return (a|b|c|d);
}
highp uint packSnorm4x8(mediump vec4 v) {
    v = round(clamp(v, -1.0, 1.0) * 127.0);
    highp uint a = uint((int(v.x) & 0xff));
    highp uint b = uint((int(v.y) & 0xff)) <<  8;
    highp uint c = uint((int(v.z) & 0xff)) << 16;
    highp uint d = uint((int(v.w) & 0xff)) << 24;
    return (a|b|c|d);
}
mediump vec4 unpackUnorm4x8(highp uint v) {
    return vec4(float((v & 0x000000ffu)      ),
                float((v & 0x0000ff00u) >>  8),
                float((v & 0x00ff0000u) >> 16),
                float((v & 0xff000000u) >> 24)) / 255.0;
}
mediump vec4 unpackSnorm4x8(highp uint v) {
    int a = int(((v       ) & 0xffu) << 24u) >> 24 ;
    int b = int(((v >>  8u) & 0xffu) << 24u) >> 24 ;
    int c = int(((v >> 16u) & 0xffu) << 24u) >> 24 ;
    int d = int(((v >> 24u) & 0xffu) << 24u) >> 24 ;
    return clamp(vec4(float(a), float(b), float(c), float(d)) / 127.0, -1.0, 1.0);
}
)"sv;
    }
#endif// BACKEND_OPENGL_VERSION_GL

#ifdef BACKEND_OPENGL_VERSION_GLES
    if (!context.isES2() && !context.isAtLeastGLES<3, 1>()) {
        return R"(

highp uint packUnorm4x8(mediump vec4 v) {
    v = round(clamp(v, 0.0, 1.0) * 255.0);
    highp uint a = uint(v.x);
    highp uint b = uint(v.y) <<  8;
    highp uint c = uint(v.z) << 16;
    highp uint d = uint(v.w) << 24;
    return (a|b|c|d);
}
highp uint packSnorm4x8(mediump vec4 v) {
    v = round(clamp(v, -1.0, 1.0) * 127.0);
    highp uint a = uint((int(v.x) & 0xff));
    highp uint b = uint((int(v.y) & 0xff)) <<  8;
    highp uint c = uint((int(v.z) & 0xff)) << 16;
    highp uint d = uint((int(v.w) & 0xff)) << 24;
    return (a|b|c|d);
}
mediump vec4 unpackUnorm4x8(highp uint v) {
    return vec4(float((v & 0x000000ffu)      ),
                float((v & 0x0000ff00u) >>  8),
                float((v & 0x00ff0000u) >> 16),
                float((v & 0xff000000u) >> 24)) / 255.0;
}
mediump vec4 unpackSnorm4x8(highp uint v) {
    int a = int(((v       ) & 0xffu) << 24u) >> 24 ;
    int b = int(((v >>  8u) & 0xffu) << 24u) >> 24 ;
    int c = int(((v >> 16u) & 0xffu) << 24u) >> 24 ;
    int d = int(((v >> 24u) & 0xffu) << 24u) >> 24 ;
    return clamp(vec4(float(a), float(b), float(c), float(d)) / 127.0, -1.0, 1.0);
}
)"sv;
    }
#endif// BACKEND_OPENGL_VERSION_GLES
    return ""sv;
}

// split shader source code in three:
// - the version line
// - extensions
// - everything else
/* static */ std::array<std::string_view, 3> splitShaderSource(std::string_view source) noexcept {
    auto const version_start = source.find("#version");
    assert_invariant(version_start != std::string_view::npos);

    auto const version_eol = source.find('\n', version_start) + 1;
    assert_invariant(version_eol != std::string_view::npos);

    auto const prolog_start = version_eol;
    auto prolog_eol = source.rfind("\n#extension");// last #extension line
    if (prolog_eol == std::string_view::npos) {
        prolog_eol = prolog_start;
    } else {
        prolog_eol = source.find('\n', prolog_eol + 1) + 1;
    }
    auto const body_start = prolog_eol;

    std::string_view const version = source.substr(version_start, version_eol - version_start);
    std::string_view const prolog = source.substr(prolog_start, prolog_eol - prolog_start);
    std::string_view const body = source.substr(body_start, source.length() - body_start);
    return { version, prolog, body };
}

} // namespace filament::backend
