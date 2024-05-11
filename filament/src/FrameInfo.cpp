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
#include <utils/Systrace.h>

#include <math/scalar.h>

#include <cmath>

namespace filament {

using namespace utils;
using namespace backend;

// this is to avoid a call to memmove
template<class InputIterator, class OutputIterator>
static inline
void move_backward(InputIterator first, InputIterator last, OutputIterator result) {
    while (first != last) {
        *--result = *--last;
    }
}

FrameInfoManager::FrameInfoManager(DriverApi& driver) noexcept {
    for (auto& query : mQueries) {
        query = driver.createTimerQuery();
    }
}

FrameInfoManager::~FrameInfoManager() noexcept = default;

void FrameInfoManager::terminate(DriverApi& driver) noexcept {
    for (auto& query : mQueries) {
        driver.destroyTimerQuery(query);
    }
}

void FrameInfoManager::beginFrame(DriverApi& driver,Config const& config, uint32_t) noexcept {
    driver.beginTimerQuery(mQueries[mIndex]);
    uint64_t elapsed = 0;
    TimerQueryResult const result = driver.getTimerQueryValue(mQueries[mLast], &elapsed);
    switch (result) {
        case TimerQueryResult::NOT_READY:
            // nothing to do
            break;
        case TimerQueryResult::ERROR:
            mLast = (mLast + 1) % POOL_COUNT;
            break;
        case TimerQueryResult::AVAILABLE:
            mLast = (mLast + 1) % POOL_COUNT;
            // conversion to our duration happens here
            mFrameTime = std::chrono::duration<uint64_t, std::nano>(elapsed);
            break;
    }
    update(config, mFrameTime);
}

void FrameInfoManager::endFrame(DriverApi& driver) noexcept {
    driver.endTimerQuery(mQueries[mIndex]);
    mIndex = (mIndex + 1) % POOL_COUNT;
}

void FrameInfoManager::update(Config const& config,
        FrameInfoManager::duration lastFrameTime) noexcept {
    // keep an history of frame times
    auto& history = mFrameTimeHistory;

    // this is like doing { pop_back(); push_front(); }
    filament::move_backward(history.begin(), history.end() - 1, history.end());
    history[0].frameTime = lastFrameTime;

    mFrameTimeHistorySize = std::min(++mFrameTimeHistorySize, uint32_t(MAX_FRAMETIME_HISTORY));
    if (UTILS_UNLIKELY(mFrameTimeHistorySize < 3)) {
        // not enough history to do anything useful
        history[0].valid = false;
        return;
    }

    // apply a median filter to get a good representation of the frame time of the last
    // N frames.
    std::array<duration, MAX_FRAMETIME_HISTORY> median; // NOLINT -- it's initialized below
    size_t const size = std::min(mFrameTimeHistorySize,
            std::min(config.historySize, (uint32_t)median.size()));
    for (size_t i = 0; i < size; ++i) {
        median[i] = history[i].frameTime;
    }
    std::sort(median.begin(), median.begin() + size);
    duration const denoisedFrameTime = median[size / 2];

    history[0].denoisedFrameTime = denoisedFrameTime;
    history[0].valid = true;
}


} // namespace filament
