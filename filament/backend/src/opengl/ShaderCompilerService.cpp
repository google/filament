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
#include "OpenGLBlobCache.h"
#include "OpenGLDriver.h"

#include <private/backend/BackendUtils.h>

#include <backend/Program.h>

#include <utils/compiler.h>
#include <utils/CString.h>
#include <utils/debug.h>
#include <utils/FixedCapacityVector.h>
#include <utils/JobSystem.h>
#include <utils/Log.h>
#include <utils/ostream.h>
#include <utils/Panic.h>
#include <utils/Systrace.h>

#include <array>
#include <cctype>
#include <chrono>
#include <mutex>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <variant>

#include <stddef.h>
#include <stdint.h>

namespace filament::backend {

using namespace utils;

// ------------------------------------------------------------------------------------------------

static inline std::string to_string(bool b) noexcept { return b ? "true" : "false"; }
static inline std::string to_string(int i) noexcept { return std::to_string(i); }
static inline std::string to_string(float f) noexcept { return "float(" + std::to_string(f) + ")"; }

static void logCompilationError(io::ostream& out, ShaderStage shaderType, const char* name,
        GLuint shaderId, CString const& sourceCode) noexcept;
static void logProgramLinkError(io::ostream& out, char const* name, GLuint program) noexcept;

static void process_GOOGLE_cpp_style_line_directive(OpenGLContext& context, char* source,
        size_t len) noexcept;
static void process_OVR_multiview2(OpenGLContext& context, int32_t eyeCount, char* source,
        size_t len) noexcept;
static std::string_view process_ARB_shading_language_packing(OpenGLContext& context) noexcept;
static std::array<std::string_view, 3> splitShaderSource(std::string_view source) noexcept;

// ------------------------------------------------------------------------------------------------

struct ShaderCompilerService::OpenGLProgramToken : ProgramToken {
    struct ProgramData {
        GLuint program{};
        shaders_t shaders{};
    };

    ~OpenGLProgramToken() override;

    OpenGLProgramToken(ShaderCompilerService& compiler, utils::CString const& name) noexcept
            : compiler(compiler), name(name) {
    }

    ShaderCompilerService& compiler;
    utils::CString const& name;
    utils::FixedCapacityVector<std::pair<utils::CString, uint8_t>> attributes;
    shaders_source_t shaderSourceCode;
    void* user = nullptr;
    struct {
        shaders_t shaders{};
        GLuint program = 0;
    } gl; // 12 bytes


    // Sets the programData, typically from the compiler thread, and signal the main thread.
    // This is similar to std::promise::set_value.
    void set(ProgramData const& data) noexcept {
        std::unique_lock const l(lock);
        programData = data;
        signaled = true;
        cond.notify_one();
    }

    // Get the programBinary, wait if necessary.
    // This is similar to std::future::get
    ProgramData const& get() const noexcept {
        std::unique_lock l(lock);
        cond.wait(l, [this](){ return signaled; });
        return programData;
    }

    void wait() const noexcept {
        std::unique_lock l(lock);
        cond.wait(l, [this](){ return signaled; });
    }

    // Checks if the programBinary is ready.
    // This is similar to std::future::wait_for(0s)
    bool isReady() const noexcept {
        std::unique_lock l(lock);
        using namespace std::chrono_literals;
        return cond.wait_for(l, 0s, [this](){ return signaled; });
    }

    CallbackManager::Handle handle{};
    BlobCacheKey key;
    mutable utils::Mutex lock;
    mutable utils::Condition cond;
    ProgramData programData;
    bool signaled = false;

