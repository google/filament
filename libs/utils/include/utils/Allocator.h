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

#ifndef TNT_UTILS_ALLOCATOR_H
#define TNT_UTILS_ALLOCATOR_H

#include <utils/compiler.h>
#include <utils/memalign.h>
#include <utils/Mutex.h>

#include <atomic>
#include <cstddef>
#include <mutex>
#include <type_traits>
#include <utility>
#include <vector>

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

namespace utils {

namespace pointermath {

template<typename P, typename T>
static P* add(P* a, T b) noexcept {
    return (P*) (uintptr_t(a) + uintptr_t(b));
}

template<typename P>
static P* align(P* p, size_t const alignment) noexcept {
    // alignment must be a power-of-two
    assert(alignment && !(alignment & alignment-1));
    return (P*) ((uintptr_t(p) + alignment - 1) & ~(alignment - 1));
}

template<typename P>
static P* align(P* p, size_t alignment, size_t offset) noexcept {
    P* const r = align(add(p, offset), alignment);
    assert(r >= add(p, offset));
    return r;
}

}

namespace detail {

template<typename AlwaysVoid, typename Alloc, typename Ptr, typename Size, typename... Args>
struct has_variadic_free : std::false_type {};

template<typename Alloc, typename Ptr, typename Size, typename... Args>
struct has_variadic_free<
            std::void_t<decltype(
                std::declval<Alloc>().free(
                        std::declval<Ptr>(),
                        std::declval<Size>(),
                        std::declval<Args>()...
                        )
            )>,
            Alloc, Ptr, Size, Args...
        > : std::true_type {};

// Helper variable template for cleaner syntax
template<typename Alloc, typename Ptr, typename Size, typename... Args>
inline constexpr bool has_variadic_free_v = has_variadic_free<void, Alloc, Ptr, Size, Args...>::value;

// Detect reset()
template<typename T, typename = void>
struct has_reset : std::false_type {};

template<typename T>
struct has_reset<T, std::void_t<decltype(std::declval<T>().reset())>> : std::true_type {};

template<typename T>
inline constexpr bool has_reset_v = has_reset<T>::value;

// Detect rewind(void*)
template<typename T, typename = void>
struct has_rewind : std::false_type {};

template<typename T>
struct has_rewind<T, std::void_t<decltype(std::declval<T>().rewind(std::declval<void*>()))>> : std::true_type {};

template<typename T>
inline constexpr bool has_rewind_v = has_rewind<T>::value;

// Detect onLogicalFree(void*, size_t)
template<typename T, typename = void>
struct has_logical_free : std::false_type {};

template<typename T>
struct has_logical_free<T, std::void_t<decltype(std::declval<T>().onLogicalFree(std::declval<void*>(), std::declval<size_t>()))>> : std::true_type {};

template<typename T>
inline constexpr bool has_logical_free_v = has_logical_free<T>::value;

// Detect getActiveAllocationCount()
template<typename T, typename = void>
struct has_active_allocation_count : std::false_type {};

template<typename T>
struct has_active_allocation_count<T, std::void_t<decltype(std::declval<T>().getActiveAllocationCount())>> : std::true_type {};

template<typename T>
inline constexpr bool has_active_allocation_count_v = has_active_allocation_count<T>::value;

template<typename Tracking>
uint32_t getActiveAllocationCount(Tracking const& listener) noexcept {
    if constexpr (has_active_allocation_count_v<Tracking>) {
        return listener.getActiveAllocationCount();
    } else {
        return 0;
    }
}

// Detect getActiveAllocationBytes()
template<typename T, typename = void>
struct has_active_allocation_bytes : std::false_type {};

template<typename T>
struct has_active_allocation_bytes<T, std::void_t<decltype(std::declval<T>().getActiveAllocationBytes())>> : std::true_type {};

template<typename T>
inline constexpr bool has_active_allocation_bytes_v = has_active_allocation_bytes<T>::value;

template<typename Tracking>
uint32_t getActiveAllocationBytes(Tracking const& listener) noexcept {
    if constexpr (has_active_allocation_bytes_v<Tracking>) {
        return listener.getActiveAllocationBytes();
    } else {
        return 0;
    }
}

} // namespace detail

/* ------------------------------------------------------------------------------------------------
 * LinearAllocator
 *
 * + Allocates blocks linearly
 * + Cannot free individual blocks
 * + Can free top of memory back up to a specified point
 * + Doesn't call destructors
 * ------------------------------------------------------------------------------------------------
 */

class LinearAllocator {
public:
    // use memory area provided
    LinearAllocator(void* begin, void* end) noexcept;

