/*
 * Copyright (C) 2024 The Android Open Source Project
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
 */

#ifndef TNT_FILAMENT_BACKEND_VULKAN_UTILS_STATICVECTOR_H
#define TNT_FILAMENT_BACKEND_VULKAN_UTILS_STATICVECTOR_H

// An Array that will be statically fixed in capacity, but the "size" (as in user added elements) is
// variable. Note that this class is movable.

#include <utils/debug.h>
#include <utils/Panic.h>

#include <array>

namespace filament::backend::fvkutils {

template<typename T, uint16_t CAPACITY>
class StaticVector {
private:
    using FixedSizeArray = std::array<T, CAPACITY>;

    static_assert(CAPACITY <= (1LL << (8 * sizeof(uint16_t))));

public:
    using const_iterator = typename FixedSizeArray::const_iterator;
    using iterator = typename FixedSizeArray::iterator;

    StaticVector() = default;

    // Delete copy constructor/assignment.
    StaticVector(StaticVector const& rhs) = delete;
    StaticVector& operator=(StaticVector& rhs) = delete;

    StaticVector(StaticVector&& rhs) noexcept {
        std::swap(mSize, rhs.mSize);
        std::swap(mArray, rhs.mArray);
    }

    StaticVector& operator=(StaticVector&& rhs) noexcept {
        std::swap(mSize, rhs.mSize);
        std::swap(mArray, rhs.mArray);
        return *this;
    }

    inline ~StaticVector() {
        clear();
    }

    inline const_iterator begin() const {
        return mArray.cbegin();
    }

    inline const_iterator end() const {
        assert_invariant(mSize <= CAPACITY);
        return mArray.begin() + mSize;
    }

    inline iterator begin() {
        return mArray.begin();
    }

    inline iterator end() {
        assert_invariant(mSize <= CAPACITY);
        return mArray.begin() + mSize;
    }

    inline T back() {
        assert_invariant(mSize > 0);
        return *(mArray.begin() + mSize);
    }

    inline void pop_back() {
        assert_invariant(mSize > 0);
        mSize--;
    }

    inline const_iterator find(T item) {
        return std::find(begin(), end(), item);
    }

    inline void push_back(T item) {
        assert_invariant(mSize < CAPACITY);
        mArray[mSize++] = item;
    }

    inline void clear() {
        mSize = 0;
    }

    inline T& operator[](size_t index) {
        assert_invariant(index < mSize);
        return mArray[index];
    }

    inline T const& operator[](size_t index) const {
        return mArray[index];
    }

    inline uint16_t size() const {
        return mSize;
    }

    T* data() {
        return mArray.data();
    }

    T const* data() const {
        return mArray.data();
    }

    bool operator==(StaticVector const& b) const {
        return this->mArray == b.mArray && this->mSize == b.mSize;
    }

private:
    FixedSizeArray mArray;
    uint16_t mSize = 0;
};

} // namespace filament::backend::fvkutils

#endif // TNT_FILAMENT_BACKEND_VULKAN_UTILS_STATICVECTOR_H
