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

#include "src/tint/lang/glsl/writer/helpers/generate_bindings.h"

#include <algorithm>
#include <unordered_set>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/glsl/writer/common/options.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::glsl::writer {

Bindings GenerateBindings(const core::ir::Module& module) {
    Bindings bindings{};

    std::unordered_set<tint::BindingPoint> seen_binding_points;

    // Set a binding point for the texture-builtins-from-uniform buffer.
    constexpr uint32_t kMaxBindGroups = 4u;
    bindings.texture_builtins_from_uniform.ubo_binding = {kMaxBindGroups, 0u};

    // Collect next valid binding number per group
    Hashmap<uint32_t, uint32_t, 4> group_to_next_binding_number;
    Vector<tint::BindingPoint, 4> ext_tex_bps;
    for (auto* inst : *module.root_block) {
        auto* var = inst->As<core::ir::Var>();
        if (!var) {
            continue;
        }

        if (auto bp = var->BindingPoint()) {
            if (auto val = group_to_next_binding_number.Get(bp->group)) {
                *val = std::max(*val, bp->binding + 1);
            } else {
                group_to_next_binding_number.Add(bp->group, bp->binding + 1);
            }

            auto* ptr_type = var->Result()->Type()->As<core::type::Pointer>();

            // Add all texture variables to the texture-builtin-from-uniform map.
            if (ptr_type->StoreType()->Is<core::type::Texture>()) {
                bindings.texture_builtins_from_uniform.ubo_bindingpoint_ordering.emplace_back(*bp);
            }

            // Store up the external textures, we'll add them in the next step
            if (ptr_type->StoreType()->Is<core::type::ExternalTexture>()) {
                ext_tex_bps.Push(*bp);
                continue;
            }

            binding::BindingInfo info{bp->binding};
            switch (ptr_type->AddressSpace()) {
                case core::AddressSpace::kHandle:
                    Switch(
                        ptr_type->StoreType(),  //
                        [&](const core::type::Sampler*) { bindings.sampler.emplace(*bp, info); },
                        [&](const core::type::StorageTexture*) {
                            bindings.storage_texture.emplace(*bp, info);
                        },
                        [&](const core::type::Texture*) { bindings.texture.emplace(*bp, info); });
                    break;
                case core::AddressSpace::kStorage:
                    bindings.storage.emplace(*bp, info);
                    break;
                case core::AddressSpace::kUniform:
                    bindings.uniform.emplace(*bp, info);
                    break;

                case core::AddressSpace::kUndefined:
                case core::AddressSpace::kPixelLocal:
                case core::AddressSpace::kPrivate:
                case core::AddressSpace::kPushConstant:
                case core::AddressSpace::kIn:
                case core::AddressSpace::kOut:
                case core::AddressSpace::kFunction:
                case core::AddressSpace::kWorkgroup:
                    break;
            }
        }
    }

    for (auto bp : ext_tex_bps) {
        uint32_t g = bp.group;
        uint32_t& next_num = group_to_next_binding_number.GetOrAddZero(g);

        binding::BindingInfo plane0{bp.binding};
        binding::BindingInfo plane1{next_num++};
        binding::BindingInfo metadata{next_num++};

        group_to_next_binding_number.Replace(g, next_num);

        bindings.external_texture.emplace(bp, binding::ExternalTexture{metadata, plane0, plane1});
    }

    return bindings;
}

}  // namespace tint::glsl::writer
