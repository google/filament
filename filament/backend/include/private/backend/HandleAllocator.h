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
#include <utils/Log.h>
#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/ostream.h>

#include <tsl/robin_map.h>

#include <exception>
#include <type_traits>
#include <unordered_map>

#include <stddef.h>
#include <stdint.h>

#if !defined(NDEBUG) && UTILS_HAS_RTTI
#   define HANDLE_TYPE_SAFETY 1
#else
#   define HANDLE_TYPE_SAFETY 0
#endif

#define HandleAllocatorGL  HandleAllocator<16, 64, 208>
#define HandleAllocatorVK  HandleAllocator<16, 64, 880>
#define HandleAllocatorMTL HandleAllocator<16, 64, 584>

namespace filament::backend {

/*
 * A utility class to efficiently allocate and manage Handle<>
 */
template <size_t P0, size_t P1, size_t P2>
class HandleAllocator {
public:

    HandleAllocator(const char* name, size_t size) noexcept;
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
    Handle<D> allocateAndConstruct(ARGS&& ... args) noexcept {
        Handle<D> h{ allocateHandle<sizeof(D)>() };
        D* addr = handle_cast<D*>(h);
        new(addr) D(std::forward<ARGS>(args)...);
#if HANDLE_TYPE_SAFETY
        mLock.lock();
        mHandleTypeId[addr] = typeid(D).name();
        mLock.unlock();
#endif
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
        Handle<D> h{ allocateHandle<sizeof(D)>() };
#if HANDLE_TYPE_SAFETY
        D* addr = handle_cast<D*>(h);
        mLock.lock();
        mHandleTypeId[addr] = typeid(D).name();
        mLock.unlock();
#endif
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
    destroyAndConstruct(Handle<B> const& handle, ARGS&& ... args) noexcept {
        assert_invariant(handle);
        D* addr = handle_cast<D*>(const_cast<Handle<B>&>(handle));
        assert_invariant(addr);

        // currently we implement construct<> with dtor+ctor, we could use operator= also
        // but all our dtors are trivial, ~D() is actually a noop.
        addr->~D();
        new(addr) D(std::forward<ARGS>(args)...);

#if HANDLE_TYPE_SAFETY
        mLock.lock();
        mHandleTypeId[addr] = typeid(D).name();
        mLock.unlock();
#endif
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

#if HANDLE_TYPE_SAFETY
        mLock.lock();
        mHandleTypeId[addr] = typeid(D).name();
        mLock.unlock();
#endif
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
#if HANDLE_TYPE_SAFETY
            mLock.lock();
            auto typeId = mHandleTypeId[p];
            mHandleTypeId.erase(p);
            mLock.unlock();
            if (UTILS_UNLIKELY(typeId != typeid(D).name())) {
                utils::slog.e << "Destroying handle " << handle.getId() << ", type " << typeid(D).name()
                       << ", but handle's actual type is " << typeId << utils::io::endl;
                std::terminate();
            }
#endif
            p->~D();
            deallocateHandle<sizeof(D)>(handle.getId());
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
    handle_cast(Handle<B>& handle) noexcept {
        assert_invariant(handle);
        void* const p = handleToPointer(handle.getId());
        return static_cast<Dp>(p);
    }

    template<typename Dp, typename B>
    inline typename std::enable_if_t<
            std::is_pointer_v<Dp> &&
            std::is_base_of_v<B, typename std::remove_pointer_t<Dp>>, Dp>
    handle_cast(Handle<B> const& handle) noexcept {
        return handle_cast<Dp>(const_cast<Handle<B>&>(handle));
    }


private:

    // template <int P0, int P1, int P2>
    class Allocator {
        friend class HandleAllocator;
        utils::PoolAllocator<P0, 16>   mPool0;
        utils::PoolAllocator<P1, 16>   mPool1;
        utils::PoolAllocator<P2, 16>   mPool2;
        UTILS_UNUSED_IN_RELEASE const utils::AreaPolicy::HeapArea& mArea;
    public:
        static constexpr size_t MIN_ALIGNMENT_SHIFT = 4;
        explicit Allocator(const utils::AreaPolicy::HeapArea& area);

        // this is in fact always called with a constexpr size argument
        [[nodiscard]] inline void* alloc(size_t size, size_t, size_t extra = 0) noexcept {
            void* p = nullptr;
                 if (size <= mPool0.getSize()) p = mPool0.alloc(size, 16, extra);
            else if (size <= mPool1.getSize()) p = mPool1.alloc(size, 16, extra);
            else if (size <= mPool2.getSize()) p = mPool2.alloc(size, 16, extra);
            return p;
        }

        // this is in fact always called with a constexpr size argument
        inline void free(void* p, size_t size) noexcept {
            assert_invariant(p >= mArea.begin() && (char*)p + size <= (char*)mArea.end());
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
    template<size_t SIZE>
    HandleBase::HandleId allocateHandle() noexcept {
        if constexpr (SIZE <= P0) { return allocateHandleInPool<P0>(); }
        if constexpr (SIZE <= P1) { return allocateHandleInPool<P1>(); }
        static_assert(SIZE <= P2);
        return allocateHandleInPool<P2>();
    }

    template<size_t SIZE>
    void deallocateHandle(HandleBase::HandleId id) noexcept {
        if constexpr (SIZE <= P0) {
            deallocateHandleFromPool<P0>(id);
        } else if constexpr (SIZE <= P1) {
            deallocateHandleFromPool<P1>(id);
        } else {
            static_assert(SIZE <= P2);
            deallocateHandleFromPool<P2>(id);
        }
    }

    // allocateHandleInPool()/deallocateHandleFromPool() is NOT inlined, which will cause three
    // versions to be generated, one for each pool. Because the arena is synchronized,
    // the code generated is not trivial (even if it's not insane either).
    template<size_t SIZE>
    UTILS_NOINLINE
    HandleBase::HandleId allocateHandleInPool() noexcept {
        void* p = mHandleArena.alloc(SIZE);
        if (UTILS_LIKELY(p)) {
            return pointerToHandle(p);
        } else {
            return allocateHandleSlow(SIZE);
        }
    }

    template<size_t SIZE>
    UTILS_NOINLINE
    void deallocateHandleFromPool(HandleBase::HandleId id) noexcept {
        if (UTILS_LIKELY(isPoolHandle(id))) {
            void* p = handleToPointer(id);
            mHandleArena.free(p, SIZE);
        } else {
            deallocateHandleSlow(id, SIZE);
        }
    }

    static constexpr uint32_t HEAP_HANDLE_FLAG = 0x80000000u;

    static bool isPoolHandle(HandleBase::HandleId id) noexcept {
        return (id & HEAP_HANDLE_FLAG) == 0u;
    }

    HandleBase::HandleId allocateHandleSlow(size_t size) noexcept;
    void deallocateHandleSlow(HandleBase::HandleId id, size_t size) noexcept;

    // We inline this because it's just 4 instructions in the fast case
    inline void* handleToPointer(HandleBase::HandleId id) const noexcept {
        // note: the null handle will end-up returning nullptr b/c it'll be handled as
        // a non-pool handle.
        if (UTILS_LIKELY(isPoolHandle(id))) {
            char* const base = (char*)mHandleArena.getArea().begin();
            size_t offset = id << Allocator::MIN_ALIGNMENT_SHIFT;
            return static_cast<void*>(base + offset);
        }
        return handleToPointerSlow(id);
    }

    void* handleToPointerSlow(HandleBase::HandleId id) const noexcept;

    // We inline this because it's just 3 instructions
    inline HandleBase::HandleId pointerToHandle(void* p) const noexcept {
        char* const base = (char*)mHandleArena.getArea().begin();
        size_t offset = (char*)p - base;
        auto id = HandleBase::HandleId(offset >> Allocator::MIN_ALIGNMENT_SHIFT);
        assert_invariant((id & HEAP_HANDLE_FLAG) == 0);
        return id;
    }

    HandleArena mHandleArena;

    // Below is only used when running out of space in the HandleArena
    mutable utils::Mutex mLock;
    tsl::robin_map<HandleBase::HandleId, void*> mOverflowMap;
    HandleBase::HandleId mId = 0;
#if HANDLE_TYPE_SAFETY
    mutable std::unordered_map<const void*, const char*> mHandleTypeId;
#endif
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_PRIVATE_HANDLEALLOCATOR_H
