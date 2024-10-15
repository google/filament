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

#if FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS)
using Timestamp = VulkanGroupMarkers::Timestamp;
#endif

VulkanCmdFence::VulkanCmdFence(VkFence ifence)
    : fence(ifence) {
    // Internally we use the VK_INCOMPLETE status to mean "not yet submitted". When this fence gets
    // submitted, its status changes to VK_NOT_READY. Finally, when the GPU actually finishes
    // executing the command buffer, the status changes to VK_SUCCESS.
    status.store(VK_INCOMPLETE);
}

VulkanCommandBuffer::VulkanCommandBuffer(VkDevice device, VkCommandPool pool)
    : mPipeline(VK_NULL_HANDLE) {
    // Create the low-level command buffer.
    const VkCommandBufferAllocateInfo allocateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
    };

    // The buffer allocated here will be implicitly reset when vkBeginCommandBuffer is called.
    // We don't need to deallocate since destroying the pool will free all of the buffers.
    vkAllocateCommandBuffers(device, &allocateInfo, &mBuffer);
}

CommandBufferObserver::~CommandBufferObserver() {}

static VkCommandPool createPool(VkDevice device, uint32_t queueFamilyIndex) {
    VkCommandPoolCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
                     | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
            .queueFamilyIndex = queueFamilyIndex,
    };
    VkCommandPool pool;
    vkCreateCommandPool(device, &createInfo, VKALLOC, &pool);
    return pool;
}

#if FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS)
void VulkanGroupMarkers::push(std::string const& marker, Timestamp start) noexcept {
    mMarkers.push_back(marker);

#if FVK_ENABLED(FVK_DEBUG_PRINT_GROUP_MARKERS)
    mTimestamps.push_back(start.time_since_epoch().count() > 0.0
                                  ? start
                                  : std::chrono::high_resolution_clock::now());
#endif
}

std::pair<std::string, Timestamp> VulkanGroupMarkers::pop() noexcept {
    auto const marker = mMarkers.back();
    mMarkers.pop_back();

#if FVK_ENABLED(FVK_DEBUG_PRINT_GROUP_MARKERS)
    auto const timestamp = mTimestamps.back();
    mTimestamps.pop_back();
    return std::make_pair(marker, timestamp);
#else
    return std::make_pair(marker, Timestamp{});
#endif
}

std::pair<std::string, Timestamp> VulkanGroupMarkers::pop_bottom() noexcept {
    auto const marker = mMarkers.front();
    mMarkers.pop_front();

#if FVK_ENABLED(FVK_DEBUG_PRINT_GROUP_MARKERS)
    auto const timestamp = mTimestamps.front();
    mTimestamps.pop_front();
    return std::make_pair(marker, timestamp);
#else
    return std::make_pair(marker, Timestamp{});
#endif
}

std::pair<std::string, Timestamp> VulkanGroupMarkers::top() const {
    assert_invariant(!empty());
    auto const marker = mMarkers.back();
#if FVK_ENABLED(FVK_DEBUG_PRINT_GROUP_MARKERS)
    auto const topTimestamp = mTimestamps.front();
    return std::make_pair(marker, topTimestamp);
#else
    return std::make_pair(marker, Timestamp{});
#endif
}

bool VulkanGroupMarkers::empty() const noexcept {
    return mMarkers.empty();
}

#endif // FVK_DEBUG_GROUP_MARKERS

