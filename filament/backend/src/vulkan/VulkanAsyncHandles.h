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
#include <limits>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <utility>
#include <vector>

namespace filament::backend {

// Wrapper to enable use of shared_ptr for implementing shared ownership of Vulkan fences
// and timer query results.
struct VulkanCmdBufferState {
    static uint64_t const UNKNOWN_BUFFER_DURATION = std::numeric_limits<uint64_t>::max();

    explicit VulkanCmdBufferState(VkFence fence)
            : mFence(fence) {
    }
    ~VulkanCmdBufferState() = default;

    void setStatus(VkResult const value, uint64_t duration) {
        std::lock_guard const l(mLock);
        mStatus = value;
        mDuration = duration;
        mCond.notify_all();
    }

    uint64_t getBufferDuration() {
        std::shared_lock const l(mLock);
        return mDuration;
    }

    VkResult getFenceStatus() {
        std::shared_lock const l(mLock);
        return mStatus;
    }

    void resetFence(VkDevice device);

    FenceStatus waitOnFence(VkDevice device, uint64_t timeout,
        std::chrono::steady_clock::time_point until);

    void cancel() {
        std::lock_guard const l(mLock);
        mCanceled = true;
        mCond.notify_all();
    }

    void setNextState(std::shared_ptr<VulkanCmdBufferState> next) {
        mNextState = std::move(next);
    }

    std::shared_ptr<VulkanCmdBufferState>& getNextState() {
        return mNextState;
    }

    VkFence getVkFence() const {
        return mFence;
    }

private:
    std::shared_mutex mLock; // NOLINT(*-include-cleaner)
    std::condition_variable_any mCond;
    bool mCanceled = false;
    // Internally we use the VK_INCOMPLETE status to mean "not yet submitted". When this fence
    // gets submitted, its status changes to VK_NOT_READY. Finally, when the GPU actually
    // finishes executing the command buffer, the status changes to VK_SUCCESS.
    VkResult mStatus{ VK_INCOMPLETE };
    VkFence const mFence;

    uint64_t mDuration = UNKNOWN_BUFFER_DURATION;

    // We assume that command buffers are serialized.  This will point to the state of the next
    // buffer. This is necessary so that we can return total duration of multiple command buffers
    // when a TimerQuery is requested by the front-end.
    std::shared_ptr<VulkanCmdBufferState> mNextState;
};

struct VulkanFence : public HwFence, fvkmemory::ThreadSafeResource {
    VulkanFence() {}

    void setCmdBufferState(std::shared_ptr<VulkanCmdBufferState> state) {
        std::lock_guard l(lock);
        cmdbufState = std::move(state);
        cond.notify_all();
    }

    std::shared_ptr<VulkanCmdBufferState>& getCmdBufferState() {
        std::lock_guard l(lock);
        return cmdbufState;
    }

    std::pair<std::shared_ptr<VulkanCmdBufferState>, bool> wait(
            std::chrono::steady_clock::time_point const until) {
        std::unique_lock l(lock);
        cond.wait_until(l, until, [&] { return bool(cmdbufState) || canceled; });
        // here cmdbufState will be null if we timed out
        return { cmdbufState, canceled };
    }

    void cancel() const {
        std::lock_guard const l(lock);
        if (cmdbufState) {
            cmdbufState->cancel();
        }
        canceled = true;
        cond.notify_all();
    }

private:
    mutable std::mutex lock;
    mutable std::condition_variable cond;
    mutable bool canceled = false;
    std::shared_ptr<VulkanCmdBufferState> cmdbufState;
};

struct VulkanSync : public HwSync, fvkmemory::ThreadSafeResource {
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

    static decltype(VulkanCmdBufferState::UNKNOWN_BUFFER_DURATION) const
            UNKNOWN_QUERY_RESULT = VulkanCmdBufferState::UNKNOWN_BUFFER_DURATION;

    VulkanTimerQuery() {}

    void setBeginState(std::shared_ptr<VulkanCmdBufferState> state) {
        {
            std::lock_guard const lock(mMutex);
            mBeginState = std::move(state);
        }
        {
            std::lock_guard<std::mutex> lock(mDurationLock);
            mDuration = UNKNOWN_QUERY_RESULT;
        }

    }

    void setEndState(std::shared_ptr<VulkanCmdBufferState> state) {
        {
            std::lock_guard const lock(mMutex);
            mEndState = std::move(state);
        }
        {
            std::lock_guard<std::mutex> lock(mDurationLock);
            mDuration = UNKNOWN_QUERY_RESULT;
        }
    }

    bool isComplete() {
        std::shared_lock const lock(mMutex);
        return mBeginState && mEndState &&
                mBeginState->getBufferDuration() != UNKNOWN_QUERY_RESULT &&
                mEndState->getBufferDuration() != UNKNOWN_QUERY_RESULT;
    }

    uint64_t getResult();

private:
    std::shared_mutex mMutex; // NOLINT(*-include-cleaner)
    std::mutex mDurationLock;
    std::shared_ptr<VulkanCmdBufferState> mBeginState;
    std::shared_ptr<VulkanCmdBufferState> mEndState;
    uint64_t mDuration = UNKNOWN_QUERY_RESULT;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANHASYNCANDLES_H
