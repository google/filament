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

#ifndef TNT_FILAMENT_BACKEND_PRIVATE_HANDLEALLOCATOR_H
#define TNT_FILAMENT_BACKEND_PRIVATE_HANDLEALLOCATOR_H

#include <backend/Handle.h>

#include <utils/Allocator.h>
#include <utils/CString.h>
#include <utils/Log.h>
#include <utils/Panic.h>
#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/ostream.h>

#include <tsl/robin_map.h>

#include <cstddef>
#include <exception>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include <stddef.h>
#include <stdint.h>

#define HandleAllocatorGL  HandleAllocator<32,  96, 136>    // ~4520 / pool / MiB
#define HandleAllocatorVK  HandleAllocator<64, 160, 600>    // ~1820 / pool / MiB
#define HandleAllocatorMTL HandleAllocator<32,  64, 552>    // ~1660 / pool / MiB

namespace filament::backend {

/*
 * A utility class to efficiently allocate and manage Handle<>
 */
template<size_t P0, size_t P1, size_t P2>
class HandleAllocator {
public:
    HandleAllocator(const char* name, size_t size, bool disableUseAfterFreeCheck) noexcept;
    HandleAllocator(HandleAllocator const& rhs) = delete;
    HandleAllocator& operator=(HandleAllocator const& rhs) = delete;
    ~HandleAllocator();

    /*
     * Constructs a D object and returns a Handle<D>
     *
     * e.g.:
     *  struct ConcreteTexture : public HwTexture {
     *      ConcreteTexture(int w, int h);
     *  };
     *  Handle<ConcreteTexture> h = allocateAndConstruct(w, h);
     *
     */
    template<typename D, typename ... ARGS>
    Handle<D> allocateAndConstruct(ARGS&& ... args) {
        Handle<D> h{ allocateHandle<D>() };
        D* addr = handle_cast<D*>(h);
        new(addr) D(std::forward<ARGS>(args)...);
        return h;
    }

    /*
     * Allocates (without constructing) a D object and returns a Handle<D>
     *
     * e.g.:
     *  struct ConcreteTexture : public HwTexture {
     *      ConcreteTexture(int w, int h);
     *  };
     *  Handle<ConcreteTexture> h = allocate();
     *
     */
    template<typename D>
    Handle<D> allocate() noexcept {
        Handle<D> h{ allocateHandle<D>() };
        return h;
    }


    /*
     * Destroys the object D at Handle<B> and construct a new D in its place
     * e.g.:
     *  Handle<ConcreteTexture> h = allocateAndConstruct(w, h);
     *  ConcreteTexture* p = reconstruct(h, w, h);
     */
    template<typename D, typename B, typename ... ARGS>
    typename std::enable_if_t<std::is_base_of_v<B, D>, D>*
    destroyAndConstruct(Handle<B> const& handle, ARGS&& ... args) {
        assert_invariant(handle);
        D* addr = handle_cast<D*>(const_cast<Handle<B>&>(handle));
        assert_invariant(addr);
        // currently we implement construct<> with dtor+ctor, we could use operator= also
        // but all our dtors are trivial, ~D() is actually a noop.
        addr->~D();
        new(addr) D(std::forward<ARGS>(args)...);
        return addr;
    }

    /*
     * Construct a new D at Handle<B>
     * e.g.:
     *  Handle<ConcreteTexture> h = allocate();
     *  ConcreteTexture* p = construct(h, w, h);
     */
    template<typename D, typename B, typename ... ARGS>
    typename std::enable_if_t<std::is_base_of_v<B, D>, D>*
    construct(Handle<B> const& handle, ARGS&& ... args) noexcept {
        assert_invariant(handle);
        D* addr = handle_cast<D*>(const_cast<Handle<B>&>(handle));
        assert_invariant(addr);
        new(addr) D(std::forward<ARGS>(args)...);
        return addr;
    }

    /*
     * Destroy the object D at Handle<B> and frees Handle<B>
     * e.g.:
     *      Handle<HwTexture> h = ...;
     *      ConcreteTexture* p = handle_cast<ConcreteTexture*>(h);
     *      deallocate(h, p);
     */
    template <typename B, typename D,
            typename = typename std::enable_if_t<std::is_base_of_v<B, D>, D>>
    void deallocate(Handle<B>& handle, D const* p) noexcept {
        // allow to destroy the nullptr, similarly to operator delete
        if (p) {
            p->~D();
            deallocateHandle<D>(handle.getId());
        }
    }

