/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "VulkanDescriptorSetLayoutCache.h"

#include "VulkanHandles.h"

namespace filament::backend {

namespace {

using BitmaskGroup = VulkanDescriptorSetLayout::Bitmask;

template<typename Bitmask>
uint32_t appendBindings(VkDescriptorSetLayoutBinding* toBind, VkDescriptorType type,
        Bitmask const& mask) {
    uint32_t count = 0;
    Bitmask alreadySeen;
    mask.forEachSetBit([&](size_t index) {
        VkShaderStageFlags stages = 0;
        uint32_t binding = 0;
        if (index < fvkutils::getFragmentStageShift<Bitmask>()) {
            binding = (uint32_t) index;
            stages |= VK_SHADER_STAGE_VERTEX_BIT;
            auto fragIndex = index + fvkutils::getFragmentStageShift<Bitmask>();
            if (mask.test(fragIndex)) {
                stages |= VK_SHADER_STAGE_FRAGMENT_BIT;
                alreadySeen.set(fragIndex);
            }
        } else if (!alreadySeen.test(index)) {
            // We are in fragment stage bits
            binding = (uint32_t) (index - fvkutils::getFragmentStageShift<Bitmask>());
            stages |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        if (stages) {
            toBind[count++] = {
                .binding = binding,
                .descriptorType = type,
                .descriptorCount = 1,
                .stageFlags = stages,
            };
        }
    });
    return count;
}

} // anonymous namespace

VulkanDescriptorSetLayoutCache::VulkanDescriptorSetLayoutCache(VkDevice device,
        fvkmemory::ResourceManager* resourceManager)
    : mDevice(device),
      mResourceManager(resourceManager) {}

VulkanDescriptorSetLayoutCache::~VulkanDescriptorSetLayoutCache() = default;

void VulkanDescriptorSetLayoutCache::terminate() noexcept {
    for (auto& itr: mVkLayouts) {
        vkDestroyDescriptorSetLayout(mDevice, itr.second, VKALLOC);
    }
}

fvkmemory::resource_ptr<VulkanDescriptorSetLayout> VulkanDescriptorSetLayoutCache::createLayout(
        Handle<HwDescriptorSetLayout> handle, backend::DescriptorSetLayout&& info) {
    auto layout = fvkmemory::resource_ptr<VulkanDescriptorSetLayout>::make(mResourceManager, handle,
            info);
    VkDescriptorSetLayout vklayout = VK_NULL_HANDLE;
    auto const& bitmasks = layout->bitmask;
    if (auto itr = mVkLayouts.find(bitmasks); itr != mVkLayouts.end()) {
        vklayout = itr->second;
    } else {
        VkDescriptorSetLayoutBinding toBind[VulkanDescriptorSetLayout::MAX_BINDINGS];
        uint32_t count = 0;
        count += appendBindings(&toBind[count], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                bitmasks.dynamicUbo);
        count += appendBindings(&toBind[count], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, bitmasks.ubo);
        count += appendBindings(&toBind[count], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                bitmasks.sampler);
        count += appendBindings(&toBind[count], VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                bitmasks.inputAttachment);

        assert_invariant(count != 0 && "Need at least one binding for descriptor set layout.");
        VkDescriptorSetLayoutCreateInfo dlinfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .bindingCount = count,
            .pBindings = toBind,
        };

        vkCreateDescriptorSetLayout(mDevice, &dlinfo, VKALLOC, &vklayout);
        mVkLayouts[bitmasks] = vklayout;
    }
    layout->setVkLayout(vklayout);
    return layout;
}

} // namespace filament::backend
