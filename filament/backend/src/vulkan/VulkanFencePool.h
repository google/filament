/*
 * Copyright (C) 2026 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_VULKANFENCEPOOL_H
#define TNT_FILAMENT_BACKEND_VULKANFENCEPOOL_H

#include "VulkanAsyncHandles.h"
#include "VulkanContext.h"

#include <deque>
#include <mutex>
#include <utility>
#include <vector>

namespace filament::backend {

class VulkanFencePool {
public:
    VulkanFencePool(VulkanContext const& context, VkDevice device, uint32_t minPoolSize);

    // Acquires a shareable fence status object that contains
    // a fence acquired from the pool. The fence status will
    // be cleared, and the fence reclaimed, once the shared_ptr
    // goes out of scope.
    std::shared_ptr<VulkanCmdFence> acquireFenceStatus() noexcept;

    // Clears idle fences, as well as cmdFences that are no longer alive.
    void gc() noexcept;

    // Reclaims fences from all fenceStatus objects, and then clears the
    // fence list.
    void terminate() noexcept;

private:
    VulkanContext const& mContext;
    const VkDevice mDevice;
    const uint32_t mMinPoolSize;
    std::mutex mFenceListMutex;
    std::deque<std::pair<uint64_t, VkFence>> mFences;
    std::vector<std::weak_ptr<VulkanCmdFence>> mFenceStatuses;
    uint32_t mNumFences = 0;
    uint64_t mCurrFrame = 0;

    // Acquires a fence from the pool. This is private because we need mechanisms
    // to ensure that all fences are returned *during* terminate. Instead, users
    // can use acquireFenceStatus(), which tracks fences.
    VkFence acquireFence() noexcept;

    // Releases a fence back into the pool. Typically done in the recycle function
    // of a VulkanCmdFence created by this pool.
    void releaseFence(VkFence fence) noexcept;

    // Allocates a fence within the pool. This is done when there are no free fences
    // available to give out in acquireFence().
    VkFence allocateFence() const noexcept;

    // Releases a fence from the pool. This is done when there are many idle fences,
    // as well as when the pool is terminated.
    void destroyFence(VkFence fence) const noexcept;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANFENCEPOOL_H
