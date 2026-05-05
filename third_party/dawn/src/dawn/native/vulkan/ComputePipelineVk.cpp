// Copyright 2018 The Dawn & Tint Authors
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

#include "dawn/native/vulkan/ComputePipelineVk.h"

#include <memory>
#include <utility>
#include <vector>

#include "dawn/native/CreatePipelineAsyncEvent.h"
#include "dawn/native/ImmediateConstantsLayout.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/PipelineCacheVk.h"
#include "dawn/native/vulkan/PipelineLayoutVk.h"
#include "dawn/native/vulkan/ShaderModuleVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"
#include "dawn/platform/metrics/HistogramMacros.h"

namespace dawn::native::vulkan {

// static
Ref<ComputePipeline> ComputePipeline::CreateUninitialized(
    Device* device,
    const UnpackedPtr<ComputePipelineDescriptor>& descriptor) {
    return AcquireRef(new ComputePipeline(device, descriptor));
}

ResultOrError<Extent3D> ComputePipeline::InitializeImpl() {
    if (GetDevice()->NeedsStaticSamplerForExternalTexture() && GetLayout()->HasExternalTextures()) {
        DAWN_ASSERT(!GetLayout()->HasAPIStaticSamplers());
        mRequiresSpecialization = true;
    }

    // The cache key is only used for storing VkPipelineCache objects in BlobStore. That's not
    // done with the monolithic pipeline cache so it's unnecessary work and memory usage.
    bool buildCacheKey =
        !GetDevice()->GetTogglesState().IsEnabled(Toggle::VulkanMonolithicPipelineCache);

    Specialization specialization = {
        .layout = {.pushConstantBytes = ToPushConstantBytes(mImmediateMask)},
    };

    SpecializationResult r;
    DAWN_TRY_ASSIGN(r, InitializeSpecialization(specialization, buildCacheKey));
    mHandles = {.pipeline = r.pipeline->Get(), .layout = r.layout->Get()};
    Extent3D workgroupSize = r.workgroupSize;

    mSpecializations.emplace(std::move(specialization), std::move(r));

    return workgroupSize;
}

ResultOrError<PipelineHandles> ComputePipeline::GetOrCreateSpecializedHandle(
    Specialization&& specializationIn) {
    Specialization specialization = specializationIn;
    specialization.layout.pushConstantBytes = ToPushConstantBytes(mImmediateMask);

    if (auto it = mSpecializations.find(specialization); it != mSpecializations.end()) {
        return PipelineHandles{.pipeline = it->second.pipeline->Get(),
                               .layout = it->second.layout->Get()};
    }

    // Do no make a new cache key, so that the VkPipelineCache from InitializeImpl is used for all
    // specializations.
    SpecializationResult r;
    DAWN_TRY_ASSIGN(r, InitializeSpecialization(specialization, /*buildCacheKey=*/false));

    auto handles = PipelineHandles{.pipeline = r.pipeline->Get(), .layout = r.layout->Get()};

    mSpecializations.emplace(std::move(specialization), std::move(r));
    return handles;
}

ResultOrError<ComputePipeline::SpecializationResult> ComputePipeline::InitializeSpecialization(
    const Specialization& specialization,
    bool buildCacheKey) {
    Device* device = ToBackend(GetDevice());
    PipelineLayout* layout = ToBackend(GetLayout());

    if (buildCacheKey) {
        // Vulkan devices need cache UUID field to be serialized into pipeline cache keys.
        StreamIn(&mCacheKey, device->GetDeviceInfo().properties.pipelineCacheUUID);
    }

    SpecializationResult result;
    DAWN_TRY_ASSIGN(result.layout,
                    layout->GetOrCreateVkLayoutObject(std::move(specialization.layout)));

    VkComputePipelineCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.layout = result.layout->Get();
    createInfo.basePipelineHandle = VkPipeline{};
    createInfo.basePipelineIndex = -1;

    createInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    createInfo.stage.pNext = nullptr;
    createInfo.stage.flags = 0;
    createInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    // Generate a new VkShaderModule with BindingRemapper tint transform for each pipeline
    const ProgrammableStage& computeStage = GetStage(SingleShaderStage::Compute);
    ShaderModule* module = ToBackend(computeStage.module.Get());

    ShaderModule::ModuleAndSpirv moduleAndSpirv;
    DAWN_TRY_ASSIGN(moduleAndSpirv,
                    module->GetHandleAndSpirv({
                        .stage = &computeStage,
                        .layout = layout,
                        .immediateMask = GetImmediateMask(),
                        .ycbcrExternalTextures = &specialization.ycbcrExternalTextures,
                    }));
    result.workgroupSize = moduleAndSpirv.workgroupSize;

    createInfo.stage.module = moduleAndSpirv.module;
    // string_view returned by GetIsolatedEntryPointName() points to a null-terminated string.
    createInfo.stage.pName = device->GetIsolatedEntryPointName().data();
    createInfo.stage.pSpecializationInfo = nullptr;

    // If the shader stage uses subgroup matrix types or explicit subgroup size, we need to enable
    // full subgroups to guarantee that all shader invocations are active. This becomes unnecessary
    // with SPIR-V 1.6.
    if (computeStage.metadata->usesSubgroupMatrix ||
        moduleAndSpirv.explicitSubgroupSize.has_value()) {
        createInfo.stage.flags |= VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT;
    }

    VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT subgroupSizeInfo = {};
    PNextChainBuilder stageExtChain(&createInfo.stage);

    std::optional<uint32_t> explicitSubgroupSize;
    if (moduleAndSpirv.explicitSubgroupSize.has_value()) {
        explicitSubgroupSize = moduleAndSpirv.explicitSubgroupSize;
    } else {
        explicitSubgroupSize = device->GetComputeSubgroupSize();
    }

    if (explicitSubgroupSize.has_value()) {
        DAWN_ASSERT(device->GetDeviceInfo().HasExt(DeviceExt::SubgroupSizeControl));
        subgroupSizeInfo.requiredSubgroupSize = explicitSubgroupSize.value();
        stageExtChain.Add(
            &subgroupSizeInfo,
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO_EXT);
    } else {
        // This is required to ensure SubgroupSize is reported as the actual size of the subgroups
        // (even if some invocations may be disabled), and that the subgroup size will be uniform
        // across the entire dispatch. This becomes unnecessary with SPIR-V 1.6. Note that according
        // to Vulkan SPEC (VUID-VkPipelineShaderStageCreateInfo-pNext-02754) flags must not have the
        // `VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT flag` set if a
        // `VkPipelineShaderStageRequiredSubgroupSizeCreateInfo` structure is included in the pNext
        // chain,
        createInfo.stage.flags |= VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT;
    }

    // Record cache key information now since createInfo is not stored. Only store for the noop
    // specialization created in InitializeImpl so that future specializations use the same pipeline
    // cache, and may reuse the VkPipeline when they happen to be the same on the driver side.
    if (buildCacheKey) {
        StreamIn(&mCacheKey, createInfo, layout, moduleAndSpirv.spirv);
    }

    // Try to see if we have anything in the blob cache.
    platform::metrics::DawnHistogramTimer cacheTimer(GetDevice()->GetPlatform());
    Ref<PipelineCache> cache = ToBackend(GetDevice()->GetOrCreatePipelineCache(GetCacheKey()));
    VkPipeline pipeline;
    DAWN_TRY(
        CheckVkSuccess(device->fn.CreateComputePipelines(device->GetVkDevice(), cache->GetHandle(),
                                                         1, &createInfo, nullptr, &*pipeline),
                       "CreateComputePipelines"));
    result.pipeline = AcquireRef(new RefCountedVkHandle<VkPipeline>(device, pipeline));
    cacheTimer.RecordMicroseconds(cache->CacheHit() ? "Vulkan.CreateComputePipelines.CacheHit"
                                                    : "Vulkan.CreateComputePipelines.CacheMiss");

    DAWN_TRY(cache->DidCompilePipeline());

    SetLabelImpl();

    device->fn.DestroyShaderModule(device->GetVkDevice(), moduleAndSpirv.module, nullptr);

    return result;
}

void ComputePipeline::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mHandles.pipeline, "Dawn_ComputePipeline", GetLabel());
}

ComputePipeline::~ComputePipeline() = default;

void ComputePipeline::DestroyImpl(DestroyReason reason) {
    ComputePipelineBase::DestroyImpl(reason);

    mSpecializations.clear();

    // Handles were owned by refs in mSpecializations that were just deleted.
    mHandles = {};
}

bool ComputePipeline::RequiresSpecialization() const {
    return mRequiresSpecialization;
}

VkPipeline ComputePipeline::GetHandle() const {
    DAWN_ASSERT(mHandles.pipeline != VK_NULL_HANDLE);
    return mHandles.pipeline;
}

VkPipelineLayout ComputePipeline::GetVkLayout() const {
    DAWN_ASSERT(mHandles.layout != nullptr);
    return mHandles.layout;
}

}  // namespace dawn::native::vulkan
