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

namespace filament::backend {

VkPipelineLayout VulkanPipelineLayoutCache::getLayout(
        DescriptorSetLayoutArray const& descriptorSetLayouts,
        fvkmemory::resource_ptr<VulkanProgram> program) {
    PipelineLayoutKey key = {};
    uint8_t descSetLayoutCount = 0;
    key.descSetLayouts = descriptorSetLayouts;
    for (auto layoutHandle: descriptorSetLayouts) {
        if (layoutHandle == VK_NULL_HANDLE) {
            break;
        }
        descSetLayoutCount++;
    }

    // build the push constant layout key
    uint32_t const pushConstantRangeCount = program->getPushConstantRangeCount();
    auto const& pushConstantRanges = program->getPushConstantRanges();
    if (pushConstantRangeCount > 0) {
        assert_invariant(pushConstantRangeCount <= Program::SHADER_TYPE_COUNT);
        for (uint8_t i = 0; i < pushConstantRangeCount; ++i) {
            auto const& range = pushConstantRanges[i];
            auto& pushConstant = key.pushConstant[i];
            if (range.stageFlags & VK_SHADER_STAGE_VERTEX_BIT) {
                pushConstant.stage = static_cast<uint8_t>(ShaderStage::VERTEX);
            }
            if (range.stageFlags & VK_SHADER_STAGE_FRAGMENT_BIT) {
                pushConstant.stage = static_cast<uint8_t>(ShaderStage::FRAGMENT);
            }
            if (range.stageFlags & VK_SHADER_STAGE_COMPUTE_BIT) {
                pushConstant.stage = static_cast<uint8_t>(ShaderStage::COMPUTE);
            }
            pushConstant.size = range.size;
        }
    }

    if (auto iter = mPipelineLayouts.find(key); iter != mPipelineLayouts.end()) {
        PipelineLayoutCacheEntry& entry = iter->second;
        entry.lastUsed = mTimestamp++;
        return entry.handle;
    }

    VkPipelineLayoutCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .setLayoutCount = (uint32_t) descSetLayoutCount,
        .pSetLayouts = key.descSetLayouts.data(),
        .pushConstantRangeCount = pushConstantRangeCount,
        .pPushConstantRanges = pushConstantRanges,
    };

    VkPipelineLayout layout;
    vkCreatePipelineLayout(mDevice, &info, VKALLOC, &layout);

    mPipelineLayouts[key] = {
        .handle = layout,
        .lastUsed = mTimestamp++,
    };
    return layout;
}

void VulkanPipelineLayoutCache::terminate() noexcept {
    for (auto const& [key, entry]: mPipelineLayouts) {
        vkDestroyPipelineLayout(mDevice, entry.handle, VKALLOC);
    }
}

}// namespace filament::backend
