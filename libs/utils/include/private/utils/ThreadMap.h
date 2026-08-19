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
 * ThreadMap is a bounded-capacity, lock-free and wait-free associative mapping from
 * std::thread::id to a value of type T (such as pointers, handles, or small structs).
 *
 * Designed for high-performance multithreading subsystems (such as job systems, thread pools,
 * and memory allocators) where a small, bounded number of threads need fast thread-local
 * association without relying on compiler-dependent thread_local storage (which is
 * problematic on some platforms and dynamic libraries).
 *
 * Sized dynamically at construction time, entries are stored in a contiguous array of
 * structures (AoS) synchronized via a seqlock version counter and atomic 3-state slot lifecycle
 * (EMPTY, WRITING, VALID). Lookups perform a linear scan over the capacity. For typical thread
 * counts (e.g. <= 32-64 threads), the entire map fits in 12-24 L1 cache lines, allowing lookups
 * to complete in ~1-2 nanoseconds without any mutex locking or hash table overhead on the
 * critical path.
 *
 * Usage Example:
 * --------------
 *
 * struct WorkerState {
 *     int workerId;
 *     WorkQueue queue;
 * };
 *
 * // Create a map with capacity for 8 threads
 * utils::ThreadMap<WorkerState*> threadMap(8);
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
template <typename VALUE>
class ThreadMap {
public:
    using value_type = VALUE;
    using size_type = uint32_t;
    using key_type = std::thread::id;

    static constexpr size_type INVALID_INDEX = std::numeric_limits<size_type>::max();

    enum class SlotState : uint8_t {
        EMPTY = 0,
        WRITING = 1,
        VALID = 2
    };

    struct Entry {
        std::atomic<uint32_t> version{0};
        std::atomic<SlotState> state{SlotState::EMPTY};
        std::atomic<key_type> tid{};
        value_type value{};
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
     * @param index Slot index (must be < getCapacity()).
     * @param value The value to associate with the thread.
     * @param tid The thread ID (defaults to the current calling thread).
     * @note Thread-safe (wait-free) across distinct slot indices and against concurrent readers.
     *       Concurrent calls to set() with the same slot index are not thread-safe.
     */
    void set(size_type index, value_type value,
            key_type tid = std::this_thread::get_id()) noexcept;

    /**
     * Registers a thread into the first available (empty) slot.
     *
     * @param value The value to associate with the thread.
     * @param tid The thread ID (defaults to the current calling thread).
     * @return The slot index where the thread was registered, or INVALID_INDEX if full.
     * @note Thread-safe (lock-free). Multiple threads may call emplace() concurrently.
     */
    size_type emplace(value_type value,
            key_type tid = std::this_thread::get_id()) noexcept;

    /**
     * Finds the pointer to the value associated with the specified thread ID.
     *
     * @param tid The thread ID to look up (defaults to the current calling thread).
     * @return Pointer to the stored value if found, or nullptr if not found.
     * @note Thread-safe (wait-free). Dereferencing the returned pointer while another thread
     *       mutates or erases that specific slot is not thread-safe.
     */
    inline value_type* find(key_type const tid = std::this_thread::get_id()) noexcept {
        size_type const idx = findIndex(tid);
        if (idx != INVALID_INDEX) {
            return &mEntries[idx].value;
        }
        return nullptr;
    }

    /**
     * Finds the const pointer to the value associated with the specified thread ID.
     *
     * @param tid The thread ID to look up (defaults to the current calling thread).
     * @return Const pointer to the stored value if found, or nullptr if not found.
     * @note Thread-safe (wait-free). Dereferencing the returned pointer while another thread
     *       mutates or erases that specific slot is not thread-safe.
     */
    inline const value_type* find(key_type const tid = std::this_thread::get_id()) const noexcept {
        size_type const idx = findIndex(tid);
        if (idx != INVALID_INDEX) {
            return &mEntries[idx].value;
        }
        return nullptr;
    }

