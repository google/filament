/*
* Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_UTILS_MONOTONICRINGMAP_H
#define TNT_UTILS_MONOTONICRINGMAP_H

#include <utils/compiler.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace utils {

/**
 * A map-like container with a fixed capacity and monotonically increasing keys.
 * When the map is full, inserting a new element overwrites the oldest one.
 * This container doesn't allocate any memory on the heap.
 * Lookups are O(log N).
 */
template<size_t N, typename KEY, typename VALUE>
class MonotonicRingMap {
public:
    using key_type = KEY;                 //!< The key type.
    using mapped_type = VALUE;            //!< The value type.
    using value_type = std::pair<key_type, mapped_type>; //!< The key-value pair type.

    //! Creates an empty map.
    MonotonicRingMap() noexcept = default;

    //! Returns the number of elements in the map.
    size_t size() const noexcept { return mSize; }

    //! Returns the maximum number of elements the map can hold.
    static constexpr size_t capacity() noexcept { return N; }

    //! Returns true if the map is empty.
    bool empty() const noexcept { return mSize == 0; }

    //! Returns true if the map is full.
    bool full() const noexcept { return mSize == N; }

    //! Clears the map entirely.
    void clear() noexcept { mSize = 0; mHead = 0; }

    /**
     * Inserts a new key-value pair.
     * The key must be greater than the key of the last inserted element.
     * If the map is full, the oldest element is overwritten.
     * @param key The key to insert.
     * @param value The value to associate with the key.
     */
    UTILS_NOINLINE void insert(key_type key, mapped_type value) {
        assert(empty() || key > back().first); // assert monotonic
        if (UTILS_LIKELY(full())) {
            // container is full, replace the oldest element
            mStorage[mHead] = { key, value };
            mHead = (mHead + 1) % N;
        } else {
            // container is not full, add to the end
            const uint32_t index = (mHead + mSize) % N;
            mStorage[index] = { key, value };
            mSize++;
        }
    }

    /**
     * Finds a value by its key.
     * @param key The key to look for.
     * @return A pointer to the value if the key is found, nullptr otherwise.
     */
    mapped_type* find(key_type key) {
        return const_cast<mapped_type*>(static_cast<const MonotonicRingMap*>(this)->find(key));
    }

    /**
     * Finds a value by its key.
     * @param key The key to look for.
     * @return A pointer to the const value if the key is found, nullptr otherwise.
     */
    UTILS_NOINLINE const mapped_type* find(key_type key) const {
        if (empty()) {
            return nullptr;
        }

        if (key < front().first || key > back().first) {
            return nullptr;
        }

        const auto comparator = [](const value_type& element, key_type k) {
            return element.first < k;
        };

        const auto endOfStorage = mStorage.cbegin() + N;
        const auto headIter = mStorage.cbegin() + mHead;

        if (mHead + mSize <= N) {
            // The logical sequence is contiguous in memory
            const auto logicalEnd = headIter + mSize;
            auto it = std::lower_bound(headIter, logicalEnd, key, comparator);
            if (it != logicalEnd && it->first == key) {
                return &it->second;
            }
        } else { // Wrapped around
            // First part: mStorage[mHead...N-1]
            auto it1 = std::lower_bound(headIter, endOfStorage, key, comparator);
            if (it1 != endOfStorage && it1->first == key) {
                return &it1->second;
            }

            // Second part: mStorage[0...head-1]
            const auto wrapPartEndIter = mStorage.cbegin() + ((mHead + mSize) % N);
            auto it2 = std::lower_bound(mStorage.cbegin(), wrapPartEndIter, key, comparator);
             if (it2 != wrapPartEndIter && it2->first == key) {
                return &it2->second;
            }
        }
        return nullptr;
    }

    //! Returns a reference to the oldest element.
    const value_type& front() const {
        assert(!empty());
        return mStorage[mHead];
    }

    //! Returns a reference to the newest element.
    const value_type& back() const {
        assert(!empty());
        return mStorage[(mHead + mSize - 1) % N];
    }

private:
    std::array<value_type, N> mStorage;
    uint32_t mSize = 0;
    uint32_t mHead = 0;
};

} // namespace utils

#endif // TNT_UTILS_MONOTONICRINGMAP_H
