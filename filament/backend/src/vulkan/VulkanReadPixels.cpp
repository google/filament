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
#include "VulkanContext.h"
#include "VulkanHandles.h"
#include "VulkanTexture.h"

#include "vulkan/utils/Conversion.h"  // getComponentType()
#include "vulkan/utils/Image.h"

#include <utils/compiler.h>
#include <utils/Log.h>
#include <utils/Mutex.h>

#include <algorithm>
#include <cstring>
#include <memory>

using namespace bluevk;

namespace filament::backend {

using TaskHandler = VulkanReadPixels::TaskHandler;
using Task = TaskHandler::Task;

TaskHandler::TaskHandler()
    : mShouldStop(false),
      mThread(&TaskHandler::loop, this) {}

TaskHandler::~TaskHandler() {
    // shutdown() is the expected path; this only catches the cases where the owner never got that
    // far (e.g. a failure while initializing VulkanReadPixels). Note that the pending tasks are
    // still invoked with `executed = false` by loop().
    if (mThread.joinable()) {
        stop();
    }
}

void TaskHandler::post(Task&& task) {
    {
        utils::LockGuard const lock(mTaskQueueMutex);
        assert_invariant(!mShouldStop);
        mTaskQueue.push(std::move(task));
    }
    mHasTaskCondition.notify_one();
}

void TaskHandler::drain() {
    utils::Mutex syncPointMutex;
    utils::Condition syncCondition;
    bool done = false;
    post([&syncPointMutex, &syncCondition, &done](bool) {
        utils::LockGuard const lock(syncPointMutex);
        done = true;
        syncCondition.notify_one();
    });

    utils::UniqueLock lock(syncPointMutex);
    syncCondition.wait(lock, [&done] { return done; });
}

void TaskHandler::stop() noexcept {
    {
        utils::LockGuard const lock(mTaskQueueMutex);
        mShouldStop = true;
    }
    mHasTaskCondition.notify_one();
    mThread.join();
}

void TaskHandler::shutdown() {
    stop();
    bool isEmpty = false;
    {
        utils::LockGuard const lock(mTaskQueueMutex);
        isEmpty = mTaskQueue.empty();
    }
    FILAMENT_CHECK_POSTCONDITION(isEmpty)
            << "ReadPixels handler has tasks in the queue after shutdown";
}

void TaskHandler::loop() {
    while (true) {
        utils::UniqueLock lock(mTaskQueueMutex);
        mHasTaskCondition.wait(lock, [this]() UTILS_NO_THREAD_SAFETY_ANALYSIS {
            return !mTaskQueue.empty() || mShouldStop;
        });
        if (mShouldStop) {
            break;
        }
        Task task = std::move(mTaskQueue.front());
        mTaskQueue.pop();
        lock.unlock();
        task(true);
    }

    // Clean-up: the tasks we did not run still own resources, so we need to give them a chance to
    // release them.
    while (true) {
        utils::UniqueLock lock(mTaskQueueMutex);
        if (mTaskQueue.empty()) {
            break;
        }
        Task task = std::move(mTaskQueue.front());
        mTaskQueue.pop();
        lock.unlock();
        task(false);
    }
}

VulkanReadPixels::VulkanReadPixels(VkDevice device, VulkanContext const& context,
        VkQueue graphicsQueue, uint32_t const graphicsQueueFamilyIndex,
        CleanUpPbdFunction&& cleanUpPbdFunc)
        : mDevice(device),
          mContext(context),
          mQueue(graphicsQueue),
          mGraphicsQueueFamilyIndex(graphicsQueueFamilyIndex),
          mCleanUpPbd(std::move(cleanUpPbdFunc)) {}

void VulkanReadPixels::terminate() noexcept {
    assert_invariant(mDevice != VK_NULL_HANDLE);
    if (!isActive()) {
        return;
    }

    // The handler thread must be done with the requests' resources before we can destroy them and
    // the pool the command buffers were allocated from.
    if (mTaskHandler) {
        mTaskHandler->shutdown();
        mTaskHandler.reset();
    }
    gc();
    // Every task retired its request above, so gc() collected all of them. Anything left would
    // outlive the ResourceManager and release its image reference far too late.
    assert_invariant(mInFlightRequests.empty());

    vkDestroyCommandPool(mDevice, mCommandPool, VKALLOC);
    mCommandPool = VK_NULL_HANDLE;
    mDevice = VK_NULL_HANDLE;
}

void VulkanReadPixels::retire(VkFence const fence) {
    utils::LockGuard const lock(mRetiredFencesMutex);
    mRetiredFences.push_back(fence);
}

void VulkanReadPixels::gc() {
    // This is called every tick, so keep the no-readPixels case to a single branch.
    if (UTILS_LIKELY(!isActive())) {
        return;
    }

    std::vector<VkFence> retired;
    {
        utils::LockGuard const lock(mRetiredFencesMutex);
        if (mRetiredFences.empty()) {
            return;
        }
        std::swap(retired, mRetiredFences);
    }
    // Collect without the lock held; the handler thread must not have to wait on us.
    for (VkFence const fence: retired) {
        auto const pos = std::find_if(mInFlightRequests.begin(), mInFlightRequests.end(),
                [fence](Request const& request) { return request.fence == fence; });
        // A retired fence always belongs to a request we are still holding.
        assert_invariant(pos != mInFlightRequests.end());
        if (UTILS_UNLIKELY(pos == mInFlightRequests.end())) {
            continue;
        }
        vkDestroyBuffer(mDevice, pos->stagingBuffer, VKALLOC);
        vkFreeMemory(mDevice, pos->stagingMemory, VKALLOC);
        vkDestroyFence(mDevice, pos->fence, VKALLOC);
        vkFreeCommandBuffers(mDevice, mCommandPool, 1, &pos->cmdbuffer);
        // This releases our reference to the source image, possibly destroying the VkImage. It is
        // only safe here because the handler thread waited on the fence before retiring, so the
        // copy we recorded is known to have completed.
        mInFlightRequests.erase(pos);
    }
}

void VulkanReadPixels::run(fvkmemory::resource_ptr<VulkanRenderTarget> srcTarget, uint32_t const x,
        uint32_t const y, uint32_t const width, uint32_t const height,
        PixelBufferDescriptor&& pbd) {
    bool const isDepthStencil = pbd.format == PixelDataFormat::DEPTH_COMPONENT ||
                         pbd.format == PixelDataFormat::DEPTH_STENCIL;
    VulkanAttachment const srcAttachment = isDepthStencil ? srcTarget->getDepthStencil() : srcTarget->getColor(0);
    run(srcAttachment.texture, srcAttachment.level, srcAttachment.layer, x, y, width, height,
            std::move(pbd));
}

void VulkanReadPixels::run(fvkmemory::resource_ptr<VulkanTexture> srcTexture, uint8_t level,
        uint16_t layer, uint32_t x, uint32_t y, uint32_t width, uint32_t height,
        PixelBufferDescriptor&& pbd) {
    assert_invariant(mDevice != VK_NULL_HANDLE);
    assert_invariant(srcTexture);

    // We always submit to the non-protected graphics queue (see mQueue), so the submission-order
    // guarantee we rely on below would not cover work recorded into VulkanCommands' protected pool,
    // which goes to a different queue. Reading back protected content is not something we support
    // (and defeats the purpose of protected memory), so we simply assume it never happens.
    assert_invariant(!srcTexture->getIsProtected());

    // We're about to allocate from the command pool, which is a good time to release the command
    // buffers (and the other resources) of the requests that completed since the last call.
    gc();

    VkDevice device = mDevice;

    if (!isActive()) {
        // First readPixels: create the command pool and start the handler thread.
        VkCommandPoolCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT |
                     VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
            .queueFamilyIndex = mGraphicsQueueFamilyIndex,
        };
        vkCreateCommandPool(device, &createInfo, VKALLOC, &mCommandPool);
        mTaskHandler = std::make_unique<TaskHandler>();
    }
    assert_invariant(mTaskHandler);

