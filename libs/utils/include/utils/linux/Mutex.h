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

#ifndef TNT_UTILS_LINUX_MUTEX_H
#define TNT_UTILS_LINUX_MUTEX_H

#include <utils/compiler.h>

#include <atomic>

#include <stdint.h>

namespace utils {

/*
 * A very simple mutex class that can be used as an (almost) drop-in replacement
 * for std::mutex.
 * It is very low overhead as most of it is inlined.
 */

class Mutex {
public:
    constexpr Mutex() noexcept = default;
    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;

    void lock() noexcept {
        uint32_t old_state = UNLOCKED;
        if (UTILS_UNLIKELY(!mState.compare_exchange_strong(old_state,
                LOCKED, std::memory_order_acquire, std::memory_order_relaxed))) {
            wait();
        }
    }

    void unlock() noexcept {
        if (UTILS_UNLIKELY(mState.exchange(UNLOCKED, std::memory_order_release) == LOCKED_CONTENDED)) {
            wake();
        }
    }

private:
    enum {
        UNLOCKED = 0, LOCKED = 1, LOCKED_CONTENDED = 2
    };
    std::atomic<uint32_t> mState = { UNLOCKED };

    void wait() noexcept;
    void wake() noexcept;
};

} // namespace utils

#endif // TNT_UTILS_LINUX_MUTEX_H
