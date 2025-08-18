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

MaybeError ComputePipeline::InitializeImpl() {
    Device* device = ToBackend(GetDevice());
    PipelineLayout* layout = ToBackend(GetLayout());

    // The cache key is only used for storing VkPipelineCache objects in BlobStore. That's not
    // done with the monolithic pipeline cache so it's unnecessary work and memory usage.
    bool buildCacheKey =
        !device->GetTogglesState().IsEnabled(Toggle::VulkanMonolithicPipelineCache);
    if (buildCacheKey) {
        // Vulkan devices need cache UUID field to be serialized into pipeline cache keys.
        StreamIn(&mCacheKey, device->GetDeviceInfo().properties.pipelineCacheUUID);
    }

    // Compute pipeline doesn't have clamp depth feature.
    // TODO(crbug.com/366291600): Setting immediate data size if needed.
    DAWN_TRY(PipelineVk::InitializeBase(layout, mImmediateMask));

    VkComputePipelineCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.layout = GetVkLayout();
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
                    module->GetHandleAndSpirv(SingleShaderStage::Compute, computeStage, layout,
                                              /*emitPointSize*/ false, GetImmediateMask()));

    createInfo.stage.module = moduleAndSpirv.module;
    // string_view returned by GetIsolatedEntryPointName() points to a null-terminated string.
    createInfo.stage.pName = device->GetIsolatedEntryPointName().data();
    createInfo.stage.pSpecializationInfo = nullptr;

    // This is required to ensure SubgroupSize is reported as the actual size of the subgroups
    // (even if some invocations may be disabled), and that the subgroup size will be uniform
    // across the entire dispatch. This becomes unnecessary with SPIR-V 1.6.
    createInfo.flags |= VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT;

    // If the shader stage uses subgroup matrix types, we need to enable full subgroups to guarantee
    // that all shader invocations are active. This becomes unnecessary with SPIR-V 1.6.
    if (computeStage.metadata->usesSubgroupMatrix) {
        createInfo.flags |= VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT;
    }

    VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT subgroupSizeInfo = {};
    PNextChainBuilder stageExtChain(&createInfo.stage);

    uint32_t computeSubgroupSize = device->GetComputeSubgroupSize();
    if (computeSubgroupSize != 0u) {
        DAWN_ASSERT(device->GetDeviceInfo().HasExt(DeviceExt::SubgroupSizeControl));
        subgroupSizeInfo.requiredSubgroupSize = computeSubgroupSize;
        stageExtChain.Add(
            &subgroupSizeInfo,
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO_EXT);
    }

    if (buildCacheKey) {
        // Record cache key information now since the createInfo is not stored.
        StreamIn(&mCacheKey, createInfo, layout, moduleAndSpirv.spirv);
    }

    // Try to see if we have anything in the blob cache.
    platform::metrics::DawnHistogramTimer cacheTimer(GetDevice()->GetPlatform());
    Ref<PipelineCache> cache = ToBackend(GetDevice()->GetOrCreatePipelineCache(GetCacheKey()));
    DAWN_TRY(
        CheckVkSuccess(device->fn.CreateComputePipelines(device->GetVkDevice(), cache->GetHandle(),
                                                         1, &createInfo, nullptr, &*mHandle),
                       "CreateComputePipelines"));
    cacheTimer.RecordMicroseconds(cache->CacheHit() ? "Vulkan.CreateComputePipelines.CacheHit"
                                                    : "Vulkan.CreateComputePipelines.CacheMiss");

    DAWN_TRY(cache->DidCompilePipeline());

    SetLabelImpl();

    device->fn.DestroyShaderModule(device->GetVkDevice(), moduleAndSpirv.module, nullptr);

    return {};
}

void ComputePipeline::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mHandle, "Dawn_ComputePipeline", GetLabel());
}

ComputePipeline::~ComputePipeline() = default;

void ComputePipeline::DestroyImpl() {
    ComputePipelineBase::DestroyImpl();
    PipelineVk::DestroyImpl();
    if (mHandle != VK_NULL_HANDLE) {
        ToBackend(GetDevice())->GetFencedDeleter()->DeleteWhenUnused(mHandle);
        mHandle = VK_NULL_HANDLE;
    }
}

VkPipeline ComputePipeline::GetHandle() const {
    return mHandle;
}

}  // namespace dawn::native::vulkan