    /*
     * Destroy the object D at Handle<B> and frees Handle<B>
     * e.g.:
     *      Handle<HwTexture> h = ...;
     *      deallocate(h);
     */
    template<typename D>
    void deallocate(Handle<D>& handle) noexcept {
        D const* d = handle_cast<const D*>(handle);
        deallocate(handle, d);
    }

    /*
     * returns a D* from a Handle<B>. B must be a base of D.
     * e.g.:
     *      Handle<HwTexture> h = ...;
     *      ConcreteTexture* p = handle_cast<ConcreteTexture*>(h);
     */
    template<typename Dp, typename B>
    inline typename std::enable_if_t<
            std::is_pointer_v<Dp> &&
            std::is_base_of_v<B, typename std::remove_pointer_t<Dp>>, Dp>
    handle_cast(Handle<B>& handle) {
        assert_invariant(handle);
        auto [p, tag] = handleToPointer(handle.getId());

        if (isPoolHandle(handle.getId())) {
            // check for pool handle use-after-free
            if (UTILS_UNLIKELY(!mUseAfterFreeCheckDisabled)) {
                uint8_t const age = (tag & HANDLE_AGE_MASK) >> HANDLE_AGE_SHIFT;
                auto const pNode = static_cast<typename Allocator::Node*>(p);
                uint8_t const expectedAge = pNode[-1].age;
                // getHandleTag() is only called if the check fails.
                FILAMENT_CHECK_POSTCONDITION(expectedAge == age)
                        << "use-after-free of Handle with id=" << handle.getId()
                        << ", tag=" << getHandleTag(handle.getId()).c_str_safe();
            }
        } else {
            // check for heap handle use-after-free
            if (UTILS_UNLIKELY(!mUseAfterFreeCheckDisabled)) {
                uint8_t const index = (handle.getId() & HANDLE_INDEX_MASK);
                // if we've already handed out this handle index before, it's definitely a
                // use-after-free, otherwise it's probably just a corrupted handle
                if (index < mId) {
                    FILAMENT_CHECK_POSTCONDITION(p != nullptr)
                            << "use-after-free of heap Handle with id=" << handle.getId()
                            << ", tag=" << getHandleTag(handle.getId()).c_str_safe();
                } else {
                    FILAMENT_CHECK_POSTCONDITION(p != nullptr)
                            << "corrupted heap Handle with id=" << handle.getId()
                            << ", tag=" << getHandleTag(handle.getId()).c_str_safe();
                }
            }
        }

        return static_cast<Dp>(p);
    }

    template<typename B>
    bool is_valid(Handle<B>& handle) {
        if (!handle) {
            // null handles are invalid
            return false;
        }
        auto [p, tag] = handleToPointer(handle.getId());
        if (isPoolHandle(handle.getId())) {
            uint8_t const age = (tag & HANDLE_AGE_MASK) >> HANDLE_AGE_SHIFT;
            auto const pNode = static_cast<typename Allocator::Node*>(p);
            uint8_t const expectedAge = pNode[-1].age;
            return expectedAge == age;
        }
        return p != nullptr;
    }

    template<typename Dp, typename B>
    inline typename std::enable_if_t<
            std::is_pointer_v<Dp> &&
            std::is_base_of_v<B, typename std::remove_pointer_t<Dp>>, Dp>
    handle_cast(Handle<B> const& handle) {
        return handle_cast<Dp>(const_cast<Handle<B>&>(handle));
    }

    void associateTagToHandle(HandleBase::HandleId id, utils::CString&& tag) noexcept {
        // TODO: for now, only pool handles check for use-after-free, so we only keep tags for
        // those
        if (isPoolHandle(id)) {
            // Truncate the age to get the debug tag
            uint32_t const key = id & ~(HANDLE_DEBUG_TAG_MASK ^ HANDLE_AGE_MASK);
            // This line is the costly part. In the future, we could potentially use a custom
            // allocator.
            mDebugTags[key] = std::move(tag);
        }
    }

    utils::CString getHandleTag(HandleBase::HandleId id) const noexcept {
        if (!isPoolHandle(id)) {
            return "(no tag)";
        }
        uint32_t const key = id & ~(HANDLE_DEBUG_TAG_MASK ^ HANDLE_AGE_MASK);
        if (auto pos = mDebugTags.find(key); pos != mDebugTags.end()) {
            return pos->second;
        }
        return "(no tag)";
    }

private:

