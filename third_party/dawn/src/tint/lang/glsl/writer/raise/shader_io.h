// Copyright 2024 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_GLSL_WRITER_RAISE_SHADER_IO_H_
#define SRC_TINT_LANG_GLSL_WRITER_RAISE_SHADER_IO_H_

#include <string>
#include <unordered_set>

#include "src/tint/lang/core/ir/transform/prepare_push_constants.h"
#include "src/tint/lang/glsl/writer/common/options.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/result/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}

namespace tint::glsl::writer::raise {

/// ShaderIOConfig describes the set of configuration options for the ShaderIO transform.
struct ShaderIOConfig {
    /// push constant layout information
    const core::ir::transform::PushConstantLayout& push_constant_layout;

    /// offsets for clamping frag depth
    std::optional<Options::RangeOffsets> depth_range_offsets{};

    /// locations of vertex input variables to apply BGRA swizzle to.
    std::unordered_set<uint32_t> bgra_swizzle_locations{};
};

/// ShaderIO is a transform that moves each entry point function's parameters and return value to
/// global variables to prepare them for GLSL codegen.
/// @param module the module to transform
/// @param config the configuration
/// @returns success or failure
Result<SuccessType> ShaderIO(core::ir::Module& module, const ShaderIOConfig& config);

}  // namespace tint::glsl::writer::raise

#endif  // SRC_TINT_LANG_GLSL_WRITER_RAISE_SHADER_IO_H_
