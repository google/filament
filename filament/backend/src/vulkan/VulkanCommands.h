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

#include "VulkanConstants.h"

#include <utils/Condition.h>
#include <utils/Mutex.h>

namespace filament::backend {

// Wrapper to enable use of shared_ptr for implementing shared ownership of low-level Vulkan fences.
struct VulkanCmdFence {
    VulkanCmdFence(VkDevice device, bool signaled = false);
    ~VulkanCmdFence();
    const VkDevice device;
    VkFence fence;
    utils::Condition condition;
    utils::Mutex mutex;
    std::atomic<VkResult> status;
};

// The submission fence has shared ownership semantics because it is potentially wrapped by a
// DriverApi fence object and should not be destroyed until both the DriverApi object is freed and
// we're done waiting on the most recent submission of the given command buffer.
struct VulkanCommandBuffer {
    VulkanCommandBuffer() {}
    VulkanCommandBuffer(VulkanCommandBuffer const&) = delete;
    VulkanCommandBuffer& operator=(VulkanCommandBuffer const&) = delete;
    VkCommandBuffer cmdbuffer = VK_NULL_HANDLE;
    std::shared_ptr<VulkanCmdFence> fence;
    uint32_t index = 0;
};

// Allows classes to be notified after a new command buffer has been activated.
class CommandBufferObserver {
public:
    virtual void onCommandBuffer(const VulkanCommandBuffer& cmdbuffer) = 0;
    virtual ~CommandBufferObserver();
};

// Lazily creates command buffers and manages a set of submitted command buffers.
// Submitted command buffers form a dependency chain using VkSemaphore.
class VulkanCommands {
    public:
        VulkanCommands(VkDevice device, uint32_t queueFamilyIndex);
        ~VulkanCommands();

        // Creates a "current" command buffer if none exists, otherwise returns the current one.
        VulkanCommandBuffer const& get();

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

    private:
        static constexpr int CAPACITY = VK_MAX_COMMAND_BUFFERS;
        const VkDevice mDevice;
        VkQueue mQueue;
        VkCommandPool mPool;
        VulkanCommandBuffer* mCurrent = nullptr;
        VkSemaphore mSubmissionSignal = {};
        VkSemaphore mInjectedSignal = {};
        VulkanCommandBuffer mStorage[CAPACITY] = {};
        VkSemaphore mSubmissionSignals[CAPACITY] = {};
        size_t mAvailableCount = CAPACITY;
        CommandBufferObserver* mObserver = nullptr;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANCOMMANDS_H
