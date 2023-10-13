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

#include <stddef.h>
#include <type_traits>

namespace utils {

template<typename T, size_t N>
class FixedCircularBuffer {
public:

    size_t size() const noexcept { return mSize; }
    bool full() const noexcept { return mSize == N; }

    void push(T v) noexcept {
        assert_invariant(!full());
        mData[mEnd] = v;
        mEnd = (mEnd + 1) % N;
        mSize++;
    }

    T pop() noexcept {
        assert_invariant(mSize > 0);
        T result = mData[mBegin];
        mBegin = (mBegin + 1) % N;
        mSize--;
        return result;
    }

private:
    T mData[N];

    size_t mBegin = 0;
    size_t mEnd = 0;
    size_t mSize = 0;
};

} // namespace utils

#endif  // TNT_UTILS_FIXEDCIRCULARBUFFER_H
