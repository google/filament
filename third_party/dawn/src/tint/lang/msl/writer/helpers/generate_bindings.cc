/// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/msl/writer/helpers/generate_bindings.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/msl/writer/common/options.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::msl::writer {

Bindings GenerateBindings(const core::ir::Module& module) {
    Bindings bindings{};

    std::unordered_set<tint::BindingPoint> seen_binding_points;

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

            auto* ptr = var->Result(0)->Type()->As<core::type::Pointer>();

            // Store up the external textures, we'll add them in the next step
            if (ptr->StoreType()->Is<core::type::ExternalTexture>()) {
                ext_tex_bps.Push(*bp);
                continue;
            }

            binding::BindingInfo info{bp->binding};
            switch (ptr->AddressSpace()) {
                case core::AddressSpace::kHandle:
                    Switch(
                        ptr->StoreType(),  //
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

        bindings.external_texture.emplace(bp, binding::ExternalTexture{metadata, plane0, plane1});
    }

    return bindings;
}

}  // namespace tint::msl::writer
