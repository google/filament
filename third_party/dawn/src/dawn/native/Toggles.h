// Copyright 2019 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_TOGGLES_H_
#define SRC_DAWN_NATIVE_TOGGLES_H_

#include <bitset>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "dawn/common/BitSetIterator.h"
#include "dawn/native/DawnNative.h"

namespace dawn::native {

struct DawnTogglesDescriptor;

enum class Toggle {
    EmulateStoreAndMSAAResolve,
    NonzeroClearResourcesOnCreationForTesting,
    AlwaysResolveIntoZeroLevelAndLayer,
    LazyClearResourceOnFirstUse,
    DisableLazyClearForMappedAtCreationBuffer,
    TurnOffVsync,
    UseTemporaryBufferInCompressedTextureToTextureCopy,
    UseD3D12ResourceHeapTier2,
    UseD3D12RenderPass,
    UseD3D12ResidencyManagement,
    DisableResourceSuballocation,
    SkipValidation,
    VulkanUseD32S8,
    VulkanUseS8,
    MetalDisableSamplerCompare,
    MetalUseSharedModeForCounterSampleBuffer,
    DisableBaseVertex,
    DisableBaseInstance,
    DisableIndexedDrawBuffers,
    DisableSampleVariables,
    UseD3D12SmallShaderVisibleHeapForTesting,
    UseDXC,
    DisableRobustness,
    MetalEnableVertexPulling,
    AllowUnsafeAPIs,
    FlushBeforeClientWaitSync,
    UseTempBufferInSmallFormatTextureToTextureCopyFromGreaterToLessMipLevel,
    EmitHLSLDebugSymbols,
    DisallowSpirv,
    DumpShaders,
    DisableWorkgroupInit,
    DisableDemoteToHelper,
    VulkanUseDemoteToHelperInvocationExtension,
    DisableSymbolRenaming,
    UseUserDefinedLabelsInBackend,
    UsePlaceholderFragmentInVertexOnlyPipeline,
    FxcOptimizations,
    RecordDetailedTimingInTraceEvents,
    DisableTimestampQueryConversion,
    TimestampQuantization,
    ClearBufferBeforeResolveQueries,
    VulkanUseZeroInitializeWorkgroupMemoryExtension,
    MetalRenderR8RG8UnormSmallMipToTempTexture,
    DisableBlobCache,
    D3D12ForceClearCopyableDepthStencilTextureOnCreation,
    D3D12DontSetClearValueOnDepthTextureCreation,
    D3D12AlwaysUseTypelessFormatsForCastableTexture,
    D3D12AllocateExtraMemoryFor2DArrayColorTexture,
    D3D12UseTempBufferInDepthStencilTextureAndBufferCopyWithNonZeroBufferOffset,
    D3D12UseTempBufferInTextureToTextureCopyBetweenDifferentDimensions,
    ApplyClearBigIntegerColorValueWithDraw,
    MetalUseMockBlitEncoderForWriteTimestamp,
    MetalDisableTimestampPeriodEstimation,
    VulkanSplitCommandBufferOnComputePassAfterRenderPass,
    DisableSubAllocationFor2DTextureWithCopyDstOrRenderAttachment,
    MetalUseCombinedDepthStencilFormatForStencil8,
    MetalUseBothDepthAndStencilAttachmentsForCombinedDepthStencilFormats,
    MetalKeepMultisubresourceDepthStencilTexturesInitialized,
    MetalFillEmptyOcclusionQueriesWithZero,
    UseBlitForBufferToDepthTextureCopy,
    UseBlitForBufferToStencilTextureCopy,
    UseBlitForStencilTextureWrite,
    UseBlitForDepthTextureToTextureCopyToNonzeroSubresource,
    UseBlitForDepth16UnormTextureToBufferCopy,
    UseBlitForDepth32FloatTextureToBufferCopy,
    UseBlitForStencilTextureToBufferCopy,
    UseBlitForSnormTextureToBufferCopy,
    UseBlitForBGRA8UnormTextureToBufferCopy,
    UseBlitForRGB9E5UfloatTextureCopy,
    UseBlitForRG11B10UfloatTextureCopy,
    UseBlitForFloat16TextureCopy,
    UseBlitForFloat32TextureCopy,
    UseBlitForT2B,
    UseBlitForB2T,
    D3D11DisableCPUUploadBuffers,
    UseT2B2TForSRGBTextureCopy,
    D3D12ReplaceAddWithMinusWhenDstFactorIsZeroAndSrcFactorIsDstAlpha,
    D3D12PolyfillReflectVec2F32,
    VulkanClearGen12TextureWithCCSAmbiguateOnCreation,
    D3D12UseRootSignatureVersion1_1,
    VulkanUseImageRobustAccess2,
    VulkanUseBufferRobustAccess2,
    D3D12Use64KBAlignedMSAATexture,
    ResolveMultipleAttachmentInSeparatePasses,
    D3D12CreateNotZeroedHeap,
    D3D12DontUseNotZeroedHeapFlagOnTexturesAsCommitedResources,
    UseTintIR,
    D3DDisableIEEEStrictness,
    PolyFillPacked4x8DotProduct,
    PolyfillPackUnpack4x8Norm,
    D3D12PolyFillPackUnpack4x8,
    ExposeWGSLTestingFeatures,
    ExposeWGSLExperimentalFeatures,
    DisablePolyfillsOnIntegerDivisonAndModulo,
    EnableImmediateErrorHandling,
    VulkanUseStorageInputOutput16,
    D3D12DontUseShaderModel66OrHigher,
    UsePackedDepth24UnormStencil8Format,
    D3D12ForceStencilComponentReplicateSwizzle,
    D3D12ExpandShaderResourceStateTransitionsToCopySource,
    GLDepthBiasModifier,
    GLForceES31AndNoExtensions,
    VulkanMonolithicPipelineCache,
    MetalSerializeTimestampGenerationAndResolution,
    D3D12RelaxMinSubgroupSizeTo8,
    D3D12RelaxBufferTextureCopyPitchAndOffsetAlignment,
    UseVulkanMemoryModel,