    template <typename AREA>
    explicit LinearAllocator(const AREA& area) : LinearAllocator(area.begin(), area.end()) { }

    // Allocators can't be copied
    LinearAllocator(const LinearAllocator& rhs) = delete;
    LinearAllocator& operator=(const LinearAllocator& rhs) = delete;

    // Allocators can be moved
    LinearAllocator(LinearAllocator&& rhs) noexcept;
    LinearAllocator& operator=(LinearAllocator&& rhs) noexcept;

    ~LinearAllocator() noexcept = default;

    // our allocator concept
    void* alloc(size_t const size, size_t const alignment = alignof(std::max_align_t), size_t const extra = 0) UTILS_RESTRICT {
        // branch-less allocation
        void* const p = pointermath::align(current(), alignment, extra);
        void* const c = pointermath::add(p, size);
        bool const success = c <= end();
        set_current(success ? c : current());
        return success ? p : nullptr;
    }

    // API specific to this allocator
    void *getCurrent() UTILS_RESTRICT noexcept {
        return current();
    }

    // free memory back to the specified point
    void rewind(void* p) UTILS_RESTRICT noexcept {
        assert(p >= mBegin && p < end());
        set_current(p);
    }

    // frees all allocated blocks
    void reset() UTILS_RESTRICT noexcept {
        rewind(mBegin);
    }

    size_t allocated() const UTILS_RESTRICT noexcept {
        return mSize;
    }

    size_t available() const UTILS_RESTRICT noexcept {
        return mSize - mCur;
    }

    void swap(LinearAllocator& rhs) noexcept;

    void *base() noexcept { return mBegin; }
    void const *base() const noexcept { return mBegin; }

protected:
    void* end() UTILS_RESTRICT noexcept { return pointermath::add(mBegin, mSize); }
    void const* end() const UTILS_RESTRICT noexcept { return pointermath::add(mBegin, mSize); }

    void* current() UTILS_RESTRICT noexcept { return pointermath::add(mBegin, mCur); }
    void const* current() const UTILS_RESTRICT noexcept { return pointermath::add(mBegin, mCur); }

private:
    void set_current(void* p) UTILS_RESTRICT noexcept {
        mCur = uint32_t(uintptr_t(p) - uintptr_t(mBegin));
    }
    void* mBegin = nullptr;
    uint32_t mSize = 0;
    uint32_t mCur = 0;
};

/* ------------------------------------------------------------------------------------------------
 * HeapAllocator
 *
 * + uses malloc() for all allocations
 * + frees blocks with free()
 * ------------------------------------------------------------------------------------------------
 */
class HeapAllocator {
public:
    HeapAllocator() noexcept = default;

    template <typename AREA>
    explicit HeapAllocator(const AREA&) { }

    // our allocator concept
    void* alloc(size_t const size, size_t const alignment = alignof(std::max_align_t)) {
        return aligned_alloc(size, alignment);
    }

    void free(void* p) noexcept {
        aligned_free(p);
    }

    void free(void* p, size_t) noexcept {
        this->free(p);
    }

    ~HeapAllocator() noexcept = default;

    void swap(HeapAllocator&) noexcept { }
};

/* ------------------------------------------------------------------------------------------------
 * LinearAllocatorWithFallback
 *
 * This is a LinearAllocator that falls back to a HeapAllocator when allocation fail. The Heap
 * allocator memory is freed only when the LinearAllocator is reset or destroyed.
 * ------------------------------------------------------------------------------------------------
 */
class LinearAllocatorWithFallback : private LinearAllocator, private HeapAllocator {
    std::vector<void*> mHeapAllocations;
public:
    LinearAllocatorWithFallback(void* begin, void* end) noexcept
        : LinearAllocator(begin, end) {
    }

    template <typename AREA>
    explicit LinearAllocatorWithFallback(const AREA& area)
        : LinearAllocatorWithFallback(area.begin(), area.end()) {
    }

    ~LinearAllocatorWithFallback() noexcept {
        reset();
    }

    void* alloc(size_t size, size_t alignment = alignof(std::max_align_t));

    void *getCurrent() noexcept {
        return LinearAllocator::getCurrent();
    }

    void rewind(void* p) noexcept {
        if (p >= base() && p < end()) {
            LinearAllocator::rewind(p);
        }
    }

    void reset() noexcept;

