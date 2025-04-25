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

void WGPUTimerQuery::beginTimeElapsedQuery(WGPUTimerQuery* query) {
    query->status->elapsed = 0;
    query->status->available.store(false);

    // Capture the timer query status via a weak_ptr because the MetalTimerQuery could be destroyed
    // before the block executes.
    std::weak_ptr<WGPUTimerQuery::Status> status = query->status;

    if (auto s = status.lock()) {
        s->elapsed = std::chrono::steady_clock::now().time_since_epoch().count();
    }

    ;
}

void WGPUTimerQuery::endTimeElapsedQuery(WGPUTimerQuery* query) {
    // Capture the timer query status via a weak_ptr because the WGPUTimerQuery could be destroyed
    // before the block executes.
    std::weak_ptr<WGPUTimerQuery::Status> status = query->status;
    if (auto s = status.lock()) {
        s->elapsed = std::chrono::steady_clock::now().time_since_epoch().count() - s->elapsed;
        s->available.store(true);
    }
}

bool WGPUTimerQuery::getQueryResult(WGPUTimerQuery* query, uint64_t* outElapsedTime) {
    if (!query->status->available.load()) {
        return false;
    }
    if (outElapsedTime) {
        *outElapsedTime = query->status->elapsed;
    }
    return true;
}

}// namespace filament::backend