    // Unresolved issues.
    NoWorkaroundSampleMaskBecomesZeroForAllButLastColorTarget,
    NoWorkaroundIndirectBaseVertexNotApplied,
    NoWorkaroundDstAlphaAsSrcBlendFactorForBothColorAndAlphaDoesNotWork,

    ClearColorWithDraw,
    VulkanSkipDraw,

    D3D11UseUnmonitoredFence,
    D3D11DisableFence,
    IgnoreImportedAHardwareBufferVulkanImageSize,

    EnumCount,
    InvalidEnum = EnumCount,
};

// A wrapper of the bitset to store if a toggle is present or not. This wrapper provides the
// convenience to convert the enums of enum class Toggle to the indices of a bitset.
struct TogglesSet {
    std::bitset<static_cast<size_t>(Toggle::EnumCount)> bitset;
    using Iterator = BitSetIterator<static_cast<size_t>(Toggle::EnumCount), uint32_t>;

    void Set(Toggle toggle, bool enabled);
    bool Has(Toggle toggle) const;
    size_t Count() const;
    Iterator Iterate() const;
};

namespace stream {
class Sink;
}

// TogglesState hold the actual state of toggles for instances, adapters and devices. Each toggle
// is in of one of these states:
//    - set
//    - defaulted to enable
//    - disabled
//    - force set to enabled
//    - force set to disabled
//    - unset without default (and thus implicitly disabled).
class TogglesState {
  public:
    // Create an empty toggles state of given stage
    explicit TogglesState(ToggleStage stage);

    // Create a TogglesState from a DawnTogglesDescriptor, only considering toggles of
    // required toggle stage.
    static TogglesState CreateFromTogglesDescriptor(const DawnTogglesDescriptor* togglesDesc,
                                                    ToggleStage requiredStage);

    // Inherit from a given toggles state of earlier stage, only inherit the forced and the
    // unrequired toggles to allow overriding. Return *this to allow method chaining manner.
    TogglesState& InheritFrom(const TogglesState& inheritedToggles);

    // Set a toggle of the same stage of toggles state stage if and only if it is not already set.
    void Default(Toggle toggle, bool enabled);
    // Force set a toggle of same stage of toggles state stage. A force-set toggle will get
    // inherited to all later stage as forced.
    void ForceSet(Toggle toggle, bool enabled);

    // Set a toggle of any stage for testing propose. Return *this to allow method chaining
    // manner.
    TogglesState& SetForTesting(Toggle toggle, bool enabled, bool forced);

    // Return whether the toggle is set or not. Force-set is always treated as set.
    bool IsSet(Toggle toggle) const;
    // Return true if and only if the toggle is set to true.
    bool IsEnabled(Toggle toggle) const;
    ToggleStage GetStage() const;
    std::vector<const char*> GetEnabledToggleNames() const;
    std::vector<const char*> GetDisabledToggleNames() const;

    // Friend definition of StreamIn which can be found by ADL to override stream::StreamIn<T>. This
    // allows writing TogglesState to stream for cache key.
    friend void StreamIn(stream::Sink* sink, const TogglesState& togglesState);

  private:
    // Indicating which stage of toggles state is this object holding for, instance, adapter, or
    // device.
    const ToggleStage mStage;
    TogglesSet mTogglesSet;
    TogglesSet mEnabledToggles;
    TogglesSet mForcedToggles;
};

const char* ToggleEnumToName(Toggle toggle);

class TogglesInfo {
  public:
    TogglesInfo();
    ~TogglesInfo();

    // Retrieves all fo the ToggleInfo information
    static std::vector<const ToggleInfo*> AllToggleInfos();

    // Used to query the details of a toggle. Return nullptr if toggleName is not a valid name
    // of a toggle supported in Dawn.
    const ToggleInfo* GetToggleInfo(const char* toggleName);
    // Used to query the details of a toggle enum. The enum value must not be Toggle::InvalidEnum,
    // as Toggle::InvalidEnum doesn't has corresponding ToggleInfo.
    static const ToggleInfo* GetToggleInfo(Toggle toggle);
    Toggle ToggleNameToEnum(const char* toggleName);

  private:
    void EnsureToggleNameToEnumMapInitialized();

    bool mToggleNameToEnumMapInitialized = false;
    absl::flat_hash_map<std::string, Toggle> mToggleNameToEnumMap;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_TOGGLES_H_