    bool isHeapAllocation(void* p) const noexcept {
        return p < base() || p >= end();
    }
};

// ------------------------------------------------------------------------------------------------

class FreeList {
public:
    FreeList() noexcept = default;
    FreeList(void* begin, void* end, size_t elementSize, size_t alignment, size_t extra) noexcept;
    FreeList(const FreeList& rhs) = delete;
    FreeList& operator=(const FreeList& rhs) = delete;
    FreeList(FreeList&& rhs) noexcept = default;
    FreeList& operator=(FreeList&& rhs) noexcept = default;

    void* pop() noexcept {
        Node* const head = mHead;
        mHead = head ? head->next : nullptr;
        // this could indicate a use after free
        assert(!mHead || mHead >= mBegin && mHead < mEnd);
        return head;
    }

    void push(void* p) noexcept {
        assert(p);
        assert(p >= mBegin && p < mEnd);
        // we use placement-new to properly manage the lifetime
        // this is noop already under O1
        Node* const head = new (p) Node;
        head->next = mHead;
        mHead = head;
    }

    void *getFirst() noexcept {
        return mHead;
    }

    struct Node {
        Node* next;
    };

private:
    static Node* init(void* begin, void* end,
            size_t elementSize, size_t alignment, size_t extra) noexcept;

    Node* mHead = nullptr;

#ifndef NDEBUG
    // These are needed only for debugging...
    void* mBegin = nullptr;
    void* mEnd = nullptr;
#endif
};

class AtomicFreeList {
public:
    AtomicFreeList() noexcept = default;
    AtomicFreeList(void* begin, void* end,
            size_t elementSize, size_t alignment, size_t extra) noexcept;
    AtomicFreeList(const AtomicFreeList& rhs) = delete;
    AtomicFreeList& operator=(const AtomicFreeList& rhs) = delete;

    void* pop() noexcept {
        Node* const pStorage = mStorage;

        HeadPtr currentHead = mHead.load(std::memory_order_relaxed);
        while (currentHead.offset >= 0) {
            // The value of "pNext" we load here might already contain application data if another
            // thread raced ahead of us. But in that case, the computed "newHead" will be discarded
            // since compare_exchange_weak() fails. Then this thread will loop with the updated
            // value of currentHead, and try again.
            // TSAN complains if we don't use a local variable here.
            Node const node = pStorage[currentHead.offset];
            Node const* const pNext = node.next;
            const HeadPtr newHead{ pNext ? int32_t(pNext - pStorage) : -1, currentHead.tag + 1 };
            // In the rare case that the other thread that raced ahead of us already returned the
            // same mHead we just loaded, but it now has a different "next" value, the tag field
            // will not match, and compare_exchange_weak() will fail and prevent that particular
            // race condition.
            // acquire: no read/write can be reordered before this
            if (mHead.compare_exchange_weak(currentHead, newHead,
                    std::memory_order_acquire, std::memory_order_relaxed)) {
                // This assert needs to occur after we have validated that there was no race condition
                // Otherwise, next might already contain application data, if another thread
                // raced ahead of us after we loaded mHead, but before we loaded mHead->next.
                assert(!pNext || pNext >= pStorage);
                break;
            }
        }
        void* p = (currentHead.offset >= 0) ? (pStorage + currentHead.offset) : nullptr;
        assert(!p || p >= pStorage);
        return p;
    }

    void push(void* p) noexcept {
        Node* const storage = mStorage;
        assert(p && p >= storage);
        // we use placement-new to properly manage the lifetime
        // this is noop already under O1
        Node* const node = new (p) Node;
        HeadPtr currentHead = mHead.load(std::memory_order_relaxed);
        HeadPtr newHead = { int32_t(node - storage), currentHead.tag + 1 };
        do {
            newHead.tag = currentHead.tag + 1;
            Node* const pNext = (currentHead.offset >= 0) ? (storage + currentHead.offset) : nullptr;
            node->next = pNext; // could be a race with pop, corrected by CAS
        } while(!mHead.compare_exchange_weak(currentHead, newHead,
                std::memory_order_release, std::memory_order_relaxed));
        // release: no read/write can be reordered after this
    }

    void* getFirst() noexcept {
        return mStorage + mHead.load(std::memory_order_relaxed).offset;
    }

