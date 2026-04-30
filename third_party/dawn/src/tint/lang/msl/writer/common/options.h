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

#ifndef SRC_TINT_LANG_MSL_WRITER_COMMON_OPTIONS_H_
#define SRC_TINT_LANG_MSL_WRITER_COMMON_OPTIONS_H_

#include <optional>
#include <string>
#include <unordered_map>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/api/common/bindings.h"
#include "src/tint/api/common/substitute_overrides_config.h"
#include "src/tint/api/common/vertex_pulling_config.h"
#include "src/tint/utils/reflection.h"

namespace tint::msl::writer {

/// Options used to specify a mapping of binding points to indices into a UBO
/// from which to load buffer sizes.
/// TODO(crbug.com/366291600): Remove ubo_binding after switch to immediates.
struct ArrayLengthOptions {
    /// The MSL binding point to use to generate a uniform buffer from which to read buffer sizes.
    std::optional<uint32_t> ubo_binding{};

    /// The offset in immediate block for buffer sizes.
    std::optional<uint32_t> buffer_sizes_offset{};

    /// The mapping from the storage buffer binding points in WGSL binding-point space to the index
    /// into the uniform buffer where the length of the buffer is stored.
    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_size_index{};

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(ArrayLengthOptions, ubo_binding, buffer_sizes_offset, bindpoint_to_size_index);
    TINT_REFLECT_HASH_CODE(ArrayLengthOptions);

    bool operator==(const ArrayLengthOptions&) const = default;
};

/// Information to configure an argument buffer
struct ArgumentBufferInfo {
    /// The buffer ID to use for this argument buffer
    uint32_t id;

    /// The buffer ID to use for the dynamic buffer if needed
    std::optional<uint32_t> dynamic_buffer_id{};

    /// Dynamic offsets map. The map is BindingIndex -> dynamic offsets array index
    std::unordered_map<uint32_t, uint32_t> binding_info_to_offset_index{};

    TINT_REFLECT(ArgumentBufferInfo, id, dynamic_buffer_id, binding_info_to_offset_index);
    TINT_REFLECT_HASH_CODE(ArgumentBufferInfo);

    bool operator==(const ArgumentBufferInfo&) const = default;
};

/// Configuration options used for generating MSL.
struct Options {
    struct RangeOffsets {
        /// The offset of the min_depth immediate data
        uint32_t min = 0;
        /// The offset of the max_depth immediate data
        uint32_t max = 0;

        /// Reflect the fields of this class so that it can be used by tint::ForeachField()
        TINT_REFLECT(RangeOffsets, min, max);
        TINT_REFLECT_HASH_CODE(RangeOffsets);

        bool operator==(const RangeOffsets&) const = default;
    };

    /// The set of options which control workarounds for driver issues.
    struct Workarounds {
        ///////////////////////////////////////////////////////////////////////////////////////////
        // NOTE: When adding a new option here, it should also be added to the FuzzedOptions     //
        // structure in writer_fuzz.cc.                                                          //
        ///////////////////////////////////////////////////////////////////////////////////////////

        /// Set to `true` to scalarize max min and clamp builtins.
        bool scalarize_max_min_clamp = false;

        /// Set to `true` to disable the module constant transform for f16
        bool disable_module_constant_f16 = false;

        /// Set to `true` to generate polyfill for `subgroupBroadcast(f16)`
        bool polyfill_subgroup_broadcast_f16 = false;

        /// Set to `true` to generate polyfill for `clamp(f16/f32)`
        bool polyfill_clamp_float = false;

        /// Set to `true` to polyfill `unpack2x16snorm()`.
        bool polyfill_unpack_2x16_snorm = false;

        /// Set to `true` to polyfill `unpack2x16unorm()`.
        bool polyfill_unpack_2x16_unorm = false;

        /// Set to `true` to polyfill tanh with an f16 value
        bool polyfill_tanh_f16 = false;

        /// Set to `true` to replace bool types in workgroup storage with u32.
        bool replace_workgroup_bool_with_u32 = false;

        TINT_REFLECT(Workarounds,
                     scalarize_max_min_clamp,
                     disable_module_constant_f16,
                     polyfill_subgroup_broadcast_f16,
                     polyfill_clamp_float,
                     polyfill_unpack_2x16_snorm,
                     polyfill_unpack_2x16_unorm,
                     polyfill_tanh_f16,
                     replace_workgroup_bool_with_u32);
        TINT_REFLECT_HASH_CODE(Workarounds);

