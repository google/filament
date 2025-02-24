/* Copyright (c) 2018-2024 The Khronos Group Inc.
 * Copyright (c) 2018-2024 Valve Corporation
 * Copyright (c) 2018-2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gpuav/resources/gpuav_vulkan_objects.h"

#include "gpuav/core/gpuav.h"
#include "generated/dispatch_functions.h"
#include <vulkan/utility/vk_struct_helper.hpp>

namespace gpuav {
namespace vko {

// Implementation for Descriptor Set Manager class
DescriptorSetManager::DescriptorSetManager(VkDevice device, uint32_t num_bindings_in_set)
    : device(device), num_bindings_in_set(num_bindings_in_set) {}

DescriptorSetManager::~DescriptorSetManager() {
    for (auto &pool : desc_pool_map_) {
        DispatchDestroyDescriptorPool(device, pool.first, nullptr);
    }
    desc_pool_map_.clear();
}

VkResult DescriptorSetManager::GetDescriptorSet(VkDescriptorPool *out_desc_pool, VkDescriptorSetLayout ds_layout,
                                                VkDescriptorSet *out_desc_sets) {
    std::vector<VkDescriptorSet> desc_sets;
    VkResult result = GetDescriptorSets(1, out_desc_pool, ds_layout, &desc_sets);
    assert(result == VK_SUCCESS);
    if (result == VK_SUCCESS) {
        *out_desc_sets = desc_sets[0];
    }
    return result;
}

VkResult DescriptorSetManager::GetDescriptorSets(uint32_t count, VkDescriptorPool *out_pool, VkDescriptorSetLayout ds_layout,
                                                 std::vector<VkDescriptorSet> *out_desc_sets) {
    auto guard = Lock();

    VkResult result = VK_SUCCESS;
    VkDescriptorPool desc_pool_to_use = VK_NULL_HANDLE;

    assert(count > 0);
    if (count == 0) {
        return result;
    }
    out_desc_sets->clear();
    out_desc_sets->resize(count);

    for (auto &[desc_pool, pool_tracker] : desc_pool_map_) {
        if (pool_tracker.used + count < pool_tracker.size) {
            desc_pool_to_use = desc_pool;
            break;
        }
    }
    if (desc_pool_to_use == VK_NULL_HANDLE) {
        constexpr uint32_t kDefaultMaxSetsPerPool = 512;
        const uint32_t max_sets = std::max(kDefaultMaxSetsPerPool, count);

        // TODO: The logic to compute descriptor pool sizes should not be
        // hardcoded like so, should be dynamic depending on the descriptor sets
        // to be created. Not too dramatic as Vulkan will gracefully fail if there is a
        // mismatch between this and created descriptor sets.
        const std::array<VkDescriptorPoolSize, 2> pool_sizes = {{{
                                                                     VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                                                     max_sets * num_bindings_in_set,
                                                                 },
                                                                 {
                                                                     VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
                                                                     max_sets * num_bindings_in_set,
                                                                 }}};

        VkDescriptorPoolCreateInfo desc_pool_info = vku::InitStructHelper();
        desc_pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        desc_pool_info.maxSets = max_sets;
        desc_pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
        desc_pool_info.pPoolSizes = pool_sizes.data();
        result = DispatchCreateDescriptorPool(device, &desc_pool_info, nullptr, &desc_pool_to_use);
        assert(result == VK_SUCCESS);
        if (result != VK_SUCCESS) {
            return result;
        }
        desc_pool_map_[desc_pool_to_use].size = desc_pool_info.maxSets;
        desc_pool_map_[desc_pool_to_use].used = 0;
    }

    std::vector<VkDescriptorSetLayout> desc_layouts(count, ds_layout);
    VkDescriptorSetAllocateInfo desc_set_alloc_info = vku::InitStructHelper();
    desc_set_alloc_info.descriptorPool = desc_pool_to_use;
    desc_set_alloc_info.descriptorSetCount = count;
    desc_set_alloc_info.pSetLayouts = desc_layouts.data();
    result = DispatchAllocateDescriptorSets(device, &desc_set_alloc_info, out_desc_sets->data());
    assert(result == VK_SUCCESS);
    if (result != VK_SUCCESS) {
        return result;
    }

    *out_pool = desc_pool_to_use;
    desc_pool_map_[desc_pool_to_use].used += count;

    return result;
}

void DescriptorSetManager::PutBackDescriptorSet(VkDescriptorPool desc_pool, VkDescriptorSet desc_set) {
    auto guard = Lock();

    auto iter = desc_pool_map_.find(desc_pool);
    assert(iter != desc_pool_map_.end());
    if (iter == desc_pool_map_.end()) {
        return;
    }

    VkResult result = DispatchFreeDescriptorSets(device, desc_pool, 1, &desc_set);
    assert(result == VK_SUCCESS);
    if (result != VK_SUCCESS) {
        return;
    }
    desc_pool_map_[desc_pool].used--;
    if (desc_pool_map_[desc_pool].used == 0) {
        DispatchDestroyDescriptorPool(device, desc_pool, nullptr);
        desc_pool_map_.erase(desc_pool);
    }

    return;
}

void SharedResourcesCache::Clear() {
    for (auto &[key, value] : shared_validation_resources_map_) {
        auto &[object, destructor] = value;
        destructor(object);
    }
    shared_validation_resources_map_.clear();
}

void *Buffer::MapMemory(const Location &loc) const {
    void *buffer_ptr = nullptr;
    VkResult result = vmaMapMemory(gpuav.vma_allocator_, allocation, &buffer_ptr);
    if (result != VK_SUCCESS) {
        gpuav.InternalVmaError(gpuav.device, loc, "Unable to map device memory.");
        return nullptr;
    }
    return buffer_ptr;
}

void Buffer::UnmapMemory() const { vmaUnmapMemory(gpuav.vma_allocator_, allocation); }

void Buffer::FlushAllocation(const Location &loc, VkDeviceSize offset, VkDeviceSize size) const {
    VkResult result = vmaFlushAllocation(gpuav.vma_allocator_, allocation, offset, size);
    if (result != VK_SUCCESS) {
        gpuav.InternalVmaError(gpuav.device, loc, "Unable to flush device memory.");
    }
}

void Buffer::InvalidateAllocation(const Location &loc, VkDeviceSize offset, VkDeviceSize size) const {
    VkResult result = vmaInvalidateAllocation(gpuav.vma_allocator_, allocation, offset, size);
    if (result != VK_SUCCESS) {
        gpuav.InternalVmaError(gpuav.device, loc, "Unable to invalidate device memory.");
    }
}

bool Buffer::Create(const Location &loc, const VkBufferCreateInfo *buffer_create_info,
                    const VmaAllocationCreateInfo *allocation_create_info) {
    VkResult result =
        vmaCreateBuffer(gpuav.vma_allocator_, buffer_create_info, allocation_create_info, &buffer, &allocation, nullptr);
    if (result != VK_SUCCESS) {
        gpuav.InternalVmaError(gpuav.device, loc, "Unable to allocate device memory for internal buffer.");
        return false;
    }

    if (buffer_create_info->usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        // After creating the buffer, get the address right away
        device_address = gpuav.GetBufferDeviceAddressHelper(buffer);
        if (device_address == 0) {
            gpuav.InternalError(gpuav.device, loc, "Failed to get address with DispatchGetBufferDeviceAddress.");
            return false;
        }
    }
    return true;
}

void Buffer::Destroy() {
    if (buffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(gpuav.vma_allocator_, buffer, allocation);
        buffer = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
        device_address = 0;
    }
}

VkDescriptorSet GpuResourcesManager::GetManagedDescriptorSet(VkDescriptorSetLayout desc_set_layout) {
    std::pair<VkDescriptorPool, VkDescriptorSet> descriptor;
    descriptor_set_manager_.GetDescriptorSet(&descriptor.first, desc_set_layout, &descriptor.second);
    descriptors_.emplace_back(descriptor);
    return descriptor.second;
}

vko::Buffer GpuResourcesManager::GetManagedBuffer(Validator &gpuav, const Location &loc, const VkBufferCreateInfo &ci,
                                                  const VmaAllocationCreateInfo &vma_ci) {
    vko::Buffer buffer(gpuav);
    const bool success = buffer.Create(loc, &ci, &vma_ci);
    if (success) {
        buffers_.emplace_back(buffer);
    }

    return buffer;
}

void GpuResourcesManager::DestroyResources() {
    for (auto &[desc_pool, desc_set] : descriptors_) {
        descriptor_set_manager_.PutBackDescriptorSet(desc_pool, desc_set);
    }
    descriptors_.clear();

    for (auto &buffer : buffers_) {
        buffer.Destroy();
    }
    buffers_.clear();
}
}  // namespace vko
}  // namespace gpuav
