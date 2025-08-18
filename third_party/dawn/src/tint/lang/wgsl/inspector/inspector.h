// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_WGSL_INSPECTOR_INSPECTOR_H_
#define SRC_TINT_LANG_WGSL_INSPECTOR_INSPECTOR_H_

#include <map>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "src/tint/api/common/override_id.h"

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/wgsl/inspector/entry_point.h"
#include "src/tint/lang/wgsl/inspector/resource_binding.h"
#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/lang/wgsl/sem/sampler_texture_pair.h"

namespace tint::inspector {

/// A temporary alias to sem::SamplerTexturePair. [DEPRECATED]
using SamplerTexturePair = sem::SamplerTexturePair;

/// Extracts information from a program
class Inspector {
  public:
    /// Constructor
    /// @param program Shader program to extract information from.
    explicit Inspector(const Program& program);

    /// Destructor
    ~Inspector();

    /// @returns error messages from the Inspector
    std::string error() { return diagnostics_.Str(); }
    /// @returns true if an error was encountered
    bool has_error() const { return diagnostics_.ContainsErrors(); }

    /// @returns vector of entry point information
    std::vector<EntryPoint> GetEntryPoints();

    /// @param entry_point name of the entry point to get information about
    /// @returns the entry point information
    EntryPoint GetEntryPoint(const std::string& entry_point);

    /// @returns map of module-constant name to pipeline constant ID
    std::map<std::string, OverrideId> GetNamedOverrideIds();

    /// @returns vector of all overrides
    std::vector<Override> Overrides();

    /// @param entry_point name of the entry point to get information about.
    /// @returns vector of all of the resource bindings.
    std::vector<ResourceBinding> GetResourceBindings(const std::string& entry_point);

    /// @param entry_point name of the entry point to get information about.
    /// @returns vector of all of the sampler/texture sampling pairs that are used
    /// by that entry point.
    std::vector<SamplerTexturePair> GetSamplerTextureUses(const std::string& entry_point);

    /// @param entry_point name of the entry point to get information about.
    /// @param non_sampler_placeholder the sampler binding point placeholder to use for texture-only
    /// access (e.g., textureLoad).
    /// @returns vector of sampler/texture sampling pairs that are used by given entry point.
    /// Contains all texture usages with and without a sampler. Note that storage textures are not
    /// included.
    std::vector<SamplerTexturePair> GetSamplerAndNonSamplerTextureUses(
        const std::string& entry_point,
        const BindingPoint& non_sampler_placeholder);

    /// @returns vector of all valid extension names used by the program. There
    /// will be no duplicated names in the returned vector even if an extension
    /// is enabled multiple times.
    std::vector<std::string> GetUsedExtensionNames();

    /// The information needed to be supplied.
    enum class TextureQueryType : uint8_t {
        /// Texture Num Levels
        kTextureNumLevels,
        /// Texture Num Samples
        kTextureNumSamples,
    };
    /// Information on level and sample calls by a given texture binding point
    struct LevelSampleInfo {
        /// The type of function
        TextureQueryType type = TextureQueryType::kTextureNumLevels;
        /// The group number
        uint32_t group = 0;
        /// The binding number
        uint32_t binding = 0;

        /// Equality operator
        /// @param rhs the LevelSampleInfo to compare against
        /// @returns true if this LevelSampleInfo is equal to `rhs`
        bool operator==(const LevelSampleInfo& rhs) const {
            return this->type == rhs.type && this->group == rhs.group &&
                   this->binding == rhs.binding;
        }

        /// Inequality operator
        /// @param rhs the LevelSampleInfo to compare against
        /// @returns true if this LevelSampleInfo is not equal to `rhs`
        bool operator!=(const LevelSampleInfo& rhs) const { return !(*this == rhs); }
    };

    /// @param ep the entry point to get the information for
    /// @returns a vector of information for textures which call textureNumLevels and
    /// textureNumSamples for backends which require additional support for those methods. Each
    /// binding point will only be returned once regardless of the number of calls made. The
    /// texture types for `textureNumSamples` is disjoint from the texture types in
    /// `textureNumLevels` so the binding point will always be one or the other.
    std::vector<LevelSampleInfo> GetTextureQueries(const std::string& ep);