    struct Node {
        // There is a benign data race when a pop() is interrupted by a
        // pop() + push() just after mHead->next is read -- it appears as though it is written
        // without synchronization (by the push), however in that case, the pop's CAS will fail
        // and things will auto-correct.
        //
        //    Pop()                       |
        //     |                          |
        //   read head->next              |
        //     |                        pop()
        //     |                          |
        //     |                        read head->next
        //     |                         CAS, tag++
        //     |                          |
        //     |                        push()
        //     |                          |
        // [TSAN: data-race here]       write head->next
        //     |                         CAS, tag++
        //    CAS fails
        //     |
        //   read head->next
        //     |
        //    CAS, tag++
        //
        Node* next = nullptr;
    };

private:
    // This struct is using a 32-bit offset into the arena rather than
    // a direct pointer, because together with the 32-bit tag, it needs to 
    // fit into 8 bytes. If it was any larger, it would not be possible to
    // access it atomically.
    struct alignas(8) HeadPtr {
        int32_t offset;
        uint32_t tag;
    };

    std::atomic<HeadPtr> mHead{};

    Node* mStorage = nullptr;
};

// ------------------------------------------------------------------------------------------------

template <
        size_t ELEMENT_SIZE,
        size_t ALIGNMENT = alignof(std::max_align_t),
        size_t OFFSET = 0,
        typename FREELIST = FreeList>
class PoolAllocator {
    static_assert(ELEMENT_SIZE >= sizeof(typename FREELIST::Node),
            "ELEMENT_SIZE must accommodate at least a FreeList::Node");
public:
    // our allocator concept
    void* alloc(size_t const size = ELEMENT_SIZE,
                size_t const alignment = ALIGNMENT, size_t const offset = OFFSET) noexcept {
        assert(size <= ELEMENT_SIZE);
        assert(alignment <= ALIGNMENT);
        assert(offset == OFFSET);
        return mFreeList.pop();
    }

    void free(void* p, size_t = ELEMENT_SIZE) noexcept {
        mFreeList.push(p);
    }

    constexpr size_t getSize() const noexcept { return ELEMENT_SIZE; }

    PoolAllocator(void* begin, void* end) noexcept
        : mFreeList(begin, end, ELEMENT_SIZE, ALIGNMENT, OFFSET) {
    }

    PoolAllocator(void* begin, size_t const size) noexcept
        : PoolAllocator(begin, static_cast<char *>(begin) + size) {
    }

    template<typename AREA>
    explicit PoolAllocator(const AREA& area) noexcept
        : PoolAllocator(area.begin(), area.end()) {
    }

    // Allocators can't be copied
    PoolAllocator(const PoolAllocator& rhs) = delete;
    PoolAllocator& operator=(const PoolAllocator& rhs) = delete;

    // Allocators can be moved
    PoolAllocator(PoolAllocator&& rhs) = default;
    PoolAllocator& operator=(PoolAllocator&& rhs) = default;

    PoolAllocator() noexcept = default;
    ~PoolAllocator() noexcept = default;

    // API specific to this allocator

    void *getBase() noexcept {
        return mFreeList.getFirst();
    }

private:
    FREELIST mFreeList;
};

template <
        size_t ELEMENT_SIZE,
        size_t ALIGNMENT = alignof(std::max_align_t),
        typename FREELIST = FreeList>
class PoolAllocatorWithFallback :
        private PoolAllocator<ELEMENT_SIZE, ALIGNMENT, 0, FREELIST>,
        private HeapAllocator {
    using PoolAllocator = PoolAllocator<ELEMENT_SIZE, ALIGNMENT, 0, FREELIST>;
    void* mBegin;
    void* mEnd;
public:
    PoolAllocatorWithFallback(void* begin, void* end) noexcept
            : PoolAllocator(begin, end), mBegin(begin), mEnd(end) {
    }

    PoolAllocatorWithFallback(void* begin, size_t const size) noexcept
            : PoolAllocatorWithFallback(begin, static_cast<char*>(begin) + size) {
    }

    template<typename AREA>
    explicit PoolAllocatorWithFallback(const AREA& area) noexcept
            : PoolAllocatorWithFallback(area.begin(), area.end()) {
    }

    bool isHeapAllocation(void* p) const noexcept {
        return  p < mBegin || p >= mEnd;
    }

    // our allocator concept
    void* alloc(size_t size = ELEMENT_SIZE, size_t alignment = ALIGNMENT) noexcept {
        void* p = PoolAllocator::alloc(size, alignment);
        if (UTILS_UNLIKELY(!p)) {
            p = HeapAllocator::alloc(size, alignment);
        }
        assert(p);
        return p;
    }

    void free(void* p, size_t size) noexcept {
        if (UTILS_LIKELY(!isHeapAllocation(p))) {
            PoolAllocator::free(p, size);
        } else {
            HeapAllocator::free(p);
        }
    }
};

#define UTILS_MAX(a,b) ((a) > (b) ? (a) : (b))

template <typename T, size_t OFFSET = 0>
using ObjectPoolAllocator = PoolAllocator<sizeof(T),
        UTILS_MAX(alignof(FreeList), alignof(T)), OFFSET>;

template <typename T, size_t OFFSET = 0>
using ThreadSafeObjectPoolAllocator = PoolAllocator<sizeof(T),
        UTILS_MAX(alignof(FreeList), alignof(T)), OFFSET, AtomicFreeList>;


// ------------------------------------------------------------------------------------------------
// Areas
// ------------------------------------------------------------------------------------------------

namespace AreaPolicy {

class StaticArea {
public:
    StaticArea() noexcept = default;

