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

#ifndef SRC_TINT_API_COMMON_RESOURCE_TABLE_CONFIG_H_
#define SRC_TINT_API_COMMON_RESOURCE_TABLE_CONFIG_H_

#include <unordered_map>
#include <vector>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/api/common/resource_type.h"
#include "src/tint/utils/reflection.h"

namespace tint {

// Configuration for the resource table transform.
//
// The resource table transform assumes that for each resource table entry there will be an
// entry in the `bindings` hash map. That binding will provide information on the storage buffer
// attached for the resource table. Specifically, the storage buffer should have a format of:
//
// ```
// struct SB {
//   array_length: u32,
//   bindings: array<u32>,  // Has `array_length` entries (doesn't include the default bindings).
// }
// ```
//
// The values used in the `bindings` are from the `u32` conversion of the `ResourceType` enum
// entries.
//
struct ResourceTableConfig {
    // The binding point for the resource_table. This is a post-remapping binding point.
    BindingPoint resource_table_binding;

    // The binding point for the supporting storage buffer. This is a post-remapping binding point.
    // TODO(crbug.com/435317394): Rename to metadata_buffer_binding
    BindingPoint storage_buffer_binding;

    // The ordering of default bindings which are placed after the user bindings. These will be used
    // as the index to lookup the given `ResourceType` if the user accesses with an incorrect
    // texture type (or out of bounds).
    //
    // These `default_binding_type_order` entries will be used when we need to substitute in a
    // default binding. So, we assume that Dawn is providing the resource table memory as:
    //
    // `[user 1 (2d_i32), user 2 (3d_f32), user 3 (1d_u32), 1d_u32_default, 2d_f32_default,
    //  3d_f32_default]`
    //
    // We would then have `default_binding_type_order` entries of:
    //
    // `[1d_u32, 2d_f32, 3d_f32]`
    //
    // Then when we compile a `getResource<T>(i)` call, we will have created the storage buffer,
    // call it `metadata` in this case:
    //
    // ```
    // const len = metadata.array_length;
    // if (i < len && type_to_u32(T) == metadata.bindings[i]) {
    //   return bindings[i];
    // }
    // return bindings[len + index_in_binding_order(T)];
    // ```
    //
    std::vector<ResourceType> default_binding_type_order;

    TINT_REFLECT(ResourceTableConfig,
                 resource_table_binding,
                 storage_buffer_binding,
                 default_binding_type_order);
    bool operator==(const ResourceTableConfig&) const = default;
};

}  // namespace tint

#endif  // SRC_TINT_API_COMMON_RESOURCE_TABLE_CONFIG_H_
