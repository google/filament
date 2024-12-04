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

#ifndef TNT_FILAMENT_BACKEND_VULKANASYNCHANDLES_H
#define TNT_FILAMENT_BACKEND_VULKANASYNCHANDLES_H

#include <bluevk/BlueVK.h>

#include "DriverBase.h"

#include "vulkan/memory/Resource.h"

#include <utils/Mutex.h>
#include <utils/Condition.h>

namespace filament::backend {

// Wrapper to enable use of shared_ptr for implementing shared ownership of low-level Vulkan fences.
struct VulkanCmdFence {
    VulkanCmdFence(VkResult initialStatus) {
        // Internally we use the VK_INCOMPLETE status to mean "not yet submitted". When this fence
        // gets submitted, its status changes to VK_NOT_READY. Finally, when the GPU actually
        // finishes executing the command buffer, the status changes to VK_SUCCESS.
        status.store(initialStatus);
    }

    ~VulkanCmdFence() = default;

    void setStatus(VkResult value) { status.store(value); }

    VkResult getStatus() { return status.load(std::memory_order_acquire); }

private:
    std::atomic<VkResult> status;
};

struct VulkanFence : public HwFence, fvkmemory::ThreadSafeResource {
    VulkanFence() {}
    std::shared_ptr<VulkanCmdFence> fence;
};

struct VulkanTimerQuery : public HwTimerQuery, fvkmemory::ThreadSafeResource {
    explicit VulkanTimerQuery(std::tuple<uint32_t, uint32_t> indices);
    ~VulkanTimerQuery();

    void setFence(std::shared_ptr<VulkanCmdFence> fence) noexcept;

    bool isCompleted() noexcept;

    uint32_t getStartingQueryIndex() const {
        return mStartingQueryIndex;
    }

    uint32_t getStoppingQueryIndex() const {
        return mStoppingQueryIndex;
    }

private:
    uint32_t mStartingQueryIndex;
    uint32_t mStoppingQueryIndex;

    std::shared_ptr<VulkanCmdFence> mFence;
    utils::Mutex mFenceMutex;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANHASYNCANDLES_H
