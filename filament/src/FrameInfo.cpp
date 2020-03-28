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

#include "FrameInfo.h"

#include <utils/Log.h>

#include <math/scalar.h>

#include <cmath>

namespace filament {
using namespace utils;
using namespace details;

FrameInfoManager::FrameInfoManager(FEngine& engine) : mEngine(engine) {
    backend::DriverApi& driver = mEngine.getDriverApi();
    for (auto& query : mQueries) {
        query = driver.createTimerQuery();
    }
}

FrameInfoManager::~FrameInfoManager() noexcept = default;

void FrameInfoManager::terminate() {
    backend::DriverApi& driver = mEngine.getDriverApi();
    for (auto& query : mQueries) {
        driver.destroyTimerQuery(query);
    }
}

void FrameInfoManager::beginFrame(uint32_t frameId) {
    backend::DriverApi& driver = mEngine.getDriverApi();
    driver.beginTimerQuery(mQueries[mIndex]);
    uint64_t elapsed = 0;
    if (driver.getTimerQueryValue(mQueries[mLast], &elapsed)) {
        mLast = (mLast + 1) % POOL_COUNT;
        mFrameTime = std::chrono::duration<uint64_t, std::nano>(elapsed);
    }
}

void FrameInfoManager::endFrame() {
    backend::DriverApi& driver = mEngine.getDriverApi();
    driver.endTimerQuery(mQueries[mIndex]);
    mIndex = (mIndex + 1) % POOL_COUNT;
}


} // namespace filament
