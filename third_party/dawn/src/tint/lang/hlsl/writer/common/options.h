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

#ifndef SRC_TINT_LANG_HLSL_WRITER_COMMON_OPTIONS_H_
#define SRC_TINT_LANG_HLSL_WRITER_COMMON_OPTIONS_H_

#include <bitset>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/api/common/bindings.h"
#include "src/tint/api/common/resource_table_config.h"
#include "src/tint/api/common/substitute_overrides_config.h"
#include "src/tint/lang/core/enums.h"
#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/reflection.h"

namespace tint::hlsl::writer {

/// kMaxInterStageLocations == D3D11_PS_INPUT_REGISTER_COUNT - 2
/// D3D11_PS_INPUT_REGISTER_COUNT == D3D12_PS_INPUT_REGISTER_COUNT
constexpr uint32_t kMaxInterStageLocations = 30;

/// Options used to specify a mapping of binding points to indices into a UBO
/// from which to load buffer sizes, or to load them from immediate blocks.
/// TODO(crbug.com/366291600): Remove ubo_binding after switch to immediates.
struct ArrayLengthFromUniformOptions {
    /// The HLSL binding point to use to generate a uniform buffer from which to read buffer sizes.
    BindingPoint ubo_binding;
    /// The offset in immediate block for buffer sizes.
    std::optional<uint32_t> buffer_sizes_offset{};
    /// The mapping from the storage buffer binding points in WGSL binding-point space to the index
    /// into the uniform buffer where the length of the buffer is stored.
    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_size_index;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(ArrayLengthFromUniformOptions,
                 ubo_binding,
                 buffer_sizes_offset,
                 bindpoint_to_size_index);
    bool operator==(const ArrayLengthFromUniformOptions&) const = default;
};

/// Options used to specify a mapping of binding points to indices into a UBO
/// from which to load buffer offsets, or to load them from immediate blocks.
/// TODO(crbug.com/366291600): Remove ubo_binding after switch to immediates.
struct ArrayOffsetFromUniformOptions {
    /// The HLSL binding point to use to generate a uniform buffer from which to read buffer
    /// offsets.
    BindingPoint ubo_binding;
    /// The offset in immediate block for buffer offsets.
    std::optional<uint32_t> buffer_offsets_offset{};
    /// The mapping from the storage buffer binding points in WGSL binding-point space to the index
    /// into the uniform buffer where the offset into the buffer is stored.
    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_offset_index;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(ArrayOffsetFromUniformOptions,
                 ubo_binding,
                 buffer_offsets_offset,
                 bindpoint_to_offset_index);
    bool operator==(const ArrayOffsetFromUniformOptions&) const = default;
};

/// Data for a single pixel local attachment
struct PixelLocalAttachment {
    /// The supported pixel local storage attachment formats
    enum class TexelFormat : uint8_t {
        kR32Sint,
        kR32Uint,
        kR32Float,
        kUndefined,
    };

    // Pixel local storage attachment index
    uint32_t index = uint32_t(-1);

    // Pixel local storage attachment format
    TexelFormat format = TexelFormat::kUndefined;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(PixelLocalAttachment, index, format);
    bool operator==(const PixelLocalAttachment&) const = default;
};

/// Data used to specify pixel local mappings
struct PixelLocalOptions {
    // Index of pixel_local structure member index to attachment info
    std::unordered_map<uint32_t, PixelLocalAttachment> attachments;

    /// The bind group index of all pixel local storage attachments
    uint32_t group_index = 0;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(PixelLocalOptions, attachments, group_index);
    bool operator==(const PixelLocalOptions&) const = default;
};

/// Configuration options used for generating HLSL.
struct Options {
    /// The downstream compiler to be used
    enum class Compiler : uint8_t {
        kFXC,
        kDXC,
    };

    /// The set of options to work around driver issues
    struct Workarounds {
        ///////////////////////////////////////////////////////////////////////////////////////////
        // NOTE: When adding a new option here, it should also be added to the FuzzedOptions     //
        // structure in writer_fuzz.cc.                                                          //
        ///////////////////////////////////////////////////////////////////////////////////////////

        /// Set to `true` to scalarize max, min, and clamp builtins.
        bool scalarize_max_min_clamp = false;

        /// Set to `true` to generate polyfill for `reflect` builtin for vec2<f32>
        bool polyfill_reflect_vec2_f32 = false;

        /// Set to `true` to generate polyfill for `subgroupBroadcast(f16)`
        bool polyfill_subgroup_broadcast_f16 = false;

