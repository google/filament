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


#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include <atomic>
#include <mutex>

#include <utils/compiler.h>
#include <utils/memalign.h>

namespace utils {

namespace pointermath {

template <typename P, typename T>
static inline P* add(P* a, T b) noexcept {
    return (P*)(uintptr_t(a) + uintptr_t(b));
}

template <typename P>
static inline P* align(P* p, size_t alignment) noexcept {
    // alignment must be a power-of-two
    assert(alignment && !(alignment & alignment-1));
    return (P*)((uintptr_t(p) + alignment - 1) & ~(alignment - 1));
}

template <typename P>
static inline P* align(P* p, size_t alignment, size_t offset) noexcept {
    P* const r = align(add(p, offset), alignment);
    assert(pointermath::add(r, -offset) >= p);
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
        void* const p = pointermath::align(mCurrent, alignment, extra);
        void* const c = pointermath::add(p, size);
        bool success = c <= mEnd;
        mCurrent = success ? c : mCurrent;
        return success ? p : nullptr;
    }

    // API specific to this allocator

    void *getCurrent() UTILS_RESTRICT noexcept {
        return mCurrent;
    }

    // free memory back to the specified point
    void rewind(void* p) UTILS_RESTRICT noexcept {
        assert(p>=mBegin && p<mEnd);
        mCurrent = p;
    }

    // frees all allocated blocks
    void reset() UTILS_RESTRICT noexcept {
        rewind(mBegin);
    }

    size_t allocated() const UTILS_RESTRICT noexcept {
        return uintptr_t(mCurrent) - uintptr_t(mBegin);
    }

    size_t available() const UTILS_RESTRICT noexcept {
        return uintptr_t(mEnd) - uintptr_t(mCurrent);
    }

    void swap(LinearAllocator& rhs) noexcept;

    void *base() noexcept { return mBegin; }

    // LinearAllocator shouldn't have a free() method
    // it's only needed to be compatible with STLAllocator<> below
    void free(void*) UTILS_RESTRICT noexcept { }

private:
    void* mBegin = nullptr;
    void* mEnd = nullptr;
    void* mCurrent = nullptr;
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
    void* alloc(size_t size, size_t alignment = alignof(std::max_align_t), size_t extra = 0) {
        // this allocator doesn't support 'extra'
        assert(extra == 0);
        return aligned_alloc(size, alignment);
    }

    void free(void* p) noexcept {
        aligned_free(p);
    }

    // Allocators can't be copied
    HeapAllocator(const HeapAllocator& rhs) = delete;
    HeapAllocator& operator=(const HeapAllocator& rhs) = delete;

    // Allocators can be moved
    HeapAllocator(HeapAllocator&& rhs) noexcept = default;
    HeapAllocator& operator=(HeapAllocator&& rhs) noexcept = default;

    ~HeapAllocator() noexcept = default;

    void swap(HeapAllocator& rhs) noexcept { }
};

// ------------------------------------------------------------------------------------------------

class FreeListBase {
public:
    struct Node {
        Node* next;
    };
    static Node* init(void* begin, void* end,
            size_t elementSize, size_t alignment, size_t extra) noexcept;
};

class FreeList : private FreeListBase {
public:
    FreeList() noexcept = default;
    FreeList(void* begin, void* end, size_t elementSize, size_t alignment, size_t extra) noexcept;
    FreeList(const FreeList& rhs) = delete;
    FreeList& operator=(const FreeList& rhs) = delete;
    FreeList(FreeList&& rhs) noexcept = default;
    FreeList& operator=(FreeList&& rhs) noexcept = default;

    void* get() noexcept {
        Node* const head = mHead;
        mHead = head ? head->next : nullptr;
        // this could indicate a use after free
        assert(!mHead || mHead >= mBegin && mHead < mEnd);
        return head;
    }

    void put(void* p) noexcept {
        assert(p);
        assert(p >= mBegin && p < mEnd);
        // TODO: assert this is one of our pointer (i.e.: it's address match one of ours)
        Node* const head = static_cast<Node*>(p);
        head->next = mHead;
        mHead = head;
    }

    void *getCurrent() noexcept {
        return mHead;
    }

private:
    Node* mHead = nullptr;

#ifndef NDEBUG
    // These are needed only for debugging...
    void* mBegin = nullptr;
    void* mEnd = nullptr;
#endif
};

class AtomicFreeList : private FreeListBase {
public:
    AtomicFreeList() noexcept = default;
    AtomicFreeList(void* begin, void* end,
            size_t elementSize, size_t alignment, size_t extra) noexcept;
    AtomicFreeList(const FreeList& rhs) = delete;
    AtomicFreeList& operator=(const FreeList& rhs) = delete;

