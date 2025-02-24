// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_COMMON_MUTEXPROTECTED_H_
#define SRC_DAWN_COMMON_MUTEXPROTECTED_H_

#include <mutex>
#include <utility>

#include "dawn/common/Mutex.h"
#include "dawn/common/NonMovable.h"
#include "dawn/common/Ref.h"
#include "dawn/common/StackAllocated.h"
#include "partition_alloc/pointers/raw_ptr_exclusion.h"

namespace dawn {

template <typename T, template <typename, typename> class Guard>
class MutexProtected;

namespace detail {

template <typename T>
struct MutexProtectedTraits {
    using MutexType = std::mutex;
    using LockType = std::unique_lock<std::mutex>;
    using ObjectType = T;

    static MutexType CreateMutex() { return std::mutex(); }
    static std::mutex& GetMutex(MutexType& m) { return m; }
    static ObjectType* GetObj(T* const obj) { return obj; }
    static const ObjectType* GetObj(const T* const obj) { return obj; }
};

template <typename T>
struct MutexProtectedTraits<Ref<T>> {
    using MutexType = Ref<Mutex>;
    using LockType = Mutex::AutoLock;
    using ObjectType = T;

    static MutexType CreateMutex() { return AcquireRef(new Mutex()); }
    static Mutex* GetMutex(MutexType& m) { return m.Get(); }
    static ObjectType* GetObj(Ref<T>* const obj) { return obj->Get(); }
    static const ObjectType* GetObj(const Ref<T>* const obj) { return obj->Get(); }
};

// Guard class is a wrapping class that gives access to a protected resource after acquiring the
// lock related to it. For the lifetime of this class, the lock is held.
template <typename T, typename Traits>
class Guard : public NonMovable, StackAllocated {
  public:
    // It's the programmer's burden to not save the pointer/reference and reuse it without the lock.
    auto* operator->() const { return Get(); }
    auto& operator*() const { return *Get(); }

  protected:
    Guard(T* obj, typename Traits::MutexType& mutex) : mLock(Traits::GetMutex(mutex)), mObj(obj) {}
    Guard(Guard&& other) : mLock(std::move(other.mLock)), mObj(std::move(other.mObj)) {
        other.mObj = nullptr;
    }

    Guard(const Guard& other) = delete;
    Guard& operator=(const Guard& other) = delete;
    Guard& operator=(Guard&& other) = delete;

    auto* Get() const { return Traits::GetObj(mObj); }

  private:
    using NonConstT = typename std::remove_const<T>::type;
    friend class MutexProtected<NonConstT, Guard>;

    typename Traits::LockType mLock;
    // RAW_PTR_EXCLUSION: This pointer is created/destroyed on each access to a MutexProtected.
    // The pointer is always transiently used while the MutexProtected is in scope so it is
    // unlikely to be used after it is freed.
    RAW_PTR_EXCLUSION T* mObj = nullptr;
};

}  // namespace detail

// Wrapping class used for object members to ensure usage of the resource is protected with a mutex.
// Example usage:
//     class Allocator {
//       public:
//         Allocation Allocate();
//         void Deallocate(Allocation&);
//     };
//     class AllocatorUser {
//       public:
//         void OnlyAllocate() {
//             auto allocation = mAllocator->Allocate();
//         }
//         void AtomicAllocateDeallocate() {
//             // Operations:
//             //   - acquire lock
//             //   - Allocate, Deallocate
//             //   - release lock
//             mAllocator.Use([](auto allocator) {
//                 auto allocation = allocator->Allocate();
//                 allocator->Deallocate(allocation);
//             });
//         }
//         void NonAtomicAllocateDeallocate() {
//             // Operations:
//             //   - acquire lock, Allocate, release lock
//             //   - acquire lock, Deallocate, release lock
//             auto allocation = mAllocator->Allocate();
//             mAllocator->Deallocate(allocation);
//         }
//       private:
//         MutexProtected<Allocator> mAllocator;
//     };
template <typename T, template <typename, typename> class Guard = detail::Guard>
class MutexProtected {
  public:
    using Traits = detail::MutexProtectedTraits<T>;
    using Usage = Guard<T, Traits>;
    using ConstUsage = Guard<const T, Traits>;

    MutexProtected() : mMutex(Traits::CreateMutex()) {}

    template <typename... Args>
    // NOLINTNEXTLINE(runtime/explicit) allow implicit construction
    MutexProtected(Args&&... args)
        : mMutex(Traits::CreateMutex()), mObj(std::forward<Args>(args)...) {}

    Usage operator->() { return Use(); }
    ConstUsage operator->() const { return Use(); }

    template <typename Fn>
    auto Use(Fn&& fn) {
        return fn(Use());
    }
    template <typename Fn>
    auto Use(Fn&& fn) const {
        return fn(Use());
    }

  private:
    Usage Use() { return Usage(&mObj, mMutex); }
    ConstUsage Use() const { return ConstUsage(&mObj, mMutex); }

    mutable typename Traits::MutexType mMutex;
    T mObj;
};

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_MUTEXPROTECTED_H_
