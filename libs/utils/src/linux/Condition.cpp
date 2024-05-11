/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <utils/linux/Condition.h>

#include "futex.h"

namespace utils {

std::cv_status Condition::wait_until(Mutex* lock,
        bool realtimeClock, struct timespec* ts) noexcept {
    if (ts && ts->tv_sec < 0) {
        return std::cv_status::timeout;
    }
    uint32_t old_state = mState.load(std::memory_order_relaxed);
    lock->unlock();
    int status = linuxutil::futex_wait_ex(&mState, false, old_state, realtimeClock, ts);
    lock->lock();
    return (status == -ETIMEDOUT) ? std::cv_status::timeout : std::cv_status::no_timeout;
}

void Condition::pulse(int threadCount) noexcept {
    mState.fetch_add(1, std::memory_order_relaxed);
    linuxutil::futex_wake_ex(&mState, false, threadCount);
}

} // namespace utils
