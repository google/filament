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
#include "VulkanContext.h"
#include "VulkanHandles.h"
#include "VulkanTexture.h"

#include "backend/CallbackHandler.h"

#include <utils/Log.h>

#include <bluevk/BlueVK.h>

#include <queue>
#include <thread>

using namespace bluevk;

namespace filament::backend {

// This class starts a thread to wait on the fence that signals the end of the copying of pixels.
class VulkanReadPixelsImpl {
public:
    struct ReadInfo {
        uint32_t width;
        uint32_t height;
        bool swizzle;
        VkFormat srcFormat;
        VkFence fence;
        VkImage stagingImage;
        VkDeviceMemory stagingMemory;
        VkCommandBuffer cmdbuffer;
        PixelBufferDescriptor&& pbd;
        VulkanReadPixels::BufferCleanUpFunction cleanPbdFunc;

        ReadInfo(uint32_t width, uint32_t height, bool swizzle, VkFormat srcFormat, VkFence fence,
                VkImage stagingImage, VkDeviceMemory stagingMemory, VkCommandBuffer cmdbuffer,
                PixelBufferDescriptor&& pbd, VulkanReadPixels::BufferCleanUpFunction cleanPbdFunc)
            : width(width),
              height(height),
              swizzle(swizzle),
              srcFormat(srcFormat),
              fence(fence),
              stagingImage(stagingImage),
              stagingMemory(stagingMemory),
              cmdbuffer(cmdbuffer),
              pbd(std::move(pbd)),
              cleanPbdFunc(cleanPbdFunc) {}

        // The default constructor exists to enable storing pbd as an r-value field. We create a
        // dummy PBD as a placeholder.
        ReadInfo()
            : pbd(std::move(*(new PixelBufferDescriptor(
                    nullptr, 1, backend::PixelBufferDescriptor::PixelDataFormat::RGBA,
                    backend::PixelBufferDescriptor::PixelDataType::UBYTE,
                    [](void*, size_t, void*) {}, nullptr)))) {}

        // Since pbd is an r-value, we explicitly define the copy op.
        void copyFrom(ReadInfo& info) noexcept {
            width = info.width;
            height = info.height;
            swizzle = info.swizzle;
            srcFormat = info.srcFormat;
            fence = info.fence;
            stagingImage = info.stagingImage;
            stagingMemory = info.stagingMemory;
            cmdbuffer = info.cmdbuffer;
            pbd = std::move(info.pbd);
            cleanPbdFunc = info.cleanPbdFunc;
        }
    };

    VulkanReadPixelsImpl(VkDevice device, uint32_t queueFamilyIndex) noexcept;

    static void destroy(VulkanReadPixelsImpl* impl) noexcept;
    VkCommandBuffer createCommandBuffer() noexcept;
    void completeRead(ReadInfo& info) noexcept;

private:
    ~VulkanReadPixelsImpl();

    void loop() noexcept;
    void stop() noexcept;

    inline void freeReadResources(ReadInfo& readInfo);

    bool mStopLoop = false;
    std::mutex mQueueMutex;
    std::queue<ReadInfo> mReadInfoQueue;
    std::condition_variable mQueueCondition;
    std::thread mThread;

    VkDevice mDevice;
    VkCommandPool mCommandPool;
};

VulkanReadPixelsImpl::VulkanReadPixelsImpl(VkDevice device, uint32_t queueFamilyIndex) noexcept
    : mDevice(device) {
    // VulkanReadPixelsImpl creates a command pool on the backend thread. Allocation of command
    // buffers from this command pool is also expected to be on the backend thread.
    VkCommandPoolCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
                     | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
            .queueFamilyIndex = queueFamilyIndex,
    };
    vkCreateCommandPool(device, &createInfo, VKALLOC, &mCommandPool);

    // Start the readPixels thread
    mThread = std::thread([this] { this->loop(); });
}

VulkanReadPixelsImpl::~VulkanReadPixelsImpl() {
    // If there are left-over resources, let's try to free them.
    while (!mReadInfoQueue.empty()) {
        auto& info = mReadInfoQueue.front();
        freeReadResources(info);
        mReadInfoQueue.pop();
    }

    vkDestroyCommandPool(mDevice, mCommandPool, VKALLOC);
}

void VulkanReadPixelsImpl::stop() noexcept {
    {
        std::unique_lock<std::mutex> lock(mQueueMutex);
        mStopLoop = true;
    }
    mQueueCondition.notify_one();
    mThread.join();
}

void VulkanReadPixelsImpl::destroy(VulkanReadPixelsImpl* impl) noexcept {
    if (!impl) { return; }
    impl->stop();
    delete impl;
}

