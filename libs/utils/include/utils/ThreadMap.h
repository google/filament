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
#include <thread>
#include <type_traits>
#include <utility>

#include <assert.h>
#include <stdint.h>

namespace utils {

/**
 * ThreadMap is a fixed-capacity, lock-free and wait-free associative mapping from
 * std::thread::id to a value of type T (such as pointers, handles, or small structs).
 *
 * Designed for high-performance multithreading subsystems (such as job systems, thread pools,
 * and memory allocators) where a small, bounded number of threads need fast thread-local
 * association without relying on compiler-dependent thread_local storage (which is
 * problematic on some platforms and dynamic libraries).
 *
 * Thread IDs are stored in an inline, cache-line-packed array of std::atomic<std::thread::id>.
 * Lookups perform a linear scan over the active capacity. For small thread counts (e.g. <= 32-64
 * threads), all thread IDs fit in 1-4 L1 cache lines, allowing lookups to complete in ~1-2
 * nanoseconds without any mutex locking, hash table overhead, or heap allocations on the critical
 * path.
 *
 * Usage Example:
 * --------------
 *
 * struct WorkerState {
 *     int workerId;
 *     WorkQueue queue;
 * };
 *
 * // Create a map with compile-time fixed capacity for 8 threads (no heap allocation)
 * utils::ThreadMap<WorkerState*, 8> threadMap;
 *
 * // In each worker thread:
 * threadMap.set(slotIndex, &localState);
 *
 * // Later, in any worker or caller thread:
 * WorkerState* state = threadMap.get(); // looks up std::this_thread::get_id()
 * if (state) {
 *     state->queue.push(...);
 * }
 *
 * // When shutting down or when a thread exits:
 * threadMap.erase(); // unregisters std::this_thread::get_id()
 */
template <typename VALUE, uint32_t CAPACITY>
class UTILS_PUBLIC ThreadMap {
    static_assert(CAPACITY > 0, "CAPACITY must be greater than zero");

public:
    using value_type = VALUE;
    using size_type = uint32_t;
    using key_type = std::thread::id;

    static constexpr size_type INVALID_INDEX = std::numeric_limits<size_type>::max();
    static constexpr size_type CAPACITY_COUNT = CAPACITY;

    /**
     * Constructs an empty ThreadMap. No heap allocations are performed.
     */
    ThreadMap() noexcept = default;

    ~ThreadMap() = default;

    ThreadMap(const ThreadMap&) = delete;
    ThreadMap& operator=(const ThreadMap&) = delete;
    ThreadMap(ThreadMap&&) = delete;
    ThreadMap& operator=(ThreadMap&&) = delete;

    /**
     * Returns the maximum thread capacity.
     */
    static constexpr size_type getCapacity() noexcept {
        return CAPACITY;
    }

    /**
     * Associates the specified slot index with a thread ID and value.
     *
     * @param index Slot index (must be < getCapacity()).
     * @param value The value to associate with the thread.
     * @param tid The thread ID (defaults to the current calling thread).
     */
    void set(size_type const index, value_type value,
            key_type const tid = std::this_thread::get_id()) noexcept {
        assert_invariant(index < CAPACITY);
        assert_invariant(tid != key_type{});
        mValues[index] = std::move(value);
        mThreadIds[index].store(tid, std::memory_order_release);
    }

    /**
     * Registers a thread into the first available (empty) slot.
     *
     * @param value The value to associate with the thread.
     * @param tid The thread ID (defaults to the current calling thread).
     * @return The slot index where the thread was registered, or INVALID_INDEX if full.
     */
    size_type emplace(value_type value,
            key_type const tid = std::this_thread::get_id()) noexcept {
        assert_invariant(tid != key_type{});
        const key_type empty{};
        for (size_type i = 0; i < CAPACITY; ++i) {
            key_type expected = empty;
            if (mThreadIds[i].compare_exchange_strong(expected, tid,
                    std::memory_order_acq_rel, std::memory_order_relaxed)) {
                mValues[i] = std::move(value);
                return i;
            }
            if (expected == tid) {
                mValues[i] = std::move(value);
                return i;
            }
        }
        return INVALID_INDEX;
    }

