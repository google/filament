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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUTIMERQUERY_H
#define TNT_FILAMENT_BACKEND_WEBGPUTIMERQUERY_H

#include "DriverBase.h"

#include <webgpu/webgpu_cpp.h>

#include <atomic>
#include <cstdint>
#include <memory>

namespace filament::backend {

class WebGPUTimerQuery : public HwTimerQuery {
public:
    WebGPUTimerQuery()
        : mStatus(std::make_shared<Status>()) {}

    void beginTimeElapsedQuery();
    void endTimeElapsedQuery();
    bool getQueryResult(uint64_t* outElapsedTimeNanoseconds);

private:
    struct Status {
        std::atomic<uint64_t> elapsedNanoseconds{ 0 };
        std::atomic<uint64_t> previousElapsed{ 0 };
    };

    std::shared_ptr<Status> mStatus;
};

} // namespace filament::backend

#endif //TNT_FILAMENT_BACKEND_WEBGPUTIMERQUERY_H
