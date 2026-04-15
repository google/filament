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

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_RESOURCE_TABLE_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_RESOURCE_TABLE_H_

#include <vector>

#include "src/tint/api/common/resource_table_config.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}  // namespace tint::core::ir

namespace tint::core::ir::transform {

// Backend specific override methods for resource table
class ResourceTableHelper {
  public:
    virtual ~ResourceTableHelper();

    // Returns a map of types to the var which is used to access the memory of that type
    virtual Hashmap<const core::type::Type*, core::ir::Var*, 4> GenerateVars(
        core::ir::Builder& b,
        const BindingPoint& bp,
        const std::vector<ResourceType>& types) const = 0;
};

/// This transform updates the provided IR to support resource_table restrictions/requirements.
///
/// We re-write the `getResource` and `hasResource` calls to use the provided storage buffer to
/// validate the requested types.
///
/// @param module the module to transform
/// @param config the transform configuration
/// @param helper the resource binding helper
/// @returns success or failure
Result<SuccessType> ResourceTable(core::ir::Module& module,
                                  const std::optional<ResourceTableConfig>& config,
                                  ResourceTableHelper* helper);

}  // namespace tint::core::ir::transform

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_RESOURCE_TABLE_H_
