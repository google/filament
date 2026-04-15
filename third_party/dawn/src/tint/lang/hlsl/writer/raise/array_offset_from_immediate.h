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

#ifndef SRC_TINT_LANG_HLSL_WRITER_RAISE_ARRAY_OFFSET_FROM_IMMEDIATE_H_
#define SRC_TINT_LANG_HLSL_WRITER_RAISE_ARRAY_OFFSET_FROM_IMMEDIATE_H_

#include <unordered_map>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/core/ir/transform/prepare_immediate_data.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}

namespace tint::hlsl::writer::raise {

using ImmediateDataLayout = core::ir::transform::ImmediateDataLayout;

/// The capabilities that the transform can support.
const core::ir::Capabilities kArrayOffsetFromImmediateCapabilities{
    core::ir::Capability::kAllow16BitIntegers,
    core::ir::Capability::kAllowClipDistancesOnF32ScalarAndVector,
    core::ir::Capability::kAllowDuplicateBindings,
    core::ir::Capability::kAllowNonCoreTypes,
};

/// ArrayOffsetFromImmediates is a transform that adds an offset to storage buffer loads and stores
/// provided via immediate blocks.
///
/// The generated immediate blocks will have the form:
/// ```
/// struct tint_immediate_data_struct {
///  ...
///    buffer_offsets: array<vec4<u32>, 8>;  // offset is provided via config
/// };
/// var<immediate> tint_immediate_data : tint_immediate_data_struct;
/// ```
/// The offset of `buffer_offsets` in the immediate block is provided by config.
/// The transform config also defines the mapping from a storage buffer's `BindingPoint` to the
/// element index that will be used to get the offset of that buffer.
///
/// @param module the module to transform
/// @param immediate_data_layout The immediate data layout information.
/// @param buffer_offsets_offset The offset in immediate block where buffer offsets start.
/// @param buffer_offsets_array_elements_num the number of vec4s used to store buffer offsets that
/// will be set into the immediate block.
/// @param bindpoint_to_offset_index The map from binding point to an index which holds the offset
/// of that buffer.
/// @returns the transform result or failure
Result<SuccessType> ArrayOffsetFromImmediates(
    core::ir::Module& module,
    const ImmediateDataLayout& immediate_data_layout,
    const uint32_t buffer_offsets_offset,
    const uint32_t buffer_offsets_array_elements_num,
    const std::unordered_map<BindingPoint, uint32_t>& bindpoint_to_offset_index);

}  // namespace tint::hlsl::writer::raise

#endif  // SRC_TINT_LANG_HLSL_WRITER_RAISE_ARRAY_OFFSET_FROM_IMMEDIATE_H_