    void* get() noexcept {
        Node* head = mHead.load(std::memory_order_relaxed);
        while (head && !mHead.compare_exchange_weak(head, head->next,
                std::memory_order_release, std::memory_order_relaxed)) {
        }
        return head;
    }

    void put(void* p) noexcept {
        assert(p);
        Node* head = static_cast<Node*>(p);
        head->next = mHead.load(std::memory_order_relaxed);
        while (!mHead.compare_exchange_weak(head->next, head,
                std::memory_order_release, std::memory_order_relaxed)) {
        }
    }

    void* getCurrent() noexcept {
        return mHead.load(std::memory_order_relaxed);
    }

private:
    std::atomic<Node*> mHead;
};

// ------------------------------------------------------------------------------------------------

template <
        size_t ELEMENT_SIZE,
        size_t ALIGNMENT = alignof(std::max_align_t),
        size_t OFFSET = 0,
        typename FREELIST = FreeList>
class PoolAllocator {
    static_assert(ELEMENT_SIZE >= sizeof(void*), "ELEMENT_SIZE must accommodate at least a pointer");
public:
    // our allocator concept
    void* alloc(size_t size = ELEMENT_SIZE,
                size_t alignment = ALIGNMENT, size_t offset = OFFSET) noexcept {
        assert(size <= ELEMENT_SIZE);
        assert(alignment <= ALIGNMENT);
        assert(offset == OFFSET);
        return mFreeList.get();
    }

    void free(void* p) noexcept {
        mFreeList.put(p);
    }

    size_t getSize() const noexcept { return ELEMENT_SIZE; }

    PoolAllocator(void* begin, void* end) noexcept
        : mFreeList(begin, end, ELEMENT_SIZE, ALIGNMENT, OFFSET) {
    }

    template <typename AREA>
    explicit PoolAllocator(const AREA& area) noexcept
        : PoolAllocator(area.begin(), area.end()) {
    }

    // Allocators can't be copied
    PoolAllocator(const PoolAllocator& rhs) = delete;
    PoolAllocator& operator=(const PoolAllocator& rhs) = delete;

    PoolAllocator() noexcept = default;
    ~PoolAllocator() noexcept = default;

    // API specific to this allocator

    void *getCurrent() noexcept {
        return mFreeList.getCurrent();
    }

private:
    FREELIST mFreeList;
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
    size_t getSize() const noexcept { return uintptr_t(mEnd) - uintptr_t(mBegin); }

private:
    void* mBegin = nullptr;
    void* mEnd = nullptr;
};


// ------------------------------------------------------------------------------------------------
// Policies
// ------------------------------------------------------------------------------------------------

namespace LockingPolicy {

struct NoLock {
    void lock() noexcept { }
    void unlock() noexcept { }
};

class SpinLock {
    std::atomic_flag mLock = ATOMIC_FLAG_INIT;

public:
    void lock() noexcept {
        UTILS_PREFETCHW(&mLock);
#ifdef __ARM_ACLE
        // we signal an event on this CPU, so that the first yield() will be a no-op,
        // and falls through the test_and_set(). This is more efficient than a while { }
        // construct.
        UTILS_SIGNAL_EVENT();
        do {
            yield();
        } while (mLock.test_and_set(std::memory_order_acquire));
#else
        goto start;
        do {
            yield();
start: ;
        } while (mLock.test_and_set(std::memory_order_acquire));
#endif
    }

    void unlock() noexcept {
        mLock.clear(std::memory_order_release);
#ifdef __ARM_ARCH_7A__
        // on ARMv7a SEL is needed
        UTILS_SIGNAL_EVENT();
        // as well as a memory barrier is needed
        __dsb(0xA);     // ISHST = 0xA (b1010)
#else
        // on ARMv8 we could avoid the call to SE, but we'de need to write the
        // test_and_set() above by hand, so the WFE only happens without a STRX first.
        UTILS_BROADCAST_EVENT();
#endif
    }

private:
    inline void yield() noexcept {
        // on x86 call pause instruction, on ARM call WFE
        UTILS_WAIT_FOR_EVENT();
    }
};

using Mutex = std::mutex;

} // namespace LockingPolicy


namespace TrackingPolicy {

struct Untracked {
    Untracked() noexcept = default;
    Untracked(const char* name, size_t size) noexcept { }
    void onAlloc(void* p, size_t size, size_t alignment, size_t extra) noexcept { }
    void onFree(void* p, size_t = 0) noexcept { }
    void onReset() noexcept { }
    void onRewind(void* addr) noexcept { }
};

// This high watermark tracker works only with allocator that either implement
// free(void*, size_t), or reset() / rewind()

struct HighWatermark {
    HighWatermark() noexcept = default;
    HighWatermark(const char* name, size_t size) noexcept
            : mName(name), mSize(uint32_t(size)) { }
    ~HighWatermark() noexcept;
    void onAlloc(void* p, size_t size, size_t alignment, size_t extra) noexcept {
        if (!mBase) { mBase = p; }
        mCurrent += uint32_t(size);
        mHighWaterMark = mCurrent > mHighWaterMark ? mCurrent : mHighWaterMark;
    }
    void onFree(void* p, size_t size) noexcept { mCurrent -= uint32_t(size); }
    void onReset() noexcept {  mCurrent = 0; }
    void onRewind(void const* addr) noexcept { mCurrent = uint32_t(uintptr_t(addr) - uintptr_t(mBase)); }

private:
    const char* mName = nullptr;
    void* mBase = nullptr;
    uint32_t mSize = 0;
    uint32_t mCurrent = 0;
    uint32_t mHighWaterMark = 0;
};

} // namespace TrackingPolicy

// ------------------------------------------------------------------------------------------------
// Arenas
// ------------------------------------------------------------------------------------------------

template<typename AllocatorPolicy, typename LockingPolicy,
        typename TrackingPolicy = TrackingPolicy::Untracked>
class Arena {
public:

