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

#ifndef SRC_TINT_LANG_SPIRV_WRITER_COMMON_OPTIONS_H_
#define SRC_TINT_LANG_SPIRV_WRITER_COMMON_OPTIONS_H_

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/api/common/bindings.h"
#include "src/tint/api/common/resource_table_config.h"
#include "src/tint/api/common/substitute_overrides_config.h"
#include "src/tint/utils/reflection.h"

namespace tint::spirv::writer {

/// Supported SPIR-V binary versions.
/// If a new version is added here, also add it to:
/// * Writer::CanGenerate
/// * Printer::Code
/// Fully usable version will also need additions to:
/// * --spir-version on the command line
/// * Dawn in the Vulkan backend
enum class SpvVersion : uint32_t {
    kSpv13,  // SPIR-V 1.3
    kSpv14,  // SPIR-V 1.4
    kSpv15,  // SPIR-V 1.5, for testing purposes only
};

/// Configuration options used for generating SPIR-V.
struct Options {
    struct RangeOffsets {
        /// The offset of the min_depth immediate data
        uint32_t min = 0;
        /// The offset of the max_depth immediate data
        uint32_t max = 0;

        /// Reflect the fields of this class so that it can be used by tint::ForeachField()
        TINT_REFLECT(RangeOffsets, min, max);
        TINT_REFLECT_HASH_CODE(RangeOffsets);
    };

    /// The set of options which control workarounds for driver issues in the SPIR-V generator.
    struct Workarounds {
        ///////////////////////////////////////////////////////////////////////////////////////////
        // NOTE: When adding a new option here, it should also be added to the FuzzedOptions     //
        // structure in writer_fuzz.cc.                                                          //
        ///////////////////////////////////////////////////////////////////////////////////////////

        /// Set to `true` to generate a polyfill for switch statements using if/else statements.
        bool polyfill_case_switch = false;

        /// Set to `true` to scalarize max min and clamp builtins.
        bool scalarize_max_min_clamp = false;

        /// Set to `true` if handles should be transformed by direct variable access.
        bool dva_transform_handle = false;

        /// Set to `true` to generate polyfill for `pack4x8snorm`, `pack4x8unorm`, `unpack4x8snorm`
        /// and `unpack4x8unorm` builtins
        bool polyfill_pack_unpack_4x8_norm = false;

        /// Set to `true` to generate a polyfill clamp of `id` param of subgroupShuffle to within
        /// the spec max subgroup size.
        bool subgroup_shuffle_clamped = false;

        /// Set to 'true' to force workaround for 'textureSampleCompare(Level)' for texture arrays
        /// of cube depth.
        bool texture_sample_compare_depth_cube_array = false;

        /// Set to `true` to generate polyfill for `subgroupBroadcast(f16)`
        bool polyfill_subgroup_broadcast_f16 = false;

        /// Set to `true` to always pass matrices to user functions by pointer instead of by value.
        bool pass_matrix_by_pointer = false;

        /// Set to `true` to generate polyfill for f32 negation.
        bool polyfill_unary_f32_negation = false;

        /// Set to `true` to generate polyfill for f32 abs.
        bool polyfill_f32_abs = false;

        /// Set to `true` to generate polyfill for length(scalar f32).
        bool polyfill_length_scalar_f32 = false;

        /// Set to `true` to generate polyfill for distance(scalar f32).
        bool polyfill_distance_scalar_f32 = false;

        /// Set to `true` to generate polyfill for f16 saturate.
        bool polyfill_saturate_as_min_max_f16 = false;

        /// Set to `true` to treat the stride operand of cooperative matrix load and store
        /// instructions as matrix elements instead of a source/dest pointee elements.
        bool cooperative_matrix_stride_is_matrix_elements = false;

        TINT_REFLECT(Workarounds,
                     polyfill_case_switch,
                     scalarize_max_min_clamp,
                     dva_transform_handle,
                     polyfill_pack_unpack_4x8_norm,
                     subgroup_shuffle_clamped,
                     texture_sample_compare_depth_cube_array,
                     polyfill_subgroup_broadcast_f16,
                     pass_matrix_by_pointer,
                     polyfill_unary_f32_negation,
                     polyfill_f32_abs,
                     polyfill_length_scalar_f32,
                     polyfill_distance_scalar_f32,
                     polyfill_saturate_as_min_max_f16,
                     cooperative_matrix_stride_is_matrix_elements);
    };

    /// Any options which are controlled by the presence/absence of a vulkan extension.
    struct Extensions {
        ///////////////////////////////////////////////////////////////////////////////////////////
        // NOTE: When adding a new option here, it should also be added to the FuzzedOptions     //
        // structure in writer_fuzz.cc.                                                          //
        ///////////////////////////////////////////////////////////////////////////////////////////

        /// Set to `true` to allow for the usage of the demote to helper extension.
        bool use_demote_to_helper_invocation = false;

