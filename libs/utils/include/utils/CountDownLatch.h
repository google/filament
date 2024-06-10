/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef TNT_UTILS_COUNTDOWNLATCH_H
#define TNT_UTILS_COUNTDOWNLATCH_H

// note: we use our version of mutex/condition to keep this public header STL free
#include <utils/Condition.h>
#include <utils/Mutex.h>

#include <stdint.h>
#include <stddef.h>

namespace utils {

/**
 * A count down latch is used to block one or several threads until the latch is signaled
 * a certain number of times.
 *
 * Threads entering the latch are blocked until the latch is signaled enough times.
 *
 * @see CyclicBarrier
 */
class CountDownLatch {
public:
    /**
     * Creates a count down latch with a specified count. The minimum useful value is 1.
     * @param count the latch counter initial value
     */
    explicit CountDownLatch(size_t count) noexcept;
    ~CountDownLatch() = default;

    /**
     * Blocks until latch() is called \p count times.
     * @see CountDownLatch(size_t count)
     */
    void await() noexcept;

    /**
     * Releases threads blocked in await() when called \p count times. Calling latch() more than
     * \p count times has no effect.
     * @see reset()
     */
    void latch() noexcept;

    /**
     * Resets the count-down latch to the given value.
     *
     * @param new_count New latch count. A value of zero will immediately unblock all waiting
     * threads.
     *
     * @warning Use with caution. It's only safe to reset the latch count when you're sure
     * that no threads are waiting in await(). This can be guaranteed in various ways, for
     * instance, if you have a single thread calling await(), you could call reset() from that
     * thread, or you could use a CyclicBarrier to make sure all threads using the CountDownLatch
     * are at a known place (i.e.: not in await()) when reset() is called.
     */
    void reset(size_t new_count) noexcept;

    /**
     * @return the number of times latch() has been called since construction or reset.
     * @see reset(),  CountDownLatch(size_t count)
     */
    size_t getCount() const noexcept;

    CountDownLatch() = delete;
    CountDownLatch(const CountDownLatch&) = delete;
    CountDownLatch& operator=(const CountDownLatch&) = delete;

private:
    uint32_t m_initial_count;
    uint32_t m_remaining_count;
    mutable Mutex m_lock;
    mutable Condition m_cv;
};

} // namespace utils

#endif // TNT_UTILS_COUNTDOWNLATCH_H