    /**
     * Finds the pointer to the value associated with the specified thread ID.
     * Wait-free and lock-free.
     *
     * @param tid The thread ID to look up (defaults to the current calling thread).
     * @return Pointer to the stored value if found, or nullptr if not found.
     */
    value_type* find(key_type const tid = std::this_thread::get_id()) noexcept {
        size_type const idx = findIndex(tid);
        return (idx != INVALID_INDEX) ? &mValues[idx] : nullptr;
    }

    /**
     * Finds the const pointer to the value associated with the specified thread ID.
     * Wait-free and lock-free.
     *
     * @param tid The thread ID to look up (defaults to the current calling thread).
     * @return Const pointer to the stored value if found, or nullptr if not found.
     */
    const value_type* find(key_type const tid = std::this_thread::get_id()) const noexcept {
        size_type const idx = findIndex(tid);
        return (idx != INVALID_INDEX) ? &mValues[idx] : nullptr;
    }

    /**
     * Retrieves the value associated with the specified thread ID.
     * Convenience method when value_type is a pointer or default-constructible type.
     *
     * @param tid The thread ID to look up (defaults to current thread).
     * @return The associated value, or value_type{} if not found.
     */
    value_type get(key_type const tid = std::this_thread::get_id()) const noexcept {
        size_type const idx = findIndex(tid);
        return (idx != INVALID_INDEX) ? mValues[idx] : value_type{};
    }

    /**
     * Finds the slot index of the specified thread ID.
     * Wait-free and lock-free.
     *
     * @param tid The thread ID to look up (defaults to current thread).
     * @return The slot index if found, or INVALID_INDEX if not registered.
     */
    size_type findIndex(key_type const tid = std::this_thread::get_id()) const noexcept {
        if (UTILS_UNLIKELY(tid == key_type{})) {
            return INVALID_INDEX;
        }
        for (size_type i = 0; i < CAPACITY; ++i) {
            if (UTILS_LIKELY(mThreadIds[i].load(std::memory_order_acquire) == tid)) {
                return i;
            }
        }
        return INVALID_INDEX;
    }

    /**
     * Returns true if the specified thread is registered in this map.
     *
     * @param tid The thread ID to check (defaults to current thread).
     */
    bool contains(key_type const tid = std::this_thread::get_id()) const noexcept {
        return findIndex(tid) != INVALID_INDEX;
    }

    /**
     * Unregisters the specified thread ID from the map and resets its slot to empty.
     *
     * @param tid The thread ID to unregister (defaults to current thread).
     * @return true if the thread was found and unregistered, false otherwise.
     */
    bool erase(key_type const tid = std::this_thread::get_id()) noexcept {
        size_type const idx = findIndex(tid);
        if (idx != INVALID_INDEX) {
            mThreadIds[idx].store(key_type{}, std::memory_order_release);
            return true;
        }
        return false;
    }

    /**
     * Unregisters the thread at a specific slot index.
     *
     * @param index The slot index to clear (must be < getCapacity()).
     */
    void eraseAt(size_type const index) noexcept {
        assert_invariant(index < CAPACITY);
        mThreadIds[index].store(key_type{}, std::memory_order_release);
    }

    /**
     * Clears all registered threads from the map.
     */
    void clear() noexcept {
        for (size_type i = 0; i < CAPACITY; ++i) {
            mThreadIds[i].store(key_type{}, std::memory_order_relaxed);
        }
    }

    /**
     * Returns the number of currently registered threads.
     */
    size_type size() const noexcept {
        size_type count = 0;
        const key_type empty{};
        for (size_type i = 0; i < CAPACITY; ++i) {
            if (mThreadIds[i].load(std::memory_order_relaxed) != empty) {
                ++count;
            }
        }
        return count;
    }

    /**
     * Returns true if no threads are currently registered.
     */
    bool empty() const noexcept {
        return size() == 0;
    }

private:
    std::atomic<key_type> mThreadIds[CAPACITY]{};
    value_type mValues[CAPACITY]{};
};

} // namespace utils

#endif // TNT_UTILS_THREADMAP_H