    StaticArea(void* b, void* e) noexcept
            : mBegin(b), mEnd(e) {
    }

    ~StaticArea() noexcept = default;

    StaticArea(const StaticArea& rhs) = default;
    StaticArea& operator=(const StaticArea& rhs) = default;
    StaticArea(StaticArea&& rhs) noexcept = default;
    StaticArea& operator=(StaticArea&& rhs) noexcept = default;

    void* data() const noexcept { return mBegin; }
    void* begin() const noexcept { return mBegin; }
    void* end() const noexcept { return mEnd; }
    size_t size() const noexcept { return uintptr_t(mEnd) - uintptr_t(mBegin); }

    friend void swap(StaticArea& lhs, StaticArea& rhs) noexcept {
        using std::swap;
        swap(lhs.mBegin, rhs.mBegin);
        swap(lhs.mEnd, rhs.mEnd);
    }

private:
    void* mBegin = nullptr;
    void* mEnd = nullptr;
};

class HeapArea {
public:
    HeapArea() noexcept = default;

    explicit HeapArea(size_t const size) {
        if (size) {
            // TODO: policy committing memory
            mBegin = malloc(size);
            mEnd = pointermath::add(mBegin, size);
        }
    }

    ~HeapArea() noexcept {
        // TODO: policy for returning memory to system
        free(mBegin);
    }

    HeapArea(const HeapArea& rhs) = delete;
    HeapArea& operator=(const HeapArea& rhs) = delete;
    HeapArea(HeapArea&& rhs) noexcept = delete;
    HeapArea& operator=(HeapArea&& rhs) noexcept = delete;

    void* data() const noexcept { return mBegin; }
    void* begin() const noexcept { return mBegin; }
    void* end() const noexcept { return mEnd; }
    size_t size() const noexcept { return uintptr_t(mEnd) - uintptr_t(mBegin); }

