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
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/tint/api/common/override_id.h"

#include "src/tint/lang/core/builtin_value.h"
#include "src/tint/lang/wgsl/inspector/entry_point.h"
#include "src/tint/lang/wgsl/inspector/resource_binding.h"
#include "src/tint/lang/wgsl/inspector/scalar.h"
#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/sampler_texture_pair.h"
#include "src/tint/utils/containers/unique_vector.h"

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

    /// @returns map of override identifier to initial value
    std::map<OverrideId, Scalar> GetOverrideDefaultValues();

    /// @returns map of module-constant name to pipeline constant ID
    std::map<std::string, OverrideId> GetNamedOverrideIds();

    /// @param entry_point name of the entry point to get information about.
    /// @returns vector of all of the resource bindings.
    std::vector<ResourceBinding> GetResourceBindings(const std::string& entry_point);

    /// @param entry_point name of the entry point to get information about.
    /// @returns vector of all of the bindings for uniform buffers.
    std::vector<ResourceBinding> GetUniformBufferResourceBindings(const std::string& entry_point);

    /// @param entry_point name of the entry point to get information about.
    /// @returns vector of all of the bindings for storage buffers.
    std::vector<ResourceBinding> GetStorageBufferResourceBindings(const std::string& entry_point);

    /// @param entry_point name of the entry point to get information about.
    /// @returns vector of all of the bindings for read-only storage buffers.
    std::vector<ResourceBinding> GetReadOnlyStorageBufferResourceBindings(
        const std::string& entry_point);

    /// @param entry_point name of the entry point to get information about.
    /// @returns vector of all of the bindings for regular samplers.
    std::vector<ResourceBinding> GetSamplerResourceBindings(const std::string& entry_point);

    /// @param entry_point name of the entry point to get information about.
    /// @returns vector of all of the bindings for comparison samplers.
    std::vector<ResourceBinding> GetComparisonSamplerResourceBindings(
        const std::string& entry_point);

    /// @param entry_point name of the entry point to get information about.
    /// @returns vector of all of the bindings for sampled textures.
    std::vector<ResourceBinding> GetSampledTextureResourceBindings(const std::string& entry_point);

    /// @param entry_point name of the entry point to get information about.
    /// @returns vector of all of the bindings for multisampled textures.
    std::vector<ResourceBinding> GetMultisampledTextureResourceBindings(
        const std::string& entry_point);

    /// @param entry_point name of the entry point to get information about.
    /// @returns vector of all of the bindings for write-only storage textures.
    std::vector<ResourceBinding> GetStorageTextureResourceBindings(const std::string& entry_point);

    /// @param entry_point name of the entry point to get information about.
    /// @returns vector of all of the bindings for depth textures.
    std::vector<ResourceBinding> GetDepthTextureResourceBindings(const std::string& entry_point);

    /// @param entry_point name of the entry point to get information about.
    /// @returns vector of all of the bindings for depth textures.
    std::vector<ResourceBinding> GetDepthMultisampledTextureResourceBindings(
        const std::string& entry_point);

    /// @param entry_point name of the entry point to get information about.
    /// @returns vector of all of the bindings for external textures.
    std::vector<ResourceBinding> GetExternalTextureResourceBindings(const std::string& entry_point);

    /// Gathers all the resource bindings of the input attachment type for the given
    /// entry point.
    /// @param entry_point name of the entry point to get information about.
    /// texture type.
    /// @returns vector of all of the bindings for input attachments.
    std::vector<ResourceBinding> GetInputAttachmentResourceBindings(const std::string& entry_point);

    /// @param entry_point name of the entry point to get information about.
    /// @returns vector of all of the sampler/texture sampling pairs that are used
    /// by that entry point.
    VectorRef<SamplerTexturePair> GetSamplerTextureUses(const std::string& entry_point);

    /// @param entry_point name of the entry point to get information about.
    /// @param placeholder the sampler binding point to use for texture-only
    /// access (e.g., textureLoad)
    /// @returns vector of all of the sampler/texture sampling pairs that are used
    /// by that entry point.
    std::vector<SamplerTexturePair> GetSamplerTextureUses(const std::string& entry_point,
                                                          const BindingPoint& placeholder);

    /// @returns vector of all valid extension names used by the program. There
    /// will be no duplicated names in the returned vector even if an extension
    /// is enabled multiple times.
    std::vector<std::string> GetUsedExtensionNames();

    /// @returns vector of all enable directives used by the program, each
    /// enable directive represented by a std::pair<std::string,
    /// tint::Source::Range> for its extension name and its location of the
    /// extension name. There may be multiple enable directives for a same
    /// extension.
    std::vector<std::pair<std::string, Source>> GetEnableDirectives();

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
    std::unique_ptr<std::unordered_map<std::string, UniqueVector<SamplerTexturePair, 4>>>
        sampler_targets_;

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
    /// Gathers all the texture resource bindings of the given type for the given
    /// entry point.
    /// @param entry_point name of the entry point to get information about.
    /// @param texture_type the type of the textures to gather.
    /// @param resource_type the ResourceBinding::ResourceType for the given
    /// texture type.
    /// @returns vector of all of the bindings for depth textures.
    std::vector<ResourceBinding> GetTextureResourceBindings(
        const std::string& entry_point,
        const tint::TypeInfo* texture_type,
        ResourceBinding::ResourceType resource_type);

    /// @param entry_point name of the entry point to get information about.
    /// @param read_only if true get only read-only bindings, if false get
    ///                  write-only bindings.
    /// @returns vector of all of the bindings for the requested storage buffers.
    std::vector<ResourceBinding> GetStorageBufferResourceBindingsImpl(
        const std::string& entry_point,
        bool read_only);

    /// @param entry_point name of the entry point to get information about.
    /// @param multisampled_only only get multisampled textures if true, otherwise
    ///                          only get sampled textures.
    /// @returns vector of all of the bindings for the request storage buffers.
    std::vector<ResourceBinding> GetSampledTextureResourceBindingsImpl(
        const std::string& entry_point,
        bool multisampled_only);

    /// @param entry_point name of the entry point to get information about.
    /// @returns vector of all of the bindings for the requested storage textures.
    std::vector<ResourceBinding> GetStorageTextureResourceBindingsImpl(
        const std::string& entry_point);

    /// Constructs |sampler_targets_| if it hasn't already been instantiated.
    void GenerateSamplerTargets();

    /// @param attributes attributes associated with the parameter or structure member
    /// @returns the interpolation type and sampling modes for the value
    std::tuple<InterpolationType, InterpolationSampling> CalculateInterpolationData(
        VectorRef<const ast::Attribute*> attributes) const;

    /// @param func the root function of the callgraph to consider for the computation.
    /// @returns the total size in bytes of all Workgroup storage-class storage accessed via func.
    uint32_t ComputeWorkgroupStorageSize(const ast::Function* func) const;

    /// @param func the root function of the callgraph to consider for the computation.
    /// @returns the total size in bytes of all push_constant variables accessed via func.
    uint32_t ComputePushConstantSize(const ast::Function* func) const;

    /// @param func the root function of the callgraph to consider for the computation
    /// @returns the list of member types for the `pixel_local` variable accessed via func, if any.
    std::vector<PixelLocalMemberType> ComputePixelLocalMemberTypes(const ast::Function* func) const;

    /// For a N-uple of expressions, resolve to the appropriate global resources
    /// and call 'cb'.
    /// 'cb' may be called multiple times.
    /// Assumes that not being able to resolve the resources is an error, so will
    /// invoke TINT_ICE when that occurs.
    /// @tparam N number of expressions in the n-uple
    /// @tparam F type of the callback provided.
    /// @param exprs N-uple of expressions to resolve.
    /// @param callsite the callsite the expressions in the n-uple originated from
    /// @param cb is a callback function with the signature:
    /// `void(std::array<const sem::GlobalVariable*, N>, em::Function* callsite)`,
    /// which is invoked whenever a set of expressions are resolved to globals. The `callsite`
    /// provides the function where we determined the sampler,texture global variables. This
    /// is the starting point to determine which entry points are using this sampler,texture.
    template <size_t N, typename F>
    void GetOriginatingResources(std::array<const ast::Expression*, N> exprs,
                                 const ast::CallExpression* callsite,
                                 F&& cb);

    /// @param func the function of the entry point. Must be non-nullptr and true for IsEntryPoint()
    /// @returns the entry point information
    EntryPoint GetEntryPoint(const tint::ast::Function* func);

    /// The information needed to be supplied.
    enum class TextureUsageType : uint8_t {
        /// textureLoad
        kTextureLoad,
        /// textureNumLevels
        kTextureNumLevels,
        /// textureNumSamples
        kTextureNumSamples,
        /// depth texture with non-comparison sampler
        kDepthTextureWithNonComparisonSampler,
    };
    /// Information on level and sample calls by a given texture binding point
    struct TextureUsageInfo {
        /// The type of function
        TextureUsageType type = TextureUsageType::kTextureNumLevels;
        /// The group number
        uint32_t group = 0;
        /// The binding number
        uint32_t binding = 0;
    };

    std::vector<Inspector::TextureUsageInfo> GetTextureUsagesForEntryPoint(
        const tint::ast::Function& ep,
        std::function<std::optional<TextureUsageType>(const tint::sem::Call* call,
                                                      tint::wgsl::BuiltinFn builtin_fn)> filter);
};

}  // namespace tint::inspector

#endif  // SRC_TINT_LANG_WGSL_INSPECTOR_INSPECTOR_H_
