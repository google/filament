//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SimpleMutex.h:
//   A simple non-recursive mutex that only supports lock and unlock operations.  As such, it can be
//   implemented more efficiently than a generic mutex such as std::mutex.  In the uncontended
//   paths, the implementation boils down to basically an inlined atomic operation and an untaken
//   branch.  The implementation in this file is inspired by Mesa's src/util/simple_mtx.h, which in
//   turn is based on "mutex3" in:
//
//       "Futexes Are Tricky"
//       http://www.akkadia.org/drepper/futex.pdf
//
//   Given that std::condition_variable only interacts with std::mutex, SimpleMutex cannot be used
//   with condition variables.
//

#ifndef COMMON_SIMPLEMUTEX_H_
#define COMMON_SIMPLEMUTEX_H_

#include "common/log_utils.h"
#include "common/platform.h"

#include <atomic>
#include <mutex>

// Enable futexes on:
//
// - Linux and derivatives (Android, ChromeOS, etc)
// - Windows 8+
//
// There is no TSAN support for futex currently, so it is disabled in that case
#if !defined(ANGLE_WITH_TSAN)
#    if defined(ANGLE_PLATFORM_LINUX) || defined(ANGLE_PLATFORM_ANDROID)
// Linux has had futexes for a very long time.  Assume support.
#        define ANGLE_USE_FUTEX 1
#    elif defined(ANGLE_PLATFORM_WINDOWS) && !defined(ANGLE_ENABLE_WINDOWS_UWP) && \
        !defined(ANGLE_WINDOWS_NO_FUTEX)
// Windows has futexes since version 8, which is already end of life (let alone older versions).
// Assume support.
#        define ANGLE_USE_FUTEX 1
#    endif  // defined(ANGLE_PLATFORM_LINUX) || defined(ANGLE_PLATFORM_ANDROID)
#endif      // !defined(ANGLE_WITH_TSAN)

namespace angle
{
namespace priv
{
#if ANGLE_USE_FUTEX
class MutexOnFutex
{
  public:
    void lock()
    {
        uint32_t oldState    = kUnlocked;
        const bool lockTaken = mState.compare_exchange_strong(oldState, kLocked);

        // In uncontended cases, the lock is acquired and there's nothing to do
        if (ANGLE_UNLIKELY(!lockTaken))
        {
            ASSERT(oldState == kLocked || oldState == kBlocked);

            // If not already marked as such, signal that the mutex is contended.
            if (oldState != kBlocked)
            {
                oldState = mState.exchange(kBlocked, std::memory_order_acq_rel);
            }
            // Wait until the lock is acquired
            while (oldState != kUnlocked)
            {
                futexWait();
                oldState = mState.exchange(kBlocked, std::memory_order_acq_rel);
            }
        }
    }
    void unlock()
    {
        // Unlock the mutex
        const uint32_t oldState = mState.fetch_add(-1, std::memory_order_acq_rel);

        // If another thread is waiting on this mutex, wake it up
        if (ANGLE_UNLIKELY(oldState != kLocked))
        {
            mState.store(kUnlocked, std::memory_order_relaxed);
            futexWake();
        }
    }
    void assertLocked() { ASSERT(mState.load(std::memory_order_relaxed) != kUnlocked); }

  private:
    void futexWait();
    void futexWake();

    // Note: the ordering of these values is important due to |unlock()|'s atomic decrement.
    static constexpr uint32_t kUnlocked = 0;
    static constexpr uint32_t kLocked   = 1;
    static constexpr uint32_t kBlocked  = 2;

    std::atomic_uint32_t mState = 0;
};
#else   // !ANGLE_USE_FUTEX
class MutexOnStd
{
  public:
    void lock() { mutex.lock(); }
    void unlock() { mutex.unlock(); }
    void assertLocked() { ASSERT(isLocked()); }

  private:
    bool isLocked()
    {
        // This works because angle::SimpleMutex does not support recursion
        const bool acquiredLock = mutex.try_lock();
        if (acquiredLock)
        {
            mutex.unlock();
        }

        return !acquiredLock;
    }

    std::mutex mutex;
};
#endif  // ANGLE_USE_FUTEX
}  // namespace priv

#if ANGLE_USE_FUTEX
using SimpleMutex = priv::MutexOnFutex;
#else
using SimpleMutex = priv::MutexOnStd;
#endif

// A no-op mutex to replace SimpleMutex where a lock is not needed.
struct NoOpMutex
{
    void lock() {}
    void unlock() {}
    bool try_lock() { return true; }
};
}  // namespace angle

#endif  // COMMON_SIMPLEMUTEX_H_
