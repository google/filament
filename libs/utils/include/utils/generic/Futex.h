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

#ifndef TNT_UTILS_GENERIC_FUTEX_H
#define TNT_UTILS_GENERIC_FUTEX_H

#include <utils/compiler.h>

#include <atomic>
#include <cstdint>

namespace utils {

class Futex {
public:
#if defined(__cpp_lib_atomic_wait) && (__cpp_lib_atomic_wait >= 201907L)
    static constexpr bool HAS_FUTEX = true;

    static inline void wait(std::atomic<uint32_t> const* addr, uint32_t value) noexcept {
        const_cast<std::atomic<uint32_t>*>(addr)->wait(value, std::memory_order_relaxed);
    }

    static inline void wake(std::atomic<uint32_t> const* addr, int count = 1) noexcept {
        if (count == 1) {
            const_cast<std::atomic<uint32_t>*>(addr)->notify_one();
        } else {
            const_cast<std::atomic<uint32_t>*>(addr)->notify_all();
        }
    }

    static inline void wakeAll(std::atomic<uint32_t> const* addr) noexcept {
        const_cast<std::atomic<uint32_t>*>(addr)->notify_all();
    }
#else
    static constexpr bool HAS_FUTEX = false;

    static inline void wait(std::atomic<uint32_t> const*, uint32_t) noexcept {}
    static inline void wake(std::atomic<uint32_t> const*, int = 1) noexcept {}
    static inline void wakeAll(std::atomic<uint32_t> const*) noexcept {}
#endif
};

} // namespace utils

#endif // TNT_UTILS_GENERIC_FUTEX_H
