/// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/hlsl/writer/common/option_helpers.h"

#include <utility>
#include <variant>

#include "src/tint/api/common/bindings.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/binding_array.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texel_buffer.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/diagnostic/diagnostic.h"

namespace tint::hlsl::writer {

using PointToPointMap = tint::Hashmap<tint::BindingPoint, tint::BindingPoint, 8>;

Result<SuccessType> ValidateBindingOptions(const core::ir::Module& ir, const Options& options) {
    diag::List diagnostics;

    tint::Hashmap<tint::BindingPoint, const core::ir::Var*, 8> binding_to_var{};
    for (auto* inst : *ir.root_block) {
        if (auto* var = inst->As<core::ir::Var>()) {
            if (auto bp = var->BindingPoint()) {
                binding_to_var.Add(*bp, var);
            }
        }
    }

    PointToPointMap seen_wgsl_bindings{};
    PointToPointMap seen_hlsl_register_b{};
    PointToPointMap seen_hlsl_register_s{};
    PointToPointMap seen_hlsl_register_t{};
    PointToPointMap seen_hlsl_register_u{};

    // Both wgsl_seen and hlsl_seen check to see if the pair of [src, dst] are unique. If we have
    // multiple entries that map the same [src, dst] pair, that's fine. We treat it as valid as it's
    // possible for multiple entry points to use the remapper at the same time. If the pair doesn't
    // match, then we report an error about a duplicate binding point.

    auto wgsl_seen = [&diagnostics, &seen_wgsl_bindings](const tint::BindingPoint& src,
                                                         const tint::BindingPoint& dst) -> bool {
        if (auto binding = seen_wgsl_bindings.Get(src)) {
            if (*binding != dst) {
                diagnostics.AddError(Source{}) << "found duplicate WGSL binding point: " << src;
                return true;
            }
        }
        seen_wgsl_bindings.Add(src, dst);
        return false;
    };

    auto hlsl_seen = [&diagnostics](PointToPointMap& map, const tint::BindingPoint& src,
                                    const tint::BindingPoint& dst) -> bool {
        if (auto binding = map.Get(src)) {
            if (*binding != dst) {
                diagnostics.AddError(Source{})
                    << "found duplicate HLSL binding point: [binding: " << src.binding << "]";
                return true;
            }
        }
        map.Add(src, dst);
        return false;
    };

    auto valid = [&wgsl_seen, &hlsl_seen](auto&& map, const auto& hsh) -> bool {
        for (const auto& it : hsh) {
            const auto& src_binding = it.first;
            const auto& dst_binding = it.second;

            if (wgsl_seen(src_binding, dst_binding)) {
                return false;
            }

            // The map is either passed directly or as a function that returns the map.
            PointToPointMap* dst_map = nullptr;
            if constexpr (std::is_same_v<decltype(map), PointToPointMap&>) {
                dst_map = &map;
            } else {
                dst_map = &map(src_binding);
            }

            if (hlsl_seen(*dst_map, dst_binding, src_binding)) {
                return false;
            }
        }
        return true;
    };

    // uniform buffers use register b#.
    if (!valid(seen_hlsl_register_b, options.bindings.uniform)) {
        diagnostics.AddNote(Source{}) << "when processing uniform";
        return Failure{diagnostics.Str()};
    }

    // read-only storage buffers use register t#.
    // read-write storage buffers use register u#.
    auto storage_map = [&](const tint::BindingPoint& src) -> PointToPointMap& {
        bool is_read_only = false;
        if (auto* var = binding_to_var.GetOr(src, nullptr)) {
            if (auto* ptr = var->Result()->Type()->As<core::type::Pointer>()) {
                is_read_only = ptr->Access() == core::Access::kRead;
            }
        }
        return is_read_only ? seen_hlsl_register_t : seen_hlsl_register_u;
    };
    if (!valid(storage_map, options.bindings.storage)) {
        diagnostics.AddNote(Source{}) << "when processing storage";
        return Failure{diagnostics.Str()};
    }

    // samplers use register s#.
    if (!valid(seen_hlsl_register_s, options.bindings.sampler)) {
        diagnostics.AddNote(Source{}) << "when processing sampler";
        return Failure{diagnostics.Str()};
    }

    // read-only textures use register t#.
    if (!valid(seen_hlsl_register_t, options.bindings.texture)) {
        diagnostics.AddNote(Source{}) << "when processing texture";
        return Failure{diagnostics.Str()};
    }

    // read-only storage textures use register t#.
    // writable storage textures use register u#.
    auto storage_texture_map = [&](const tint::BindingPoint& src) -> PointToPointMap& {
        bool is_read_only = false;
        if (auto* var = binding_to_var.GetOr(src, nullptr)) {
            auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
            auto* store_ty = ptr->StoreType();
            if (auto* arr = store_ty->As<core::type::BindingArray>()) {
                store_ty = arr->ElemType();
            }
            if (auto* st = store_ty->As<core::type::StorageTexture>()) {
                is_read_only = st->Access() == core::Access::kRead;
            }
        }
        return is_read_only ? seen_hlsl_register_t : seen_hlsl_register_u;
    };
    if (!valid(storage_texture_map, options.bindings.storage_texture)) {
        diagnostics.AddNote(Source{}) << "when processing storage_texture";
        return Failure{diagnostics.Str()};
    }

    // texel buffers use register t# or u#.
    auto texel_buffer_map = [&](const tint::BindingPoint& src) -> PointToPointMap& {
        bool is_read_only = false;
        if (auto* var = binding_to_var.GetOr(src, nullptr)) {
            auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
            auto* store_ty = ptr->StoreType();
            if (auto* arr = store_ty->As<core::type::BindingArray>()) {
                store_ty = arr->ElemType();
            }
            if (auto* tb = store_ty->As<core::type::TexelBuffer>()) {
                is_read_only = tb->Access() == core::Access::kRead;
            }
        }
        return is_read_only ? seen_hlsl_register_t : seen_hlsl_register_u;
    };
    if (!valid(texel_buffer_map, options.bindings.texel_buffer)) {
        diagnostics.AddNote(Source{}) << "when processing texel_buffer";
        return Failure{diagnostics.Str()};
    }

    for (const auto& it : options.bindings.external_texture) {
        const auto& src_binding = it.first;

        auto& data = it.second;
        BindingPoint src;
        BindingPoint metadata;
        if (std::holds_alternative<ExternalMultiplanarTexture>(data)) {
            ExternalMultiplanarTexture et = std::get<ExternalMultiplanarTexture>(data);
            src = et.plane0;
            metadata = et.metadata;

            // Plane0 & Plane1 both use register t#.
            if (hlsl_seen(seen_hlsl_register_t, et.plane0, src_binding)) {
                diagnostics.AddNote(Source{}) << "when processing external_texture";
                return Failure{diagnostics.Str()};
            }
            if (hlsl_seen(seen_hlsl_register_t, et.plane1, src_binding)) {
                diagnostics.AddNote(Source{}) << "when processing external_texture";
                return Failure{diagnostics.Str()};
            }
        } else if (std::holds_alternative<ExternalYCBCRTexture>(data)) {
            ExternalYCBCRTexture ycb = std::get<ExternalYCBCRTexture>(data);
            src = ycb.texture;
            metadata = ycb.metadata;

            // The texture uses register t#.
            if (hlsl_seen(seen_hlsl_register_t, ycb.texture, src_binding)) {
                diagnostics.AddNote(Source{}) << "when processing external_texture";
                return Failure{diagnostics.Str()};
            }
            // The sampler uses register s#.
            if (hlsl_seen(seen_hlsl_register_s, ycb.sampler, src_binding)) {
                diagnostics.AddNote(Source{}) << "when processing external_texture";
                return Failure{diagnostics.Str()};
            }
        } else {
            TINT_UNREACHABLE();
        }

        // Validate with the actual source regardless of what the remapper will do
        if (wgsl_seen(src_binding, src)) {
            diagnostics.AddNote(Source{}) << "when processing external_texture";
            return Failure{diagnostics.Str()};
        }

        // Metadata is a uniform buffer, which uses register b#.
        if (hlsl_seen(seen_hlsl_register_b, metadata, src_binding)) {
            diagnostics.AddNote(Source{}) << "when processing external_texture";
            return Failure{diagnostics.Str()};
        }
    }

    return Success;
}

// The remapped binding data and external texture data need to coordinate in order to put things in
// the correct place when we're done. The binding remapper is run first, so make sure that the
// external texture uses the new binding point.
//
// When the data comes in we have a list of all WGSL origin (group,binding) pairs to HLSL
// (group,binding) in the `uniform`, `storage`, `texture`, and `sampler` arrays.
void PopulateBindingRelatedOptions(
    const Options& options,
    RemapperData& remapper_data,
    tint::transform::multiplanar::BindingsMap& multiplanar_map,
    ArrayLengthFromUniformOptions& array_length_from_uniform_options,
    ArrayOffsetFromUniformOptions& array_offset_from_uniform_options) {
    auto create_remappings = [&remapper_data](const auto& hsh) {
        for (const auto& it : hsh) {
            const BindingPoint& src_binding_point = it.first;
            const auto& dst_binding_point = it.second;

            // Skip redundant bindings
            if (src_binding_point == dst_binding_point) {
                continue;
            }

            remapper_data.emplace(src_binding_point, dst_binding_point);
        }
    };

    create_remappings(options.bindings.uniform);
    create_remappings(options.bindings.storage);
    create_remappings(options.bindings.texture);
    create_remappings(options.bindings.storage_texture);
    create_remappings(options.bindings.texel_buffer);
    create_remappings(options.bindings.sampler);

    // External textures are re-bound to their plane0 location
    for (const auto& it : options.bindings.external_texture) {
        const BindingPoint& src_binding_point = it.first;
        auto& data = it.second;

        BindingPoint dest_bp;
        if (std::holds_alternative<ExternalMultiplanarTexture>(data)) {
            ExternalMultiplanarTexture et = std::get<ExternalMultiplanarTexture>(data);
            const auto& plane0 = et.plane0;
            const auto& plane1 = et.plane1;
            const auto& metadata = et.metadata;

            // Use the re-bound HLSL plane0 value for the lookup key.
            multiplanar_map.emplace(
                plane0, tint::transform::multiplanar::MultiplanarTexture{plane1, metadata});

            dest_bp = plane0;
        } else if (std::holds_alternative<ExternalYCBCRTexture>(data)) {
            ExternalYCBCRTexture ycb = std::get<ExternalYCBCRTexture>(data);
            const auto& texture = ycb.texture;
            const auto& sampler = ycb.sampler;
            const auto& metadata = ycb.metadata;

            // Use the re-bound HLSL texture value for the lookup key.
            multiplanar_map.emplace(texture,
                                    tint::transform::multiplanar::YCBCRTexture{sampler, metadata});

            dest_bp = texture;
        } else {
            TINT_UNREACHABLE();
        }

        // Bindings which go to the same slot in HLSL do not need to be re-bound.
        if (src_binding_point == dest_bp) {
            continue;
        }

        remapper_data.emplace(src_binding_point, dest_bp);
    }

    // ArrayLengthFromUniformOptions and ArrayOffsetFromUniformOptions bindpoints may need to be
    // remapped
    auto remap = [&remapper_data](const std::unordered_map<BindingPoint, uint32_t>& bp_to_index) {
        std::unordered_map<BindingPoint, uint32_t> remapped;
        for (auto& [bindpoint, index] : bp_to_index) {
            auto it = remapper_data.find(bindpoint);
            if (it != remapper_data.end()) {
                remapped.emplace(it->second, index);
            } else {
                remapped.emplace(bindpoint, index);
            }
        }
        return remapped;
    };

    array_length_from_uniform_options.ubo_binding = options.array_length_from_uniform.ubo_binding;
    array_length_from_uniform_options.buffer_sizes_offset =
        options.array_length_from_uniform.buffer_sizes_offset;
    array_length_from_uniform_options.bindpoint_to_size_index =
        remap(options.array_length_from_uniform.bindpoint_to_size_index);

    array_offset_from_uniform_options.ubo_binding = options.array_offset_from_uniform.ubo_binding;
    array_offset_from_uniform_options.buffer_offsets_offset =
        options.array_offset_from_uniform.buffer_offsets_offset;
    array_offset_from_uniform_options.bindpoint_to_offset_index =
        remap(options.array_offset_from_uniform.bindpoint_to_offset_index);
}

}  // namespace tint::hlsl::writer
