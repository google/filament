/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include "MetalTimerQuery.h"

#include "MetalContext.h"
#include "MetalHandles.h"

namespace filament {
namespace backend {

void MetalTimerQueryImpl::beginTimeElapsedQuery(MetalTimerQuery* query) {
    query->status->startNs.store(0);
    query->status->elapsedNs.store(0);
    query->status->available.store(false);

    // Capture the timer query status via a weak_ptr because the MetalTimerQuery could be destroyed
    // before the block executes.
    std::weak_ptr<MetalTimerQuery::Status> status = query->status;
    id<MTLCommandBuffer> commandBuffer = getPendingCommandBuffer(&mContext);
    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
        if (auto s = status.lock()) {
            s->startNs.store((uint64_t) (buffer.GPUStartTime * 1000000000.0));
        }
    }];
}

void MetalTimerQueryImpl::endTimeElapsedQuery(MetalTimerQuery* query) {
    // Capture the timer query status via a weak_ptr because the MetalTimerQuery could be destroyed
    // before the block executes.
    std::weak_ptr<MetalTimerQuery::Status> status = query->status;
    id<MTLCommandBuffer> commandBuffer = getPendingCommandBuffer(&mContext);
    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
        if (auto s = status.lock()) {
            const uint64_t end = (uint64_t) (buffer.GPUEndTime * 1000000000.0);
            const uint64_t begin = s->startNs.load();
            // A driver that does not fill in the timestamps leaves this at 0, which
            // FrameInfoManager already reads as "no measurement" rather than as zero work.
            s->elapsedNs.store(end > begin ? end - begin : 0);
            s->available.store(true);
        }
    }];
}

bool MetalTimerQueryImpl::getQueryResult(MetalTimerQuery* query, uint64_t* outElapsedTime) {
    if (!query->status->available.load()) {
        return false;
    }
    if (outElapsedTime) {
        *outElapsedTime = query->status->elapsedNs;
    }
    return true;
}

} // namespace backend
} // namespace filament
