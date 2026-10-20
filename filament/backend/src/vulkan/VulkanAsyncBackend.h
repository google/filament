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
#ifndef TNT_VULKANASYNCBACKEND_H
#define TNT_VULKANASYNCBACKEND_H
#include "VulkanCommands.h"
#include "VulkanContext.h"
#include "backend/platforms/VulkanPlatform.h"
#include "vulkan/utils/TaskHandler.h"

namespace filament::backend {
class VulkanAsyncBackend {
    public:
        explicit VulkanAsyncBackend(const VulkanPlatform* platform, const VulkanContext& context,
                fvkmemory::ResourceManager* resourceManager, bool asyncAvailable);

        void postUpdateJob(std::function<void(VulkanCommandBuffer&)> job, AsyncCallId jobId,
            DriverBase::AsyncCompletion* completion, JobQueue* queue);
        void terminate() noexcept;

        void runUntilComplete();
    private:
        std::unique_ptr<VulkanCommands> mAsyncCommands = nullptr;
        std::unique_ptr<VulkanSemaphoreManager> mSemaphoreManager = nullptr;
        std::unique_ptr<fvkutils::TaskHandler> mTaskHandler = nullptr;
        std::unique_ptr<fvkmemory::ResourceManager> mResourceManager = nullptr;
        void startTaskHandler();
        void grabSyncHandles();
};
}// namespace filament::backend

#endif // TNT_VULKANASYNCBACKEND_H