  private:
    const Program& program_;
    diag::List diagnostics_;

    struct EntryPointTextureMetadata {
        std::unordered_set<BindingPoint> textures_used_without_samplers;
        std::unordered_set<BindingPoint> textures_with_num_levels;
        std::unordered_set<BindingPoint> textures_with_num_samples;
        std::unordered_set<SamplerTexturePair> sampling_pairs;
        bool has_texture_load_with_depth_texture = false;
        bool has_depth_texture_with_non_comparison_sampler = false;
    };
    std::unordered_map<std::string, EntryPointTextureMetadata> texture_metadata_;

    /// Computes the texture metadata for `entry_point` and returns it.
    const EntryPointTextureMetadata& ComputeTextureMetadata(const std::string& entry_point);

    /// @param name name of the entry point to find
    /// @returns a pointer to the entry point if it exists, otherwise returns
    ///          nullptr and sets the error string.
    const ast::Function* FindEntryPointByName(const std::string& name);

    /// Recursively add entry point IO variables.
    /// If `type` is a struct, recurse into members, appending the member name.
    /// Otherwise, add the variable unless it is a builtin.
    /// @param name the name of the variable being added, including struct nested accessings.
    /// @param variable_name the name of the variable being added
    /// @param type the type of the variable
    /// @param attributes the variable attributes
    /// @param location the location attribute value if provided
    /// @param color the color attribute value if provided
    /// @param blend_src the blend_src attribute value if provided
    /// @param variables the list to add the variables to
    void AddEntryPointInOutVariables(std::string name,
                                     std::string variable_name,
                                     const core::type::Type* type,
                                     VectorRef<const ast::Attribute*> attributes,
                                     std::optional<uint32_t> location,
                                     std::optional<uint32_t> color,
                                     std::optional<uint32_t> blend_src,
                                     std::vector<StageVariable>& variables) const;

    /// Recursively determine if the type contains builtin.
    /// If `type` is a struct, recurse into members to check for the attribute.
    /// Otherwise, check `attributes` for the attribute.
    bool ContainsBuiltin(core::BuiltinValue builtin,
                         const core::type::Type* type,
                         VectorRef<const ast::Attribute*> attributes) const;
    /// Get the array length of the builtin clip_distances when it is used.
    /// @param type the type of the variable
    /// @returns the array length of the builtin clip_distances or empty when it is not used
    std::optional<uint32_t> GetClipDistancesBuiltinSize(const core::type::Type* type) const;

    /// @param attributes attributes associated with the parameter or structure member
    /// @returns the interpolation type and sampling modes for the value
    std::tuple<InterpolationType, InterpolationSampling> CalculateInterpolationData(
        VectorRef<const ast::Attribute*> attributes) const;

    /// @param func the root function of the callgraph to consider for the computation.
    /// @returns the total size in bytes of all Workgroup storage-class storage accessed via func.
    uint32_t ComputeWorkgroupStorageSize(const ast::Function* func) const;

    /// @param func the root function of the callgraph to consider for the computation.
    /// @returns the total size in bytes of all immediate data variables accessed via func.
    uint32_t ComputeImmediateDataSize(const ast::Function* func) const;

    /// @param func the root function of the callgraph to consider for the computation
    /// @returns the list of member types for the `pixel_local` variable accessed via func, if any.
    std::vector<PixelLocalMemberType> ComputePixelLocalMemberTypes(const ast::Function* func) const;

    /// @returns `true` if @p func uses any subgroup matrix types
    bool UsesSubgroupMatrix(const sem::Function* func) const;

    /// @param func the function of the entry point. Must be non-nullptr and true for IsEntryPoint()
    /// @returns the entry point information
    EntryPoint GetEntryPoint(const tint::ast::Function* func);
};

}  // namespace tint::inspector

#endif  // SRC_TINT_LANG_WGSL_INSPECTOR_INSPECTOR_H_
