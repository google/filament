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

#include <filament/Renderer.h>

#include <backend/DriverEnums.h>

#include <private/utils/Tracing.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Log.h>
#include <utils/ostream.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <memory>
#include <ratio>

#include <stdint.h>
#include <stddef.h>

namespace filament {

using namespace utils;
using namespace backend;

FrameInfoManager::FrameInfoManager(DriverApi& driver) noexcept {
    for (auto& query : mQueries) {
        query.handle = driver.createTimerQuery();
    }
}

FrameInfoManager::~FrameInfoManager() noexcept = default;

void FrameInfoManager::terminate(DriverApi& driver) noexcept {
    for (auto& query : mQueries) {
        driver.destroyTimerQuery(query.handle);
    }
}

void FrameInfoManager::beginFrame(DriverApi& driver, Config const& config, uint32_t frameId) noexcept {
    auto& history = mFrameTimeHistory;
    // don't exceed the capacity, drop the oldest entry
    if (UTILS_LIKELY(history.size() == history.capacity())) {
        history.pop_back();
    }

    // create a new entry
    auto& front = history.emplace_front(frameId);

    // store the current time
    front.beginFrame = std::chrono::steady_clock::now();

    // references are not invalidated by CircularQueue<>, so we can associate a reference to
    // the slot we created to the timer query used to find the frame time.
    mQueries[mIndex].pInfo = std::addressof(front);
    // issue the timer query
    driver.beginTimerQuery(mQueries[mIndex].handle);
    // issue the custom backend command to get the backend time
    driver.queueCommand([&front](){
        front.backendBeginFrame = std::chrono::steady_clock::now();
    });

    // now is a good time to check the oldest active query
    while (mLast != mIndex) {
        uint64_t elapsed = 0;
        TimerQueryResult const result = driver.getTimerQueryValue(mQueries[mLast].handle, &elapsed);
        switch (result) {
            case TimerQueryResult::NOT_READY:
                // nothing to do
                break;
            case TimerQueryResult::ERROR:
                mLast = (mLast + 1) % POOL_COUNT;
                break;
            case TimerQueryResult::AVAILABLE: {
                FILAMENT_TRACING_CONTEXT(FILAMENT_TRACING_CATEGORY_FILAMENT);
                FILAMENT_TRACING_VALUE(FILAMENT_TRACING_CATEGORY_FILAMENT, "FrameInfo::elapsed", uint32_t(elapsed));
                // conversion to our duration happens here
                pFront = mQueries[mLast].pInfo;
                pFront->frameTime = std::chrono::duration<uint64_t, std::nano>(elapsed);
                mLast = (mLast + 1) % POOL_COUNT;
                denoiseFrameTime(history, config);
                break;
            }
        }
        if (result != TimerQueryResult::AVAILABLE) {
            break;
        }
        // read the pending timer queries until we find one that's not ready
    }

    // keep this just for debugging
    if constexpr (false) {
        using namespace utils;
        auto h = getFrameInfoHistory(1);
        if (!h.empty()) {
            slog.d << frameId << ": "
                   << h[0].frameId << " (" << frameId - h[0].frameId << ")"
                   << ", Dm=" << h[0].endFrame - h[0].beginFrame
                   << ", L =" << h[0].backendBeginFrame - h[0].beginFrame
                   << ", Db=" << h[0].backendEndFrame - h[0].backendBeginFrame
                   << ", T =" << h[0].frameTime
                   << io::endl;
        }
    }
}

void FrameInfoManager::endFrame(DriverApi& driver) noexcept {
    auto& front = mFrameTimeHistory.front();
    // close the timer query
    driver.endTimerQuery(mQueries[mIndex].handle);
    // queue custom backend command to query the current time
    driver.queueCommand([&front](){
        // backend frame end-time
        front.backendEndFrame = std::chrono::steady_clock::now();
        // signal that the data is available
        front.ready.store(true, std::memory_order_release);
    });
    // and finally acquire the time on the main thread
    front.endFrame = std::chrono::steady_clock::now();
    mIndex = (mIndex + 1) % POOL_COUNT;
}

void FrameInfoManager::denoiseFrameTime(FrameHistoryQueue& history, Config const& config) noexcept {
    assert_invariant(!history.empty());

    // find the first slot that has a valid frame duration
    size_t first = history.size();
    for (size_t i = 0, c = history.size(); i < c; ++i) {
        if (history[i].frameTime != duration(0)) {
            first = i;
            break;
        }
    }
    assert_invariant(first != history.size());

    // we need at least 3 valid frame time to calculate the median
    if (history.size() >= first + 3) {
        // apply a median filter to get a good representation of the frame time of the last
        // N frames.
        std::array<duration, MAX_FRAMETIME_HISTORY> median; // NOLINT -- it's initialized below
        size_t const size = std::min({
            history.size() - first,
            median.size(),
            size_t(config.historySize) });

        for (size_t i = 0; i < size; ++i) {
            median[i] = history[first + i].frameTime;
        }
        std::sort(median.begin(), median.begin() + size);
        duration const denoisedFrameTime = median[size / 2];

        history[first].denoisedFrameTime = denoisedFrameTime;
        history[first].valid = true;
     }
}

FixedCapacityVector<Renderer::FrameInfo> FrameInfoManager::getFrameInfoHistory(
        size_t historySize) const noexcept {
    auto result = FixedCapacityVector<Renderer::FrameInfo>::with_capacity(MAX_FRAMETIME_HISTORY);
    auto const& history = mFrameTimeHistory;
    size_t i = 0;
    size_t const c = history.size();
    for (; i < c; ++i) {
        auto const& entry = history[i];
        if (entry.ready.load(std::memory_order_acquire) && entry.valid) {
            // once we found an entry ready,
            // we know by construction that all following ones are too
            break;
        }
    }
    for (; i < c && historySize; ++i, --historySize) {
        auto const& entry = history[i];
        using namespace std::chrono;
        result.push_back({
                entry.frameId,
                duration_cast<nanoseconds>(entry.frameTime).count(),
                duration_cast<nanoseconds>(entry.denoisedFrameTime).count(),
                duration_cast<nanoseconds>(entry.beginFrame.time_since_epoch()).count(),
                duration_cast<nanoseconds>(entry.endFrame.time_since_epoch()).count(),
                duration_cast<nanoseconds>(entry.backendBeginFrame.time_since_epoch()).count(),
                duration_cast<nanoseconds>(entry.backendEndFrame.time_since_epoch()).count()
        });
    }
    return result;
}

} // namespace filament