    friend void swap(HeapArea& lhs, HeapArea& rhs) noexcept {
        using std::swap;
        swap(lhs.mBegin, rhs.mBegin);
        swap(lhs.mEnd, rhs.mEnd);
    }

private:
    void* mBegin = nullptr;
    void* mEnd = nullptr;
};

class NullArea {
public:
    void* data() const noexcept { return nullptr; }
    size_t size() const noexcept { return 0; }
};

} // namespace AreaPolicy

// ------------------------------------------------------------------------------------------------
// Policies
// ------------------------------------------------------------------------------------------------

namespace LockingPolicy {

struct NoLock {
    void lock() noexcept { }
    void unlock() noexcept { }
};

using Mutex = utils::Mutex;

} // namespace LockingPolicy


namespace TrackingPolicy {

// default no-op tracker
struct Untracked {
    Untracked() noexcept = default;
    Untracked(const char* name, void const* base, size_t const size) noexcept {
        (void)name, void(base), (void)size;
    }
    void onAlloc(void const* p, size_t const size, size_t const alignment, size_t const extra) noexcept {
        (void)p, (void)size, (void)alignment, (void)extra;
    }
    void onFree(void const* p, size_t const size) noexcept { (void)p; (void)size; }
    void onLogicalFree(void const* p, size_t const size) noexcept { (void)p; (void)size; }
    void onReset() noexcept { }
    void onRewind(void const* addr) noexcept { (void)addr; }
    uint32_t getActiveAllocationCount() const noexcept { return 0; }
    uint32_t getActiveAllocationBytes() const noexcept { return 0; }
};

// This just track the max memory usage and logs it in the destructor
struct HighWatermark {
    HighWatermark() noexcept = default;
    HighWatermark(char const* name, void const* base, size_t const size) noexcept
        : mName(name), mBase(base), mSize(uint32_t(size)) { }
    ~HighWatermark() noexcept;
    void onAlloc(void* p, size_t size, size_t alignment, size_t extra) noexcept;
    void onFree(void* p, size_t size) noexcept;
    void onLogicalFree(void const* p, size_t const size) noexcept { (void)p; (void)size; }
    void onReset() noexcept;
    void onRewind(void const* addr) noexcept;
    uint32_t getHighWatermark() const noexcept { return mHighWaterMark; }
    uint32_t getActiveAllocationCount() const noexcept { return 0; }
    uint32_t getActiveAllocationBytes() const noexcept { return 0; }
protected:
    char const* const mName = nullptr;
    void const* const mBase = nullptr;
    uint32_t const mSize = 0;
    uint32_t mCurrent = 0;
    uint32_t mHighWaterMark = 0;
};

// This just fills buffers with known values to help catch uninitialized access and use after free.
struct Debug {
    Debug() noexcept = default;
    Debug(const char* name, void* base, size_t const size) noexcept
            : mName(name), mBase(base), mSize(uint32_t(size)) { }
    void onAlloc(void* p, size_t size, size_t alignment, size_t extra) noexcept;
    void onFree(void* p, size_t size) noexcept;
    void onLogicalFree(void* p, size_t size) noexcept;
    void onReset() noexcept;
    void onRewind(void* addr) noexcept;
    uint32_t getActiveAllocationCount() const noexcept { return mActiveAllocationCount; }
    uint32_t getActiveAllocationBytes() const noexcept { return mActiveAllocationBytes; }
protected:
    char const* const mName = nullptr;
    void* const mBase = nullptr;
    uint32_t const mSize = 0;
    uint32_t mActiveAllocationCount = 0;
    uint32_t mActiveAllocationBytes = 0;
};

struct DebugAndHighWatermark : protected HighWatermark, protected Debug {
    DebugAndHighWatermark() noexcept = default;
    DebugAndHighWatermark(const char* name, void* base, size_t const size) noexcept
            : HighWatermark(name, base, size), Debug(name, base, size) { }
    void onAlloc(void* p, size_t const size, size_t const alignment, size_t const extra) noexcept {
        HighWatermark::onAlloc(p, size, alignment, extra);
        Debug::onAlloc(p, size, alignment, extra);
    }
    void onFree(void* p, size_t const size) noexcept {
        HighWatermark::onFree(p, size);
        Debug::onFree(p, size);
    }
    void onLogicalFree(void* p, size_t const size) noexcept {
        HighWatermark::onLogicalFree(p, size);
        Debug::onLogicalFree(p, size);
    }
    void onReset() noexcept {
        HighWatermark::onReset();
        Debug::onReset();
    }
    void onRewind(void* addr) noexcept {
        HighWatermark::onRewind(addr);
        Debug::onRewind(addr);
    }
    uint32_t getHighWatermark() const noexcept { return HighWatermark::getHighWatermark(); }
    uint32_t getActiveAllocationCount() const noexcept { return Debug::getActiveAllocationCount(); }
    uint32_t getActiveAllocationBytes() const noexcept { return Debug::getActiveAllocationBytes(); }
};

} // namespace TrackingPolicy

// ------------------------------------------------------------------------------------------------
// Arenas
// ------------------------------------------------------------------------------------------------

template<typename AllocatorPolicy, typename LockingPolicy,
        typename TrackingPolicy = TrackingPolicy::Untracked,
        typename AreaPolicy = AreaPolicy::HeapArea>
class Arena {
public:

    Arena() = default;

    // construct an arena with a name and forward argument to its allocator
    template<typename ... ARGS>
    Arena(const char* name, size_t size, ARGS&& ... args)
            : mArenaName(name),
              mArea(size),
              mAllocator(mArea, std::forward<ARGS>(args) ... ),
              mListener(name, mArea.data(), mArea.size()) {
    }

    template<typename ... ARGS>
    Arena(const char* name, AreaPolicy&& area, ARGS&& ... args)
            : mArenaName(name),
              mArea(std::forward<AreaPolicy>(area)),
              mAllocator(mArea, std::forward<ARGS>(args) ... ),
              mListener(name, mArea.data(), mArea.size()) {
    }

    // -----------------------------------------------------------------------------

    template<typename ... ARGS>
    void* alloc(size_t size, size_t alignment, size_t extra, ARGS&& ... args) noexcept {
        std::lock_guard<LockingPolicy> guard(mLock);
        void* p = mAllocator.alloc(size, alignment, extra, std::forward<ARGS>(args) ...);
        mListener.onAlloc(p, size, alignment, extra);
        return p;
    }

