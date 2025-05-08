// Copyright 2017 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_SHADERMODULE_H_
#define SRC_DAWN_NATIVE_SHADERMODULE_H_

#include <bitset>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/strings/string_view.h"
#include "dawn/common/Constants.h"
#include "dawn/common/ContentLessObjectCacheable.h"
#include "dawn/common/MutexProtected.h"
#include "dawn/common/RefCountedWithExternalCount.h"
#include "dawn/common/ityp_array.h"
#include "dawn/native/BindingInfo.h"
#include "dawn/native/CachedObject.h"
#include "dawn/native/CompilationMessages.h"
#include "dawn/native/Error.h"
#include "dawn/native/Format.h"
#include "dawn/native/Forward.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/Limits.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/PerStage.h"
#include "dawn/native/dawn_platform.h"
#include "tint/tint.h"

namespace tint {

class Program;

namespace ast::transform {
class DataMap;
class Manager;
class Transform;
class VertexPulling;
}  // namespace ast::transform

}  // namespace tint

namespace dawn::native {

struct EntryPointMetadata;

// Base component type of an inter-stage variable
enum class InterStageComponentType {
    I32,
    U32,
    F32,
    F16,
};

enum class InterpolationType {
    Perspective,
    Linear,
    Flat,
};

enum class InterpolationSampling {
    None,
    Center,
    Centroid,
    Sample,
    First,
    Either,
};

enum class PixelLocalMemberType {
    I32,
    U32,
    F32,
};

// Use map to make sure constant keys are sorted for creating shader cache keys
using PipelineConstantEntries = std::map<std::string, double>;

// A map from name to EntryPointMetadata.
using EntryPointMetadataTable =
    absl::flat_hash_map<std::string, std::unique_ptr<EntryPointMetadata>>;

struct TintProgram : public RefCounted {
    TintProgram(tint::Program program, std::unique_ptr<tint::Source::File> file)
        : program(std::move(program)), file(std::move(file)) {}
    const tint::Program program;
    const std::unique_ptr<tint::Source::File> file;  // Keep the tint::Source::File alive
};

struct ShaderModuleParseResult {
    ShaderModuleParseResult();
    ~ShaderModuleParseResult();
    ShaderModuleParseResult(ShaderModuleParseResult&& rhs);
    ShaderModuleParseResult& operator=(ShaderModuleParseResult&& rhs);

    bool HasParsedShader() const;

    Ref<TintProgram> tintProgram;
};

struct ShaderModuleEntryPoint {
    bool defaulted;
    std::string name;
};

MaybeError ValidateAndParseShaderModule(
    DeviceBase* device,
    const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
    const std::vector<tint::wgsl::Extension>& internalExtensions,
    ShaderModuleParseResult* parseResult,
    OwnedCompilationMessages* outMessages);
MaybeError ValidateCompatibilityWithPipelineLayout(DeviceBase* device,
                                                   const EntryPointMetadata& entryPoint,
                                                   const PipelineLayoutBase* layout);

// Return extent3D with workgroup size dimension info if it is valid.
// width = x, height = y, depthOrArrayLength = z.
ResultOrError<Extent3D> ValidateComputeStageWorkgroupSize(
    const tint::Program& program,
    const char* entryPointName,
    bool usesSubgroupMatrix,
    uint32_t maxSubgroupSize,
    const LimitsForCompilationRequest& limits,
    const LimitsForCompilationRequest& adaterSupportedlimits);

// Return extent3D with workgroup size dimension info if it is valid.
// width = x, height = y, depthOrArrayLength = z.
ResultOrError<Extent3D> ValidateComputeStageWorkgroupSize(
    uint32_t x,
    uint32_t y,
    uint32_t z,
    size_t workgroupStorageSize,
    bool usesSubgroupMatrix,
    uint32_t maxSubgroupSize,
    const LimitsForCompilationRequest& limits,
    const LimitsForCompilationRequest& adaterSupportedlimits);

RequiredBufferSizes ComputeRequiredBufferSizesForLayout(const EntryPointMetadata& entryPoint,
                                                        const PipelineLayoutBase* layout);
ResultOrError<tint::Program> RunTransforms(tint::ast::transform::Manager* transformManager,
                                           const tint::Program* program,
                                           const tint::ast::transform::DataMap& inputs,
                                           tint::ast::transform::DataMap* outputs,
                                           OwnedCompilationMessages* messages);

// Shader metadata for a binding, very similar to information contained in a pipeline layout.
struct ShaderBindingInfo {
    BindingNumber binding;

