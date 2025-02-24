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
#include "src/tint/lang/core/access.h"
#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/reflection.h"

namespace tint::hlsl::writer {

namespace binding {

/// Generic binding point
struct BindingInfo {
    /// The group
    uint32_t group = 0;
    /// The binding
    uint32_t binding = 0;

    /// Equality operator
    /// @param rhs the BindingInfo to compare against
    /// @returns true if this BindingInfo is equal to `rhs`
    inline bool operator==(const BindingInfo& rhs) const {
        return group == rhs.group && binding == rhs.binding;
    }
    /// Inequality operator
    /// @param rhs the BindingInfo to compare against
    /// @returns true if this BindingInfo is not equal to `rhs`
    inline bool operator!=(const BindingInfo& rhs) const { return !(*this == rhs); }

    /// @returns the hash code of the BindingInfo
    tint::HashCode HashCode() const { return Hash(group, binding); }

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(BindingInfo, group, binding);
};

using Uniform = BindingInfo;
using Storage = BindingInfo;
using Texture = BindingInfo;
using StorageTexture = BindingInfo;
using Sampler = BindingInfo;

/// An external texture
struct ExternalTexture {
    /// Metadata
    BindingInfo metadata{};
    /// Plane0 binding data
    BindingInfo plane0{};
    /// Plane1 binding data;
    BindingInfo plane1{};

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(ExternalTexture, metadata, plane0, plane1);
};

}  // namespace binding

/// Maps the WGSL binding point to the SPIR-V group,binding for uniforms
using UniformBindings = std::unordered_map<BindingPoint, binding::Uniform>;
/// Maps the WGSL binding point to the SPIR-V group,binding for storage
using StorageBindings = std::unordered_map<BindingPoint, binding::Storage>;
/// Maps the WGSL binding point to the SPIR-V group,binding for textures
using TextureBindings = std::unordered_map<BindingPoint, binding::Texture>;
/// Maps the WGSL binding point to the SPIR-V group,binding for storage textures
using StorageTextureBindings = std::unordered_map<BindingPoint, binding::StorageTexture>;
/// Maps the WGSL binding point to the SPIR-V group,binding for samplers
using SamplerBindings = std::unordered_map<BindingPoint, binding::Sampler>;
/// Maps the WGSL binding point to the plane0, plane1, and metadata for external textures
using ExternalTextureBindings = std::unordered_map<BindingPoint, binding::ExternalTexture>;

/// Binding information
struct Bindings {
    /// Uniform bindings
    UniformBindings uniform{};
    /// Storage bindings
    StorageBindings storage{};
    /// Texture bindings
    TextureBindings texture{};
    /// Storage texture bindings
    StorageTextureBindings storage_texture{};
    /// Sampler bindings
    SamplerBindings sampler{};
    /// External bindings
    ExternalTextureBindings external_texture{};
    /// Mapping of BindingPoint to new Access
    std::unordered_map<BindingPoint, tint::core::Access> access_controls;
    /// The binding points that will be ignored by the rebustness transform.
    std::vector<BindingPoint> ignored_by_robustness_transform;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(Bindings,
                 uniform,
                 storage,
                 texture,
                 storage_texture,
                 sampler,
                 external_texture,
                 access_controls,
                 ignored_by_robustness_transform);
};

/// kMaxInterStageLocations == D3D11_PS_INPUT_REGISTER_COUNT - 2
/// D3D11_PS_INPUT_REGISTER_COUNT == D3D12_PS_INPUT_REGISTER_COUNT
constexpr uint32_t kMaxInterStageLocations = 30;

/// Options used to specify a mapping of binding points to indices into a UBO
/// from which to load buffer sizes.
struct ArrayLengthFromUniformOptions {
    /// The HLSL binding point to use to generate a uniform buffer from which to read buffer sizes.
    binding::Uniform ubo_binding;
    /// The mapping from the storage buffer binding points in WGSL binding-point space to the index
    /// into the uniform buffer where the length of the buffer is stored.
    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_size_index;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(ArrayLengthFromUniformOptions, ubo_binding, bindpoint_to_size_index);
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
};

/// Data used to specify pixel local mappings
struct PixelLocalOptions {
    // Index of pixel_local structure member index to attachment info
    std::unordered_map<uint32_t, PixelLocalAttachment> attachments;

    /// The bind group index of all pixel local storage attachments
    uint32_t group_index = 0;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(PixelLocalOptions, attachments, group_index);
};

/// Configuration options used for generating HLSL.
struct Options {
    /// The downstream compiler to be used
    enum class Compiler : uint8_t {
        kFXC,
        kDXC,
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

    /// An optional remapped name to use when emitting the entry point.
    std::string remapped_entry_point_name = {};

    /// Set to `true` to strip all user-declared identifiers from the module.
    bool strip_all_names = false;

    /// Set to `true` to disable software robustness that prevents out-of-bounds accesses.
    bool disable_robustness = false;

    /// Set to `true` to disable workgroup memory zero initialization
    bool disable_workgroup_init = false;

    /// Set to `true` to run the TruncateInterstageVariables transform.
    bool truncate_interstage_variables = false;

    /// Set to `true` to generate polyfill for `reflect` builtin for vec2<f32>
    bool polyfill_reflect_vec2_f32 = false;

    /// Set to `true` to generate polyfill for `dot4I8Packed` and `dot4U8Packed` builtins
    bool polyfill_dot_4x8_packed = false;

    /// Set to `true` to disable the polyfills on integer division and modulo.
    bool disable_polyfill_integer_div_mod = false;

    /// Set to `true` to generate polyfill for `pack4xI8`, `pack4xU8`, `pack4xI8Clamp`,
    /// `unpack4xI8` and `unpack4xU8` builtins
    bool polyfill_pack_unpack_4x8 = false;

    /// The downstream compiler which will be used
    Compiler compiler = Compiler::kDXC;

    /// Options used to specify a mapping of binding points to indices into a UBO
    /// from which to load buffer sizes.
    ArrayLengthFromUniformOptions array_length_from_uniform = {};

    /// Interstage locations actually used as inputs in the next stage of the pipeline.
    /// This is potentially used for truncating unused interstage outputs at current shader stage.
    std::bitset<kMaxInterStageLocations> interstage_locations;

    /// The binding point to use for information passed via root constants.
    std::optional<BindingPoint> root_constant_binding_point;

    /// The bindings
    Bindings bindings;

    /// Pixel local configuration
    PixelLocalOptions pixel_local;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(Options,
                 remapped_entry_point_name,
                 strip_all_names,
                 disable_robustness,
                 disable_workgroup_init,
                 truncate_interstage_variables,
                 polyfill_reflect_vec2_f32,
                 polyfill_dot_4x8_packed,
                 disable_polyfill_integer_div_mod,
                 polyfill_pack_unpack_4x8,
                 compiler,
                 array_length_from_uniform,
                 interstage_locations,
                 root_constant_binding_point,
                 bindings,
                 pixel_local);
};

}  // namespace tint::hlsl::writer

namespace tint {

/// Reflect valid value ranges for the PixelLocalAttachment::TexelFormat enum.
TINT_REFLECT_ENUM_RANGE(hlsl::writer::PixelLocalAttachment::TexelFormat, kR32Sint, kR32Float);
TINT_REFLECT_ENUM_RANGE(hlsl::writer::Options::Compiler, kFXC, kDXC);

}  // namespace tint

#endif  // SRC_TINT_LANG_HLSL_WRITER_COMMON_OPTIONS_H_
