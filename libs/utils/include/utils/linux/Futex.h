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

#ifndef TNT_UTILS_LINUX_FUTEX_H
#define TNT_UTILS_LINUX_FUTEX_H

#include <utils/compiler.h>

#include <linux/futex.h>

#include <atomic>
#include <cstdint>
#include <limits>

#include <sys/syscall.h>
#include <unistd.h>

namespace utils {

class Futex {
public:
    static constexpr bool HAS_FUTEX = true;

    static inline void wait(std::atomic<uint32_t> const* addr, uint32_t value) noexcept {
        syscall(__NR_futex, const_cast<std::atomic<uint32_t>*>(addr),
                FUTEX_WAIT_PRIVATE, value, nullptr, nullptr, 0);
    }

    static inline void wake(std::atomic<uint32_t> const* addr, int count = 1) noexcept {
        syscall(__NR_futex, const_cast<std::atomic<uint32_t>*>(addr),
                FUTEX_WAKE_PRIVATE, count, nullptr, nullptr, 0);
    }

    static inline void wakeAll(std::atomic<uint32_t> const* addr) noexcept {
        wake(addr, std::numeric_limits<int>::max());
    }
};

} // namespace utils

#endif // TNT_UTILS_LINUX_FUTEX_H
