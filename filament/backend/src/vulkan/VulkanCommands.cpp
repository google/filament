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

namespace {

#if FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS)
using Timestamp = VulkanGroupMarkers::Timestamp;
#endif

VkCommandBuffer createCommandBuffer(VkDevice device, VkCommandPool pool) {
    VkCommandBuffer cmdbuffer;
    // Create the low-level command buffer.
    VkCommandBufferAllocateInfo const allocateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    // The buffer allocated here will be implicitly reset when vkBeginCommandBuffer is called.
    // We don't need to deallocate since destroying the pool will free all of the buffers.
    vkAllocateCommandBuffers(device, &allocateInfo, &cmdbuffer);
    return cmdbuffer;
}

} // anonymous namespace

#if FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS)
void VulkanGroupMarkers::push(std::string const& marker, Timestamp start) noexcept {
    mMarkers.push_back({marker,
        start.time_since_epoch().count() > 0.0
        ? start
        : std::chrono::high_resolution_clock::now()});
}

std::pair<std::string, Timestamp> VulkanGroupMarkers::pop() noexcept {
    auto ret = mMarkers.back();
    mMarkers.pop_back();
    return ret;
}

std::pair<std::string, Timestamp> VulkanGroupMarkers::pop_bottom() noexcept {
    auto ret = mMarkers.front();
    mMarkers.pop_front();
    return ret;
}

std::pair<std::string, Timestamp> const& VulkanGroupMarkers::top() const {
    assert_invariant(!empty());
    return mMarkers.back();
}

bool VulkanGroupMarkers::empty() const noexcept {
    return mMarkers.empty();
}
#endif // FVK_DEBUG_GROUP_MARKERS

VulkanCommandBuffer::VulkanCommandBuffer(VulkanContext* context, VkDevice device, VkQueue queue,
        VkCommandPool pool, bool isProtected)
    : mContext(context),
      mMarkerCount(0),
      isProtected(isProtected),
      mDevice(device),
      mQueue(queue),
      mBuffer(createCommandBuffer(device, pool)),
      mFenceStatus(std::make_shared<VulkanCmdFence>(VK_INCOMPLETE)) {
    VkSemaphoreCreateInfo sci{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    vkCreateSemaphore(mDevice, &sci, VKALLOC, &mSubmission);

    VkFenceCreateInfo fenceCreateInfo{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    vkCreateFence(device, &fenceCreateInfo, VKALLOC, &mFence);
}

VulkanCommandBuffer::~VulkanCommandBuffer() {
    vkDestroySemaphore(mDevice, mSubmission, VKALLOC);
    vkDestroyFence(mDevice, mFence, VKALLOC);
}

void VulkanCommandBuffer::reset() noexcept {
    mMarkerCount = 0;
    mResources.clear();
    mWaitSemaphores.clear();

    // Internally we use the VK_INCOMPLETE status to mean "not yet submitted". When this fence
    // gets, gets submitted, its status changes to VK_NOT_READY. Finally, when the GPU actually
    // finishes executing the command buffer, the status changes to VK_SUCCESS.
    mFenceStatus = std::make_shared<VulkanCmdFence>(VK_INCOMPLETE);
    vkResetFences(mDevice, 1, &mFence);
}

void VulkanCommandBuffer::pushMarker(char const* marker) noexcept {
    if (mContext->isDebugUtilsSupported()) {
        VkDebugUtilsLabelEXT labelInfo = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                .pLabelName = marker,
                .color = {0, 1, 0, 1},
        };
        vkCmdBeginDebugUtilsLabelEXT(mBuffer, &labelInfo);
    } else if (mContext->isDebugMarkersSupported()) {
        VkDebugMarkerMarkerInfoEXT markerInfo = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
                .pMarkerName = marker,
                .color = {0.0f, 1.0f, 0.0f, 1.0f},
        };
        vkCmdDebugMarkerBeginEXT(mBuffer, &markerInfo);
    }
    mMarkerCount++;
}

