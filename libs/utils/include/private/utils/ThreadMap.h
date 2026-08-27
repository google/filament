/*
 * Copyright (C) 2026 The Android Open Source Project
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

#ifndef TNT_UTILS_THREADMAP_H
#define TNT_UTILS_THREADMAP_H

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Panic.h>

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <limits>
#include <memory>
#include <thread>
#include <type_traits>
#include <utility>

#include <assert.h>
#include <stdint.h>

namespace utils {

/**
 * ThreadMap is a bounded-capacity, wait-free associative mapping from std::thread::id to
 * trivially copyable, lock-free values (such as pointers, handles, or integers).
 *
 * Preconditions & Concurrency Model (JobSystem Invariants):
 * ---------------------------------------------------------
 * ThreadMap is designed specifically for JobSystem where the following invariants hold:
 * 1. Fixed Capacity: Capacity is established at construction time (<= MAX_THREADS, typically <= 32)
 *    and is never dynamically resized.
 * 2. Exclusive Slot Ownership: Mutating operations (set, eraseAt) on a given slot index are
 *    performed exclusively by one thread at a time (guaranteed by JobSystem's thread pool
 *    partitioning and atomic adoptable slot mask). Two threads never concurrently mutate the
 *    same slot index.
 * 3. Thread-Bound Registration: Each thread registers itself (std::this_thread::get_id()) via
 *    set() and unregisters itself via erase()/eraseAt(). Lookups (get()) are predominantly
 *    invoked by a thread querying its own state.
 * 4. Lock-Free and Wait-Free: All reads and writes complete in bounded, single-pass O(N) scans
 *    with acquire-release atomics, without locks, CAS spin loops, seqlocks, version counters,
 *    or spurious misses.
 */
template <typename VALUE>
class ThreadMap {
    static_assert(std::is_trivially_copyable_v<VALUE>,
            "ThreadMap requires VALUE to be trivially copyable (e.g. pointers, handles)");
    static_assert(std::atomic<VALUE>::is_always_lock_free,
            "ThreadMap requires std::atomic<VALUE> to be lock-free on this platform");
    static_assert(std::atomic<std::thread::id>::is_always_lock_free,
            "ThreadMap requires std::atomic<std::thread::id> to be lock-free on this platform");
    static_assert(std::has_unique_object_representations_v<std::thread::id>,
            "ThreadMap requires std::thread::id to have unique object representations (no padding bits)");

public:
    using value_type = VALUE;
    using size_type = uint32_t;
    using key_type = std::thread::id;

    static constexpr size_type INVALID_INDEX = std::numeric_limits<size_type>::max();

    struct Entry {
        std::atomic<key_type> tid{};
        std::atomic<value_type> value{};
    };

    /**
     * Constructs a ThreadMap with the specified maximum thread capacity.
     *
     * @param capacity Maximum number of threads that can be registered simultaneously.
     * @note Not thread-safe. Standard C++ object lifecycle rules apply.
     */
    explicit ThreadMap(size_type capacity = 0);
    ~ThreadMap();

    ThreadMap(const ThreadMap&) = delete;
    ThreadMap& operator=(const ThreadMap&) = delete;

    ThreadMap(ThreadMap&& rhs) noexcept;
    ThreadMap& operator=(ThreadMap&& rhs) noexcept;

    /**
     * Returns the maximum thread capacity.
     *
     * @note Thread-safe (wait-free, read-only).
     */
    size_type getCapacity() const noexcept;

    /**
     * Associates the specified slot index with a thread ID and value.
     *
     * Precondition: index < getCapacity(). Two threads must not call set() concurrently
     * with the same slot index.
     *
     * @param index Slot index (must be < getCapacity()).
     * @param value The value to associate with the thread.
     * @param tid The thread ID (defaults to the current calling thread).
     * @note Thread-safe (wait-free) across distinct slot indices and against concurrent readers.
     */
    void set(size_type index, value_type value,
            key_type tid = std::this_thread::get_id()) noexcept;

    /**
     * Retrieves the value associated with the specified thread ID.
     *
     * @param tid The thread ID to look up (defaults to current thread).
     * @return The associated value, or value_type{} if not found.
     * @note Thread-safe (wait-free). Performs a single bounded O(N) scan.
     */
    inline value_type get(key_type const tid = std::this_thread::get_id()) const noexcept {
        if (UTILS_UNLIKELY(tid == key_type{} || mCapacity == 0)) {
            return value_type{};
        }
        for (size_type i = 0; i < mCapacity; ++i) {
            Entry const& entry = mEntries[i];
            if (entry.tid.load(std::memory_order_acquire) == tid) {
                value_type const val = entry.value.load(std::memory_order_relaxed);
                if (entry.tid.load(std::memory_order_acquire) == tid) {
                    return val;
                }
            }
        }
        return value_type{};
    }

    /**
     * Finds the slot index of the specified thread ID.
     *
     * @param tid The thread ID to look up (defaults to current thread).
     * @return The slot index if found, or INVALID_INDEX if not registered.
     * @note Thread-safe (wait-free). Performs a single bounded O(N) scan.
     */
    inline size_type findIndex(key_type const tid = std::this_thread::get_id()) const noexcept {
        if (UTILS_UNLIKELY(tid == key_type{} || mCapacity == 0)) {
            return INVALID_INDEX;
        }
        for (size_type i = 0; i < mCapacity; ++i) {
            if (mEntries[i].tid.load(std::memory_order_acquire) == tid) {
                return i;
            }
        }
        return INVALID_INDEX;
    }

