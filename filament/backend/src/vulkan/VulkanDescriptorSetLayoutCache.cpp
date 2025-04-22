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

#include <utils/Hash.h>

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

uint32_t appendSamplerBindings(VkDescriptorSetLayoutBinding* toBind,
        fvkutils::SamplerBitmask const& mask, fvkutils::SamplerBitmask const& external,
        utils::FixedCapacityVector<VkSampler> const& immutableSamplers) {
    using Bitmask = fvkutils::SamplerBitmask;
    uint32_t count = 0;
    Bitmask alreadySeen;
    uint8_t immutableIndex = 0;
    size_t const immutableSamplerCount = immutableSamplers.size();
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
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = 1,
                .stageFlags = stages,
                .pImmutableSamplers = external[index] && immutableSamplerCount > immutableIndex
                                              ? &immutableSamplers[immutableIndex++]
                                              : nullptr,
            };
        }
    });
    return count;
}

uint64_t computeImmutableSamplerHash(utils::FixedCapacityVector<VkSampler> const& samplers) {
    size_t const size = samplers.size();
    if (size == 0) {
        return 0;
    } else if (size == 1) {
        return (uint64_t) samplers[0];
    }
    return utils::hash::murmur3((uint32_t*) samplers.data(), samplers.size() * 2, 0);
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

VkDescriptorSetLayout VulkanDescriptorSetLayoutCache::getVkLayout(
        VulkanDescriptorSetLayout::Bitmask const& bitmasks,
        fvkutils::SamplerBitmask externalSamplers,
        utils::FixedCapacityVector<VkSampler> immutableSamplers) {
    LayoutKey key = {
        .bitmask = bitmasks,
        .immutableSamplerHash = computeImmutableSamplerHash(immutableSamplers),
    };
    if (auto itr = mVkLayouts.find(key); itr != mVkLayouts.end()) {
        return itr->second;
    }

    VkDescriptorSetLayoutBinding toBind[VulkanDescriptorSetLayout::MAX_BINDINGS];
    uint32_t count = 0;
    count += appendBindings(&toBind[count], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            bitmasks.dynamicUbo);
    count += appendBindings(&toBind[count], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, bitmasks.ubo);
    count += appendSamplerBindings(&toBind[count], bitmasks.sampler, externalSamplers,
            immutableSamplers);
    count += appendBindings(&toBind[count], VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            bitmasks.inputAttachment);

    assert_invariant(count != 0 && "Need at least one binding for descriptor set layout.");
    VkDescriptorSetLayoutCreateInfo dlinfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = count,
        .pBindings = toBind,
    };
    VkDescriptorSetLayout vklayout;
    vkCreateDescriptorSetLayout(mDevice, &dlinfo, VKALLOC, &vklayout);
    mVkLayouts[key] = vklayout;
    return vklayout;
}

fvkmemory::resource_ptr<VulkanDescriptorSetLayout> VulkanDescriptorSetLayoutCache::createLayout(
        Handle<HwDescriptorSetLayout> handle, backend::DescriptorSetLayout&& info) {
    BitmaskGroup maskGroup = VulkanDescriptorSetLayout::Bitmask::fromLayoutDescription(info);
    auto layout = fvkmemory::resource_ptr<VulkanDescriptorSetLayout>::make(mResourceManager, handle,
            std::move(info), getVkLayout(maskGroup, maskGroup.externalSampler));
    return layout;
}

} // namespace filament::backend
