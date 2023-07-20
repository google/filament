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

#include <backend/platforms/OpenGLPlatform.h>
#include <backend/Program.h>

#include <utils/compiler.h>
#include <utils/CString.h>
#include <utils/JobSystem.h>
#include <utils/Log.h>
#include <utils/Systrace.h>

#include <chrono>
#include <future>
#include <string>
#include <string_view>
#include <variant>

namespace filament::backend {

using namespace utils;

// ------------------------------------------------------------------------------------------------

static void logCompilationError(utils::io::ostream& out,
        ShaderStage shaderType, const char* name,
        GLuint shaderId, CString const& sourceCode) noexcept;

static void logProgramLinkError(utils::io::ostream& out,
        const char* name, GLuint program) noexcept;

static inline std::string to_string(bool b) noexcept {
    return b ? "true" : "false";
}

static inline std::string to_string(int i) noexcept {
    return std::to_string(i);
}

static inline std::string to_string(float f) noexcept {
    return "float(" + std::to_string(f) + ")";
}

// ------------------------------------------------------------------------------------------------

struct ShaderCompilerService::ProgramToken {
    struct ProgramBinary {
        GLenum format{};
        GLuint program{};
        std::array<GLuint, Program::SHADER_TYPE_COUNT> shaders{};
        std::vector<char> blob;
    };

    ProgramToken(ShaderCompilerService& compiler, utils::CString const& name) noexcept
            : compiler(compiler), name(name) {
    }
    ShaderCompilerService& compiler;
    utils::CString const& name;
    utils::FixedCapacityVector<std::pair<utils::CString, uint8_t>> attributes;
    std::array<utils::CString, Program::SHADER_TYPE_COUNT> shaderSourceCode;
    void* user = nullptr;
    struct {
        std::array<GLuint, Program::SHADER_TYPE_COUNT> shaders{};
        GLuint program = 0;
    } gl; // 12 bytes

