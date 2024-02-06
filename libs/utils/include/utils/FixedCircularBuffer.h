/*
 * Copyright (C) 2023 The Android Open Source Project
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
 *
 */

#ifndef TNT_UTILS_FIXEDCIRCULARBUFFER_H
#define TNT_UTILS_FIXEDCIRCULARBUFFER_H

#include <utils/debug.h>

#include <memory>
#include <optional>
#include <type_traits>

#include <stddef.h>

namespace utils {

template<typename T>
class FixedCircularBuffer {
public:
    explicit FixedCircularBuffer(size_t capacity)
        : mData(std::make_unique<T[]>(capacity)), mCapacity(capacity) {}

    size_t size() const noexcept { return mSize; }
    size_t capacity() const noexcept { return mCapacity; }
    bool full() const noexcept { return mCapacity > 0 && mSize == mCapacity; }
    bool empty() const noexcept { return mSize == 0; }

    /**
     * Push v into the buffer. If the buffer is already full, removes the oldest item and returns
     * it. If this buffer has no capacity, simply returns v.
     * @param v the new value to push into the buffer
     * @return if the buffer was full, the oldest value which was displaced
     */
    std::optional<T> push(T v) noexcept {
        if (mCapacity == 0) {
            return v;
        }
        std::optional<T> displaced = full() ? pop() : std::optional<T>{};
        mData[mEnd] = v;
        mEnd = (mEnd + 1) % mCapacity;
        mSize++;
        return displaced;
    }

    T pop() noexcept {
        assert_invariant(mSize > 0);
        T result = mData[mBegin];
        mBegin = (mBegin + 1) % mCapacity;
        mSize--;
        return result;
    }

private:
    std::unique_ptr<T[]> mData;

    size_t mBegin = 0;
    size_t mEnd = 0;
    size_t mSize = 0;
    size_t mCapacity;
};

} // namespace utils

#endif  // TNT_UTILS_FIXEDCIRCULARBUFFER_H