    /**
     * Retrieves the value associated with the specified thread ID.
     * Convenience method when value_type is a pointer or default-constructible type.
     *
     * @param tid The thread ID to look up (defaults to current thread).
     * @return The associated value, or value_type{} if not found.
     * @note Thread-safe (wait-free). Safe to call concurrently with all mutating operations
     *       for pointer or trivially copyable value types.
     */
    inline value_type get(key_type const tid = std::this_thread::get_id()) const noexcept {
        if (UTILS_UNLIKELY(tid == key_type{} || mCapacity == 0)) {
            return value_type{};
        }
        for (size_type i = 0; i < mCapacity; ++i) {
            Entry const& entry = mEntries[i];
            for (int retry = 0; retry < 64; ++retry) {
                uint32_t const v1 = entry.version.load(std::memory_order_acquire);
                if (v1 & 1) {
                    continue;
                }
                if (entry.state.load(std::memory_order_acquire) != SlotState::VALID) {
                    break;
                }
                if (entry.tid.load(std::memory_order_relaxed) != tid) {
                    break;
                }
                value_type val = entry.value;
                uint32_t const v2 = entry.version.load(std::memory_order_acquire);
                if (v1 == v2 && entry.state.load(std::memory_order_acquire) == SlotState::VALID &&
                    entry.tid.load(std::memory_order_relaxed) == tid) {
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
     * @note Thread-safe (wait-free).
     */
    inline size_type findIndex(key_type const tid = std::this_thread::get_id()) const noexcept {
        if (UTILS_UNLIKELY(tid == key_type{} || mCapacity == 0)) {
            return INVALID_INDEX;
        }
        for (size_type i = 0; i < mCapacity; ++i) {
            Entry const& entry = mEntries[i];
            for (int retry = 0; retry < 64; ++retry) {
                uint32_t const v1 = entry.version.load(std::memory_order_acquire);
                if (v1 & 1) {
                    continue;
                }
                if (entry.state.load(std::memory_order_acquire) != SlotState::VALID) {
                    break;
                }
                if (entry.tid.load(std::memory_order_relaxed) != tid) {
                    break;
                }
                uint32_t const v2 = entry.version.load(std::memory_order_acquire);
                if (v1 == v2 && entry.state.load(std::memory_order_acquire) == SlotState::VALID &&
                    entry.tid.load(std::memory_order_relaxed) == tid) {
                    return i;
                }
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
     * Unregisters the thread at a specific slot index.
     *
     * @param index The slot index to clear (must be < getCapacity()).
     * @return true if the slot was valid and cleared, false otherwise.
     * @note Thread-safe (lock-free).
     */
    bool eraseAt(size_type index) noexcept;

    /**
     * Clears all registered threads from the map.
     *
     * @note Thread-safe (lock-free).
     */
    void clear() noexcept;

    /**
     * Returns the number of currently registered threads.
     *
     * @note Thread-safe (wait-free). Returns an instantaneous snapshot count.
     */
    size_type size() const noexcept;

    /**
     * Returns true if no threads are currently registered.
     *
     * @note Thread-safe (wait-free).
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
void ThreadMap<VALUE>::set(size_type const index, value_type value, key_type const tid) noexcept {
    assert_invariant(index < mCapacity);
    assert_invariant(tid != key_type{});

    Entry& entry = mEntries[index];
    entry.version.fetch_add(1, std::memory_order_acq_rel);
    entry.state.store(SlotState::WRITING, std::memory_order_release);

    entry.tid.store(tid, std::memory_order_relaxed);
    entry.value = std::move(value);

    entry.state.store(SlotState::VALID, std::memory_order_release);
    entry.version.fetch_add(1, std::memory_order_release);
}

template <typename VALUE>
typename ThreadMap<VALUE>::size_type ThreadMap<VALUE>::emplace(value_type value,
        key_type const tid) noexcept {
    assert_invariant(tid != key_type{});

    while (true) {
        bool writingObserved = false;
        for (size_type i = 0; i < mCapacity; ++i) {
            Entry& entry = mEntries[i];
            SlotState expected = SlotState::EMPTY;
            if (entry.state.compare_exchange_strong(expected, SlotState::WRITING,
                    std::memory_order_acquire, std::memory_order_relaxed)) {
                entry.version.fetch_add(1, std::memory_order_acq_rel);

                entry.tid.store(tid, std::memory_order_relaxed);
                entry.value = std::move(value);

                entry.state.store(SlotState::VALID, std::memory_order_release);
                entry.version.fetch_add(1, std::memory_order_release);
                return i;
            }
            if (expected == SlotState::VALID &&
                entry.state.load(std::memory_order_acquire) == SlotState::VALID &&
                entry.tid.load(std::memory_order_relaxed) == tid) {
                SlotState stateExpected = SlotState::VALID;
                if (entry.state.compare_exchange_strong(stateExpected, SlotState::WRITING,
                        std::memory_order_acquire, std::memory_order_relaxed)) {
                    entry.version.fetch_add(1, std::memory_order_acq_rel);

                    entry.value = std::move(value);

                    entry.state.store(SlotState::VALID, std::memory_order_release);
                    entry.version.fetch_add(1, std::memory_order_release);
                    return i;
                }
            }
            if (expected == SlotState::WRITING ||
                entry.state.load(std::memory_order_relaxed) == SlotState::WRITING) {
                writingObserved = true;
            }
        }
        if (!writingObserved) {
            return INVALID_INDEX;
        }
        std::this_thread::yield();
    }
}

template <typename VALUE>
bool ThreadMap<VALUE>::erase(key_type const tid) noexcept {
    size_type const idx = findIndex(tid);
    if (idx != INVALID_INDEX) {
        return eraseAt(idx);
    }
    return false;
}

template <typename VALUE>
bool ThreadMap<VALUE>::eraseAt(size_type const index) noexcept {
    assert_invariant(index < mCapacity);

    Entry& entry = mEntries[index];
    SlotState expected = SlotState::VALID;
    if (entry.state.compare_exchange_strong(expected, SlotState::WRITING,
            std::memory_order_acquire, std::memory_order_relaxed)) {
        entry.version.fetch_add(1, std::memory_order_acq_rel);

        entry.tid.store(key_type{}, std::memory_order_relaxed);
        entry.value = value_type{};

        entry.state.store(SlotState::EMPTY, std::memory_order_release);
        entry.version.fetch_add(1, std::memory_order_release);
        return true;
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
        if (mEntries[i].state.load(std::memory_order_relaxed) == SlotState::VALID) {
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
