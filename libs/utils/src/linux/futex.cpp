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

#if defined(__linux__)

#include <errno.h>
#include <linux/futex.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/cdefs.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <utils/linux/futex.h>

namespace utils {
namespace linuxutil {

static inline int futex(volatile void* ftx, int op, int value,
        const struct timespec* timeout, int bitset) {
    return syscall(__NR_futex, ftx, op, value, timeout, NULL, bitset);
}

int futex_wake(volatile void* ftx, int count) {
    return futex(ftx, FUTEX_WAKE, count, NULL, 0);
}

int futex_wake_ex(volatile void* ftx, bool shared, int count) {
    return futex(ftx, shared ? FUTEX_WAKE : FUTEX_WAKE_PRIVATE, count, NULL, 0);
}
int futex_wait(volatile void* ftx, int value, const struct timespec* timeout) {
    return futex(ftx, FUTEX_WAIT, value, timeout, 0);
}

int futex_wait_ex(volatile void* ftx, bool shared, int value,
        bool use_realtime_clock, const struct timespec* abs_timeout) {
    return futex(ftx, (shared ? FUTEX_WAIT_BITSET : FUTEX_WAIT_BITSET_PRIVATE) |
                        (use_realtime_clock ? FUTEX_CLOCK_REALTIME : 0), value, abs_timeout,
            FUTEX_BITSET_MATCH_ANY);
}

} //namespace linuxutil
} // namespace utils

#endif
