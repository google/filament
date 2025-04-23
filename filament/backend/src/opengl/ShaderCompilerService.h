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

#ifndef TNT_FILAMENT_BACKEND_OPENGL_SHADERCOMPILERSERVICE_H
#define TNT_FILAMENT_BACKEND_OPENGL_SHADERCOMPILERSERVICE_H

#include "gl_headers.h"

#include "CallbackManager.h"
#include "CompilerThreadPool.h"
#include "OpenGLBlobCache.h"

#include <backend/CallbackHandler.h>
#include <backend/DriverEnums.h>
#include <backend/Program.h>

#include <utils/CString.h>
#include <utils/FixedCapacityVector.h>
#include <utils/JobSystem.h>

#include <array>
#include <functional>
#include <memory>
#include <mutex>
#include <tuple>
#include <utility>
#include <vector>

#include <stdint.h>

namespace filament::backend {

class OpenGLDriver;
class OpenGLContext;
class OpenGLPlatform;
class Program;
class CallbackHandler;

/*
 * A class handling shader compilation that supports asynchronous compilation.
 */
class ShaderCompilerService {
    struct OpenGLProgramToken;

public:
    using program_token_t = std::shared_ptr<OpenGLProgramToken>;
    using shaders_t = std::array<GLuint, Program::SHADER_TYPE_COUNT>;
    using shaders_source_t = std::array<utils::CString, Program::SHADER_TYPE_COUNT>;

    explicit ShaderCompilerService(OpenGLDriver& driver);

    ShaderCompilerService(ShaderCompilerService const& rhs) = delete;
    ShaderCompilerService(ShaderCompilerService&& rhs) = delete;
    ShaderCompilerService& operator=(ShaderCompilerService const& rhs) = delete;
    ShaderCompilerService& operator=(ShaderCompilerService&& rhs) = delete;

    ~ShaderCompilerService() noexcept;

    bool isParallelShaderCompileSupported() const noexcept;

    void init() noexcept;
    void terminate() noexcept;

    // creates a program (compile + link) asynchronously if supported
    program_token_t createProgram(utils::CString const& name, Program&& program);

    // Return the GL program, blocks if necessary. The Token is destroyed and becomes invalid.
    GLuint getProgram(program_token_t& token);

    // Must be called at least once per frame.
    void tick();

    // Destroys a valid token and all associated resources. Used to "cancel" a program compilation.
    // This function is not called if `initialize(token)` is already invoked.
    static void terminate(program_token_t& token);

    // stores a user data pointer in the token
    static void setUserData(const program_token_t& token, void* user) noexcept;

    // retrieves the user data pointer stored in the token
    static void* getUserData(const program_token_t& token) noexcept;

    // Issue one callback handle.
    CallbackManager::Handle issueCallbackHandle() const noexcept;

    // Return a callback handle to the callback manager.
    void submitCallbackHandle(CallbackManager::Handle handle) noexcept;

    // call the callback when all active programs are ready
    void notifyWhenAllProgramsAreReady(
            CallbackHandler* handler, CallbackHandler::Callback callback, void* user);

private:
    struct Job {
        template<typename FUNC>
        Job(FUNC&& fn) : fn(std::forward<FUNC>(fn)) {} // NOLINT(*-explicit-constructor)
        Job(std::function<bool(Job const& job)> fn,
                CallbackHandler* handler, void* user, CallbackHandler::Callback callback)
                : fn(std::move(fn)), handler(handler), user(user), callback(callback) {
        }
        std::function<bool(Job const& job)> fn;
        CallbackHandler* handler = nullptr;
        void* user = nullptr;
        CallbackHandler::Callback callback{};
    };

    enum class Mode {
        UNDEFINED,      // init() has not been called yet.
        SYNCHRONOUS,    // synchronous shader compilation
        THREAD_POOL,    // asynchronous shader compilation using a thread-pool (most common)
        ASYNCHRONOUS    // asynchronous shader compilation using KHR_parallel_shader_compile
    };

    OpenGLDriver& mDriver;
    OpenGLBlobCache mBlobCache;
    CallbackManager mCallbackManager;
    CompilerThreadPool mCompilerThreadPool;

    uint32_t mShaderCompilerThreadCount = 0u;
    Mode mMode = Mode::UNDEFINED; // valid after init() is called

    using ContainerType = std::tuple<CompilerPriorityQueue, program_token_t, Job>;
    std::vector<ContainerType> mRunAtNextTickOps;

    GLuint initialize(program_token_t& token);
    void ensureTokenIsReady(program_token_t const& token);

    void runAtNextTick(CompilerPriorityQueue priority, program_token_t const& token,
            Job job) noexcept;
    void executeTickOps() noexcept;
    bool cancelTickOp(program_token_t const& token) noexcept;

    // Compile shaders with the given `shaderSource`. `gl.shaders` is always populated with valid
    // shader IDs after this method. But this doesn't necessarily mean the shaders are successfully
    // compiled. Errors can be checked by calling `checkCompileStatus` later.
    static void compileShaders(OpenGLContext& context, Program::ShaderSource shadersSource,
            utils::FixedCapacityVector<Program::SpecializationConstant> const&
                    specializationConstants,
            bool multiview, program_token_t const& token) noexcept;

    // Check if the shader compilation is completed. You may want to call this when the extension
    // `KHR_parallel_shader_compile` is enabled.
    static bool isCompileCompleted(program_token_t const& token) noexcept;

    // Check compilation status of the shaders and log errors on failure.
    static void checkCompileStatus(program_token_t const& token) noexcept;

    // Create a program by linking the compiled shaders. `gl.program` is always populated with a
    // valid program ID after this method. But this doesn't necessarily mean the program is
    // successfully linked. Errors can be checked by calling `checkLinkStatusAndCleanupShaders`
    // later.
    static void linkProgram(OpenGLContext const& context, program_token_t const& token) noexcept;

    // Check if the program link is completed. You may want to call this when the extension
    // `KHR_parallel_shader_compile` is enabled.
    static bool isLinkCompleted(program_token_t const& token) noexcept;

    // Check link status of the program and log errors on failure. Return the result of the link.
    // Also cleanup shaders regardless of the result.
    static bool checkLinkStatusAndCleanupShaders(program_token_t const& token) noexcept;

    // Try caching the program if we haven't done it yet. Cache it only when the program is valid.
    static void tryCachingProgram(OpenGLBlobCache& cache, OpenGLPlatform& platform,
            program_token_t const& token) noexcept;

    // Cleanup GL resources.
    static void cleanupProgramAndShaders(program_token_t const& token) noexcept;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_OPENGL_SHADERCOMPILERSERVICE_H
