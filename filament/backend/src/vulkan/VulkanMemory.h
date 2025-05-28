/*
 * Copyright (C) 2021 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_VULKANMEMORY_H
#define TNT_FILAMENT_BACKEND_VULKANMEMORY_H

#include <bluevk/BlueVK.h> // must be included before vk_mem_alloc

#ifndef VMA_STATIC_VULKAN_FUNCTIONS
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#endif

#ifndef VMA_DYNAMIC_VULKAN_FUNCTIONS
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#endif

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wc++98-compat-extra-semi"
#endif

#include "vk_mem_alloc.h"
#include "vulkan/memory/Resource.h"

#include <functional>

VK_DEFINE_HANDLE(VmaAllocator)
VK_DEFINE_HANDLE(VmaAllocation)
VK_DEFINE_HANDLE(VmaPool)

namespace filament::backend {

enum class VulkanBufferUsage : uint8_t {
    UNKNOWN,
    VERTEX,
    INDEX,
    UNIFORM,
    SHADER_STORAGE,
};

struct VulkanGpuBuffer {
    VkBuffer vkbuffer = VK_NULL_HANDLE;
    VmaAllocation vmaAllocation = VK_NULL_HANDLE;
    VmaAllocationInfo allocationInfo;
    uint32_t numBytes = 0;
    VulkanBufferUsage usage = VulkanBufferUsage::UNKNOWN;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANMEMORY_H
