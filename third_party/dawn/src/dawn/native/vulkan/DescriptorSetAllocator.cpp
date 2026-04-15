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

#include "dawn/native/vulkan/DescriptorSetAllocator.h"

#include <algorithm>
#include <utility>

#include "dawn/native/Queue.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

// TODO(https://crbug.com/439522242): Consider adding some better heuristic, like an exponential
// increase up to a larger constant.
static constexpr uint32_t kMaxDescriptorsPerPool = 512;

// static
Ref<DescriptorSetAllocator> DescriptorSetAllocator::Create(
    Device* device,
    absl::flat_hash_map<VkDescriptorType, uint32_t> descriptorCountPerType) {
    return AcquireRef(new DescriptorSetAllocator(device, descriptorCountPerType));
}

DescriptorSetAllocator::DescriptorSetAllocator(
    Device* device,
    absl::flat_hash_map<VkDescriptorType, uint32_t> descriptorCountPerType)
    : mDevice(device) {
    // Compute the total number of descriptors for this layout.
    uint32_t totalDescriptorCount = 0;
    mPoolSizes.reserve(descriptorCountPerType.size());
    for (const auto& [type, count] : descriptorCountPerType) {
        DAWN_ASSERT(count > 0);
        totalDescriptorCount += count;
        mPoolSizes.push_back(VkDescriptorPoolSize{type, count});
    }

    // Always assume there is one descriptor requested to avoid a division by 0 below.
    totalDescriptorCount = std::max(1u, totalDescriptorCount);

    // Compute the total number of descriptors sets that fits given the max but always make sure
    // that at least one descriptor set can be made (bindings with visibility none can force giant
    // sets to be made).
    mMaxSets = std::max(kMaxDescriptorsPerPool / totalDescriptorCount, 1u);
    DAWN_ASSERT(mMaxSets > 0);

    // Grow the number of descriptors in the pool to fit the computed |mMaxSets|.
    for (auto& poolSize : mPoolSizes) {
        poolSize.descriptorCount *= mMaxSets;
    }
}

DescriptorSetAllocator::~DescriptorSetAllocator() {
    for (auto& pool : mDescriptorPools) {
        DAWN_ASSERT(pool.freeSetIndices.size() == mMaxSets);
        if (pool.vkPool != VK_NULL_HANDLE) {
            mDevice->GetFencedDeleter()->DeleteWhenUnused(pool.vkPool);
        }
    }
}

ResultOrError<DescriptorSetAllocation> DescriptorSetAllocator::Allocate(
    VkDescriptorSetLayout dsLayout) {
    Mutex::AutoLock lock(&mMutex);

    if (mAvailableDescriptorPoolIndices.empty()) {
        DAWN_TRY(AllocateDescriptorPool(dsLayout));
    }

    DAWN_ASSERT(!mAvailableDescriptorPoolIndices.empty());

    const PoolIndex poolIndex = mAvailableDescriptorPoolIndices.back();
    DescriptorPool* pool = &mDescriptorPools[poolIndex];

    DAWN_ASSERT(!pool->freeSetIndices.empty());

    SetIndex setIndex = pool->freeSetIndices.back();
    pool->freeSetIndices.pop_back();

    if (pool->freeSetIndices.empty()) {
        mAvailableDescriptorPoolIndices.pop_back();
    }

    return DescriptorSetAllocation{pool->sets[setIndex], poolIndex, setIndex};
}

void DescriptorSetAllocator::Deallocate(DescriptorSetAllocation* allocationInfo) {
    bool enqueueDeferredDeallocation = false;

    {
        Mutex::AutoLock lock(&mMutex);

        DAWN_ASSERT(allocationInfo != nullptr);
        DAWN_ASSERT(allocationInfo->set != VK_NULL_HANDLE);

        // We can't reuse the descriptor set right away because the Vulkan spec says in the
        // documentation for vkCmdBindDescriptorSets that the set may be consumed any time between
        // host execution (on "bindful" GPU that inline the descriptors in the command stream) of
        // the command and the end of the draw/dispatch (for "bindless" GPUs where shaders read
        // directly from the VkDescriptorPool).
        const ExecutionSerial serial = mDevice->GetQueue()->GetPendingCommandSerial();
        mPendingDeallocations.Enqueue({allocationInfo->poolIndex, allocationInfo->setIndex},
                                      serial);

        if (mLastDeallocationSerial != serial) {
            enqueueDeferredDeallocation = true;
            mLastDeallocationSerial = serial;
        }

        // Clear the content of the allocation so that use after frees are more visible.
        *allocationInfo = {};
    }

    if (enqueueDeferredDeallocation) {
        // Release lock before calling EnqueueDeferredDeallocation() to avoid lock acquisition
        // order inversion with lock used there.
        mDevice->EnqueueDeferredDeallocation(this);
    }
}

void DescriptorSetAllocator::FinishDeallocation(ExecutionSerial completedSerial) {
    Mutex::AutoLock lock(&mMutex);

    for (const Deallocation& dealloc : mPendingDeallocations.IterateUpTo(completedSerial)) {
        DAWN_ASSERT(dealloc.poolIndex < mDescriptorPools.size());

        auto& freeSetIndices = mDescriptorPools[dealloc.poolIndex].freeSetIndices;
        if (freeSetIndices.empty()) {
            mAvailableDescriptorPoolIndices.emplace_back(dealloc.poolIndex);
        }
        freeSetIndices.emplace_back(dealloc.setIndex);
    }
    mPendingDeallocations.ClearUpTo(completedSerial);
}

MaybeError DescriptorSetAllocator::AllocateDescriptorPool(VkDescriptorSetLayout dsLayout) {
    VkDescriptorPoolCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.maxSets = mMaxSets;
    createInfo.poolSizeCount = mPoolSizes.size();
    createInfo.pPoolSizes = mPoolSizes.data();

    VkDescriptorPool descriptorPool;
    DAWN_TRY(CheckVkSuccess(mDevice->fn.CreateDescriptorPool(mDevice->GetVkDevice(), &createInfo,
                                                             nullptr, &*descriptorPool),
                            "CreateDescriptorPool"));

    std::vector<VkDescriptorSetLayout> layouts(mMaxSets, dsLayout);

    VkDescriptorSetAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.descriptorPool = descriptorPool;
    allocateInfo.descriptorSetCount = mMaxSets;
    allocateInfo.pSetLayouts = AsVkArray(layouts.data());

    std::vector<VkDescriptorSet> sets(mMaxSets);
    MaybeError result =
        CheckVkSuccess(mDevice->fn.AllocateDescriptorSets(mDevice->GetVkDevice(), &allocateInfo,
                                                          AsVkArray(sets.data())),
                       "AllocateDescriptorSets");
    if (result.IsError()) {
        // On an error we can destroy the pool immediately because no command references it.
        mDevice->fn.DestroyDescriptorPool(mDevice->GetVkDevice(), descriptorPool, nullptr);
        DAWN_TRY(std::move(result));
    }

    std::vector<SetIndex> freeSetIndices;
    freeSetIndices.reserve(mMaxSets);

    for (SetIndex i = 0; i < mMaxSets; ++i) {
        freeSetIndices.push_back(i);
    }

    mAvailableDescriptorPoolIndices.push_back(mDescriptorPools.size());
    mDescriptorPools.emplace_back(
        DescriptorPool{descriptorPool, std::move(sets), std::move(freeSetIndices)});

    return {};
}

}  // namespace dawn::native::vulkan