        /// Set to `true` to use the StorageInputOutput16 capability for shader IO that uses f16
        /// types.
        bool use_storage_input_output_16 = true;

        /// Set to `true` to initialize workgroup memory with OpConstantNull when
        /// VK_KHR_zero_initialize_workgroup_memory is enabled.
        bool use_zero_initialize_workgroup_memory = false;

        /// Set to `true` if the Vulkan Memory Model should be used
        bool use_vulkan_memory_model = false;

        /// Set to `true` to skip robustness transform on textures.
        bool disable_image_robustness = false;

        /// Set to `true` to disable index clamping on the runtime-sized arrays in robustness
        /// transform.
        bool disable_runtime_sized_array_index_clamping = false;

        /// Set to `true` to generate polyfill for `dot4I8Packed` and `dot4U8Packed` builtins
        bool dot_4x8_packed = false;

        /// Set to `true` to use the uniform buffer directly, `false` to decompose into array<vec4u,
        /// ...>.
        bool use_uniform_buffers = false;

        TINT_REFLECT(Extensions,
                     use_demote_to_helper_invocation,
                     use_storage_input_output_16,
                     use_zero_initialize_workgroup_memory,
                     use_vulkan_memory_model,
                     disable_image_robustness,
                     disable_runtime_sized_array_index_clamping,
                     dot_4x8_packed,
                     use_uniform_buffers);
    };

    ///////////////////////////////////////////////////////////////////////////////////////////
    // NOTE: When adding a new option here, it should also be added to the FuzzedOptions     //
    // structure in writer_fuzz.cc (if fuzzing is desired).                                  //
    ///////////////////////////////////////////////////////////////////////////////////////////

    /// The entry point name to generate
    std::string entry_point_name;

    /// An optional remapped name to use when emitting the entry point.
    std::string remapped_entry_point_name{};

    /// The bindings
    Bindings bindings{};

    // BindingPoints for textures that are paired with static samplers in the
    // BGL. These BindingPoints are the only ones that are allowed to map to
    // duplicate spir-v bindings, since they must map to the spir-v bindings of
    // the samplers with which they are paired.
    std::unordered_set<BindingPoint> statically_paired_texture_binding_points{};

    /// Mapping of colour indices to the binding point the var should be bound too
    std::unordered_map<uint32_t, BindingPoint> colour_index_to_binding_point{};

    /// Set to `true` to strip all user-declared identifiers from the module.
    bool strip_all_names = false;

    /// Set to `true` to disable software robustness that prevents out-of-bounds accesses.
    bool disable_robustness = false;

    /// Set to `true` to disable workgroup memory zero initialization
    bool disable_workgroup_init = false;

    /// Set to `true` to disable the polyfills on integer division and modulo.
    bool disable_polyfill_integer_div_mod = false;

    /// Set to `true` to enable integer range analysis in robustness transform.
    bool disable_integer_range_analysis = false;

    /// Set to `true` to generate a PointSize builtin and have it set to 1.0
    /// from all vertex shaders in the module.
    bool emit_vertex_point_size = true;

    /// Set to `true` to apply builtin 'position' pixel center emulation.
    bool polyfill_pixel_center = false;

    /// Set to `true` if framebuffer fetch should be multisampled
    bool multisampled_framebuffer_fetch = false;

    /// Any workarounds to enable/disable.
    Workarounds workarounds{};

    /// Any used extensions
    Extensions extensions{};

    /// Offsets of the minDepth and maxDepth push constants.
    std::optional<RangeOffsets> depth_range_offsets = std::nullopt;

    /// SPIR-V binary version.
    SpvVersion spirv_version = SpvVersion::kSpv13;

    /// Resource table information
    std::optional<ResourceTableConfig> resource_table = std::nullopt;

    // Configuration for substitute overrides
    SubstituteOverridesConfig substitute_overrides_config{};

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(Options,
                 entry_point_name,
                 remapped_entry_point_name,
                 bindings,
                 statically_paired_texture_binding_points,
                 colour_index_to_binding_point,
                 strip_all_names,
                 disable_robustness,
                 disable_workgroup_init,
                 disable_polyfill_integer_div_mod,
                 disable_integer_range_analysis,
                 emit_vertex_point_size,
                 polyfill_pixel_center,
                 multisampled_framebuffer_fetch,
                 workarounds,
                 extensions,
                 depth_range_offsets,
                 spirv_version,
                 resource_table,
                 substitute_overrides_config);
};

}  // namespace tint::spirv::writer

namespace tint {

/// Reflect enum information for SPIR-V version.
TINT_REFLECT_ENUM_RANGE(spirv::writer::SpvVersion, kSpv13, kSpv14);

}  // namespace tint

#endif  // SRC_TINT_LANG_SPIRV_WRITER_COMMON_OPTIONS_H_
