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

#ifndef TNT_FILAMENT_BACKEND_METAL_METALSHADERCOMPILER_H
#define TNT_FILAMENT_BACKEND_METAL_METALSHADERCOMPILER_H

#include "CompilerThreadPool.h"

#include "CallbackManager.h"

#include <backend/CallbackHandler.h>
#include <backend/Program.h>

#include <utils/CString.h>

#include <Metal/Metal.h>

#include <array>
#include <memory>

namespace filament::backend {

class MetalDriver;

class MetalShaderCompiler {
    struct MetalProgramToken;

public:
    class MetalFunctionBundle {
    public:
        MetalFunctionBundle() = default;
        MetalFunctionBundle(id<MTLFunction> fragment, id<MTLFunction> vertex)
            : functions{fragment, vertex} {
            assert_invariant(fragment && vertex);
            assert_invariant(fragment.functionType == MTLFunctionTypeFragment);
            assert_invariant(vertex.functionType == MTLFunctionTypeVertex);
        }
        explicit MetalFunctionBundle(id<MTLFunction> compute) : functions{compute, nil} {
            assert_invariant(compute);
            assert_invariant(compute.functionType == MTLFunctionTypeKernel);
        }

        std::pair<id<MTLFunction>, id<MTLFunction>> getRasterFunctions() const noexcept {
            assert_invariant(functions[0].functionType == MTLFunctionTypeFragment);
            assert_invariant(functions[1].functionType == MTLFunctionTypeVertex);
            return {functions[0], functions[1]};
        }

        id<MTLFunction> getComputeFunction() const noexcept {
            assert_invariant(functions[0].functionType == MTLFunctionTypeKernel);
            return functions[0];
        }

        explicit operator bool() const { return functions[0] != nil; }

    private:
        // Can hold two functions, either:
        // - fragment and vertex (for rasterization pipelines)
        // - compute (for compute pipelines)
        id<MTLFunction> functions[2] = {nil, nil};
    };

    using program_token_t = std::shared_ptr<MetalProgramToken>;

    explicit MetalShaderCompiler(id<MTLDevice> device, MetalDriver& driver);

    MetalShaderCompiler(MetalShaderCompiler const& rhs) = delete;
    MetalShaderCompiler(MetalShaderCompiler&& rhs) = delete;
    MetalShaderCompiler& operator=(MetalShaderCompiler const& rhs) = delete;
    MetalShaderCompiler& operator=(MetalShaderCompiler&& rhs) = delete;

    void init() noexcept;
    void terminate() noexcept;

    // Creates a program asynchronously
    program_token_t createProgram(utils::CString const& name, Program&& program);

    // Returns the functions, blocking if necessary. The Token is destroyed and becomes invalid.
    MetalFunctionBundle getProgram(program_token_t& token);

    // Destroys a valid token and all associated resources. Used to "cancel" a program compilation.
    static void terminate(program_token_t& token);

    void notifyWhenAllProgramsAreReady(
            CallbackHandler* handler, CallbackHandler::Callback callback, void* user);

private:
    static MetalFunctionBundle compileProgram(const Program& program, id<MTLDevice> device);

    CompilerThreadPool mCompilerThreadPool;
    id<MTLDevice> mDevice;
    CallbackManager mCallbackManager;
};

} // namespace filament::backend

#endif  // TNT_FILAMENT_BACKEND_METAL_METALSHADERCOMPILER_H
