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

#ifndef TNT_FILAMENT_BACKEND_VULKANCOMMANDS_H
#define TNT_FILAMENT_BACKEND_VULKANCOMMANDS_H

#include <bluevk/BlueVK.h>

#include "DriverBase.h"

#include "VulkanAsyncHandles.h"
#include "VulkanConstants.h"
#include "VulkanResources.h"

#include <utils/Condition.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Mutex.h>

#include <atomic>

#include <chrono>
#include <list>
#include <string>
#include <utility>

namespace filament::backend {

struct VulkanContext;

#if FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS)
class VulkanGroupMarkers {
public:
    using Timestamp = std::chrono::time_point<std::chrono::high_resolution_clock>;

    void push(std::string const& marker, Timestamp start = {}) noexcept;
    std::pair<std::string, Timestamp> pop() noexcept;
    std::pair<std::string, Timestamp> pop_bottom() noexcept;
    std::pair<std::string, Timestamp> top() const;
    bool empty() const noexcept;

private:
    std::list<std::string> mMarkers;
#if FVK_ENABLED(FVK_DEBUG_PRINT_GROUP_MARKERS)
    std::list<Timestamp> mTimestamps;
#endif
};

#endif // FVK_DEBUG_GROUP_MARKERS

// The submission fence has shared ownership semantics because it is potentially wrapped by a
// DriverApi fence object and should not be destroyed until both the DriverApi object is freed and
// we're done waiting on the most recent submission of the given command buffer.
struct VulkanCommandBuffer {
    VulkanCommandBuffer(VulkanResourceAllocator* allocator, VkDevice device, VkCommandPool pool);

    VulkanCommandBuffer(VulkanCommandBuffer const&) = delete;
    VulkanCommandBuffer& operator=(VulkanCommandBuffer const&) = delete;

    inline void acquire(VulkanResource* resource) {
        mResourceManager.acquire(resource);
    }

    inline void acquire(VulkanAcquireOnlyResourceManager* srcResources) {
        mResourceManager.acquireAll(srcResources);
    }

    inline void reset() {
        fence.reset();
        mResourceManager.clear();
        mPipeline = VK_NULL_HANDLE;
    }

    inline void setPipeline(VkPipeline pipeline) {
        mPipeline = pipeline;
    }

    inline VkPipeline pipeline() const {
        return mPipeline;
    }

    inline VkCommandBuffer buffer() const {
        if (fence) {
            return mBuffer;
        }
        return VK_NULL_HANDLE;
    }

    std::shared_ptr<VulkanCmdFence> fence;

private:
    VulkanAcquireOnlyResourceManager mResourceManager;
    VkCommandBuffer mBuffer;
    VkPipeline mPipeline;
};

// Allows classes to be notified after a new command buffer has been activated.
class CommandBufferObserver {
public:
    virtual void onCommandBuffer(const VulkanCommandBuffer& cmdbuffer) = 0;
    virtual ~CommandBufferObserver();
};

// Manages a set of command buffers and semaphores, exposing an API that is significantly simpler
// than the raw Vulkan API.
//
// The manager's API primarily consists of get() and flush(). The "get" method acquires a fresh
// command buffer in the recording state, while the "flush" method releases a command buffer and
// changes its state from recording to executing. Some of the operational details are listed below.
//
// - Manages a dependency chain of submitted command buffers using VkSemaphore.
//    - This creates a guarantee of in-order execution.
//    - Semaphores are recycled to prevent create / destroy churn.
//
// - Notifies listeners when recording begins in a new VkCommandBuffer.
//    - Used by PipelineCache so that it knows when to clear out its shadow state.
//
// - Allows 1 user to inject a "dependency" semaphore that stalls the next flush.
//    - This is used for asynchronous acquisition of a swap chain image, since the GPU
//      might require a valid swap chain image when it starts executing the command buffer.
//
// - Allows 1 user to listen to the most recent flush event using a "finished" VkSemaphore.
//    - This is used to trigger presentation of the swap chain image.
//
// - Allows off-thread queries of command buffer status.
//    - Exposes an "updateFences" method that transfers current fence status into atomics.
//    - Users can examine these atomic variables (see VulkanCmdFence) to determine status.
//    - We do this because vkGetFenceStatus must be called from the rendering thread.
//
class VulkanCommands {
public:
    VulkanCommands(VkDevice device, VkQueue queue, uint32_t queueFamilyIndex,
            VulkanContext* context, VulkanResourceAllocator* allocator);

    void terminate();

    // Creates a "current" command buffer if none exists, otherwise returns the current one.
    VulkanCommandBuffer& get();

    // Submits the current command buffer if it exists, then sets "current" to null.
    // If there are no outstanding commands then nothing happens and this returns false.
    bool flush();

    // Returns the "rendering finished" semaphore for the most recent flush and removes
    // it from the existing dependency chain. This is especially useful for setting up
    // vkQueuePresentKHR.
    VkSemaphore acquireFinishedSignal();

    // Takes a semaphore that signals when the next flush can occur. Only one injected
    // semaphore is allowed per flush. Useful after calling vkAcquireNextImageKHR.
    void injectDependency(VkSemaphore next);

    // Destroys all command buffers that are no longer in use.
    void gc();

    // Waits for all outstanding command buffers to finish.
    void wait();

    // Updates the atomic "status" variable in every extant fence.
    void updateFences();

    // Sets an observer who is notified every time a new command buffer has been made "current".
    // The observer's event handler can only be called during get().
    void setObserver(CommandBufferObserver* observer) { mObserver = observer; }

#if FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS)
    void pushGroupMarker(char const* str, VulkanGroupMarkers::Timestamp timestamp = {});

    void popGroupMarker();

    void insertEventMarker(char const* string, uint32_t len);

    std::string getTopGroupMarker() const;
#endif

private:
    static constexpr int CAPACITY = FVK_MAX_COMMAND_BUFFERS;
    VkDevice const mDevice;
    VkQueue const mQueue;
    VkCommandPool const mPool;
    VulkanContext const* mContext;

    // int8 only goes up to 127, therefore capacity must be less than that.
    static_assert(CAPACITY < 128);
    int8_t mCurrentCommandBufferIndex = -1;
    VkSemaphore mSubmissionSignal = {};
    VkSemaphore mInjectedSignal = {};
    utils::FixedCapacityVector<std::unique_ptr<VulkanCommandBuffer>> mStorage;
    VkFence mFences[CAPACITY] = {};
    VkSemaphore mSubmissionSignals[CAPACITY] = {};
    uint8_t mAvailableBufferCount = CAPACITY;
    CommandBufferObserver* mObserver = nullptr;

#if FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS)
    std::unique_ptr<VulkanGroupMarkers> mGroupMarkers;
    std::unique_ptr<VulkanGroupMarkers> mCarriedOverMarkers;
#endif
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANCOMMANDS_H
