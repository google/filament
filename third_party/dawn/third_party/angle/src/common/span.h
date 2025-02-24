//
// Copyright 2025 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Span.h:
//   Basic implementation of C++20's span.
//

#ifndef COMMON_SPAN_H_
#define COMMON_SPAN_H_

#include <type_traits>

#include "common/log_utils.h"

namespace angle
{

// Basic implementation of C++20's span.
// See the reference for std::span here: https://en.cppreference.com/w/cpp/container/span
template <typename T>
class Span
{
  public:
    typedef size_t size_type;

    constexpr Span() = default;
    constexpr Span(T *ptr, size_type size) : mData(ptr), mSize(size) {}

    template <typename V,
              typename = std::enable_if_t<std::remove_reference_t<V>::is_pool_allocated>>
    constexpr Span(V &&vec) : mData(vec.data()), mSize(vec.size())
    {}

    template <typename V,
              typename = std::enable_if_t<std::remove_reference_t<V>::is_pool_allocated>>
    constexpr Span &operator=(V &&vec)
    {
        mData = vec.data();
        mSize = vec.size();
        return *this;
    }

    constexpr bool operator==(const Span &that) const
    {
        if (mSize != that.mSize)
        {
            return false;
        }

        if (mData == that.mData)
        {
            return true;
        }

        for (size_type index = 0; index < mSize; ++index)
        {
            if (mData[index] != that.mData[index])
            {
                return false;
            }
        }

        return true;
    }
    constexpr bool operator!=(const Span &that) const { return !(*this == that); }

    constexpr T *data() const { return mData; }
    constexpr size_type size() const { return mSize; }
    constexpr bool empty() const { return mSize == 0; }

    constexpr T &operator[](size_type index) const { return mData[index]; }
    constexpr T &front() const { return mData[0]; }
    constexpr T &back() const { return mData[mSize - 1]; }

    constexpr T *begin() const { return mData; }
    constexpr T *end() const { return mData + mSize; }

    constexpr std::reverse_iterator<T *> rbegin() const
    {
        return std::make_reverse_iterator(end());
    }
    constexpr std::reverse_iterator<T *> rend() const
    {
        return std::make_reverse_iterator(begin());
    }

    constexpr Span first(size_type count) const
    {
        ASSERT(count <= mSize);
        return count == 0 ? Span() : Span(mData, count);
    }
    constexpr Span last(size_type count) const
    {
        ASSERT(count <= mSize);
        return count == 0 ? Span() : Span(mData + mSize - count, count);
    }
    constexpr Span subspan(size_type offset, size_type count) const
    {
        ASSERT(offset + count <= mSize);
        return count == 0 ? Span() : Span(mData + offset, count);
    }

  private:
    T *mData     = nullptr;
    size_t mSize = 0;
};

}  // namespace angle

#endif  // COMMON_SPAN_