void VulkanCommandBuffer::popMarker() noexcept{
    assert_invariant(mMarkerCount > 0);
    if (mContext->isDebugUtilsSupported()) {
        vkCmdEndDebugUtilsLabelEXT(mBuffer);
    } else if (mContext->isDebugMarkersSupported()) {
        vkCmdDebugMarkerEndEXT(mBuffer);
    }
    mMarkerCount--;
}

void VulkanCommandBuffer::insertEvent(char const* marker) noexcept {
    if (mContext->isDebugUtilsSupported()) {
        VkDebugUtilsLabelEXT labelInfo = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                .pLabelName = marker,
                .color = {1, 1, 0, 1},
        };
        vkCmdInsertDebugUtilsLabelEXT(mBuffer, &labelInfo);
    } else if (mContext->isDebugMarkersSupported()) {
        VkDebugMarkerMarkerInfoEXT markerInfo = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
                .pMarkerName = marker,
                .color = {0.0f, 1.0f, 0.0f, 1.0f},
        };
        vkCmdDebugMarkerInsertEXT(mBuffer, &markerInfo);
    }
}

void VulkanCommandBuffer::begin() noexcept {
    // Begin writing into the command buffer.
    VkCommandBufferBeginInfo const binfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(mBuffer, &binfo);
}

VkSemaphore VulkanCommandBuffer::submit() {
    while (mMarkerCount > 0) {
        popMarker();
    }

    vkEndCommandBuffer(mBuffer);

    VkPipelineStageFlags const waitDestStageMasks[2] = {
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    };

    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = mWaitSemaphores.size(),
        .pWaitSemaphores = mWaitSemaphores.data(),
        .pWaitDstStageMask = waitDestStageMasks,
        .commandBufferCount = 1u,
        .pCommandBuffers = &mBuffer,
        .signalSemaphoreCount = 1u,
        .pSignalSemaphores = &mSubmission,
    };
    // add submit protection if needed
    VkProtectedSubmitInfo protectedSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO,
        .protectedSubmit = VK_TRUE,
    };

    if (isProtected) {
        submitInfo.pNext = &protectedSubmitInfo;
    }

#if FVK_ENABLED(FVK_DEBUG_COMMAND_BUFFER)
    FVK_LOGI << "Submitting cmdbuffer=" << mBuffer
             << " wait=(";
    for (size_t s = 0, count = mWaitSemaphores.size(); s < count; ++s) {
        FVK_LOGI << mWaitSemaphores[s] << " ";
    }
    FVK_LOGI << ") "
             << " signal=" << mSubmission
             << " fence=" << mFence << utils::io::endl;
#endif

    mFenceStatus->setStatus(VK_NOT_READY);
    UTILS_UNUSED_IN_RELEASE VkResult result =
        vkQueueSubmit(mQueue, 1, &submitInfo, mFence);

#if FVK_ENABLED(FVK_DEBUG_COMMAND_BUFFER)
    if (result != VK_SUCCESS) {
        FVK_LOGD << "Failed command buffer submission result: " << result << utils::io::endl;
    }
#endif
    assert_invariant(result == VK_SUCCESS);
    mWaitSemaphores.clear();
    return mSubmission;
}

CommandBufferPool::CommandBufferPool(VulkanContext* context, VkDevice device, VkQueue queue,
        uint8_t queueFamilyIndex, bool isProtected)
    : mDevice(device),
      mRecording(INVALID) {
    VkCommandPoolCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT |
                 VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
                (isProtected ? VK_COMMAND_POOL_CREATE_PROTECTED_BIT : 0u),
        .queueFamilyIndex = queueFamilyIndex,
    };
    vkCreateCommandPool(device, &createInfo, VKALLOC, &mPool);

    for (size_t i = 0; i < CAPACITY; ++i) {
        mBuffers.emplace_back(
                std::make_unique<VulkanCommandBuffer>(context, device, queue, mPool, isProtected));
    }
}

CommandBufferPool::~CommandBufferPool() {
    wait();
    gc();
    vkDestroyCommandPool(mDevice, mPool, VKALLOC);
}