    BlobCacheKey key;
    std::future<ProgramBinary> binary;
    CompilerPriorityQueue priorityQueue = CompilerPriorityQueue::HIGH;
    bool canceled = false;
};

void ShaderCompilerService::setUserData(const program_token_t& token, void* user) noexcept {
    token->user = user;
}

void* ShaderCompilerService::getUserData(const program_token_t& token) noexcept {
    return token->user;
}

// ------------------------------------------------------------------------------------------------

void ShaderCompilerService::CompilerThreadPool::init(
        bool useSharedContexts, uint32_t threadCount, OpenGLPlatform& platform) noexcept {

    for (size_t i = 0; i < threadCount; i++) {
        mCompilerThreads.emplace_back([this, useSharedContexts, &platform]() {
            // give the thread a name
            JobSystem::setThreadName("CompilerThreadPool");

            // create a gl context current to this thread
            platform.createContext(useSharedContexts);

            // process jobs from the queue until we're asked to exit
            while (!mExitRequested) {
                std::unique_lock lock(mQueueLock);
                mQueueCondition.wait(lock, [this]() {
                        return  mExitRequested ||
                                mUrgentJob ||
                                (!std::all_of( std::begin(mQueues), std::end(mQueues),
                                        [](auto&& q) { return q.empty(); }));
                });
                if (!mExitRequested) {
                    Job job{ std::move(mUrgentJob) };
                    if (!job) {
                        // use the first queue that's not empty
                        auto& queue = [this]() -> auto& {
                            for (auto& q: mQueues) {
                                if (!q.empty()) {
                                    return q;
                                }
                            }
                            return mQueues[0]; // we should never end-up here.
                        }();
                        assert_invariant(!queue.empty());
                        std::swap(job, queue.front().second);
                        queue.pop_front();
                    }

                    // execute the job without holding any locks
                    lock.unlock();
                    job();
                }
            }
        });

    }
}

auto ShaderCompilerService::CompilerThreadPool::dequeue(program_token_t const& token) -> Job {
    auto& q = mQueues[size_t(token->priorityQueue)];
    auto pos = std::find_if(q.begin(), q.end(), [&token](auto&& item) {
        return item.first == token;
    });
    Job job;
    if (pos != q.end()) {
        std::swap(job, pos->second);
        q.erase(pos);
    }
    return job;
}

void ShaderCompilerService::CompilerThreadPool::makeUrgent(program_token_t const& token) {
    std::unique_lock const lock(mQueueLock);
    assert_invariant(!mUrgentJob);
    Job job{ dequeue(token) };
    std::swap(job, mUrgentJob);
    mQueueCondition.notify_one();
}

void ShaderCompilerService::CompilerThreadPool::queue(program_token_t const& token, Job&& job) {
    std::unique_lock const lock(mQueueLock);
    mQueues[size_t(token->priorityQueue)].emplace_back(token, std::move(job));
    mQueueCondition.notify_one();
}

void ShaderCompilerService::CompilerThreadPool::exit() noexcept {
    std::unique_lock lock(mQueueLock);
    mExitRequested = true;
    mQueueCondition.notify_all();
    lock.unlock();
    for (auto& thread: mCompilerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

// ------------------------------------------------------------------------------------------------

ShaderCompilerService::ShaderCompilerService(OpenGLDriver& driver)
        : mDriver(driver),
          KHR_parallel_shader_compile(driver.getContext().ext.KHR_parallel_shader_compile) {
}

ShaderCompilerService::~ShaderCompilerService() noexcept = default;

void ShaderCompilerService::init() noexcept {
    // If we have KHR_parallel_shader_compile, we always use it, it should be more resource
    // friendly.
    if (!KHR_parallel_shader_compile) {
        // - on Adreno there is a single compiler object. We can't use a pool > 1
        //   also glProgramBinary blocks if other threads are compiling.
        // - on Mali shader compilation can be multithreaded, but program linking happens on
        //   a single service thread, so we don't bother using more than one thread either.
        // - on desktop we could use more threads, tbd.
        if (mDriver.mPlatform.isExtraContextSupported()) {
            mShaderCompilerThreadCount = 1;
            mCompilerThreadPool.init(mUseSharedContext,
                    mShaderCompilerThreadCount, mDriver.mPlatform);
        }
    }
}

void ShaderCompilerService::terminate() noexcept {
    // FIXME: could we have some user callbacks pending here?
    mCompilerThreadPool.exit();
}

ShaderCompilerService::program_token_t ShaderCompilerService::createProgram(
        utils::CString const& name, Program&& program) {
    auto& gl = mDriver.getContext();

    auto token = std::make_shared<ProgramToken>(*this, name);

    if (UTILS_UNLIKELY(gl.isES2())) {
        token->attributes = std::move(program.getAttributes());
    }

    token->gl.program = OpenGLBlobCache::retrieve(&token->key, mDriver.mPlatform, program);
    if (!token->gl.program) {
        if (mShaderCompilerThreadCount) {
            // set the future in the token and pass the promise to the worker thread
            std::promise<ProgramToken::ProgramBinary> promise;
            token->binary = promise.get_future();
            token->priorityQueue = program.getPriorityQueue();
            // queue a compile job
            mCompilerThreadPool.queue(token,
                    [this, &gl, promise = std::move(promise),
                            program = std::move(program), token]() mutable {

                        // compile the shaders
                        std::array<GLuint, Program::SHADER_TYPE_COUNT> shaders{};
                        std::array<utils::CString, Program::SHADER_TYPE_COUNT> shaderSourceCode;
                        compileShaders(gl,
                                std::move(program.getShadersSource()),
                                program.getSpecializationConstants(),
                                shaders,
                                shaderSourceCode);

                        // link the program
                        GLuint const glProgram = linkProgram(gl, shaders, token->attributes);

                        ProgramToken::ProgramBinary binary;
                        binary.shaders = shaders;

                        if (UTILS_LIKELY(mUseSharedContext)) {
                            // We need to query the link status here to guarantee that the
                            // program is compiled and linked now (we don't want this to be
                            // deferred to later). We don't care about the result at this point.
                            GLint status;
                            glGetProgramiv(glProgram, GL_LINK_STATUS, &status);
                            binary.program = glProgram;
                            if (token->key) {
                                // Attempt to cache. This calls glGetProgramBinary.
                                OpenGLBlobCache::insert(mDriver.mPlatform,
                                        token->key, token->gl.program);
                            }
                        }
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
                        else {
                            // retrieve the program binary
                            GLsizei programBinarySize = 0;
                            glGetProgramiv(glProgram, GL_PROGRAM_BINARY_LENGTH, &programBinarySize);
                            assert_invariant(programBinarySize);
                            if (programBinarySize) {
                                binary.blob.resize(programBinarySize);
                                glGetProgramBinary(glProgram, programBinarySize,
                                        &programBinarySize, &binary.format, binary.blob.data());
                            }
                            // and we can destroy the program
                            glDeleteProgram(glProgram);
                            if (token->key) {
                                // attempt to cache
                                OpenGLBlobCache::insert(mDriver.mPlatform, token->key,
                                        binary.format,
                                        binary.blob.data(), GLsizei(binary.blob.size()));
                            }
                        }
#endif
                        // we don't need to check for success here, it'll be done on the
                        // main thread side.
                        promise.set_value(binary);
                    });
        } else
        {
            // this cannot fail because we check compilation status after linking the program
            // shaders[] is filled with id of shader stages present.
            compileShaders(gl,
                    std::move(program.getShadersSource()),
                    program.getSpecializationConstants(),
                    token->gl.shaders,
                    token->shaderSourceCode);

        }

        runAtNextTick(token->priorityQueue, token, [this, token]() {
            if (mShaderCompilerThreadCount) {
                if (!token->gl.program) {
                    // TODO: see if we could completely eliminate this callback here
                    //       and instead just rely on token->gl.program being atomically
                    //       set by the compiler thread.
                    assert_invariant(token->binary.valid());
                    // we're using the compiler thread, check if the program is ready, no-op if not.
                    using namespace std::chrono_literals;
                    if (token->binary.wait_for(0s) != std::future_status::ready) {
                        return false;
                    }
                    // program binary is ready, retrieve it without blocking
                    ShaderCompilerService::getProgramFromCompilerPool(
                            const_cast<program_token_t&>(token));
                }
            } else {
                if (KHR_parallel_shader_compile) {
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
                    if (KHR_parallel_shader_compile) {
                        // wait until the link finishes...
                        return false;
                    }
                }
            }

            assert_invariant(token->gl.program);

            if (token->key && !mShaderCompilerThreadCount) {
                // TODO: technically we don't have to cache right now. Is it advantageous to
                //       do this later, maybe depending on CPU usage?
                // attempt to cache if we don't have a thread pool (otherwise it's done
                // by the pool).
                OpenGLBlobCache::insert(mDriver.mPlatform, token->key, token->gl.program);
            }

            return true;
        });
    }

    return token;
}

bool ShaderCompilerService::isProgramReady(
        const ShaderCompilerService::program_token_t& token) const noexcept {

    assert_invariant(token);

    if (!token->gl.program) {
        return false;
    }

    if (KHR_parallel_shader_compile) {
        GLint status = GL_FALSE;
        glGetProgramiv(token->gl.program, GL_COMPLETION_STATUS, &status);
        return (bool)status;
    }

    // If gl.program is set, this means the program was linked. Some drivers may defer the link
    // in which case we might block in getProgram() when we check the program status.
    // Unfortunately, this is nothing we can do about that.
    return bool(token->gl.program);
}

GLuint ShaderCompilerService::getProgram(ShaderCompilerService::program_token_t& token) {
    GLuint const program = initialize(token);
    assert_invariant(token == nullptr);
    assert_invariant(program);
    return program;
}

/* static*/ void ShaderCompilerService::terminate(program_token_t& token) {
    assert_invariant(token);

    token->canceled = true;

    token->compiler.cancelTickOp(token);

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

    token = nullptr;
}

void ShaderCompilerService::tick() {
    executeTickOps();
}

void ShaderCompilerService::notifyWhenAllProgramsAreReady(CompilerPriorityQueue priority,
        CallbackHandler* handler, CallbackHandler::Callback callback, void* user) {

    if (KHR_parallel_shader_compile || mShaderCompilerThreadCount) {
        // list all programs up to this point, both low and high priority
        utils::FixedCapacityVector<program_token_t, std::allocator<program_token_t>, false> tokens;
        tokens.reserve(mRunAtNextTickOps.size());
        for (auto& [priority_, token, fn_] : mRunAtNextTickOps) {
            if (token) {
                tokens.push_back(token);
            }
        }

        runAtNextTick(priority, nullptr,
                [this, tokens = std::move(tokens), handler, user, callback]() {
            for (auto const& token : tokens) {
                assert_invariant(token);
                if (!isProgramReady(token)) {
                    // one of the program is not ready, try next time
                    return false;
                }
            }
            if (callback) {
                // all programs are ready, we can call the callbacks
                mDriver.scheduleCallback(handler, user, callback);
            }
            // and we're done
            return true;
        });

        return;
    }

    // we don't have KHR_parallel_shader_compile

    runAtNextTick(priority, nullptr, [this, handler, user, callback]() {
        mDriver.scheduleCallback(handler, user, callback);
        return true;
    });

    // TODO: we could spread the compiles over several frames, the tick() below then is not
    //       needed here. We keep it for now as to not change the current behavior too much.
    // this will block until all programs are linked
    tick();
}

// ------------------------------------------------------------------------------------------------

void ShaderCompilerService::getProgramFromCompilerPool(program_token_t& token) noexcept {
    ProgramToken::ProgramBinary const binary{ token->binary.get() };
    if (!token->canceled) {
        token->gl.shaders = binary.shaders;
        if (UTILS_LIKELY(mUseSharedContext)) {
            token->gl.program = binary.program;
        }
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
        else {
            token->gl.program = glCreateProgram();
            glProgramBinary(token->gl.program, binary.format,
                    binary.blob.data(), GLsizei(binary.blob.size()));
        }
#endif
    }
}

GLuint ShaderCompilerService::initialize(program_token_t& token) noexcept {
    SYSTRACE_CALL();
    if (!token->gl.program) {
        if (mShaderCompilerThreadCount) {
            // Block until the program is ready. This could take a very long time.
            assert_invariant(token->binary.valid());

            // we need this program right now, so move it to the head of the queue.
            mCompilerThreadPool.makeUrgent(token);

            if (!token->canceled) {
                token->compiler.cancelTickOp(token);
            }

            // block until we get the program from the pool
            ShaderCompilerService::getProgramFromCompilerPool(token);
        } else if (KHR_parallel_shader_compile) {
            // we force the program link -- which might stall, either here or below in
            // checkProgramStatus(), but we don't have a choice, we need to use the program now.
            token->compiler.cancelTickOp(token);
            token->gl.program = linkProgram(mDriver.getContext(),
                    token->gl.shaders, token->attributes);
        } else {
            // if we don't have a program yet, block until we get it.
            tick();
        }
    }

    // by this point we must have a GL program
    assert_invariant(token->gl.program);

    GLuint program = 0;

    // check status of program linking and shader compilation, logs error and free all resources
    // in case of error.
    bool const success = checkProgramStatus(token);
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
void ShaderCompilerService::compileShaders(OpenGLContext& context,
        Program::ShaderSource shadersSource,
        utils::FixedCapacityVector<Program::SpecializationConstant> const& specializationConstants,
        std::array<GLuint, Program::SHADER_TYPE_COUNT>& outShaders,
        UTILS_UNUSED_IN_RELEASE std::array<CString, Program::SHADER_TYPE_COUNT>& outShaderSourceCode) noexcept {

    SYSTRACE_CALL();

    auto appendSpecConstantString = +[](std::string& s, Program::SpecializationConstant const& sc) {
        s += "#define SPIRV_CROSS_CONSTANT_ID_" + std::to_string(sc.id) + ' ';
        s += std::visit([](auto&& arg) { return to_string(arg); }, sc.value);
        s += '\n';
        return s;
    };

    std::string specializationConstantString;
    for (auto const& sc : specializationConstants) {
        appendSpecConstantString(specializationConstantString, sc);
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

            // remove GOOGLE_cpp_style_line_directive
            std::string_view const source = process_GOOGLE_cpp_style_line_directive(context,
                    reinterpret_cast<char*>(shader.data()), shader.size());

            // add support for ARB_shading_language_packing if needed
            auto const packingFunctions = process_ARB_shading_language_packing(context);

            // split shader source, so we can insert the specification constants and the packing functions
            auto const [prolog, body] = splitShaderSource(source);

            const std::array<const char*, 4> sources = {
                    prolog.data(),
                    specializationConstantString.c_str(),
                    packingFunctions.data(),
                    body.data()
            };

            const std::array<GLint, 4> lengths = {
                    (GLint)prolog.length(),
                    (GLint)specializationConstantString.length(),
                    (GLint)packingFunctions.length(),
                    (GLint)body.length() - 1 // null terminated
            };

            GLuint const shaderId = glCreateShader(glShaderType);
            glShaderSource(shaderId, sources.size(), sources.data(), lengths.data());
            glCompileShader(shaderId);

#ifndef NDEBUG
            // for debugging we return the original shader source (without the modifications we
            // made here), otherwise the line numbers wouldn't match.
            outShaderSourceCode[i] = { source.data(), source.length() };
#endif

            outShaders[i] = shaderId;
        }
    }
}

// If usages of the Google-style line directive are present, remove them, as some
// drivers don't allow the quotation marks. This happens in-place.
std::string_view ShaderCompilerService::process_GOOGLE_cpp_style_line_directive(OpenGLContext& context,
        char* source, size_t len) noexcept {
    if (!context.ext.GOOGLE_cpp_style_line_directive) {
        if (UTILS_UNLIKELY(requestsGoogleLineDirectivesExtension({ source, len }))) {
            removeGoogleLineDirectives(source, len); // length is unaffected
        }
    }
    return { source, len };
}

// Tragically, OpenGL 4.1 doesn't support unpackHalf2x16 (appeared in 4.2) and
// macOS doesn't support GL_ARB_shading_language_packing
std::string_view ShaderCompilerService::process_ARB_shading_language_packing(OpenGLContext& context) noexcept {
    using namespace std::literals;
#ifdef BACKEND_OPENGL_VERSION_GL
    if (!context.isAtLeastGL<4, 2>() && !context.ext.ARB_shading_language_packing) {
        return R"(

// these don't handle denormals, NaNs or inf
float u16tofp32(highp uint v) {
    v <<= 16u;
    highp uint s = v & 0x80000000u;
    highp uint n = v & 0x7FFFFFFFu;
    highp uint nz = n == 0u ? 0u : 0xFFFFFFFF;
    return uintBitsToFloat(s | ((((n >> 3u) + (0x70u << 23))) & nz));
}
vec2 unpackHalf2x16(highp uint v) {
    return vec2(u16tofp32(v&0xFFFFu), u16tofp32(v>>16u));
}
uint fp32tou16(float val) {
    uint f32 = floatBitsToUint(val);
    uint f16 = 0u;
    uint sign = (f32 >> 16) & 0x8000u;
    int exponent = int((f32 >> 23) & 0xFFu) - 127;
    uint mantissa = f32 & 0x007FFFFFu;
    if (exponent > 15) {
        f16 = sign | (0x1Fu << 10);
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
    return (y << 16) | x;
}
)"sv;
    }
#endif // BACKEND_OPENGL_VERSION_GL
    return ""sv;
}

// split shader source code in two, the first section goes from the start to the line after the
// last #extension, and the 2nd part goes from there to the end.
std::array<std::string_view, 2> ShaderCompilerService::splitShaderSource(std::string_view source) noexcept {
    auto start = source.find("#version");
    assert_invariant(start != std::string_view::npos);

    auto pos = source.rfind("\n#extension");
    if (pos == std::string_view::npos) {
        pos = start;
    } else {
        ++pos;
    }

    auto eol = source.find('\n', pos) + 1;
    assert_invariant(eol != std::string_view::npos);

    std::string_view const version = source.substr(start, eol - start);
    std::string_view const body = source.substr(version.length(), source.length() - version.length());
    return { version, body };
}

/*
 * Create a program from the given shader IDs and links it. This cannot fail because errors
 * are checked later. This always returns a valid GL program ID (which doesn't mean the
 * program itself is valid).
 */
GLuint ShaderCompilerService::linkProgram(OpenGLContext& context,
        std::array<GLuint, Program::SHADER_TYPE_COUNT> shaders,
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
        const program_token_t& token, std::function<bool()> fn) noexcept {
    // insert items in order of priority and at the end of the range
    auto& ops = mRunAtNextTickOps;
    auto const pos = std::lower_bound(ops.begin(), ops.end(), priority,
            [](ContainerType const& lhs, CompilerPriorityQueue priorityQueue) {
                return std::get<0>(lhs) < priorityQueue;
            });
    ops.emplace(pos, priority, token, std::move(fn));

    SYSTRACE_CONTEXT();
    SYSTRACE_VALUE32("ShaderCompilerService Jobs", mRunAtNextTickOps.size());
}

void ShaderCompilerService::cancelTickOp(program_token_t token) noexcept {
    // We do a linear search here, but this is rare, and we know the list is pretty small.
    auto& ops = mRunAtNextTickOps;
    auto pos = std::find_if(ops.begin(), ops.end(),
            [&](const auto& item) {
        return std::get<1>(item) == token;
    });
    if (pos != ops.end()) {
        ops.erase(pos);
    }
    SYSTRACE_CONTEXT();
    SYSTRACE_VALUE32("ShaderCompilerService Jobs", ops.size());
}

void ShaderCompilerService::executeTickOps() noexcept {
    auto& ops = mRunAtNextTickOps;
    auto it = ops.begin();
    while (it != ops.end()) {
        auto fn = std::get<2>(*it);
        bool const remove = fn();
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
bool ShaderCompilerService::checkProgramStatus(program_token_t const& token) noexcept {

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

UTILS_NOINLINE
void logCompilationError(io::ostream& out, ShaderStage shaderType,
        const char* name, GLuint shaderId,
        UTILS_UNUSED_IN_RELEASE CString const& sourceCode) noexcept {

    auto to_string = [](ShaderStage type) -> const char* {
        switch (type) {
            case ShaderStage::VERTEX:   return "vertex";
            case ShaderStage::FRAGMENT: return "fragment";
            case ShaderStage::COMPUTE:  return "compute";
        }
    };

    { // scope for the temporary string storage
        GLint length = 0;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);

        CString infoLog(length);
        glGetShaderInfoLog(shaderId, length, nullptr, infoLog.data());

        out << "Compilation error in " << to_string(shaderType) << " shader \"" << name << "\":\n"
            << "\"" << infoLog.c_str() << "\""
            << io::endl;
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
void logProgramLinkError(io::ostream& out, char const* name, GLuint program) noexcept {
    GLint length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

    CString infoLog(length);
    glGetProgramInfoLog(program, length, nullptr, infoLog.data());

    out << "Link error in \"" << name << "\":\n"
        << "\"" << infoLog.c_str() << "\""
        << io::endl;
}


} // namespace filament::backend
