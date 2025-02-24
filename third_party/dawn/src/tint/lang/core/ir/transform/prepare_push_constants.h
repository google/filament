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

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_PREPARE_PUSH_CONSTANTS_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_PREPARE_PUSH_CONSTANTS_H_

#include <map>

#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/reflection.h"
#include "src/tint/utils/result/result.h"
#include "src/tint/utils/symbol/symbol.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
class Var;
}  // namespace tint::core::ir
namespace tint::core::type {
class Type;
}

namespace tint::core::ir::transform {

/// A descriptor for an internal push constant.
struct InternalPushConstant {
    Symbol name;
    const core::type::Type* type = nullptr;
};

/// A struct that describes the layout of the generated push constant structure.
struct PushConstantLayout {
    /// The push constant variable.
    core::ir::Var* var = nullptr;

    /// A map from member offset to member index.
    Hashmap<uint32_t, uint32_t, 4> offset_to_index;

    /// @returns the member index of the constant at @p offset
    uint32_t IndexOf(uint32_t offset) const {
        auto itr = offset_to_index.Get(offset);
        TINT_ASSERT(itr);
        return *itr.value;
    }
};

/// The set of polyfills that should be applied.
struct PreparePushConstantsConfig {
    /// Add an internal constant to the map.
    void AddInternalConstant(uint32_t offset, Symbol name, const core::type::Type* type) {
        internal_constants.emplace(offset, InternalPushConstant{name, type});
    }

    /// The ordered map from offset to internally used constant descriptor.
    std::map<uint32_t, InternalPushConstant> internal_constants{};

    /// Reflection for this class.
    TINT_REFLECT(PreparePushConstantsConfig, internal_constants);
};

/// PreparePushConstants is a transform that sets up the structure and variable used for push
/// constants to combine both user-defined and internally used push constant values.
/// @param module the module to transform
/// @param config the transform config
/// @returns the generated push constant layout or failure
Result<PushConstantLayout> PreparePushConstants(Module& module,
                                                const PreparePushConstantsConfig& config);

}  // namespace tint::core::ir::transform

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_PREPARE_PUSH_CONSTANTS_H_
