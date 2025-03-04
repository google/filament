// Copyright 2021 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_HLSL_VALIDATE_VALIDATE_H_
#define SRC_TINT_LANG_HLSL_VALIDATE_VALIDATE_H_

#include <string>
#include <utility>
#include <vector>

#include "src/tint/lang/wgsl/ast/pipeline_stage.h"

// Forward declarations
namespace tint {
class Program;
}  // namespace tint

namespace tint::hlsl::validate {

using EntryPointList = std::vector<std::pair<std::string, ast::PipelineStage>>;

/// Name of the FXC compiler DLL
static constexpr const char kFxcDLLName[] = "d3dcompiler_47.dll";

#if TINT_BUILD_IS_WIN
static constexpr const char* kDxcDLLName = "dxcompiler.dll";
#elif TINT_BUILD_IS_LINUX
static constexpr const char* kDxcDLLName = "libdxcompiler.so";
#elif TINT_BUILD_IS_MAC
static constexpr const char* kDxcDLLName = "libdxcompiler.dylib";
#else
static constexpr const char* kDxcDLLName = "Invalid";
#endif

/// The return structure of Validate()
struct Result {
    /// True if validation passed
    bool failed = false;
    /// Output of DXC.
    std::string output;
};

/// Hlsl attempts to compile the shader with DXC, verifying that the shader
/// compiles successfully.
/// @param dxc_path path to DXC
/// @param source the generated HLSL source
/// @param entry_points the list of entry points to validate
/// @param require_16bit_types set to `true` to require the support of float16 in DXC
/// @param hlsl_shader_model the shader model of the HLSL source
/// @return the result of the compile
Result ValidateUsingDXC(const std::string& dxc_path,
                        const std::string& source,
                        const EntryPointList& entry_points,
                        bool require_16bit_types,
                        uint32_t hlsl_shader_model);

#ifdef _WIN32
/// Hlsl attempts to compile the shader with FXC, verifying that the shader
/// compiles successfully.
/// @param fxc_path path to the FXC DLL
/// @param source the generated HLSL source
/// @param entry_points the list of entry points to validate
/// @return the result of the compile
Result ValidateUsingFXC(const std::string& fxc_path,
                        const std::string& source,
                        const EntryPointList& entry_points);
#endif  // _WIN32

}  // namespace tint::hlsl::validate

#endif  // SRC_TINT_LANG_HLSL_VALIDATE_VALIDATE_H_
