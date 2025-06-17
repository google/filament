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

#include "WebGPUTimerQuery.h"

#include <chrono>
#include <cstdint>
#include <memory>

namespace filament::backend {

void WebGPUTimerQuery::beginTimeElapsedQuery() {
    mStatus->elapsedNanoseconds = 0;
    // Capture the timer query status via a weak_ptr because the WGPUTimerQuery could be destroyed
    // before the block executes.
    std::weak_ptr<WebGPUTimerQuery::Status> statusPtr = mStatus;

    if (auto s = statusPtr.lock()) {
        s->elapsedNanoseconds = std::chrono::steady_clock::now().time_since_epoch().count();
    }
}

void WebGPUTimerQuery::endTimeElapsedQuery() {
    // Capture the timer query status via a weak_ptr because the WGPUTimerQuery could be destroyed
    // before the block executes.
    if (mStatus->elapsedNanoseconds != 0) {
        std::weak_ptr<WebGPUTimerQuery::Status> statusPtr = mStatus;
        if (auto s = statusPtr.lock()) {
            s->previousElapsed = s->elapsedNanoseconds =
                    std::chrono::steady_clock::now().time_since_epoch().count() -
                    s->elapsedNanoseconds;
        }
    }
}

bool WebGPUTimerQuery::getQueryResult(uint64_t* outElapsedTime) {
    if (mStatus->previousElapsed == 0) {
        return false;
    }
    if (outElapsedTime) {
        *outElapsedTime = mStatus->previousElapsed;
        mStatus->previousElapsed = 0;
    }
    return true;
}

}// namespace filament::backend