VulkanCommandBuffer& CommandBufferPool::getRecording() {
    if (isRecording()) {
        return *mBuffers[mRecording];
    }

    auto const findNext = [this]() {
        for (int8_t i = 0; i < CAPACITY; ++i) {
            if (!mSubmitted[i]) {
                return i;
            }
        }
        return INVALID;
    };

    while ((mRecording = findNext()) == INVALID) {
        wait();
        gc();
    }

    auto& recording = *mBuffers[mRecording];
    recording.begin();

#if FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS)
    if (mGroupMarkers) {
        std::unique_ptr<VulkanGroupMarkers> markers = std::make_unique<VulkanGroupMarkers>();
        while (!mGroupMarkers->empty()) {
            auto [marker, timestamp] = mGroupMarkers->pop_bottom();
            recording.pushMarker(marker.c_str());
            markers->push(marker, timestamp);
        }
        std::swap(mGroupMarkers, markers);
    }
#endif

    return recording;
}

void CommandBufferPool::gc() {
    ActiveBuffers reclaimed;
    mSubmitted.forEachSetBit([this,&reclaimed] (size_t index) {
        auto& buffer = mBuffers[index];
        if (buffer->getStatus() == VK_SUCCESS) {
            reclaimed.set(index, true);
            buffer->reset();
        }
    });
    mSubmitted &= ~reclaimed;
}

void CommandBufferPool::update() {
    mSubmitted.forEachSetBit([this] (size_t index) {
        auto& buffer = mBuffers[index];
        VkResult status = vkGetFenceStatus(mDevice, buffer->getVkFence());
        if (status == VK_SUCCESS) {
            buffer->setComplete();
        }
    });
}

VkSemaphore CommandBufferPool::flush() {
    // We're not recording right now.
    if (!isRecording()) {
        return VK_NULL_HANDLE;
    }
    auto submitSemaphore = mBuffers[mRecording]->submit();
    mSubmitted.set(mRecording, true);
    mRecording = INVALID;
    return submitSemaphore;
}

void CommandBufferPool::wait() {
    uint8_t count = 0;
    VkFence fences[CAPACITY];
    mSubmitted.forEachSetBit([this, &count, &fences] (size_t index) {
        fences[count++] = mBuffers[index]->getVkFence();
    });
    vkWaitForFences(mDevice, count, fences, VK_TRUE, UINT64_MAX);
    update();
}

void CommandBufferPool::waitFor(VkSemaphore previousAction) {
    if (!isRecording()) {
        return;
    }
    auto& recording = mBuffers[mRecording];
    recording->insertWait(previousAction);
}

#if FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS)
std::string CommandBufferPool::topMarker() const {
    if (!mGroupMarkers || mGroupMarkers->empty()) {
        return "";
    }
    return std::get<0>(mGroupMarkers->top());
}

void CommandBufferPool::pushMarker(char const* marker, VulkanGroupMarkers::Timestamp timestamp) {
    if (!mGroupMarkers) {
        mGroupMarkers = std::make_unique<VulkanGroupMarkers>();
    }
    mGroupMarkers->push(marker, timestamp);
    getRecording().pushMarker(marker);
}

std::pair<std::string, VulkanGroupMarkers::Timestamp> CommandBufferPool::popMarker() {
    assert_invariant(mGroupMarkers && !mGroupMarkers->empty());
    auto ret = mGroupMarkers->pop();

    // Note that if we're popping a marker while not recording, we would just pop the conceptual
    // stack of marker (i.e. mGroupMarkers) and not carry out the pop on the command buffer.
    if (isRecording()) {
        getRecording().popMarker();
    }
    return ret;
}

void CommandBufferPool::insertEvent(char const* marker) {
    getRecording().insertEvent(marker);
}
#endif // FVK_DEBUG_GROUP_MARKERS

VulkanCommands::VulkanCommands(VkDevice device, VkQueue queue, uint32_t queueFamilyIndex,
        VkQueue protectedQueue, uint32_t protectedQueueFamilyIndex, VulkanContext* context)
    : mDevice(device),
      mProtectedQueue(protectedQueue),
      mProtectedQueueFamilyIndex(protectedQueueFamilyIndex),
      mContext(context),
      mPool(std::make_unique<CommandBufferPool>(context, device, queue, queueFamilyIndex, false)) {}

void VulkanCommands::terminate() {
    mPool.reset();
    mProtectedPool.reset();
}

