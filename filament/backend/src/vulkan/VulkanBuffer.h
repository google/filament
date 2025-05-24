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
#include "VulkanGpuBufferCache.h"
#include "VulkanMemory.h"
#include "VulkanStagePool.h"

namespace filament::backend {

// Encapsulates a Vulkan buffer, its attached DeviceMemory and a staging area.
class VulkanBuffer {
public:
    VulkanBuffer(VmaAllocator allocator, VulkanStagePool& stagePool,
            VulkanGpuBufferCache& gpuBufferCache, VulkanBufferUsage usage, uint32_t numBytes);

    void loadFromCpu(VkCommandBuffer cmdbuf, const void* cpuData, uint32_t byteOffset,
            uint32_t numBytes);

    VkBuffer getVkBuffer() const noexcept;

    VulkanBufferUsage getUsage() const noexcept;

private:
    VmaAllocator mAllocator;
    VulkanStagePool& mStagePool;
    VulkanGpuBufferCache& mGpuBufferCache;

    fvkmemory::resource_ptr<VulkanGpuBufferHolder> mGpuBufferHolder;
    uint32_t mUpdatedOffset = 0;
    uint32_t mUpdatedBytes = 0;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANBUFFER_H
