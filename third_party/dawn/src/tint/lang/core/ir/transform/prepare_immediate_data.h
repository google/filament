// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_PREPARE_IMMEDIATE_DATA_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_PREPARE_IMMEDIATE_DATA_H_

#include <map>

#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/reflection.h"
#include "src/tint/utils/result.h"
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

/// A descriptor for an internal immediate constant.
struct InternalImmediateData {
    Symbol name;
    const core::type::Type* type = nullptr;
};

/// A struct that describes the layout of the generated immediate data structure.
struct ImmediateDataLayout {
    /// The immediate data variable.
    core::ir::Var* var = nullptr;

    /// A map from member offset to member index.
    Hashmap<uint32_t, uint32_t, 6> offset_to_index;

    /// @returns the member index of the constant at @p offset
    uint32_t IndexOf(uint32_t offset) const {
        auto itr = offset_to_index.Get(offset);
        TINT_ASSERT(itr);
        return *itr.value;
    }
};

/// The set of polyfills that should be applied.
struct PrepareImmediateDataConfig {
    /// Add an internal immediate data to the map.
    void AddInternalImmediateData(uint32_t offset, Symbol name, const core::type::Type* type) {
        internal_immediate_data.emplace(offset, InternalImmediateData{name, type});
    }

    /// The ordered map from offset to internally used constant descriptor.
    std::map<uint32_t, InternalImmediateData> internal_immediate_data{};

    /// Reflection for this class.
    TINT_REFLECT(PrepareImmediateDataConfig, internal_immediate_data);
};

/// PrepareImmediateData is a transform that sets up the structure and variable used for immediate
/// data to combine both user-defined and internally used immediate data values.
/// @param module the module to transform
/// @param config the transform config
/// @returns the generated immediate constant layout or failure
Result<ImmediateDataLayout> PrepareImmediateData(Module& module,
                                                 const PrepareImmediateDataConfig& config);

}  // namespace tint::core::ir::transform

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_PREPARE_IMMEDIATE_DATA_H_
