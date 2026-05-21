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
 * Filament provides two types of mutexes: C++ standard std::mutex and custom utils::Mutex.
 * Choosing the correct one is a critical tradeoff between runtime performance, cache hygiene,
 * and OS scheduler integration.
 *
 * ARCHITECTURAL TRADEOFFS & CHOICE CRITERIA:
 *
 * 1. Use C++ standard std::mutex when:
 *    - HIGH RISK OF PRIORITY INVERSION: The lock is shared between threads running at significantly
 *      different priorities (e.g., the critical Display/Render thread vs. low-priority background loader
 *      threads). std::mutex leverages OS-level pthread mutexes which support Priority Inheritance (PI)
 *      protocols, temporarily elevating the low-priority holder to prevent render pipeline stalls.
 *    - CONDITION VARIABLE SYNCHRON_VARIABLE: The lock is used in conjunction with condition variables
 *      (std::condition_variable) to block/sleep threads (e.g. CommandBufferQueue). Standard condition
 *      variables require std::mutex to operate natively and efficiently with the OS scheduler.
 *    - HIGH CONTENTION: The locked section is heavily contended, allowing modern CPU architectures
 *      (like ARMv8.1+ LSE) to utilize single-instruction atomics resolved dynamically at startup (IFUNCs).
 *
 * 2. Use custom utils::Mutex when:
 *    - CACHE & MEMORY FOOTPRINT IS CRITICAL: utils::Mutex is only 4 bytes (a single futex-based uint32_t)
 *      compared to standard std::mutex which is 40 bytes. For classes allocated in large numbers (such as
 *      fences, sync points, or handle maps), reducing size by 10x prevents padding bloat and maintains
 *      optimal cache line alignment hygiene (avoiding false sharing).
 *    - SAME-PRIORITY ACCESS: The lock is shared only between threads of identical high priorities
 *      (e.g., Main thread and Driver thread, both DISPLAY priority), eliminating priority inversion risks.
 *    - EXTREMELY BRIEF HOLD TIMES / LOW CONTENTION: The lock is taken for simple pointer swaps or list
 *      insertions. The fully-inlined userspace compare-and-swap (CAS) assembly fast-path executes
 *      instantly without incurring PLT/dynamic library call overhead or kernel syscalls.
 */

#include <utils/compiler.h>
#include <mutex>

#if defined(__ANDROID__)
#include <utils/linux/Mutex.h>
#else
#include <utils/generic/Mutex.h>
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