        bool operator==(const Workarounds&) const = default;
    };

    /// Any options which are controlled by the current Metal version.
    struct Extensions {
        ///////////////////////////////////////////////////////////////////////////////////////////
        // NOTE: When adding a new option here, it should also be added to the FuzzedOptions     //
        // structure in writer_fuzz.cc.                                                          //
        ///////////////////////////////////////////////////////////////////////////////////////////

        /// Set to `true` to disable demote to helper transform
        bool disable_demote_to_helper = false;

        TINT_REFLECT(Extensions, disable_demote_to_helper);
        TINT_REFLECT_HASH_CODE(Extensions);

        bool operator==(const Extensions&) const = default;
    };

    /// Constructor
    Options();
    /// Destructor
    ~Options();
    /// Copy constructor
    Options(const Options&);
    /// Copy assignment
    /// @returns this Options
    Options& operator=(const Options&);

    ///////////////////////////////////////////////////////////////////////////////////////////
    // NOTE: When adding a new option here, it should also be added to the FuzzedOptions     //
    // structure in writer_fuzz.cc (if fuzzing is desired).                                  //
    ///////////////////////////////////////////////////////////////////////////////////////////

    /// The entry point name to emit.
    std::string entry_point_name = {};

    /// An optional remapped name to use when emitting the entry point.
    std::string remapped_entry_point_name = {};

    /// Set to `true` to strip all user-declared identifiers from the module.
    bool strip_all_names = false;

    /// Set to `true` to disable software robustness that prevents out-of-bounds accesses.
    bool disable_robustness = false;

    /// Set to `true` to enable integer range analysis in robustness transform.
    bool disable_integer_range_analysis = false;

    /// Set to `true` to disable workgroup memory zero initialization
    bool disable_workgroup_init = false;

    /// Set to `true` to generate a [[point_size]] attribute which is set to 1.0
    /// for all vertex shaders in the module.
    bool emit_vertex_point_size = false;

    /// Set to `true` to disable the polyfills on integer division and modulo.
    bool disable_polyfill_integer_div_mod = false;

    /// Emit argument buffers
    bool use_argument_buffers = false;

    /// Any workarounds to enable/disable.
    Workarounds workarounds{};

    /// Any used extensions
    Extensions extensions{};

    /// The fixed sample mask to combine with fragment shader outputs.
    /// Defaults to 0xFFFFFFFF.
    uint32_t fixed_sample_mask = 0xFFFFFFFF;

    /// Index of pixel_local structure member index to attachment index
    std::unordered_map<uint32_t, uint32_t> pixel_local_attachments;

    /// Options used to specify a mapping of binding points to indices into a UBO
    /// or immediate block from which to load buffer sizes.
    ArrayLengthOptions array_length_from_constants = {};

    /// The optional vertex pulling configuration.
    std::optional<VertexPullingConfig> vertex_pulling_config = {};

    /// Immediate binding point info
    std::optional<BindingPoint> immediate_binding_point = {};

    /// Map of group id to argument buffer information
    std::unordered_map<uint32_t, ArgumentBufferInfo> group_to_argument_buffer_info;

    /// Offsets of the minDepth and maxDepth push constants.
    std::optional<RangeOffsets> depth_range_offsets = std::nullopt;

    /// The bindings.
    Bindings bindings;

    // Substitute Overrides
    SubstituteOverridesConfig substitute_overrides_config = {};

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(Options,
                 entry_point_name,
                 remapped_entry_point_name,
                 strip_all_names,
                 disable_robustness,
                 disable_integer_range_analysis,
                 disable_workgroup_init,
                 emit_vertex_point_size,
                 disable_polyfill_integer_div_mod,
                 use_argument_buffers,
                 workarounds,
                 extensions,
                 fixed_sample_mask,
                 pixel_local_attachments,
                 array_length_from_constants,
                 vertex_pulling_config,
                 immediate_binding_point,
                 group_to_argument_buffer_info,
                 depth_range_offsets,
                 bindings,
                 substitute_overrides_config);
    TINT_REFLECT_HASH_CODE(Options);

    bool operator==(const Options&) const = default;
};

}  // namespace tint::msl::writer

#endif  // SRC_TINT_LANG_MSL_WRITER_COMMON_OPTIONS_H_
