/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include "VulkanStagePool.h"

#include "VulkanCommands.h"
#include "VulkanConstants.h"
#include "VulkanMemory.h"
#include "vulkan/utils/Conversion.h"
#include "vulkan/utils/Image.h"

#include <utils/Panic.h>

static constexpr uint32_t TIME_BEFORE_EVICTION = 3;

namespace filament::backend {

namespace {

// Note: these are temporary values, they will be configurable.
static constexpr uint32_t MAX_EMPTY_STAGES_TO_RETAIN = 1;
constexpr uint32_t STAGE_SIZE = 1048576;

uint32_t alignValue(uint32_t valueToAlign, uint32_t alignment) {
    if (alignment == 0) {
        return valueToAlign;
    }

    uint32_t remainder = valueToAlign % alignment;
    return remainder == 0 ? valueToAlign : valueToAlign + (alignment - remainder);
}

}// namespace

fvkmemory::resource_ptr<VulkanStage::Segment> VulkanStage::acquireSegment(
        fvkmemory::ResourceManager* resManager, uint32_t segmentOffset, uint32_t numBytes) {
    auto segment = fvkmemory::resource_ptr<Segment>::construct(resManager, this, numBytes,
            segmentOffset, [this](uint32_t offset) { mSegments.erase(offset); });
    mSegments.insert({ segmentOffset, segment.get() });
    mCurrentOffset = segmentOffset + numBytes;
    return segment;
}

VulkanStagePool::VulkanStagePool(VmaAllocator allocator, fvkmemory::ResourceManager* resManager,
        VulkanCommands* commands, const VkPhysicalDeviceLimits* deviceLimits)
    : mAllocator(allocator),
      mResManager(resManager),
      mCommands(commands),
      mDeviceLimits(deviceLimits) {}

fvkmemory::resource_ptr<VulkanStage::Segment> VulkanStagePool::acquireStage(uint32_t numBytes,
        uint32_t alignment) {
    // Apply alignment to the byte count to ensure that, when we later flush
    // data written by the host, we only flush the atoms that we modified, and
    // no adjacent atoms.
    numBytes = alignToNonCoherentAtomSize(numBytes);

    // First check if a stage segment exists whose capacity is greater than or
    // equal to the requested size.
    auto iter = mStages.lower_bound(numBytes);

    // If we have potential buffer candidates, loop through and try to find one
    // that works with our alignment. Also, set the offset for the buffer.
    // If we have no buffer, create a new one.
    VulkanStage* pStage = nullptr;
    uint32_t segmentOffset = 0;
    while (iter != mStages.end()) {
        segmentOffset = alignValue(iter->second->currentOffset(), alignment);

        // Check for overflow + if there's space.
        if (segmentOffset >= iter->second->currentOffset() &&
                iter->second->capacity() - numBytes >= segmentOffset) {
            // This buffer has enough space.
            pStage = iter->second;
            mStages.erase(iter);
            break;
        }

        // This buffer wasn't suitable. Check the next larger one.
        ++iter;
    }

    if (!pStage) {
        pStage = allocateNewStage(std::max(numBytes, STAGE_SIZE));
    }

    // Note: this allocation updates `currentOffset` and `segments` within
    // the parent stage. When destroyed, it will update `segments`.
    fvkmemory::resource_ptr<VulkanStage::Segment> pSegment =
            pStage->acquireSegment(mResManager, segmentOffset, numBytes);

    // Update the stage's metadata, and reinsert it with the remaining segment
    // capacity.
    uint32_t spaceRemaining = pStage->capacity() - pStage->currentOffset();
    mStages.insert({ spaceRemaining, pStage });

    return pSegment;
}

uint32_t VulkanStagePool::alignToNonCoherentAtomSize(uint32_t bytes) {
    VkDeviceSize alignment = mDeviceLimits->nonCoherentAtomSize;
    return alignValue(bytes, alignment);
}

VulkanStage* VulkanStagePool::allocateNewStage(uint32_t capacity) {
    VkBufferCreateInfo bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = alignToNonCoherentAtomSize(capacity),
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    };
    VmaAllocationCreateInfo allocInfo { .usage = VMA_MEMORY_USAGE_CPU_ONLY };
    VkBuffer buffer;
    VmaAllocation memory;
    VkResult result =
            vmaCreateBuffer(mAllocator, &bufferInfo, &allocInfo, &buffer, &memory, nullptr);

#if FVK_ENABLED(FVK_DEBUG_STAGING_ALLOCATION)
    if (result != VK_SUCCESS) {
        FVK_LOGE << "Allocation error: " << result;
    } else {
        FVK_LOGD << "Allocated stage with hndl " << buffer;
    }
#endif

    void* pMapping = nullptr;
    if (result == VK_SUCCESS) {
        result = vmaMapMemory(mAllocator, memory, &pMapping);

#if FVK_ENABLED(FVK_DEBUG_STAGING_ALLOCATION)
        if (result != VK_SUCCESS) {
            FVK_LOGE << "Memory mapping erryr: " << result << utils::io::endl;
        }
#endif
    }

    return new VulkanStage(memory, buffer, capacity, pMapping);
}

