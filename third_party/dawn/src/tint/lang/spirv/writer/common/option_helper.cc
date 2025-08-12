// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/writer/common/option_helpers.h"

#include <utility>

#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/diagnostic/diagnostic.h"

namespace tint::spirv::writer {

Result<SuccessType> ValidateBindingOptions(const Options& options) {
    diag::List diagnostics;

    tint::Hashmap<tint::BindingPoint, tint::BindingPoint, 8> seen_wgsl_bindings{};
    tint::Hashmap<tint::BindingPoint, tint::BindingPoint, 8> seen_spirv_bindings{};

    // Both wgsl_seen and spirv_seen check to see if the pair of [src, dst] are unique. If we have
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

    const auto& statically_paired_texture_binding_points =
        options.statically_paired_texture_binding_points;
    auto spirv_seen = [&diagnostics, &seen_spirv_bindings,
                       &statically_paired_texture_binding_points](
                          const tint::BindingPoint& src, const tint::BindingPoint& dst) -> bool {
        if (auto binding = seen_spirv_bindings.Get(src)) {
            if (*binding != dst && !statically_paired_texture_binding_points.count(*binding) &&
                !statically_paired_texture_binding_points.count(dst)) {
                diagnostics.AddError(Source{})
                    << "found duplicate SPIR-V binding point: [group: " << src.group
                    << ", binding: " << src.binding << "]";
                return true;
            }
        }
        seen_spirv_bindings.Add(src, dst);
        return false;
    };

    auto valid = [&wgsl_seen, &spirv_seen](const auto& hsh) -> bool {
        for (const auto& it : hsh) {
            const auto& src_binding = it.first;
            const auto& dst_binding = it.second;

            if (wgsl_seen(src_binding, dst_binding)) {
                return false;
            }

            if (spirv_seen(dst_binding, src_binding)) {
                return false;
            }
        }
        return true;
    };

    if (!valid(options.bindings.uniform)) {
        diagnostics.AddNote(Source{}) << "when processing uniform";
        return Failure{diagnostics.Str()};
    }
    if (!valid(options.bindings.storage)) {
        diagnostics.AddNote(Source{}) << "when processing storage";
        return Failure{diagnostics.Str()};
    }
    if (!valid(options.bindings.texture)) {
        diagnostics.AddNote(Source{}) << "when processing texture";
        return Failure{diagnostics.Str()};
    }
    if (!valid(options.bindings.storage_texture)) {
        diagnostics.AddNote(Source{}) << "when processing storage_texture";
        return Failure{diagnostics.Str()};
    }
    if (!valid(options.bindings.sampler)) {
        diagnostics.AddNote(Source{}) << "when processing sampler";
        return Failure{diagnostics.Str()};
    }
    if (!valid(options.bindings.input_attachment)) {
        diagnostics.AddNote(Source{}) << "when processing input_attachment";
        return Failure{diagnostics.Str()};
    }

    for (const auto& it : options.bindings.external_texture) {
        const auto& src_binding = it.first;
        const auto& plane0 = it.second.plane0;
        const auto& plane1 = it.second.plane1;
        const auto& metadata = it.second.metadata;

        // Validate with the actual source regardless of what the remapper will do
        if (wgsl_seen(src_binding, plane0)) {
            diagnostics.AddNote(Source{}) << "when processing external_texture";
            return Failure{diagnostics.Str()};
        }

        if (spirv_seen(plane0, src_binding)) {
            diagnostics.AddNote(Source{}) << "when processing external_texture";
            return Failure{diagnostics.Str()};
        }
        if (spirv_seen(plane1, src_binding)) {
            diagnostics.AddNote(Source{}) << "when processing external_texture";
            return Failure{diagnostics.Str()};
        }
        if (spirv_seen(metadata, src_binding)) {
            diagnostics.AddNote(Source{}) << "when processing external_texture";
            return Failure{diagnostics.Str()};
        }
    }

    return Success;
}

// The remapped binding data and external texture data need to coordinate in order to put things in
// the correct place when we're done.
//
// When the data comes in we have a list of all WGSL origin (group,binding) pairs to SPIR-V
// (group,binding) pairs in the `uniform`, `storage`, `texture`, and `sampler` arrays.
//
// The `external_texture` array stores a WGSL origin (group,binding) pair for the external textures
// which provide `plane0`, `plane1`, and `metadata` SPIR-V (group,binding) pairs.
//
// If the remapper is run first, then the `external_texture` will end up being moved from the WGSL
// point, or the SPIR-V point (or the `plane0` value). There will also, possibly, have been bindings
// moved aside in order to place the `external_texture` bindings.
//
// If multiplanar runs first, care needs to be taken that when the texture is split and we create
// `plane1` and `metadata` that they do not collide with existing bindings. If they would collide
// then we need to place them elsewhere and have the remapper place them in the correct locations.
//
// # Example
// WGSL:
//   @group(0) @binding(0) var<uniform> u: Uniforms;
//   @group(0) @binding(1) var s: sampler;
//   @group(0) @binding(2) var t: texture_external;
//
// Given that program, Dawn may decide to do the remappings such that:
//   * WGSL u (0, 0) -> SPIR-V (0, 1)
//   * WGSL s (0, 1) -> SPIR-V (0, 2)
//   * WGSL t (0, 2):
//     * plane0 -> SPIR-V (0, 3)
//     * plane1 -> SPIR-V (0, 4)
//     * metadata -> SPIR-V (0, 0)
//
// In this case, if we run binding remapper first, then tell multiplanar to look for the texture at
// (0, 3) instead of the original (0, 2).
//
// If multiplanar runs first, then metadata (0, 0) needs to be placed elsewhere and then remapped
// back to (0, 0) by the remapper. (Otherwise, we'll have two `@group(0) @binding(0)` items in the
// program.)
//
// # Status
// The below method assumes we run binding remapper first. So it will setup the binding data and
// switch the value used by the multiplanar.
void PopulateRemapperAndMultiplanarOptions(
    const Options& options,
    RemapperData& remapper_data,
    tint::transform::multiplanar::BindingsMap& multiplanar_map) {
    auto create_remappings = [&remapper_data](const auto& hsh) {
        for (const auto& it : hsh) {
            const BindingPoint& src_binding_point = it.first;
            const auto& dst_binding_point = it.second;

            // Bindings which go to the same slot in SPIR-V do not need to be re-bound.
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
    create_remappings(options.bindings.sampler);
    create_remappings(options.bindings.input_attachment);

    // External textures are re-bound to their plane0 location
    for (const auto& it : options.bindings.external_texture) {
        const BindingPoint& src_binding_point = it.first;
        const auto& plane0 = it.second.plane0;
        const auto& plane1 = it.second.plane1;
        const auto& metadata = it.second.metadata;

        // Use the re-bound spir-v plane0 value for the lookup key.
        multiplanar_map.emplace(plane0,
                                tint::transform::multiplanar::BindingPoints{plane1, metadata});

        // Bindings which go to the same slot in SPIR-V do not need to be re-bound.
        if (src_binding_point == plane0) {
            continue;
        }

        remapper_data.emplace(src_binding_point, plane0);
    }
}

}  // namespace tint::spirv::writer