    template<typename D>
    static constexpr size_t getBucketSize() noexcept {
        if constexpr (sizeof(D) <= P0) { return P0; }
        if constexpr (sizeof(D) <= P1) { return P1; }
        static_assert(sizeof(D) <= P2);
        return P2;
    }

    class Allocator {
        friend class HandleAllocator;
        static constexpr size_t MIN_ALIGNMENT = alignof(std::max_align_t);
        struct Node { uint8_t age; };
        // Note: using the `extra` parameter of PoolAllocator<>, even with a 1-byte structure,
        // generally increases all pool allocations by 8-bytes because of alignment restrictions.
        template<size_t SIZE>
        using Pool = utils::PoolAllocator<SIZE, MIN_ALIGNMENT, sizeof(Node)>;
        Pool<P0> mPool0;
        Pool<P1> mPool1;
        Pool<P2> mPool2;
        UTILS_UNUSED_IN_RELEASE const utils::AreaPolicy::HeapArea& mArea;
        bool mUseAfterFreeCheckDisabled;
    public:
        explicit Allocator(const utils::AreaPolicy::HeapArea& area, bool disableUseAfterFreeCheck);

        static constexpr size_t getAlignment() noexcept { return MIN_ALIGNMENT; }

        // this is in fact always called with a constexpr size argument
        [[nodiscard]] inline void* alloc(size_t size, size_t, size_t, uint8_t* outAge) noexcept {
            void* p = nullptr;
            if      (size <= mPool0.getSize()) p = mPool0.alloc(size);
            else if (size <= mPool1.getSize()) p = mPool1.alloc(size);
            else if (size <= mPool2.getSize()) p = mPool2.alloc(size);
            if (UTILS_LIKELY(p)) {
                Node const* const pNode = static_cast<Node const*>(p);
                // we are guaranteed to have at least sizeof<Node> bytes of extra storage before
                // the allocation address.
                *outAge = pNode[-1].age;
            }
            return p;
        }

        // this is in fact always called with a constexpr size argument
        inline void free(void* p, size_t size, uint8_t age) noexcept {
            assert_invariant(p >= mArea.begin() && (char*)p + size <= (char*)mArea.end());

            // check for double-free
            Node* const pNode = static_cast<Node*>(p);
            uint8_t& expectedAge = pNode[-1].age;
            if (UTILS_UNLIKELY(!mUseAfterFreeCheckDisabled)) {
                FILAMENT_CHECK_POSTCONDITION(expectedAge == age) <<
                        "double-free of Handle of size " << size << " at " << p;
            }
            expectedAge = (expectedAge + 1) & 0xF; // fixme

            if (size <= mPool0.getSize()) { mPool0.free(p); return; }
            if (size <= mPool1.getSize()) { mPool1.free(p); return; }
            if (size <= mPool2.getSize()) { mPool2.free(p); return; }
        }
    };

// FIXME: We should be using a Spinlock here, at least on platforms where mutexes are not
//        efficient (i.e. non-Linux). However, we've seen some hangs on that spinlock, which
//        we don't understand well (b/308029108).
#ifndef NDEBUG
    using HandleArena = utils::Arena<Allocator,
            utils::LockingPolicy::Mutex,
            utils::TrackingPolicy::DebugAndHighWatermark>;
#else
    using HandleArena = utils::Arena<Allocator,
            utils::LockingPolicy::Mutex>;
#endif

    // allocateHandle()/deallocateHandle() selects the pool to use at compile-time based on the
    // allocation size this is always inlined, because all these do is to call
    // allocateHandleInPool()/deallocateHandleFromPool() with the right pool size.
    template<typename D>
    HandleBase::HandleId allocateHandle() noexcept {
        constexpr size_t BUCKET_SIZE = getBucketSize<D>();
        return allocateHandleInPool<BUCKET_SIZE>();
    }

    template<typename D>
    void deallocateHandle(HandleBase::HandleId id) noexcept {
        constexpr size_t BUCKET_SIZE = getBucketSize<D>();
        deallocateHandleFromPool<BUCKET_SIZE>(id);
    }

