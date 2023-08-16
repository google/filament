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

#include <backend/CallbackHandler.h>
#include <backend/Program.h>

#include <utils/CString.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Invocable.h>
#include <utils/JobSystem.h>

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

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

    explicit ShaderCompilerService(OpenGLDriver& driver);

    ShaderCompilerService(ShaderCompilerService const& rhs) = delete;
    ShaderCompilerService(ShaderCompilerService&& rhs) = delete;
    ShaderCompilerService& operator=(ShaderCompilerService const& rhs) = delete;
    ShaderCompilerService& operator=(ShaderCompilerService&& rhs) = delete;

    ~ShaderCompilerService() noexcept;

    void init() noexcept;
    void terminate() noexcept;

    // creates a program (compile + link) asynchronously if supported
    program_token_t createProgram(utils::CString const& name, Program&& program);

    // Return the GL program, blocks if necessary. The Token is destroyed and becomes invalid.
    GLuint getProgram(program_token_t& token);

    // Must be called at least once per frame.
    void tick();

    // Destroys a valid token and all associated resources. Used to "cancel" a program compilation.
    static void terminate(program_token_t& token);

    // stores a user data pointer in the token
    static void setUserData(const program_token_t& token, void* user) noexcept;

    // retrieves the user data pointer stored in the token
    static void* getUserData(const program_token_t& token) noexcept;

    // call the callback when all active programs are ready
    void notifyWhenAllProgramsAreReady(
            CallbackHandler* handler, CallbackHandler::Callback callback, void* user);

private:
    OpenGLDriver& mDriver;
    CallbackManager mCallbackManager;
    CompilerThreadPool mCompilerThreadPool;

    const bool KHR_parallel_shader_compile;
    uint32_t mShaderCompilerThreadCount = 0u;

    GLuint initialize(ShaderCompilerService::program_token_t& token) noexcept;

    static void getProgramFromCompilerPool(program_token_t& token) noexcept;

        static void compileShaders(
            OpenGLContext& context,
            Program::ShaderSource shadersSource,
            utils::FixedCapacityVector<Program::SpecializationConstant> const& specializationConstants,
            std::array<GLuint, Program::SHADER_TYPE_COUNT>& outShaders,
            std::array<utils::CString, Program::SHADER_TYPE_COUNT>& outShaderSourceCode) noexcept;

    static std::string_view process_GOOGLE_cpp_style_line_directive(OpenGLContext& context,
            char* source, size_t len) noexcept;

    static std::string_view process_ARB_shading_language_packing(OpenGLContext& context) noexcept;

    static std::array<std::string_view, 2> splitShaderSource(std::string_view source) noexcept;

    static GLuint linkProgram(OpenGLContext& context,
            std::array<GLuint, Program::SHADER_TYPE_COUNT> shaders,
            utils::FixedCapacityVector<std::pair<utils::CString, uint8_t>> const& attributes) noexcept;

    static bool checkProgramStatus(program_token_t const& token) noexcept;

    struct Job {
        template<typename FUNC>
        Job(FUNC&& fn) : fn(std::forward<FUNC>(fn)) {}
        Job(std::function<bool(Job const& job)> fn,
                CallbackHandler* handler, void* user, CallbackHandler::Callback callback)
                : fn(std::move(fn)), handler(handler), user(user), callback(callback) {
        }
        std::function<bool(Job const& job)> fn;
        CallbackHandler* handler = nullptr;
        void* user = nullptr;
        CallbackHandler::Callback callback{};
    };

    void runAtNextTick(CompilerPriorityQueue priority,
            const program_token_t& token, Job job) noexcept;
    void executeTickOps() noexcept;
    void cancelTickOp(program_token_t token) noexcept;
    // order of insertion is important

    using ContainerType = std::tuple<CompilerPriorityQueue, program_token_t, Job>;
    std::vector<ContainerType> mRunAtNextTickOps;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_OPENGL_SHADERCOMPILERSERVICE_H
