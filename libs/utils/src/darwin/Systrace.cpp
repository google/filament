/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include <utils/darwin/Systrace.h>

#ifndef FILAMENT_APPLE_SYSTRACE
#   define FILAMENT_APPLE_SYSTRACE 0
#endif

#if FILAMENT_APPLE_SYSTRACE

#include <atomic>
#include <stack>
#include <stdint.h>
#include <pthread.h>

static pthread_once_t atrace_once_control = PTHREAD_ONCE_INIT;

thread_local std::stack<const char*> ___tracerSections;

namespace utils {
namespace details {

Systrace::GlobalState Systrace::sGlobalState = {};

void Systrace::init_once() noexcept {
    GlobalState& s = sGlobalState;

    s.systraceLog = os_log_create("com.google.filament", "systrace");
    s.frameIdLog = os_log_create("com.google.filament", "frameId");
}

void Systrace::setup() noexcept {
    pthread_once(&atrace_once_control, init_once);
}

void Systrace::enable(uint32_t tag) noexcept {
    setup();
    uint32_t const mask = 1 << tag;
    sGlobalState.isTracingEnabled.fetch_or(mask, std::memory_order_relaxed);
}

void Systrace::disable(uint32_t tag) noexcept {
    uint32_t const mask = 1 << tag;
    sGlobalState.isTracingEnabled.fetch_and(~mask, std::memory_order_relaxed);
}

// Unfortunately, this generates quite a bit of code because reading a global is not
// trivial. For this reason, we do not inline this method.
bool Systrace::isTracingEnabled(uint32_t tag) noexcept {
    if (tag) {
        setup();
        uint32_t const mask = 1 << tag;
        return bool(sGlobalState.isTracingEnabled.load(std::memory_order_relaxed) & mask);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------

void Systrace::init(uint32_t tag) noexcept {
    // must be called first
    mIsTracingEnabled = isTracingEnabled(tag);
}

} // namespace details
} // namespace utils

#endif // FILAMENT_APPLE_SYSTRACE
