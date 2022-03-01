/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include "utils/ThreadUtils.h"

#include <utils/compiler.h>

namespace utils {

UTILS_NOINLINE
std::thread::id ThreadUtils::getThreadId() noexcept {
    return std::this_thread::get_id();
}

bool ThreadUtils::isThisThread(std::thread::id id) noexcept {
    return getThreadId() == id;
}

} // namespace utils
