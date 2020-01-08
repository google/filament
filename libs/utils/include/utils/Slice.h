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

#include <algorithm>
#include <atomic>

#include <assert.h>
#include <stddef.h>

#include <utils/compiler.h>

namespace utils {

/*
 * A fixed-size slice of a container
 */
template <typename T, typename SIZE_TYPE = uint32_t>
class Slice {
public:
    using iterator = T*;
    using const_iterator = T const*;
    using value_type = T;
    using reference = T&;
    using const_reference = T const&;
    using pointer = T*;
    using const_pointer = T const*;
    using size_type = SIZE_TYPE;

    Slice() noexcept = default;

    template <typename Iter>
    Slice(Iter begin, Iter end) noexcept
            : mBegin(iterator(begin)),
              mEndOffset(size_type(iterator(end)-iterator(begin))) {
    }

    template <typename Iter>
    Slice(Iter begin, size_type count) noexcept
            : mBegin(begin), mEndOffset(size_type(count)) {
    }

    Slice(Slice const& rhs) noexcept = default;
    Slice(Slice&& rhs) noexcept = default;
    Slice& operator=(Slice const& rhs) noexcept = default;
    Slice& operator=(Slice&& rhs) noexcept = default;

    template<typename Iter>
    void set(Iter begin, size_type count) UTILS_RESTRICT noexcept {
        mBegin = &*begin;
        mEndOffset = size_type(count);
    }

    template<typename Iter>
    void set(Iter begin, Iter end) UTILS_RESTRICT noexcept {
        mBegin = &*begin;
        mEndOffset = size_type(end - begin);
    }

    void swap(Slice& rhs) UTILS_RESTRICT noexcept {
        std::swap(mBegin, rhs.mBegin);
        std::swap(mEndOffset, rhs.mEndOffset);
    }

    void clear() UTILS_RESTRICT noexcept {
        mBegin = nullptr;
        mEndOffset = 0;
    }

    // size
    size_t size() const UTILS_RESTRICT noexcept { return mEndOffset; }
    size_t sizeInBytes() const UTILS_RESTRICT noexcept { return size() * sizeof(T); }
    bool empty() const UTILS_RESTRICT noexcept { return size() == 0; }

    // iterators
    iterator begin() UTILS_RESTRICT noexcept { return mBegin; }
    const_iterator begin() const UTILS_RESTRICT noexcept { return mBegin; }
    const_iterator cbegin() const UTILS_RESTRICT noexcept { return this->begin(); }
    iterator end() UTILS_RESTRICT noexcept { return &mBegin[mEndOffset]; }
    const_iterator end() const UTILS_RESTRICT noexcept { return  &mBegin[mEndOffset]; }
    const_iterator cend() const UTILS_RESTRICT noexcept { return this->end(); }

    // data access
    reference operator[](size_t n) UTILS_RESTRICT noexcept {
        assert(n < size());
        return mBegin[n];
    }

    const_reference operator[](size_t n) const UTILS_RESTRICT noexcept {
        assert(n < size());
        return mBegin[n];
    }

    reference at(size_t n) UTILS_RESTRICT noexcept {
        return operator[](n);
    }

    const_reference at(size_t n) const UTILS_RESTRICT noexcept {
        return operator[](n);
    }

    reference front() UTILS_RESTRICT noexcept {
        assert(!empty());
        return *mBegin;
    }

    const_reference front() const UTILS_RESTRICT noexcept {
        assert(!empty());
        return *mBegin;
    }

    reference back() UTILS_RESTRICT noexcept {
        assert(!empty());
        return *(this->end() - 1);
    }

    const_reference back() const UTILS_RESTRICT noexcept {
        assert(!empty());
        return *(this->end() - 1);
    }

    pointer data() UTILS_RESTRICT noexcept {
        return this->begin();
    }

    const_pointer data() const UTILS_RESTRICT noexcept {
        return this->begin();
    }

protected:
    iterator mBegin = nullptr;
    size_type mEndOffset = 0;
};

/*
 * A fixed-capacity (but growable) slice of a container
 */
template<typename T, typename SIZE_TYPE = uint32_t>
class UTILS_PRIVATE GrowingSlice : public Slice<T, SIZE_TYPE> {
public:
    using iterator          = typename Slice<T, SIZE_TYPE>::iterator;
    using const_iterator    = typename Slice<T, SIZE_TYPE>::const_iterator;
    using value_type        = typename Slice<T, SIZE_TYPE>::value_type;
    using reference         = typename Slice<T, SIZE_TYPE>::reference;
    using const_reference   = typename Slice<T, SIZE_TYPE>::const_reference;
    using pointer           = typename Slice<T, SIZE_TYPE>::pointer;
    using const_pointer     = typename Slice<T, SIZE_TYPE>::const_pointer;
    using size_type         = typename Slice<T, SIZE_TYPE>::size_type;

    GrowingSlice() noexcept = default;
    GrowingSlice(GrowingSlice const& rhs) noexcept = default;
    GrowingSlice(GrowingSlice&& rhs) noexcept = default;

    template<typename Iter>
    GrowingSlice(Iter begin, size_type count) noexcept
            : Slice<T, SIZE_TYPE>(begin, size_type(0)),
              mCapOffset(count) {
    }

    GrowingSlice& operator=(GrowingSlice const& rhs) noexcept = default;
    GrowingSlice& operator=(GrowingSlice&& rhs) noexcept = default;

    // size
    size_t remain() const UTILS_RESTRICT noexcept { return mCapOffset - this->mEndOffset; }
    size_t capacity() const UTILS_RESTRICT noexcept { return mCapOffset; }

