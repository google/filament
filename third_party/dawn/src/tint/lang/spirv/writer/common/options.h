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
#include "src/tint/utils/reflection.h"

namespace tint::spirv::writer {

/// An external texture
struct ExternalTexture {
    /// Metadata
    BindingPoint metadata{};
    /// Plane0 binding data
    BindingPoint plane0{};
    /// Plane1 binding data
    BindingPoint plane1{};

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(ExternalTexture, metadata, plane0, plane1);
};

using BindingMap = std::unordered_map<BindingPoint, BindingPoint>;
using ExternalTextureBindings = std::unordered_map<BindingPoint, ExternalTexture>;

/// Binding information
struct Bindings {
    /// Uniform bindings
    BindingMap uniform{};
    /// Storage bindings
    BindingMap storage{};
    /// Texture bindings
    BindingMap texture{};
    /// Storage texture bindings
    BindingMap storage_texture{};
    /// Sampler bindings
    BindingMap sampler{};
    /// External bindings
    ExternalTextureBindings external_texture{};
    /// Input attachment bindings
    BindingMap input_attachment{};

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(Bindings,
                 uniform,
                 storage,
                 texture,
                 storage_texture,
                 sampler,
                 external_texture,
                 input_attachment);
};

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
    };

    /// An optional remapped name to use when emitting the entry point.
    std::string remapped_entry_point_name = {};

    /// The bindings
    Bindings bindings = {};

    // BindingPoints for textures that are paired with static samplers in the
    // BGL. These BindingPoints are the only ones that are allowed to map to
    // duplicate spir-v bindings, since they must map to the spir-v bindings of
    // the samplers with which they are paired.
    std::unordered_set<BindingPoint> statically_paired_texture_binding_points = {};

    /// Set to `true` to strip all user-declared identifiers from the module.
    bool strip_all_names = false;

    /// Set to `true` to disable software robustness that prevents out-of-bounds accesses.
    bool disable_robustness = false;

    /// Set to `true` to enable integer range analysis in robustness transform.
    bool enable_integer_range_analysis = false;

    /// Set to `true` to skip robustness transform on textures.
    bool disable_image_robustness = false;

    /// Set to `true` to disable index clamping on the runtime-sized arrays in robustness transform.
    bool disable_runtime_sized_array_index_clamping = false;

    /// Set to `true` to disable workgroup memory zero initialization
    bool disable_workgroup_init = false;

    /// Set to `true` to allow for the usage of the demote to helper extension.
    bool use_demote_to_helper_invocation_extensions = false;

    /// Set to `true` to initialize workgroup memory with OpConstantNull when
    /// VK_KHR_zero_initialize_workgroup_memory is enabled.
    bool use_zero_initialize_workgroup_memory_extension = false;

    /// Set to `true` to use the StorageInputOutput16 capability for shader IO that uses f16 types.
    bool use_storage_input_output_16 = true;

    /// Set to `true` to generate a PointSize builtin and have it set to 1.0
    /// from all vertex shaders in the module.
    bool emit_vertex_point_size = true;

    /// Set to `true` to always pass matrices to user functions by pointer instead of by value.
    bool pass_matrix_by_pointer = false;

    /// Set to `true` to generate polyfill for `dot4I8Packed` and `dot4U8Packed` builtins
    bool polyfill_dot_4x8_packed = false;

    /// Set to `true` to generate polyfill for `pack4x8snorm`, `pack4x8unorm`, `unpack4x8snorm` and
    /// `unpack4x8unorm` builtins
    bool polyfill_pack_unpack_4x8_norm = false;

    /// Set to `true` to generate a polyfill clamp of `id` param of subgroupShuffle to within the
    /// spec max subgroup size.
    bool subgroup_shuffle_clamped = false;

    /// Set to `true` to disable the polyfills on integer division and modulo.
    bool disable_polyfill_integer_div_mod = false;

    /// Set to `true` to scalarize max min and clamp builtins.
    bool scalarize_max_min_clamp = false;

    /// Set to `true` if the Vulkan Memory Model should be used
    bool use_vulkan_memory_model = false;

    /// Set to `true` if handles should be transformed by direct variable access.
    bool dva_transform_handle = false;

    /// Offsets of the minDepth and maxDepth push constants.
    std::optional<RangeOffsets> depth_range_offsets = std::nullopt;

    /// SPIR-V binary version.
    SpvVersion spirv_version = SpvVersion::kSpv13;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(Options,
                 remapped_entry_point_name,
                 bindings,
                 statically_paired_texture_binding_points,
                 strip_all_names,
                 disable_robustness,
                 enable_integer_range_analysis,
                 disable_image_robustness,
                 disable_runtime_sized_array_index_clamping,
                 disable_workgroup_init,
                 use_demote_to_helper_invocation_extensions,
                 use_zero_initialize_workgroup_memory_extension,
                 use_storage_input_output_16,
                 emit_vertex_point_size,
                 pass_matrix_by_pointer,
                 polyfill_dot_4x8_packed,
                 polyfill_pack_unpack_4x8_norm,
                 subgroup_shuffle_clamped,
                 disable_polyfill_integer_div_mod,
                 scalarize_max_min_clamp,
                 use_vulkan_memory_model,
                 dva_transform_handle,
                 depth_range_offsets,
                 spirv_version);
};

}  // namespace tint::spirv::writer

namespace tint {

/// Reflect enum information for SPIR-V version.
TINT_REFLECT_ENUM_RANGE(spirv::writer::SpvVersion, kSpv13, kSpv14);

}  // namespace tint

#endif  // SRC_TINT_LANG_SPIRV_WRITER_COMMON_OPTIONS_H_
