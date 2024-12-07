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

#ifndef TNT_FILAMENT_BACKEND_VULKAN_UTILS_CAPPEDARRAY_H
#define TNT_FILAMENT_BACKEND_VULKAN_UTILS_CAPPEDARRAY_H

// An Array that will be statically fixed in capacity, but the "size" (as in user added elements) is
// variable. Note that this class is movable.

#include <utils/Panic.h>

#include <array>

namespace filament::backend::fvkutils {

template<typename T, uint16_t CAPACITY>
class CappedArray {
private:
    using FixedSizeArray = std::array<T, CAPACITY>;

public:
    using const_iterator = typename FixedSizeArray::const_iterator;
    using iterator = typename FixedSizeArray::iterator;

    CappedArray() = default;

    // Delete copy constructor/assignment.
    CappedArray(CappedArray const& rhs) = delete;
    CappedArray& operator=(CappedArray& rhs) = delete;

    CappedArray(CappedArray&& rhs) noexcept {
        this->swap(rhs);
    }

    CappedArray& operator=(CappedArray&& rhs) noexcept {
        this->swap(rhs);
        return *this;
    }

    inline ~CappedArray() {
        clear();
    }

    inline const_iterator begin() const {
        if (mInd == 0) {
            return mArray.cend();
        }
        return mArray.cbegin();
    }

    inline const_iterator end() const {
        if (mInd > 0 && mInd < CAPACITY) {
            return mArray.begin() + mInd;
        }
        return mArray.cend();
    }

    inline iterator begin() {
        if (mInd == 0) {
            return mArray.end();
        }
        return mArray.begin();
    }

    inline iterator end() {
        if (mInd > 0 && mInd < CAPACITY) {
            return mArray.begin() + mInd;
        }
        return mArray.end();
    }

    inline T back() {
        assert_invariant(mInd > 0);
        return *(mArray.begin() + mInd);
    }

    inline void pop_back() {
        assert_invariant(mInd > 0);
        mInd--;
    }

    inline const_iterator find(T item) {
        return std::find(begin(), end(), item);
    }

    inline void insert(T item) {
        assert_invariant(mInd < CAPACITY);
        mArray[mInd++] = item;
    }

    inline void erase(T item) {
        PANIC_PRECONDITION("CappedArray::erase should not be called");
    }

    inline void clear() {
        if (mInd == 0) {
            return;
        }
        mInd = 0;
    }

    inline T& operator[](uint16_t ind) {
        return mArray[ind];
    }

    inline T const& operator[](uint16_t ind) const {
        return mArray[ind];
    }

    inline uint32_t size() const {
        return mInd;
    }

    T* data() {
        return mArray.data();
    }

    T const* data() const {
        return mArray.data();
    }

    bool operator==(CappedArray const& b) const {
        return this->mArray == b.mArray;
    }

private:
    void swap(CappedArray& rhs) {
        std::swap(mArray, rhs.mArray);
        std::swap(mInd, rhs.mInd);
    }

    FixedSizeArray mArray;
    uint32_t mInd = 0;
};

} // namespace filament::backend::fvkutils

#endif // TNT_FILAMENT_BACKEND_VULKAN_UTILS_CAPPEDARRAY_H
