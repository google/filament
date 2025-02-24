//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FixedQueue.h:
//   An array based fifo queue class that supports concurrent push and pop.
//

#ifndef COMMON_FIXEDQUEUE_H_
#define COMMON_FIXEDQUEUE_H_

#include "common/debug.h"

#include <algorithm>
#include <array>
#include <atomic>

namespace angle
{
// class FixedQueue: An vector based fifo queue class that supports concurrent push and
// pop. Caller must ensure queue is not empty before pop and not full before push. This class
// supports concurrent push and pop from different threads, but only with single producer single
// consumer usage. If caller want to push from two different threads, proper mutex must be used to
// ensure the access is serialized. You can also call updateCapacity to adjust the storage size, but
// caller must take proper mutex lock to ensure no one is accessing the storage. In a typical usage
// case is that you have two mutex lock, enqueueLock and dequeueLock. You use enqueueLock to push
// and use dequeueLock to pop. You dont need the lock for checking size (i.e, call empty/full). You
// take both lock in a given order to call updateCapacity. See unit test
// FixedQueue.ConcurrentPushPopWithResize for example.
template <class T>
class FixedQueue final : angle::NonCopyable
{
  public:
    using Storage         = std::vector<T>;
    using value_type      = typename Storage::value_type;
    using size_type       = typename Storage::size_type;
    using reference       = typename Storage::reference;
    using const_reference = typename Storage::const_reference;

    FixedQueue(size_t capacity);
    ~FixedQueue();

    size_type size() const;
    bool empty() const;
    bool full() const;

    size_type capacity() const;
    // Caller must ensure no one is accessing the data while update storage. This should happen
    // infrequently since all data will be copied between old storage and new storage.
    void updateCapacity(size_t newCapacity);

    reference front();
    const_reference front() const;

    void push(const value_type &value);
    void push(value_type &&value);

    reference back();
    const_reference back() const;

    void pop();
    void clear();

  private:
    Storage mData;
    // The front and back indices are virtual indices (think about queue size is infinite). They
    // will never wrap around when hit N. The wrap around occur when element is referenced. Virtual
    // index for current head
    size_type mFrontIndex;
    // Virtual index for next write.
    size_type mEndIndex;
    // Atomic so that we can support concurrent push and pop.
    std::atomic<size_type> mSize;
    size_type mMaxSize;
};

template <class T>
FixedQueue<T>::FixedQueue(size_t capacity)
    : mFrontIndex(0), mEndIndex(0), mSize(0), mMaxSize(capacity)
{
    mData.resize(mMaxSize);
}

template <class T>
FixedQueue<T>::~FixedQueue()
{
    mData.clear();
}

template <class T>
ANGLE_INLINE typename FixedQueue<T>::size_type FixedQueue<T>::size() const
{
    ASSERT(mSize <= mMaxSize);
    return mSize;
}

template <class T>
ANGLE_INLINE bool FixedQueue<T>::empty() const
{
    return size() == 0;
}

template <class T>
ANGLE_INLINE bool FixedQueue<T>::full() const
{
    return size() == mMaxSize;
}

template <class T>
ANGLE_INLINE typename FixedQueue<T>::size_type FixedQueue<T>::capacity() const
{
    return mMaxSize;
}

template <class T>
ANGLE_INLINE void FixedQueue<T>::updateCapacity(size_t newCapacity)
{
    ASSERT(newCapacity >= size());
    Storage newData(newCapacity);
    for (size_type i = mFrontIndex; i < mEndIndex; i++)
    {
        newData[i % newCapacity] = std::move(mData[i % mMaxSize]);
    }
    mData.clear();
    std::swap(newData, mData);
    mMaxSize = newCapacity;
    ASSERT(mData.size() == mMaxSize);
}

template <class T>
ANGLE_INLINE typename FixedQueue<T>::reference FixedQueue<T>::front()
{
    ASSERT(!empty());
    return mData[mFrontIndex % mMaxSize];
}

template <class T>
ANGLE_INLINE typename FixedQueue<T>::const_reference FixedQueue<T>::front() const
{
    ASSERT(!empty());
    return mData[mFrontIndex % mMaxSize];
}

template <class T>
void FixedQueue<T>::push(const value_type &value)
{
    ASSERT(!full());
    mData[mEndIndex % mMaxSize] = value;
    mEndIndex++;
    // We must increment size last, after we wrote data. That way if another thread is doing
    // `if(!dq.empty()){ s = dq.front(); }`, it will only see not empty until element is fully
    // pushed.
    mSize++;
}

template <class T>
void FixedQueue<T>::push(value_type &&value)
{
    ASSERT(!full());
    mData[mEndIndex % mMaxSize] = std::move(value);
    mEndIndex++;
    // We must increment size last, after we wrote data. That way if another thread is doing
    // `if(!dq.empty()){ s = dq.front(); }`, it will only see not empty until element is fully
    // pushed.
    mSize++;
}

template <class T>
ANGLE_INLINE typename FixedQueue<T>::reference FixedQueue<T>::back()
{
    ASSERT(!empty());
    return mData[(mEndIndex + (mMaxSize - 1)) % mMaxSize];
}

template <class T>
ANGLE_INLINE typename FixedQueue<T>::const_reference FixedQueue<T>::back() const
{
    ASSERT(!empty());
    return mData[(mEndIndex + (mMaxSize - 1)) % mMaxSize];
}

template <class T>
void FixedQueue<T>::pop()
{
    ASSERT(!empty());
    mData[mFrontIndex % mMaxSize] = value_type();
    mFrontIndex++;
    // We must decrement size last, after we wrote data. That way if another thread is doing
    // `if(!dq.full()){ dq.push; }`, it will only see not full until element is fully popped.
    mSize--;
}

template <class T>
void FixedQueue<T>::clear()
{
    // Size will change in the "pop()" and also by "push()" calls from other thread.
    const size_type localSize = size();
    for (size_type i = 0; i < localSize; i++)
    {
        pop();
    }
}
}  // namespace angle

#endif  // COMMON_FIXEDQUEUE_H_
