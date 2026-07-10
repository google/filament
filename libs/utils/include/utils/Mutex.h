/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef TNT_UTILS_MUTEX_H
#define TNT_UTILS_MUTEX_H

/**
 * @file Mutex.h
 * @brief Custom low-overhead mutex primitives for Filament.
 *
 * Filament prefers custom `utils::Mutex` over C++ standard `std::mutex` across all engine code.
 *
 * WHY `utils::Mutex` IS PREFERRED OVER `std::mutex`:
 *
 * 1. UNIFIED CONCURRENCY & DEADLOCK DEBUGGING (`-u` / `FILAMENT_DEBUG_MUTEX`):
 *    When `FILAMENT_DEBUG_MUTEX` or `UTILS_DEBUG_MUTEX` is enabled, `utils::Mutex` transparently
 *    instruments every lock acquisition and release across the engine. It maintains a global dependency
 *    graph verified via BFS during `lock()` and `try_lock()` to detect multi-threaded lock-order inversions
 *    and recursive self-deadlocks on non-recursive locks, logging exact creation and acquisition `CallStack`s.
 *    Raw `std::mutex` instances are completely untracked and invisible to this debugging facility.
 *
 * 2. CACHE & MEMORY FOOTPRINT HYGIENE:
 *    On Android and Linux (`linuxutil::Mutex`), `utils::Mutex` is only 4 bytes (a single atomic uint32_t
 *    futex word), compared to `std::mutex` (`pthread_mutex_t`), which consumes 40 bytes. For objects embedded
 *    in high quantities (fences, sync points, handle allocators, or queue buffers), `utils::Mutex` prevents
 *    struct bloat and preserves cache-line density and alignment hygiene.
 *
 * 3. FAST-PATH INLINING & HIGH CONTENTION PERFORMANCE:
 *    `utils::Mutex` uses fully inlined atomic compare-and-swap (`compare_exchange_strong`) userspace
 *    fast-paths. When uncontended, acquisition and release execute with zero PLT/dynamic library overhead or
 *    kernel syscalls. Under contention (`LOCKED_CONTENDED`), `utils::Mutex` drops directly into OS kernel
 *    futex sleep (`futex_wait_ex`), matching standard mutex high-contention efficiency without overhead.
 *
 * 4. CONDITION VARIABLE COMPATIBILITY:
 *    `utils::Condition` (`Condition::wait` and `wait_until`) is explicitly templated on the mutex type (`template <typename M>`)
 *    and works seamlessly with `std::unique_lock<utils::Mutex>` or `utils::UniqueLock<utils::Mutex>`.
 *
 * NOTE ON PRIORITY INVERSION:
 *    C++ `std::mutex` (backed by POSIX `pthread_mutex_t`) does NOT prevent priority inversions out of the
 *    box, because standard libraries initialize mutex attributes to `PTHREAD_PRIO_NONE` (rather than
 *    `PTHREAD_PRIO_INHERIT`). Therefore, `std::mutex` provides no priority inheritance advantage over `utils::Mutex`.
 */

#include <utils/compiler.h>
#include <mutex>

#if defined(__ANDROID__)
#include <utils/linux/Mutex.h>
#else
#include <utils/generic/Mutex.h>
#endif

#if defined(UTILS_DEBUG_MUTEX) || defined(FILAMENT_DEBUG_MUTEX)
#include <utils/debug/Mutex.h>
namespace utils {
using Mutex = debug::Mutex;
} // namespace utils
#endif

namespace utils {

/**
 * UniqueLock is an annotated wrapper subclass of C++ standard std::unique_lock.
 * 
 * WHY IT EXISTS:
 * In standard C++ libraries, std::unique_lock is intentionally left UN-ANNOTATED
 * with thread safety capabilities. This is because std::unique_lock is a highly
 * dynamic lock guard that supports deferred locking (std::defer_lock), manual
 * unlock/lock cycles, and move semantics. Proving these dynamic states statically at
 * compile-time is undecidable in the general case, so standard library developers
 * avoid annotating it to prevent false-positive compile warnings.
 * 
 * However, this means standard std::unique_lock cannot be used to access guarded
 * fields (UTILS_GUARDED_BY) in methods that also synchronize via condition variables
 * (which strictly require std::unique_lock) without triggering static analysis errors.
 * 
 * utils::UniqueLock solves this by acting as a strictly lexical, scoped subclass
 * of std::unique_lock. It is explicitly decorated with Clang's SCOPED_CAPABILITY,
 * ACQUIRE, and RELEASE attributes. This allows low-level synchronization methods
 * (like condition waits) to be fully verified by compile-time thread checks.
 */
template <typename M>
class UTILS_SCOPED_CAPABILITY UniqueLock : public std::unique_lock<M> {
public:
    UniqueLock(M& m) UTILS_ACQUIRE(m) : std::unique_lock<M>(m) {}
    ~UniqueLock() UTILS_RELEASE() {}

    void lock() UTILS_ACQUIRE() { std::unique_lock<M>::lock(); }
    void unlock() UTILS_RELEASE() { std::unique_lock<M>::unlock(); }
};

/**
 * LockGuard is an annotated wrapper subclass of C++ standard std::lock_guard.
 * 
 * WHY IT EXISTS:
 * On some platforms and toolchains (such as the Android NDK sysroot toolchain), the 
 * standard library's std::lock_guard class template is not annotated with the 
 * 'scoped_lockable' capability attribute. This prevents Clang from verifying that 
 * exclusive locks are held throughout the scope of a standard lock_guard object, 
 * resulting in compiler errors when accessing guarded fields (UTILS_GUARDED_BY) on Android.
 * 
 * utils::LockGuard solves this by subclassing std::lock_guard and explicitly adding the
 * UTILS_SCOPED_CAPABILITY, UTILS_ACQUIRE, and UTILS_RELEASE attributes. This guarantees 
 * platform-independent, warning-free compile-time thread checking for scoped locks.
 */
template <typename M>
class UTILS_SCOPED_CAPABILITY LockGuard : public std::lock_guard<M> {
public:
    LockGuard(M& m) UTILS_ACQUIRE(m) : std::lock_guard<M>(m) {}
    ~LockGuard() UTILS_RELEASE() {}
};

} // namespace utils

#endif // TNT_UTILS_MUTEX_H