    VkFormat const srcFormat = srcTexture->getVkFormat();
    VkImageAspectFlags const aspectMask = fvkutils::getImageAspect(srcFormat);

    bool const swizzle
            = srcFormat == VK_FORMAT_B8G8R8A8_UNORM || srcFormat == VK_FORMAT_B8G8R8A8_SRGB;

    bool const isDepth =
            (aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) || (aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT);

    VkImageAspectFlags copyAspect = aspectMask;
    if (isDepth) {
        // If the user requested DEPTH_COMPONENT or DEPTH_STENCIL, we only extract the depth aspect
        // since Vulkan cannot copy interleaved depth/stencil data into a buffer via vkCmdCopyImageToBuffer.
        copyAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    }

    uint32_t componentCount = fvkutils::getComponentCount(srcFormat);
    PixelDataType componentType = fvkutils::getComponentType(srcFormat);

    if (isDepth) {
        // When extracting depth or stencil, Vulkan returns tightly packed 1-component data.
        componentCount = 1;
        if (srcFormat == VK_FORMAT_D16_UNORM) {
            componentType = PixelDataType::USHORT;
        } else if (copyAspect == VK_IMAGE_ASPECT_STENCIL_BIT) {
            componentType = PixelDataType::UBYTE;
        } else {
            componentType =
                    (srcFormat == VK_FORMAT_D32_SFLOAT || srcFormat == VK_FORMAT_D32_SFLOAT_S8_UINT)
                            ? PixelDataType::FLOAT
                            : PixelDataType::UINT;
        }
    }

