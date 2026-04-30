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

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <type_traits>
#include <utility>

#include "dawn/common/Compiler.h"
#include "dawn/common/Defer.h"
#include "dawn/common/Mutex.h"
#include "dawn/common/NonMovable.h"
#include "dawn/common/Ref.h"
#include "dawn/common/StackAllocated.h"
#include "dawn/common/Time.h"

namespace dawn {

template <typename T, template <typename, typename> class Guard, typename Traits>
class MutexProtected;

// Used by MutexCondVarProtected below where sometimes, it's useful to be able to specify which type
// of notify scope we want.
enum class NotifyType {
    All,
    One,
    None,
};

template <typename T, template <typename, typename, NotifyType> class Guard, typename Traits>
class MutexCondVarProtected;

namespace detail {

template <typename T>
struct MutexProtectedTraits {
    using MutexType = std::mutex;
    using LockType = std::unique_lock<std::mutex>;
    using ObjectType = T;

    static constexpr bool kSupportsTryLock = true;

    static MutexType CreateMutex() { return std::mutex(); }
    static std::mutex& GetMutex(MutexType& m) { return m; }
    static ObjectType* GetObj(T* const obj) { return obj; }
    static const ObjectType* GetObj(const T* const obj) { return obj; }

    static std::optional<LockType> TryLock(MutexType& mutex) {
        LockType lock(GetMutex(mutex), std::try_to_lock);
        if (!lock.owns_lock()) {
            return std::nullopt;
        }
        return lock;
    }
};

template <typename T>
struct MutexProtectedTraits<Ref<T>> {
    using MutexType = Ref<Mutex>;
    using LockType = Mutex::AutoLock;
    using ObjectType = T;

    static constexpr bool kSupportsTryLock = false;

    static MutexType CreateMutex() { return AcquireRef(new Mutex()); }
    static Mutex* GetMutex(MutexType& m) { return m.Get(); }
    static ObjectType* GetObj(Ref<T>* const obj) { return obj->Get(); }
    static const ObjectType* GetObj(const Ref<T>* const obj) { return obj->Get(); }
};

template <typename T>
struct MutexRefProtectedTraits {
    using MutexType = Ref<Mutex>;
    using LockType = Mutex::AutoLock;
    using ObjectType = T;

    static constexpr bool kSupportsTryLock = false;

    static MutexType CreateMutex() { return AcquireRef(new Mutex()); }
    static Mutex* GetMutex(MutexType& m) { return m.Get(); }
    static ObjectType* GetObj(T* const obj) { return obj; }
    static const ObjectType* GetObj(const T* const obj) { return obj; }
};

template <typename T, typename Traits>
class Guard;
template <typename T, typename Traits, NotifyType NotifyT>
class CondVarGuard;

// Guard class is a wrapping class that gives access to a protected resource after acquiring the
// lock related to it. For the lifetime of this class, the lock is held.
template <typename T, typename Traits>
class DAWN_SCOPED_LOCKABLE Guard : public NonMovable, StackAllocated {
  public:
    // It's the programmer's burden to not save the pointer/reference and reuse it without the lock.
    auto* operator->() const { return Get(); }
    auto& operator*() const { return *Get(); }

    void Defer(std::function<void()> f) {
        DAWN_ASSERT(mDefer);
        mDefer->Append(std::move(f));
    }

  protected:
    Guard() : mLock() {}
    Guard(T* obj, typename Traits::MutexType& mutex, class Defer* defer = nullptr)
        : mLock(Traits::GetMutex(mutex)), mObj(obj), mDefer(defer) {}
    Guard(T* obj, Traits::LockType&& lock, class Defer* defer = nullptr)
        : mLock(std::move(lock)), mObj(obj), mDefer(defer) {}
    Guard(Guard&& other)
        : mLock(std::move(other.mLock)),
          mObj(std::move(other.mObj)),
          mDefer(std::move(other.mDefer)) {
        other.mObj = nullptr;
    }

    Guard& operator=(Guard&& other) {
        if (this != &other) {
            mLock = std::move(other.mLock);
            mObj = std::move(other.mObj);
            mDefer = std::move(other.mDefer);
            other.mObj = nullptr;
            other.mDefer = nullptr;
        }
        return *this;
    }

    Guard(const Guard& other) = delete;
    Guard& operator=(const Guard& other) = delete;

    auto* Get() const { return Traits::GetObj(mObj); }

