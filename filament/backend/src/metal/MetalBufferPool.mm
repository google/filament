/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "MetalBufferPool.h"

#include "MetalContext.h"

#include <utils/Panic.h>
#include <utils/trap.h>

#include <thread>
#include <chrono>

namespace filament {
namespace backend {
namespace metal {

MetalBufferPoolEntry const* MetalBufferPool::acquireBuffer(size_t numBytes) {
    std::lock_guard<std::mutex> lock(mMutex);

    // First check if a stage exists whose capacity is greater than or equal to the requested size.
    auto iter = mFreeStages.lower_bound(numBytes);
    if (iter != mFreeStages.end()) {
        auto stage = iter->second;
        mFreeStages.erase(iter);
        mUsedStages.insert(stage);
        stage->referenceCount = 1;
        return stage;
    }

    // We were not able to find a sufficiently large stage, so create a new one.
    id<MTLBuffer> buffer = [mContext.device newBufferWithLength:numBytes
                                                        options:MTLResourceStorageModeShared];
    MetalBufferPoolEntry* stage = new MetalBufferPoolEntry({
        .buffer = buffer,
        .capacity = numBytes,
        .lastAccessed = mCurrentFrame,
        .referenceCount = 1
    });
    mUsedStages.insert(stage);

    return stage;
}

void MetalBufferPool::retainBuffer(MetalBufferPoolEntry const *stage) noexcept {
    std::lock_guard<std::mutex> lock(mMutex);

    (stage->referenceCount)++;
}

void MetalBufferPool::releaseBuffer(MetalBufferPoolEntry const *stage) noexcept {
    std::lock_guard<std::mutex> lock(mMutex);

    // Decrement the ref count. If it is at 0, move the buffer entry to the free list.
    if (--(stage->referenceCount) > 0) {
        return;
    }

    auto iter = mUsedStages.find(stage);
    if (iter == mUsedStages.end()) {
        utils::slog.e << "Unknown Metal buffer: " << stage->capacity << " bytes"
                << utils::io::endl;
        return;
    }
    stage->lastAccessed = mCurrentFrame;
    mUsedStages.erase(iter);
    mFreeStages.insert(std::make_pair(stage->capacity, stage));
}

void MetalBufferPool::gc() noexcept {
    std::lock_guard<std::mutex> lock(mMutex);

    mCurrentFrame++;
    decltype(mFreeStages) stages;
    stages.swap(mFreeStages);
    const uint64_t evictionTime = mCurrentFrame - TIME_BEFORE_EVICTION;
    for (auto pair : stages) {
        if (pair.second->lastAccessed < evictionTime) {
            [pair.second->buffer release];
            delete pair.second;
        } else {
            mFreeStages.insert(pair);
        }
    }
}

void MetalBufferPool::reset() noexcept {
    std::lock_guard<std::mutex> lock(mMutex);

    assert(mUsedStages.empty());
    for (auto pair : mFreeStages) {
        [pair.second->buffer release];
        delete pair.second;
    }
    mFreeStages.clear();
}

} // namespace metal
} // namespace backend
} // namespace filament
