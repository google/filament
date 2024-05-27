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

#ifndef TNT_UTILS_FIXEDCAPACITYVECTOR_H
#define TNT_UTILS_FIXEDCAPACITYVECTOR_H

#include <utils/compiler.h>
#include <utils/compressed_pair.h>
#include <utils/Panic.h>

#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#ifndef NDEBUG
#define FILAMENT_FORCE_CAPACITY_CHECK true
#else
#define FILAMENT_FORCE_CAPACITY_CHECK false
#endif

namespace utils {

/**
 * FixedCapacityVector is (almost) a drop-in replacement for std::vector<> except it has a
 * fixed capacity decided at runtime. The vector storage is never reallocated unless reserve()
 * is called. Operations that add elements to the vector can fail if there is not enough
 * capacity.
 *
 * An empty vector with a given capacity is created with
 *   FixedCapacityVector<T>::with_capacity( capacity );
 *
 * NOTE: When passing an initial size into the FixedCapacityVector constructor, default construction
 * of the elements is skipped when their construction is trivial. This behavior is different from
 * std::vector. e.g., std::vector<int>(4) constructs 4 zeros while FixedCapacityVector<int>(4)
 * allocates 4 uninitialized values. Note that zero initialization is easily achieved by passing in
 * the optional value argument, e.g. FixedCapacityVector<int>(4, 0) or foo.resize(4, 0).
 */
template<typename T, typename A = std::allocator<T>, bool CapacityCheck = true>
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
    using storage_traits = std::allocator_traits<allocator_type>;

public:
    /** returns an empty vector with the specified capacity */
    static FixedCapacityVector with_capacity(
            size_type capacity, const allocator_type& allocator = allocator_type()) {
        return FixedCapacityVector(construct_with_capacity, capacity, allocator);
    }

    FixedCapacityVector() = default;

    explicit FixedCapacityVector(const allocator_type& allocator) noexcept
            : mCapacityAllocator({}, allocator) {
    }

    explicit FixedCapacityVector(size_type size, const allocator_type& allocator = allocator_type())
            : mSize(size),
              mCapacityAllocator(size, allocator) {
        mData = this->allocator().allocate(this->capacity());
        construct(begin(), end());
    }

    FixedCapacityVector(std::initializer_list<T> list,
            const allocator_type& alloc = allocator_type())
            : mSize(list.size()),
              mCapacityAllocator(list.size(), alloc) {
        mData = this->allocator().allocate(this->capacity());
        std::uninitialized_copy(list.begin(), list.end(), begin());
    }

    FixedCapacityVector(size_type size, const_reference value,
            const allocator_type& alloc = allocator_type())
            : mSize(size),
              mCapacityAllocator(size, alloc) {
        mData = this->allocator().allocate(this->capacity());
        construct(begin(), end(), value);
    }

    FixedCapacityVector(FixedCapacityVector const& rhs)
            : mSize(rhs.mSize),
              mCapacityAllocator(rhs.capacity(),
                    storage_traits::select_on_container_copy_construction(rhs.allocator())) {
        mData = allocator().allocate(capacity());
        std::uninitialized_copy(rhs.begin(), rhs.end(), begin());
    }

    FixedCapacityVector(FixedCapacityVector&& rhs) noexcept {
        this->swap(rhs);
    }

    ~FixedCapacityVector() noexcept {
        destroy(begin(), end());
        allocator().deallocate(data(), capacity());
    }

