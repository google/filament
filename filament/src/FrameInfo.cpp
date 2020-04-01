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

namespace details {
// this is to avoid a call to memmove
template<class InputIterator, class OutputIterator>
static inline
void move_backward(InputIterator first, InputIterator last, OutputIterator result) {
    while (first != last) {
        *--result = *--last;
    }
}
} // namespace details

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

void FrameInfoManager::beginFrame(Config const& config, uint32_t frameId) {
    backend::DriverApi& driver = mEngine.getDriverApi();
    driver.beginTimerQuery(mQueries[mIndex]);
    uint64_t elapsed = 0;
    if (driver.getTimerQueryValue(mQueries[mLast], &elapsed)) {
        mLast = (mLast + 1) % POOL_COUNT;
        // convertion to our duration happens here
        mFrameTime = std::chrono::duration<uint64_t, std::nano>(elapsed);
    }
    update(config,mFrameTime);
}

void FrameInfoManager::endFrame() {
    backend::DriverApi& driver = mEngine.getDriverApi();
    driver.endTimerQuery(mQueries[mIndex]);
    mIndex = (mIndex + 1) % POOL_COUNT;
}

void FrameInfoManager::update(Config const& config, FrameInfoManager::duration lastFrameTime) {
    const float kFeedbackConstant = (1.0f - std::exp(-config.oneOverTau));

    // keep an history of frame times
    auto& history = mFrameTimeHistory;

    // this is like doing { pop_back(); push_front(); }
    details::move_backward(history.begin(), history.end() - 1, history.end());
    history[0].frameTime = lastFrameTime;

    mFrameTimeHistorySize = std::min(++mFrameTimeHistorySize, size_t(MAX_FRAMETIME_HISTORY));
    if (UTILS_UNLIKELY(mFrameTimeHistorySize < 3)) {
        // not enough history to do anything usefull
        history[0].valid = false;
        return;
    }

    // apply a median filter to get a good representation of the frame time of the last
    // N frames.
    std::array<duration, MAX_FRAMETIME_HISTORY> median; // NOLINT -- it's initialized below
    size_t size = std::min(mFrameTimeHistorySize, std::min(config.historySize, median.size()));
    for (size_t i = 0; i < size; ++i) {
        median[i] = history[i].frameTime;
    }
    std::sort(median.begin(), median.begin() + size);
    duration denoisedFrameTime = median[size / 2];

    history[0].denoisedFrameTime = denoisedFrameTime;

    // how much we need to scale the current workload to fit in our target, at this instant
    const float targetWithHeadroom = config.targetFrameTime * (1.0f - config.headRoomRatio);
    const float workload = denoisedFrameTime.count() / targetWithHeadroom;
    history[0].workLoad = workload;
    history[0].smoothedWorkLoad = history[1].smoothedWorkLoad +
            kFeedbackConstant * (workload - history[1].smoothedWorkLoad);
    history[0].valid = true;

//    slog.d  << history[0].frameTime.count() << ", "
//            << history[0].denoisedFrameTime.count() << ", "
//            << history[0].workLoad << ", "
//            << history[0].smoothedWorkLoad << io::endl;
}


} // namespace filament
