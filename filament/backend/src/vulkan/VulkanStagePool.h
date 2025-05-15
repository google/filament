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

#include "backend/DriverEnums.h"
#include "VulkanMemory.h"

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

    class Block {
    public:
        Block(VulkanStage* parentStage, uint32_t capacity, uint32_t offset)
            : mParentStage(parentStage),
              mCapacity(capacity),
              mOffset(offset) {
            mParentStage->mStageBlocks.insert({ mOffset, this });
        }

        ~Block() { mParentStage->mStageBlocks.erase(offset()); }

        // Should not be copying this around.
        Block(const Block& other) = delete;
        Block(Block&& other) = delete;
        Block& operator=(const Block& other) = delete;
        Block& operator=(Block&& other) = delete;

        inline VulkanStage* parentStage() const { return mParentStage; }
        inline VkBuffer buffer() const { return parentStage()->buffer(); }
        inline VmaAllocation memory() const { return parentStage()->memory(); }
        inline uint32_t capacity() const { return mCapacity; }
        inline uint32_t offset() const { return mOffset; }

        inline void* mapping() {
            return reinterpret_cast<void*>(
                    reinterpret_cast<char*>(mParentStage->mapping()) + offset());
        }

    private:
        VulkanStage* const mParentStage;
        const uint32_t mCapacity;
        const uint32_t mOffset;
    };

    inline VmaAllocation memory() const { return mMemory; }
    inline VkBuffer buffer() const { return mBuffer; }
    inline uint32_t capacity() const { return mCapacity; }
    inline void* mapping() const { return mMapping; }

    inline uint32_t& currentOffset() { return mCurrentOffset; }

    inline bool isSafeToReset() const { return mStageBlocks.empty(); }

    inline void reset() { mCurrentOffset = 0; }

    inline std::unique_ptr<Block> acquireBlock(uint32_t numBytes) {
        auto block = std::make_unique<Block>(this, numBytes, currentOffset());
        currentOffset() += numBytes;
        return block;
    }

private:
    const VmaAllocation mMemory;
    const VkBuffer mBuffer;
    const uint32_t mCapacity;

    void* mMapping;

    uint32_t mCurrentOffset = 0;

    // Maps the start offset of a vulkan stage block to the stage block,
    // for easy deletions later. This is managed by the blocks themselves, in an
    // RAII pattern, during construction and destruction.
    std::unordered_map<uint32_t, const Block*> mStageBlocks;
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
    VulkanStagePool(VmaAllocator allocator, VulkanCommands* commands);

    // Finds or creates a stage block whose capacity is at least the given
    // number of bytes. Internally, creates and manages and subdivides large
    // buffers so that we have less objects around that we have to keep track
    // of.
    // This function is NOT thread-safe.
    // Note: the unqiue pointer returned from this function should be kept alive
    // until the block is no longer in use. This is how we know when it's safe
    // to reuse the block of memory.
    std::unique_ptr<VulkanStage::Block> acquireStage(uint32_t numBytes);

    // Images have VK_IMAGE_LAYOUT_GENERAL and must not be transitioned to any other layout
    VulkanStageImage const* acquireImage(PixelDataFormat format, PixelDataType type,
            uint32_t width, uint32_t height);

    // Evicts old unused stages and bumps the current frame number.
    void gc() noexcept;

    // Destroys all unused stages and asserts that there are no stages currently in use.
    // This should be called while the context's VkDevice is still alive.
    void terminate() noexcept;

private:
    VmaAllocator mAllocator;
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
