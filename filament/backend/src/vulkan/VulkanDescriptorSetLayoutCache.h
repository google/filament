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

#ifndef TNT_FILAMENT_BACKEND_CACHING_VULKANDESCRIPTORSETLAYOUTCACHE_H
#define TNT_FILAMENT_BACKEND_CACHING_VULKANDESCRIPTORSETLAYOUTCACHE_H

#include "VulkanHandles.h"

#include "vulkan/memory/ResourcePointer.h"

#include <backend/DriverEnums.h>
#include <backend/Program.h>
#include <backend/TargetBufferInfo.h>

#include <utils/bitset.h>
#include <utils/FixedCapacityVector.h>

#include <bluevk/BlueVK.h>
#include <tsl/robin_map.h>

namespace filament::backend {

class VulkanDescriptorSetLayoutCache {
public:
    VulkanDescriptorSetLayoutCache(VkDevice device, fvkmemory::ResourceManager* resourceManager);
    ~VulkanDescriptorSetLayoutCache();

    void terminate() noexcept;

    // Just a wrapper around getVkLayout()
    fvkmemory::resource_ptr<VulkanDescriptorSetLayout> createLayout(
            Handle<HwDescriptorSetLayout> handle, backend::DescriptorSetLayout&& info);

    // This method is meant to be used with external samplers
    VkDescriptorSetLayout getVkLayout(VulkanDescriptorSetLayout::Bitmask const& bitmasks,
            fvkutils::SamplerBitmask externalSamplers,
            utils::FixedCapacityVector<VkSampler> immutableSamplers = {});

private:
    VkDevice mDevice;
    fvkmemory::ResourceManager* mResourceManager;

    struct LayoutKey {
        // this describes the layout using bitset.
        VulkanDescriptorSetLayout::Bitmask bitmask = {};
        // number of immutable samplers can be arbitrary; so we hash them into 64-bit.
        uint64_t immutableSamplerHash = 0;
    };
    static_assert(sizeof(LayoutKey) == 48);

    using LayoutKeyHashFn = utils::hash::MurmurHashFn<LayoutKey>;
    struct LayoutKeyEqual {
        bool operator()(LayoutKey const& k1, LayoutKey const& k2) const {
            return k1.bitmask == k2.bitmask && k1.immutableSamplerHash == k2.immutableSamplerHash;
        }
    };

    tsl::robin_map<LayoutKey, VkDescriptorSetLayout, LayoutKeyHashFn, LayoutKeyEqual> mVkLayouts;
};

} // namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_CACHING_VULKANDESCRIPTORSETLAYOUTCACHE_H
