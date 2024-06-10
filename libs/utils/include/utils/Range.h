/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef TNT_UTILS_RANGE_H
#define TNT_UTILS_RANGE_H

#include <iterator>

#include <stddef.h>

namespace utils {

template<typename T>
struct Range {
    using value_type = T;
    T first = 0;
    T last = 0; // this actually refers to one past the last

    size_t size() const noexcept { return last - first; }
    bool empty() const noexcept { return !size(); }
    bool contains(const T& t) const noexcept { return first <= t && t < last; }

    bool overlaps(const Range<T>& that) const noexcept {
        return (that.first < this->last) && (that.last > this->first);
    }

    class const_iterator {
        friend struct Range;
        T value = {};

    public:
        const_iterator() noexcept = default;
        explicit const_iterator(T value) noexcept : value(value) {}

        using value_type = T;
        using pointer = value_type*;
        using difference_type = ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;


        const value_type operator*() const { return value; }
        const value_type operator[](size_t n) const { return value + n; }

        const_iterator& operator++() { ++value; return *this; }
        const_iterator& operator--() { --value; return *this; }

        const const_iterator operator++(int) { const_iterator t(value); value++; return t; }
        const const_iterator operator--(int) { const_iterator t(value); value--; return t; }

        const_iterator operator+(size_t rhs) const { return { value + rhs }; }
        const_iterator operator+(size_t rhs)       { return { value + rhs }; }
        const_iterator operator-(size_t rhs) const { return { value - rhs }; }

        difference_type operator-(const_iterator const& rhs) const { return value - rhs.value; }

        bool operator==(const_iterator const& rhs) const { return (value == rhs.value); }
        bool operator!=(const_iterator const& rhs) const { return (value != rhs.value); }
        bool operator>=(const_iterator const& rhs) const { return (value >= rhs.value); }
        bool operator> (const_iterator const& rhs) const { return (value >  rhs.value); }
        bool operator<=(const_iterator const& rhs) const { return (value <= rhs.value); }
        bool operator< (const_iterator const& rhs) const { return (value <  rhs.value); }
    };

    const_iterator begin() noexcept { return const_iterator{ first }; }
    const_iterator end() noexcept { return const_iterator{ last }; }
    const_iterator begin() const noexcept { return const_iterator{ first }; }
    const_iterator end() const noexcept { return const_iterator{ last }; }

    const_iterator front() const noexcept { return const_iterator{ first }; }
    const_iterator back() const noexcept { return const_iterator{ last - 1 }; }
};

} // namespace utils

#endif // TNT_UTILS_RANGE_H
