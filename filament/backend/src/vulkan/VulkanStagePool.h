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

#ifndef TNT_FILAMENT_BACKEND_VULKANSTAGEPOOL_H
#define TNT_FILAMENT_BACKEND_VULKANSTAGEPOOL_H

#include "VulkanMemory.h"
#include "backend/DriverEnums.h"
#include "vulkan/memory/Resource.h"
#include "vulkan/memory/ResourceManager.h"
#include "vulkan/memory/ResourcePointer.h"

#include <map>
#include <unordered_set>
#include <vector>

namespace filament::backend {

class VulkanCommands;

// Object representing a shared CPU-GPU staging area, which can be subdivided
// into smaller buffers as needed.
class VulkanStage {
public:
    VulkanStage(VmaAllocation memory, VkBuffer buffer, uint32_t capacity, void* mapping)
        : mMemory(memory),
          mBuffer(buffer),
          mCapacity(capacity),
          mMapping(mapping) {}

    ~VulkanStage() = default;
    VulkanStage(const VulkanStage& other) = delete;
    VulkanStage(VulkanStage&& other) = delete;
    VulkanStage& operator=(const VulkanStage& other) = delete;
    VulkanStage& operator=(VulkanStage&& other) = delete;

    class Segment : public fvkmemory::Resource {
    public:
        using OnRecycle = std::function<void(uint32_t offset)>;

        Segment(VulkanStage* parentStage, uint32_t capacity, uint32_t offset,
                OnRecycle&& onRecycleFn)
            : mParentStage(parentStage),
              mCapacity(capacity),
              mOffset(offset),
              mOnRecycleFn(onRecycleFn) {}

        ~Segment() {
            if (mOnRecycleFn) {
                mOnRecycleFn(offset());
            }
        }

        // Should not be copying this around.
        Segment(const Segment& other) = delete;
        Segment(Segment&& other) = delete;
        Segment& operator=(const Segment& other) = delete;
        Segment& operator=(Segment&& other) = delete;

        inline VulkanStage* parentStage() const { return mParentStage; }
        inline VkBuffer buffer() const { return parentStage()->buffer(); }
        inline VmaAllocation memory() const { return parentStage()->memory(); }
        inline uint32_t capacity() const { return mCapacity; }
        inline uint32_t offset() const { return mOffset; }

        inline void* mapping() const {
            return reinterpret_cast<void*>(
                    reinterpret_cast<char*>(mParentStage->mapping()) + offset());
        }

    private:
        // Ensure parent class can access the terminate method.
        friend class VulkanStage;

        VulkanStage* const mParentStage;
        const uint32_t mCapacity;
        const uint32_t mOffset;
        OnRecycle mOnRecycleFn;
    };

    inline VmaAllocation memory() const { return mMemory; }
    inline VkBuffer buffer() const { return mBuffer; }
    inline uint32_t capacity() const { return mCapacity; }
    inline void* mapping() const { return mMapping; }

    inline uint32_t currentOffset() { return mCurrentOffset; }

    inline bool isSafeToReset() const { return mSegments.empty(); }

    inline void reset() { mCurrentOffset = 0; }

    fvkmemory::resource_ptr<Segment> acquireSegment(fvkmemory::ResourceManager* resManager,
            uint32_t numBytes);

private:
    const VmaAllocation mMemory;
    const VkBuffer mBuffer;
    const uint32_t mCapacity;

    void* mMapping;

    uint32_t mCurrentOffset = 0;

    // Maps the start offset of a vulkan stage block to the stage block,
    // for easy deletions later. This is managed by the blocks themselves, in an
    // RAII pattern, during construction and destruction.
    std::unordered_map<uint32_t, Segment*> mSegments;
};

struct VulkanStageImage {
    VkFormat format;
    uint32_t width;
    uint32_t height;
    mutable uint64_t lastAccessed;
    VmaAllocation memory;
    VkImage image;
};

// Manages a pool of stages, periodically releasing stages that have been unused for a while.
// This class manages two types of host-mappable staging areas: buffer stages and image stages.
class VulkanStagePool {
public:
    VulkanStagePool(VmaAllocator allocator, fvkmemory::ResourceManager* resManager,
            VulkanCommands* commands);

    // Finds or creates a stage block whose capacity is at least the given
    // number of bytes. Internally, creates and manages and subdivides large
    // buffers so that we have less objects around that we have to keep track
    // of.
    // This function is NOT thread-safe.
    fvkmemory::resource_ptr<VulkanStage::Segment> acquireStage(uint32_t numBytes);

    // Images have VK_IMAGE_LAYOUT_GENERAL and must not be transitioned to any other layout
    VulkanStageImage const* acquireImage(PixelDataFormat format, PixelDataType type,
            uint32_t width, uint32_t height);

    // Evicts old unused stages and bumps the current frame number.
    void gc() noexcept;

    // Destroys all unused stages and asserts that there are no stages currently in use.
    // This should be called while the context's VkDevice is still alive.
    // Note: it is expected that all resources have been reclaimed before this
    // is called. It is also expected that this stage pool does not hold any
    // resource_ptrs, as this would lead to undefined behavior.
    void terminate() noexcept;

private:
    VmaAllocator mAllocator;
    fvkmemory::ResourceManager* mResManager;
    VulkanCommands* mCommands;

    // Allocates a new stage buffer, and optionally subdivides it into stage
    // blocks. If subdivideBlocks is true, predefined divisions will be used.
    // Otherwise, it's expected that capacity is defined to a value, and that
    // is the size that will be used for the buffer (as well as the only block
    // being created).
    VulkanStage* allocateNewStage(uint32_t capacity);

    // Performs any bookkeeping required to delete a VulkanStage object; namely,
    // unmapping memory, freeing the allocation, and deleting the VulkanStage
    // object. Note: takes an r-value because after this call, `stage` won't
    // exist.
    void destroyStage(VulkanStage const*&& stage);

    // Use an ordered multimap for quick (capacity => stage) lookups using lower_bound().
    std::multimap<uint32_t, VulkanStage*> mStages;

    std::unordered_set<VulkanStageImage const*> mFreeImages;
    std::vector<VulkanStageImage const*> mUsedImages;

    // Store the current "time" (really just a frame count) and LRU eviction parameters.
    uint64_t mCurrentFrame = 0;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANSTAGEPOOL_H
