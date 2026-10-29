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

#ifndef TNT_FILAMENT_BACKEND_VULKANBUFFERPROXY_H
#define TNT_FILAMENT_BACKEND_VULKANBUFFERPROXY_H

#include "VulkanBufferCache.h"
#include "VulkanCommands.h"
#include "VulkanContext.h"
#include "VulkanMemory.h"
#include "VulkanStagePool.h"

#include "vulkan/memory/Resource.h"
#include "vulkan/utils/CommandBufferAgeCounter.h"

#include <backend/DriverEnums.h>

namespace filament::backend {

struct VulkanDescriptorSet;
struct VulkanCommandBuffer;

// This class acts as a dynamic wrapper for a `VulkanBuffer`. It allows you to modify the
// `VulkanBuffer` it references at runtime, wihtout affecting any external objects.
class VulkanBufferProxy {
public:
    VulkanBufferProxy(VulkanContext const& context, VmaAllocator allocator,
            VulkanStagePool& stagePool, VulkanBufferCache& bufferCache, VulkanBufferBinding binding,
            BufferUsage usage, uint32_t numBytes);

    // Uploads `numBytes` of `cpuData` at `byteOffset`. This either writes straight into the
    // buffer's memory, or records a staging copy into the recording command buffer of `commands`.
    // A command buffer is only started (via VulkanCommands::get()) for the staging copy. Returns
    // the command buffer that the copy was recorded into, or nullptr if no GPU work was needed.
    VulkanCommandBuffer* loadFromCpu(VulkanCommands& commands, const void* cpuData,
            uint32_t byteOffset, uint32_t numBytes);

    VkBuffer getVkBuffer() const noexcept;

    // Records that `commands` reads from this buffer: the command buffer holds the backing
    // VulkanBuffer until it completes, and loadFromCpu() will not memcpy into it (and will issue a
    // barrier) until then. Calling this repeatedly for the same command buffer is cheap, since the
    // buffer is only acquired once per command buffer.
    void referencedBy(VulkanCommandBuffer& commands,
            fvkutils::CommandBufferAgeCounter const& ageCounter);

    // Incremented every time the backing VkBuffer is swapped out. Descriptor sets record the
    // generation when they write this buffer into a descriptor, and compare it later to detect a
    // stale VkBuffer.
    uint64_t getGeneration() const noexcept { return mGeneration; }

    // Declares the current contents dead (see DriverAPI's resetBufferObject()). If the GPU may
    // still be reading the buffer, and it can be orphaned, this swaps in a fresh one, so that the
    // next update can write it directly even if it does not cover the whole buffer.
    void reset(fvkutils::CommandBufferAgeCounter const& ageCounter);

private:
    VulkanBufferBinding getBinding() const noexcept;

    // Returns true if mBuffer may be swapped out for a fresh VkBuffer (see orphan()).
    bool canOrphan() const noexcept;

    // Swaps out mBuffer for a fresh VkBuffer. Command buffers that use the old one keep it alive
    // until they complete, after which it is recycled.
    void orphan();

    bool const mStagingBufferBypassEnabled;
    VmaAllocator mAllocator;
    VulkanStagePool& mStagePool;
    VulkanBufferCache& mBufferCache;

    fvkmemory::resource_ptr<VulkanBuffer> mBuffer;

    BufferUsage const mUsage;
    VulkanBufferBinding const mBinding;
    uint32_t const mNumBytes;
    uint64_t mGeneration = 0;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANBUFFERPROXY_H
