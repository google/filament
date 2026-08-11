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
 * Sized dynamically at construction time, thread IDs and values are stored in
 * cache-line-packed arrays synchronized via an atomic 3-state slot lifecycle (EMPTY, WRITING, VALID).
 * Lookups perform a linear scan over the capacity. For typical thread counts (e.g. <= 32-64
 * threads), all thread IDs fit in 1-4 L1 cache lines, allowing lookups to complete in ~1-2
 * nanoseconds without any mutex locking or hash table overhead on the critical path.
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
class UTILS_PUBLIC ThreadMap {
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

    /**
     * Constructs a ThreadMap with the specified maximum thread capacity.
     *
     * @param capacity Maximum number of threads that can be registered simultaneously.
     * @note Not thread-safe. Standard C++ object lifecycle rules apply.
     */
    explicit ThreadMap(size_type const capacity = 0)
            : mCapacity(capacity),
              mStates(capacity ? new std::atomic<SlotState>[capacity]{} : nullptr),
              mThreadIds(capacity ? new key_type[capacity]{} : nullptr),
              mValues(capacity ? new value_type[capacity]{} : nullptr) {
    }

    ~ThreadMap() = default;

    ThreadMap(const ThreadMap&) = delete;
    ThreadMap& operator=(const ThreadMap&) = delete;

    ThreadMap(ThreadMap&& rhs) noexcept
            : mCapacity(std::exchange(rhs.mCapacity, 0)),
              mStates(std::move(rhs.mStates)),
              mThreadIds(std::move(rhs.mThreadIds)),
              mValues(std::move(rhs.mValues)) {
    }

    ThreadMap& operator=(ThreadMap&& rhs) noexcept {
        if (this != &rhs) {
            mCapacity = std::exchange(rhs.mCapacity, 0);
            mStates = std::move(rhs.mStates);
            mThreadIds = std::move(rhs.mThreadIds);
            mValues = std::move(rhs.mValues);
        }
        return *this;
    }

    /**
     * Returns the maximum thread capacity.
     *
     * @note Thread-safe (wait-free, read-only).
     */
    size_type getCapacity() const noexcept {
        return mCapacity;
    }

    /**
     * Associates the specified slot index with a thread ID and value.
     *
     * @param index Slot index (must be < getCapacity()).
     * @param value The value to associate with the thread.
     * @param tid The thread ID (defaults to the current calling thread).
     * @note Thread-safe (wait-free) across distinct slot indices and against concurrent readers.
     *       Concurrent calls to set() with the same slot index are not thread-safe.
     */
    void set(size_type const index, value_type value,
            key_type const tid = std::this_thread::get_id()) noexcept {
        assert_invariant(index < mCapacity);
        assert_invariant(tid != key_type{});
        mStates[index].store(SlotState::WRITING, std::memory_order_release);
        mThreadIds[index] = tid;
        mValues[index] = std::move(value);
        mStates[index].store(SlotState::VALID, std::memory_order_release);
    }

    /**
     * Registers a thread into the first available (empty) slot.
     *
     * @param value The value to associate with the thread.
     * @param tid The thread ID (defaults to the current calling thread).
     * @return The slot index where the thread was registered, or INVALID_INDEX if full.
     * @note Thread-safe (lock-free). Multiple threads may call emplace() concurrently.
     */
    size_type emplace(value_type value,
            key_type const tid = std::this_thread::get_id()) noexcept {
        assert_invariant(tid != key_type{});
        for (size_type i = 0; i < mCapacity; ++i) {
            SlotState expected = SlotState::EMPTY;
            if (mStates[i].compare_exchange_strong(expected, SlotState::WRITING,
                    std::memory_order_acquire, std::memory_order_relaxed)) {
                mThreadIds[i] = tid;
                mValues[i] = std::move(value);
                mStates[i].store(SlotState::VALID, std::memory_order_release);
                return i;
            }
            if (expected == SlotState::VALID &&
                mStates[i].load(std::memory_order_acquire) == SlotState::VALID &&
                mThreadIds[i] == tid) {
                SlotState stateExpected = SlotState::VALID;
                if (mStates[i].compare_exchange_strong(stateExpected, SlotState::WRITING,
                        std::memory_order_acquire, std::memory_order_relaxed)) {
                    mValues[i] = std::move(value);
                    mStates[i].store(SlotState::VALID, std::memory_order_release);
                    return i;
                }
            }
        }
        return INVALID_INDEX;
    }