    uint32_t bpp = 0;
    switch (componentType) {
        case PixelDataType::UBYTE:
        case PixelDataType::BYTE:
            bpp = 1;
            break;
        case PixelDataType::USHORT:
        case PixelDataType::SHORT:
        case PixelDataType::HALF:
        case PixelDataType::USHORT_565:
            bpp = 2;
            break;
        case PixelDataType::UINT:
        case PixelDataType::INT:
        case PixelDataType::FLOAT:
        case PixelDataType::UINT_10F_11F_11F_REV:
        case PixelDataType::UINT_2_10_10_10_REV:
            bpp = 4;
            break;
        case PixelDataType::COMPRESSED:
            bpp = 1; // Note: Compressed formats aren't fully supported for readPixels.
            break;
    }
    if (componentType != PixelDataType::UINT_10F_11F_11F_REV &&
        componentType != PixelDataType::USHORT_565 &&
        componentType != PixelDataType::UINT_2_10_10_10_REV &&
        componentType != PixelDataType::COMPRESSED) {
        bpp *= componentCount;
    }

    uint32_t const samples = srcTexture->samples > 1 ? srcTexture->samples : 1;

    // Use a VkBuffer as the staging area for readback.
    // Using a buffer instead of a linearly tiled VkImage unifies the readback path and avoids
    // driver/validation layer issues since some implementations strictly prohibit linear tiling
    // for certain formats (such as depth/stencil).
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkMemoryRequirements memReqs;

    uint32_t const stagingSize = width * height * bpp * samples;
    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = stagingSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    };
    // TODO: we could now allocate from VulkanStagePool/VulkanBufferCache: the staging buffer is
    // created and destroyed on the backend thread, the handler thread only maps it.
    vkCreateBuffer(device, &bufferInfo, VKALLOC, &stagingBuffer);
    vkGetBufferMemoryRequirements(device, stagingBuffer, &memReqs);