    FixedCapacityVector& operator=(FixedCapacityVector const& rhs) {
        if (this != &rhs) {
            FixedCapacityVector t(rhs);
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
    size_type max_size() const noexcept {
        return std::min(storage_traits::max_size(allocator()),
                std::numeric_limits<size_type>::max());
    }

    // --------------------------------------------------------------------------------------------

    reference operator[](size_type n) noexcept {
        assert(n < size());
        return *(begin() + n);
    }

    const_reference operator[](size_type n) const noexcept {
        assert(n < size());
        return *(begin() + n);
    }

    reference front() noexcept { return *begin(); }
    const_reference front() const noexcept { return *begin(); }
    reference back() noexcept { return *(end() - 1); }
    const_reference back() const noexcept { return *(end() - 1); }
    value_type* data() noexcept { return mData; }
    const value_type* data() const noexcept { return mData; }

    // --------------------------------------------------------------------------------------------

    void push_back(const_reference v) {
        auto pos = assertCapacityForSize(size() + 1);
        ++mSize;
        storage_traits::construct(allocator(), pos, v);
    }

    void push_back(value_type&& v) {
        auto pos = assertCapacityForSize(size() + 1);
        ++mSize;
        storage_traits::construct(allocator(), pos, std::move(v));
    }

    template<typename ... ARGS>
    reference emplace_back(ARGS&& ... args) {
        auto pos = assertCapacityForSize(size() + 1);
        ++mSize;
        storage_traits::construct(allocator(), pos, std::forward<ARGS>(args)...);
        return *pos;
    }

    void pop_back() {
        assert(!empty());
        --mSize;
        destroy(end(), end() + 1);
    }

    iterator insert(const_iterator position, const_reference v) {
        if (position == end()) {
            push_back(v);
        } else {
            assertCapacityForSize(size() + 1);
            pointer p = const_cast<pointer>(position);
            move_range(p, end(), p + 1);
            ++mSize;
            // here we handle inserting an element of this vector!
            const_pointer pv = std::addressof(v);
            if (p <= pv && pv < end()) {
                *p = *(pv + 1);
            } else {
                *p = v;
            }
        }
        return const_cast<iterator>(position);
    }

    iterator insert(const_iterator position, value_type&& v) {
        if (position == end()) {
            push_back(std::move(v));
        } else {
            assertCapacityForSize(size() + 1);
            pointer p = const_cast<pointer>(position);
            move_range(p, end(), p + 1);
            ++mSize;
            *p = std::move(v);
        }
        return const_cast<iterator>(position);
    }

    iterator erase(const_iterator pos) {
        assert(pos != end());
        return erase(pos, pos + 1);
    }

    iterator erase(const_iterator first, const_iterator last) {
        assert(first <= last);
        auto e = std::move(const_cast<iterator>(last), end(), const_cast<iterator>(first));
        destroy(e, end());
        mSize -= std::distance(first, last);
        return const_cast<iterator>(first);
    }

    void clear() noexcept {
        destroy(begin(), end());
        mSize = 0;
    }

    void resize(size_type count) {
        assertCapacityForSize(count);
        if constexpr(std::is_trivially_constructible_v<value_type> &&
                     std::is_trivially_destructible_v<value_type>) {
            // we check for triviality here so that the implementation could be non-inline
            mSize = count;
        } else {
            resize_non_trivial(count);
        }
    }

    void resize(size_type count, const_reference v) {
        assertCapacityForSize(count);
        resize_non_trivial(count, v);
    }

    void swap(FixedCapacityVector& other) {
        using std::swap;
        swap(mData, other.mData);
        swap(mSize, other.mSize);
        mCapacityAllocator.swap(other.mCapacityAllocator);
    }

    UTILS_NOINLINE
    void reserve(size_type c) {
        if (c > capacity()) {
            FixedCapacityVector t(construct_with_capacity, c, allocator());
            t.mSize = size();
            std::uninitialized_move(begin(), end(), t.begin());
            this->swap(t);
        }
    }

    UTILS_NOINLINE
    void shrink_to_fit() {
        if (size() < capacity()) {
            FixedCapacityVector t(construct_with_capacity, size(), allocator());
            t.mSize = size();
            std::uninitialized_move(begin(), end(), t.begin());
            this->swap(t);
        }
    }

private:
    enum construct_with_capacity_tag{ construct_with_capacity };

    FixedCapacityVector(construct_with_capacity_tag,
            size_type capacity, const allocator_type& allocator = allocator_type())
            : mCapacityAllocator(capacity, allocator) {
        mData = this->allocator().allocate(this->capacity());
    }

    allocator_type& allocator() noexcept {
        return mCapacityAllocator.second();
    }

    allocator_type const& allocator() const noexcept {
        return mCapacityAllocator.second();
    }

    iterator assertCapacityForSize(size_type s) {
        if constexpr(CapacityCheck || FILAMENT_FORCE_CAPACITY_CHECK) {
            FILAMENT_CHECK_PRECONDITION(capacity() >= s)
                    << "capacity exceeded: requested size " << (unsigned long)s
                    << "u, available capacity " << (unsigned long)capacity() << "u.";
        }
        return end();
    }

    inline void construct(iterator first, iterator last) noexcept {
        // we check for triviality here so that the implementation could be non-inline
        if constexpr(!std::is_trivially_constructible_v<value_type>) {
            construct_non_trivial(first, last);
        }
    }

    void construct(iterator first, iterator last, const_reference proto) noexcept {
        UTILS_NOUNROLL
        while (first != last) {
            storage_traits::construct(allocator(), first++, proto);
        }
    }

    // should this be NOINLINE?
    void construct_non_trivial(iterator first, iterator last) noexcept {
        UTILS_NOUNROLL
        while (first != last) {
            storage_traits::construct(allocator(), first++);
        }
    }


    inline void destroy(iterator first, iterator last) noexcept {
        // we check for triviality here so that the implementation could be non-inline
        if constexpr(!std::is_trivially_destructible_v<value_type>) {
            destroy_non_trivial(first, last);
        }
    }

    // should this be NOINLINE?
    void destroy_non_trivial(iterator first, iterator last) noexcept {
        UTILS_NOUNROLL
        while (first != last) {
            storage_traits::destroy(allocator(), --last);
        }
    }

    // should this be NOINLINE?
    void resize_non_trivial(size_type count) {
        if (count > size()) {
            construct(end(), begin() + count);
        } else if (count < size()) {
            destroy(begin() + count, end());
        }
        mSize = count;
    }

    // should this be NOINLINE?
    void resize_non_trivial(size_type count, const_reference v) {
        if (count > size()) {
            construct(end(), begin() + count, v);
        } else if (count < size()) {
            destroy(begin() + count, end());
        }
        mSize = count;
    }

    // should this be NOINLINE?
    void move_range(pointer s, pointer e, pointer to) {
        if constexpr(std::is_trivially_copy_assignable_v<value_type>
                && std::is_trivially_destructible_v<value_type>) {
            // this generates memmove -- which doesn't happen otherwise
            std::move_backward(s, e, to + std::distance(s, e));
        } else {
            pointer our_end = end();
            difference_type n = our_end - to;   // nb of elements to move by operator=
            pointer i = s + n;                  // 1st element to move by move ctor
            for (pointer d = our_end ; i < our_end ; ++i, ++d) {
                storage_traits::construct(allocator(), d, std::move(*i));
            }
            std::move_backward(s, s + n, our_end);
        }
    }

    template<typename TYPE>
    class SizeTypeWrapper {
        TYPE value{};
    public:
        SizeTypeWrapper() noexcept = default;
        SizeTypeWrapper(SizeTypeWrapper const& rhs) noexcept = default;
        explicit  SizeTypeWrapper(TYPE value) noexcept : value(value) { }
        SizeTypeWrapper& operator=(TYPE rhs) noexcept { value = rhs; return *this; }
        SizeTypeWrapper& operator=(SizeTypeWrapper& rhs) noexcept = delete;
        operator TYPE() const noexcept { return value; }
    };

    pointer mData{};
    size_type mSize{};
    compressed_pair<SizeTypeWrapper<size_type>, allocator_type> mCapacityAllocator{};
};

} // namespace utils

#endif // TNT_UTILS_FIXEDCAPACITYVECTOR_H
