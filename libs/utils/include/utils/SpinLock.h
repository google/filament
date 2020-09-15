/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef TNT_UTILS_SPINLOCK_H
#define TNT_UTILS_SPINLOCK_H

#include <utils/compiler.h>

#include <atomic>
#include <type_traits>

#include <assert.h>
#include <stddef.h>

namespace utils {

class SpinLock {
    std::atomic_flag mLock = ATOMIC_FLAG_INIT;

public:
    void lock() noexcept {
        UTILS_PREFETCHW(&mLock);
#ifdef __ARM_ACLE
        // we signal an event on this CPU, so that the first yield() will be a no-op,
        // and falls through the test_and_set(). This is more efficient than a while { }
        // construct.
        UTILS_SIGNAL_EVENT();
        do {
            yield();
        } while (mLock.test_and_set(std::memory_order_acquire));
#else
        goto start;
        do {
            yield();
start: ;
        } while (mLock.test_and_set(std::memory_order_acquire));
#endif
    }

    void unlock() noexcept {
        mLock.clear(std::memory_order_release);
#ifdef __ARM_ARCH_7A__
        // on ARMv7a SEL is needed
        UTILS_SIGNAL_EVENT();
        // as well as a memory barrier is needed
        __dsb(0xA);     // ISHST = 0xA (b1010)
#else
        // on ARMv8 we could avoid the call to SE, but we'd need to write the
        // test_and_set() above by hand, so the WFE only happens without a STRX first.
        UTILS_BROADCAST_EVENT();
#endif
    }

private:
    inline void yield() noexcept {
        // on x86 call pause instruction, on ARM call WFE
        UTILS_WAIT_FOR_EVENT();
    }
};

} // namespace utils

#endif //TNT_UTILS_SPINLOCK_H
