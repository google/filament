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

#include "src/tint/lang/core/ir/transform/resource_table_helper.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/type/resource_type.h"

namespace tint::core::ir::transform {

std::optional<ResourceTableConfig> GenerateResourceTableConfig(Module& mod) {
    ResourceTableConfig cfg{
        .resource_table_binding = BindingPoint{.group = 43, .binding = 51},
        .storage_buffer_binding = BindingPoint{.group = 42, .binding = 52},
        .default_binding_type_order = {},
    };

    std::vector<ResourceType> default_binding_type_order;

    for (auto* inst : mod.Instructions()) {
        auto* call = inst->As<core::ir::CoreBuiltinCall>();
        if (!call) {
            continue;
        }

        if (call->Func() != core::BuiltinFn::kGetResource &&
            call->Func() != core::BuiltinFn::kHasResource) {
            continue;
        }
        auto exp = call->ExplicitTemplateParams();
        TINT_IR_ASSERT(mod, exp.Length() == 1);

        default_binding_type_order.push_back(type::TypeToResourceType(exp[0]));
    }
    // If we found any resource uses, then we can just return an empty config.
    if (default_binding_type_order.empty()) {
        return {};
    }

    // Sort so we get stable generated results
    std::sort(default_binding_type_order.begin(), default_binding_type_order.end());
    cfg.default_binding_type_order = std::move(default_binding_type_order);

    return cfg;
}

}  // namespace tint::core::ir::transform
