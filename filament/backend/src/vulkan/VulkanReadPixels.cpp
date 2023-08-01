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

#include "VulkanReadPixels.h"

#include "DataReshaper.h"
#include "VulkanCommands.h"
#include "VulkanHandles.h"
#include "VulkanImageUtility.h"
#include "VulkanTexture.h"

#include <utils/Log.h>

using namespace bluevk;

namespace filament::backend {

using ImgUtil = VulkanImageUtility;
using TaskHandler = VulkanReadPixels::TaskHandler;
using WorkloadFunc = TaskHandler::WorkloadFunc;
using OnCompleteFunc = TaskHandler::OnCompleteFunc;

TaskHandler::TaskHandler()
    : mShouldStop(false),
      mThread(&TaskHandler::loop, this) {}

void TaskHandler::post(WorkloadFunc&& workload, OnCompleteFunc&& oncomplete) {
    assert_invariant(!mShouldStop);
    {
        std::unique_lock<std::mutex> lock(mTaskQueueMutex);
        mTaskQueue.push(std::make_pair(std::move(workload), std::move(oncomplete)));
    }
    mHasTaskCondition.notify_one();
}

void TaskHandler::drain() {
    assert_invariant(!mShouldStop);

    std::mutex syncPointMutex;
    std::condition_variable syncCondition;
    bool done = false;
    post([] {},
            [&syncPointMutex, &syncCondition, &done] {
                {
                    std::unique_lock<std::mutex> lock(syncPointMutex);
                    done = true;
                    syncCondition.notify_one();
                }
            });

    std::unique_lock<std::mutex> lock(syncPointMutex);
    syncCondition.wait(lock, [&done] { return done; });
}

void TaskHandler::shutdown() {
    {
        std::unique_lock<std::mutex> lock(mTaskQueueMutex);
        mShouldStop = true;
    }
    mHasTaskCondition.notify_one();
    mThread.join();
    ASSERT_POSTCONDITION(mTaskQueue.empty(),
            "ReadPixels handler has tasks in the queue after shutdown");
}

void TaskHandler::loop() {
    while (true) {
        std::unique_lock<std::mutex> lock(mTaskQueueMutex);
        mHasTaskCondition.wait(lock, [this] { return !mTaskQueue.empty() || mShouldStop; });
        if (mShouldStop) {
            break;
        }
        auto [workload, oncomplete] = mTaskQueue.front();
        mTaskQueue.pop();
        lock.unlock();
        workload();
        oncomplete();
    }

    // Clean-up: we still need to call oncomplete for clients to do clean-up.
    while (true) {
        std::unique_lock<std::mutex> lock(mTaskQueueMutex);
        if (mTaskQueue.empty()) {
            break;
        }
        auto [workload, oncomplete] = mTaskQueue.front();
        mTaskQueue.pop();
        lock.unlock();
        oncomplete();
    }
}

void VulkanReadPixels::terminate() noexcept {
    assert_invariant(mDevice != VK_NULL_HANDLE);
    if (mCommandPool == VK_NULL_HANDLE) {
        return;
    }
    vkDestroyCommandPool(mDevice, mCommandPool, VKALLOC);
    mDevice = VK_NULL_HANDLE;

    mTaskHandler->shutdown();
    mTaskHandler.reset();
}

VulkanReadPixels::VulkanReadPixels(VkDevice device)
    : mDevice(device) {}

void VulkanReadPixels::run(VulkanRenderTarget const* srcTarget, uint32_t const x, uint32_t const y,
        uint32_t const width, uint32_t const height, uint32_t const graphicsQueueFamilyIndex,
        PixelBufferDescriptor&& pbd, SelecteMemoryFunction const& selectMemoryFunc,
        OnReadCompleteFunction const& readCompleteFunc) {
    assert_invariant(mDevice != VK_NULL_HANDLE);

    VkDevice& device = mDevice;

    if (mCommandPool == VK_NULL_HANDLE) {
        // Create a command pool if one has not been created.
        VkCommandPoolCreateInfo createInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
                         | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                .queueFamilyIndex = graphicsQueueFamilyIndex,
        };
        vkCreateCommandPool(device, &createInfo, VKALLOC, &mCommandPool);
    }

    // We don't create a task handler (start a thread) unless readPixels is called.
    if (!mTaskHandler) {
        mTaskHandler = std::make_unique<TaskHandler>();
    }

    VkCommandPool& cmdpool = mCommandPool;

    VulkanTexture* srcTexture = srcTarget->getColor(0).texture;
    assert_invariant(srcTexture);
    VkFormat const srcFormat = srcTexture->getVkFormat();
    bool const swizzle
            = srcFormat == VK_FORMAT_B8G8R8A8_UNORM || srcFormat == VK_FORMAT_B8G8R8A8_SRGB;

    // Create a host visible, linearly tiled image as a staging area.
    VkImageCreateInfo const imageInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = srcFormat,
            .extent = {width, height, 1},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_LINEAR,
            .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VkImage stagingImage;
    vkCreateImage(device, &imageInfo, VKALLOC, &stagingImage);

#if FILAMENT_VULKAN_VERBOSE
    utils::slog.d << "readPixels created image=" << stagingImage
                  << " to copy from image=" << srcTexture->getVkImage()
                  << " src-layout=" << srcTexture->getLayout(0, 0) << utils::io::endl;
#endif

