/*
 * Copyright (C) 2025 The Android Open Source Project
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


#include "WebGPUHandles.h"

#include <chrono>

namespace filament::backend {

void WGPUTimerQuery::beginTimeElapsedQuery() {
    status->elapsedNanoseconds = 0;
    // Capture the timer query status via a weak_ptr because the WGPUTimerQuery could be destroyed
    // before the block executes.
    std::weak_ptr<WGPUTimerQuery::Status> statusPtr = status;

    if (auto s = statusPtr.lock()) {
        s->elapsedNanoseconds = std::chrono::steady_clock::now().time_since_epoch().count();
    }
}

void WGPUTimerQuery::endTimeElapsedQuery() {
    // Capture the timer query status via a weak_ptr because the WGPUTimerQuery could be destroyed
    // before the block executes.
    std::weak_ptr<WGPUTimerQuery::Status> statusPtr = status;
    if (auto s = statusPtr.lock()) {
        s->previousElapsed = s->elapsedNanoseconds = std::chrono::steady_clock::now().time_since_epoch().count() - s->elapsedNanoseconds;
    }
}

bool WGPUTimerQuery::getQueryResult(uint64_t* outElapsedTime) {
    if (status->previousElapsed == 0) {
        return false;
    }
    if (outElapsedTime) {
        *outElapsedTime = status->previousElapsed;
        status->previousElapsed = 0;
    }
    return true;
}

}// namespace filament::backend