        TINT_REFLECT(Workarounds,
                     scalarize_max_min_clamp,
                     polyfill_reflect_vec2_f32,
                     polyfill_subgroup_broadcast_f16);
        bool operator==(const Workarounds&) const = default;
    };

    /// The set of options for things which are only available in certain shader models
    struct Extensions {
        ///////////////////////////////////////////////////////////////////////////////////////////
        // NOTE: When adding a new option here, it should also be added to the FuzzedOptions     //
        // structure in writer_fuzz.cc.                                                          //
        ///////////////////////////////////////////////////////////////////////////////////////////

        /// Set to `true` to generate polyfill for `dot4I8Packed` and `dot4U8Packed` builtins
        bool polyfill_dot_4x8_packed = false;

        /// Set to `true` to generate polyfill for `pack4xI8`, `pack4xU8`, `pack4xI8Clamp`,
        /// `unpack4xI8` and `unpack4xU8` builtins
        bool polyfill_pack_unpack_4x8 = false;

        TINT_REFLECT(Extensions, polyfill_dot_4x8_packed, polyfill_pack_unpack_4x8);
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

    /// The entry point to emit
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

    /// Set to `true` to run truncate interstage variables during the shader IO transform.
    bool truncate_interstage_variables = false;

    /// Set to `true` to disable the polyfills on integer division and modulo.
    bool disable_polyfill_integer_div_mod = false;

    /// Any enabled workarounds
    Workarounds workarounds{};

    /// Any enabled extensions
    Extensions extensions{};

    /// The downstream compiler which will be used
    Compiler compiler = Compiler::kDXC;

    /// Options used to specify a mapping of binding points to indices into a UBO
    /// from which to load buffer sizes.
    ArrayLengthFromUniformOptions array_length_from_uniform = {};

    /// Options used to specify a mapping of binding points to indices into a UBO
    /// from which to load buffer offsets.
    ArrayOffsetFromUniformOptions array_offset_from_uniform = {};

    /// Interstage locations actually used as inputs in the next stage of the pipeline.
    /// This is potentially used for truncating unused interstage outputs at current shader stage.
    std::bitset<kMaxInterStageLocations> interstage_locations;

    /// The binding point to use for information passed via root constants.
    std::optional<BindingPoint> root_constant_binding_point;

    /// Immediate binding point info
    std::optional<BindingPoint> immediate_binding_point;

    /// The offset of the first_index_offset push constant.
    std::optional<uint32_t> first_index_offset;

    /// The offset of the first_instance_offset push constant.
    std::optional<uint32_t> first_instance_offset;

    /// Offsets of num_workgroups push constant.
    std::optional<uint32_t> num_workgroups_start_offset;

    /// The bindings
    Bindings bindings;

    /// The binding points that will be ignored by the rebustness transform.
    std::vector<BindingPoint> ignored_by_robustness_transform;

    /// Pixel local configuration
    PixelLocalOptions pixel_local;

    /// Resource table information
    std::optional<ResourceTableConfig> resource_table;

    // Configuration for substitute overrides
    SubstituteOverridesConfig substitute_overrides_config = {};

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(Options,
                 entry_point_name,
                 remapped_entry_point_name,
                 strip_all_names,
                 disable_robustness,
                 disable_integer_range_analysis,
                 disable_workgroup_init,
                 truncate_interstage_variables,
                 disable_polyfill_integer_div_mod,
                 workarounds,
                 extensions,
                 compiler,
                 array_length_from_uniform,
                 array_offset_from_uniform,
                 interstage_locations,
                 root_constant_binding_point,
                 immediate_binding_point,
                 first_index_offset,
                 first_instance_offset,
                 num_workgroups_start_offset,
                 bindings,
                 ignored_by_robustness_transform,
                 pixel_local,
                 resource_table,
                 substitute_overrides_config);
    bool operator==(const Options&) const = default;
};

}  // namespace tint::hlsl::writer

namespace tint {

/// Reflect valid value ranges for the PixelLocalAttachment::TexelFormat enum.
TINT_REFLECT_ENUM_RANGE(hlsl::writer::PixelLocalAttachment::TexelFormat, kR32Sint, kR32Float);
TINT_REFLECT_ENUM_RANGE(hlsl::writer::Options::Compiler, kFXC, kDXC);

}  // namespace tint

#endif  // SRC_TINT_LANG_HLSL_WRITER_COMMON_OPTIONS_H_
