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

#include "VulkanAsyncHandles.h"

#include <backend/DriverEnums.h>

#include <utils/debug.h>

#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <chrono>

using namespace bluevk;

namespace filament::backend {

FenceStatus VulkanCmdFence::wait(VkDevice device, uint64_t const timeout,
    std::chrono::steady_clock::time_point const until) {

    // this lock MUST be held for READ when calling vkWaitForFences()
    std::shared_lock rl(mLock);

    // If the vulkan fence has not been submitted yet, we need to wait for that before we
    // can use vkWaitForFences()
    if (mStatus == VK_INCOMPLETE) {
        bool const success = mCond.wait_until(rl, until, [this] {
            // Internally we use the VK_INCOMPLETE status to mean "not yet submitted".
            // When this fence gets submitted, its status changes to VK_NOT_READY.
            return mStatus != VK_INCOMPLETE || mCanceled;
        });
        if (!success) {
            // !success indicates a timeout or cancel
            return mCanceled ? FenceStatus::ERROR : FenceStatus::TIMEOUT_EXPIRED;
        }
    }

    // The fence could have already signaled, avoid calling into vkWaitForFences()
    if (mStatus == VK_SUCCESS) {
        return FenceStatus::CONDITION_SATISFIED;
    }

    // Or it could have been canceled, return immediately
    if (mCanceled) {
        return FenceStatus::ERROR;
    }

    // If we're here, we know that vkQueueSubmit has been called (because it sets the status
    // to VK_NOT_READY).
    // Now really wait for the fence while holding the shared_lock, this allows several
    // threads to call vkWaitForFences(), but will prevent vkResetFence from taking
    // place simultaneously. vkResetFence is only called once it knows the fence has signaled,
    // which guaranties that vkResetFence won't have to wait too long, just enough for
    // all the vkWaitForFences() to return.
    VkResult const status = vkWaitForFences(device, 1, &mFence, VK_TRUE, timeout);
    if (status == VK_TIMEOUT) {
        return FenceStatus::TIMEOUT_EXPIRED;
    }

    if (status == VK_SUCCESS) {
        rl.unlock();
        std::lock_guard const wl(mLock);
        mStatus = status;
        return FenceStatus::CONDITION_SATISFIED;
    }

    return FenceStatus::ERROR; // not supported
}

void VulkanCmdFence::resetFence(VkDevice device) {
    // This lock prevents vkResetFences() from being called simultaneously with vkWaitForFences(),
    // but by construction, when we're here, we know that the fence has signaled and
    // vkWaitForFences() will return shortly.
    std::lock_guard const l(mLock);
    assert_invariant(mStatus == VK_SUCCESS);
    vkResetFences(device, 1, &mFence);
}

} // namespace filament::backend
