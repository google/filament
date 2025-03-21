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

#include <bluevk/BlueVK.h>
#include <tsl/robin_map.h>

#include <memory>

namespace filament::backend {

class VulkanDescriptorSetLayoutCache {
public:
    VulkanDescriptorSetLayoutCache(VkDevice device, fvkmemory::ResourceManager* resourceManager);
    ~VulkanDescriptorSetLayoutCache();

    void terminate() noexcept;

    fvkmemory::resource_ptr<VulkanDescriptorSetLayout> createLayout(
            Handle<HwDescriptorSetLayout> handle, backend::DescriptorSetLayout&& info);

private:
    VkDevice mDevice;
    fvkmemory::ResourceManager* mResourceManager;

    using BitmaskGroup = VulkanDescriptorSetLayout::Bitmask;
    using BitmaskGroupHashFn = utils::hash::MurmurHashFn<BitmaskGroup>;
    struct BitmaskGroupEqual {
        bool operator()(BitmaskGroup const& k1, BitmaskGroup const& k2) const { return k1 == k2; }
    };

    tsl::robin_map<BitmaskGroup, VkDescriptorSetLayout, BitmaskGroupHashFn, BitmaskGroupEqual>
            mVkLayouts;
};

} // namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_CACHING_VULKANDESCRIPTORSETLAYOUTCACHE_H