#if FVK_ENABLED(FVK_DEBUG_READ_PIXELS)
    FVK_LOGD << "readPixels created staging area buffer"
             << " to copy from image=" << srcTexture->getVkImage()
             << " src-layout=" << srcTexture->getLayout(level, layer);
#endif

    VkDeviceMemory stagingMemory;

    uint32_t memoryTypeIndex = mContext.selectMemoryType(memReqs.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                    VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
    bool hostCachedStaging = true;

    // If VK_MEMORY_PROPERTY_HOST_CACHED_BIT is not supported, we try only
    // HOST_VISIBLE+HOST_COHERENT.  HOST_CACHED helps a lot with readpixels performance.
    if (memoryTypeIndex >= VK_MAX_MEMORY_TYPES) {
        memoryTypeIndex = mContext.selectMemoryType(memReqs.memoryTypeBits,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        hostCachedStaging = false;
        FVK_LOGW << "readPixels: VK_MEMORY_PROPERTY_HOST_CACHED_BIT is not available; "
                    "reshaping through a cached bounce copy";
    }

    FILAMENT_CHECK_POSTCONDITION(memoryTypeIndex < VK_MAX_MEMORY_TYPES)
            << "VulkanReadPixels: unable to find a memory type that meets requirements.";

    VkMemoryAllocateInfo const allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReqs.size,
        .memoryTypeIndex = memoryTypeIndex,
    };

    vkAllocateMemory(device, &allocInfo, VKALLOC, &stagingMemory);
    vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0);

    VkCommandBuffer cmdbuffer;
    VkCommandBufferAllocateInfo const allocateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = mCommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    vkAllocateCommandBuffers(device, &allocateInfo, &cmdbuffer);

    VkCommandBufferBeginInfo const binfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(cmdbuffer, &binfo);

    VkImageSubresourceRange const srcRange = {
        .aspectMask = aspectMask,
        .baseMipLevel = level,
        .levelCount = 1,
        .baseArrayLayer = layer,
        .layerCount = 1,
    };
    // This is what orders the copy below against the rendering that produced the image. We submit
    // to the same queue as the renderer (see mQueue), and a pipeline barrier recorded outside a
    // render pass has a first synchronization scope covering "all commands that occur earlier in
    // submission order" - an order that spans vkQueueSubmit boundaries on a single queue. So the
    // barrier reaches back into the previously submitted frame commands, and no semaphore is
    // needed: those are for ordering across queues.
    //   https://docs.vulkan.org/spec/latest/chapters/synchronization.html#synchronization-pipeline-barriers
    //   https://docs.vulkan.org/spec/latest/chapters/synchronization.html#synchronization-submission-order
    VulkanLayout const srcLayout = srcTexture->getLayout(level, layer);
    if (!srcTexture->transitionLayout(cmdbuffer, srcRange, VulkanLayout::TRANSFER_SRC)) {
        // transitionLayout() emits nothing when the image is already in TRANSFER_SRC, which would
        // leave this command buffer with no barrier at all, and therefore nothing tying the copy to
        // the earlier submissions. Emit the dependency explicitly.
        VkMemoryBarrier const barrier = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
        };
        vkCmdPipelineBarrier(cmdbuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &barrier, 0, nullptr, 0, nullptr);
    }

    uint32_t const mipHeight = std::max(1u, srcTexture->height >> level);

    VkBufferImageCopy const region = {
        .bufferOffset = 0,
        .bufferRowLength = width,
        .bufferImageHeight = height,
        .imageSubresource = {
            .aspectMask = copyAspect,
            .mipLevel = level,
            .baseArrayLayer = layer,
            .layerCount = 1,
        },
        .imageOffset = {
            .x = (int32_t)x,
            .y = (int32_t)(mipHeight - (height + y)),
            .z = 0,
        },
        .imageExtent = {
            .width = width,
            .height = height,
            .depth = 1,
        },
    };

    // Copy the specific aspect from the image into the tightly packed staging buffer.
    vkCmdCopyImageToBuffer(cmdbuffer, srcTexture->getVkImage(),
            fvkutils::getVkLayout(VulkanLayout::TRANSFER_SRC), stagingBuffer, 1, &region);

    // Restore the source image layout.
    srcTexture->transitionLayout(cmdbuffer, srcRange, srcLayout);

    vkEndCommandBuffer(cmdbuffer);

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
    vkQueueSubmit(mQueue, 1, &submitInfo, readCompleteFence);

    // The request keeps the source image alive until `gc()` collects it. It must be recorded before
    // the task is posted: the handler thread can retire the fence as soon as it picks the task up.
    mInFlightRequests.push_back(Request{
        .stagingBuffer = stagingBuffer,
        .stagingMemory = stagingMemory,
        .fence = readCompleteFence,
        .cmdbuffer = cmdbuffer,
        .srcTexture = std::move(srcTexture),
    });

    // Note that the task owns `pbd` and the right to retire the request: whether or not it gets to
    // run, it must give both back (to the client, and to us respectively).
    mTaskHandler->post([this, device, width, height, swizzle, bpp, componentType, componentCount,
                               hostCachedStaging, stagingMemory, fence = readCompleteFence,
                               pbd = std::move(pbd)](bool const executed) mutable {
        // The command buffer was submitted before this task was posted, so the GPU may still be
        // using the request's resources. We must wait for the fence even when the task was
        // cancelled (`executed == false`): otherwise we would hand the resources back to gc() while
        // they are still in flight, and it would destroy a command buffer in the pending state (and
        // release the last reference to the image being copied). The wait is bounded since the work
        // has already been submitted.
        VkResult const status = vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
        if (UTILS_LIKELY(executed && status == VK_SUCCESS)) {
            // Map memory so that we can start copying from it.
            uint8_t const* srcPixels;
            vkMapMemory(device, stagingMemory, 0, VK_WHOLE_SIZE, 0, (void**) &srcPixels);

            // If MSAA, MoltenVK returns samples in planar layout (Sample 0 is the first
            // width * height pixels). So we can simply ask DataReshaper to read width * height
            // elements with standard row pitch!
            int const rowPitch = width * bpp;

            // Without HOST_CACHED the mapping is uncached, and DataReshaper's per-texel loop turns
            // every 2-4 byte load into its own memory transaction. Bounce the mapped range into
            // cached heap memory with one bulk memcpy (which the CPU can issue as wide, streaming
            // loads) and reshape from there; reshape only ever reads the first sample plane.
            uint8_t const* reshapeSrc = srcPixels;
            std::unique_ptr<uint8_t[]> cachedCopy;
            if (!hostCachedStaging) {
                size_t const size = size_t(rowPitch) * height;
                cachedCopy = std::make_unique_for_overwrite<uint8_t[]>(size);
                memcpy(cachedCopy.get(), srcPixels, size);
                reshapeSrc = cachedCopy.get();
            }
            if (!DataReshaper::reshapeImage(&pbd, componentType, componentCount, reshapeSrc,
                        rowPitch, static_cast<int>(width), static_cast<int>(height), swizzle)) {
                FVK_LOGE << "Unsupported PixelDataFormat or PixelDataType";
            }

            vkUnmapMemory(device, stagingMemory);
        } else if (status != VK_SUCCESS) {
            FVK_LOGE << "Failed to wait for readPixels fence";
        } else {
            // The handler was shut down before it got to us, so we skip the readback: all that is
            // left to do is to release what the task owns (see below).
        }

        // We're done with the request's resources; the backend thread will release them in gc().
        retire(fence);

        mCleanUpPbd(std::move(pbd));
    });
}

void VulkanReadPixels::runUntilComplete() {
    if (UTILS_LIKELY(!isActive())) {
        return;
    }
    mTaskHandler->drain();
    gc();
}

}// namespace filament::backend
