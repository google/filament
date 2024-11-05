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
#include <utils/debug.h>
#include <utils/memalign.h>
#include <utils/Mutex.h>

#include <atomic>
#include <cstddef>
#include <mutex>
#include <type_traits>

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>

namespace utils {

namespace pointermath {

template <typename P, typename T>
static inline P* add(P* a, T b) noexcept {
    return (P*)(uintptr_t(a) + uintptr_t(b));
}

template <typename P>
static inline P* align(P* p, size_t alignment) noexcept {
    // alignment must be a power-of-two
    assert_invariant(alignment && !(alignment & alignment-1));
    return (P*)((uintptr_t(p) + alignment - 1) & ~(alignment - 1));
}

template <typename P>
static inline P* align(P* p, size_t alignment, size_t offset) noexcept {
    P* const r = align(add(p, offset), alignment);
    assert_invariant(r >= add(p, offset));
    return r;
}

}

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
    void* alloc(size_t size, size_t alignment = alignof(std::max_align_t), size_t extra = 0) UTILS_RESTRICT {
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
        assert_invariant(p >= mBegin && p < end());
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

    void free(void*, size_t) UTILS_RESTRICT noexcept { }

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
    void* alloc(size_t size, size_t alignment = alignof(std::max_align_t)) {
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
        LinearAllocatorWithFallback::reset();
    }

    void* alloc(size_t size, size_t alignment = alignof(std::max_align_t));

    void *getCurrent() noexcept {
        return LinearAllocator::getCurrent();
    }

    void rewind(void* p) noexcept {
        if (p >= LinearAllocator::base() && p < LinearAllocator::end()) {
            LinearAllocator::rewind(p);
        }
    }

    void reset() noexcept;

    void free(void*, size_t) noexcept { }

    bool isHeapAllocation(void* p) const noexcept {
        return p < LinearAllocator::base() || p >= LinearAllocator::end();
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
        assert_invariant(!mHead || mHead >= mBegin && mHead < mEnd);
        return head;
    }

    void push(void* p) noexcept {
        assert_invariant(p);
        assert_invariant(p >= mBegin && p < mEnd);
        // TODO: assert this is one of our pointer (i.e.: it's address match one of ours)
        Node* const head = static_cast<Node*>(p);
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

    // TSAN complains about a race on the memory used by the atomic mHead and the user
    // (see b/377369108). There is a race, but it is handled below.
    UTILS_NO_SANITIZE_THREAD
    void* pop() noexcept {
        Node* const pStorage = mStorage;

        HeadPtr currentHead = mHead.load();
        while (currentHead.offset >= 0) {
            // The value of "pNext" we load here might already contain application data if another
            // thread raced ahead of us. But in that case, the computed "newHead" will be discarded
            // since compare_exchange_weak fails. Then this thread will loop with the updated
            // value of currentHead, and try again.
            Node* const pNext = pStorage[currentHead.offset].next.load(std::memory_order_relaxed);
            const HeadPtr newHead{ pNext ? int32_t(pNext - pStorage) : -1, currentHead.tag + 1 };
            // In the rare case that the other thread that raced ahead of us already returned the 
            // same mHead we just loaded, but it now has a different "next" value, the tag field will not 
            // match, and compare_exchange_weak will fail and prevent that particular race condition.
            if (mHead.compare_exchange_weak(currentHead, newHead)) {
                // This assert needs to occur after we have validated that there was no race condition
                // Otherwise, next might already contain application data, if another thread
                // raced ahead of us after we loaded mHead, but before we loaded mHead->next.
                assert_invariant(!pNext || pNext >= pStorage);
                break;
            }
        }
        void* p = (currentHead.offset >= 0) ? (pStorage + currentHead.offset) : nullptr;
        assert_invariant(!p || p >= pStorage);
        return p;
    }

    void push(void* p) noexcept {
        Node* const storage = mStorage;
        assert_invariant(p && p >= storage);
        Node* const node = static_cast<Node*>(p);
        HeadPtr currentHead = mHead.load();
        HeadPtr newHead = { int32_t(node - storage), currentHead.tag + 1 };
        do {
            newHead.tag = currentHead.tag + 1;
            Node* const n = (currentHead.offset >= 0) ? (storage + currentHead.offset) : nullptr;
            node->next.store(n, std::memory_order_relaxed);
        } while(!mHead.compare_exchange_weak(currentHead, newHead));
    }

    void* getFirst() noexcept {
        return mStorage + mHead.load(std::memory_order_relaxed).offset;
    }

    struct Node {
        // This should be a regular (non-atomic) pointer, but this causes TSAN to complain
        // about a data-race that exists but is benin. We always use this atomic<> in
        // relaxed mode.
        // The data race TSAN complains about is when a pop() is interrupted by a
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
        std::atomic<Node*> next;
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
    void* alloc(size_t size = ELEMENT_SIZE,
                size_t alignment = ALIGNMENT, size_t offset = OFFSET) noexcept {
        assert_invariant(size <= ELEMENT_SIZE);
        assert_invariant(alignment <= ALIGNMENT);
        assert_invariant(offset == OFFSET);
        return mFreeList.pop();
    }

    void free(void* p, size_t = ELEMENT_SIZE) noexcept {
        mFreeList.push(p);
    }

    constexpr size_t getSize() const noexcept { return ELEMENT_SIZE; }

    PoolAllocator(void* begin, void* end) noexcept
        : mFreeList(begin, end, ELEMENT_SIZE, ALIGNMENT, OFFSET) {
    }

    PoolAllocator(void* begin, size_t size) noexcept
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

    void *getCurrent() noexcept {
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

    PoolAllocatorWithFallback(void* begin, size_t size) noexcept
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
        assert_invariant(p);
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

    explicit HeapArea(size_t size) {
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
    Untracked(const char* name, void* base, size_t size) noexcept {
        (void)name, void(base), (void)size;
    }
    void onAlloc(void* p, size_t size, size_t alignment, size_t extra) noexcept {
        (void)p, (void)size, (void)alignment, (void)extra;
    }
    void onFree(void* p, size_t = 0) noexcept { (void)p; }
    void onReset() noexcept { }
    void onRewind(void* addr) noexcept { (void)addr; }
};

// This just track the max memory usage and logs it in the destructor
struct HighWatermark {
    HighWatermark() noexcept = default;
    HighWatermark(const char* name, void* base, size_t size) noexcept
        : mName(name), mBase(base), mSize(uint32_t(size)) { }
    ~HighWatermark() noexcept;
    void onAlloc(void* p, size_t size, size_t alignment, size_t extra) noexcept;
    void onFree(void* p, size_t size) noexcept;
    void onReset() noexcept;
    void onRewind(void const* addr) noexcept;
    uint32_t getHighWatermark() const noexcept { return mHighWaterMark; }
protected:
    const char* mName = nullptr;
    void* mBase = nullptr;
    uint32_t mSize = 0;
    uint32_t mCurrent = 0;
    uint32_t mHighWaterMark = 0;
};

// This just fills buffers with known values to help catch uninitialized access and use after free.
struct Debug {
    Debug() noexcept = default;
    Debug(const char* name, void* base, size_t size) noexcept
            : mName(name), mBase(base), mSize(uint32_t(size)) { }
    void onAlloc(void* p, size_t size, size_t alignment, size_t extra) noexcept;
    void onFree(void* p, size_t size) noexcept;
    void onReset() noexcept;
    void onRewind(void* addr) noexcept;
protected:
    const char* mName = nullptr;
    void* mBase = nullptr;
    uint32_t mSize = 0;
};

struct DebugAndHighWatermark : protected HighWatermark, protected Debug {
    DebugAndHighWatermark() noexcept = default;
    DebugAndHighWatermark(const char* name, void* base, size_t size) noexcept
            : HighWatermark(name, base, size), Debug(name, base, size) { }
    void onAlloc(void* p, size_t size, size_t alignment, size_t extra) noexcept {
        HighWatermark::onAlloc(p, size, alignment, extra);
        Debug::onAlloc(p, size, alignment, extra);
    }
    void onFree(void* p, size_t size) noexcept {
        HighWatermark::onFree(p, size);
        Debug::onFree(p, size);
    }
    void onReset() noexcept {
        HighWatermark::onReset();
        Debug::onReset();
    }
    void onRewind(void* addr) noexcept {
        HighWatermark::onRewind(addr);
        Debug::onRewind(addr);
    }
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

    template<typename ... ARGS>
    void* alloc(size_t size, size_t alignment, size_t extra, ARGS&& ... args) noexcept {
        std::lock_guard<LockingPolicy> guard(mLock);
        void* p = mAllocator.alloc(size, alignment, extra, std::forward<ARGS>(args) ...);
        mListener.onAlloc(p, size, alignment, extra);
        return p;
    }


    // allocate memory from arena with given size and alignment
    // (acceptable size/alignment may depend on the allocator provided)
    void* alloc(size_t size, size_t alignment, size_t extra) noexcept {
        std::lock_guard<LockingPolicy> guard(mLock);
        void* p = mAllocator.alloc(size, alignment, extra);
        mListener.onAlloc(p, size, alignment, extra);
        return p;
    }

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
    template <typename T,
            typename = typename std::enable_if<std::is_trivially_destructible<T>::value>::type>
    T* alloc(size_t count, size_t alignment, size_t extra) noexcept {
        return (T*)alloc(count * sizeof(T), alignment, extra);
    }

    template <typename T,
            typename = typename std::enable_if<std::is_trivially_destructible<T>::value>::type>
    T* alloc(size_t count, size_t alignment = alignof(T)) noexcept {
        return (T*)alloc(count * sizeof(T), alignment);
    }

    // some allocators require more parameters
    template<typename ... ARGS>
    void free(void* p, size_t size, ARGS&& ... args) noexcept {
        if (p) {
            std::lock_guard<LockingPolicy> guard(mLock);
            mListener.onFree(p, size);
            mAllocator.free(p, size, std::forward<ARGS>(args) ...);
        }
    }

    // some allocators require the size of the allocation for free
    void free(void* p, size_t size) noexcept {
        if (p) {
            std::lock_guard<LockingPolicy> guard(mLock);
            mListener.onFree(p, size);
            mAllocator.free(p, size);
        }
    }

    // return memory pointed by p to the arena
    // (actual behaviour may depend on allocator provided)
    void free(void* p) noexcept {
        if (p) {
            std::lock_guard<LockingPolicy> guard(mLock);
            mListener.onFree(p);
            mAllocator.free(p);
        }
    }

    // some allocators don't have a free() call, but a single reset() or rewind() instead
    void reset() noexcept {
        std::lock_guard<LockingPolicy> guard(mLock);
        mListener.onReset();
        mAllocator.reset();
    }

    void* getCurrent() noexcept { return mAllocator.getCurrent(); }

    void rewind(void *addr) noexcept {
        std::lock_guard<LockingPolicy> guard(mLock);
        mListener.onRewind(addr);
        mAllocator.rewind(addr);
    }

    // Allocate and construct an object
    template<typename T, size_t ALIGN = alignof(T), typename... ARGS>
    T* make(ARGS&& ... args) noexcept {
        void* const p = this->alloc(sizeof(T), ALIGN);
        return p ? new(p) T(std::forward<ARGS>(args)...) : nullptr;
    }

    // destroys an object created with make<T>() above, and frees associated memory
    template<typename T>
    void destroy(T* p) noexcept {
        if (p) {
            p->~T();
            this->free((void*)p, sizeof(T));
        }
    }

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

// This doesn't implement our allocator concept, because it's too risky to use this as an allocator
// in particular, doing ArenaScope<ArenaScope>.
template<typename ARENA>
class ArenaScope {

    struct Finalizer {
        void (*finalizer)(void* p) = nullptr;
        Finalizer* next = nullptr;
    };

    template <typename T>
    static void destruct(void* p) noexcept {
        static_cast<T*>(p)->~T();
    }

public:
    using Arena = ARENA;

    explicit ArenaScope(ARENA& allocator)
            : mArena(allocator), mRewind(allocator.getCurrent()) {
    }

    ArenaScope& operator=(const ArenaScope& rhs) = delete;
    ArenaScope(ArenaScope&& rhs) noexcept = delete;
    ArenaScope& operator=(ArenaScope&& rhs) noexcept = delete;

    ~ArenaScope() {
        // run the finalizer chain
        Finalizer* head = mFinalizerHead;
        while (head) {
            void* p = pointermath::add(head, sizeof(Finalizer));
            head->finalizer(p);
            head = head->next;
        }
        // ArenaScope works only with Arena that implements rewind()
        mArena.rewind(mRewind);
    }

    template<typename T, size_t ALIGN = alignof(T), typename... ARGS>
    T* make(ARGS&& ... args) noexcept {
        T* o = nullptr;
        if (std::is_trivially_destructible<T>::value) {
            o = mArena.template make<T, ALIGN>(std::forward<ARGS>(args)...);
        } else {
            void* const p = (Finalizer*)mArena.alloc(sizeof(T), ALIGN, sizeof(Finalizer));
            if (p != nullptr) {
                Finalizer* const f = static_cast<Finalizer*>(p) - 1;
                // constructor must be called before adding the dtor to the list
                // so that the ctor can allocate objects in a nested scope and have the
                // finalizers called in reverse order.
                o = new(p) T(std::forward<ARGS>(args)...);
                f->finalizer = &destruct<T>;
                f->next = mFinalizerHead;
                mFinalizerHead = f;
            }
        }
        return o;
    }

    void* allocate(size_t size, size_t alignment = 1) noexcept {
        return mArena.template alloc<uint8_t>(size, alignment, 0);
    }

    template <typename T>
    T* allocate(size_t size, size_t alignment = alignof(T), size_t extra = 0) noexcept {
        return mArena.template alloc<T>(size, alignment, extra);
    }

    // use with caution
    ARENA& getArena() noexcept { return mArena; }

private:
    ARENA& mArena;
    void* mRewind = nullptr;
    Finalizer* mFinalizerHead = nullptr;
};


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

    TYPE* allocate(std::size_t n) {
        auto p = static_cast<TYPE *>(mArena.alloc(n * sizeof(TYPE), alignof(TYPE)));
        assert_invariant(p);
        return p;
    }

    void deallocate(TYPE* p, std::size_t n) {
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