    bool canceled = false; // not part of the signaling
};

ShaderCompilerService::OpenGLProgramToken::~OpenGLProgramToken() = default;

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
        // - on Mali shader compilation can be multi-threaded, but program linking happens on
        //   a single service thread, so we don't bother using more than one thread either.
        // - on PowerVR shader compilation and linking can be multi-threaded.
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
                [&platform = mDriver.mPlatform, priority]() {
                    // give the thread a name
                    JobSystem::setThreadName("CompilerThreadPool");
                    // run at a slightly lower priority than other filament threads
                    JobSystem::setThreadPriority(priority);
                    // create a gl context current to this thread
                    platform.createContext(true);
                },
                [&platform = mDriver.mPlatform]() {
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
        utils::CString const& name, Program&& program) {
    auto& gl = mDriver.getContext();

    auto token = std::make_shared<OpenGLProgramToken>(*this, name);
    if (UTILS_UNLIKELY(gl.isES2())) {
        token->attributes = std::move(program.getAttributes());
    }

    token->gl.program = mBlobCache.retrieve(&token->key, mDriver.mPlatform, program);
    if (token->gl.program) {
        return token;
    }

    token->handle = mCallbackManager.get();

    CompilerPriorityQueue const priorityQueue = program.getPriorityQueue();
    switch (mMode) {
        case Mode::THREAD_POOL: {
            // queue a compile job
            mCompilerThreadPool.queue(priorityQueue, token,
                    [this, &gl, program = std::move(program), token]() mutable {
                        // compile the shaders
                        shaders_t shaders{};
                        compileShaders(gl,
                                std::move(program.getShadersSource()),
                                program.getSpecializationConstants(),
                                program.isMultiview(),
                                shaders,
                                token->shaderSourceCode);

                        // link the program
                        GLuint const glProgram = linkProgram(gl, shaders, token->attributes);

                        OpenGLProgramToken::ProgramData programData;
                        programData.shaders = shaders;

                        // We need to query the link status here to guarantee that the
                        // program is compiled and linked now (we don't want this to be
                        // deferred to later). We don't care about the result at this point.
                        GLint status = GL_FALSE;
                        glGetProgramiv(glProgram, GL_LINK_STATUS, &status);
                        programData.program = glProgram;

                        // we don't need to check for success here, it'll be done on the
                        // main thread side.
                        token->set(programData);

                        mCallbackManager.put(token->handle);

                        // caching must be the last thing we do
                        if (token->key && status == GL_TRUE) {
                            // Attempt to cache. This calls glGetProgramBinary.
                            mBlobCache.insert(mDriver.mPlatform, token->key, glProgram);
                        }
                    });
            break;
        }

        case Mode::SYNCHRONOUS:
        case Mode::ASYNCHRONOUS: {
            // this cannot fail because we check compilation status after linking the program
            // shaders[] is filled with id of shader stages present.
            compileShaders(gl,
                    std::move(program.getShadersSource()),
                    program.getSpecializationConstants(),
                    program.isMultiview(),
                    token->gl.shaders,
                    token->shaderSourceCode);

            runAtNextTick(priorityQueue, token, [this, token](Job const&) {
                assert_invariant(mMode != Mode::THREAD_POOL);
                if (mMode == Mode::ASYNCHRONOUS) {
                    // don't attempt to link this program if all shaders are not done compiling
                    GLint status;
                    if (token->gl.program) {
                        glGetProgramiv(token->gl.program, GL_COMPLETION_STATUS, &status);
                        if (status == GL_FALSE) {
                            return false;
                        }
                    } else {
                        for (auto shader: token->gl.shaders) {
                            if (shader) {
                                glGetShaderiv(shader, GL_COMPLETION_STATUS, &status);
                                if (status == GL_FALSE) {
                                    return false;
                                }
                            }
                        }
                    }
                }

                if (!token->gl.program) {
                    // link the program, this also cannot fail because status is checked later.
                    token->gl.program = linkProgram(mDriver.getContext(),
                            token->gl.shaders, token->attributes);
                    if (mMode == Mode::ASYNCHRONOUS) {
                        // wait until the link finishes...
                        return false;
                    }
                }

                assert_invariant(token->gl.program);

                mCallbackManager.put(token->handle);

                if (token->key) {
                    // TODO: technically we don't have to cache right now. Is it advantageous to
                    //       do this later, maybe depending on CPU usage?
                    // attempt to cache if we don't have a thread pool (otherwise it's done
                    // by the pool).
                    mBlobCache.insert(mDriver.mPlatform, token->key, token->gl.program);
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

GLuint ShaderCompilerService::getProgram(ShaderCompilerService::program_token_t& token) {
    GLuint const program = initialize(token);
    assert_invariant(token == nullptr);
#if !FILAMENT_ENABLE_MATDBG
    assert_invariant(program);
#endif
    return program;
}

/* static */ void ShaderCompilerService::terminate(program_token_t& token) {
    assert_invariant(token);

    token->canceled = true;

    bool const isTickOpCanceled = token->compiler.cancelTickOp(token);

    if (token->compiler.mMode == Mode::THREAD_POOL) {
        auto job = token->compiler.mCompilerThreadPool.dequeue(token);
        if (!job) {
            // The job is being executed right now. We need to wait for it to finish to avoid a
            // race.
            token->wait();
        } else {
            // The job has not been executed, but we still need to inform the callback manager in
            // order for future callbacks to be successfully called.
            token->compiler.mCallbackManager.put(token->handle);
        }
    } else if (isTickOpCanceled) {
        // Since the tick op was canceled, we need to .put the token here.
        token->compiler.mCallbackManager.put(token->handle);
    }

    for (GLuint& shader: token->gl.shaders) {
        if (shader) {
            if (token->gl.program) {
                glDetachShader(token->gl.program, shader);
            }
            glDeleteShader(shader);
            shader = 0;
        }
    }
    if (token->gl.program) {
        glDeleteProgram(token->gl.program);
    }

    token.reset();
}

void ShaderCompilerService::tick() {
    // we don't need to run executeTickOps() if we're using the thread-pool
    if (UTILS_UNLIKELY(mMode != Mode::THREAD_POOL)) {
        executeTickOps();
    }
}

void ShaderCompilerService::notifyWhenAllProgramsAreReady(
        CallbackHandler* handler, CallbackHandler::Callback callback, void* user) {
    if (callback) {
        mCallbackManager.setCallback(handler, callback, user);
    }
}

// ------------------------------------------------------------------------------------------------

/* static */ void ShaderCompilerService::getProgramFromCompilerPool(
        program_token_t& token) noexcept {
    OpenGLProgramToken::ProgramData const& programData{ token->get() };
    if (!token->canceled) {
        token->gl.shaders = programData.shaders;
        token->gl.program = programData.program;
    }
}

GLuint ShaderCompilerService::initialize(program_token_t& token) noexcept {
    SYSTRACE_CALL();
    if (!token->gl.program) {
        switch (mMode) {
            case Mode::THREAD_POOL: {
                // we need this program right now, remove it from the queue
                auto job = mCompilerThreadPool.dequeue(token);
                if (job) {
                    // if we were able to remove it, we execute the job now, otherwise it means
                    // it's being executed right now.
                    job();
                }

                if (!token->canceled) {
                    token->compiler.cancelTickOp(token);
                }

                // Block until we get the program from the pool. Generally this wouldn't block
                // because we just compiled the program above, when executing job.
                ShaderCompilerService::getProgramFromCompilerPool(token);
                break;
            }

            case Mode::ASYNCHRONOUS: {
                // we force the program link -- which might stall, either here or below in
                // checkProgramStatus(), but we don't have a choice, we need to use the program now.
                token->compiler.cancelTickOp(token);

                token->gl.program =
                        linkProgram(mDriver.getContext(), token->gl.shaders, token->attributes);

                assert_invariant(token->gl.program);

                mCallbackManager.put(token->handle);

                if (token->key) {
                    mBlobCache.insert(mDriver.mPlatform, token->key, token->gl.program);
                }
                break;
            }

            case Mode::SYNCHRONOUS: {
                // if we don't have a program yet, block until we get it.
                tick();
                break;
            }

            case Mode::UNDEFINED: {
                assert_invariant(false);
            }
        }
    }

    // by this point we must have a GL program
    assert_invariant(token->gl.program);

    GLuint program = 0;

    // check status of program linking and shader compilation, logs error and free all resources
    // in case of error.
    bool const success = checkProgramStatus(token);

    // Unless we have matdbg, we panic if a program is invalid. Otherwise, we'd get a UB.
    // The compilation error has been logged to log.e by this point.
    FILAMENT_CHECK_POSTCONDITION(FILAMENT_ENABLE_MATDBG || success)
            << "OpenGL program " << token->name.c_str_safe() << " failed to link or compile";

    if (UTILS_LIKELY(success)) {
        program = token->gl.program;
        // no need to keep the shaders around
        UTILS_NOUNROLL
        for (GLuint& shader: token->gl.shaders) {
            if (shader) {
                glDetachShader(program, shader);
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }

    // and destroy all temporary init data
    token = nullptr;

    return program;
}


/*
 * Compile shaders in the ShaderSource. This cannot fail because compilation failures are not
 * checked until after the program is linked.
 * This always returns the GL shader IDs or zero a shader stage is not present.
 */
/* static */ void ShaderCompilerService::compileShaders(OpenGLContext& context,
        Program::ShaderSource shadersSource,
        utils::FixedCapacityVector<Program::SpecializationConstant> const& specializationConstants,
        bool multiview,
        shaders_t& outShaders,
        UTILS_UNUSED_IN_RELEASE shaders_source_t& outShaderSourceCode) noexcept {

    SYSTRACE_CALL();

    auto appendSpecConstantString = +[](std::string& s, Program::SpecializationConstant const& sc) {
        s += "#define SPIRV_CROSS_CONSTANT_ID_" + std::to_string(sc.id) + ' ';
        s += std::visit([](auto&& arg) { return to_string(arg); }, sc.value);
        s += '\n';
        return s;
    };

    std::string specializationConstantString;
    int32_t numViews = 2;
    for (auto const& sc : specializationConstants) {
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
            size_t shader_len = shader.size();

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
                version,
                prolog,
                specializationConstantString,
                packingFunctions,
                { body.data(), body.size() - 1 }  // null-terminated
            };

            // Some of the sources may be zero-length. Remove them as to avoid passing lengths of
            // zero to glShaderSource(). glShaderSource should work with lengths of zero, but some
            // drivers instead interpret zero as a sentinel for a null-terminated string.
            auto partitionPoint = std::stable_partition(
                    sources.begin(), sources.end(), [](std::string_view s) { return !s.empty(); });
            size_t count = std::distance(sources.begin(), partitionPoint);

            std::array<const char*, 5> shaderStrings;
            std::array<GLint, 5> lengths;
            for (size_t i = 0; i < count; i++) {
                shaderStrings[i] = sources[i].data();
                lengths[i] = sources[i].size();
            }

            GLuint const shaderId = glCreateShader(glShaderType);
            glShaderSource(shaderId, count, shaderStrings.data(), lengths.data());

            glCompileShader(shaderId);

#ifndef NDEBUG
            // for debugging we return the original shader source (without the modifications we
            // made here), otherwise the line numbers wouldn't match.
            outShaderSourceCode[i] = { shader_src, shader_len };
#endif

            outShaders[i] = shaderId;
        }
    }
}

/*
 * Create a program from the given shader IDs and links it. This cannot fail because errors
 * are checked later. This always returns a valid GL program ID (which doesn't mean the
 * program itself is valid).
 */
/* static */ GLuint ShaderCompilerService::linkProgram(OpenGLContext& context,
        shaders_t const& shaders,
        utils::FixedCapacityVector<std::pair<utils::CString, uint8_t>> const& attributes) noexcept {

    SYSTRACE_CALL();

    GLuint const program = glCreateProgram();
    for (auto shader : shaders) {
        if (shader) {
            glAttachShader(program, shader);
        }
    }

    if (UTILS_UNLIKELY(context.isES2())) {
        for (auto const& [ name, loc ] : attributes) {
            glBindAttribLocation(program, loc, name.c_str());
        }
    }

    glLinkProgram(program);

    return program;
}

// ------------------------------------------------------------------------------------------------

void ShaderCompilerService::runAtNextTick(CompilerPriorityQueue priority,
        const program_token_t& token, Job job) noexcept {
    // insert items in order of priority and at the end of the range
    auto& ops = mRunAtNextTickOps;
    auto const pos = std::lower_bound(ops.begin(), ops.end(), priority,
            [](ContainerType const& lhs, CompilerPriorityQueue priorityQueue) {
                return std::get<0>(lhs) < priorityQueue;
            });
    ops.emplace(pos, priority, token, std::move(job));

    SYSTRACE_CONTEXT();
    SYSTRACE_VALUE32("ShaderCompilerService Jobs", mRunAtNextTickOps.size());
}

bool ShaderCompilerService::cancelTickOp(program_token_t token) noexcept {
    // We do a linear search here, but this is rare, and we know the list is pretty small.
    auto& ops = mRunAtNextTickOps;
    auto pos = std::find_if(ops.begin(), ops.end(), [&](const auto& item) {
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

// ------------------------------------------------------------------------------------------------

/*
 * Checks a program link status and logs errors and frees resources on failure.
 * Returns true on success.
 */
/* static */ bool ShaderCompilerService::checkProgramStatus(program_token_t const& token) noexcept {

    SYSTRACE_CALL();

    assert_invariant(token->gl.program);

    GLint status;
    glGetProgramiv(token->gl.program, GL_LINK_STATUS, &status);
    if (UTILS_LIKELY(status == GL_TRUE)) {
        return true;
    }

    // only if the link fails, we check the compilation status
    UTILS_NOUNROLL
    for (size_t i = 0; i < Program::SHADER_TYPE_COUNT; i++) {
        const ShaderStage type = static_cast<ShaderStage>(i);
        const GLuint shader = token->gl.shaders[i];
        if (shader) {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
            if (status != GL_TRUE) {
                logCompilationError(slog.e, type,
                        token->name.c_str_safe(), shader, token->shaderSourceCode[i]);
            }
            glDetachShader(token->gl.program, shader);
            glDeleteShader(shader);
            token->gl.shaders[i] = 0;
        }
    }
    // log the link error as well
    logProgramLinkError(slog.e, token->name.c_str_safe(), token->gl.program);
    glDeleteProgram(token->gl.program);
    token->gl.program = 0;
    return false;
}

// ------------------------------------------------------------------------------------------------

UTILS_NOINLINE
/* static */ void logCompilationError(io::ostream& out, ShaderStage shaderType, const char* name,
        GLuint shaderId, UTILS_UNUSED_IN_RELEASE CString const& sourceCode) noexcept {

    auto to_string = [](ShaderStage type) -> const char* {
        switch (type) {
            case ShaderStage::VERTEX:
                return "vertex";
            case ShaderStage::FRAGMENT:
                return "fragment";
            case ShaderStage::COMPUTE:
                return "compute";
        }
    };

    {// scope for the temporary string storage
        GLint length = 0;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);

        CString infoLog(length);
        glGetShaderInfoLog(shaderId, length, nullptr, infoLog.data());

        out << "Compilation error in " << to_string(shaderType) << " shader \"" << name << "\":\n"
            << "\"" << infoLog.c_str() << "\"" << io::endl;
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
        out << lc++ << ":   " << line.c_str() << '\n';
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
    out << io::endl;
#endif
}

UTILS_NOINLINE
/* static */ void logProgramLinkError(io::ostream& out, char const* name, GLuint program) noexcept {
    GLint length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

    CString infoLog(length);
    glGetProgramInfoLog(program, length, nullptr, infoLog.data());

    out << "Link error in \"" << name << "\":\n"
        << "\"" << infoLog.c_str() << "\"" << io::endl;
}

// If usages of the Google-style line directive are present, remove them, as some
// drivers don't allow the quotation marks. This source modification happens in-place.
/* static */ void process_GOOGLE_cpp_style_line_directive(OpenGLContext& context, char* source,
        size_t len) noexcept {
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
/* static */ void process_OVR_multiview2(OpenGLContext& context, int32_t eyeCount, char* source,
        size_t len) noexcept {
    // We don't use regular expression in favor of performance.
    if (context.ext.OVR_multiview2) {
        const std::string_view shader{ source, len };
        const std::string_view layout = "layout";
        const std::string_view num_views = "num_views";
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
    auto version_start = source.find("#version");
    assert_invariant(version_start != std::string_view::npos);

    auto version_eol = source.find('\n', version_start) + 1;
    assert_invariant(version_eol != std::string_view::npos);

    auto prolog_start = version_eol;
    auto prolog_eol = source.rfind("\n#extension");// last #extension line
    if (prolog_eol == std::string_view::npos) {
        prolog_eol = prolog_start;
    } else {
        prolog_eol = source.find('\n', prolog_eol + 1) + 1;
    }
    auto body_start = prolog_eol;

    std::string_view const version = source.substr(version_start, version_eol - version_start);
    std::string_view const prolog = source.substr(prolog_start, prolog_eol - prolog_start);
    std::string_view const body = source.substr(body_start, source.length() - body_start);
    return { version, prolog, body };
}

} // namespace filament::backend
