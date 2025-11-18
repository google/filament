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

#include "VulkanHandles.h"
#include "VulkanDescriptorSetLayoutCache.h"

#include <bluevk/BlueVK.h>

#include <utils/Hash.h>

#include <unordered_map>

namespace filament::backend {

class VulkanPipelineLayoutCache {
public:
    using DescriptorSetLayoutArray = VulkanDescriptorSetLayout::DescriptorSetLayoutArray;
    using DescriptorSetLayoutHashArray = VulkanDescriptorSetLayout::DescriptorSetLayoutHashArray;
    
    VulkanPipelineLayoutCache(VkDevice device, VulkanDescriptorSetLayoutCache* cache)
        : mDevice(device),
          mTimestamp(0), mDescriptorSetCache(cache) {}

    void terminate() noexcept;

    struct PushConstantKey {
        uint8_t stage = 0;// We have one set of push constant per shader stage (fragment, vertex, etc).
        uint8_t size = 0;
        // Note that there is also an offset parameter for push constants, but
        // we always assume our update range will have the offset 0.
    };

    struct PipelineLayoutKey {
        DescriptorSetLayoutHashArray descSetLayouts = {};              // 8 * 4
        PushConstantKey pushConstant[Program::SHADER_TYPE_COUNT] = {};                    // 2 * 3
        uint16_t padding = 0;
    };
    static_assert(sizeof(PipelineLayoutKey) == 40);

    VulkanPipelineLayoutCache(VulkanPipelineLayoutCache const&) = delete;
    VulkanPipelineLayoutCache& operator=(VulkanPipelineLayoutCache const&) = delete;

    // A pipeline layout depends on the descriptor set layout and the push constant ranges, which
    // are described in the program.
    VkPipelineLayout getLayout(DescriptorSetLayoutArray const& descriptorSetLayouts,
            fvkmemory::resource_ptr<VulkanProgram> program);
    uint32_t getKey(VkPipelineLayout layout) {
        uint32_t key = 0;
        auto iter = mPipelineToKey.find(layout);
        if (iter != mPipelineToKey.end()) {
            key = iter->second;
        }
        return key;
    }

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

    using PipelineLayoutMap = std::unordered_map<PipelineLayoutKey, PipelineLayoutCacheEntry,
            PipelineLayoutKeyHashFn, PipelineLayoutKeyEqual>;

    VkDevice mDevice;
    Timestamp mTimestamp;
    PipelineLayoutMap mPipelineLayouts;
    std::map<VkPipelineLayout, uint32_t> mPipelineToKey;
    VulkanDescriptorSetLayoutCache* mDescriptorSetCache;
};

} // filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANPIPELINECACHE_H
