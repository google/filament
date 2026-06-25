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

#include <utils/compiler.h>
#include <utils/Hash.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <type_traits>
#include <utility>

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

namespace utils {

template<typename T>
class Slice;

constexpr size_t dynamic_extent = static_cast<size_t>(-1);

namespace details {

template<typename T>
struct is_slice : std::false_type {};

template<typename T>
struct is_slice<Slice<T>> : std::true_type {};

} // namespace details

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
    using difference_type = ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using iterator = T*;
    using const_iterator = T const*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static constexpr size_type extent = dynamic_extent;

    Slice() = default;
    Slice(iterator const begin, iterator const end) noexcept : mBegin(begin), mEnd(end) {}
    Slice(pointer begin, size_type count) noexcept : mBegin(begin), mEnd(begin + count) {}

    template<size_t N>
    Slice(element_type (&arr)[N]) noexcept : mBegin(arr), mEnd(arr + N) {}

    template<typename Cont>
    using IsConvertibleToSlice = std::enable_if_t<
            !details::is_slice<std::remove_cv_t<std::remove_reference_t<Cont>>>::value &&
            !std::is_array_v<std::remove_cv_t<std::remove_reference_t<Cont>>> &&
            std::is_convertible_v<decltype(std::declval<Cont&>().data()), pointer> &&
            std::is_convertible_v<decltype(std::declval<Cont&>().size()), size_type>>;

    template<typename Cont,
            typename = IsConvertibleToSlice<Cont>>
    Slice(Cont&& cont) noexcept : Slice(cont.data(), cont.size()) {}

    Slice(Slice const& rhs) : mBegin(rhs.begin()), mEnd(rhs.end()) {}

    // If Slice<T> is Slice<const U>, define coercive copy constructor from Slice<U>.
    template<typename U = T>
    Slice(std::enable_if_t<std::is_const_v<U>, Slice<value_type> const&> rhs)
        : mBegin(rhs.begin()), mEnd(rhs.end()) {}

    Slice& operator=(Slice const& rhs) noexcept {
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

    void swap(Slice& rhs) noexcept {
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

    // subviews
    Slice first(size_type count) const noexcept {
        assert(count <= size());
        return Slice(data(), count);
    }

    Slice last(size_type count) const noexcept {
        assert(count <= size());
        return Slice(data() + size() - count, count);
    }

    Slice subspan(size_type offset, size_type count = dynamic_extent) const noexcept {
        assert(offset <= size());
        if (count == dynamic_extent) {
            count = size() - offset;
        }
        assert(count <= size() - offset);
        return Slice(data() + offset, count);
    }

    // size
    size_t size() const noexcept { return mEnd - mBegin; }
    size_t sizeInBytes() const noexcept { return size() * sizeof(T); }
    size_type size_bytes() const noexcept { return this->sizeInBytes(); }
    bool empty() const noexcept { return size() == 0; }

    // iterators
    iterator begin() const noexcept { return mBegin; }
    const_iterator cbegin() const noexcept { return this->begin(); }
    iterator end() const noexcept { return mEnd; }
    const_iterator cend() const noexcept { return this->end(); }

    reverse_iterator rbegin() const noexcept { return reverse_iterator(this->end()); }
    const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(this->end()); }
    reverse_iterator rend() const noexcept { return reverse_iterator(this->begin()); }
    const_reverse_iterator crend() const noexcept { return const_reverse_iterator(this->begin()); }

    // data access
    reference operator[](size_t n) const noexcept {
        assert(n < size());
        return mBegin[n];
    }

    reference at(size_t const n) const noexcept {
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
            hash::combine_fast(seed, hasher(it));
        }
        return seed;
    }

protected:
    iterator mBegin = nullptr;
    iterator mEnd = nullptr;
};

template<typename T>
Slice<const uint8_t> as_bytes(Slice<T> s) noexcept {
    return Slice<const uint8_t>(reinterpret_cast<const uint8_t*>(s.data()), s.sizeInBytes());
}

template<typename T, typename = std::enable_if_t<!std::is_const_v<T>>>
Slice<uint8_t> as_writable_bytes(Slice<T> s) noexcept {
    return Slice<uint8_t>(reinterpret_cast<uint8_t*>(s.data()), s.sizeInBytes());
}

#if __cplusplus >= 201703L
template<typename Cont>
Slice(Cont&) -> Slice<typename Cont::value_type>;

template<typename Cont>
Slice(Cont const&) -> Slice<const typename Cont::value_type>;
#endif

} // namespace utils

template<typename T>
struct std::hash<utils::Slice<T>> {
    size_t operator()(utils::Slice<T> const& lhs) const noexcept { return lhs.hash(); }
}; // namespace std

#endif // TNT_UTILS_SLICE_H
