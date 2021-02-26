/*
 * Copyright (C) 2021 The Android Open Source Project
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

#ifndef TNT_UTILS_FIXEDCAPACITYFACTOR_H
#define TNT_UTILS_FIXEDCAPACITYFACTOR_H

#include <utils/compressed_pair.h>
#include <utils/debug.h>
#include <utils/Panic.h>

#include <algorithm>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include <stddef.h>
#include <stdint.h>

namespace utils {

template<typename T, typename A = std::allocator<T>>
class UTILS_PUBLIC FixedCapacityVector {
public:
    using allocator_type = A;
    using value_type = T;
    using reference = T&;
    using const_reference = T const&;
    using size_type = uint32_t;
    using difference_type = int32_t;
    using pointer = T*;
    using const_pointer = T const*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    using const_reference_or_value = std::conditional_t<
            std::is_fundamental_v<T> || std::is_pointer_v<T>, value_type, const_reference>;

public:
    FixedCapacityVector() = default;

    explicit FixedCapacityVector(const allocator_type& allocator) noexcept
            : mCapacityAllocator({}, allocator) {
    }

    explicit FixedCapacityVector(size_type size, const allocator_type& allocator = allocator_type())
            : mCapacityAllocator(size, allocator),
              mSize(size) {
        mData = this->allocator().allocate(this->capacity());
        construct(begin(), end());
    }

    FixedCapacityVector(size_type size, const_reference_or_value value,
            const allocator_type& alloc = allocator_type())
            : mCapacityAllocator(size, alloc),
              mSize(size) {
        mData = this->allocator().allocate(this->capacity());
        construct(begin(), end(), value);
    }


    FixedCapacityVector(FixedCapacityVector const& rhs)
            : mCapacityAllocator(rhs.mCapacityAllocator),
              mSize(rhs.mSize) {
        mData = allocator().allocate(capacity());
        std::uninitialized_copy(rhs.begin(), rhs.end(), begin());
    }

    FixedCapacityVector(FixedCapacityVector&& rhs) noexcept {
        this->swap(rhs);
    }

    ~FixedCapacityVector() noexcept {
        destroy(begin(), end());
        allocator().deallocate(data(), size());
    }

    FixedCapacityVector& operator=(FixedCapacityVector const& rhs) {
        if (this != &rhs) {
            FixedCapacityVector t(rhs.capacity(), rhs.allocator());
            t.mSize = rhs.mSize;
            std::uninitialized_copy(rhs.begin(), rhs.end(), t.begin());
            this->swap(t);
        }
        return *this;
    }

    FixedCapacityVector& operator=(FixedCapacityVector&& rhs) noexcept {
        this->swap(rhs);
        return *this;
    }

    allocator_type get_allocator() const noexcept {
        return mCapacityAllocator.second();
    }

    // --------------------------------------------------------------------------------------------

    iterator begin() noexcept { return data(); }
    iterator end() noexcept { return data() + size(); }
    const_iterator begin() const noexcept { return data(); }
    const_iterator end() const noexcept { return data() + size(); }
    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    const_iterator cbegin() const noexcept { return begin(); }
    const_iterator cend() const noexcept { return end(); }
    const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    const_reverse_iterator crend() const noexcept { return rend(); }

    // --------------------------------------------------------------------------------------------

    size_type size() const noexcept { return mSize; }
    size_type capacity() const noexcept { return mCapacityAllocator.first(); }
    bool empty() const noexcept { return size() == 0; }

    // --------------------------------------------------------------------------------------------

    reference operator[](size_type n) noexcept {
        assert_invariant(n < size());
        return *(begin() + n);
    }

    const_reference operator[](size_type n) const noexcept {
        assert_invariant(n < size());
        return *(begin() + n);
    }

    reference front() noexcept { return *begin(); }
    const_reference front() const noexcept { return *begin(); }
    reference back() noexcept { return *(end() - 1); }
    const_reference back() const noexcept { return *(end() - 1); }
    value_type* data() noexcept { return mData; }
    const value_type* data() const noexcept { return mData; }

    // --------------------------------------------------------------------------------------------

    void push_back(const_reference_or_value v) {
        auto pos = assertCapacityForSize(size() + 1);
        std::allocator_traits<allocator_type>::construct(allocator(), pos, v);
    }

    template<typename ... ARGS>
    reference emplace_back(ARGS&& ... args) {
        auto pos = assertCapacityForSize(size() + 1);
        std::allocator_traits<allocator_type>::construct(allocator(), pos, std::forward<ARGS>(args)...);
        return *pos;
    }

    void pop_back() {
        assert_invariant(!empty());
        --mSize;
        destroy(end(), end() + 1);
    }

    iterator insert(const_iterator position, const_reference_or_value v) {
        auto pos = assertCapacityForSize(size() + 1);
        std::move_backward(const_cast<iterator>(position), pos, pos + 1);
        std::allocator_traits<allocator_type>::construct(allocator(), position, v);
    }

    iterator erase(const_iterator position) {
        destroy(position, position + 1);
        std::move(const_cast<iterator>(position) + 1, end(), const_cast<iterator>(position));
        --mSize;
        return const_cast<iterator>(position);
    }

    iterator erase(const_iterator first, const_iterator last) {
        destroy(first, last);
        std::move(const_cast<iterator>(last), end(), const_cast<iterator>(first));
        mSize -= std::distance(first, last);
        return const_cast<iterator>(first);
    }

    void clear() noexcept {
        destroy(begin(), end());
        mSize = 0;
    }

    void resize(size_type count) {
        ASSERT_PRECONDITION(capacity() >= count,
                "capacity exceeded: requested %lu, available %lu.",
                (unsigned long)count, (unsigned long)capacity());
        if constexpr(!std::is_trivially_constructible_v<value_type> ||
                     !std::is_trivially_destructible_v<value_type>) {
            resize_non_trivial(count);
        } else {
            mSize = count;
        }
    }

    void resize(size_type count, const_reference_or_value v) {
        ASSERT_PRECONDITION(capacity() >= count,
                "capacity exceeded: requested %lu, available %lu.",
                (unsigned long)count, (unsigned long)capacity());
        resize_non_trivial(count, v);
    }

    void swap(FixedCapacityVector& other) {
        using std::swap;
        swap(mData, other.mData);
        swap(mSize, other.mSize);
        mCapacityAllocator.swap(other.mCapacityAllocator);
    }

    void reserve(size_type c) {
        ASSERT_PRECONDITION(c > capacity(), "capacity can't be lowered.");
        FixedCapacityVector t(c, allocator());
        t.mSize = size();
        std::uninitialized_copy(begin(), end(), t.begin());
        this->swap(t);
    }

private:
    allocator_type& allocator() noexcept {
        return mCapacityAllocator.second();
    }

    iterator assertCapacityForSize(size_type s) {
        ASSERT_PRECONDITION(capacity() >= s,
                "capacity exceeded: requested size %lu, available capacity %lu.",
                (unsigned long)s, (unsigned long)capacity());
        iterator e = end();
        mSize = s;
        return e;
    }

    inline void construct(iterator first, iterator last) noexcept {
        if constexpr(!std::is_trivially_constructible_v<value_type>) {
            construct_non_trivial(first, last);
        }
    }

    void construct(iterator first, iterator last, const_reference_or_value proto) noexcept {
        while (first != last) {
            std::allocator_traits<allocator_type>::construct(allocator(), first++, proto);
        }
    }

    inline void destroy(iterator first, iterator last) noexcept {
        if constexpr(!std::is_trivially_destructible_v<value_type>) {
            destroy_non_trivial(first, last);
        }
    }

    // should this be NOINLINE?
    void construct_non_trivial(iterator first, iterator last) noexcept {
        while (first != last) {
            std::allocator_traits<allocator_type>::construct(allocator(), first++);
        }
    }

    // should this be NOINLINE?
    void destroy_non_trivial(iterator first, iterator last) noexcept {
        while (first != last) {
            std::allocator_traits<allocator_type>::destroy(allocator(), --last);
        }
    }

    // should this be NOINLINE?
    void resize_non_trivial(size_type count) {
        if (count > size()) {
            construct(begin() + size(), begin() + count);
        } else if (count < size()) {
            destroy(begin() + count, begin() + size());
        }
        mSize = count;
    }

    // should this be NOINLINE?
    void resize_non_trivial(size_type count, const_reference_or_value v) {
        if (count > size()) {
            construct(begin() + size(), begin() + count, v);
        } else if (count < size()) {
            destroy(begin() + count, begin() + size());
        }
        mSize = count;
    }

    template<typename TYPE>
    class SizeTypeWrapper {
        TYPE value{};
    public:
        SizeTypeWrapper() noexcept = default;
        SizeTypeWrapper(SizeTypeWrapper const& rhs) noexcept = default;
        explicit  SizeTypeWrapper(TYPE value) noexcept : value(value) { }
        SizeTypeWrapper operator=(TYPE rhs) noexcept { value = rhs; return *this; }
        operator TYPE() const noexcept { return value; }
    };

    pointer mData{};
    size_type mSize{};
    compressed_pair<SizeTypeWrapper<size_type>, allocator_type> mCapacityAllocator{};
};

} // namespace utils

#endif //TNT_UTILS_FIXEDCAPACITYFACTOR_H
