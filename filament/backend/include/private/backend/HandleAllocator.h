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

#ifndef TNT_FILAMENT_BACKEND_HANDLEALLOCATOR_H
#define TNT_FILAMENT_BACKEND_HANDLEALLOCATOR_H

#include <backend/Handle.h>

#include <utils/Allocator.h>
#include <utils/Log.h>
#include <utils/compiler.h>

namespace filament::backend {

/*
 * A utility class to efficiently allocate and manage Handle<>
 */
class HandleAllocator {
public:

    HandleAllocator(const char* name, size_t size) noexcept;

    /*
     * Constructs a D object and returns a Handle<D>
     *
     * e.g.:
     *  struct ConcreteTexture : public HwTexture {
     *      ConcreteTexture(int w, int h);
     *  };
     *  Handle<ConcreteTexture> h = allocate(w, h);
     *
     */
    template<typename D, typename ... ARGS>
    Handle<D> allocate(ARGS&& ... args) noexcept {
        Handle<D> h{ allocateHandle(sizeof(D)) };
        D* addr = handle_cast<D*>(h);
        new(addr) D(std::forward<ARGS>(args)...);
#if !defined(NDEBUG) && UTILS_HAS_RTTI
        addr->typeId = typeid(D).name();
#endif
        return h;
    }

    /*
     * Destroys the object D at Handle<B> and construct a new D in its place
     * e.g.:
     *  ConcreteTexture* p = construct(h, w, h);
     */
    template<typename D, typename B, typename ... ARGS>
    typename std::enable_if_t<std::is_base_of_v<B, D>, D>*
    construct(Handle<B> const& handle, ARGS&& ... args) noexcept {
        assert_invariant(handle);
        D* addr = handle_cast<D*>(const_cast<Handle<B>&>(handle));
        assert_invariant(addr);

        // currently we implement construct<> with dtor+ctor, we could use operator= also
        // but all our dtors are trivial, ~D() is actually a noop.
        addr->~D();
        new(addr) D(std::forward<ARGS>(args)...);

#if !defined(NDEBUG) && UTILS_HAS_RTTI
        addr->typeId = typeid(D).name();
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
#if !defined(NDEBUG) && UTILS_HAS_RTTI
            if (UTILS_UNLIKELY(p->typeId != typeid(D).name())) {
                utils::slog.e << "Destroying handle " << handle.getId() << ", type " << typeid(D).name()
                       << ", but handle's actual type is " << p->typeId << utils::io::endl;
                std::terminate();
            }
            const_cast<D *>(p)->typeId = "(deleted)";
#endif
            p->~D();
            deallocateHandle(handle.getId(), sizeof(D));
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
        if (!handle) return nullptr; // better to get a NPE than random behavior/corruption
        void* p = handleToPointer(handle.getId(), sizeof(typename std::remove_pointer_t<Dp>));
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

    class Allocator {
        friend class HandleAllocator;
        const utils::AreaPolicy::HeapArea& mArea;
        utils::PoolAllocator< 16, 16>   mPool0;
        utils::PoolAllocator< 64, 16>   mPool1;
        utils::PoolAllocator<208, 16>   mPool2;
    public:
        static constexpr size_t MIN_ALIGNMENT_SHIFT = 4;
        explicit Allocator(const utils::AreaPolicy::HeapArea& area);

        // this is in fact always called with a constexpr size argument
        [[nodiscard]] inline void* alloc(size_t size, size_t alignment, size_t extra) noexcept {
            void* p = nullptr;
                 if (size <= mPool0.getSize()) p = mPool0.alloc(size, 16, extra);
            else if (size <= mPool1.getSize()) p = mPool1.alloc(size, 16, extra);
            else if (size <= mPool2.getSize()) p = mPool2.alloc(size, 16, extra);
            return UTILS_LIKELY(p) ? p : alloc_slow(size, alignment, extra);
        }

        // this is in fact always called with a constexpr size argument
        inline void free(void* p, size_t size) noexcept {
            if (UTILS_LIKELY(isPoolAllocation(p, size))) {
                if (size <= mPool0.getSize()) { mPool0.free(p); return; }
                if (size <= mPool1.getSize()) { mPool1.free(p); return; }
                if (size <= mPool2.getSize()) { mPool2.free(p); return; }
            } else {
                free_slow(p, size);
            }
        }

    private:
        bool isPoolAllocation(void* p, size_t size) const noexcept;
        void* alloc_slow(size_t size, size_t alignment, size_t extra) noexcept;
        void free_slow(void* p, size_t size) noexcept;
    };


#ifndef NDEBUG
    using HandleArena = utils::Arena<Allocator,
            utils::LockingPolicy::SpinLock,
            utils::TrackingPolicy::Debug>;
#else
    using HandleArena = utils::Arena<Allocator,
            utils::LockingPolicy::SpinLock>;
#endif

    // this is inlined because we're always called with a constexpr size
    HandleBase::HandleId allocateHandle(size_t size) noexcept {
        void* p = mHandleArena.alloc(size);
        return pointerToHandle(p);
    }

    // this is inlined because we're always called with a constexpr size
    void deallocateHandle(HandleBase::HandleId id, size_t size) noexcept {
        void* p = handleToPointer(id, size);
        mHandleArena.free(p, size);
    }

    void* handleToPointer(HandleBase::HandleId id, size_t size) const noexcept;
    HandleBase::HandleId pointerToHandle(void* p) const noexcept;

    HandleArena mHandleArena;
};

} // namespace filament::backend

#endif //TNT_FILAMENT_BACKEND_HANDLEALLOCATOR_H
