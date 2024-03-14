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

#include "VulkanPipelineLayoutCache.h"

#include <vulkan/VulkanResourceAllocator.h>

namespace filament::backend {

VkPipelineLayout VulkanPipelineLayoutCache::getLayout(
        VulkanDescriptorSetLayoutList const& descriptorSetLayouts) {
    PipelineLayoutKey key = {VK_NULL_HANDLE};
    uint8_t descSetLayoutCount = 0;
    for (auto layoutHandle : descriptorSetLayouts) {
        if (layoutHandle) {
            auto layout = mAllocator->handle_cast<VulkanDescriptorSetLayout*>(layoutHandle);
            key[descSetLayoutCount++] = layout->vklayout;
        }
    }

    auto iter = mPipelineLayouts.find(key);
    if (iter != mPipelineLayouts.end()) {
        PipelineLayoutCacheEntry& entry = mPipelineLayouts[key];
        entry.lastUsed = mTimestamp++;
        return entry.handle;
    }

    VkPipelineLayoutCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .setLayoutCount = (uint32_t) descSetLayoutCount,
        .pSetLayouts = key.data(),
        .pushConstantRangeCount = 0,
    };
    VkPipelineLayout layout;
    vkCreatePipelineLayout(mDevice, &info, VKALLOC, &layout);

    mPipelineLayouts[key] = {
        .handle = layout,
        .lastUsed = mTimestamp++,
    };
    return layout;
}

}// namespace filament::backend
