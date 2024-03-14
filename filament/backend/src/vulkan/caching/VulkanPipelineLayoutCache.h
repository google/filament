/*
 * Copyright (C) 2024 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILAMENT_BACKEND_VULKANPIPELINELAYOUTCACHE_H
#define TNT_FILAMENT_BACKEND_VULKANPIPELINELAYOUTCACHE_H

#include <vulkan/VulkanHandles.h>
#include <bluevk/BlueVK.h>

#include <utils/Hash.h>

#include <tsl/robin_map.h>

namespace filament::backend {

class VulkanPipelineLayoutCache {
public:
    VulkanPipelineLayoutCache(VkDevice device, VulkanResourceAllocator* allocator)
        : mDevice(device),
          mAllocator(allocator),
          mTimestamp(0) {}

    using PipelineLayoutKey = std::array<VkDescriptorSetLayout,
            VulkanDescriptorSetLayout::UNIQUE_DESCRIPTOR_SET_COUNT>;

    VulkanPipelineLayoutCache(VulkanPipelineLayoutCache const&) = delete;
    VulkanPipelineLayoutCache& operator=(VulkanPipelineLayoutCache const&) = delete;

    VkPipelineLayout getLayout(VulkanDescriptorSetLayoutList const& descriptorSetLayouts);

private:
    using Timestamp = uint64_t;
    struct PipelineLayoutCacheEntry {
        VkPipelineLayout handle;
        Timestamp lastUsed;
    };

    using PipelineLayoutKeyHashFn = utils::hash::MurmurHashFn<PipelineLayoutKey>;
    struct PipelineLayoutKeyEqual {
        bool operator()(PipelineLayoutKey const& k1, PipelineLayoutKey const& k2) const {
            return 0 == memcmp((const void*) &k1, (const void*) &k2, sizeof(PipelineLayoutKey));
        }
    };

    using PipelineLayoutMap = tsl::robin_map<PipelineLayoutKey, PipelineLayoutCacheEntry,
            PipelineLayoutKeyHashFn, PipelineLayoutKeyEqual>;

    VkDevice mDevice;
    VulkanResourceAllocator* mAllocator;
    Timestamp mTimestamp;
    PipelineLayoutMap mPipelineLayouts;
};

}

#endif // TNT_FILAMENT_BACKEND_VULKANPIPELINECACHE_H
