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

#ifndef TNT_UTILS_GENERIC_CONDITION_H
#define TNT_UTILS_GENERIC_CONDITION_H

#include <condition_variable>

#include <stddef.h>

namespace utils {

class Condition : public std::condition_variable {
public:
    using std::condition_variable::condition_variable;

    inline void notify_n(size_t n) noexcept {
        if (n == 1) {
            notify_one();
        } else if (n > 1) {
            notify_all();
        }
    }
};

} // namespace utils

#endif // TNT_UTILS_GENERIC_CONDITION_H