VkCommandBuffer VulkanReadPixelsImpl::createCommandBuffer() noexcept {
    VkCommandBuffer buffer;
    VkCommandBufferAllocateInfo const allocateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = mCommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
    };
    vkAllocateCommandBuffers(mDevice, &allocateInfo, &buffer);
    return buffer;
}

void VulkanReadPixelsImpl::completeRead(ReadInfo& info) noexcept {
    {
        std::unique_lock<std::mutex> lock(mQueueMutex);
        mReadInfoQueue.push(ReadInfo());
        mReadInfoQueue.back().copyFrom(info);
    }
    mQueueCondition.notify_one();
}

void VulkanReadPixelsImpl::freeReadResources(ReadInfo& info) {
    vkUnmapMemory(mDevice, info.stagingMemory);
    vkDestroyImage(mDevice, info.stagingImage, VKALLOC);
    vkFreeMemory(mDevice, info.stagingMemory, VKALLOC);
    vkDestroyFence(mDevice, info.fence, VKALLOC);
    vkFreeCommandBuffers(mDevice, mCommandPool, 1, &info.cmdbuffer);
    info.cleanPbdFunc(std::move(info.pbd));
}

void VulkanReadPixelsImpl::loop() noexcept {
    while (true) {
        ReadInfo info;
        {
            std::unique_lock<std::mutex> lock(mQueueMutex);
            mQueueCondition.wait(lock, [this] { return !mReadInfoQueue.empty() || mStopLoop; });
            if (mStopLoop) { return; }
            info.copyFrom(mReadInfoQueue.front());
            mReadInfoQueue.pop();
        }

        vkWaitForFences(mDevice, 1, &info.fence, VK_TRUE, UINT64_MAX);
        VkImageSubresource subResource{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT};
        VkSubresourceLayout subResourceLayout;
        vkGetImageSubresourceLayout(mDevice, info.stagingImage, &subResource, &subResourceLayout);

        // Map image memory so that we can start copying from it.
        const uint8_t* srcPixels;
        vkMapMemory(mDevice, info.stagingMemory, 0, VK_WHOLE_SIZE, 0, (void**) &srcPixels);
        srcPixels += subResourceLayout.offset;

        if (!DataReshaper::reshapeImage(&info.pbd, getComponentType(info.srcFormat),
                    getComponentCount(info.srcFormat), srcPixels, subResourceLayout.rowPitch,
                    info.width, info.height, info.swizzle)) {
            utils::slog.e << "Unsupported PixelDataFormat or PixelDataType" << utils::io::endl;
        }

        // It's ok (no need for external synchronization) to free these on the readPixels thread
        // because the backend thread no longer references/owns them.
        freeReadResources(info);
    }
}

// This method is called from the backend thread. Most of the setup of readPixels is done on the
// backend thread. We pass the allocated objects over to the readPixels thread to wait on the fence.
// Copying of the image bits and clean-up are done on the readPixels thread. We only start the
// readPixels thread (i.e. VulkanReadPixelsImpl) only if readPixels is called.
void VulkanReadPixels::run(VulkanRenderTarget const* srcTarget, VulkanContext const* context,
        BufferCleanUpFunction cleanFunc, uint32_t x, uint32_t y, uint32_t width, uint32_t height,
        PixelBufferDescriptor&& pbd) {
    VkDevice const device = context->device;
    if (!mImpl) {
        mImpl = new VulkanReadPixelsImpl(device, context->graphicsQueueFamilyIndex);
    }

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
    VkMemoryAllocateInfo allocInfo = {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memReqs.size,
            .memoryTypeIndex = context->selectMemoryType(memReqs.memoryTypeBits,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
                            | VK_MEMORY_PROPERTY_HOST_CACHED_BIT)};

    vkAllocateMemory(device, &allocInfo, VKALLOC, &stagingMemory);
    vkBindImageMemory(device, stagingImage, stagingMemory, 0);

    // Transition the staging image layout.
    const VkCommandBuffer cmdbuffer = mImpl->createCommandBuffer();

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

    VkImageCopy imageCopyRegion = {
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
    vkGetDeviceQueue(device, context->graphicsQueueFamilyIndex, 0, &queue);

    VkSubmitInfo submitInfo{
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

    VulkanReadPixelsImpl::ReadInfo readInfo{width, height, swizzle, srcFormat, fence, stagingImage,
            stagingMemory, cmdbuffer, std::move(pbd), cleanFunc};

    // At this point, we pass the ownership of all the device resources to the readPixels thread.
    // Once the read completes (fence is reached) and the image is copied to the client, then the
    // resources are freed.
    mImpl->completeRead(readInfo);
}

void VulkanReadPixels::cleanup() {
    VulkanReadPixelsImpl::destroy(mImpl);
    VulkanReadPixels::mImpl = nullptr;
}

}// namespace filament::backend