    template<typename Iter>
    void set(Iter begin, size_type count) UTILS_RESTRICT noexcept {
        this->Slice<T, SIZE_TYPE>::set(begin, count);
        mCapOffset = count;
    }

    template<typename Iter>
    void set(Iter begin, Iter end) UTILS_RESTRICT noexcept {
        this->Slice<T, SIZE_TYPE>::set(begin, end);
        mCapOffset = size_type(end - begin);
    }

    void swap(GrowingSlice& rhs) UTILS_RESTRICT noexcept {
        Slice<T, SIZE_TYPE>::swap(rhs);
        std::swap(mCapOffset, rhs.mCapOffset);
    }

    void clear() UTILS_RESTRICT noexcept {
        this->mEndOffset = 0;
    }

    void resize(size_type count) UTILS_RESTRICT noexcept {
        assert(count < mCapOffset);
        this->mEndOffset = size_type(count);
    }

    T* grow(size_type count) UTILS_RESTRICT noexcept {
        assert(this->size() + count <= mCapOffset);
        size_t offset = this->mEndOffset;
        this->mEndOffset += count;
        return this->mBegin + offset;
    }

    // data access
    void push_back(T const& item) UTILS_RESTRICT noexcept {
        T* const p = this->grow(1);
        *p = item;
    }

    void push_back(T&& item) UTILS_RESTRICT noexcept {
        T* const p = this->grow(1);
        *p = std::move(item);
    }

    template<typename ... ARGS>
    void emplace_back(ARGS&& ... args) UTILS_RESTRICT noexcept {
        T* const p = this->grow(1);
        new(p) T(std::forward<ARGS>(args)...);
    }

private:
    // we use size_type == uint32_t to reduce the size on 64-bits machines
    size_type mCapOffset = 0;
};

// ------------------------------------------------------------------------------------------------

/*
 * A fixed-capacity (but atomically growable) slice of a container
 */
template <typename T, typename SIZE_TYPE = uint32_t>
class AtomicGrowingSlice {
public:
    using iterator = T*;
    using const_iterator = T const*;
    using value_type = T;
    using reference = T&;
    using const_reference = T const&;
    using pointer = T*;
    using const_pointer = T const*;
    using size_type = SIZE_TYPE;

    AtomicGrowingSlice() noexcept = default;

    template<typename Iter>
    AtomicGrowingSlice(Iter begin, Iter end) noexcept
            : mBegin(iterator(begin)),
              mEndOffset(0),
              mCapOffset(size_type(iterator(end) - iterator(begin))) {
    }

    template<typename Iter>
    AtomicGrowingSlice(Iter begin, size_type count) noexcept
            : mBegin(iterator(begin)), mEndOffset(0), mCapOffset(size_type(count)) {
    }

    template<typename Iter>
    void set(Iter begin, size_type count) noexcept {
        assert(mBegin == nullptr);
        mBegin = iterator(begin);
        mEndOffset.store(0, std::memory_order_relaxed);
        mCapOffset = count;
    }

    // clear
    void clear() noexcept {
        mEndOffset.store(0, std::memory_order_relaxed);
    }

    // size
    size_type size() const noexcept { return mEndOffset.load(std::memory_order_relaxed); }
    bool empty() const noexcept { return size() == 0; }
    size_type remain() const noexcept { return mCapOffset - size(); }
    size_type capacity() const noexcept { return mCapOffset; }

    // iterators
    iterator begin() noexcept { return mBegin; }
    const_iterator begin() const noexcept { return mBegin; }
    const_iterator cbegin() const noexcept { return begin(); }
    iterator end() noexcept { return &mBegin[size()]; }
    const_iterator end() const noexcept { return  &mBegin[size()]; }
    const_iterator cend() const noexcept { return end(); }

    // data access
    reference operator[](size_type n) noexcept {
        assert(n < size());
        return mBegin[n];
    }

    const_reference operator[](size_type n) const noexcept {
        assert(n < size());
        return mBegin[n];
    }

    reference at(size_type n) noexcept {
        return operator[](n);
    }

    const_reference at(size_type n) const noexcept {
        return operator[](n);
    }

    reference front() noexcept {
        assert(!empty());
        return *mBegin;
    }

    const_reference front() const noexcept {
        assert(!empty());
        return *mBegin;
    }

    reference back() noexcept {
        assert(!empty());
        return *(end() - 1);
    }

    const_reference back() const noexcept {
        assert(!empty());
        return *(end() - 1);
    }

    pointer data() noexcept {
        return begin();
    }

    const_pointer data() const noexcept {
        return begin();
    }

    T* grow(size_type count) noexcept {
        size_type offset = this->mEndOffset.load(std::memory_order_relaxed);
        do {
            if (UTILS_UNLIKELY(offset + count > mCapOffset)) {
                return nullptr;
            }
        } while (UTILS_UNLIKELY(!this->mEndOffset.compare_exchange_weak(offset, offset + count,
                std::memory_order_relaxed, std::memory_order_relaxed)));
        return this->mBegin + offset;
    }

    // data access
    void push_back(T const& item) noexcept {
        T* const p = this->grow(1);
        *p = item;
    }

    void push_back(T&& item) noexcept {
        T* const p = this->grow(1);
        *p = std::move(item);
    }

    template<typename ... ARGS>
    void emplace_back(ARGS&& ... args) noexcept {
        T* const p = this->grow(1);
        new(p) T(std::forward<ARGS>(args)...);
    }

private:
    iterator mBegin = nullptr;
    std::atomic<size_type> mEndOffset = ATOMIC_VAR_INIT(0);
    size_type mCapOffset = 0;
};

} // namespace utils

#endif // TNT_UTILS_SLICE_H