    // The variable name of the binding resource.
    std::string name;

    std::variant<BufferBindingInfo,
                 SamplerBindingInfo,
                 TextureBindingInfo,
                 StorageTextureBindingInfo,
                 ExternalTextureBindingInfo,
                 InputAttachmentBindingInfo>
        bindingInfo;
};

using BindingGroupInfoMap = absl::flat_hash_map<BindingNumber, ShaderBindingInfo>;
using BindingInfoArray = ityp::array<BindGroupIndex, BindingGroupInfoMap, kMaxBindGroups>;

// The WebGPU override variables only support these scalar types
union OverrideScalar {
    // Use int32_t for boolean to initialize the full 32bit
    int32_t b;
    float f32;
    int32_t i32;
    uint32_t u32;
};

// Contains all the reflection data for a valid (ShaderModule, entryPoint, stage). They are
// stored in the ShaderModuleBase and destroyed only when the shader program is destroyed so
// pointers to EntryPointMetadata are safe to store as long as you also keep a Ref to the
// ShaderModuleBase.
struct EntryPointMetadata {
    // It is valid for a shader to contain entry points that go over limits. To keep this
    // structure with packed arrays and bitsets, we still validate against limits when
    // doing reflection, but store the errors in this vector, for later use if the application
    // tries to use the entry point.
    std::vector<std::string> infringedLimitErrors;

    // bindings[G][B] is the reflection data for the binding defined with @group(G) @binding(B)
    BindingInfoArray bindings;

    struct SamplerTexturePair {
        BindingSlot sampler;
        BindingSlot texture;
    };
    std::vector<SamplerTexturePair> samplerTexturePairs;

    // The set of vertex attributes this entryPoint uses.
    PerVertexAttribute<VertexFormatBaseType> vertexInputBaseTypes;
    VertexAttributeMask usedVertexInputs;

    // An array to record the basic types (float, int and uint) of the fragment shader framebuffer
    // input/outputs (inputs being "framebuffer fetch").
    struct FragmentRenderAttachmentInfo {
        TextureComponentType baseType;
        uint8_t componentCount;
        uint8_t blendSrc;
    };
    PerColorAttachment<FragmentRenderAttachmentInfo> fragmentOutputVariables;
    ColorAttachmentMask fragmentOutputMask;

    PerColorAttachment<FragmentRenderAttachmentInfo> fragmentInputVariables;
    ColorAttachmentMask fragmentInputMask;

    struct InterStageVariableInfo {
        std::string name;
        InterStageComponentType baseType;
        uint32_t componentCount;
        InterpolationType interpolationType;
        InterpolationSampling interpolationSampling;
    };
    // Now that we only support vertex and fragment stages, there can't be both inter-stage
    // inputs and outputs in one shader stage.
    std::vector<bool> usedInterStageVariables;
    std::vector<InterStageVariableInfo> interStageVariables;
    uint32_t totalInterStageShaderVariables;

    // The shader stage for this entry point.
    SingleShaderStage stage;

    struct Override {
        tint::OverrideId id;

        // Match tint::inspector::Override::Type
        // Bool is defined as a macro on linux X11 and cannot compile
        enum class Type { Boolean, Float32, Uint32, Int32, Float16 } type;

        // If the constant doesn't not have an initializer in the shader
        // Then it is required for the pipeline stage to have a constant record to initialize a
        // value
        bool isInitialized;

        // Set to true if the override is used in the entry point
        bool isUsed = true;
    };

    using OverridesMap = absl::flat_hash_map<std::string, Override>;

    // Map identifier to override variable
    // Identifier is unique: either the variable name or the numeric ID if specified
    OverridesMap overrides;

    // Override variables that are not initialized in shaders
    // They need value initialization from pipeline stage or it is a validation error
    absl::flat_hash_set<std::string> uninitializedOverrides;

    // Store constants with shader initialized values as well
    // This is used by metal backend to set values with default initializers that are not
    // overridden
    absl::flat_hash_set<std::string> initializedOverrides;

    // Reflection information about potential `pixel_local` variable use.
    bool usesPixelLocal = false;
    size_t pixelLocalBlockSize = 0;
    std::vector<PixelLocalMemberType> pixelLocalMembers;

