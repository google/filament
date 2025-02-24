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

#include "dawn/native/vulkan/PipelineLayoutVk.h"

#include <string>
#include <utility>

#include "dawn/common/BitSetIterator.h"
#include "dawn/common/Range.h"
#include "dawn/common/ityp_bitset.h"
#include "dawn/native/vulkan/BindGroupLayoutVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

// static
ResultOrError<Ref<PipelineLayout>> PipelineLayout::Create(
    Device* device,
    const UnpackedPtr<PipelineLayoutDescriptor>& descriptor) {
    Ref<PipelineLayout> layout = AcquireRef(new PipelineLayout(device, descriptor));
    DAWN_TRY(layout->Initialize());
    return layout;
}

ResultOrError<Ref<RefCountedVkHandle<VkPipelineLayout>>> PipelineLayout::CreateVkPipelineLayout(
    uint32_t internalImmediateDataSize) {
    // Compute the array of VkDescriptorSetLayouts that will be chained in the create info.
    BindGroupMask bindGroupMask = GetBindGroupLayoutsMask();
    BindGroupIndex highestBindGroupIndex = GetHighestBitIndexPlusOne(bindGroupMask);
    PerBindGroup<VkDescriptorSetLayout> setLayouts;
    for (BindGroupIndex i : Range(highestBindGroupIndex)) {
        if (bindGroupMask[i]) {
            setLayouts[i] = ToBackend(GetBindGroupLayout(i))->GetHandle();
        } else {
            setLayouts[i] =
                ToBackend(GetDevice()->GetEmptyBindGroupLayout()->GetInternalBindGroupLayout())
                    ->GetHandle();
        }
    }

    VkPipelineLayoutCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.setLayoutCount = static_cast<uint32_t>(highestBindGroupIndex);
    createInfo.pSetLayouts = AsVkArray(setLayouts.data());
    createInfo.pushConstantRangeCount = 0;
    createInfo.pPushConstantRanges = nullptr;

    // Create pipeline layout without internal immediate data size.
    uint32_t immediateDataSize = GetImmediateDataRangeByteSize() + internalImmediateDataSize;
    VkPushConstantRange immediateDataRange;

    // createInfo has been initialized based on PipelineLayout attributes.
    // Create immediateDataRange info based on
    // Record cache key information now to represent pipelineLayout to exclude future changes
    // caused by internal immediate data size from pipeline.
    if (immediateDataSize > 0) {
        immediateDataRange.stageFlags = kImmediateDataRangeShaderStage;
        immediateDataRange.offset = 0;
        immediateDataRange.size = immediateDataSize;
        createInfo.pushConstantRangeCount = 1;
        createInfo.pPushConstantRanges = &immediateDataRange;
    }

    VkPipelineLayout vkPipelineLayout;
    Device* device = ToBackend(GetDevice());
    DAWN_TRY(CheckVkSuccess(device->fn.CreatePipelineLayout(device->GetVkDevice(), &createInfo,
                                                            nullptr, &*vkPipelineLayout),
                            "CreatePipelineLayout"));

    return AcquireRef(new RefCountedVkHandle<VkPipelineLayout>(device, vkPipelineLayout));
}

MaybeError PipelineLayout::Initialize() {
    BindGroupMask bindGroupMask = GetBindGroupLayoutsMask();
    BindGroupIndex highestBindGroupIndex = GetHighestBitIndexPlusOne(bindGroupMask);
    PerBindGroup<const CachedObject*> cachedObjects;
    for (BindGroupIndex i : Range(highestBindGroupIndex)) {
        if (bindGroupMask[i]) {
            cachedObjects[i] = GetBindGroupLayout(i);
        } else {
            cachedObjects[i] = GetDevice()->GetEmptyBindGroupLayout()->GetInternalBindGroupLayout();
        }
    }

    // Record bind group layout objects and user immediate data size into pipeline layout cache key.
    // It represents pipeline layout base attributes and ignored future changes caused by internal
    // immediate data size from pipeline.
    uint32_t numSetLayoutsWithHoles =
        static_cast<uint32_t>(GetHighestBitIndexPlusOne(bindGroupMask));
    StreamIn(&mCacheKey, stream::Iterable(cachedObjects.data(), numSetLayoutsWithHoles),
             GetImmediateDataRangeByteSize());

    return {};
}

ResultOrError<Ref<RefCountedVkHandle<VkPipelineLayout>>> PipelineLayout::GetOrCreateVkLayoutObject(
    uint32_t internalImmediateDataSize) {
    // Check cache
    Ref<RefCountedVkHandle<VkPipelineLayout>> pipelineLayoutVk;
    mVkPipelineLayouts.Use([&](auto vkPipelineLayouts) {
        auto it = vkPipelineLayouts->find(internalImmediateDataSize);
        if (it != vkPipelineLayouts->end()) {
            pipelineLayoutVk = it->second;
        }
    });

    if (pipelineLayoutVk != nullptr) {
        return pipelineLayoutVk;
    }

    DAWN_TRY_ASSIGN(pipelineLayoutVk, CreateVkPipelineLayout(internalImmediateDataSize));

    return mVkPipelineLayouts.Use([&](auto vkPipelineLayouts) {
        return vkPipelineLayouts->insert({internalImmediateDataSize, std::move(pipelineLayoutVk)})
            .first->second;
    });
}

VkShaderStageFlags PipelineLayout::GetImmediateDataRangeStage() const {
    return kImmediateDataRangeShaderStage;
}

PipelineLayout::~PipelineLayout() = default;

void PipelineLayout::DestroyImpl() {
    PipelineLayoutBase::DestroyImpl();
    mVkPipelineLayouts->clear();
}

void PipelineLayout::SetLabelImpl() {
    Device* device = ToBackend(GetDevice());
    const std::string& label = GetLabel();

    mVkPipelineLayouts.Use([&](auto vkPipelineLayouts) {
        for (const auto& [_, vkLayout] : *vkPipelineLayouts) {
            SetDebugName(device, vkLayout->Get(), "Dawn_PipelineLayout", label);
        }
    });
}

}  // namespace dawn::native::vulkan
