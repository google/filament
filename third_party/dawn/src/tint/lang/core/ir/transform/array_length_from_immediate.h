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

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_ARRAY_LENGTH_FROM_IMMEDIATE_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_ARRAY_LENGTH_FROM_IMMEDIATE_H_

#include <unordered_map>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}

namespace tint::core::ir::transform {
struct ImmediateDataLayout;

/// The capabilities that the transform can support.
const Capabilities kArrayLengthFromImmediateCapabilities{Capability::kAllowDuplicateBindings};

/// The result of running the ArrayLengthFromUniform transform.
struct ArrayLengthFromImmediateResult {
    /// `true` if the transformed module needs the storage buffer sizes immediate data.
    bool needs_storage_buffer_sizes = false;
};

/// ArrayLengthFromImmediates is a transform that replaces calls to the arrayLength() builtin by
/// calculating the array length from the total size of the storage buffer, which is received via
/// immediate blocks.
///
/// The generated immediate blocks will have the form:
/// ```
/// @group(0) @binding(30)
/// struct tint_immediate_data_struct {
///  ...
///    buffer_sizes: array<vec4<u32>, 8>;  // offset is provided via config
// };
/// var<immediate> tint_immediate_data : tint_immediate_data_struct;
/// ```
/// The offset of `buffer_sizes` in the immediate block is provided by config.
/// The transform config also defines the mapping from a storage buffer's `BindingPoint` to the
/// element index that will be used to get the size of that buffer.
///
/// @param module the module to transform
/// @param immediate_data_layout The immediate data layout information.
/// @param bindpoint_to_size_index The map from binding point to an index which holds the size
/// of that buffer.
/// @param buffer_sizes_offset The offset in immediate block where buffer sizes start.
/// @param buffer_sizes_array_elements_num the number of vec4s used to store buffer sizes that will
/// be set into the immediate block.
/// @returns the transform result or failure
/// TODO(crbug.com/366291600): Replace ArrayLengthFromUniform.
Result<ArrayLengthFromImmediateResult> ArrayLengthFromImmediates(
    Module& module,
    const core::ir::transform::ImmediateDataLayout& immediate_data_layout,
    const uint32_t buffer_sizes_offset,
    const uint32_t buffer_sizes_array_elements_num,
    const std::unordered_map<BindingPoint, uint32_t>& bindpoint_to_size_index);

}  // namespace tint::core::ir::transform

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_ARRAY_LENGTH_FROM_IMMEDIATE_H_
