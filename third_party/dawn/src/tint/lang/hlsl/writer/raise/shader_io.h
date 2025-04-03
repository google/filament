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

#ifndef SRC_TINT_LANG_HLSL_WRITER_RAISE_SHADER_IO_H_
#define SRC_TINT_LANG_HLSL_WRITER_RAISE_SHADER_IO_H_

#include <bitset>
#include <optional>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/utils/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}

namespace tint::hlsl::writer::raise {

struct ShaderIOConfig {
    /// The binding point to use for the num_workgroups generated uniform buffer. If it contains
    /// no value, a free binding point will be used. Specifically, binding 0 of the largest used
    /// group plus 1 is used if at least one resource is bound, otherwise group 0 binding 0 is used.
    std::optional<BindingPoint> num_workgroups_binding;

    /// The binding point to use for the first_index_offset uniform buffer. If set, and if a vertex
    /// entry point contains a vertex_index or instance_index input parameter (or both), this
    /// transform will add a uniform buffer with both indices, and will add the offsets to the input
    /// variables, respectively.
    std::optional<BindingPoint> first_index_offset_binding;

    /// If one doesn't exist, adds a @position member to the input struct as the last member.
    /// This is used for PixelLocal, for which Dawn requires such a member in the final HLSL shader.
    bool add_input_position_member = false;

    /// Set to `true` to truncate location variables not found in `interstage_locations`
    bool truncate_interstage_variables = false;

    /// Indicate which interstage io locations are actually used by the later stage.
    /// There can be at most 30 user defined interstage variables with locations.
    std::bitset<30> interstage_locations;
};

/// ShaderIO is a transform that prepares entry point inputs and outputs for HLSL codegen.
/// For HLSL, all entry point input parameters are moved to a struct and passed in as a single
/// entry point parameter, and all outputs are wrapped in a struct and returned by the entry point.
/// @param module the module to transform
/// @returns success or failure
Result<SuccessType> ShaderIO(core::ir::Module& module, const ShaderIOConfig& config);

}  // namespace tint::hlsl::writer::raise

#endif  // SRC_TINT_LANG_HLSL_WRITER_RAISE_SHADER_IO_H_
