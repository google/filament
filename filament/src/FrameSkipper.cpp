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

#include <utils/Log.h>
#include "details/FrameSkipper.h"
#include "details/Engine.h"

namespace filament {

using namespace utils;
using namespace backend;

FrameSkipper::FrameSkipper(FEngine& engine, size_t latency) noexcept
        : mEngine(engine), mLast(latency) {
    assert(latency <= MAX_FRAME_LATENCY);
}

FrameSkipper::~FrameSkipper() noexcept {
    auto& driver = mEngine.getDriverApi();
    for (auto sync : mDelayedSyncs) {
        if (sync) {
            driver.destroySync(sync);
        }
    }
}

bool FrameSkipper::beginFrame() noexcept {
    auto& driver = mEngine.getDriverApi();
    auto& syncs = mDelayedSyncs;
    auto sync = syncs.front();
    if (sync) {
        auto status = driver.getSyncStatus(sync);
        if (status == SyncStatus::NOT_SIGNALED) {
            // Sync not ready, skip frame
            return false;
        }
        driver.destroySync(sync);
    }
    // shift all fences down by 1
    std::move(syncs.begin() + 1, syncs.end(), syncs.begin());
    syncs.back() = {};
    return true;
}

void FrameSkipper::endFrame() noexcept {
    // if the user produced a new frame despite the fact that the previous one wasn't finished
    // (i.e. FrameSkipper::beginFrame() returned false), we need to make sure to replace
    // a fence that might be here already)
    auto& driver = mEngine.getDriverApi();
    auto& sync = mDelayedSyncs[mLast];
    if (sync) {
        driver.destroySync(sync);
    }
    sync = driver.createSync();
}

} // namespace filament
