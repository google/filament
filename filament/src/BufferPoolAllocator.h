/*
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BUFFERPOOLALLOCATOR_H
#define TNT_FILAMENT_BUFFERPOOLALLOCATOR_H

#include <utils/Allocator.h>
#include <utils/FixedCapacityVector.h>

#include <utility>

#include <stdint.h>
#include <stdlib.h>

namespace filament {

/*
 * A simple buffer pool-allocator. The pool has a fixed size and buffers held by the pool all have
 * the same size which is defined by the most recent largest allocation request -- so buffers in the
 * pool can only grow, unless the pool is reset. All buffers in the pool know their own size.
 */
template<size_t POOL_SIZE,
        size_t ALIGNMENT = alignof(std::max_align_t),
        typename AllocatorPolicy = utils::HeapAllocator,
        typename LockingPolicy = utils::LockingPolicy::NoLock>
class BufferPoolAllocator {
public:
    using size_type = uint32_t;

    BufferPoolAllocator() = default;

    // pool is not copyable
    BufferPoolAllocator(BufferPoolAllocator const& rhs) = delete;
    BufferPoolAllocator& operator=(BufferPoolAllocator const& rhs) = delete;

    // pool is movable
    BufferPoolAllocator(BufferPoolAllocator&& rhs) noexcept = default;
    BufferPoolAllocator& operator=(BufferPoolAllocator&& rhs) noexcept = default;

    // free all buffers in the pool
    ~BufferPoolAllocator() noexcept;

    // return a buffer of at least size bytes
    void* get(size_type size) noexcept;

    // return a buffer to the pool
    void put(void* buffer) noexcept;

    // empty the pool and reset the size to 0
    void reset() noexcept;

private:
    struct alignas(ALIGNMENT) Header {
        size_type size;
    };

    static constexpr size_t ALLOCATION_ROUNDING = 4096;

    void deallocate(Header const* p) noexcept {
        --mOutstandingBuffers;
        mAllocator.free((void*)p, p->size + sizeof(Header));
    }

    void clearInternal() noexcept {
        for (auto p: mEntries) {
            assert_invariant(p->size == mSize);
            deallocate(p);
        }
        mEntries.clear();
    }

    using Container = utils::FixedCapacityVector<Header*, std::allocator<Header*>, false>;
    size_type mSize = 0; // size of our buffers in the pool
    size_type mOutstandingBuffers = 0;
    Container mEntries = Container::with_capacity(POOL_SIZE);
    AllocatorPolicy mAllocator;
    LockingPolicy mLock;
};

template<size_t POOL_SIZE, size_t ALIGNMENT, typename AllocatorPolicy, typename LockingPolicy>
BufferPoolAllocator<POOL_SIZE, ALIGNMENT, AllocatorPolicy, LockingPolicy>::~BufferPoolAllocator() noexcept {
    clearInternal();
}

template<size_t POOL_SIZE, size_t ALIGNMENT, typename AllocatorPolicy, typename LockingPolicy>
void BufferPoolAllocator<POOL_SIZE, ALIGNMENT, AllocatorPolicy, LockingPolicy>::reset() noexcept {
    std::lock_guard<LockingPolicy> guard(mLock);
    clearInternal();
    mSize = 0;
}

template<size_t POOL_SIZE, size_t ALIGNMENT, typename AllocatorPolicy, typename LockingPolicy>
void* BufferPoolAllocator<POOL_SIZE, ALIGNMENT, AllocatorPolicy, LockingPolicy>::get(size_type const size) noexcept {
    std::lock_guard<LockingPolicy> guard(mLock);

    // if the requested size is larger that our buffers in the pool, we just empty the pool
    if (UTILS_UNLIKELY(size > mSize)) {
        clearInternal(); // free all buffers
        // round to 4K allocations to help cutting down on calling malloc.
        size_t roundedSize = ((size + sizeof(Header)) + (ALLOCATION_ROUNDING - 1)) & ~(ALLOCATION_ROUNDING - 1);
        mSize = roundedSize - sizeof(Header); // record the new buffer size
        assert_invariant(mSize >= size);
    }

    // if the pool is empty, allocate a new buffer of the pool buffer size (which may be
    // larger than the requested size).
    if (UTILS_UNLIKELY(mEntries.empty())) {
        ++mOutstandingBuffers;
        Header* p = (Header*)mAllocator.alloc(mSize + sizeof(Header), ALIGNMENT);
        p->size = mSize;
        return p + 1;
    }

    // if we have an entry in the pool we know it's at least of the requested size
    assert_invariant(mSize >= size);
    // return the last entry
    Header* p = mEntries.back();
    mEntries.pop_back();
    return p + 1;
}

template<size_t POOL_SIZE, size_t ALIGNMENT, typename AllocatorPolicy, typename LockingPolicy>
void BufferPoolAllocator<POOL_SIZE, ALIGNMENT, AllocatorPolicy, LockingPolicy>::put(void* buffer) noexcept {
    std::lock_guard<LockingPolicy> guard(mLock);

    // retrieve this buffer's header
    Header const* const p = static_cast<Header const*>(buffer) - 1;

    // if the returned buffer is smaller than the current pool buffer size, or, the pool
    // is full, just free that buffer.
    if (UTILS_UNLIKELY(mEntries.size() == mEntries.capacity() || p->size < mSize)) {
        deallocate(p);
        return;
    }

    // add this buffer to the pool
    assert_invariant(p->size == mSize);
    mEntries.push_back(const_cast<Header *>(p));
}

} // namespace filament

#endif // TNT_FILAMENT_BUFFERPOOLALLOCATOR_H
