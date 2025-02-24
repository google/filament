//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// AllocatorHelperPool:
//    Manages the pool allocators used in the command buffers.
//

#ifndef LIBANGLE_RENDERER_VULKAN_ALLOCATORHELPERPOOL_H_
#define LIBANGLE_RENDERER_VULKAN_ALLOCATORHELPERPOOL_H_

#include "common/PoolAlloc.h"
#include "common/vulkan/vk_headers.h"
#include "libANGLE/renderer/vulkan/vk_command_buffer_utils.h"
#include "libANGLE/renderer/vulkan/vk_wrapper.h"

namespace rx
{
namespace vk
{
namespace priv
{
class SecondaryCommandBuffer;
}  // namespace priv

using DedicatedCommandMemoryAllocator = angle::PoolAllocator;

// Used in CommandBufferHelperCommon
class DedicatedCommandBlockAllocator
{
  public:
    DedicatedCommandBlockAllocator() = default;
    void resetAllocator();
    bool hasAllocatorLinks() const { return false; }

    static constexpr size_t kDefaultPoolAllocatorPageSize = 16 * 1024;
    void init()
    {
        mAllocator.initialize(kDefaultPoolAllocatorPageSize, 1);
        // Push a scope into the pool allocator so we can easily free and re-init on reset()
        mAllocator.push();
    }

    // Placeholder functions for attaching and detaching the allocator.
    void attachAllocator(DedicatedCommandMemoryAllocator *allocator) {}
    DedicatedCommandMemoryAllocator *detachAllocator(bool isCommandBufferEmpty) { return nullptr; }

    DedicatedCommandMemoryAllocator *getAllocator() { return &mAllocator; }

  private:
    // Using a pool allocator per CBH to avoid threading issues that occur w/ shared allocator
    // between multiple CBHs.
    DedicatedCommandMemoryAllocator mAllocator;
};

// Used in SecondaryCommandBuffer
class DedicatedCommandBlockPool final
{
  public:
    DedicatedCommandBlockPool()
        : mAllocator(nullptr),
          mCurrentWritePointer(nullptr),
          mCurrentBytesRemaining(0),
          mCommandBuffer(nullptr)
    {}

    static constexpr size_t kCommandHeaderSize = 4;
    using CommandHeaderIDType                  = uint16_t;
    // Make sure the size of command header ID type is less than total command header size.
    static_assert(sizeof(CommandHeaderIDType) < kCommandHeaderSize, "Check size of CommandHeader");
    // Pool Alloc uses 16kB pages w/ 16byte header = 16368bytes. To minimize waste
    //  using a 16368/12 = 1364. Also better perf than 1024 due to fewer block allocations
    static constexpr size_t kBlockSize = 1360;
    // Make sure block size is 8-byte aligned to avoid ASAN errors.
    static_assert((kBlockSize % 8) == 0, "Check kBlockSize alignment");

    void setCommandBuffer(priv::SecondaryCommandBuffer *commandBuffer)
    {
        mCommandBuffer = commandBuffer;
    }
    void resetCommandBuffer() { mCommandBuffer = nullptr; }

    void reset(CommandBufferCommandTracker *commandBufferTracker);

    // Initialize the SecondaryCommandBuffer by setting the allocator it will use
    angle::Result initialize(DedicatedCommandMemoryAllocator *allocator);

    bool valid() const { return mAllocator != nullptr; }
    bool empty() const;

    void getMemoryUsageStats(size_t *usedMemoryOut, size_t *allocatedMemoryOut) const;

    void onNewVariableSizedCommand(const size_t requiredSize,
                                   const size_t allocationSize,
                                   uint8_t **headerOut)
    {
        if (mCurrentBytesRemaining < requiredSize)
        {
            // variable size command can potentially exceed default cmd allocation blockSize
            if (requiredSize <= kBlockSize)
            {
                allocateNewBlock();
            }
            else
            {
                // Make sure allocation is 4-byte aligned
                const size_t alignedSize = roundUpPow2<size_t>(requiredSize, 4);
                ASSERT((alignedSize % 4) == 0);
                allocateNewBlock(alignedSize);
            }
        }

        *headerOut = updateHeaderAndAllocatorParams(allocationSize);
    }

    void onNewCommand(const size_t requiredSize, const size_t allocationSize, uint8_t **headerOut)
    {
        if (mCurrentBytesRemaining < requiredSize)
        {
            ASSERT(requiredSize <= kBlockSize);
            allocateNewBlock();
        }

        *headerOut = updateHeaderAndAllocatorParams(allocationSize);
    }

    // Placeholder functions
    void terminateLastCommandBlock() {}
    void attachAllocator(vk::DedicatedCommandMemoryAllocator *source) {}
    void detachAllocator(vk::DedicatedCommandMemoryAllocator *destination) {}

  private:
    void allocateNewBlock(size_t blockSize = kBlockSize);

    uint8_t *updateHeaderAndAllocatorParams(size_t allocationSize)
    {
        mCurrentBytesRemaining -= allocationSize;
        uint8_t *headerPointer = mCurrentWritePointer;
        mCurrentWritePointer += allocationSize;
        // Set next cmd header to Invalid (0) so cmd sequence will be terminated
        reinterpret_cast<CommandHeaderIDType &>(*mCurrentWritePointer) = 0;

        return headerPointer;
    }

    // Using a pool allocator per CBH to avoid threading issues that occur w/ shared allocator
    // between multiple CBHs.
    DedicatedCommandMemoryAllocator *mAllocator;
    uint8_t *mCurrentWritePointer;
    size_t mCurrentBytesRemaining;

    // Points to the parent command buffer.
    priv::SecondaryCommandBuffer *mCommandBuffer;
};

}  // namespace vk
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_ALLOCATORHELPERPOOL_H_