    /**
     * Returns true if the specified thread is registered in this map.
     *
     * @param tid The thread ID to check (defaults to current thread).
     * @note Thread-safe (wait-free).
     */
    inline bool contains(key_type const tid = std::this_thread::get_id()) const noexcept {
        return findIndex(tid) != INVALID_INDEX;
    }

    /**
     * Unregisters the specified thread ID from the map and resets its slot to empty.
     *
     * @param tid The thread ID to unregister (defaults to current thread).
     * @return true if the thread was found and unregistered, false otherwise.
     * @note Thread-safe (lock-free).
     */
    bool erase(key_type tid = std::this_thread::get_id()) noexcept;

    /**
     * Resets the slot at the specified index to empty.
     *
     * Precondition: index < getCapacity().
     *
     * @param index The slot index to reset.
     * @return true if the slot was occupied and cleared, false if already empty.
     * @note Thread-safe (wait-free) across distinct slot indices and against concurrent readers.
     */
    bool eraseAt(size_type index) noexcept;

    /**
     * Resets all slots in the map to empty.
     *
     * @note Not thread-safe with concurrent mutating operations.
     */
    void clear() noexcept;

    /**
     * Returns the number of registered threads.
     *
     * @note Thread-safe (wait-free, approximate snapshot under concurrent mutation).
     */
    size_type size() const noexcept;

    /**
     * Returns true if no threads are currently registered.
     *
     * @note Thread-safe (wait-free, approximate snapshot under concurrent mutation).
     */
    bool empty() const noexcept;

private:
    size_type mCapacity = 0;
    std::unique_ptr<Entry[]> mEntries;
};

// ------------------------------------------------------------------------------------------------
// Out-of-line method definitions
// ------------------------------------------------------------------------------------------------

template <typename VALUE>
ThreadMap<VALUE>::ThreadMap(size_type const capacity)
        : mCapacity(capacity),
          mEntries(capacity ? std::make_unique<Entry[]>(capacity) : nullptr) {
}

template <typename VALUE>
ThreadMap<VALUE>::~ThreadMap() = default;

template <typename VALUE>
ThreadMap<VALUE>::ThreadMap(ThreadMap&& rhs) noexcept
        : mCapacity(std::exchange(rhs.mCapacity, 0)),
          mEntries(std::move(rhs.mEntries)) {
}

template <typename VALUE>
ThreadMap<VALUE>& ThreadMap<VALUE>::operator=(ThreadMap&& rhs) noexcept {
    if (this != &rhs) {
        mCapacity = std::exchange(rhs.mCapacity, 0);
        mEntries = std::move(rhs.mEntries);
    }
    return *this;
}

template <typename VALUE>
typename ThreadMap<VALUE>::size_type ThreadMap<VALUE>::getCapacity() const noexcept {
    return mCapacity;
}

template <typename VALUE>
void ThreadMap<VALUE>::set(size_type const index, value_type const value, key_type const tid) noexcept {
    assert_invariant(index < mCapacity);
    assert_invariant(tid != key_type{});

    Entry& entry = mEntries[index];
    entry.value.store(value, std::memory_order_relaxed);
    entry.tid.store(tid, std::memory_order_release);
}


template <typename VALUE>
bool ThreadMap<VALUE>::erase(key_type const tid) noexcept {
    if (UTILS_UNLIKELY(tid == key_type{} || mCapacity == 0)) {
        return false;
    }
    for (size_type i = 0; i < mCapacity; ++i) {
        Entry& entry = mEntries[i];
        key_type expected = tid;
        if (entry.tid.compare_exchange_strong(expected, key_type{},
                std::memory_order_release, std::memory_order_relaxed)) {
            return true;
        }
    }
    return false;
}

template <typename VALUE>
bool ThreadMap<VALUE>::eraseAt(size_type const index) noexcept {
    assert_invariant(index < mCapacity);

    Entry& entry = mEntries[index];
    key_type expected = entry.tid.load(std::memory_order_relaxed);
    if (expected != key_type{}) {
        if (entry.tid.compare_exchange_strong(expected, key_type{},
                std::memory_order_release, std::memory_order_relaxed)) {
            return true;
        }
    }
    return false;
}

template <typename VALUE>
void ThreadMap<VALUE>::clear() noexcept {
    for (size_type i = 0; i < mCapacity; ++i) {
        eraseAt(i);
    }
}

template <typename VALUE>
typename ThreadMap<VALUE>::size_type ThreadMap<VALUE>::size() const noexcept {
    size_type count = 0;
    for (size_type i = 0; i < mCapacity; ++i) {
        if (mEntries[i].tid.load(std::memory_order_relaxed) != key_type{}) {
            ++count;
        }
    }
    return count;
}

template <typename VALUE>
bool ThreadMap<VALUE>::empty() const noexcept {
    return size() == 0;
}

} // namespace utils

#endif // TNT_UTILS_THREADMAP_H
