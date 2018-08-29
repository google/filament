/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef TNT_UTILS_RANGESET_H
#define TNT_UTILS_RANGESET_H

#include <algorithm>
#include <array>
#include <assert.h>
#include <stdint.h>

#include <utils/algorithm.h>
#include <utils/Slice.h>

namespace utils {

struct BufferRange {
    uint32_t start;
    uint32_t end;
    uint32_t getCount() const noexcept { return end - start; }
};

template <size_t RANGE_COUNT = 4>
class RangeSet {
public:
    RangeSet() noexcept { clear(); }
    void set(uint32_t offset, uint32_t count);
    bool isEmpty() const noexcept { return mStorage.empty(); }
    void clear() noexcept {
        std::fill_n(mStorage.begin(), mStorage.capacity(),
                BufferRange{ 0, std::numeric_limits<uint32_t>::max() });
        mStorage.clear();
    }

    Slice<const BufferRange> getRanges() const { return { mStorage.cbegin(), mStorage.cend() }; }

    BufferRange const* begin() const noexcept { return mStorage.data(); }
    BufferRange const* end() const noexcept { return mStorage.data() + mStorage.size(); }
    BufferRange const* cbegin() const noexcept { return mStorage.data(); }
    BufferRange const* cend() const noexcept { return mStorage.data() + mStorage.size(); }
    BufferRange const& operator[](size_t i) const noexcept { return mStorage[i]; }

private:
    // Fixed capacity vector
    template<typename TYPE, size_t CAPACITY,
            typename = typename std::enable_if<std::is_pod<TYPE>::value>::type>
    class vector {
        std::array<TYPE, RANGE_COUNT> mElements;
        uint32_t mSize = 0;
        using value_type     = typename std::array<TYPE, CAPACITY>::value_type;
        using iterator       = value_type*;
        using const_iterator = value_type const*;

    public:
        bool empty() const noexcept { return size() == 0; }
        size_t size() const noexcept { return mSize; }
        size_t capacity() const noexcept { return CAPACITY; }
        value_type *data() { return mElements.data(); }
        value_type const *data() const { return mElements.data(); }
        iterator begin() { return mElements.data(); }
        iterator end() { return begin() + mSize; }
        const_iterator begin() const { return mElements.data(); }
        const_iterator end() const { return begin() + mSize; }
        const_iterator cbegin() const { return mElements.data(); }
        const_iterator cend() const { return cbegin() + mSize; }
        value_type const& operator[](size_t i) const noexcept { return mElements[i]; }
        value_type& operator[](size_t i)  noexcept { return mElements[i]; }

        void pop_back() noexcept {
            assert(mSize > 0);
            --mSize;
        }

        iterator insert(iterator pos, value_type const& value) noexcept {
            assert(mSize < CAPACITY);
            // equivalent to std::move_backward(pos, end(), end() + 1) but we don't want a call to memmove
            iterator e = end();
            while (pos != e) {
                --e;
                e[1] = e[0];
            }
            *pos = value;
            ++mSize;
            return pos;
        }

        iterator remove(iterator from, iterator to) noexcept {
            assert(from < to);
            assert((uint32_t) (to - from) <= mSize);
            std::move(to, end(), from);
            mSize -= (to-from);
            return from;
        }

        iterator remove(iterator pos) noexcept {
            return remove(pos, pos + 1);
        }

        void clear() noexcept { mSize = 0; }
    };

    vector<BufferRange, RANGE_COUNT> mStorage;
};

// ------------------------------------------------------------------------------------------------

template <size_t RANGE_COUNT>
void RangeSet<RANGE_COUNT>::set(uint32_t offset, uint32_t count) {
    if (count == 0) return;

    BufferRange r = { offset, offset + count };
    auto& storage = mStorage;

    // Note: we sort by "end" so that we can merge from the back of the array
    // allowing faster remove() when dealing with the last element (which should be
    // a common case)
    // We also use a constant size of the binary-search (so the code gets inlined and branch-less),
    // for this to work, we have to make sure the unused end of the array (between size and capacity)
    // is initialized to INT_MAX (as to always fail the search).
    auto pos = utils::upper_bound(storage.begin(), storage.begin() + storage.capacity(), r,
            [](BufferRange const& lhs, BufferRange const& rhs)->bool{
                return lhs.end < rhs.end;
            });

    if (storage.size() == storage.capacity()) {
        // if there is no space available, just merge with the range we found
        // (note: it's not always the best choice, but it's simple)
        if (pos == storage.end()) {
            pos = storage.end() - 1;
            pos->end = r.end;
        }
        pos->start = std::min(pos->start, r.start);
    } else {
        storage.insert(pos, r);
    }

    // inspect from the end, and collapse ranges that overlap
    auto const begin = storage.begin();
    auto cur = storage.end() - 1;
    auto prev = cur - 1;
    for ( ; cur > begin ; --cur, --prev) {
        if (prev->end >= cur->start) {
            prev->start = std::min(cur->start, prev->start);
            prev->end = cur->end;
            storage.remove(cur);
            storage.end()->end = std::numeric_limits<uint32_t>::max();
        }
    }
}

} // namespace utils


#endif // TNT_UTILS_RANGESET_H
