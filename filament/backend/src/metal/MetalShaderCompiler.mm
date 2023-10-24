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

#include "MetalShaderCompiler.h"

#include "MetalDriver.h"

#include <backend/Program.h>

#include <utils/JobSystem.h>
#include <utils/Mutex.h>

#include <chrono>

namespace filament::backend {

using namespace utils;

struct MetalShaderCompiler::MetalProgramToken : ProgramToken {

    MetalProgramToken(MetalShaderCompiler& compiler) noexcept
            : compiler(compiler) {
    }
    ~MetalProgramToken() override;

    void set(MetalFunctionBundle p) noexcept {
        std::unique_lock const l(lock);
        std::swap(program, p);
        signaled = true;
        cond.notify_one();
    }

    MetalFunctionBundle get() const noexcept {
        std::unique_lock l(lock);
        cond.wait(l, [this](){ return signaled; });
        return program;
    }

    void wait() const noexcept {
        std::unique_lock l(lock);
        cond.wait(l, [this]() { return signaled; });
    }

    bool isReady() const noexcept {
        std::unique_lock l(lock);
        using namespace std::chrono_literals;
        return cond.wait_for(l, 0s, [this]() { return signaled; });
    }

    MetalShaderCompiler& compiler;
    CallbackManager::Handle handle{};
    MetalFunctionBundle program{};
    mutable utils::Mutex lock;
    mutable utils::Condition cond;
    bool signaled = false;
};

MetalShaderCompiler::MetalProgramToken::~MetalProgramToken() = default;

MetalShaderCompiler::MetalShaderCompiler(id<MTLDevice> device, MetalDriver& driver)
        : mDevice(device),
          mCallbackManager(driver) {

}

void MetalShaderCompiler::init() noexcept {
    const uint32_t poolSize = 2;
    mCompilerThreadPool.init(poolSize, []() {}, []() {});
}

void MetalShaderCompiler::terminate() noexcept {
    mCompilerThreadPool.terminate();
    mCallbackManager.terminate();
}

/* static */ MetalShaderCompiler::MetalFunctionBundle MetalShaderCompiler::compileProgram(
        const Program& program, id<MTLDevice> device) {
    std::array<id<MTLFunction>, Program::SHADER_TYPE_COUNT> functions = { nil };
    const auto& sources = program.getShadersSource();
    for (size_t i = 0; i < Program::SHADER_TYPE_COUNT; i++) {
        const auto& source = sources[i];
        // It's okay for some shaders to be empty, they shouldn't be used in any draw calls.
        if (source.empty()) {
            continue;
        }

        assert_invariant(source[source.size() - 1] == '\0');

        // the shader string is null terminated and the length includes the null character
        NSString* objcSource = [[NSString alloc] initWithBytes:source.data()
                                                        length:source.size() - 1
                                                      encoding:NSUTF8StringEncoding];
        NSError* error = nil;
        // When options is nil, Metal uses the most recent language version available.
        id<MTLLibrary> library = [device newLibraryWithSource:objcSource
                                                      options:nil
                                                        error:&error];
        if (library == nil) {
            if (error) {
                auto description =
                        [error.localizedDescription cStringUsingEncoding:NSUTF8StringEncoding];
                utils::slog.w << description << utils::io::endl;
            }
            PANIC_LOG("Failed to compile Metal program.");
            return {};
        }

        MTLFunctionConstantValues* constants = [MTLFunctionConstantValues new];
        auto const& specializationConstants = program.getSpecializationConstants();
        for (auto const& sc : specializationConstants) {
            const std::array<MTLDataType, 3> types{
                    MTLDataTypeInt, MTLDataTypeFloat, MTLDataTypeBool };
            std::visit([&sc, constants, type = types[sc.value.index()]](auto&& arg) {
                [constants setConstantValue:&arg
                                       type:type
                                    atIndex:sc.id];
            }, sc.value);
        }

        id<MTLFunction> function = [library newFunctionWithName:@"main0"
                                                 constantValues:constants
                                                          error:&error];
        if (!program.getName().empty()) {
            function.label = @(program.getName().c_str());
        }
        assert_invariant(function);
        functions[i] = function;
    }

    static_assert(Program::SHADER_TYPE_COUNT == 3,
            "Only vertex, fragment, and/or compute shaders expected.");
    id<MTLFunction> vertexFunction = functions[0];
    id<MTLFunction> fragmentFunction = functions[1];
    id<MTLFunction> computeFunction = functions[2];
    const bool isRasterizationProgram = vertexFunction != nil && fragmentFunction != nil;
    const bool isComputeProgram = computeFunction != nil;
    // The program must be either a rasterization program XOR a compute program.
    assert_invariant(isRasterizationProgram != isComputeProgram);

    if (isRasterizationProgram) {
        return {fragmentFunction, vertexFunction};
    }

    if (isComputeProgram) {
        return MetalFunctionBundle{computeFunction};
    }

    return {};
}

MetalShaderCompiler::program_token_t MetalShaderCompiler::createProgram(
        CString const& name, Program&& program) {
    auto token = std::make_shared<MetalProgramToken>(*this);

    token->handle = mCallbackManager.get();

    CompilerPriorityQueue const priorityQueue = program.getPriorityQueue();
    mCompilerThreadPool.queue(priorityQueue, token,
            [this, name, device = mDevice, program = std::move(program), token]() {
                int sleepTime = atoi(name.c_str());
                sleep(sleepTime);

                MetalFunctionBundle compiledProgram = compileProgram(program, device);

                token->set(compiledProgram);
                mCallbackManager.put(token->handle);
            });

    return token;
}

MetalShaderCompiler::MetalFunctionBundle MetalShaderCompiler::getProgram(program_token_t& token) {
    assert_invariant(token);

    if (!token->isReady()) {
        auto job = mCompilerThreadPool.dequeue(token);
        if (job) {
            job();
        }
    }

    MetalShaderCompiler::MetalFunctionBundle program = token->get();

    token = nullptr;

    return program;
}

/* static */ void MetalShaderCompiler::terminate(program_token_t& token) {
    assert_invariant(token);

    auto job = token->compiler.mCompilerThreadPool.dequeue(token);
    if (!job) {
        // The job is being executed right now (or has already executed).
        token->wait();
    } else {
        // The job has not executed yet.
        token->compiler.mCallbackManager.put(token->handle);
    }

    token.reset();
}

void MetalShaderCompiler::notifyWhenAllProgramsAreReady(
        CallbackHandler* handler, CallbackHandler::Callback callback, void* user) {
    mCallbackManager.setCallback(handler, callback, user);
}

} // namespace filament::backend
