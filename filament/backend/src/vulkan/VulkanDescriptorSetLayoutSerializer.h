/*
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_VULKANDESCRIPTORSETLAYOUTSERIALIZER_H
#define TNT_FILAMENT_BACKEND_VULKANDESCRIPTORSETLAYOUTSERIALIZER_H

#include "VulkanPipelineLayoutCache.h"
#include "VulkanYcbcrConversionCache.h"
#include <backend/DriverEnums.h>

#include <bluevk/BlueVK.h>
#include <iostream>
#include <sstream>
#include <vector>

namespace filament::backend {

class VulkanDescriptorSetLayoutSerializer {
public:
    VulkanDescriptorSetLayoutSerializer(VulkanDescriptorSetLayoutSerializer const&) = delete;
    VulkanDescriptorSetLayoutSerializer& operator=(
            VulkanDescriptorSetLayoutSerializer const&) = delete;

    VulkanDescriptorSetLayoutSerializer(const VkDescriptorSetLayoutCreateInfo& info,
            utils::FixedCapacityVector<uint32_t> immutableSamplers,
            uint32_t hash);
};

class VulkanPipelineLayoutSerializer {
public:
    VulkanPipelineLayoutSerializer(VulkanPipelineLayoutSerializer const&) = delete;
    VulkanPipelineLayoutSerializer& operator=(VulkanPipelineLayoutSerializer const&) = delete;

    VulkanPipelineLayoutSerializer(const VkPipelineLayoutCreateInfo& layout,
            VulkanDescriptorSetLayoutCache* cache, uint32_t hash);
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANDESCRIPTORSETLAYOUTSERIALIZER_H
