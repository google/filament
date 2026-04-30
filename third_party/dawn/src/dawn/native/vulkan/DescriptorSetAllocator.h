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

#ifndef SRC_DAWN_NATIVE_VULKAN_DESCRIPTORSETALLOCATOR_H_
#define SRC_DAWN_NATIVE_VULKAN_DESCRIPTORSETALLOCATOR_H_

#include <memory>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "dawn/common/Mutex.h"
#include "dawn/common/RefCounted.h"
#include "dawn/common/SerialQueue.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/Error.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/vulkan/DescriptorSetAllocation.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native::vulkan {

class Device;

// Vulkan requires that descriptor sets are sub-allocated from pre-created pools of descriptors.
// Creating one pool per descriptor set is inefficient as each pool might be a different GPU
// allocation (causing syscalls etc). This class handles allocating larger pools and reusing
// sub-allocations when descriptor sets are no longer in use.
//
// Pools are allocated to be a multiple of the size required by a BindGroupLayout (multidimensional,
// one per-type of descriptor) and all the descriptor sets for a layout created immediately. This is
// necessary because the Vulkan spec doesn't require drivers to implement anything for
// VkDescriptorPool but a simple bump allocator (with the ability to roll back one allocation).
// Instead we allocate all the descriptors sets and overwrite over time.
// TODO(https://crbug.com/439522242): We could reuse allocators between BindGroupLayouts with the
// same amount of descriptors if we added logic to be resilient to vkAllocateDescriptorSet failures
// and some form of GCing / repointing of the dawn::native::vulkan::BindGroupLayout.
//
// It is RefCounted because when descriptor set is destroyed, the allocator is added to a
// notification queue on the device to get called back when the queue serial is completed. As such
// it has multiple owners: the BindGroupLayout it corresponds to, and sometimes the device as well.
class DescriptorSetAllocator : public RefCounted {
    using PoolIndex = uint32_t;
    using SetIndex = uint16_t;

  public:
    static Ref<DescriptorSetAllocator> Create(
        Device* device,
        absl::flat_hash_map<VkDescriptorType, uint32_t> descriptorCountPerType);

    ResultOrError<DescriptorSetAllocation> Allocate(VkDescriptorSetLayout dsLayout);
    void Deallocate(DescriptorSetAllocation* allocationInfo);
    void FinishDeallocation(ExecutionSerial completedSerial);

  private:
    DescriptorSetAllocator(Device* device,
                           absl::flat_hash_map<VkDescriptorType, uint32_t> descriptorCountPerType);
    ~DescriptorSetAllocator() override;

    MaybeError AllocateDescriptorPool(VkDescriptorSetLayout dsLayout);

    std::vector<VkDescriptorPoolSize> mPoolSizes;
    SetIndex mMaxSets;

    struct DescriptorPool {
        VkDescriptorPool vkPool;
        std::vector<VkDescriptorSet> sets;
        std::vector<SetIndex> freeSetIndices;
    };

    std::vector<PoolIndex> mAvailableDescriptorPoolIndices;
    std::vector<DescriptorPool> mDescriptorPools;

    struct Deallocation {
        PoolIndex poolIndex;
        SetIndex setIndex;
    };
    SerialQueue<ExecutionSerial, Deallocation> mPendingDeallocations;
    ExecutionSerial mLastDeallocationSerial = ExecutionSerial(0);

    // Used to guard all public member functions.
    Mutex mMutex;

    const raw_ptr<Device> mDevice;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_DESCRIPTORSETALLOCATOR_H_
