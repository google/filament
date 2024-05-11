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

#ifndef TNT_UTILS_LINUX_CONDITION_H
#define TNT_UTILS_LINUX_CONDITION_H

#include <atomic>
#include <chrono>
#include <condition_variable> // for cv_status
#include <limits>
#include <mutex> // for unique_lock

#include <utils/linux/Mutex.h>

#include <time.h>

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

    void notify_n(size_t n) noexcept {
        if (n > 0) pulse(n);
    }

    void wait(std::unique_lock<Mutex>& lock) noexcept {
        wait_until(lock.mutex(), false, nullptr);
    }

    template <class P>
    void wait(std::unique_lock<Mutex>& lock, P predicate) {
        while (!predicate()) {
            wait(lock);
        }
    }

    template<typename D>
    std::cv_status wait_until(std::unique_lock<Mutex>& lock,
            const std::chrono::time_point<std::chrono::steady_clock, D>& timeout_time) noexcept {
        // convert to nanoseconds
        uint64_t ns = std::chrono::duration<uint64_t, std::nano>(timeout_time.time_since_epoch()).count();
        using sec_t = decltype(timespec::tv_sec);
        using nsec_t = decltype(timespec::tv_nsec);
        timespec ts{ sec_t(ns / 1000000000), nsec_t(ns % 1000000000) };
        return wait_until(lock.mutex(), false, &ts);
    }

    template<typename D>
    std::cv_status wait_until(std::unique_lock<Mutex>& lock,
            const std::chrono::time_point<std::chrono::system_clock, D>& timeout_time) noexcept {
        // convert to nanoseconds
        uint64_t ns = std::chrono::duration<uint64_t, std::nano>(timeout_time.time_since_epoch()).count();
        using sec_t = decltype(timespec::tv_sec);
        using nsec_t = decltype(timespec::tv_nsec);
        timespec ts{ sec_t(ns / 1000000000), nsec_t(ns % 1000000000) };
        return wait_until(lock.mutex(), true, &ts);
    }

    template<typename C, typename D, typename P>
    bool wait_until(std::unique_lock<Mutex>& lock,
            const std::chrono::time_point<C, D>& timeout_time, P predicate) noexcept {
        while (!predicate()) {
            if (wait_until(lock, timeout_time) == std::cv_status::timeout) {
                return predicate();
            }
        }
        return true;
    }

    template<typename R, typename Period>
    std::cv_status wait_for(std::unique_lock<Mutex>& lock,
            const std::chrono::duration<R, Period>& rel_time) noexcept {
        return wait_until(lock, std::chrono::steady_clock::now() + rel_time);
    }

    template<typename R, typename Period, typename P>
    bool wait_for(std::unique_lock<Mutex>& lock,
            const std::chrono::duration<R, Period>& rel_time, P pred) noexcept {
        return wait_until(lock, std::chrono::steady_clock::now() + rel_time, std::move(pred));
    }

private:
    std::atomic<uint32_t> mState = { 0 };

    void pulse(int threadCount) noexcept;

    std::cv_status wait_until(Mutex* lock,
            bool realtimeClock, timespec* ts) noexcept;
};

} // namespace utils

#endif // TNT_UTILS_LINUX_CONDITION_H
