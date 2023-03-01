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
#include "VulkanHandles.h"
#include "VulkanTaskHandler.h"
#include "VulkanTexture.h"

#include <utils/Log.h>

using namespace bluevk;

namespace filament::backend {

VulkanReadPixels::~VulkanReadPixels() noexcept {
    assert_invariant(mDevice != VK_NULL_HANDLE);
    if (mCommandPool == VK_NULL_HANDLE) {
        return;
    }
    vkDestroyCommandPool(mDevice, mCommandPool, VKALLOC);
}

void VulkanReadPixels::initialize(VkDevice device) {
    mDevice = device;
}

void VulkanReadPixels::run(VulkanRenderTarget const* srcTarget, uint32_t const x, uint32_t const y,
        uint32_t const width, uint32_t const height, uint32_t const graphicsQueueFamilyIndex,
        PixelBufferDescriptor&& pbd, VulkanTaskHandler& taskHandler,
        SelecteMemoryFunction const& selectMemoryFunc,
        OnReadCompleteFunction const& readCompleteFunc) {
    assert_invariant(mDevice != VK_NULL_HANDLE);

    VkDevice device = mDevice;

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
    VkCommandPool cmdpool = mCommandPool;

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

    VkMemoryRequirements memReqs;
    VkDeviceMemory stagingMemory;
    vkGetImageMemoryRequirements(device, stagingImage, &memReqs);
    VkMemoryAllocateInfo const allocInfo = {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memReqs.size,
            .memoryTypeIndex = selectMemoryFunc(memReqs.memoryTypeBits,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
                            | VK_MEMORY_PROPERTY_HOST_CACHED_BIT)};

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

    transitionImageLayout(cmdbuffer, {
            .image = stagingImage,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_GENERAL,
            .subresources = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
            },
            .srcStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            .srcAccessMask = 0,
            .dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT,
            .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
    });

    VulkanAttachment const srcAttachment = srcTarget->getColor(0);

    VkImageCopy const imageCopyRegion = {
            .srcSubresource = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = srcAttachment.level,
                    .baseArrayLayer = srcAttachment.layer,
                    .layerCount = 1,
            },
            .srcOffset = {
                    .x = (int32_t) x,
                    .y = (int32_t) (srcTarget->getExtent().height - (height + y)),
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

    // Transition the source image layout (which might be the swap chain)
    VkImageSubresourceRange const srcRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = srcAttachment.level,
            .levelCount = 1,
            .baseArrayLayer = srcAttachment.layer,
            .layerCount = 1,
    };

    srcTexture->transitionLayout(cmdbuffer, srcRange, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    // Perform the copy into the staging area. At this point we know that the src layout is
    // TRANSFER_SRC_OPTIMAL and the staging area is GENERAL.
    UTILS_UNUSED_IN_RELEASE VkExtent2D srcExtent = srcAttachment.getExtent2D();
    assert_invariant(imageCopyRegion.srcOffset.x + imageCopyRegion.extent.width <= srcExtent.width);
    assert_invariant(
            imageCopyRegion.srcOffset.y + imageCopyRegion.extent.height <= srcExtent.height);

    vkCmdCopyImage(cmdbuffer, srcAttachment.getImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            stagingImage, VK_IMAGE_LAYOUT_GENERAL, 1, &imageCopyRegion);

    // Restore the source image layout. Between driver API calls, color images are always kept in
    // UNDEFINED layout or in their "usage default" layout (see comment for getDefaultImageLayout).
    srcTexture->transitionLayout(cmdbuffer, srcRange,
            getDefaultImageLayout(TextureUsage::COLOR_ATTACHMENT));

    vkEndCommandBuffer(cmdbuffer);

    VkQueue queue;
    vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, &queue);

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
    VkFence fence;
    VkFenceCreateInfo const fenceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };
    vkCreateFence(device, &fenceCreateInfo, VKALLOC, &fence);
    vkQueueSubmit(queue, 1, &submitInfo, fence);

    auto* const pUserBuffer = new PixelBufferDescriptor(std::move(pbd));
    auto const waitTaskId = taskHandler.createTask(
            [device, width, height, swizzle, srcFormat, fence, stagingImage, stagingMemory, cmdpool,
                    cmdbuffer, pUserBuffer, readCompleteFunc,
                    &taskHandler](VulkanTaskHandler::TaskId taskId, void* data) mutable {
                PixelBufferDescriptor& p = *pUserBuffer;
                vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
                VkResult status = vkGetFenceStatus(device, fence);

                // Fence hasn't been reached. Try waiting again.
                if (status == VK_NOT_READY) {
                    taskHandler.post(taskId);
                    return;
                }

                // Need to abort the readPixels if the device is lost.
                if (status == VK_ERROR_DEVICE_LOST) {
                    utils::slog.e << "Device lost while in VulkanReadPixels::run"
                                  << utils::io::endl;
                    taskHandler.completed(taskId);

                    // Try to free the pbd anyway
                    readCompleteFunc(std::move(p));
                    delete pUserBuffer;
                    return;
                }

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
                    utils::slog.e << "Unsupported PixelDataFormat or PixelDataType"
                                  << utils::io::endl;
                }

                vkUnmapMemory(device, stagingMemory);
                vkDestroyImage(device, stagingImage, VKALLOC);
                vkFreeMemory(device, stagingMemory, VKALLOC);
                vkDestroyFence(device, fence, VKALLOC);
                vkFreeCommandBuffers(device, cmdpool, 1, &cmdbuffer);
                readCompleteFunc(std::move(p));
                delete pUserBuffer;

                taskHandler.completed(taskId);
            },
            nullptr);

    taskHandler.post(waitTaskId);
}

}// namespace filament::backend