    // allocateHandleInPool()/deallocateHandleFromPool() is NOT inlined, which will cause three
    // versions to be generated, one for each pool. Because the arena is synchronized,
    // the code generated is not trivial (even if it's not insane either).
    template<size_t SIZE>
    UTILS_NOINLINE
    HandleBase::HandleId allocateHandleInPool() noexcept {
        uint8_t age;
        void* p = mHandleArena.alloc(SIZE, alignof(std::max_align_t), 0, &age);
        if (UTILS_LIKELY(p)) {
            uint32_t const tag = (uint32_t(age) << HANDLE_AGE_SHIFT) & HANDLE_AGE_MASK;
            return arenaPointerToHandle(p, tag);
        } else {
            return allocateHandleSlow(SIZE);
        }
    }

    template<size_t SIZE>
    UTILS_NOINLINE
    void deallocateHandleFromPool(HandleBase::HandleId id) noexcept {
        if (UTILS_LIKELY(isPoolHandle(id))) {
            auto [p, tag] = handleToPointer(id);
            uint8_t const age = (tag & HANDLE_AGE_MASK) >> HANDLE_AGE_SHIFT;
            mHandleArena.free(p, SIZE, age);
        } else {
            deallocateHandleSlow(id, SIZE);
        }
    }

    // number if bits allotted to the handle's age (currently 4 max)
    static constexpr uint32_t HANDLE_AGE_BIT_COUNT = 4;
    // number if bits allotted to the handle's debug tag (HANDLE_AGE_BIT_COUNT max)
    static constexpr uint32_t HANDLE_DEBUG_TAG_BIT_COUNT = 2;
    // bit shift for both the age and debug tag
    static constexpr uint32_t HANDLE_AGE_SHIFT = 27;
    // mask for the heap (vs pool) flag
    static constexpr uint32_t HANDLE_HEAP_FLAG = 0x80000000u;
    // mask for the age
    static constexpr uint32_t HANDLE_AGE_MASK =
            ((1 << HANDLE_AGE_BIT_COUNT) - 1) << HANDLE_AGE_SHIFT;
    // mask for the debug tag
    static constexpr uint32_t HANDLE_DEBUG_TAG_MASK =
            ((1 << HANDLE_DEBUG_TAG_BIT_COUNT) - 1) << HANDLE_AGE_SHIFT;
    // mask for the index
    static constexpr uint32_t HANDLE_INDEX_MASK = 0x07FFFFFFu;

    static_assert(HANDLE_DEBUG_TAG_BIT_COUNT <= HANDLE_AGE_BIT_COUNT);

    static bool isPoolHandle(HandleBase::HandleId id) noexcept {
        return (id & HANDLE_HEAP_FLAG) == 0u;
    }

    HandleBase::HandleId allocateHandleSlow(size_t size);
    void deallocateHandleSlow(HandleBase::HandleId id, size_t size) noexcept;

    // We inline this because it's just 4 instructions in the fast case
    inline std::pair<void*, uint32_t> handleToPointer(HandleBase::HandleId id) const noexcept {
        // note: the null handle will end-up returning nullptr b/c it'll be handled as
        // a non-pool handle.
        if (UTILS_LIKELY(isPoolHandle(id))) {
            char* const base = (char*)mHandleArena.getArea().begin();
            uint32_t const tag = id & HANDLE_AGE_MASK;
            size_t const offset = (id & HANDLE_INDEX_MASK) * Allocator::getAlignment();
            return { static_cast<void*>(base + offset), tag };
        }
        return { handleToPointerSlow(id), 0 };
    }

    void* handleToPointerSlow(HandleBase::HandleId id) const noexcept;

    // We inline this because it's just 3 instructions
    inline HandleBase::HandleId arenaPointerToHandle(void* p, uint32_t tag) const noexcept {
        char* const base = (char*)mHandleArena.getArea().begin();
        size_t const offset = (char*)p - base;
        assert_invariant((offset % Allocator::getAlignment()) == 0);
        auto id = HandleBase::HandleId(offset / Allocator::getAlignment());
        id |= tag & HANDLE_AGE_MASK;
        assert_invariant((id & HANDLE_HEAP_FLAG) == 0);
        return id;
    }

    HandleArena mHandleArena;

    // Below is only used when running out of space in the HandleArena
    mutable utils::Mutex mLock;
    tsl::robin_map<HandleBase::HandleId, void*> mOverflowMap;
    tsl::robin_map<HandleBase::HandleId, utils::CString> mDebugTags;
    HandleBase::HandleId mId = 0;
    bool mUseAfterFreeCheckDisabled = false;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_PRIVATE_HANDLEALLOCATOR_H