VulkanCommandBuffer& VulkanCommands::get() {
    auto& ret = mPool->getRecording();
    return ret;
}

VulkanCommandBuffer& VulkanCommands::getProtected() {
    assert_invariant(mProtectedQueue != VK_NULL_HANDLE);

    if (!mProtectedPool) {
        mProtectedPool = std::make_unique<CommandBufferPool>(mContext, mDevice, mProtectedQueue,
                mProtectedQueueFamilyIndex, true);
    }
    auto& ret = mProtectedPool->getRecording();
    return ret;
}

bool VulkanCommands::flush() {
    // It's possible to call flush and wait at "terminate", in which case, we'll just return.
    if (!mPool && !mProtectedPool) {
        return false;
    }

    VkSemaphore dependency = mInjectedDependency;
    VkSemaphore lastSubmit = mLastSubmit;
    bool hasFlushed = false;

    // Note that we've ordered it so that the non-protected commands are followed by the protected
    // commands.  This assumes that the protected commands will be that one doing the rendering into
    // the protected memory (i.e. protected render target).
    for (auto pool: {mPool.get(), mProtectedPool.get()}) {
        if (!pool || !pool->isRecording()) {
            continue;
        }
        if (dependency != VK_NULL_HANDLE) {
            pool->waitFor(dependency);
        }
        if (lastSubmit != VK_NULL_HANDLE) {
            pool->waitFor(lastSubmit);
            lastSubmit = VK_NULL_HANDLE;
        }
        dependency = pool->flush();
        hasFlushed = true;
    }

    if (hasFlushed) {
        mInjectedDependency = VK_NULL_HANDLE;
        mLastSubmit = dependency;
    }

    return true;
}

void VulkanCommands::wait() {
    // It's possible to call flush and wait at "terminate", in which case, we'll just return.
    if (!mPool && !mProtectedPool) {
        return;
    }

    FVK_SYSTRACE_CONTEXT();
    FVK_SYSTRACE_START("commands::wait");

    mPool->wait();
    if (mProtectedPool) {
        mProtectedPool->wait();
    }
    FVK_SYSTRACE_END();
}

void VulkanCommands::gc() {
    FVK_SYSTRACE_CONTEXT();
    FVK_SYSTRACE_START("commands::gc");

    mPool->gc();
    if (mProtectedPool) {
        mProtectedPool->gc();
    }
    FVK_SYSTRACE_END();
}

void VulkanCommands::updateFences() {
    mPool->update();
    if (mProtectedPool) {
        mProtectedPool->update();
    }
}

#if FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS)

void VulkanCommands::pushGroupMarker(char const* str, VulkanGroupMarkers::Timestamp timestamp) {
    mPool->pushMarker(str, timestamp);
    if (mProtectedPool) {
        mProtectedPool->pushMarker(str, timestamp);
    }
#if FVK_ENABLED(FVK_DEBUG_PRINT_GROUP_MARKERS)
    FVK_LOGD << "----> " << str << utils::io::endl;
#endif
}

void VulkanCommands::popGroupMarker() {

#if FVK_ENABLED(FVK_DEBUG_PRINT_GROUP_MARKERS)
    auto ret = mPool->popMarker();
    auto const& marker = ret.first;
    auto const& startTime = ret.second;
    auto const endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = endTime - startTime;
    FVK_LOGD << "<---- " << marker << " elapsed: " << (diff.count() * 1000) << " ms"
             << utils::io::endl;
#else
    mPool->popMarker();
#endif // FVK_DEBUG_PRINT_GROUP_MARKERS

    if (mProtectedPool) {
        mProtectedPool->popMarker();
    }
}

void VulkanCommands::insertEventMarker(char const* str, uint32_t len) {
    mPool->insertEvent(str);
    if (mProtectedPool) {
        mProtectedPool->insertEvent(str);
    }
}

std::string VulkanCommands::getTopGroupMarker() const {
    if (mProtectedPool) {
        return mProtectedPool->topMarker();
    }
    return mPool->topMarker();
}
#endif // FVK_DEBUG_GROUP_MARKERS

} // namespace filament::backend

#if defined(_MSC_VER)
#pragma warning( pop )
#endif