void VulkanStagePool::destroyStage(VulkanStage const*&& stage) {
    assert(stage->isSafeToReset());  // Ensure all segments have been reset already.
    vmaUnmapMemory(mAllocator, stage->memory());
    vmaDestroyBuffer(mAllocator, stage->buffer(), stage->memory());
    delete stage;
}

fvkmemory::resource_ptr<VulkanStageImage::Resource> VulkanStagePool::acquireImage(
        PixelDataFormat format, PixelDataType type, uint32_t width, uint32_t height) {
    // Helper lambda so we can return stage images wrapped as resources that can
    // be held by command buffers until no longer needed.
    auto wrapAsResource = [this](VulkanStageImage* image) {
        auto recycleFn = [this](VulkanStageImage* image) {
            this->mFreeImages.insert(image);
        };
        return fvkmemory::resource_ptr<VulkanStageImage::Resource>::construct(
            this->mResManager, image, recycleFn);
    };

    const VkFormat vkformat = fvkutils::getVkFormat(format, type);
    for (auto stageImage : mFreeImages) {
        if (stageImage->format() == vkformat && stageImage->width() == width && stageImage->height() == height) {
            mFreeImages.erase(stageImage);
            stageImage->mLastAccessed = mCurrentFrame;
            return wrapAsResource(stageImage);
        }
    }

    const VkImageCreateInfo imageInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = vkformat,
        .extent = { width, height, 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_LINEAR,
        .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
    };

    const VmaAllocationCreateInfo allocInfo {
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_CPU_TO_GPU
    };

    VkImage image;
    VmaAllocation memory;
    const UTILS_UNUSED VkResult result = vmaCreateImage(mAllocator, &imageInfo, &allocInfo,
            &image, &memory, nullptr);

    assert_invariant(result == VK_SUCCESS);

    VkImageAspectFlags const aspectFlags = fvkutils::getImageAspect(vkformat);
    VkCommandBuffer const cmdbuffer = mCommands->get().buffer();

    VulkanStageImage* stageImage = new VulkanStageImage(
        vkformat, width, height, memory, image, mCurrentFrame);

    // We use VK_IMAGE_LAYOUT_GENERAL here because the spec says:
    // "Host access to image memory is only well-defined for linear images and for image
    // subresources of those images which are currently in either the
    // VK_IMAGE_LAYOUT_PREINITIALIZED or VK_IMAGE_LAYOUT_GENERAL layout. Calling
    // vkGetImageSubresourceLayout for a linear image returns a subresource layout mapping that is
    // valid for either of those image layouts."
    fvkutils::transitionLayout(cmdbuffer, {
            .image = stageImage->image(),
            .oldLayout = VulkanLayout::UNDEFINED,
            .newLayout = VulkanLayout::STAGING, // (= VK_IMAGE_LAYOUT_GENERAL)
            .subresources = { aspectFlags, 0, 1, 0, 1 },
        });

    return wrapAsResource(stageImage);
}

void VulkanStagePool::gc() noexcept {
    FVK_SYSTRACE_CONTEXT();
    FVK_SYSTRACE_START("stagepool::gc");

    // If this is one of the first few frames, return early to avoid wrapping unsigned integers.
    if (++mCurrentFrame <= TIME_BEFORE_EVICTION) {
        return;
    }
    const uint64_t evictionTime = mCurrentFrame - TIME_BEFORE_EVICTION;

    decltype(mStages) freeStages;
    freeStages.swap(mStages);
    uint8_t freeStageCount = 0;  // Assuming we'll never have > 255 free stages
    for (auto& pair : freeStages) {
        // First, find any stages that have no segments within them.
        if (pair.second->isSafeToReset()) {
            if (++freeStageCount > MAX_EMPTY_STAGES_TO_RETAIN) {
#if FVK_ENABLED(FVK_DEBUG_STAGING_ALLOCATION)
                FVK_LOGD << "Destroying a staging buffer with hndl " << pair.second->buffer()
                         << utils::io::endl;
#endif
                destroyStage(std::move(pair.second));
                continue;
            }

#if FVK_ENABLED(FVK_DEBUG_STAGING_ALLOCATION)
            if (pair.first == 0) {
                FVK_LOGD << "Recycling an unused staging buffer with hndl " << pair.second->buffer()
                         << utils::io::endl;
            }
#endif

            // Note - this segment is free, make sure the structure is cleared
            // and reinsert it into our free stage list.
            pair.second->reset();
            mStages.insert({ pair.second->capacity(), pair.second });
        } else {
            mStages.insert(pair);
        }
    }

    // Destroy images that have not been used for several frames.
    decltype(mFreeImages) freeImages;
    freeImages.swap(mFreeImages);
    for (auto image : freeImages) {
        if (image->mLastAccessed < evictionTime) {
            vmaDestroyImage(mAllocator, image->image(), image->memory());
            delete image;
        } else {
            mFreeImages.insert(image);
        }
    }

    FVK_SYSTRACE_END();
}

void VulkanStagePool::terminate() noexcept {
    for (auto& pair : mStages) {
        destroyStage(std::move(pair.second));
    }
    mStages.clear();

    for (auto image : mFreeImages) {
        vmaDestroyImage(mAllocator, image->image(), image->memory());
        delete image;
    }
    mFreeImages.clear();
}

} // namespace filament::backend