  private:
    using NonConstT = typename std::remove_const<T>::type;
    friend class MutexProtected<NonConstT, Guard, Traits>;

    // Currently need to explicitly list the notify types because we can't partially specialize
    // friend classes.
    friend class CondVarGuard<T, Traits, NotifyType::All>;
    friend class CondVarGuard<T, Traits, NotifyType::One>;
    friend class CondVarGuard<T, Traits, NotifyType::None>;

    typename Traits::LockType mLock;
    T* mObj = nullptr;
    class Defer* mDefer = nullptr;
};

// CondVarGuard is a different guard class that internally holds a Guard, but provides additional
// functionality w.r.t condition variables. Specifically, the non-const version of this Guard will
// automatically call a notify function on the underlying condition variable so that calls to
// |Wait*()| will unblock when |Pred| is true.
template <typename T, typename Traits, NotifyType NotifyT = NotifyType::All>
class CondVarGuard : public NonMovable, StackAllocated {
  public:
    // It's the programmer's burden to not save the pointer/reference and reuse it without the lock.
    auto* operator->() const { return mGuard.Get(); }
    auto& operator*() const { return *mGuard.Get(); }

    template <typename Predicate>
    void Wait(Predicate pred) {
        DAWN_ASSERT(mNotifyScope.cv);
        mNotifyScope.cv->wait(mGuard.mLock, [&] { return pred((*Get())); });
    }
    template <typename Predicate>
    bool WaitFor(Nanoseconds timeout, Predicate pred) {
        DAWN_ASSERT(mNotifyScope.cv);
        if (timeout < kMaxDurationNanos) {
            return mNotifyScope.cv->wait_for(
                mGuard.mLock, std::chrono::nanoseconds(static_cast<uint64_t>(timeout)),
                [&] { return pred(*Get()); });
        } else {
            Wait(pred);
            return true;
        }
    }

  protected:
    CondVarGuard(T* obj, Traits::MutexType& mutex, std::condition_variable* cv)
        : mNotifyScope(cv), mGuard(obj, mutex) {}

    auto* Get() const { return mGuard.Get(); }

  private:
    using NonConstT = typename std::remove_const<T>::type;
    friend class MutexCondVarProtected<NonConstT, CondVarGuard, Traits>;

    struct NotifyScopeBase : public StackAllocated {
        explicit NotifyScopeBase(std::condition_variable* cv) : cv(cv) { DAWN_ASSERT(cv); }
        std::condition_variable* cv = nullptr;
    };

    template <NotifyType U>
    struct NotifyScope : NotifyScopeBase {
        using NotifyScopeBase::NotifyScopeBase;
        ~NotifyScope() {
            if constexpr (!std::is_const_v<T>) {
                if constexpr (U == NotifyType::All) {
                    this->cv->notify_all();
                } else if constexpr (U == NotifyType::One) {
                    this->cv->notify_one();
                }
            }
        }
    };

    NotifyScope<NotifyT> mNotifyScope;
    // Note that this class needs to hold a Guard member instead of extending it because we want the
    // lock to be released before we notify.
    Guard<T, Traits> mGuard;
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
template <typename T,
          template <typename, typename> class Guard = detail::Guard,
          typename Traits = detail::MutexProtectedTraits<T>>
class MutexProtected {
  public:
    using Usage = Guard<T, Traits>;
    using ConstUsage = Guard<const T, Traits>;

    template <typename... Args>
        requires(sizeof...(Args) != 1 ||
                 !(std::is_same_v<std::decay_t<Args>, MutexProtected> && ...))
    // NOLINTNEXTLINE(runtime/explicit) allow implicit construction
    MutexProtected(Args&&... args)
        : mMutex(Traits::CreateMutex()), mObj(std::forward<Args>(args)...) {}
    virtual ~MutexProtected() = default;

    MutexProtected(const MutexProtected&)
        requires std::copy_constructible<typename Traits::MutexType> && std::copy_constructible<T>
    = default;
    MutexProtected& operator=(const MutexProtected&)
        requires std::is_copy_assignable_v<typename Traits::MutexType> &&
                     std::is_copy_assignable_v<T>
    = default;

    MutexProtected(MutexProtected&&)
        requires std::move_constructible<typename Traits::MutexType> && std::move_constructible<T>
    = default;
    MutexProtected& operator=(MutexProtected&&)
        requires std::is_move_assignable_v<typename Traits::MutexType> &&
                     std::is_move_assignable_v<T>
    = default;

