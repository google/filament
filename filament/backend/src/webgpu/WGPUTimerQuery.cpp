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
#include <memory>
#include <iostream>

namespace filament::backend {

void WGPUTimerQuery::beginTimeElapsedQuery() {
    std::cout << "    WGPUTimerQuery::beginTimeElapsedQuery()" <<std::endl;
    status->elapsedNanoseconds = 0;
    // Capture the timer query status via a weak_ptr because the WGPUTimerQuery could be destroyed
    // before the block executes.
    std::weak_ptr<WGPUTimerQuery::Status> statusPtr = status;

    if (auto s = statusPtr.lock()) {
        s->elapsedNanoseconds = std::chrono::steady_clock::now().time_since_epoch().count();
    }
}

void WGPUTimerQuery::endTimeElapsedQuery() {
    std::cout << "    WGPUTimerQuery::endTimeElapsedQuery()" <<std::endl;
    // Capture the timer query status via a weak_ptr because the WGPUTimerQuery could be destroyed
    // before the block executes.
    if (status->elapsedNanoseconds == 0)
    {
        std::cout << "    %%%%%%%%WGPUTimerQuery::endTimeElapsedQuery()" <<std::endl;
    }
    else {
        std::weak_ptr<WGPUTimerQuery::Status> statusPtr = status;
        if (auto s = statusPtr.lock()) {
            s->previousElapsed = s->elapsedNanoseconds =
                    std::chrono::steady_clock::now().time_since_epoch().count() -
                    s->elapsedNanoseconds;
        }
    }
}

bool WGPUTimerQuery::getQueryResult(uint64_t* outElapsedTime) {
    std::cout << "    WGPUTimerQuery::getQueryResult: " << std::endl;
    if (status->previousElapsed == 0) {
        return false;
    }
    if (outElapsedTime) {
        *outElapsedTime = status->previousElapsed;
        std::cout << "        TimeResult: " << status->previousElapsed << std::endl;
        status->previousElapsed = 0;
    }
    return true;
}

}// namespace filament::backend