    // some allocators don't support the "extra" parameter
    void* alloc(size_t size, size_t alignment = alignof(std::max_align_t)) noexcept {
        std::lock_guard<LockingPolicy> guard(mLock);
        void* p = mAllocator.alloc(size, alignment);
        mListener.onAlloc(p, size, alignment, 0);
        return p;
    }

    // Allocate an array of trivially destructible objects
    // for safety, we disable the object-based alloc method if the object type is not
    // trivially destructible, since free() won't call the destructor and this is allocating
    // an array.
    template<typename T, typename = std::enable_if_t<std::is_trivially_destructible_v<T>>>
    T* alloc(size_t const count, size_t const alignment, size_t const extra) noexcept {
        size_t size;
        if (UTILS_UNLIKELY(UTILS_MUL_OVERFLOW(count, sizeof(T), &size))) {
            return nullptr;
        }
        return static_cast<T*>(alloc(size, alignment, extra));
    }

    template<typename T, typename = std::enable_if_t<std::is_trivially_destructible_v<T>>>
    T* alloc(size_t const count, size_t const alignment = alignof(T)) noexcept {
        size_t size;
        if (UTILS_UNLIKELY(UTILS_MUL_OVERFLOW(count, sizeof(T), &size))) {
            return nullptr;
        }
        return static_cast<T*>(alloc(size, alignment));
    }

    // some allocators require more parameters
    template<typename... ARGS>
    void free(void* p, size_t size, ARGS&&... args) noexcept {
        if (p) {
            std::lock_guard<LockingPolicy> guard(mLock);
            if constexpr (detail::has_variadic_free_v<decltype(mAllocator), void*, size_t, ARGS...>) {
                mListener.onFree(p, size);
                mAllocator.free(p, size, std::forward<ARGS>(args)...);
            } else {
                if constexpr (detail::has_logical_free_v<TrackingPolicy>) {
                    mListener.onLogicalFree(p, size);
                }
            }
        }
    }

    // Only present if the allocator has reset()
    template<typename A = AllocatorPolicy, std::enable_if_t<detail::has_reset_v<A>, int> = 0>
    void reset() noexcept {
        std::lock_guard<LockingPolicy> guard(mLock);
        mListener.onReset();
        mAllocator.reset();
    }

    // Only present if the allocator has rewind(void*)
    template<typename A = AllocatorPolicy, std::enable_if_t<detail::has_rewind_v<A>, int> = 0>
    void rewind(void *addr) noexcept {
        std::lock_guard<LockingPolicy> guard(mLock);
        mListener.onRewind(addr);
        mAllocator.rewind(addr);
    }

    // -----------------------------------------------------------------------------

    // Allocate and construct an object
    template<typename T, size_t ALIGN = alignof(T), typename... ARGS>
    T* make(ARGS&& ... args) noexcept {
        void* const p = this->alloc(sizeof(T), ALIGN);
        return p ? new(p) T(std::forward<ARGS>(args)...) : nullptr;
    }

    template<typename T>
    void destroy(T* p, size_t const size) noexcept {
        if (p) {
            static_assert(!std::is_polymorphic_v<T> || std::has_virtual_destructor_v<T>,
                    "Polymorphic types must declare a virtual destructor");
            std::destroy_at(p);
            this->free((void*)p, size);
        }
    }

    // Visible if T is NOT polymorphic OR if T is final
    template<typename T, std::enable_if_t<!std::is_polymorphic_v<T> || std::is_final_v<T>, int> = 0>
    void destroy(T* p) noexcept {
        if (p) {
            std::destroy_at(p);
            this->free((void*)p, sizeof(T));
        }
    }

    // -----------------------------------------------------------------------------

    char const* getName() const noexcept { return mArenaName; }

    AllocatorPolicy& getAllocator() noexcept { return mAllocator; }
    AllocatorPolicy const& getAllocator() const noexcept { return mAllocator; }

    TrackingPolicy& getListener() noexcept { return mListener; }
    TrackingPolicy const& getListener() const noexcept { return mListener; }

    AreaPolicy& getArea() noexcept { return mArea; }
    AreaPolicy const& getArea() const noexcept { return mArea; }

    void setListener(TrackingPolicy listener) noexcept {
        std::swap(mListener, listener);
    }

    template <typename ... ARGS>
    void emplaceListener(ARGS&& ... args) noexcept {
        mListener.~TrackingPolicy();
        new (&mListener) TrackingPolicy(std::forward<ARGS>(args)...);
    }

    // An arena can't be copied
    Arena(Arena const& rhs) noexcept = delete;
    Arena& operator=(Arena const& rhs) noexcept = delete;

