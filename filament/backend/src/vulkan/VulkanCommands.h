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
#include "VulkanContext.h"
#include "vulkan/memory/ResourcePointer.h"
#include "vulkan/utils/StaticVector.h"

#include <utils/Condition.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Mutex.h>

#include <atomic>

#include <chrono>
#include <list>
#include <string>
#include <utility>

namespace filament::backend {

using namespace fvkmemory;

#if FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS)
class VulkanGroupMarkers {
public:
    using Timestamp = std::chrono::time_point<std::chrono::high_resolution_clock>;

    void push(std::string const& marker, Timestamp start = {}) noexcept;
    std::pair<std::string, Timestamp> pop() noexcept;
    std::pair<std::string, Timestamp> pop_bottom() noexcept;
    std::pair<std::string, Timestamp> const& top() const;
    bool empty() const noexcept;

private:
    std::list<std::pair<std::string, Timestamp>> mMarkers;
};

#endif // FVK_DEBUG_GROUP_MARKERS

// The submission fence has shared ownership semantics because it is potentially wrapped by a
// DriverApi fence object and should not be destroyed until both the DriverApi object is freed and
// we're done waiting on the most recent submission of the given command buffer.
struct VulkanCommandBuffer {
    VulkanCommandBuffer(VulkanContext const& mContext,
            VkDevice device, VkQueue queue, VkCommandPool pool, bool isProtected);

    VulkanCommandBuffer(VulkanCommandBuffer const&) = delete;
    VulkanCommandBuffer& operator=(VulkanCommandBuffer const&) = delete;

    ~VulkanCommandBuffer();

    inline void acquire(fvkmemory::resource_ptr<fvkmemory::Resource> resource) {
        mResources.push_back(resource);
    }

    void reset() noexcept;

    inline void insertWait(VkSemaphore sem, VkPipelineStageFlags waitStage) {
        mWaitSemaphores.push_back(sem);
        mWaitSemaphoreStages.push_back(waitStage);
    }

    void pushMarker(char const* marker) noexcept;
    void popMarker() noexcept;
    void insertEvent(char const* marker) noexcept;

    void begin() noexcept;
    VkSemaphore submit();

    inline void setComplete() {
        mFenceStatus->setStatus(VK_SUCCESS);
    }

    VkResult getStatus() {
        return mFenceStatus->getStatus();
    }

    std::shared_ptr<VulkanCmdFence> getFenceStatus() const {
        return mFenceStatus;
    }

    VkFence getVkFence() const {
        return mFence;
    }

    VkCommandBuffer buffer() const {
        return mBuffer;
    }

    uint32_t age() const {
        return mAge;
    }

private:
    static uint32_t sAgeCounter;

    VulkanContext const& mContext;
    uint8_t mMarkerCount;
    bool const isProtected;
    VkDevice mDevice;
    VkQueue mQueue;
    fvkutils::StaticVector<VkSemaphore, 2> mWaitSemaphores;
    fvkutils::StaticVector<VkPipelineStageFlags, 2> mWaitSemaphoreStages;
    VkCommandBuffer mBuffer;
    VkSemaphore mSubmission;
    VkFence mFence;
    std::shared_ptr<VulkanCmdFence> mFenceStatus;
    std::vector<fvkmemory::resource_ptr<Resource>> mResources;
    uint32_t mAge;
};

struct CommandBufferPool {
    using ActiveBuffers = utils::bitset64;
    static constexpr int8_t INVALID = -1;

    CommandBufferPool(VulkanContext const& context, VkDevice device, VkQueue queue,
            uint8_t queueFamilyIndex, bool isProtected);
    ~CommandBufferPool();

    VulkanCommandBuffer& getRecording();

    void gc();
    void update();
    VkSemaphore flush();
    void wait();
    void waitFor(VkSemaphore previousAction, VkPipelineStageFlags waitStage);

#if FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS)
    std::string topMarker() const;
    void pushMarker(char const* marker, VulkanGroupMarkers::Timestamp timestamp);
    std::pair<std::string, VulkanGroupMarkers::Timestamp> popMarker();
    void insertEvent(char const* marker);
#endif

    inline bool isRecording() const { return mRecording != INVALID; }

private:
    static constexpr int CAPACITY = FVK_MAX_COMMAND_BUFFERS;
    // int8 only goes up to 127, therefore capacity must be less than that.
    static_assert(CAPACITY < 128);

    // The number of bits in ActiveBuffers describe the usage of the buffers in the pool, so must be
    // larger than the size of the pool.
    static_assert(sizeof(ActiveBuffers) * 8 >= CAPACITY);

    using BufferList = utils::FixedCapacityVector<std::unique_ptr<VulkanCommandBuffer>>;
    VkDevice mDevice;
    VkCommandPool mPool;
    ActiveBuffers mSubmitted;
    std::vector<std::unique_ptr<VulkanCommandBuffer>> mBuffers;
    int8_t mRecording;

#if FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS)
    std::unique_ptr<VulkanGroupMarkers> mGroupMarkers;
#endif
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
            VkQueue protectedQueue, uint32_t protectedQueueFamilyIndex,
            VulkanContext const& context);

    void terminate();

    // Creates a "current" command buffer if none exists, otherwise returns the current one.
    VulkanCommandBuffer& get();

    // Creates a "current" protected capable command buffer if none exists, otherwise
    // returns the current one.
    VulkanCommandBuffer& getProtected();

    // Submits the current command buffer if it exists, then sets "current" to null.
    // If there are no outstanding commands then nothing happens and this returns false.
    bool flush();

    // Returns the "rendering finished" semaphore for the most recent flush and removes
    // it from the existing dependency chain. This is especially useful for setting up
    // vkQueuePresentKHR.
    VkSemaphore acquireFinishedSignal() {
        VkSemaphore ret= mLastSubmit;
        mLastSubmit = VK_NULL_HANDLE;
        return ret;
    }

    // Takes a semaphore that signals when the next flush can occur. Only one injected
    // semaphore is allowed per flush. Useful after calling vkAcquireNextImageKHR.
    // waitStage
    void injectDependency(VkSemaphore next, VkPipelineStageFlags waitStage) {
        mInjectedDependency = next;
        mInjectedDependencyWaitStage = waitStage;
    }

    // Destroys all command buffers that are no longer in use.
    void gc();

    // Waits for all outstanding command buffers to finish.
    void wait();

    // Updates the atomic "status" variable in every extant fence.
    void updateFences();

#if FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS)
    void pushGroupMarker(char const* str, VulkanGroupMarkers::Timestamp timestamp = {});
    void popGroupMarker();
    void insertEventMarker(char const* string, uint32_t len);
    std::string getTopGroupMarker() const;
#endif

private:
    VkDevice const mDevice;
    VkQueue const mProtectedQueue;
    // For defered initialization if/when we need protected content
    uint32_t const mProtectedQueueFamilyIndex;
    VulkanContext const& mContext;

    std::unique_ptr<CommandBufferPool> mPool;
    std::unique_ptr<CommandBufferPool> mProtectedPool;

    VkSemaphore mInjectedDependency = VK_NULL_HANDLE;
    VkSemaphore mLastSubmit = VK_NULL_HANDLE;

    VkPipelineStageFlags mInjectedDependencyWaitStage = 0;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANCOMMANDS_H
