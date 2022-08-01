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

#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 26812) // Unscoped enums
#endif

#include "VulkanCommands.h"

#include "VulkanConstants.h"

#include <utils/Log.h>
#include <utils/Panic.h>
#include <utils/debug.h>

using namespace bluevk;
using namespace utils;

namespace filament::backend {

VulkanCmdFence::VulkanCmdFence(VkDevice device, bool signaled) : device(device) {
    VkFenceCreateInfo fenceCreateInfo { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    if (signaled) {
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }
    vkCreateFence(device, &fenceCreateInfo, VKALLOC, &fence);

    // Internally we use the VK_INCOMPLETE status to mean "not yet submitted". When this fence gets
    // submitted, its status changes to VK_NOT_READY. Finally, when the GPU actually finishes
    // executing the command buffer, the status changes to VK_SUCCESS.
    status.store(VK_INCOMPLETE);
}

VulkanCmdFence::~VulkanCmdFence() {
    vkDestroyFence(device, fence, VKALLOC);
}

CommandBufferObserver::~CommandBufferObserver() {}

static VkQueue getQueue(VkDevice device, uint32_t queueFamilyIndex) {
    VkQueue queue;
    vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);
    return queue;
}

static VkCommandPool createPool(VkDevice device, uint32_t queueFamilyIndex) {
    VkCommandPoolCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags =
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = queueFamilyIndex,
    };
    VkCommandPool pool;
    vkCreateCommandPool(device, &createInfo, VKALLOC, &pool);
    return pool;

}

VulkanCommands::VulkanCommands(VkDevice device, uint32_t queueFamilyIndex) : mDevice(device),
        mQueue(getQueue(device, queueFamilyIndex)), mPool(createPool(mDevice, queueFamilyIndex)) {
    VkSemaphoreCreateInfo sci { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    for (auto& semaphore : mSubmissionSignals) {
        vkCreateSemaphore(mDevice, &sci, nullptr, &semaphore);
    }
}

VulkanCommands::~VulkanCommands() {
    wait();
    gc();
    vkDestroyCommandPool(mDevice, mPool, VKALLOC);
    for (VkSemaphore sema : mSubmissionSignals) {
        vkDestroySemaphore(mDevice, sema, VKALLOC);
    }
}

VulkanCommandBuffer const& VulkanCommands::get() {
    if (mCurrent) {
        return *mCurrent;
    }

    // If we ran out of available command buffers, stall until one finishes. This is very rare.
    // It occurs only when Filament invokes commit() or endFrame() a large number of times without
    // presenting the swap chain or waiting on a fence.
    while (mAvailableCount == 0) {
#if VK_REPORT_STALLS
        slog.i  << "VulkanCommands has stalled. "
                << "If this occurs frequently, consider increasing VK_MAX_COMMAND_BUFFERS."
                << io::endl;
#endif
        wait();
        gc();
    }

    // Find an available slot.
    for (VulkanCommandBuffer& wrapper : mStorage) {
        if (wrapper.cmdbuffer == VK_NULL_HANDLE) {
            mCurrent = &wrapper;
            break;
        }
    }

    assert_invariant(mCurrent);
    --mAvailableCount;

    // Create the low-level command buffer.
    const VkCommandBufferAllocateInfo allocateInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = mPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    vkAllocateCommandBuffers(mDevice, &allocateInfo, &mCurrent->cmdbuffer);

    // Note that the fence wrapper uses shared_ptr because a DriverAPI fence can also have ownership
    // over it.  The destruction of the low-level fence occurs either in VulkanCommands::gc(), or in
    // VulkanDriver::destroyFence(), both of which are safe spots.
    mCurrent->fence = std::make_shared<VulkanCmdFence>(mDevice);

    // Begin writing into the command buffer.
    const VkCommandBufferBeginInfo binfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(mCurrent->cmdbuffer, &binfo);

    // Notify the observer that a new command buffer has been activated.
    if (mObserver) {
        mObserver->onCommandBuffer(*mCurrent);
    }

    return *mCurrent;
}

bool VulkanCommands::flush() {
    // It's perfectly fine to call flush when no commands have been written.
    if (mCurrent == nullptr) {
        return false;
    }

    const int64_t index = mCurrent - &mStorage[0];
    VkSemaphore renderingFinished = mSubmissionSignals[index];

    vkEndCommandBuffer(mCurrent->cmdbuffer);

    // If the injected semaphore is an "image available" semaphore that has not yet been signaled,
    // it is sometimes fine to start executing commands anyway, as along as we stall the GPU at the
    // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT stage. However we need to assume the worst
    // here and use VK_PIPELINE_STAGE_ALL_COMMANDS_BIT. This is a more aggressive stall, but it is
    // the only safe option because the previously submitted command buffer might have set up some
    // state that the new command buffer depends on.
    VkPipelineStageFlags waitDestStageMasks[2] = {
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    };

    VkSemaphore signals[2] = {
        VK_NULL_HANDLE,
        VK_NULL_HANDLE,
    };

    VkSubmitInfo submitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = signals,
        .pWaitDstStageMask = waitDestStageMasks,
        .commandBufferCount = 1,
        .pCommandBuffers = &mCurrent->cmdbuffer,
        .signalSemaphoreCount = 1u,
        .pSignalSemaphores = &renderingFinished,
    };

    if (mSubmissionSignal) {
        signals[submitInfo.waitSemaphoreCount++] = mSubmissionSignal;
    }

    if (mInjectedSignal) {
        signals[submitInfo.waitSemaphoreCount++] = mInjectedSignal;
    }

    if (FILAMENT_VULKAN_VERBOSE) {
        slog.i << "Submitting cmdbuffer=" << mCurrent->cmdbuffer
            << " wait=(" << signals[0] << ", " << signals[1] << ") "
            << " signal=" << renderingFinished
            << io::endl;
    }

    auto& cmdfence = mCurrent->fence;
    std::unique_lock<utils::Mutex> lock(cmdfence->mutex);
    cmdfence->status.store(VK_NOT_READY);
    UTILS_UNUSED_IN_RELEASE VkResult result = vkQueueSubmit(mQueue, 1, &submitInfo, cmdfence->fence);
    cmdfence->condition.notify_all();
    lock.unlock();

    assert_invariant(result == VK_SUCCESS);

    mSubmissionSignal = renderingFinished;
    mInjectedSignal = VK_NULL_HANDLE;
    mCurrent = nullptr;
    return true;
}

