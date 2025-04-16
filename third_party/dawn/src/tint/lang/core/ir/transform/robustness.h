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

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_ROBUSTNESS_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_ROBUSTNESS_H_

#include <unordered_set>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/utils/reflection.h"
#include "src/tint/utils/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}

namespace tint::core::ir::transform {

/// Configuration options that control when to clamp accesses.
struct RobustnessConfig {
    /// Should non-pointer accesses be clamped?
    bool clamp_value = true;

    /// Should texture accesses be clamped?
    bool clamp_texture = true;

    /// Should accesses to pointers with the 'function' address space be clamped?
    bool clamp_function = true;
    /// Should accesses to pointers with the 'private' address space be clamped?
    bool clamp_private = true;
    /// Should accesses to pointers with the 'push_constant' address space be clamped?
    bool clamp_push_constant = true;
    /// Should accesses to pointers with the 'storage' address space be clamped?
    bool clamp_storage = true;
    /// Should accesses to pointers with the 'uniform' address space be clamped?
    bool clamp_uniform = true;
    /// Should accesses to pointers with the 'workgroup' address space be clamped?
    bool clamp_workgroup = true;

    /// Bindings that should always be ignored.
    std::unordered_set<tint::BindingPoint> bindings_ignored;

    /// Should the transform skip index clamping on runtime-sized arrays?
    bool disable_runtime_sized_array_index_clamping = false;

    /// Reflection for this class
    TINT_REFLECT(RobustnessConfig,
                 clamp_value,
                 clamp_texture,
                 clamp_function,
                 clamp_private,
                 clamp_push_constant,
                 clamp_storage,
                 clamp_uniform,
                 clamp_workgroup,
                 bindings_ignored,
                 disable_runtime_sized_array_index_clamping);
};

/// Robustness is a transform that prevents out-of-bounds memory accesses.
/// @param module the module to transform
/// @param config the robustness configuration
/// @returns success or failure
Result<SuccessType> Robustness(Module& module, const RobustnessConfig& config);

}  // namespace tint::core::ir::transform

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_ROBUSTNESS_H_
