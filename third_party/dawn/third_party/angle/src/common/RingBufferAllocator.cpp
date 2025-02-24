//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RingBufferAllocator.cpp:
//    Implements classes used for ring buffer allocators.
//

#include "common/RingBufferAllocator.h"

namespace angle
{

// RingBufferAllocator implementation.
RingBufferAllocator::RingBufferAllocator(RingBufferAllocator &&other)
{
    *this = std::move(other);
}

RingBufferAllocator &RingBufferAllocator::operator=(RingBufferAllocator &&other)
{
    mOldBuffers      = std::move(other.mOldBuffers);
    mBuffer          = std::move(other.mBuffer);
    mDataBegin       = other.mDataBegin;
    mDataEnd         = other.mDataEnd;
    mFragmentEnd     = other.mFragmentEnd;
    mFragmentEndR    = other.mFragmentEndR;
    mFragmentReserve = other.mFragmentReserve;

    mMinCapacity      = other.mMinCapacity;
    mCurrentCapacity  = other.mCurrentCapacity;
    mAllocationMargin = other.mAllocationMargin;
    mDecaySpeedFactor = other.mDecaySpeedFactor;

    ASSERT(other.mOldBuffers.size() == 0);
    ASSERT(other.mBuffer.isEmpty());
    other.mBuffer.resetId();
    other.mDataBegin       = nullptr;
    other.mDataEnd         = nullptr;
    other.mFragmentEnd     = nullptr;
    other.mFragmentEndR    = nullptr;
    other.mFragmentReserve = 0;

    other.mMinCapacity      = 0;
    other.mCurrentCapacity  = 0;
    other.mAllocationMargin = 0;
    other.mDecaySpeedFactor = 0;

    return *this;
}

void RingBufferAllocator::reset()
{
    mListener         = nullptr;
    mFragmentReserve  = 0;
    mDecaySpeedFactor = kDefaultDecaySpeedFactor;
    mMinCapacity      = kMinRingBufferAllocationCapacity;
    resize(mMinCapacity);
    mOldBuffers.clear();
}

void RingBufferAllocator::setListener(RingBufferAllocateListener *listener)
{
    ASSERT(!mListener || !listener);
    mListener = listener;
}

void RingBufferAllocator::setDecaySpeedFactor(uint32_t decaySpeedFactor)
{
    ASSERT(valid());
    mDecaySpeedFactor = std::max(decaySpeedFactor, 1u);
}

RingBufferAllocatorCheckPoint RingBufferAllocator::getReleaseCheckPoint() const
{
    ASSERT(valid());
    RingBufferAllocatorCheckPoint result;
    result.mBufferId   = mBuffer.getId();
    result.mReleasePtr = mDataEnd;
    return result;
}

void RingBufferAllocator::release(const RingBufferAllocatorCheckPoint &checkPoint)
{
    ASSERT(valid());
    ASSERT(checkPoint.valid());

    if (mOldBuffers.size() > 0)
    {
        // mOldBuffers are sorted by id
        int removeCount = 0;
        for (uint32_t i = 0;
             (i < mOldBuffers.size()) && (mOldBuffers[i].getId() < checkPoint.mBufferId); ++i)
        {
            ++removeCount;
        }
        mOldBuffers.erase(mOldBuffers.begin(), mOldBuffers.begin() + removeCount);
    }

    if (checkPoint.mBufferId == mBuffer.getId())
    {
        const uint32_t allocatedBefore = getNumAllocatedInBuffer();

        release(checkPoint.mReleasePtr);

        if (allocatedBefore >= mAllocationMargin)
        {
            if ((mCurrentCapacity > mMinCapacity) && (allocatedBefore * 6 <= mCurrentCapacity))
            {
                resize(std::max(allocatedBefore * 3, mMinCapacity));
            }
            else
            {
                mAllocationMargin = mCurrentCapacity;
            }
        }
        else
        {
            const uint64_t numReleased      = (allocatedBefore - getNumAllocatedInBuffer());
            const uint64_t distanceToMargin = (mAllocationMargin - allocatedBefore);
            mAllocationMargin -=
                std::max(static_cast<uint32_t>(numReleased * distanceToMargin / mAllocationMargin /
                                               mDecaySpeedFactor),
                         1u);
        }
    }
}

void RingBufferAllocator::setFragmentReserve(uint32_t reserve)
{
    ASSERT(valid());
    mFragmentReserve = reserve;
    mFragmentEndR    = mBuffer.decClamped(mFragmentEnd, mFragmentReserve);
}

uint8_t *RingBufferAllocator::allocateInNewFragment(uint32_t size)
{
    if (mListener)
    {
        mListener->onRingBufferFragmentEnd();
    }

    uint8_t *newFragmentBegin = nullptr;
    if (mFragmentEnd != mDataBegin)
    {
        newFragmentBegin               = mBuffer.data();
        uint8_t *const newFragmentEnd  = mDataBegin;
        uint8_t *const newFragmentEndR = mBuffer.decClamped(newFragmentEnd, mFragmentReserve);

        // It should wrap around only if it can allocate.
        if (newFragmentEndR - newFragmentBegin >= static_cast<ptrdiff_t>(size))
        {
            mDataEnd      = newFragmentBegin;
            mFragmentEnd  = newFragmentEnd;
            mFragmentEndR = newFragmentEndR;

            if (mListener)
            {
                mListener->onRingBufferNewFragment();
            }

            mDataEnd = newFragmentBegin + size;
            return newFragmentBegin;
        }
    }

    resize(std::max(mCurrentCapacity + mCurrentCapacity / 2, size + mFragmentReserve));

    if (mListener)
    {
        mListener->onRingBufferNewFragment();
    }

    ASSERT(mFragmentEndR - mDataEnd >= static_cast<ptrdiff_t>(size));
    newFragmentBegin = mDataEnd;
    mDataEnd         = newFragmentBegin + size;
    return newFragmentBegin;
}

void RingBufferAllocator::resize(uint32_t newCapacity)
{
    ASSERT(newCapacity >= mMinCapacity);

    if (mBuffer.getId() != 0)
    {
        mOldBuffers.emplace_back(std::move(mBuffer));
    }

    mCurrentCapacity = newCapacity;
    mBuffer.incrementId();
    mBuffer.resize(mCurrentCapacity);
    resetPointers();

    mAllocationMargin = mCurrentCapacity;
}

void RingBufferAllocator::release(uint8_t *releasePtr)
{
    if (releasePtr == mDataEnd)
    {
        // Ensures "mDataEnd == mBuffer.data()" with 0 allocations.
        resetPointers();
        return;
    }

    if (mDataBegin == mFragmentEnd)
    {
        ASSERT((releasePtr >= mBuffer.data() && releasePtr < mDataEnd) ||
               (releasePtr >= mDataBegin && releasePtr <= mBuffer.data() + mCurrentCapacity));
        if (releasePtr < mDataBegin)
        {
            mFragmentEnd = mBuffer.data() + mCurrentCapacity;
        }
        else
        {
            mFragmentEnd = releasePtr;
        }
        mFragmentEndR = mBuffer.decClamped(mFragmentEnd, mFragmentReserve);
    }
    else
    {
        ASSERT(releasePtr >= mDataBegin && releasePtr < mDataEnd);
    }
    mDataBegin = releasePtr;
}

uint32_t RingBufferAllocator::getNumAllocatedInBuffer() const
{
    // 2 free fragments: [mBuffer.begin, mDataBegin)                    [mDataEnd, mBuffer.end);
    // 1 used fragment:                             [DataBegin, DataEnd)
    if (mFragmentEnd != mDataBegin)
    {
        ASSERT(mDataEnd >= mDataBegin);
        return static_cast<uint32_t>(mDataEnd - mDataBegin);
    }

    // 1 free fragment:                           [mDataEnd, mDataBegin)
    // 2 used fragments: [mBuffer.begin, mDataEnd)                      [mDataBegin, mBuffer.end)
    ASSERT(mDataBegin >= mDataEnd);
    return (mCurrentCapacity - static_cast<uint32_t>(mDataBegin - mDataEnd));
}

void RingBufferAllocator::resetPointers()
{
    mDataBegin    = mBuffer.data();
    mDataEnd      = mDataBegin;
    mFragmentEnd  = mDataEnd + mCurrentCapacity;
    mFragmentEndR = mBuffer.decClamped(mFragmentEnd, mFragmentReserve);
}

// SharedRingBufferAllocator implementation.
SharedRingBufferAllocator::SharedRingBufferAllocator()
{
    mAllocator.reset();
}

SharedRingBufferAllocator::~SharedRingBufferAllocator()
{
#if defined(ANGLE_ENABLE_ASSERTS)
    ASSERT(!mSharedCP || mSharedCP->mRefCount == 1);
#endif
    SafeDelete(mSharedCP);
}

SharedRingBufferAllocatorCheckPoint *SharedRingBufferAllocator::acquireSharedCP()
{
    if (!mSharedCP)
    {
        mSharedCP = new SharedRingBufferAllocatorCheckPoint();
    }
#if defined(ANGLE_ENABLE_ASSERTS)
    ASSERT(++mSharedCP->mRefCount > 1);
    // Must always be 1 ref before
#endif
    return mSharedCP;
}

void SharedRingBufferAllocator::releaseToSharedCP()
{
    ASSERT(mSharedCP);
    const RingBufferAllocatorCheckPoint releaseCP = mSharedCP->pop();
    if (releaseCP.valid())
    {
        mAllocator.release(releaseCP);
    }
}

void SharedRingBufferAllocatorCheckPoint::releaseAndUpdate(RingBufferAllocatorCheckPoint *newValue)
{
    ASSERT(newValue && newValue->valid());
#if defined(ANGLE_ENABLE_ASSERTS)
    ASSERT(--mRefCount >= 1);
    // Must always remain 1 ref
#endif
    {
        std::lock_guard<angle::SimpleMutex> lock(mMutex);
        mValue = *newValue;
    }
    newValue->reset();
}

RingBufferAllocatorCheckPoint SharedRingBufferAllocatorCheckPoint::pop()
{
    std::lock_guard<angle::SimpleMutex> lock(mMutex);
    RingBufferAllocatorCheckPoint value = mValue;
    mValue.reset();
    return value;
}

}  // namespace angle
