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

#ifndef SRC_TINT_LANG_GLSL_WRITER_COMMON_OPTIONS_H_
#define SRC_TINT_LANG_GLSL_WRITER_COMMON_OPTIONS_H_

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/glsl/writer/common/version.h"

namespace tint::glsl::writer {

/// Generic binding point
struct BindingInfo {
    /// The binding
    uint32_t binding = 0;

    /// Equality operator
    /// @param rhs the BindingInfo to compare against
    /// @returns true if this BindingInfo is equal to `rhs`
    inline bool operator==(const BindingInfo& rhs) const { return binding == rhs.binding; }
    /// Inequality operator
    /// @param rhs the BindingInfo to compare against
    /// @returns true if this BindingInfo is not equal to `rhs`
    inline bool operator!=(const BindingInfo& rhs) const { return !(*this == rhs); }

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(BindingInfo, binding);
};

/// An external texture
struct ExternalTexture {
    /// Metadata
    BindingInfo metadata{};
    /// Plane0 binding data
    BindingInfo plane0{};
    /// Plane1 binding data
    BindingInfo plane1{};

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(ExternalTexture, metadata, plane0, plane1);
};

/// A combined texture/sampler pair
// Note, these are the WGSL binding points that are used to create the combined samplers
struct CombinedTextureSamplerPair {
    /// The WGSL texture binding
    BindingPoint texture = {};
    /// The WGSL sampler binding
    BindingPoint sampler = {};

    /// Flag if this is an external textures plane1
    bool is_external_plane1 = false;

    /// Equality operator
    /// @param rhs the CombinedTextureSamplerPair to compare against
    /// @returns true if this CombinedTextureSamplerPair is equal to `rhs`
    inline bool operator==(const CombinedTextureSamplerPair& rhs) const {
        return texture == rhs.texture && sampler == rhs.sampler;
    }

    /// Less then operator
    /// @param rhs the CombinedTextureSamplerPair to compare against
    /// @returns if this is less then rhs
    inline bool operator<(const CombinedTextureSamplerPair& rhs) const {
        if (texture < rhs.texture) {
            return true;
        }
        if (texture == rhs.texture) {
            return sampler < rhs.sampler;
        }
        return false;
    }

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(CombinedTextureSamplerPair, texture, sampler, is_external_plane1);
};

}  // namespace tint::glsl::writer

namespace std {

/// Custom std::hash specialization for tint::glsl::writer::BindingInfo
template <>
class hash<tint::glsl::writer::BindingInfo> {
  public:
    /// @param n the binding info
    /// @return the hash value
    inline std::size_t operator()(const tint::glsl::writer::BindingInfo& n) const {
        return tint::Hash(n.binding);
    }
};

/// Custom std::hash specialization for tint::glsl::writer::CombinedTextureSamplerPair
template <>
class hash<tint::glsl::writer::CombinedTextureSamplerPair> {
  public:
    /// @param n the combined sampler texture pair
    /// @return the hash value
    inline std::size_t operator()(const tint::glsl::writer::CombinedTextureSamplerPair& n) const {
        return tint::Hash(n.texture, n.sampler);
    }
};

}  // namespace std

namespace tint::glsl::writer {

using BindingMap = std::unordered_map<BindingPoint, BindingInfo>;
using ExternalTextureBindings = std::unordered_map<BindingPoint, ExternalTexture>;
using CombinedTextureSamplerInfo = std::unordered_map<CombinedTextureSamplerPair, std::string>;

/// Options used to specify a mapping of binding points to indices into a UBO
/// from which to load texture builtin values.
struct TextureBuiltinsFromUniformOptions {
    /// The binding point to use to generate a uniform buffer from which to read texture builtin
    /// values. Note that this is a post-remapping binding.
    BindingInfo ubo_binding = {};

    /// Ordered list of post-remapping bindings in the uniform buffer for polyfilling
    /// `textureNumSamples` and `textureNumLevels`.
    struct EmulatedBuiltin {
        uint32_t offset;
        uint32_t count;
        BindingInfo binding;

