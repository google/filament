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

#ifndef UTILS_LINUX_FUTEX_H
#define UTILS_LINUX_FUTEX_H

#include <stdbool.h>

struct timespec;

namespace utils {
namespace linuxutil {

int futex_wake_ex(volatile void* ftx, bool shared, int count);
int futex_wait(volatile void* ftx, int value, const struct timespec* timeout);
int futex_wait_ex(volatile void* ftx, bool shared, int value,
        bool use_realtime_clock, const struct timespec* abs_timeout);

} // namespace linuxutil
} // namespace utils


#endif // UTILS_LINUX_FUTEX_H