    VkMemoryRequirements memReqs;
    VkDeviceMemory stagingMemory;
    vkGetImageMemoryRequirements(device, stagingImage, &memReqs);
    VkMemoryAllocateInfo const allocInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memReqs.size,
            .memoryTypeIndex = selectMemoryFunc(memReqs.memoryTypeBits,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
                            | VK_MEMORY_PROPERTY_HOST_CACHED_BIT),
    };

    vkAllocateMemory(device, &allocInfo, VKALLOC, &stagingMemory);
    vkBindImageMemory(device, stagingImage, stagingMemory, 0);

    VkCommandBuffer cmdbuffer;
    VkCommandBufferAllocateInfo const allocateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = cmdpool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
    };
    vkAllocateCommandBuffers(device, &allocateInfo, &cmdbuffer);

    VkCommandBufferBeginInfo const binfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(cmdbuffer, &binfo);

    ImgUtil::transitionLayout(cmdbuffer, {
        .image = stagingImage,
        .oldLayout = VulkanLayout::UNDEFINED,
        .newLayout = VulkanLayout::TRANSFER_DST,
        .subresources = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    });

    VulkanAttachment const srcAttachment = srcTarget->getColor(0);
    const VkImageSubresourceRange srcRange
            = srcAttachment.getSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
    srcTexture->transitionLayout(cmdbuffer, srcRange, VulkanLayout::TRANSFER_SRC);

    VkImageCopy const imageCopyRegion = {
        .srcSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = srcAttachment.level,
            .baseArrayLayer = srcAttachment.layer,
            .layerCount = 1,
        },
        .srcOffset = {
            .x = (int32_t)x,
            .y = (int32_t)(srcTarget->getExtent().height - (height + y)),
        },
        .dstSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .layerCount = 1,
        },
        .extent = {
            .width = width,
            .height = height,
            .depth = 1,
        },
    };

    // Perform the copy into the staging area. At this point we know that the src
    // layout is TRANSFER_SRC_OPTIMAL and the staging area is GENERAL.
    UTILS_UNUSED_IN_RELEASE VkExtent2D srcExtent = srcAttachment.getExtent2D();
    assert_invariant(imageCopyRegion.srcOffset.x + imageCopyRegion.extent.width <= srcExtent.width);
    assert_invariant(
            imageCopyRegion.srcOffset.y + imageCopyRegion.extent.height <= srcExtent.height);

    vkCmdCopyImage(cmdbuffer, srcAttachment.getImage(),
            ImgUtil::getVkLayout(VulkanLayout::TRANSFER_SRC), stagingImage,
            ImgUtil::getVkLayout(VulkanLayout::TRANSFER_DST), 1, &imageCopyRegion);

    // Restore the source image layout.
    srcTexture->transitionLayout(cmdbuffer, srcRange, VulkanLayout::COLOR_ATTACHMENT);

    vkEndCommandBuffer(cmdbuffer);

    VkQueue queue;
    vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, &queue);
    VkFence readCompleteFence;
    VkFenceCreateInfo const fenceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };
    vkCreateFence(device, &fenceCreateInfo, VKALLOC, &readCompleteFence);
    VkSubmitInfo const submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = VK_NULL_HANDLE,
            .pWaitDstStageMask = VK_NULL_HANDLE,
            .commandBufferCount = 1,
            .pCommandBuffers = &cmdbuffer,
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = VK_NULL_HANDLE,
    };
    vkQueueSubmit(queue, 1, &submitInfo, readCompleteFence);

    auto* const pUserBuffer = new PixelBufferDescriptor(std::move(pbd));
    auto cleanPbdFunc = [pUserBuffer, readCompleteFunc]() {
        PixelBufferDescriptor& p = *pUserBuffer;
        readCompleteFunc(std::move(p));
        delete pUserBuffer;
    };
    auto waitFenceFunc = [device, width, height, swizzle, srcFormat, stagingImage, stagingMemory,
                                 cmdpool, cmdbuffer, pUserBuffer,
                                 fence = readCompleteFence]() mutable {
        VkResult status = vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
        // Fence hasn't been reached. Try waiting again.
        if (status != VK_SUCCESS) {
            utils::slog.e << "Failed to wait for readPixels fence" << utils::io::endl;
            return;
        }

        PixelBufferDescriptor& p = *pUserBuffer;
        VkImageSubresource subResource{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT};
        VkSubresourceLayout subResourceLayout;
        vkGetImageSubresourceLayout(device, stagingImage, &subResource, &subResourceLayout);

        // Map image memory so that we can start copying from it.
        uint8_t const* srcPixels;
        vkMapMemory(device, stagingMemory, 0, VK_WHOLE_SIZE, 0, (void**) &srcPixels);
        srcPixels += subResourceLayout.offset;

        if (!DataReshaper::reshapeImage(&p, getComponentType(srcFormat),
                    getComponentCount(srcFormat), srcPixels,
                    static_cast<int>(subResourceLayout.rowPitch), static_cast<int>(width),
                    static_cast<int>(height), swizzle)) {
            utils::slog.e << "Unsupported PixelDataFormat or PixelDataType" << utils::io::endl;
        }

        vkUnmapMemory(device, stagingMemory);
        vkDestroyImage(device, stagingImage, VKALLOC);
        vkFreeMemory(device, stagingMemory, VKALLOC);
        vkDestroyFence(device, fence, VKALLOC);
        vkFreeCommandBuffers(device, cmdpool, 1, &cmdbuffer);
    };
    mTaskHandler->post(std::move(waitFenceFunc), std::move(cleanPbdFunc));
}

void VulkanReadPixels::runUntilComplete() noexcept {
    if (!mTaskHandler) {
        return;
    }
    mTaskHandler->drain();
}

}// namespace filament::backend
