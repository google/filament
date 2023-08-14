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
#include "VulkanContext.h"

#include <utils/Log.h>
#include <utils/Panic.h>
#include <utils/debug.h>

using namespace bluevk;
using namespace utils;

namespace filament::backend {

using Timestamp = VulkanGroupMarkers::Timestamp;

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

void VulkanGroupMarkers::push(std::string const& marker, Timestamp start) noexcept {
    mMarkers.push(marker);
#if FILAMENT_VULKAN_VERBOSE
    mTimestamps.push(start.time_since_epoch().count() > 0.0
                                  ? start
                                  : std::chrono::high_resolution_clock::now());
#endif
}

std::pair<std::string, Timestamp> VulkanGroupMarkers::pop() noexcept {
    auto const marker = mMarkers.top();
    mMarkers.pop();

#if FILAMENT_VULKAN_VERBOSE
    auto const topTimestamp = mTimestamps.top();
    mTimestamps.pop();
    return std::make_pair(marker, topTimestamp);
#else
    return std::make_pair(marker, Timestamp{});
#endif
}

std::pair<std::string, Timestamp> VulkanGroupMarkers::top() const {
    assert_invariant(!empty());
    auto const marker = mMarkers.top();
#if FILAMENT_VULKAN_VERBOSE
    auto const topTimestamp = mTimestamps.top();
    return std::make_pair(marker, topTimestamp);
#else
    return std::make_pair(marker, Timestamp{});
#endif
}

bool VulkanGroupMarkers::empty() const noexcept {
    return mMarkers.empty();
}

VulkanCommands::VulkanCommands(VkDevice device, VkQueue queue, uint32_t queueFamilyIndex,
        VulkanContext* context, VulkanResourceAllocator* allocator)
    : mDevice(device),
      mQueue(queue),
      mPool(createPool(mDevice, queueFamilyIndex)),
      mContext(context),
      mStorage(CAPACITY) {
    VkSemaphoreCreateInfo sci{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    for (auto& semaphore: mSubmissionSignals) {
        vkCreateSemaphore(mDevice, &sci, nullptr, &semaphore);
    }

    for (size_t i = 0; i < CAPACITY; ++i) {
        mStorage[i] = std::make_unique<VulkanCommandBuffer>(allocator);
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

VulkanCommandBuffer& VulkanCommands::get() {
    if (mCurrentCommandBufferIndex >= 0) {
        return *mStorage[mCurrentCommandBufferIndex].get();
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

    VulkanCommandBuffer* currentbuf = nullptr;
    // Find an available slot.
    for (size_t i = 0; i < CAPACITY; ++i) {
        auto wrapper = mStorage[i].get();
        if (wrapper->cmdbuffer == VK_NULL_HANDLE) {
            mCurrentCommandBufferIndex = static_cast<int8_t>(i);
            currentbuf = wrapper;
            break;
        }
    }

    assert_invariant(currentbuf);
    --mAvailableCount;

    // Create the low-level command buffer.
    const VkCommandBufferAllocateInfo allocateInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = mPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    vkAllocateCommandBuffers(mDevice, &allocateInfo, &currentbuf->cmdbuffer);

    // Note that the fence wrapper uses shared_ptr because a DriverAPI fence can also have ownership
    // over it.  The destruction of the low-level fence occurs either in VulkanCommands::gc(), or in
    // VulkanDriver::destroyFence(), both of which are safe spots.
    currentbuf->fence = std::make_shared<VulkanCmdFence>(mDevice);

    // Begin writing into the command buffer.
    const VkCommandBufferBeginInfo binfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(currentbuf->cmdbuffer, &binfo);

    // Notify the observer that a new command buffer has been activated.
    if (mObserver) {
        mObserver->onCommandBuffer(*currentbuf);
    }

    // We push the current markers onto a temporary stack. This must be placed after currentbuf is
    // set to the new command buffer since pushGroupMarker also calls get().
    while (mCarriedOverMarkers && !mCarriedOverMarkers->empty()) {
        auto [marker, time] = mCarriedOverMarkers->pop();
        pushGroupMarker(marker.c_str(), time);
    }

    return *currentbuf;
}

bool VulkanCommands::flush() {
    // It's perfectly fine to call flush when no commands have been written.
    if (mCurrentCommandBufferIndex < 0) {
        return false;
    }

    int8_t const index = mCurrentCommandBufferIndex;
    VulkanCommandBuffer const* currentbuf = mStorage[index].get();
    VkSemaphore const renderingFinished = mSubmissionSignals[index];

    vkEndCommandBuffer(currentbuf->cmdbuffer);

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
        .pCommandBuffers = &currentbuf->cmdbuffer,
        .signalSemaphoreCount = 1u,
        .pSignalSemaphores = &renderingFinished,
    };

    if (mSubmissionSignal) {
        signals[submitInfo.waitSemaphoreCount++] = mSubmissionSignal;
    }

    if (mInjectedSignal) {
        signals[submitInfo.waitSemaphoreCount++] = mInjectedSignal;
    }

    // To address a validation warning.
    if (submitInfo.waitSemaphoreCount == 0) {
        submitInfo.pWaitSemaphores = VK_NULL_HANDLE;
    }

#if FILAMENT_VULKAN_VERBOSE
    slog.i << "Submitting cmdbuffer=" << currentbuf->cmdbuffer
        << " wait=(" << signals[0] << ", " << signals[1] << ") "
        << " signal=" << renderingFinished
        << io::endl;
#endif

    // Before actually submitting, we need to pop any leftover group markers.
    while (mGroupMarkers && !mGroupMarkers->empty()) {
        if (!mCarriedOverMarkers) {
            mCarriedOverMarkers = std::make_unique<VulkanGroupMarkers>();
        }
        auto const [marker, time] = mGroupMarkers->top();
        mCarriedOverMarkers->push(marker, time);
        // We still need to call through to vkCmdEndDebugUtilsLabelEXT.
        popGroupMarker();
    }

    auto& cmdfence = currentbuf->fence;
    std::unique_lock<utils::Mutex> lock(cmdfence->mutex);
    cmdfence->status.store(VK_NOT_READY);
    UTILS_UNUSED_IN_RELEASE VkResult result = vkQueueSubmit(mQueue, 1, &submitInfo, cmdfence->fence);
    cmdfence->condition.notify_all();
    lock.unlock();

#if FILAMENT_VULKAN_VERBOSE
    if (result != VK_SUCCESS) {
        utils::slog.d <<"Failed command buffer submission result: " << result << utils::io::endl;
    }
#endif
    assert_invariant(result == VK_SUCCESS);

    mSubmissionSignal = renderingFinished;
    mInjectedSignal = VK_NULL_HANDLE;
    mCurrentCommandBufferIndex = -1;
    return true;
}

VkSemaphore VulkanCommands::acquireFinishedSignal() {
    VkSemaphore semaphore = mSubmissionSignal;
    mSubmissionSignal = VK_NULL_HANDLE;
#if FILAMENT_VULKAN_VERBOSE
     slog.i << "Acquiring " << semaphore << " (e.g. for vkQueuePresentKHR)" << io::endl;
#endif
    return semaphore;
}

void VulkanCommands::injectDependency(VkSemaphore next) {
    assert_invariant(mInjectedSignal == VK_NULL_HANDLE);
    mInjectedSignal = next;
#if FILAMENT_VULKAN_VERBOSE
    slog.i << "Injecting " << next << " (e.g. due to vkAcquireNextImageKHR)" << io::endl;
#endif
}

void VulkanCommands::wait() {
    VkFence fences[CAPACITY];
    size_t count = 0;
    for (size_t i = 0; i < CAPACITY; i++) {
        auto wrapper = mStorage[i].get();
        if (wrapper->cmdbuffer != VK_NULL_HANDLE
                && mCurrentCommandBufferIndex != static_cast<int8_t>(i)) {
            fences[count++] = wrapper->fence->fence;
        }
    }
    if (count > 0) {
        vkWaitForFences(mDevice, count, fences, VK_TRUE, UINT64_MAX);
    }
}

void VulkanCommands::gc() {
    VkCommandBuffer buffers[CAPACITY];
    size_t count = 0;
    for (size_t i = 0; i < CAPACITY; i++) {
        auto wrapper = mStorage[i].get();
        if (wrapper->cmdbuffer == VK_NULL_HANDLE) {
            continue;
        }
        VkResult const result = vkWaitForFences(mDevice, 1, &wrapper->fence->fence, VK_TRUE, 0);
        if (result != VK_SUCCESS) {
            continue;
        }
        buffers[count++] = wrapper->cmdbuffer;
        wrapper->cmdbuffer = VK_NULL_HANDLE;
        wrapper->fence->status.store(VK_SUCCESS);
        wrapper->fence.reset();
        wrapper->clearResources();
        ++mAvailableCount;
    }
    if (count > 0) {
        vkFreeCommandBuffers(mDevice, mPool, count, buffers);
    }
}

void VulkanCommands::updateFences() {
    for (size_t i = 0; i < CAPACITY; i++) {
        auto wrapper = mStorage[i].get();
        if (wrapper->cmdbuffer != VK_NULL_HANDLE) {
            VulkanCmdFence* fence = wrapper->fence.get();
            if (fence) {
                VkResult status = vkGetFenceStatus(mDevice, fence->fence);
                // This is either VK_SUCCESS, VK_NOT_READY, or VK_ERROR_DEVICE_LOST.
                fence->status.store(status, std::memory_order_relaxed);
            }
        }
    }
}

void VulkanCommands::pushGroupMarker(char const* str, VulkanGroupMarkers::Timestamp timestamp) {
#if FILAMENT_VULKAN_VERBOSE
    // If the timestamp is not 0, then we are carrying over a marker across buffer submits.
    // If it is 0, then this is a normal marker push and we should just print debug line as usual.
    if (timestamp.time_since_epoch().count() == 0.0) {
        utils::slog.d << "----> " << str << utils::io::endl;
    }
#endif

    // TODO: Add group marker color to the Driver API
    const VkCommandBuffer cmdbuffer = get().cmdbuffer;

    if (!mGroupMarkers) {
        mGroupMarkers = std::make_unique<VulkanGroupMarkers>();
    }
    mGroupMarkers->push(str, timestamp);

    if (mContext->isDebugUtilsSupported()) {
        VkDebugUtilsLabelEXT labelInfo = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                .pLabelName = str,
                .color = {0, 1, 0, 1},
        };
        vkCmdBeginDebugUtilsLabelEXT(cmdbuffer, &labelInfo);
    } else if (mContext->isDebugMarkersSupported()) {
        VkDebugMarkerMarkerInfoEXT markerInfo = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
                .pMarkerName = str,
                .color = {0.0f, 1.0f, 0.0f, 1.0f},
        };
        vkCmdDebugMarkerBeginEXT(cmdbuffer, &markerInfo);
    }
}

void VulkanCommands::popGroupMarker() {
     assert_invariant(mGroupMarkers);

    if (!mGroupMarkers->empty()) {
        const VkCommandBuffer cmdbuffer = get().cmdbuffer;
        #if FILAMENT_VULKAN_VERBOSE
            auto const [marker, startTime] = mGroupMarkers->pop();
            auto const endTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diff = endTime - startTime;
            utils::slog.d << "<---- " << marker << " elapsed: " << (diff.count() * 1000) << " ms\n"
                          << utils::io::flush;
        #else
            mGroupMarkers->pop();
        #endif

        if (mContext->isDebugUtilsSupported()) {
            vkCmdEndDebugUtilsLabelEXT(cmdbuffer);
        } else if (mContext->isDebugMarkersSupported()) {
            vkCmdDebugMarkerEndEXT(cmdbuffer);
        }
    } else if (mCarriedOverMarkers && !mCarriedOverMarkers->empty()) {
        // It could be that pop is called between flush() and get() (new command buffer), in which
        // case the marker is in "carried over" state. We'd just remove that
        mCarriedOverMarkers->pop();
    }
}

void VulkanCommands::insertEventMarker(char const* string, uint32_t len) {
    VkCommandBuffer const cmdbuffer = get().cmdbuffer;
    if (mContext->isDebugUtilsSupported()) {
        VkDebugUtilsLabelEXT labelInfo = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                .pLabelName = string,
                .color = {1, 1, 0, 1},
        };
        vkCmdInsertDebugUtilsLabelEXT(cmdbuffer, &labelInfo);
    } else if (mContext->isDebugMarkersSupported()) {
        VkDebugMarkerMarkerInfoEXT markerInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
            .pMarkerName = string,
            .color = {0.0f, 1.0f, 0.0f, 1.0f},
        };
        vkCmdDebugMarkerInsertEXT(cmdbuffer, &markerInfo);
    }
}

std::string VulkanCommands::getTopGroupMarker() const {
    if (!mGroupMarkers || mGroupMarkers->empty()) {
        return "";
    }
    return std::get<0>(mGroupMarkers->top());
}

}// namespace filament::backend

#if defined(_MSC_VER)
#pragma warning( pop )
#endif
