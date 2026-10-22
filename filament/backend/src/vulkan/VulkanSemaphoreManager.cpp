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

#include "VulkanSemaphoreManager.h"

#include "VulkanConstants.h"

using namespace bluevk;

namespace {
constexpr size_t INITIAL_POOL_SIZE = FVK_MAX_COMMAND_BUFFERS;

VkSemaphore createSemaphore(VkDevice device) {
    VkSemaphore semaphore;
    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    vkCreateSemaphore(device, &semaphoreInfo, VKALLOC, &semaphore);
    return semaphore;
}

} // namespace

namespace filament::backend {

VulkanSemaphoreManager::VulkanSemaphoreManager(VkDevice device,
        fvkmemory::ResourceManager* resourceManager)
    : mDevice(device),
      mResourceManager(resourceManager) {
    for (size_t i= 0; i < INITIAL_POOL_SIZE; ++i) {
        mPool.push_back(createSemaphore(mDevice));
    }
}

void VulkanSemaphoreManager::terminate() {
    for (VkSemaphore semaphore : mPool) {
        vkDestroySemaphore(mDevice, semaphore, VKALLOC);
    }
    mPool.clear();
}

VulkanSemaphoreManager::Semaphore VulkanSemaphoreManager::acquire() {
    VkSemaphore semaphore;
    if (!mPool.empty()) {
        semaphore = mPool.back();
        mPool.pop_back();
    } else {
        semaphore = createSemaphore(mDevice);
    }
    return Semaphore::construct(mResourceManager, this, semaphore);
}

void VulkanSemaphoreManager::recycle(VkSemaphore semaphore) {
    mPool.push_back(semaphore);
}

} // namespace filament::backend