        TINT_REFLECT(EmulatedBuiltin, offset, count, binding);
    };
    std::vector<EmulatedBuiltin> ubo_contents = {};

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(TextureBuiltinsFromUniformOptions, ubo_binding, ubo_contents);
};

/// Options used to specify a mapping of binding points to indices into a UBO
/// from which to load buffer sizes.
struct ArrayLengthFromUniformOptions {
    /// The binding point to use to generate a uniform buffer from which to read buffer sizes.
    BindingPoint ubo_binding;
    /// The mapping from the storage buffer binding points in WGSL binding-point space to the index
    /// into the uniform buffer where the length of the buffer is stored.
    std::unordered_map<BindingPoint, uint32_t> bindpoint_to_size_index;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(ArrayLengthFromUniformOptions, ubo_binding, bindpoint_to_size_index);
};

/// Binding information
struct Bindings {
    /// Constructor
    Bindings();
    /// Destructor
    ~Bindings();

    /// Copy constructor
    Bindings(const Bindings&) = default;

    /// Copy assign
    Bindings& operator=(const Bindings&) = default;

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

    /// A map of SamplerTexturePair to combined sampler names for the
    /// CombineSamplers transform
    CombinedTextureSamplerInfo sampler_texture_to_name;

    /// The binding point to use for placeholder samplers.
    BindingPoint placeholder_sampler_bind_point;

    /// Options used to map WGSL textureNumLevels/textureNumSamples builtins to internal uniform
    /// buffer values. If not specified, emits corresponding GLSL builtins
    /// textureQueryLevels/textureSamples directly.
    TextureBuiltinsFromUniformOptions texture_builtins_from_uniform = {};

    /// Options used to specify a mapping of binding points to indices into a UBO
    /// from which to load buffer sizes. If not specified, emits corresponding GLSL builtins
    /// length() directly
    ArrayLengthFromUniformOptions array_length_from_uniform = {};

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(Bindings,
                 uniform,
                 storage,
                 texture,
                 storage_texture,
                 sampler,
                 external_texture,
                 sampler_texture_to_name,
                 placeholder_sampler_bind_point,
                 texture_builtins_from_uniform,
                 array_length_from_uniform);
};

/// Configuration options used for generating GLSL.
struct Options {
    struct RangeOffsets {
        /// The offset of the min_depth immediate data
        uint32_t min = 0;
        /// The offset of the max_depth immediate data
        uint32_t max = 0;

        /// Reflect the fields of this class so that it can be used by tint::ForeachField()
        TINT_REFLECT(RangeOffsets, min, max);
    };

    /// Constructor
    Options();

    /// Destructor
    ~Options();

    /// Copy constructor
    Options(const Options&);

    /// Set to `true` to strip all user-declared identifiers from the module.
    bool strip_all_names = false;

    /// Set to `true` to disable software robustness that prevents out-of-bounds accesses.
    bool disable_robustness = false;

    /// Set to `true` to enable integer range analysis in robustness transform.
    bool enable_integer_range_analysis = false;

    /// Set to `true` to disable workgroup memory zero initialization
    bool disable_workgroup_init = false;

    /// Set to `true` to disable the polyfills on integer division and modulo.
    bool disable_polyfill_integer_div_mod = false;

    /// Set to `true` to run ArrayLengthFromTransform workaround
    bool use_array_length_from_uniform = false;

    /// The GLSL version to emit
    Version version;

    /// Offset of the firstVertex immediate data.
    std::optional<uint32_t> first_vertex_offset;

    /// Offset of the firstInstance immediate data.
    std::optional<uint32_t> first_instance_offset;

    /// Offsets of the minDepth and maxDepth immediate data.
    std::optional<RangeOffsets> depth_range_offsets;

    /// Vertex inputs to perform BGRA swizzle on.
    std::unordered_set<uint32_t> bgra_swizzle_locations;

    /// The bindings
    Bindings bindings{};

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(Options,
                 strip_all_names,
                 disable_robustness,
                 enable_integer_range_analysis,
                 disable_workgroup_init,
                 disable_polyfill_integer_div_mod,
                 use_array_length_from_uniform,
                 version,
                 first_vertex_offset,
                 first_instance_offset,
                 depth_range_offsets,
                 bgra_swizzle_locations,
                 bindings);
};

}  // namespace tint::glsl::writer

#endif  // SRC_TINT_LANG_GLSL_WRITER_COMMON_OPTIONS_H_