    Arena() = default;

    // construct an arena with a name and forward argument to its allocator
    template<typename ... ARGS>
    Arena(const char* name, size_t size, ARGS&& ... args)
            : mArea(size),
              mAllocator(mArea, std::forward<ARGS>(args) ... ),
              mListener(name, size),
              mArenaName(name) {
    }

    // allocate memory from arena with given size and alignment
    // (acceptable size/alignment may depend on the allocator provided)
    void* alloc(size_t size, size_t alignment = alignof(std::max_align_t), size_t extra = 0) noexcept {
        std::lock_guard<LockingPolicy> guard(mLock);
        void* p = mAllocator.alloc(size, alignment, extra);
        mListener.onAlloc(p, size, alignment, extra);
        return p;
    }

    // Allocate an array of trivially destructible objects
    // for safety, we disable the object-based alloc method if the object type is not
    // trivially destructible, since free() won't call the destructor and this is allocating
    // an array.
    template <typename T,
            typename = typename std::enable_if<std::is_trivially_destructible<T>::value>::type>
    T* alloc(size_t count, size_t alignment = alignof(T), size_t extra = 0) noexcept {
        return (T*)alloc(count * sizeof(T), alignment, extra);
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

    // some allocators require the size of the allocation for free
    void free(void* p, size_t size) noexcept {
        if (p) {
            std::lock_guard<LockingPolicy> guard(mLock);
            mListener.onFree(p, size);
            mAllocator.free(p, size);
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
            this->free((void*)p);
        }
    }

    char const* getName() const noexcept { return mArenaName; }

    AllocatorPolicy& getAllocator() noexcept { return mAllocator; }
    AllocatorPolicy const& getAllocator() const noexcept { return mAllocator; }

    TrackingPolicy& getListener() noexcept { return mListener; }
    TrackingPolicy const& getListener() const noexcept { return mListener; }

    HeapArea& getArea() noexcept { return mArea; }
    HeapArea const& getArea() const noexcept { return mArea; }

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

private:
    HeapArea mArea; // We might want to make that a template parameter too eventually.
    AllocatorPolicy mAllocator;
    LockingPolicy mLock;
    TrackingPolicy mListener;
    char const* mArenaName = nullptr;
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
        return mArena.template alloc(size, alignment, 0);
    }

    template <typename T>
    T* allocate(size_t size, size_t alignment = alignof(T), size_t extra = 0) noexcept {
        return mArena.template alloc<T>(size, alignment, extra);
    }

    // use with caution
    ARENA& getAllocator() noexcept { return mArena; }

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
    explicit STLAllocator(ARENA& arena) : mArena(arena) { }

    TYPE* allocate(std::size_t n) {
        return static_cast<TYPE *>(mArena.alloc(n * sizeof(n), alignof(TYPE)));
    }

    void deallocate(TYPE* p, std::size_t n) {
        mArena.free(p);
    }

private:
    template <typename T, typename U, typename A>
    friend bool operator==(const STLAllocator<T, A>& rhs, const STLAllocator<U, A>& lhs) {
        return &rhs.mArena == &lhs.mArena;
    }

    template <typename T, typename U, typename A>
    friend bool operator!=(const STLAllocator<T, A>& rhs, const STLAllocator<U, A>& lhs) {
        return !operator==(rhs, lhs);
    }

    ARENA& mArena;
};

} // namespace utils

#endif // TNT_UTILS_ALLOCATOR_H
