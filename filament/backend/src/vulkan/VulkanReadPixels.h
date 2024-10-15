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
#include "private/backend/Driver.h"

#include <bluevk/BlueVK.h>
#include <math/vec4.h>

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace filament::backend {

struct VulkanRenderTarget;

class VulkanReadPixels {
public:
    // A helper class that runs tasks on a separate thread.
    class TaskHandler {
    public:
        using WorkloadFunc = std::function<void()>;
        using OnCompleteFunc = std::function<void()>;
        using Task = std::pair<WorkloadFunc, OnCompleteFunc>;

        TaskHandler();

        // In addition to the workload that the handler will call, client must also provide an
        // oncomplete function that the handler will call either when the workload completes or when
        // the handler is shutdown (so that we can clean-up even when the task was not carried out).
        void post(WorkloadFunc&& workload, OnCompleteFunc&& oncomplete);

        // This will block until all of the tasks are done.
        void drain();

        // This will quit without running the workloads, but oncomplete callbacks will still be
        // called.
        void shutdown();

    private:
        void loop();

        bool mShouldStop;
        std::condition_variable mHasTaskCondition;
        std::mutex mTaskQueueMutex;
        std::queue<Task> mTaskQueue;
        std::thread mThread;
    };

    using OnReadCompleteFunction = std::function<void(PixelBufferDescriptor&&)>;
    using SelecteMemoryFunction = std::function<uint32_t(uint32_t, VkFlags)>;

    explicit VulkanReadPixels(VkDevice device);

    void terminate() noexcept;

    void run(fvkmemory::resource_ptr<VulkanRenderTarget> srcTarget, uint32_t x, uint32_t y,
            uint32_t width, uint32_t height, uint32_t graphicsQueueFamilyIndex,
            PixelBufferDescriptor&& pbd, SelecteMemoryFunction const& selectMemoryFunc,
            OnReadCompleteFunction const& readCompleteFunc);

    // This method will block until all of the in-flight requests are complete.
    void runUntilComplete() noexcept;

private:
    VkDevice mDevice = VK_NULL_HANDLE;
    VkCommandPool mCommandPool = VK_NULL_HANDLE;
    std::unique_ptr<TaskHandler> mTaskHandler;
};

}// namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_VULKANREADPIXELS_H