VkSemaphore VulkanCommands::acquireFinishedSignal() {
    VkSemaphore semaphore = mSubmissionSignal;
    mSubmissionSignal = VK_NULL_HANDLE;
    if (FILAMENT_VULKAN_VERBOSE) {
        slog.i << "Acquiring " << semaphore << " (e.g. for vkQueuePresentKHR)" << io::endl;
    }
    return semaphore;
}

void VulkanCommands::injectDependency(VkSemaphore next) {
    assert_invariant(mInjectedSignal == VK_NULL_HANDLE);
    mInjectedSignal = next;
    if (FILAMENT_VULKAN_VERBOSE) {
        slog.i << "Injecting " << next << " (e.g. due to vkAcquireNextImageKHR)" << io::endl;
    }
}

void VulkanCommands::wait() {
    VkFence fences[CAPACITY];
    uint32_t count = 0;
    for (auto& wrapper : mStorage) {
        if (wrapper.cmdbuffer != VK_NULL_HANDLE && mCurrent != &wrapper) {
            fences[count++] = wrapper.fence->fence;
        }
    }
    if (count > 0) {
        vkWaitForFences(mDevice, count, fences, VK_TRUE, UINT64_MAX);
    }
}

void VulkanCommands::gc() {
    for (auto& wrapper : mStorage) {
        if (wrapper.cmdbuffer != VK_NULL_HANDLE) {
            VkResult result = vkWaitForFences(mDevice, 1, &wrapper.fence->fence, VK_TRUE, 0);
            if (result == VK_SUCCESS) {
                vkFreeCommandBuffers(mDevice, mPool, 1, &wrapper.cmdbuffer);
                wrapper.cmdbuffer = VK_NULL_HANDLE;
                wrapper.fence->status.store(VK_SUCCESS);
                wrapper.fence.reset();
                ++mAvailableCount;
            }
        }
    }
}

void VulkanCommands::updateFences() {
    for (auto& wrapper : mStorage) {
        if (wrapper.cmdbuffer != VK_NULL_HANDLE) {
            VulkanCmdFence* fence = wrapper.fence.get();
            if (fence) {
                VkResult status = vkGetFenceStatus(mDevice, fence->fence);
                // This is either VK_SUCCESS, VK_NOT_READY, or VK_ERROR_DEVICE_LOST.
                fence->status.store(status, std::memory_order_relaxed);
            }
        }
    }
}

} // namespace filament::backend

#if defined(_MSC_VER)
#pragma warning( pop )
#endif
