//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMMON_MEMORYBUFFER_H_
#define COMMON_MEMORYBUFFER_H_

#include "common/Optional.h"
#include "common/angleutils.h"
#include "common/debug.h"

#include <stdint.h>
#include <cstddef>

namespace angle
{

class MemoryBuffer final : NonCopyable
{
  public:
    MemoryBuffer() = default;
    ~MemoryBuffer();

    MemoryBuffer(MemoryBuffer &&other);
    MemoryBuffer &operator=(MemoryBuffer &&other);

    // Destroy underlying memory
    void destroy();

    // Updates mSize to newSize. Updates mCapacity iff newSize > mCapacity
    [[nodiscard]] bool resize(size_t newSize);

    // Resets mSize to 0. Reserves memory and updates mCapacity iff newSize > mCapacity
    [[nodiscard]] bool clearAndReserve(size_t newSize);

    // Updates mCapacity iff newSize > mCapacity
    [[nodiscard]] bool reserve(size_t newSize);

    // Appends content from "other" MemoryBuffer
    [[nodiscard]] bool append(const MemoryBuffer &other);
    // Appends content from "[buffer, buffer + bufferSize)"
    [[nodiscard]] bool appendRaw(const uint8_t *buffer, const size_t bufferSize);

    // Sets size bound by capacity.
    void setSize(size_t size)
    {
        ASSERT(size <= mCapacity);
        mSize = size;
    }
    void setSizeToCapacity() { setSize(mCapacity); }

    // Invalidate current content
    void clear() { (void)resize(0); }

    size_t size() const { return mSize; }
    size_t capacity() const { return mCapacity; }
    bool empty() const { return mSize == 0; }

    const uint8_t *data() const { return mData; }
    uint8_t *data()
    {
        ASSERT(mData);
        return mData;
    }

    uint8_t &operator[](size_t pos)
    {
        ASSERT(mData && pos < mSize);
        return mData[pos];
    }
    const uint8_t &operator[](size_t pos) const
    {
        ASSERT(mData && pos < mSize);
        return mData[pos];
    }

    void fill(uint8_t datum);

    // Only used by unit tests
    // Validate total bytes allocated during a resize
    void assertTotalAllocatedBytes(size_t totalAllocatedBytes) const
    {
#if defined(ANGLE_ENABLE_ASSERTS)
        ASSERT(totalAllocatedBytes == mTotalAllocatedBytes);
#endif  // ANGLE_ENABLE_ASSERTS
    }
    // Validate total bytes copied during a resize
    void assertTotalCopiedBytes(size_t totalCopiedBytes) const
    {
#if defined(ANGLE_ENABLE_ASSERTS)
        ASSERT(totalCopiedBytes == mTotalCopiedBytes);
#endif  // ANGLE_ENABLE_ASSERTS
    }

  private:
    size_t mSize     = 0;
    size_t mCapacity = 0;
    uint8_t *mData   = nullptr;
#if defined(ANGLE_ENABLE_ASSERTS)
    size_t mTotalAllocatedBytes = 0;
    size_t mTotalCopiedBytes    = 0;
#endif  // ANGLE_ENABLE_ASSERTS
};

class ScratchBuffer final : NonCopyable
{
  public:
    ScratchBuffer();
    // If we request a scratch buffer requesting a smaller size this many times, release and
    // recreate the scratch buffer. This ensures we don't have a degenerate case where we are stuck
    // hogging memory.
    ScratchBuffer(uint32_t lifetime);
    ~ScratchBuffer();

    ScratchBuffer(ScratchBuffer &&other);
    ScratchBuffer &operator=(ScratchBuffer &&other);

    // Returns true with a memory buffer of the requested size, or false on failure.
    bool get(size_t requestedSize, MemoryBuffer **memoryBufferOut);

    // Same as get, but ensures new values are initialized to a fixed constant.
    bool getInitialized(size_t requestedSize, MemoryBuffer **memoryBufferOut, uint8_t initValue);

    // Ticks the release counter for the scratch buffer. Also done implicitly in get().
    void tick();

    void clear();

    MemoryBuffer *getMemoryBuffer() { return &mScratchMemory; }

  private:
    bool getImpl(size_t requestedSize, MemoryBuffer **memoryBufferOut, Optional<uint8_t> initValue);

    uint32_t mLifetime;
    uint32_t mResetCounter;
    MemoryBuffer mScratchMemory;
};

}  // namespace angle

#endif  // COMMON_MEMORYBUFFER_H_
