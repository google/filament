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
#include "src/tint/api/common/vertex_pulling_config.h"
#include "src/tint/utils/reflection.h"

namespace tint::msl::writer {

/// An external texture
struct ExternalTexture {
    /// Metadata
    BindingPoint metadata{};
    /// Plane0 binding data
    BindingPoint plane0{};
    /// Plane1 binding data;
    BindingPoint plane1{};

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(ExternalTexture, metadata, plane0, plane1);
    TINT_REFLECT_EQUALS(ExternalTexture);
    TINT_REFLECT_HASH_CODE(ExternalTexture);
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

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(Bindings, uniform, storage, texture, storage_texture, sampler, external_texture);
    TINT_REFLECT_EQUALS(Bindings);
    TINT_REFLECT_HASH_CODE(Bindings);
};

/// Options used to specify a mapping of binding points to indices into a UBO
/// from which to load buffer sizes.
/// TODO(crbug.com/366291600): Remove ubo_binding after switch to immediates.
struct ArrayLengthOptions {
    /// The MSL binding point to use to generate a uniform buffer from which to read buffer sizes.
    std::optional<uint32_t> ubo_binding;

    /// The offset in immediate block for buffer sizes.
    std::optional<uint32_t> buffer_sizes_offset;

    /// The mapping from the storage buffer binding points in WGSL binding-point space to the index
    /// into the uniform buffer where the length of the buffer is stored.
    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_size_index;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(ArrayLengthOptions, ubo_binding, buffer_sizes_offset, bindpoint_to_size_index);
    TINT_REFLECT_EQUALS(ArrayLengthOptions);
    TINT_REFLECT_HASH_CODE(ArrayLengthOptions);
};

/// Information to configure an argument buffer
struct ArgumentBufferInfo {
    /// The buffer ID to use for this argument buffer
    uint32_t id;

    /// The buffer ID to use for the dynamic buffer if needed
    std::optional<uint32_t> dynamic_buffer_id{};

    /// Dynamic offsets map. The map is binding number -> offset index
    std::unordered_map<uint32_t, uint32_t> binding_info_to_offset_index{};

    TINT_REFLECT(ArgumentBufferInfo, id, dynamic_buffer_id, binding_info_to_offset_index);
    TINT_REFLECT_EQUALS(ArgumentBufferInfo);
    TINT_REFLECT_HASH_CODE(ArgumentBufferInfo);
};

/// Configuration options used for generating MSL.
struct Options {
    /// Constructor
    Options();
    /// Destructor
    ~Options();
    /// Copy constructor
    Options(const Options&);
    /// Copy assignment
    /// @returns this Options
    Options& operator=(const Options&);

    /// An optional remapped name to use when emitting the entry point.
    std::string remapped_entry_point_name = {};

    /// Set to `true` to strip all user-declared identifiers from the module.
    bool strip_all_names = false;

    /// Set to `true` to disable software robustness that prevents out-of-bounds accesses.
    bool disable_robustness = false;

    /// Set to `true` to enable integer range analysis in robustness transform.
    bool enable_integer_range_analysis = false;

    /// Set to `true` to disable workgroup memory zero initialization
    bool disable_workgroup_init = false;

    /// Set to `true` to disable demote to helper transform
    bool disable_demote_to_helper = false;

    /// Set to `true` to generate a [[point_size]] attribute which is set to 1.0
    /// for all vertex shaders in the module.
    bool emit_vertex_point_size = false;

    /// Set to `true` to disable the polyfills on integer division and modulo.
    bool disable_polyfill_integer_div_mod = false;

    /// Set to `true` to scalarize max min and clamp builtins.
    bool scalarize_max_min_clamp = false;

    /// Set to `true` to disable the module constant transform for f16
    bool disable_module_constant_f16 = false;

    /// Emit argument buffers
    bool use_argument_buffers = false;

    /// The index to use when generating a UBO to receive storage buffer sizes.
    /// Defaults to 30, which is the last valid buffer slot.
    uint32_t buffer_size_ubo_index = 30;

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

    /// The bindings.
    Bindings bindings;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(Options,
                 remapped_entry_point_name,
                 strip_all_names,
                 disable_robustness,
                 enable_integer_range_analysis,
                 disable_workgroup_init,
                 disable_demote_to_helper,
                 emit_vertex_point_size,
                 disable_polyfill_integer_div_mod,
                 scalarize_max_min_clamp,
                 disable_module_constant_f16,
                 use_argument_buffers,
                 buffer_size_ubo_index,
                 fixed_sample_mask,
                 pixel_local_attachments,
                 array_length_from_constants,
                 vertex_pulling_config,
                 immediate_binding_point,
                 group_to_argument_buffer_info,
                 bindings);
    TINT_REFLECT_EQUALS(Options);
    TINT_REFLECT_HASH_CODE(Options);
};

}  // namespace tint::msl::writer

#endif  // SRC_TINT_LANG_MSL_WRITER_COMMON_OPTIONS_H_