    Usage operator->() { return Usage(&mObj, mMutex); }
    template <typename Fn>
    auto Use(Fn&& fn) {
        return fn(Usage(&mObj, mMutex));
    }

    ConstUsage operator->() const { return ConstUsage(&mObj, mMutex); }
    template <typename Fn>
    auto ConstUse(Fn&& fn) const {
        return fn(ConstUsage(&mObj, mMutex));
    }
    template <typename Fn>
    auto Use(Fn&& fn) const {
        return ConstUse(fn);
    }

    std::optional<Usage> TryUse()
        requires Traits::kSupportsTryLock
    {
        auto maybeLock = Traits::TryLock(mMutex);
        if (!maybeLock.has_value()) {
            return std::nullopt;
        }
        return Usage(&mObj, std::move(*maybeLock), nullptr);
    }

    template <typename Fn>
    auto UseWithDefer(Fn&& fn) {
        Defer defer;
        return fn(Usage(&mObj, mMutex, &defer));
    }

  private:
    mutable Traits::MutexType mMutex;
    T mObj;
};

// A moveable version of MutexProtected.
template <typename T>
using MutexRefProtected = MutexProtected<T, detail::Guard, detail::MutexRefProtectedTraits<T>>;

// Wrapping class for object members to provide the protections with a mutex of a MutexProtected
// with some additional helpers to allow waiting with a conditional variable as well. The general
// usage should look the same as MutexProtected above, with additional usages like the following
// example:
//     class Example {
//       public:
//         void Complete() {
//             mDone.Use([](auto done) {
//                 // Do something
//                 mDone = true;
//             });
//         }
//         void WaitUntilDone() {
//             mDone.Use([](auto done) {
//                 done.Wait([](auto& done) { return done; });
//             });
//         }
//       private:
//         MutexCondVarProtected<bool> mDone = false;
//     };
template <typename T,
          template <typename, typename, NotifyType> class Guard = detail::CondVarGuard,
          typename Traits = detail::MutexProtectedTraits<T>>
class MutexCondVarProtected {
  public:
    using Usage = Guard<T, Traits, NotifyType::All>;
    using ConstUsage = Guard<const T, Traits, NotifyType::None>;

    template <typename... Args>
        requires(sizeof...(Args) != 1 ||
                 !(std::is_same_v<std::decay_t<Args>, MutexCondVarProtected> && ...))
    // NOLINTNEXTLINE(runtime/explicit) allow implicit construction
    MutexCondVarProtected(Args&&... args)
        : mMutex(Traits::CreateMutex()), mObj(std::forward<Args>(args)...) {}
    virtual ~MutexCondVarProtected() = default;

    MutexCondVarProtected(const MutexCondVarProtected&)
        requires std::copy_constructible<typename Traits::MutexType> && std::copy_constructible<T>
    = default;
    MutexCondVarProtected& operator=(const MutexCondVarProtected&)
        requires std::is_copy_assignable_v<typename Traits::MutexType> &&
                     std::is_copy_assignable_v<T>
    = default;

    MutexCondVarProtected(MutexCondVarProtected&&)
        requires std::move_constructible<typename Traits::MutexType> && std::move_constructible<T>
    = default;
    MutexCondVarProtected& operator=(MutexCondVarProtected&&)
        requires std::is_move_assignable_v<typename Traits::MutexType> &&
                     std::is_move_assignable_v<T>
    = default;

    Usage operator->() { return Usage(&mObj, mMutex, &mCv); }
    template <NotifyType NotifyT = NotifyType::All, typename Fn>
    auto Use(Fn&& fn) {
        return fn(Guard<T, Traits, NotifyT>(&mObj, mMutex, &mCv));
    }

    // Note that unlike in MutexProtected where |Use| and |ConstUse| guarantee the lock for the
    // entire critical section, if a user calls |Wait| within |Fn|, the lock may be released and
    // reacquired in order for another thread to update the condition.
    ConstUsage operator->() const { return ConstUsage(&mObj, mMutex, &mCv); }
    template <typename Fn>
    auto ConstUse(Fn&& fn) const {
        return fn(ConstUsage(&mObj, mMutex, &mCv));
    }
    template <typename Fn>
    auto Use(Fn&& fn) const {
        return ConstUse(fn);
    }

  private:
    mutable Traits::MutexType mMutex;
    mutable std::condition_variable mCv;
    T mObj;
};

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_MUTEXPROTECTED_H_