    friend void swap(Arena& lhs, Arena& rhs) noexcept {
        using std::swap;
        swap(lhs.mArea, rhs.mArea);
        swap(lhs.mAllocator, rhs.mAllocator);
        swap(lhs.mLock, rhs.mLock);
        swap(lhs.mListener, rhs.mListener);
        swap(lhs.mArenaName, rhs.mArenaName);
    }

private:
    char const* mArenaName = nullptr;
    AreaPolicy mArea;
    // note: we should use something like compressed_pair for the members below
    AllocatorPolicy mAllocator;
    LockingPolicy mLock;
    TrackingPolicy mListener;
};

// ------------------------------------------------------------------------------------------------

template<typename TrackingPolicy = TrackingPolicy::Untracked>
using HeapArena = Arena<HeapAllocator, LockingPolicy::NoLock, TrackingPolicy>;

// ------------------------------------------------------------------------------------------------

/*
 * ArenaScope wraps an Arena whose allocator supports rewind() and automatically rewinds
 * the allocator when destroyed.
 */
template<typename AllocatorPolicy,
        typename LockingPolicy,
        typename TrackingPolicy = TrackingPolicy::Untracked,
        typename AreaPolicy = AreaPolicy::HeapArea>
class ArenaScope {
public:
    using Arena = Arena<AllocatorPolicy, LockingPolicy, TrackingPolicy, AreaPolicy>;

    explicit ArenaScope(Arena& allocator)
            : mArena(allocator),
              mRewind(allocator.getAllocator().getCurrent())
#ifndef NDEBUG
            , mInitialCount(detail::getActiveAllocationCount(allocator.getListener())),
              mInitialBytes(detail::getActiveAllocationBytes(allocator.getListener()))
#endif
    {
    }

    ArenaScope& operator=(const ArenaScope& rhs) = delete;
    ArenaScope(ArenaScope&& rhs) noexcept = delete;
    ArenaScope& operator=(ArenaScope&& rhs) noexcept = delete;

    ~ArenaScope() {
#ifndef NDEBUG
        assert(detail::getActiveAllocationCount(mArena.getListener()) == mInitialCount);
        assert(detail::getActiveAllocationBytes(mArena.getListener()) == mInitialBytes);
#endif
        // ArenaScope works only with Arena that implements rewind()
        mArena.rewind(mRewind);
    }

    Arena& getArena() noexcept { return mArena; }
    Arena const& getArena() const noexcept { return mArena; }

private:
    Arena& mArena;
    void* mRewind = nullptr;
#ifndef NDEBUG
    uint32_t mInitialCount = 0;
    uint32_t mInitialBytes = 0;
#endif
};

// ------------------------------------------------------------------------------------------------

template <typename TYPE, typename ARENA>
class STLAllocator {
public:
    using value_type = TYPE;
    using pointer = TYPE*;
    using const_pointer = const TYPE*;
    using reference = TYPE&;
    using const_reference = const TYPE&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using propagate_on_container_move_assignment = std::true_type;
    using is_always_equal = std::true_type;

    template<typename OTHER>
    struct rebind { using other = STLAllocator<OTHER, ARENA>; };

public:
    // we don't make this explicit, so that we can initialize a vector using a STLAllocator
    // from an Arena, avoiding having to repeat the vector type.
    STLAllocator(ARENA& arena) : mArena(arena) { } // NOLINT(google-explicit-constructor)

    template<typename U>
    explicit STLAllocator(STLAllocator<U, ARENA> const& rhs) : mArena(rhs.mArena) { }

    TYPE* allocate(std::size_t const n) {
        auto p = static_cast<TYPE *>(mArena.alloc(n * sizeof(TYPE), alignof(TYPE)));
        assert(p);
        return p;
    }

    void deallocate(TYPE* p, std::size_t const n) {
        mArena.free(p, n * sizeof(TYPE));
    }

    // these should be out-of-class friends, but this doesn't seem to work with some compilers
    // which complain about multiple definition each time a STLAllocator<> is instantiated.
    template <typename U, typename A>
    bool operator==(const STLAllocator<U, A>& rhs) const noexcept {
        return std::addressof(mArena) == std::addressof(rhs.mArena);
    }

    template <typename U, typename A>
    bool operator!=(const STLAllocator<U, A>& rhs) const noexcept {
        return !operator==(rhs);
    }

private:
    template<typename U, typename A>
    friend class STLAllocator;

    ARENA& mArena;
};

} // namespace utils

#endif // TNT_UTILS_ALLOCATOR_H
