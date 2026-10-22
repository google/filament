/// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/api/helpers/generate_bindings.h"

#include <algorithm>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/referenced_module_vars.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texel_buffer.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint {

Bindings GenerateBindings(const core::ir::Module& module,
                          const std::string& ep,
                          bool set_group_to_zero,
                          bool flatten_bindings,
                          std::unordered_set<tint::BindingPoint> ycbcr_bindings) {
    Bindings bindings{};

    uint32_t next_buffer_idx = 0;
    uint32_t next_sampler_idx = 0;
    uint32_t next_texture_idx = 0;

    // Collect next valid binding number per group
    Hashmap<uint32_t, uint32_t, 4> group_to_next_binding_number;
    Vector<tint::BindingPoint, 4> ext_tex_bps;

    core::ir::Function* ep_func = nullptr;
    for (auto* f : module.functions) {
        if (!f->IsEntryPoint()) {
            continue;
        }
        if (module.NameOf(f).NameView() == ep) {
            ep_func = f;
            break;
        }
    }
    // No entrypoint, so no bindings needed
    if (!ep_func) {
        return bindings;
    }

    core::ir::ReferencedModuleVars<const core::ir::Module> referenced_module_vars{module};
    auto& refs = referenced_module_vars.TransitiveReferences(ep_func);

    for (auto* var : refs) {
        auto bp = var->BindingPoint();
        if (!bp.has_value()) {
            continue;
        }

        if (auto val = group_to_next_binding_number.Get(bp->group)) {
            *val = std::max(*val, bp->binding + 1);
        } else {
            group_to_next_binding_number.Add(bp->group, bp->binding + 1);
        }

        auto* ptr = var->Result()->Type()->As<core::type::Pointer>();

        // Store up the external textures, we'll add them in the next step
        if (ptr->StoreType()->Is<core::type::ExternalTexture>()) {
            ext_tex_bps.Push(*bp);
            continue;
        }

        switch (ptr->AddressSpace()) {
            case core::AddressSpace::kHandle:
                Switch(
                    ptr->StoreType(),  //
                    [&](const core::type::Sampler*) {
                        tint::BindingPoint info{
                            .group = set_group_to_zero ? 0 : bp->group,
                            .binding = flatten_bindings ? next_sampler_idx++ : bp->binding,
                        };
                        bindings.sampler.emplace(*bp, info);
                    },
                    [&](const core::type::StorageTexture*) {
                        tint::BindingPoint info{
                            .group = set_group_to_zero ? 0 : bp->group,
                            .binding = flatten_bindings ? next_texture_idx++ : bp->binding,
                        };
                        bindings.storage_texture.emplace(*bp, info);
                    },
                    [&](const core::type::TexelBuffer*) {
                        tint::BindingPoint info{
                            .group = set_group_to_zero ? 0 : bp->group,
                            .binding = flatten_bindings ? next_texture_idx++ : bp->binding,
                        };
                        bindings.texel_buffer.emplace(*bp, info);
                    },
                    [&](const core::type::Texture*) {
                        tint::BindingPoint info{
                            .group = set_group_to_zero ? 0 : bp->group,
                            .binding = flatten_bindings ? next_texture_idx++ : bp->binding,
                        };
                        bindings.texture.emplace(*bp, info);
                    });
                break;
            case core::AddressSpace::kStorage: {
                tint::BindingPoint info{
                    .group = set_group_to_zero ? 0 : bp->group,
                    .binding = flatten_bindings ? next_buffer_idx++ : bp->binding,
                };
                bindings.storage.emplace(*bp, info);
                break;
            }
            case core::AddressSpace::kUniform: {
                tint::BindingPoint info{
                    .group = set_group_to_zero ? 0 : bp->group,
                    .binding = flatten_bindings ? next_buffer_idx++ : bp->binding,
                };
                bindings.uniform.emplace(*bp, info);
                break;
            }
            case core::AddressSpace::kUndefined:
            case core::AddressSpace::kPixelLocal:
            case core::AddressSpace::kPrivate:
            case core::AddressSpace::kImmediate:
            case core::AddressSpace::kIn:
            case core::AddressSpace::kOut:
            case core::AddressSpace::kFunction:
            case core::AddressSpace::kWorkgroup:
                break;
        }
    }

    if (flatten_bindings) {
        for (auto bp : ext_tex_bps) {
            uint32_t g = set_group_to_zero ? 0 : bp.group;

            tint::BindingPoint plane0{.group = g, .binding = next_texture_idx++};
            tint::BindingPoint metadata{.group = g, .binding = next_buffer_idx++};

            if (ycbcr_bindings.contains(bp)) {
                tint::BindingPoint sampler{.group = g, .binding = next_sampler_idx++};
                bindings.external_texture.emplace(bp,
                                                  ExternalYCBCRTexture{metadata, plane0, sampler});
            } else {
                tint::BindingPoint plane1{.group = g, .binding = next_texture_idx++};
                bindings.external_texture.emplace(
                    bp, ExternalMultiplanarTexture{metadata, plane0, plane1});
            }
        }
    } else {
        for (auto bp : ext_tex_bps) {
            uint32_t& next_num = group_to_next_binding_number.GetOrAddZero(bp.group);
            uint32_t g = set_group_to_zero ? 0 : bp.group;

            tint::BindingPoint plane0{.group = g, .binding = bp.binding};
            tint::BindingPoint plane1{.group = g, .binding = next_num++};
            tint::BindingPoint metadata{.group = g, .binding = next_num++};

            if (ycbcr_bindings.contains(bp)) {
                bindings.external_texture.emplace(bp,
                                                  ExternalYCBCRTexture{metadata, plane0, plane1});
            } else {
                bindings.external_texture.emplace(
                    bp, ExternalMultiplanarTexture{metadata, plane0, plane1});
            }
        }
    }

    return bindings;
}

}  // namespace tint