VulkanCommands::VulkanCommands(VkDevice device, VkQueue queue, uint32_t queueFamilyIndex,
        VulkanContext* context)
    : mDevice(device),
      mQueue(queue),
      mPool(createPool(mDevice, queueFamilyIndex)),
      mContext(context),
      mStorage(CAPACITY) {
    VkSemaphoreCreateInfo sci{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    for (auto& semaphore: mSubmissionSignals) {
        vkCreateSemaphore(mDevice, &sci, nullptr, &semaphore);
    }

    VkFenceCreateInfo fenceCreateInfo{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    for (auto& fence: mFences) {
        vkCreateFence(device, &fenceCreateInfo, VKALLOC, &fence);
    }

    for (size_t i = 0; i < CAPACITY; ++i) {
        mStorage[i] = std::make_unique<VulkanCommandBuffer>(mDevice, mPool);
    }

#if !FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS)
    (void) mContext;
#endif
}

void VulkanCommands::terminate() {
    wait();
    gc();
    vkDestroyCommandPool(mDevice, mPool, VKALLOC);
    for (VkSemaphore sema: mSubmissionSignals) {
        vkDestroySemaphore(mDevice, sema, VKALLOC);
    }
    for (VkFence fence: mFences) {
        vkDestroyFence(mDevice, fence, VKALLOC);
    }
}

VulkanCommandBuffer& VulkanCommands::get() {
    if (mCurrentCommandBufferIndex >= 0) {
        return *mStorage[mCurrentCommandBufferIndex].get();
    }

    // If we ran out of available command buffers, stall until one finishes. This is very rare.
    // It occurs only when Filament invokes commit() or endFrame() a large number of times without
    // presenting the swap chain or waiting on a fence.
    while (mAvailableBufferCount == 0) {
#if FVK_ENABLED(FVK_DEBUG_COMMAND_BUFFER)
        FVK_LOGI << "VulkanCommands has stalled. "
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
        if (wrapper->buffer() == VK_NULL_HANDLE) {
            mCurrentCommandBufferIndex = static_cast<int8_t>(i);
            currentbuf = wrapper;
            break;
        }
    }

    assert_invariant(currentbuf);
    mAvailableBufferCount--;

    // Note that the fence wrapper uses shared_ptr because a DriverAPI fence can also have ownership
    // over it.  The destruction of the low-level fence occurs either in VulkanCommands::gc(), or in
    // VulkanDriver::destroyFence(), both of which are safe spots.
    currentbuf->fence = std::make_shared<VulkanCmdFence>(mFences[mCurrentCommandBufferIndex]);

    // Begin writing into the command buffer.
    const VkCommandBufferBeginInfo binfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(currentbuf->buffer(), &binfo);

    // Notify the observer that a new command buffer has been activated.
    if (mObserver) {
        mObserver->onCommandBuffer(*currentbuf);
    }

#if FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS)
    // We push the current markers onto a temporary stack. This must be placed after currentbuf is
    // set to the new command buffer since pushGroupMarker also calls get().
    while (mCarriedOverMarkers && !mCarriedOverMarkers->empty()) {
        auto [marker, time] = mCarriedOverMarkers->pop();
        pushGroupMarker(marker.c_str(), time);
    }
#endif
    return *currentbuf;
}

bool VulkanCommands::flush() {
    // It's perfectly fine to call flush when no commands have been written.
    if (mCurrentCommandBufferIndex < 0) {
        return false;
    }

    // Before actually submitting, we need to pop any leftover group markers.
    // Note that this needs to occur before vkEndCommandBuffer.
#if FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS)
    while (mGroupMarkers && !mGroupMarkers->empty()) {
        if (!mCarriedOverMarkers) {
            mCarriedOverMarkers = std::make_unique<VulkanGroupMarkers>();
        }
        auto const [marker, time] = mGroupMarkers->top();
        mCarriedOverMarkers->push(marker, time);
        // We still need to call through to vkCmdEndDebugUtilsLabelEXT.
        popGroupMarker();
    }
#endif

    int8_t const index = mCurrentCommandBufferIndex;
    VulkanCommandBuffer const* currentbuf = mStorage[index].get();
    VkSemaphore const renderingFinished = mSubmissionSignals[index];

    vkEndCommandBuffer(currentbuf->buffer());

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
    uint32_t waitSemaphoreCount = 0;
    if (mSubmissionSignal) {
        signals[waitSemaphoreCount++] = mSubmissionSignal;
    }
    if (mInjectedSignal) {
        signals[waitSemaphoreCount++] = mInjectedSignal;
    }
    VkCommandBuffer const cmdbuffer = currentbuf->buffer();
    VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = waitSemaphoreCount,
            .pWaitSemaphores = waitSemaphoreCount > 0 ? signals : nullptr,
            .pWaitDstStageMask = waitDestStageMasks,
            .commandBufferCount = 1,
            .pCommandBuffers = &cmdbuffer,
            .signalSemaphoreCount = 1u,
            .pSignalSemaphores = &renderingFinished,
    };

#if FVK_ENABLED(FVK_DEBUG_COMMAND_BUFFER)
    FVK_LOGI << "Submitting cmdbuffer=" << cmdbuffer
           << " wait=(" << signals[0] << ", " << signals[1] << ") "
           << " signal=" << renderingFinished
           << " fence=" << currentbuf->fence->fence
           << utils::io::endl;
#endif

    auto& cmdfence = currentbuf->fence;
    UTILS_UNUSED_IN_RELEASE VkResult result = VK_SUCCESS;
    {
        auto scope = cmdfence->setValue(VK_NOT_READY);
        result = vkQueueSubmit(mQueue, 1, &submitInfo, cmdfence->getFence());
    }

#if FVK_ENABLED(FVK_DEBUG_COMMAND_BUFFER)
    if (result != VK_SUCCESS) {
        FVK_LOGD << "Failed command buffer submission result: " << result << utils::io::endl;
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
#if FVK_ENABLED(FVK_DEBUG_COMMAND_BUFFER)
    FVK_LOGI << "Acquiring " << semaphore << " (e.g. for vkQueuePresentKHR)" << io::endl;
#endif
    return semaphore;
}

void VulkanCommands::injectDependency(VkSemaphore next) {
    assert_invariant(mInjectedSignal == VK_NULL_HANDLE);
    mInjectedSignal = next;
#if FVK_ENABLED(FVK_DEBUG_COMMAND_BUFFER)
    FVK_LOGI << "Injecting " << next << " (e.g. due to vkAcquireNextImageKHR)" << io::endl;
#endif
}

void VulkanCommands::wait() {
    VkFence fences[CAPACITY];
    size_t count = 0;
    for (size_t i = 0; i < CAPACITY; i++) {
        auto wrapper = mStorage[i].get();
        if (wrapper->buffer() != VK_NULL_HANDLE
                && mCurrentCommandBufferIndex != static_cast<int8_t>(i)) {
            fences[count++] = wrapper->fence->getFence();
        }
    }
    if (count > 0) {
        vkWaitForFences(mDevice, count, fences, VK_TRUE, UINT64_MAX);
        updateFences();
    }
}

void VulkanCommands::gc() {
    FVK_SYSTRACE_CONTEXT();
    FVK_SYSTRACE_START("commands::gc");

    VkFence fences[CAPACITY];
    size_t count = 0;

    for (size_t i = 0; i < CAPACITY; i++) {
        auto wrapper = mStorage[i].get();
        if (wrapper->buffer() == VK_NULL_HANDLE) {
            continue;
        }
        auto const vkfence = wrapper->fence->getFence();
        VkResult const result = vkGetFenceStatus(mDevice, vkfence);
        if (result != VK_SUCCESS) {
            continue;
        }
        fences[count++] = vkfence;
        wrapper->fence->setValue(VK_SUCCESS);
        wrapper->reset();
        mAvailableBufferCount++;
    }

    if (count > 0) {
        vkResetFences(mDevice, count, fences);
    }
    FVK_SYSTRACE_END();
}

void VulkanCommands::updateFences() {
    for (size_t i = 0; i < CAPACITY; i++) {
        auto wrapper = mStorage[i].get();
        if (wrapper->buffer() != VK_NULL_HANDLE) {
            VulkanCmdFence* fence = wrapper->fence.get();
            if (fence) {
                VkResult status = vkGetFenceStatus(mDevice, fence->getFence());
                // This is either VK_SUCCESS, VK_NOT_READY, or VK_ERROR_DEVICE_LOST.
                fence->setValue(status);
            }
        }
    }
}

#if FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS)

void VulkanCommands::pushGroupMarker(char const* str, VulkanGroupMarkers::Timestamp timestamp) {
#if FVK_ENABLED(FVK_DEBUG_PRINT_GROUP_MARKERS)
    // If the timestamp is not 0, then we are carrying over a marker across buffer submits.
    // If it is 0, then this is a normal marker push and we should just print debug line as usual.
    if (timestamp.time_since_epoch().count() == 0.0) {
        FVK_LOGD << "----> " << str << utils::io::endl;
    }
#endif

    // TODO: Add group marker color to the Driver API
    VkCommandBuffer const cmdbuffer = get().buffer();

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
        VkCommandBuffer const cmdbuffer = get().buffer();
#if FVK_ENABLED(FVK_DEBUG_PRINT_GROUP_MARKERS)
        auto const [marker, startTime] = mGroupMarkers->pop();
        auto const endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = endTime - startTime;
        FVK_LOGD << "<---- " << marker << " elapsed: " << (diff.count() * 1000) << " ms"
                      << utils::io::endl;
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
        // case the marker is in "carried over" state, we'd just remove that. Since the
        // mCarriedOverMarkers is in the opposite order, we pop the bottom instead of the top.
        mCarriedOverMarkers->pop_bottom();
    }
}

void VulkanCommands::insertEventMarker(char const* string, uint32_t len) {
    VkCommandBuffer const cmdbuffer = get().buffer();
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
#endif // FVK_DEBUG_GROUP_MARKERS

} // namespace filament::backend

#if defined(_MSC_VER)
#pragma warning( pop )
#endif
