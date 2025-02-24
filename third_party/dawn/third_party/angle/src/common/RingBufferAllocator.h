//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RingBufferAllocator.h:
//    Classes used to implement ring buffer allocators.
//

#ifndef COMMON_RING_BUFFER_ALLOCATOR_H_
#define COMMON_RING_BUFFER_ALLOCATOR_H_

#include "angleutils.h"
#include "common/SimpleMutex.h"
#include "common/debug.h"

#include <atomic>

namespace angle
{

static constexpr uint32_t kMinRingBufferAllocationCapacity = 1024;
static constexpr uint32_t kDefaultDecaySpeedFactor         = 10;

// Only called from RingBufferAllocator::allocate(). Other function may also change the fragment.
class RingBufferAllocateListener
{
  public:
    virtual void onRingBufferNewFragment() = 0;
    virtual void onRingBufferFragmentEnd() = 0;

  protected:
    ~RingBufferAllocateListener() = default;
};

class RingBufferAllocatorCheckPoint final
{
  public:
    void reset() { *this = {}; }
    bool valid() const { return (mReleasePtr != nullptr); }

  private:
    friend class RingBufferAllocator;
    uint64_t mBufferId   = 0;
    uint8_t *mReleasePtr = nullptr;
};

class RingBufferAllocatorBuffer final
{
  public:
    static constexpr uint32_t kBaseOffset = alignof(std::max_align_t);

    void resize(uint32_t size) { mStorage.resize(size + kBaseOffset); }
    uint8_t *data() { return mStorage.data() + kBaseOffset; }

    uint8_t *decClamped(uint8_t *ptr, uint32_t offset) const
    {
        ASSERT(ptr >= mStorage.data() + kBaseOffset && ptr <= mStorage.data() + mStorage.size());
        return ptr - std::min(offset, static_cast<uint32_t>(ptr - mStorage.data()));
    }

    uint64_t getId() const { return mId; }
    void resetId() { mId = 0; }
    void incrementId() { ++mId; }

    bool isEmpty() const { return mStorage.empty(); }

  private:
    uint64_t mId = 0;
    std::vector<uint8_t> mStorage;
};

class RingBufferAllocator final : angle::NonCopyable
{
  public:
    RingBufferAllocator() = default;
    RingBufferAllocator(RingBufferAllocator &&other);
    RingBufferAllocator &operator=(RingBufferAllocator &&other);

    void reset();
    bool valid() const { return (getPointer() != nullptr); }

    void setListener(RingBufferAllocateListener *listener);

    void setDecaySpeedFactor(uint32_t decaySpeedFactor);

    void setFragmentReserve(uint32_t reserve);

    uint8_t *allocate(uint32_t size)
    {
        ASSERT(valid());
        if (ANGLE_LIKELY(mFragmentEndR - mDataEnd >= static_cast<ptrdiff_t>(size)))
        {
            uint8_t *const result = mDataEnd;
            mDataEnd              = result + size;
            return result;
        }
        return allocateInNewFragment(size);
    }

    uint8_t *getPointer() const { return mDataEnd; }
    uint32_t getFragmentSize() const
    {
        ASSERT(mFragmentEnd >= mDataEnd);
        return static_cast<uint32_t>(mFragmentEnd - mDataEnd);
    }

    RingBufferAllocatorCheckPoint getReleaseCheckPoint() const;
    void release(const RingBufferAllocatorCheckPoint &checkPoint);

  private:
    void release(uint8_t *releasePtr);
    uint32_t getNumAllocatedInBuffer() const;
    void resetPointers();

    uint8_t *allocateInNewFragment(uint32_t size);
    void resize(uint32_t newCapacity);

    RingBufferAllocateListener *mListener = nullptr;

    std::vector<RingBufferAllocatorBuffer> mOldBuffers;
    RingBufferAllocatorBuffer mBuffer;
    uint8_t *mDataBegin       = nullptr;
    uint8_t *mDataEnd         = nullptr;
    uint8_t *mFragmentEnd     = nullptr;
    uint8_t *mFragmentEndR    = nullptr;
    uint32_t mFragmentReserve = 0;

    uint32_t mMinCapacity      = 0;
    uint32_t mCurrentCapacity  = 0;
    uint32_t mAllocationMargin = 0;

    // 1 - fastest decay speed.
    // 2 - 2x slower than fastest, and so on.
    uint32_t mDecaySpeedFactor = 0;
};

class SharedRingBufferAllocatorCheckPoint final : angle::NonCopyable
{
  public:
    void releaseAndUpdate(RingBufferAllocatorCheckPoint *newValue);

  private:
    friend class SharedRingBufferAllocator;
    RingBufferAllocatorCheckPoint pop();
    angle::SimpleMutex mMutex;
    RingBufferAllocatorCheckPoint mValue;

#if defined(ANGLE_ENABLE_ASSERTS)
    std::atomic<uint32_t> mRefCount{1};
#endif
};

class SharedRingBufferAllocator final : angle::NonCopyable
{
  public:
    SharedRingBufferAllocator();
    ~SharedRingBufferAllocator();

    RingBufferAllocator &get() { return mAllocator; }

    // Once shared - always shared
    bool isShared() const { return mSharedCP != nullptr; }

    // Once acquired must be released with releaseAndUpdate().
    SharedRingBufferAllocatorCheckPoint *acquireSharedCP();
    void releaseToSharedCP();

  private:
    RingBufferAllocator mAllocator;
    SharedRingBufferAllocatorCheckPoint *mSharedCP = nullptr;
};

}  // namespace angle

#endif  // COMMON_RING_BUFFER_ALLOCATOR_H_
