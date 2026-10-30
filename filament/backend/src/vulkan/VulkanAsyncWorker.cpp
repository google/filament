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
#include "VulkanAsyncWorker.h"
namespace filament::backend {
VulkanAsyncWorker::VulkanAsyncWorker(const VulkanPlatform* platform, const VulkanContext& context,
    ResourceManager* resourceManager, bool asyncAvailable) {
    if (asyncAvailable) {
        // keep track of our own Semaphore manager for the later Sync work
        mSemaphoreManager = std::make_unique<VulkanSemaphoreManager>(platform->getDevice(), resourceManager);
        mAsyncCommands = std::make_unique<VulkanCommands>(
                        platform->getDevice(),
                        platform->getAsyncQueue(),
                        platform->getAsyncQueueFamilyIndex(),
                        platform->getProtectedGraphicsQueue(),
                        platform->getProtectedGraphicsQueueFamilyIndex(),
                        context,
                        mSemaphoreManager.get()
                        );
    }
}
void VulkanAsyncWorker::terminate() noexcept{
    if (mTaskHandler) {
        mTaskHandler->shutdown();
        mTaskHandler.reset();
    }
    mSemaphoreManager->terminate();
    mAsyncCommands->terminate();
}

void VulkanAsyncWorker::runUntilComplete() {
    if (mTaskHandler) {
        mTaskHandler->drain();
    }
}

void VulkanAsyncWorker::postUpdateJob(std::function<void(VulkanCommandBuffer&)> job,
    AsyncCallId jobId, DriverBase::AsyncCompletion* completion, JobQueue* queue) {

    startTaskHandler();
    mTaskHandler->post([this, job, queue, completion] (bool const success) mutable {
        VulkanCommandBuffer& commands = mAsyncCommands->get();
        job(commands);
        if (success) {
            mAsyncCommands->flush();
            mAsyncCommands->wait();
            mAsyncCommands->gc();
            queue->push([completion]()  {
                completion->schedule(AsyncCallStatus::COMPLETED);
                delete completion;
            });
        }
    });

}

void VulkanAsyncWorker::startTaskHandler () {
    // We don't create a task handler (start a thread) unless an Async method is called.
    if (!mTaskHandler) {
        mTaskHandler = std::make_unique<fvkutils::TaskHandler>();
    }
}

}// namespace filament::backend
