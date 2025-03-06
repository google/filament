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

MetalShaderCompiler::MetalShaderCompiler(id<MTLDevice> device, MetalDriver& driver, Mode mode)
        : mDevice(device),
          mCallbackManager(driver),
          mMode(mode) {

}

void MetalShaderCompiler::init() noexcept {
    const uint32_t poolSize = 1;
    if (mMode == Mode::ASYNCHRONOUS) {
        mCompilerThreadPool.init(poolSize, []() {}, []() {});
    }
}

void MetalShaderCompiler::terminate() noexcept {
    if (mMode == Mode::ASYNCHRONOUS) {
        mCompilerThreadPool.terminate();
    }
    mCallbackManager.terminate();
}

bool MetalShaderCompiler::isParallelShaderCompileSupported() const noexcept {
    return mMode == Mode::ASYNCHRONOUS;
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

        NSError* error = nil;
        id<MTLLibrary> library = nil;
        switch (program.getShaderLanguage()) {
            case ShaderLanguage::MSL: {
                // By default, Metal uses the most recent language version.
                MTLCompileOptions* options = [MTLCompileOptions new];

                // Disable Fast Math optimizations.
                // This ensures that operations adhere to IEEE standards for floating-point
                // arithmetic, which is crucial for half precision floats in scenarios where fast
                // math optimizations lead to inaccuracies, such as in handling special values like
                // NaN or Infinity.
                options.fastMathEnabled = NO;

                assert_invariant(source[source.size() - 1] == '\0');
                // the shader string is null terminated and the length includes the null character
                NSString* objcSource = [[NSString alloc] initWithBytes:source.data()
                                                                length:source.size() - 1
                                                              encoding:NSUTF8StringEncoding];
                library = [device newLibraryWithSource:objcSource options:options error:&error];
                break;
            }
            case ShaderLanguage::METAL_LIBRARY: {
                dispatch_data_t data = dispatch_data_create(source.data(), source.size(),
                        dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                        DISPATCH_DATA_DESTRUCTOR_DEFAULT);
                library = [device newLibraryWithData:data error:&error];
                break;
            }
            case ShaderLanguage::ESSL1:
            case ShaderLanguage::ESSL3:
            case ShaderLanguage::SPIRV:
            case ShaderLanguage::WGSL:
                break;
        }

        if (library == nil) {
            NSString* errorMessage = @"unknown error";
            if (error) {
                auto description =
                        [error.localizedDescription cStringUsingEncoding:NSUTF8StringEncoding];
                utils::slog.w << description << utils::io::endl;
                errorMessage = error.localizedDescription;
            }
            PANIC_LOG("Failed to compile Metal program.");
            NSString* programName = [NSString stringWithFormat:@"%s", program.getName().c_str_safe()];
            return MetalFunctionBundle::error(errorMessage, programName);
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
        if (function == nil) {
            // If the library loads but functions within it fail to load, it usually means the
            // GPU backend crashed. (This can happen if it's a Metallib shader that was compiled
            // with a minimum iOS version that's newer than this device.)
            NSString* errorMessage = @"unknown error";
            if (error) {
                auto description =
                        [error.localizedDescription cStringUsingEncoding:NSUTF8StringEncoding];
                utils::slog.w << description << utils::io::endl;
                errorMessage = error.localizedDescription;
            }
            PANIC_LOG("Failed to load main0 in Metal program.");
            NSString* programName =
                [NSString stringWithFormat:@"%s::main0", program.getName().c_str_safe()];
            return MetalFunctionBundle::error(errorMessage, programName);
        }

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
        return MetalFunctionBundle::raster(fragmentFunction, vertexFunction);
    }

    if (isComputeProgram) {
        return MetalFunctionBundle::compute(computeFunction);
    }

    // Should never reach here.
    return MetalFunctionBundle::none();
}

MetalShaderCompiler::program_token_t MetalShaderCompiler::createProgram(
        CString const& name, Program&& program) {
    auto token = std::make_shared<MetalProgramToken>(*this);

    token->handle = mCallbackManager.get();

    switch (mMode) {
        case Mode::ASYNCHRONOUS: {
            CompilerPriorityQueue const priorityQueue = program.getPriorityQueue();
            mCompilerThreadPool.queue(priorityQueue, token,
                    [this, name, device = mDevice, program = std::move(program), token]() {
                        @autoreleasepool {
                            MetalFunctionBundle compiledProgram = compileProgram(program, device);
                            token->set(compiledProgram);
                            mCallbackManager.put(token->handle);
                        }
                    });

            break;
        }

        case Mode::SYNCHRONOUS: {
            MetalFunctionBundle compiledProgram = compileProgram(program, mDevice);
            token->set(compiledProgram);
            mCallbackManager.put(token->handle);
            break;
        }
    }

    return token;
}

MetalShaderCompiler::MetalFunctionBundle MetalShaderCompiler::getProgram(program_token_t& token) {
    assert_invariant(token);

    if (mMode == Mode::ASYNCHRONOUS) {
        if (!token->isReady()) {
            auto job = mCompilerThreadPool.dequeue(token);
            if (job) {
                job();
            }
        }
    }

    // The job isn't guaranteed to have finished yet. We may have failed to dequeue it above,
    // which means it's currently running. In that case get() will block until it finishes.

    MetalShaderCompiler::MetalFunctionBundle program = token->get();
    token = nullptr;
    return program;
}

void MetalShaderCompiler::notifyWhenAllProgramsAreReady(
        CallbackHandler* handler, CallbackHandler::Callback callback, void* user) {
    mCallbackManager.setCallback(handler, callback, user);
}

UTILS_NOINLINE
void MetalShaderCompiler::MetalFunctionBundle::validate() const {
    if (UTILS_UNLIKELY(std::holds_alternative<Error>(mPrograms))) {
        auto [errorMessage, programName] = std::get<Error>(mPrograms);
        NSString* reason =
                [NSString stringWithFormat:
                        @"Attempting to draw with an id<MTLFunction> that failed to compile.\n"
                        @"Program: %@\n"
                        @"%@", programName, errorMessage];
        [[NSException exceptionWithName:@"MetalCompilationFailure"
                                reason:reason
                              userInfo:nil] raise];
    } else if (UTILS_UNLIKELY(std::holds_alternative<None>(mPrograms))) {
        NSString* reason = @"Attempting to draw with an empty id<MTLFunction>.";
        [[NSException exceptionWithName:@"MetalEmptyFunctionBundle"
                                reason:reason
                              userInfo:nil] raise];
    }
}

} // namespace filament::backend
