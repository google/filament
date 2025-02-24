//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "common/MemoryBuffer.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>

#include "common/debug.h"

namespace angle
{

// MemoryBuffer implementation.
MemoryBuffer::~MemoryBuffer()
{
    destroy();
}

void MemoryBuffer::destroy()
{
    if (mData)
    {
        free(mData);
        mData = nullptr;
    }

    mSize     = 0;
    mCapacity = 0;
#if defined(ANGLE_ENABLE_ASSERTS)
    mTotalAllocatedBytes = 0;
    mTotalCopiedBytes    = 0;
#endif  // ANGLE_ENABLE_ASSERTS
}

bool MemoryBuffer::resize(size_t newSize)
{
    // If new size is within mCapacity, update mSize and early-return
    if (newSize <= mCapacity)
    {
        mSize = newSize;
        return true;
    }

    // New size exceeds mCapacity, need to reallocate and copy over previous content.
    uint8_t *newMemory = static_cast<uint8_t *>(malloc(sizeof(uint8_t) * newSize));
    if (newMemory == nullptr)
    {
        return false;
    }

// Book keeping
#if defined(ANGLE_ENABLE_ASSERTS)
    mTotalAllocatedBytes += newSize;
#endif  // ANGLE_ENABLE_ASSERTS

    if (mData)
    {
        if (mSize > 0)
        {
            // Copy the intersection of the old data and the new data
            std::copy(mData, mData + mSize, newMemory);

// Book keeping
#if defined(ANGLE_ENABLE_ASSERTS)
            mTotalCopiedBytes += mSize;
#endif  // ANGLE_ENABLE_ASSERTS
        }
        free(mData);
    }

    mData     = newMemory;
    mSize     = newSize;
    mCapacity = newSize;

    return true;
}

bool MemoryBuffer::clearAndReserve(size_t newSize)
{
    clear();
    return reserve(newSize);
}

bool MemoryBuffer::reserve(size_t newSize)
{
    if (newSize <= mCapacity)
    {
        // Can already accommodate newSize, nothing to do.
        return true;
    }

    // Cache original size
    size_t originalSize = mSize;

    if (!resize(newSize))
    {
        return false;
    }

    // reserve(...) won't affect mSize, reset to original value.
    mSize = originalSize;

    return true;
}

bool MemoryBuffer::append(const MemoryBuffer &other)
{
    uint8_t *srcBuffer         = other.mData;
    const size_t srcBufferSize = other.mSize;

    // Handle the corner case where we are appending to self
    if (this == &other)
    {
        if (!reserve(mSize * 2))
        {
            return false;
        }
        srcBuffer = other.mData;
    }

    return appendRaw(srcBuffer, srcBufferSize);
}

bool MemoryBuffer::appendRaw(const uint8_t *buffer, const size_t bufferSize)
{
    ASSERT(buffer && bufferSize > 0);

    if (!reserve(mSize + bufferSize))
    {
        return false;
    }

    std::memcpy(mData + mSize, buffer, bufferSize);
    mSize += bufferSize;

    return true;
}

void MemoryBuffer::fill(uint8_t datum)
{
    if (!empty())
    {
        std::fill(mData, mData + mSize, datum);
    }
}

MemoryBuffer::MemoryBuffer(MemoryBuffer &&other) : MemoryBuffer()
{
    *this = std::move(other);
}

MemoryBuffer &MemoryBuffer::operator=(MemoryBuffer &&other)
{
    std::swap(mSize, other.mSize);
    std::swap(mCapacity, other.mCapacity);
    std::swap(mData, other.mData);
    return *this;
}

namespace
{
static constexpr uint32_t kDefaultScratchBufferLifetime = 1000u;

}  // anonymous namespace

// ScratchBuffer implementation.
ScratchBuffer::ScratchBuffer() : ScratchBuffer(kDefaultScratchBufferLifetime) {}

ScratchBuffer::ScratchBuffer(uint32_t lifetime) : mLifetime(lifetime), mResetCounter(lifetime) {}

ScratchBuffer::~ScratchBuffer() {}

ScratchBuffer::ScratchBuffer(ScratchBuffer &&other)
{
    *this = std::move(other);
}

ScratchBuffer &ScratchBuffer::operator=(ScratchBuffer &&other)
{
    std::swap(mLifetime, other.mLifetime);
    std::swap(mResetCounter, other.mResetCounter);
    std::swap(mScratchMemory, other.mScratchMemory);
    return *this;
}

bool ScratchBuffer::get(size_t requestedSize, MemoryBuffer **memoryBufferOut)
{
    return getImpl(requestedSize, memoryBufferOut, Optional<uint8_t>::Invalid());
}

bool ScratchBuffer::getInitialized(size_t requestedSize,
                                   MemoryBuffer **memoryBufferOut,
                                   uint8_t initValue)
{
    return getImpl(requestedSize, memoryBufferOut, Optional<uint8_t>(initValue));
}

bool ScratchBuffer::getImpl(size_t requestedSize,
                            MemoryBuffer **memoryBufferOut,
                            Optional<uint8_t> initValue)
{
    mScratchMemory.setSizeToCapacity();

    if (mScratchMemory.size() == requestedSize)
    {
        mResetCounter    = mLifetime;
        *memoryBufferOut = &mScratchMemory;
        return true;
    }

    if (mScratchMemory.size() > requestedSize)
    {
        tick();
    }

    if (mScratchMemory.size() < requestedSize)
    {
        if (!mScratchMemory.resize(requestedSize))
        {
            return false;
        }
        mResetCounter = mLifetime;
        if (initValue.valid())
        {
            mScratchMemory.fill(initValue.value());
        }
    }

    ASSERT(mScratchMemory.size() >= requestedSize);

    *memoryBufferOut = &mScratchMemory;
    return true;
}

void ScratchBuffer::tick()
{
    if (mResetCounter > 0)
    {
        --mResetCounter;
        if (mResetCounter == 0)
        {
            clear();
        }
    }
}

void ScratchBuffer::clear()
{
    mResetCounter = mLifetime;
    if (mScratchMemory.size() > 0)
    {
        mScratchMemory.clear();
    }
}

}  // namespace angle
