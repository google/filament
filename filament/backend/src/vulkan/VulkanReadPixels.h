/*
 * Copyright (C) 2023 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_VULKANREADPIXELS_H
#define TNT_FILAMENT_BACKEND_VULKANREADPIXELS_H

#include "vulkan/memory/ResourcePointer.h"

#include <private/backend/Driver.h>

#include <bluevk/BlueVK.h>

#include <utils/compiler.h>
#include <utils/Condition.h>
#include <utils/Invocable.h>
#include <utils/Mutex.h>

#include <math/vec4.h>

#include <functional>
#include <memory>
#include <queue>
#include <thread>
#include <vector>

namespace filament::backend {

struct VulkanContext;
struct VulkanRenderTarget;
struct VulkanTexture;

class VulkanReadPixels {
public:
    // A helper class that runs tasks on a separate thread.
    class TaskHandler {
    public:
        // A task is invoked with `executed = true` from the handler thread. If the handler is shut
        // down before the task is picked up, the task is instead invoked with `executed = false` so
        // that clients can still release whatever the task owns (the user's PixelBufferDescriptor
        // and the Vulkan objects of the request).
        using Task = utils::Invocable<void(bool executed)>;

        TaskHandler();

        // Joins the thread if `shutdown()` was not called: destroying a joinable std::thread
        // terminates the process. Unlike `shutdown()`, this cannot panic: throwing out of a
        // destructor terminates the process as well.
        ~TaskHandler();

        void post(Task&& task);

        // This will block until all of the tasks are done.
        void drain();

        // This will quit without running the pending tasks, but they will still be invoked with
        // `executed = false` so that they can clean up after themselves.
        void shutdown();

    private:
        void loop();

        // Stops the thread and flushes the queue. Unlike `shutdown()` this makes no assertion, so
        // it is safe to call from the destructor.
        void stop() noexcept;

        utils::Mutex mTaskQueueMutex;
        utils::Condition mHasTaskCondition;
        bool mShouldStop UTILS_GUARDED_BY(mTaskQueueMutex);
        std::queue<Task> mTaskQueue UTILS_GUARDED_BY(mTaskQueueMutex);
        // Must be declared last: the thread runs `loop()`, which uses all of the above.
        std::thread mThread;
    };

    using CleanUpPbdFunction = std::function<void(PixelBufferDescriptor&&)>;

    // `graphicsQueue` must be the queue the renderer submits to (VulkanCommands'), not merely a
    // queue of the same family: the readback relies on being ordered against the rendering by
    // submission order, which is only defined within a single queue. This is the non-protected
    // queue; we assume protected content is never read back (see `run()`).
    // `cleanUpPbdFunc` is called (from the handler thread) to hand the pixel buffer back to the
    // client once the readback completed - or was abandoned.
    VulkanReadPixels(VkDevice device, VulkanContext const& context, VkQueue graphicsQueue,
            uint32_t graphicsQueueFamilyIndex, CleanUpPbdFunction&& cleanUpPbdFunc);

    // Must be called from the backend thread.
    void terminate() noexcept;

    // Must be called from the backend thread.
    void run(fvkmemory::resource_ptr<VulkanRenderTarget> srcTarget, uint32_t x, uint32_t y,
            uint32_t width, uint32_t height, PixelBufferDescriptor&& pbd);

    // Must be called from the backend thread.
    void run(fvkmemory::resource_ptr<VulkanTexture> srcTexture, uint8_t level, uint16_t layer,
            uint32_t x, uint32_t y, uint32_t width, uint32_t height, PixelBufferDescriptor&& pbd);

    // Releases the resources of the requests that the handler thread has retired: the Vulkan
    // objects are destroyed and the reference to the source image is dropped. Must be called from
    // the backend thread (see `Request`). This is cheap when there is nothing to collect, so it can
    // be called every tick.
    void gc();

    // This method will block until all of the in-flight requests are complete, and collects their
    // resources. Must be called from the backend thread.
    void runUntilComplete();

private:
    // Everything a single readback owns. This is only ever touched on the backend thread: the
    // Vulkan objects are allocated there and VkCommandPool is externally synchronized, and
    // `srcTexture`'s refcount is not atomic. The handler thread therefore never holds a Request -
    // it hands one back by fence via `retire()`, and `gc()` releases it.
    struct Request {
        VkBuffer stagingBuffer = VK_NULL_HANDLE;
        VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
        VkFence fence = VK_NULL_HANDLE;
        VkCommandBuffer cmdbuffer = VK_NULL_HANDLE;
        // Nothing else keeps the source image alive while the copy is in flight: `run()`'s argument
        // dies when it returns, and, unlike the command buffers recorded through
        // VulkanCommandBuffer, ours does not acquire() the resources it references. Dropping this
        // reference can destroy the VkImage, so it must not be released before `fence` signals.
        fvkmemory::resource_ptr<VulkanTexture> srcTexture;
    };

    // Whether readPixels has ever been called: until then we have neither a command pool nor a
    // handler thread, and therefore nothing to collect. `mCommandPool` is only ever written from
    // the backend thread (in `run()` and `terminate()`), so this needs no lock.
    bool isActive() const noexcept { return mCommandPool != VK_NULL_HANDLE; }

    // Called from the handler thread once it is done using the resources of the request that owns
    // `fence`. The request itself stays on the backend thread (see `Request`).
    void retire(VkFence fence);

    VkDevice mDevice = VK_NULL_HANDLE;
    VulkanContext const& mContext;
    VkQueue const mQueue;
    // Only used to create the command pool.
    uint32_t const mGraphicsQueueFamilyIndex;
    CleanUpPbdFunction const mCleanUpPbd;
    VkCommandPool mCommandPool = VK_NULL_HANDLE;

    // The readbacks that have been submitted but not collected yet. Backend thread only.
    std::vector<Request> mInFlightRequests;

    utils::Mutex mRetiredFencesMutex;
    std::vector<VkFence> mRetiredFences UTILS_GUARDED_BY(mRetiredFencesMutex);

    // Must be declared last. If `terminate()` was never called, ~TaskHandler() flushes the pending
    // tasks, and those call `retire()` and `mCleanUpPbd`: everything they touch must outlive the
    // handler, and members are destroyed in reverse declaration order.
    std::unique_ptr<TaskHandler> mTaskHandler;
};

}// namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_VULKANREADPIXELS_H
