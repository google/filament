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
#include "src/tint/lang/core/type/binding_array.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/glsl/writer/common/options.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::glsl::writer {

Bindings GenerateBindings(const core::ir::Module& module) {
    Bindings bindings{};
    // Set a next_binding point for the texture-builtins-from-uniform buffer.
    bindings.texture_builtins_from_uniform.ubo_binding = {0u};
    uint32_t texture_builtin_offset = 0;

    // Track the next available GLSL binding number.
    // NOTE: GLSL does not have a concept of groups, so the next GLSL binding number
    // is a global value across all WGSL binding groups. The same WGSL binding point should map
    // consistently to the same GLSL binding number though.
    uint32_t next_binding = 0;
    Hashmap<tint::BindingPoint, uint32_t, 4> wgsl_to_glsl_bindings;
    auto get_binding = [&next_binding,
                        &wgsl_to_glsl_bindings](const tint::BindingPoint& bp) -> uint32_t {
        if (auto binding = wgsl_to_glsl_bindings.Get(bp)) {
            return *binding;
        }
        auto binding = next_binding++;
        wgsl_to_glsl_bindings.Add(bp, binding);
        return binding;
    };

    Vector<tint::BindingPoint, 4> ext_tex_bps;
    for (auto* inst : *module.root_block) {
        auto* var = inst->As<core::ir::Var>();
        if (!var) {
            continue;
        }

        if (auto bp = var->BindingPoint()) {
            auto* ptr_type = var->Result()->Type()->As<core::type::Pointer>();

            // Store up the external textures, we'll add them in the next step
            if (ptr_type->StoreType()->Is<core::type::ExternalTexture>()) {
                ext_tex_bps.Push(*bp);
                continue;
            }

            BindingInfo info{get_binding(bp.value())};
            switch (ptr_type->AddressSpace()) {
                case core::AddressSpace::kHandle: {
                    // Handle binding_array<handle> before logic dependent on the base handle type.
                    const core::type::Type* handle_type = ptr_type->StoreType();
                    uint32_t count = 1;
                    if (auto* ba = handle_type->As<core::type::BindingArray>()) {
                        handle_type = ba->ElemType();
                        count = ba->Count()->As<core::type::ConstantArrayCount>()->value;
                    }

                    Switch(
                        handle_type,
                        [&](const core::type::Sampler*) { bindings.sampler.emplace(*bp, info); },
                        [&](const core::type::StorageTexture*) {
                            bindings.storage_texture.emplace(*bp, info);
                        },
                        [&](const core::type::Texture*) {
                            bindings.texture.emplace(*bp, info);

                            // Add all texture variables to the texture-builtin-from-uniform map.
                            bindings.texture_builtins_from_uniform.ubo_contents.push_back(
                                {.offset = texture_builtin_offset,
                                 .count = count,
                                 .binding = info});
                            texture_builtin_offset += count;
                        });
                    break;
                }
                case core::AddressSpace::kStorage:
                    bindings.storage.emplace(*bp, info);
                    break;
                case core::AddressSpace::kUniform:
                    bindings.uniform.emplace(*bp, info);
                    break;

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
    }

    for (auto bp : ext_tex_bps) {
        BindingInfo plane0{get_binding(bp)};
        BindingInfo plane1{next_binding++};
        BindingInfo metadata{next_binding++};

        bindings.external_texture.emplace(bp, ExternalTexture{metadata, plane0, plane1});
    }

    return bindings;
}

}  // namespace tint::glsl::writer
