//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Suballocation.cpp:
//    Implements class methods for BufferBlock and Suballocation and other related classes
//

// #include "libANGLE/renderer/vulkan/vk_utils.h"

#include "libANGLE/renderer/vulkan/Suballocation.h"
#include "libANGLE/Context.h"
#include "libANGLE/renderer/vulkan/vk_mem_alloc_wrapper.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

namespace rx
{
namespace vk
{
// BufferBlock implementation.
BufferBlock::BufferBlock()
    : mMemoryPropertyFlags(0),
      mSize(0),
      mAllocatedBufferSize(0),
      mMemoryAllocationType(MemoryAllocationType::InvalidEnum),
      mMemoryTypeIndex(kInvalidMemoryTypeIndex),
      mMappedMemory(nullptr)
{}

BufferBlock::BufferBlock(BufferBlock &&other)
    : mVirtualBlock(std::move(other.mVirtualBlock)),
      mBuffer(std::move(other.mBuffer)),
      mDeviceMemory(std::move(other.mDeviceMemory)),
      mMemoryPropertyFlags(other.mMemoryPropertyFlags),
      mSize(other.mSize),
      mAllocatedBufferSize(other.mAllocatedBufferSize),
      mMemoryAllocationType(other.mMemoryAllocationType),
      mMemoryTypeIndex(other.mMemoryTypeIndex),
      mMappedMemory(other.mMappedMemory),
      mSerial(other.mSerial),
      mCountRemainsEmpty(0)
{}

BufferBlock &BufferBlock::operator=(BufferBlock &&other)
{
    std::swap(mVirtualBlock, other.mVirtualBlock);
    std::swap(mBuffer, other.mBuffer);
    std::swap(mDeviceMemory, other.mDeviceMemory);
    std::swap(mMemoryPropertyFlags, other.mMemoryPropertyFlags);
    std::swap(mSize, other.mSize);
    std::swap(mAllocatedBufferSize, other.mAllocatedBufferSize);
    std::swap(mMemoryAllocationType, other.mMemoryAllocationType);
    std::swap(mMemoryTypeIndex, other.mMemoryTypeIndex);
    std::swap(mMappedMemory, other.mMappedMemory);
    std::swap(mSerial, other.mSerial);
    std::swap(mCountRemainsEmpty, other.mCountRemainsEmpty);
    return *this;
}

BufferBlock::~BufferBlock()
{
    ASSERT(!mVirtualBlock.valid());
    ASSERT(!mBuffer.valid());
    ASSERT(!mDeviceMemory.valid());
    ASSERT(mDescriptorSetCacheManager.empty());
}

void BufferBlock::destroy(Renderer *renderer)
{
    VkDevice device = renderer->getDevice();

    mDescriptorSetCacheManager.destroyKeys(renderer);
    if (mMappedMemory)
    {
        unmap(device);
    }

    renderer->onMemoryDealloc(mMemoryAllocationType, mAllocatedBufferSize, mMemoryTypeIndex,
                              mDeviceMemory.getHandle());

    mVirtualBlock.destroy(device);
    mBuffer.destroy(device);
    mDeviceMemory.destroy(device);
}

VkResult BufferBlock::init(ErrorContext *context,
                           Buffer &buffer,
                           uint32_t memoryTypeIndex,
                           vma::VirtualBlockCreateFlags flags,
                           DeviceMemory &deviceMemory,
                           VkMemoryPropertyFlags memoryPropertyFlags,
                           VkDeviceSize size)
{
    Renderer *renderer = context->getRenderer();
    ASSERT(!mVirtualBlock.valid());
    ASSERT(!mBuffer.valid());
    ASSERT(!mDeviceMemory.valid());

    VK_RESULT_TRY(mVirtualBlock.init(renderer->getDevice(), flags, size));

    mBuffer               = std::move(buffer);
    mDeviceMemory         = std::move(deviceMemory);
    mMemoryPropertyFlags  = memoryPropertyFlags;
    mSize                 = size;
    mAllocatedBufferSize  = size;
    mMemoryAllocationType = MemoryAllocationType::Buffer;
    mMemoryTypeIndex      = memoryTypeIndex;
    mMappedMemory         = nullptr;
    mSerial               = renderer->getResourceSerialFactory().generateBufferSerial();

    return VK_SUCCESS;
}

void BufferBlock::initWithoutVirtualBlock(ErrorContext *context,
                                          Buffer &buffer,
                                          MemoryAllocationType memoryAllocationType,
                                          uint32_t memoryTypeIndex,
                                          DeviceMemory &deviceMemory,
                                          VkMemoryPropertyFlags memoryPropertyFlags,
                                          VkDeviceSize size,
                                          VkDeviceSize allocatedBufferSize)
{
    Renderer *renderer = context->getRenderer();
    ASSERT(!mVirtualBlock.valid());
    ASSERT(!mBuffer.valid());
    ASSERT(!mDeviceMemory.valid());

    mBuffer               = std::move(buffer);
    mDeviceMemory         = std::move(deviceMemory);
    mMemoryPropertyFlags  = memoryPropertyFlags;
    mSize                 = size;
    mAllocatedBufferSize  = allocatedBufferSize;
    mMemoryAllocationType = memoryAllocationType;
    mMemoryTypeIndex      = memoryTypeIndex;
    mMappedMemory         = nullptr;
    mSerial               = renderer->getResourceSerialFactory().generateBufferSerial();
}

VkResult BufferBlock::map(const VkDevice device)
{
    ASSERT(mMappedMemory == nullptr);
    return mDeviceMemory.map(device, 0, mSize, 0, &mMappedMemory);
}

void BufferBlock::unmap(const VkDevice device)
{
    mDeviceMemory.unmap(device);
    mMappedMemory = nullptr;
}

VkResult BufferBlock::allocate(VkDeviceSize size,
                               VkDeviceSize alignment,
                               VmaVirtualAllocation *allocationOut,
                               VkDeviceSize *offsetOut)
{
    std::unique_lock<angle::SimpleMutex> lock(mVirtualBlockMutex);
    mCountRemainsEmpty = 0;
    return mVirtualBlock.allocate(size, alignment, allocationOut, offsetOut);
}

void BufferBlock::free(VmaVirtualAllocation allocation, VkDeviceSize offset)
{
    std::unique_lock<angle::SimpleMutex> lock(mVirtualBlockMutex);
    mVirtualBlock.free(allocation, offset);
}

int32_t BufferBlock::getAndIncrementEmptyCounter()
{
    return ++mCountRemainsEmpty;
}

void BufferBlock::calculateStats(vma::StatInfo *pStatInfo) const
{
    std::unique_lock<angle::SimpleMutex> lock(mVirtualBlockMutex);
    mVirtualBlock.calculateStats(pStatInfo);
}

// BufferSuballocation implementation.
VkResult BufferSuballocation::map(ErrorContext *context)
{
    return mBufferBlock->map(context->getDevice());
}

// BufferSuballocationGarbage implementation.
bool BufferSuballocationGarbage::destroyIfComplete(Renderer *renderer)
{
    if (renderer->hasResourceUseFinished(mLifetime))
    {
        mBuffer.destroy(renderer->getDevice());
        mSuballocation.destroy(renderer);
        return true;
    }
    return false;
}

bool BufferSuballocationGarbage::hasResourceUseSubmitted(Renderer *renderer) const
{
    return renderer->hasResourceUseSubmitted(mLifetime);
}
}  // namespace vk
}  // namespace rx
