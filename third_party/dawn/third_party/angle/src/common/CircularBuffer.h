//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CircularBuffer.h:
//   An array class with an index that loops through the elements.
//

#ifndef COMMON_CIRCULARBUFFER_H_
#define COMMON_CIRCULARBUFFER_H_

#include "common/debug.h"

#include <algorithm>
#include <array>

namespace angle
{
template <class T, size_t N, class Storage = std::array<T, N>>
class CircularBuffer final
{
  public:
    using value_type      = typename Storage::value_type;
    using size_type       = typename Storage::size_type;
    using reference       = typename Storage::reference;
    using const_reference = typename Storage::const_reference;
    using pointer         = typename Storage::pointer;
    using const_pointer   = typename Storage::const_pointer;
    using iterator        = typename Storage::iterator;
    using const_iterator  = typename Storage::const_iterator;

    CircularBuffer();
    CircularBuffer(const value_type &value);

    CircularBuffer(const CircularBuffer<T, N, Storage> &other);
    CircularBuffer(CircularBuffer<T, N, Storage> &&other);

    CircularBuffer<T, N, Storage> &operator=(const CircularBuffer<T, N, Storage> &other);
    CircularBuffer<T, N, Storage> &operator=(CircularBuffer<T, N, Storage> &&other);

    ~CircularBuffer();

    // begin() and end() are used to iterate over all elements regardless of the current position of
    // the front of the buffer.  Useful for initialization and clean up, as otherwise only the front
    // element is expected to be accessed.
    iterator begin();
    const_iterator begin() const;

    iterator end();
    const_iterator end() const;

    size_type size() const;

    reference front();
    const_reference front() const;

    void swap(CircularBuffer<T, N, Storage> &other);

    // Move the front forward to the next index, looping back to the beginning if the end of the
    // array is reached.
    void next();

  private:
    Storage mData;
    size_type mFrontIndex;
};

template <class T, size_t N, class Storage>
CircularBuffer<T, N, Storage>::CircularBuffer() : mFrontIndex(0)
{}

template <class T, size_t N, class Storage>
CircularBuffer<T, N, Storage>::CircularBuffer(const value_type &value) : CircularBuffer()
{
    std::fill(begin(), end(), value);
}

template <class T, size_t N, class Storage>
CircularBuffer<T, N, Storage>::CircularBuffer(const CircularBuffer<T, N, Storage> &other)
{
    *this = other;
}

template <class T, size_t N, class Storage>
CircularBuffer<T, N, Storage>::CircularBuffer(CircularBuffer<T, N, Storage> &&other)
    : CircularBuffer()
{
    swap(other);
}

template <class T, size_t N, class Storage>
CircularBuffer<T, N, Storage> &CircularBuffer<T, N, Storage>::operator=(
    const CircularBuffer<T, N, Storage> &other)
{
    std::copy(other.begin(), other.end(), begin());
    mFrontIndex = other.mFrontIndex;
    return *this;
}

template <class T, size_t N, class Storage>
CircularBuffer<T, N, Storage> &CircularBuffer<T, N, Storage>::operator=(
    CircularBuffer<T, N, Storage> &&other)
{
    swap(other);
    return *this;
}

template <class T, size_t N, class Storage>
CircularBuffer<T, N, Storage>::~CircularBuffer() = default;

template <class T, size_t N, class Storage>
ANGLE_INLINE typename CircularBuffer<T, N, Storage>::iterator CircularBuffer<T, N, Storage>::begin()
{
    return mData.begin();
}

template <class T, size_t N, class Storage>
ANGLE_INLINE typename CircularBuffer<T, N, Storage>::const_iterator
CircularBuffer<T, N, Storage>::begin() const
{
    return mData.begin();
}

template <class T, size_t N, class Storage>
ANGLE_INLINE typename CircularBuffer<T, N, Storage>::iterator CircularBuffer<T, N, Storage>::end()
{
    return mData.end();
}

template <class T, size_t N, class Storage>
ANGLE_INLINE typename CircularBuffer<T, N, Storage>::const_iterator
CircularBuffer<T, N, Storage>::end() const
{
    return mData.end();
}

template <class T, size_t N, class Storage>
ANGLE_INLINE typename CircularBuffer<T, N, Storage>::size_type CircularBuffer<T, N, Storage>::size()
    const
{
    return N;
}

template <class T, size_t N, class Storage>
ANGLE_INLINE typename CircularBuffer<T, N, Storage>::reference
CircularBuffer<T, N, Storage>::front()
{
    ASSERT(mFrontIndex < size());
    return mData[mFrontIndex];
}

template <class T, size_t N, class Storage>
ANGLE_INLINE typename CircularBuffer<T, N, Storage>::const_reference
CircularBuffer<T, N, Storage>::front() const
{
    ASSERT(mFrontIndex < size());
    return mData[mFrontIndex];
}

template <class T, size_t N, class Storage>
void CircularBuffer<T, N, Storage>::swap(CircularBuffer<T, N, Storage> &other)
{
    std::swap(mData, other.mData);
    std::swap(mFrontIndex, other.mFrontIndex);
}

template <class T, size_t N, class Storage>
void CircularBuffer<T, N, Storage>::next()
{
    mFrontIndex = (mFrontIndex + 1) % size();
}
}  // namespace angle

#endif  // COMMON_CIRCULARBUFFER_H_