    bool usesFragDepth = false;
    bool usesInstanceIndex = false;
    bool usesNumWorkgroups = false;
    bool usesSampleMaskOutput = false;
    bool usesSampleIndex = false;
    bool usesVertexIndex = false;
    bool usesTextureLoadWithDepthTexture = false;
    bool usesDepthTextureWithNonComparisonSampler = false;
    bool usesSubgroupMatrix = false;

    // Immediate Data block byte size
    uint32_t immediateDataRangeByteSize = 0;

    // Number of texture+sampler combinations, computed as
    // 1 for every texture+sampler combination + 1 for every texture used
    // without a sampler that wasn't previously counted.
    // Note: this is only set in compatibility mode.
    uint32_t numTextureSamplerCombinations = 0;
};

class ShaderModuleBase : public RefCountedWithExternalCount<ApiObjectBase>,
                         public CachedObject,
                         public ContentLessObjectCacheable<ShaderModuleBase> {
  public:
    using Base = RefCountedWithExternalCount<ApiObjectBase>;
    ShaderModuleBase(DeviceBase* device,
                     const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
                     std::vector<tint::wgsl::Extension> internalExtensions,
                     ApiObjectBase::UntrackedByDeviceTag tag);
    ShaderModuleBase(DeviceBase* device,
                     const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
                     std::vector<tint::wgsl::Extension> internalExtensions);
    ~ShaderModuleBase() override;

    static Ref<ShaderModuleBase> MakeError(DeviceBase* device, StringView label);

    ObjectType GetType() const override;

    // Return true iff the program has an entrypoint called `entryPoint`.
    bool HasEntryPoint(absl::string_view entryPoint) const;

    // Return the number of entry points for a stage.
    size_t GetEntryPointCount(SingleShaderStage stage) const { return mEntryPointCounts[stage]; }

    // Return the entry point for a stage. If no entry point name, returns the default one.
    ShaderModuleEntryPoint ReifyEntryPointName(StringView entryPointName,
                                               SingleShaderStage stage) const;

    // Return the metadata for the given `entryPoint`. HasEntryPoint with the same argument
    // must be true.
    const EntryPointMetadata& GetEntryPoint(absl::string_view entryPoint) const;

    // Functions necessary for the unordered_set<ShaderModuleBase*>-based cache.
    size_t ComputeContentHash() override;

    struct EqualityFunc {
        bool operator()(const ShaderModuleBase* a, const ShaderModuleBase* b) const;
    };

    std::optional<bool> GetStrictMath() const;

    using ScopedUseTintProgram = APIRef<ShaderModuleBase>;
    ScopedUseTintProgram UseTintProgram();

    // Get tintProgram, (re)create it if necessary.
    Ref<TintProgram> GetTintProgram();

    Future APIGetCompilationInfo(const WGPUCompilationInfoCallbackInfo& callbackInfo);

    void InjectCompilationMessages(std::unique_ptr<OwnedCompilationMessages> compilationMessages);
    OwnedCompilationMessages* GetCompilationMessages() const;

    // Return nullable tintProgram directly without any recreation, can be used for testing the
    // releasing/recreation behaviors.
    Ref<TintProgram> GetNullableTintProgramForTesting() const;
    int GetTintProgramRecreateCountForTesting() const;

  protected:
    void DestroyImpl() override;

    MaybeError InitializeBase(ShaderModuleParseResult* parseResult,
                              OwnedCompilationMessages* compilationMessages);

  private:
    ShaderModuleBase(DeviceBase* device, ObjectBase::ErrorTag tag, StringView label);

    void WillDropLastExternalRef() override;

    // The original data in the descriptor for caching.
    enum class Type { Undefined, Spirv, Wgsl };
    Type mType;
    std::vector<uint32_t> mOriginalSpirv;
    std::string mWgsl;

    // TODO(dawn:2503): Remove the optional when Dawn can has a consistent default across backends.
    // Right now D3D uses strictness by default, and Vulkan/Metal use fast math by default.
    std::optional<bool> mStrictMath;

    EntryPointMetadataTable mEntryPoints;
    PerStage<std::string> mDefaultEntryPointNames;
    PerStage<size_t> mEntryPointCounts;

    struct TintData {
        // tintProgram is nullable so that it can be lazily (re)generated right before actual using.
        Ref<TintProgram> tintProgram = nullptr;
        int tintProgramRecreateCount = 0;
    };
    MutexProtected<TintData> mTintData;

    std::unique_ptr<OwnedCompilationMessages> mCompilationMessages;

    const std::vector<tint::wgsl::Extension> mInternalExtensions;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_SHADERMODULE_H_
