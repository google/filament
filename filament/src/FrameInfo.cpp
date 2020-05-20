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

// this is to avoid a call to memmove
template<class InputIterator, class OutputIterator>
static inline
void move_backward(InputIterator first, InputIterator last, OutputIterator result) {
    while (first != last) {
        *--result = *--last;
    }
}

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
    // keep an history of frame times
    auto& history = mFrameTimeHistory;

    // this is like doing { pop_back(); push_front(); }
    filament::move_backward(history.begin(), history.end() - 1, history.end());
    history[0].frameTime = lastFrameTime;

    mFrameTimeHistorySize = std::min(++mFrameTimeHistorySize, uint32_t(MAX_FRAMETIME_HISTORY));
    if (UTILS_UNLIKELY(mFrameTimeHistorySize < 3)) {
        // not enough history to do anything usefull
        history[0].valid = false;
        return;
    }

    // apply a median filter to get a good representation of the frame time of the last
    // N frames.
    std::array<duration, MAX_FRAMETIME_HISTORY> median; // NOLINT -- it's initialized below
    size_t size = std::min(mFrameTimeHistorySize, std::min(config.historySize, (uint32_t)median.size()));
    for (size_t i = 0; i < size; ++i) {
        median[i] = history[i].frameTime;
    }
    std::sort(median.begin(), median.begin() + size);
    duration denoisedFrameTime = median[size / 2];

    history[0].denoisedFrameTime = denoisedFrameTime;

    // how much we need to scale the current workload to fit in our target, at this instant
    const duration targetWithHeadroom = config.targetFrameTime * (1.0f - config.headRoomRatio);
    const duration measured = denoisedFrameTime;

    // We use a P.I.D. controller below to figure out the scaling factor to apply. In practice we
    // don't use the Derivative gain (so it's really a PI controller).
    const float Kp = (1.0f - std::exp(-config.oneOverTau));
    const float Ki = Kp / 10.0f;
    const float Kd = 0.0;

    history[0].pid.error = (targetWithHeadroom - measured) / targetWithHeadroom;
    history[0].pid.integral = history[1].pid.integral + Ki * history[0].pid.error;
    history[0].pid.integral = math::clamp(history[0].pid.integral, -6.0f, 2.0f);

    const float derivative = Kd * (history[0].pid.error - history[1].pid.error);
    const float out = Kp * history[0].pid.error + history[0].pid.integral + derivative;

    // maps the command to a ratio, it really doesn't mater much how the conversion is done
    // the system will find the right value automatically
    const float scale = std::exp2(out);
    history[0].scale = scale;
    history[0].valid = true;

    SYSTRACE_CONTEXT();
    SYSTRACE_VALUE32("info_e", history[0].pid.error * 100);
    SYSTRACE_VALUE32("info_s", scale * 100);
//    slog.d << history[0].pid.error * 100 << "%, " << scale << io::endl;
}


} // namespace filament
