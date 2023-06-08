/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_VULKANBUFFER_H
#define TNT_FILAMENT_BACKEND_VULKANBUFFER_H

#include "VulkanContext.h"
#include "VulkanStagePool.h"

namespace filament::backend {

// Encapsulates a Vulkan buffer, its attached DeviceMemory and a staging area.
class VulkanBuffer {
public:
    VulkanBuffer(VmaAllocator allocator, VulkanCommands* commands, VulkanStagePool& stagePool,
            VkBufferUsageFlags usage, uint32_t numBytes);
    ~VulkanBuffer();
    void terminate();
    void loadFromCpu(const void* cpuData, uint32_t byteOffset, uint32_t numBytes) const;
    VkBuffer getGpuBuffer() const { return mGpuBuffer; }
private:
    VmaAllocator mAllocator;
    VulkanCommands* mCommands;
    VulkanStagePool& mStagePool;

    VmaAllocation mGpuMemory = VK_NULL_HANDLE;
    VkBuffer mGpuBuffer = VK_NULL_HANDLE;
    VkBufferUsageFlags mUsage = {};
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANBUFFER_H
