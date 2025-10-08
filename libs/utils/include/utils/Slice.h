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

#ifndef TNT_UTILS_SLICE_H
#define TNT_UTILS_SLICE_H

#include <utils/Hash.h>
#include <utils/compiler.h>

#include <algorithm>
#include <type_traits>
#include <utility>

#include <assert.h>
#include <stddef.h>

namespace utils {

/** A fixed-size slice of a container.
 *
 * Analogous to std::span.
 */
template<typename T>
class Slice {
public:
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using size_type = size_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using iterator = T*;
    using const_iterator = T const*;

    Slice() = default;
    Slice(iterator begin, iterator end) noexcept : mBegin(begin), mEnd(end) {}
    Slice(pointer begin, size_type count) noexcept : mBegin(begin), mEnd(begin + count) {}

    Slice(Slice<T> const& rhs) : mBegin(rhs.begin()), mEnd(rhs.end()) {}

    // If Slice<T> is Slice<const U>, define coercive copy constructor from Slice<U>.
    template<typename U = T>
    Slice(std::enable_if_t<std::is_const_v<U>, Slice<value_type> const&> rhs)
        : mBegin(rhs.begin()), mEnd(rhs.end()) {}

    Slice& operator=(Slice<T> const& rhs) noexcept {
        mBegin = rhs.begin();
        mEnd = rhs.end();
        return *this;
    }

    // If Slice<T> is Slice<const U>, define assignment operator from Slice<U>.
    template<typename U = T>
    Slice& operator=(
            std::enable_if_t<std::is_const_v<U>, Slice<value_type> const&> rhs) noexcept {
        mBegin = rhs.begin();
        mEnd = rhs.end();
        return *this;
    }

    void set(pointer begin, size_type count) noexcept {
        mBegin = begin;
        mEnd = begin + count;
    }

    void set(iterator begin, iterator end) noexcept {
        mBegin = begin;
        mEnd = end;
    }

    void swap(Slice<T>& rhs) noexcept {
        std::swap(mBegin, rhs.mBegin);
        std::swap(mEnd, rhs.mEnd);
    }

    void clear() noexcept {
        mBegin = nullptr;
        mEnd = nullptr;
    }

    bool operator==(Slice<const value_type> const& rhs) const noexcept {
        if (size() != rhs.size()) {
            return false;
        }
        if (mBegin == rhs.cbegin()) {
            return true;
        }
        return std::equal(cbegin(), cend(), rhs.cbegin());
    }

    bool operator==(Slice<value_type> const& rhs) const noexcept {
        return *this == Slice<const value_type>(rhs);
    }

    // size
    size_t size() const noexcept { return mEnd - mBegin; }
    size_t sizeInBytes() const noexcept { return size() * sizeof(T); }
    bool empty() const noexcept { return size() == 0; }

    // iterators
    iterator begin() const noexcept { return mBegin; }
    const_iterator cbegin() const noexcept { return this->begin(); }
    iterator end() const noexcept { return mEnd; }
    const_iterator cend() const noexcept { return this->end(); }

    // data access
    reference operator[](size_t n) const noexcept {
        assert(n < size());
        return mBegin[n];
    }

    reference at(size_t n) const noexcept {
        return operator[](n);
    }

    reference front() const noexcept {
        assert(!empty());
        return *mBegin;
    }

    reference back() const noexcept {
        assert(!empty());
        return *(this->end() - 1);
    }

    pointer data() const noexcept {
        return this->begin();
    }

    template<typename Hash = std::hash<value_type>>
    size_t hash() const noexcept {
        Hash hasher;
        size_t seed = size();
        for (auto const& it : *this) {
            utils::hash::combine_fast(seed, hasher(it));
        }
        return seed;
    }

protected:
    iterator mBegin = nullptr;
    iterator mEnd = nullptr;
};

} // namespace utils

namespace std {

template<typename T>
struct hash<utils::Slice<T>> {
    inline size_t operator()(utils::Slice<T> const& lhs) const noexcept { return lhs.hash(); }
};

} // namespace std

#endif // TNT_UTILS_SLICE_H
