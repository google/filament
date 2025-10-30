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
#include "backend/DriverEnums.h"
#include "backend/Platform.h"

#include "vulkan/memory/Resource.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <utility>
#include <vector>

namespace filament::backend {

// Wrapper to enable use of shared_ptr for implementing shared ownership of low-level Vulkan fences.
struct VulkanCmdFence {
    explicit VulkanCmdFence(VkFence fence) : mFence(fence) { }
    ~VulkanCmdFence() = default;

    void setStatus(VkResult const value) {
        std::lock_guard const l(mLock);
        mStatus = value;
        mCond.notify_all();
    }

    VkResult getStatus() {
        std::shared_lock const l(mLock);
        return mStatus;
    }

    void resetFence(VkDevice device);

    FenceStatus wait(VkDevice device, uint64_t timeout,
        std::chrono::steady_clock::time_point until);

    void cancel() {
        setStatus(VK_ERROR_UNKNOWN);
    }

private:
    std::shared_mutex mLock; // NOLINT(*-include-cleaner)
    std::condition_variable_any mCond;
    // Internally we use the VK_INCOMPLETE status to mean "not yet submitted". When this fence
    // gets submitted, its status changes to VK_NOT_READY. Finally, when the GPU actually
    // finishes executing the command buffer, the status changes to VK_SUCCESS.
    VkResult mStatus{ VK_INCOMPLETE };
    VkFence mFence;
};

struct VulkanFence : public HwFence, fvkmemory::ThreadSafeResource {
    VulkanFence() {}

    void setFence(std::shared_ptr<VulkanCmdFence> fence) {
        std::lock_guard const lock(mState->lock);
        mState->sharedFence = std::move(fence);
        mState->cond.notify_all();
    }

    std::shared_ptr<VulkanCmdFence>& getSharedFence() {
        std::lock_guard const lock(mState->lock);
        return mState->sharedFence;
    }

    std::pair<std::shared_ptr<VulkanCmdFence>, bool>
            wait(std::chrono::steady_clock::time_point const until) {
        // hold a reference so that our state doesn't disappear while we wait
        std::shared_ptr state{ mState };
        std::unique_lock lock(state->lock);
        state->cond.wait_until(lock, until, [&state] {
            return bool(state->sharedFence) || state->canceled;
        });
        // here mSharedFence will be null if we timed out
        return { state->sharedFence, state->canceled };
    }

    void cancel() const {
        std::shared_ptr const state{ mState };
        std::unique_lock const lock(state->lock);
        if (state->sharedFence) {
            state->sharedFence->cancel();
        }
        state->canceled = true;
        state->cond.notify_all();
    }

private:
    struct State {
        std::mutex lock;
        std::condition_variable cond;
        bool canceled = false;
        std::shared_ptr<VulkanCmdFence> sharedFence;
    };
    std::shared_ptr<State> mState{ std::make_shared<State>() };
};

struct VulkanSync : fvkmemory::ThreadSafeResource, public HwSync {
    struct CallbackData {
        CallbackHandler* handler;
        Platform::SyncCallback cb;
        Platform::Sync* sync;
        void* userData;
    };

    VulkanSync() {}
    std::mutex lock;
    std::vector<std::unique_ptr<CallbackData>> conversionCallbacks;
};

struct VulkanTimerQuery : public HwTimerQuery, fvkmemory::ThreadSafeResource {
    VulkanTimerQuery(uint32_t startingIndex, uint32_t stoppingIndex)
        : mStartingQueryIndex(startingIndex),
          mStoppingQueryIndex(stoppingIndex) {}

    void setFence(std::shared_ptr<VulkanCmdFence> fence) noexcept {
        std::unique_lock const lock(mFenceMutex);
        mFence = std::move(fence);
    }

    bool isCompleted() noexcept {
        std::unique_lock const lock(mFenceMutex);
        // QueryValue is a synchronous call and might occur before beginTimerQuery has written
        // anything into the command buffer, which is an error according to the validation layer
        // that ships in the Android NDK.  Even when AVAILABILITY_BIT is set, validation seems to
        // require that the timestamp has at least been written into a processed command buffer.

        // This fence indicates that the corresponding buffer has been completed.
        return mFence && mFence->getStatus() == VK_SUCCESS;
    }

    uint32_t getStartingQueryIndex() const { return mStartingQueryIndex; }

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