    /**
     * Finds the pointer to the value associated with the specified thread ID.
     *
     * @param tid The thread ID to look up (defaults to the current calling thread).
     * @return Pointer to the stored value if found, or nullptr if not found.
     * @note Thread-safe (wait-free). Dereferencing the returned pointer while another thread
     *       mutates or erases that specific slot is not thread-safe.
     */
    value_type* find(key_type const tid = std::this_thread::get_id()) noexcept {
        size_type const idx = findIndex(tid);
        if (idx != INVALID_INDEX && mStates[idx].load(std::memory_order_acquire) == SlotState::VALID) {
            return &mValues[idx];
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
    const value_type* find(key_type const tid = std::this_thread::get_id()) const noexcept {
        size_type const idx = findIndex(tid);
        if (idx != INVALID_INDEX && mStates[idx].load(std::memory_order_acquire) == SlotState::VALID) {
            return &mValues[idx];
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
    value_type get(key_type const tid = std::this_thread::get_id()) const noexcept {
        size_type const idx = findIndex(tid);
        if (idx != INVALID_INDEX) {
            value_type val = mValues[idx];
            if (mStates[idx].load(std::memory_order_acquire) == SlotState::VALID) {
                return val;
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
    size_type findIndex(key_type const tid = std::this_thread::get_id()) const noexcept {
        if (UTILS_UNLIKELY(tid == key_type{} || mCapacity == 0)) {
            return INVALID_INDEX;
        }
        for (size_type i = 0; i < mCapacity; ++i) {
            if (mStates[i].load(std::memory_order_acquire) == SlotState::VALID) {
                if (UTILS_LIKELY(mThreadIds[i] == tid)) {
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
    bool contains(key_type const tid = std::this_thread::get_id()) const noexcept {
        return findIndex(tid) != INVALID_INDEX;
    }

    /**
     * Unregisters the specified thread ID from the map and resets its slot to empty.
     *
     * @param tid The thread ID to unregister (defaults to current thread).
     * @return true if the thread was found and unregistered, false otherwise.
     * @note Thread-safe (lock-free).
     */
    bool erase(key_type const tid = std::this_thread::get_id()) noexcept {
        size_type const idx = findIndex(tid);
        if (idx != INVALID_INDEX) {
            return eraseAt(idx);
        }
        return false;
    }

    /**
     * Unregisters the thread at a specific slot index.
     *
     * @param index The slot index to clear (must be < getCapacity()).
     * @return true if the slot was valid and cleared, false otherwise.
     * @note Thread-safe (lock-free).
     */
    bool eraseAt(size_type const index) noexcept {
        assert_invariant(index < mCapacity);
        SlotState expected = SlotState::VALID;
        if (mStates[index].compare_exchange_strong(expected, SlotState::WRITING,
                std::memory_order_acquire, std::memory_order_relaxed)) {
            mThreadIds[index] = key_type{};
            mValues[index] = value_type{};
            mStates[index].store(SlotState::EMPTY, std::memory_order_release);
            return true;
        }
        return false;
    }

    /**
     * Clears all registered threads from the map.
     *
     * @note Thread-safe (lock-free).
     */
    void clear() noexcept {
        for (size_type i = 0; i < mCapacity; ++i) {
            eraseAt(i);
        }
    }

    /**
     * Returns the number of currently registered threads.
     *
     * @note Thread-safe (wait-free). Returns an instantaneous snapshot count.
     */
    size_type size() const noexcept {
        size_type count = 0;
        for (size_type i = 0; i < mCapacity; ++i) {
            if (mStates[i].load(std::memory_order_relaxed) == SlotState::VALID) {
                ++count;
            }
        }
        return count;
    }

    /**
     * Returns true if no threads are currently registered.
     *
     * @note Thread-safe (wait-free).
     */
    bool empty() const noexcept {
        return size() == 0;
    }

private:
    size_type mCapacity = 0;
    std::unique_ptr<std::atomic<SlotState>[]> mStates;
    std::unique_ptr<key_type[]> mThreadIds;
    std::unique_ptr<value_type[]> mValues;
};

} // namespace utils

#endif // TNT_UTILS_THREADMAP_H
