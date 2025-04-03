// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NATIVE_D3D_D3DCOMPILATIONREQUEST_H_
#define SRC_DAWN_NATIVE_D3D_D3DCOMPILATIONREQUEST_H_

#include <d3dcompiler.h>

#include <string_view>

#include "dawn/native/Adapter.h"
#include "dawn/native/CacheRequest.h"
#include "dawn/native/Serializable.h"
#include "dawn/native/d3d/d3d_platform.h"

#include "tint/tint.h"

namespace dawn::native::stream {

// Define no-op serializations for pD3DCompile, IDxcLibrary, and IDxcCompiler3.
// These are output-only interfaces used to generate bytecode.
template <>
inline void Stream<IDxcLibrary*>::Write(Sink*, IDxcLibrary* const&) {}
template <>
inline void Stream<IDxcCompiler3*>::Write(Sink*, IDxcCompiler3* const&) {}
template <>
inline void Stream<pD3DCompile>::Write(Sink*, pD3DCompile const&) {}

}  // namespace dawn::native::stream

namespace dawn::native::d3d {

enum class Compiler { FXC, DXC };

using InterStageShaderVariablesMask = std::bitset<tint::hlsl::writer::kMaxInterStageLocations>;

#define HLSL_COMPILATION_REQUEST_MEMBERS(X)                                                      \
    X(const tint::Program*, inputProgram)                                                        \
    X(std::string_view, entryPointName)                                                          \
    X(SingleShaderStage, stage)                                                                  \
    X(uint32_t, shaderModel)                                                                     \
    X(uint32_t, compileFlags)                                                                    \
    X(Compiler, compiler)                                                                        \
    X(uint64_t, compilerVersion)                                                                 \
    X(std::wstring_view, dxcShaderProfile)                                                       \
    X(std::string_view, fxcShaderProfile)                                                        \
    X(pD3DCompile, d3dCompile)                                                                   \
    X(IDxcLibrary*, dxcLibrary)                                                                  \
    X(IDxcCompiler3*, dxcCompiler)                                                               \
    X(uint32_t, firstIndexOffsetShaderRegister)                                                  \
    X(uint32_t, firstIndexOffsetRegisterSpace)                                                   \
    X(tint::hlsl::writer::Options, tintOptions)                                                  \
    X(std::optional<tint::ast::transform::SubstituteOverride::Config>, substituteOverrideConfig) \
    X(LimitsForCompilationRequest, limits)                                                       \
    X(CacheKey::UnsafeUnkeyedValue<LimitsForCompilationRequest>, adapterSupportedLimits)         \
    X(uint32_t, maxSubgroupSize)                                                                 \
    X(bool, disableSymbolRenaming)                                                               \
    X(bool, dumpShaders)                                                                         \
    X(bool, useTintIR)

#define D3D_BYTECODE_COMPILATION_REQUEST_MEMBERS(X) \
    X(bool, hasShaderF16Feature)                    \
    X(uint32_t, compileFlags)                       \
    X(Compiler, compiler)                           \
    X(uint64_t, compilerVersion)                    \
    X(std::wstring_view, dxcShaderProfile)          \
    X(std::string_view, fxcShaderProfile)           \
    X(pD3DCompile, d3dCompile)                      \
    X(IDxcLibrary*, dxcLibrary)                     \
    X(IDxcCompiler3*, dxcCompiler)

DAWN_SERIALIZABLE(struct, HlslCompilationRequest, HLSL_COMPILATION_REQUEST_MEMBERS){};
#undef HLSL_COMPILATION_REQUEST_MEMBERS

DAWN_SERIALIZABLE(struct,
                  D3DBytecodeCompilationRequest,
                  D3D_BYTECODE_COMPILATION_REQUEST_MEMBERS){};
#undef D3D_BYTECODE_COMPILATION_REQUEST_MEMBERS

#define D3D_COMPILATION_REQUEST_MEMBERS(X)     \
    X(HlslCompilationRequest, hlsl)            \
    X(D3DBytecodeCompilationRequest, bytecode) \
    X(CacheKey::UnsafeUnkeyedValue<dawn::platform::Platform*>, tracePlatform)

DAWN_MAKE_CACHE_REQUEST(D3DCompilationRequest, D3D_COMPILATION_REQUEST_MEMBERS);
#undef D3D_COMPILATION_REQUEST_MEMBERS

}  // namespace dawn::native::d3d

#endif  // SRC_DAWN_NATIVE_D3D_D3DCOMPILATIONREQUEST_H_
