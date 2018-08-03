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

#ifndef UTILS_LINUX_CONDITION_H
#define UTILS_LINUX_CONDITION_H

#include <limits>
#include <mutex>
#include <utils/linux/Mutex.h>

namespace utils {


/*
 * A very simple condition variable class that can be used as an (almost) drop-in replacement
 * for std::condition_variable (doesn't have the timed wait() though).
 * It is very low overhead as most of it is inlined.
 */

class Condition {
public:
    Condition() noexcept = default;
    Condition(const Condition&) = delete;
    Condition& operator=(const Condition&) = delete;

    void notify_all() noexcept {
        pulse(std::numeric_limits<int>::max());
    }

    void notify_one() noexcept {
        pulse(1);
    }

    UTILS_NOINLINE
    void wait(std::unique_lock<Mutex>& lock) noexcept {
        uint32_t old_state = mState.load(std::memory_order_relaxed);
        lock.unlock();
        linuxutil::futex_wait_ex(&mState, false, old_state, false, nullptr);
        lock.lock();
    }

    template <class P>
    void wait(std::unique_lock<Mutex>& lock, P predicate) {
        while (!predicate()) {
            wait(lock);
        }
    }

private:
    std::atomic<uint32_t> mState = { 0 };

    inline void pulse(int threadCount) noexcept {
        mState.fetch_add(1, std::memory_order_relaxed);
        linuxutil::futex_wake_ex(&mState, false, threadCount);
    }
};

} // namespace utils

#endif // UTILS_LINUX_CONDITION_H
