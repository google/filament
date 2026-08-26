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
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/resource_type.h"

namespace tint::core::ir::transform {

std::optional<ResourceTableConfig> GenerateResourceTableConfig(Module& mod,
                                                               bool treat_samplers_as_filtering) {
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
        TINT_IR_ASSERT(mod, std::holds_alternative<const core::type::Type*>(exp[0]));

        std::vector<ResourceType> converts =
            ConvertsFrom(std::get<const core::type::Type*>(exp[0]));
        // The converts from only contains values for the filterable types, for
        // the others it returns empty so we need to add the ResourceType for
        // that specific type.
        if (converts.empty()) {
            default_binding_type_order.push_back(
                TypeToResourceType(std::get<const core::type::Type*>(exp[0])));
        } else {
            for (ResourceType from : converts) {
                default_binding_type_order.push_back(from);
            }
        }
    }
    // If we didn't find any resource uses, then we can just return an empty config.
    if (default_binding_type_order.empty()) {
        return {};
    }

    std::unordered_map<BindingPoint, ResourceType> binding_to_resource_type;
    for (auto* inst : *(mod.root_block)) {
        auto* var = inst->As<core::ir::Var>();
        if (!var) {
            continue;
        }

        const type::Type* ty = var->Result()->Type()->UnwrapPtr();
        if (!ty->Is<type::Sampler>() && !ty->Is<type::Texture>()) {
            continue;
        }

        BindingPoint bp = var->BindingPoint().value();

        if (ty->Is<type::Sampler>() && !ty->As<type::Sampler>()->IsComparison() &&
            treat_samplers_as_filtering) {
            // Push both them to defaults because we need to fall back to the non-filtering version
            default_binding_type_order.push_back(ResourceType::kSampler_filtering);
            default_binding_type_order.push_back(ResourceType::kSampler_non_filtering);

            binding_to_resource_type.emplace(bp, ResourceType::kSampler_filtering);
            continue;
        }

        binding_to_resource_type.emplace(bp, DefaultResourceTypeFor(ty));

        std::vector<ResourceType> converts = ConvertsFrom(ty);
        // The converts from only contains values for the filterable types, for
        // the others it returns empty so we need to add the ResourceType for
        // that specific type.
        if (converts.empty()) {
            default_binding_type_order.push_back(TypeToResourceType(ty));
        } else {
            for (ResourceType from : converts) {
                default_binding_type_order.push_back(from);
            }
        }
    }

    // Sort so we get stable generated results
    std::sort(default_binding_type_order.begin(), default_binding_type_order.end());

    return ResourceTableConfig{
        .resource_table_binding = BindingPoint{.group = 43, .binding = 51},
        .storage_buffer_binding = BindingPoint{.group = 42, .binding = 52},
        .default_binding_type_order = std::move(default_binding_type_order),
        .binding_to_resource_type = std::move(binding_to_resource_type),
    };
}

}  // namespace tint::core::ir::transform
