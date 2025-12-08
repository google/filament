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

#include <details/Engine.h>

#include <filament/Renderer.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <private/utils/Tracing.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/FixedCapacityVector.h>
#include <utils/JobSystem.h>
#include <utils/Logger.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <memory>
#include <ratio>
#include <utility>

#include <stdint.h>
#include <stddef.h>

namespace filament {

using namespace utils;
using namespace backend;

FrameInfoManager::FrameInfoManager(FEngine& engine, DriverApi& driver) noexcept
    : mJobQueue("FrameInfoGpuComplete", JobSystem::Priority::URGENT_DISPLAY),
      mHasTimerQueries(driver.isFrameTimeSupported()),
      mDisableGpuFrameComplete(engine.features.engine.frame_info.disable_gpu_frame_complete_metric) {
    if (mHasTimerQueries) {
        for (auto& query : mQueries) {
            query.handle = driver.createTimerQuery();
        }
    }
}

FrameInfoManager::~FrameInfoManager() noexcept = default;

void FrameInfoManager::terminate(FEngine& engine) noexcept {
    DriverApi& driver = engine.getDriverApi();

    if (mHasTimerQueries) {
        for (auto const& query : mQueries) {
            driver.destroyTimerQuery(query.handle);
        }
    }

    if (!mDisableGpuFrameComplete) {
        // remove all pending callbacks. This is okay to do because they have no
        // side effect.
        mJobQueue.cancelAll();

        // request cancel for all the fences, which may speed up drainAndExit() below
        for (auto& info : mFrameTimeHistory) {
            if (info.fence) {
                driver.fenceCancel(info.fence);
            }
        }

        // wait for all pending callbacks to be called & terminate the thread
        mJobQueue.drainAndExit();

        // Destroy the fences that are still alive, they will error out.
        for (size_t i = 0, c = mFrameTimeHistory.size(); i < c; i++) {
            auto& info = mFrameTimeHistory[i];
            if (info.fence) {
                driver.destroyFence(std::move(info.fence));
            }
        }
    }
}

void FrameInfoManager::beginFrame(FSwapChain* swapChain, DriverApi& driver,
        Config const& config, uint32_t frameId, std::chrono::steady_clock::time_point const vsync) noexcept {
    auto const now = std::chrono::steady_clock::now();

    auto& history = mFrameTimeHistory;
    // don't exceed the capacity, drop the oldest entry
    if (UTILS_LIKELY(history.size() == history.capacity())) {
        FrameInfoImpl& frameInfo = history.back();
        if (frameInfo.ready.load(std::memory_order_relaxed)) {
            if (!mDisableGpuFrameComplete) {
                assert_invariant(frameInfo.fence);
                driver.destroyFence(std::move(frameInfo.fence));
            }
            history.pop_back();
        } else {
            // This is a big problem, we ran out of space in the circular queue and that entry
            // hasn't been processed yet. Because the code below keeps a reference to the
            // front element of the queue, we can't pop/push. Our only option is to not record
            // a new entry for this frame, which will create a false skipped frame in the
            // data.
            LOG(WARNING) << "FrameInfo's circular queue is full, but the latest item hasn't "
                            " been processed yet. Skipping this frame, id = " << frameId;
        }
    }

    // create a new entry
    FrameInfoImpl& front = history.emplace_front(frameId);

    // store the current time
    front.vsync = vsync;
    front.beginFrame = now;

    // store compositor timings if supported
    CompositorTiming compositorTiming{};
    if (driver.isCompositorTimingSupported() &&
        driver.queryCompositorTiming(swapChain->getHwHandle(), &compositorTiming)) {
        front.presentDeadline = compositorTiming.compositeDeadline;
        front.displayPresentInterval = compositorTiming.compositeInterval;
        front.compositionToPresentLatency = compositorTiming.compositeToPresentLatency;
        front.expectedPresentTime = compositorTiming.expectedPresentTime;
        if (compositorTiming.frameTime != CompositorTiming::INVALID) {
            // of we have a vsync time from the compositor, ignore the one from the user
            front.vsync = FrameInfoImpl::time_point{
                std::chrono::nanoseconds(compositorTiming.frameTime) };
        }
    }

    if (mHasTimerQueries) {
        // references are not invalidated by CircularQueue<>, so we can associate a reference to
        // the slot we created to the timer query used to find the frame time.
        mQueries[mIndex].pInfo = std::addressof(front);
        // issue the timer query
        driver.beginTimerQuery(mQueries[mIndex].handle);
    }

    // issue the custom backend command to get the backend time
    driver.queueCommand([&front](){
        front.backendBeginFrame = std::chrono::steady_clock::now();
    });

    if (mHasTimerQueries) {
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
                    pFront->gpuFrameDuration = std::chrono::duration<uint64_t, std::nano>(elapsed);
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
    } else {
        if (mLast != mIndex) {
            mLast = (mLast + 1) % POOL_COUNT;
        }
    }

#if 0
    // keep this just for debugging
    using namespace utils;
    auto h = getFrameInfoHistory(1); // this can throw
    if (!h.empty()) {
        DLOG(INFO) << frameId << ": " << h[0].frameId << " (" << frameId - h[0].frameId <<
                ")"
                << ", Dm=" << h[0].endFrame - h[0].beginFrame
                << ", L =" << h[0].backendBeginFrame - h[0].beginFrame
                << ", Db=" << h[0].backendEndFrame - h[0].backendBeginFrame
                << ", T =" << h[0].gpuFrameDuration;
    }
#endif
}

void FrameInfoManager::endFrame(DriverApi& driver) noexcept {
    auto& front = mFrameTimeHistory.front();
    front.endFrame = std::chrono::steady_clock::now();

    if (!mDisableGpuFrameComplete) {
        // create a Fence to capture the GPU complete time
        FenceHandle const fence = driver.createFence();
        front.fence = fence;
    }

    if (mHasTimerQueries) {
        // close the timer query
        driver.endTimerQuery(mQueries[mIndex].handle);
    }

    // queue custom backend command to query the current time
    driver.queueCommand([&jobQueue = mJobQueue, &driver, &front,
            disableGpuFrameComplete = mDisableGpuFrameComplete] {
        // backend frame end-time
        front.backendEndFrame = std::chrono::steady_clock::now();

        if (UTILS_UNLIKELY(disableGpuFrameComplete || !jobQueue.isValid())) {
            front.gpuFrameComplete = {};
            front.ready.store(true, std::memory_order_release);
            return;
        }

        if (!disableGpuFrameComplete) {
            // now launch a job that'll wait for the gpu to complete
            jobQueue.push([&driver, &front] {
                FenceStatus const status = driver.fenceWait(front.fence, FENCE_WAIT_FOR_EVER);
                if (status == FenceStatus::CONDITION_SATISFIED) {
                    front.gpuFrameComplete = std::chrono::steady_clock::now();
                } else if (status == FenceStatus::TIMEOUT_EXPIRED) {
                    // that should never happen because:
                    // - we wait forever
                    // - made sure that the createFence() command was processed on the backed
                    //   (because we're inside a custom command)
                } else {
                    // We got an error, fenceWait might not be supported
                    front.gpuFrameComplete = {};
                }
                // finally, signal that the data is available
                front.ready.store(true, std::memory_order_release);
            });
        }
    });

    mIndex = (mIndex + 1) % POOL_COUNT;
}

void FrameInfoManager::denoiseFrameTime(FrameHistoryQueue& history, Config const& config) noexcept {
    assert_invariant(!history.empty());

    // find the first slot that has a valid frame duration
    size_t first = history.size();
    for (size_t i = 0, c = history.size(); i < c; ++i) {
        if (history[i].gpuFrameDuration != duration(0)) {
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
            median[i] = history[first + i].gpuFrameDuration;
        }
        std::sort(median.begin(), median.begin() + size);
        duration const denoisedFrameTime = median[size / 2];

        history[first].denoisedFrameTime = denoisedFrameTime;
        history[first].valid = true;
     }
}

void FrameInfoManager::updateUserHistory(FSwapChain* swapChain, DriverApi& driver) {

    if (!swapChain) {
        swapChain = mLastSeenSwapChain;
    } else {
        mLastSeenSwapChain = swapChain;
    }

    auto result = FixedCapacityVector<Renderer::FrameInfo>::with_capacity(MAX_FRAMETIME_HISTORY);
    auto& history = mFrameTimeHistory;
    size_t i = 0;
    size_t const c = history.size();
    for (; i < c; ++i) {
        auto const& entry = history[i];
        if (entry.ready.load(std::memory_order_acquire) && (entry.valid || !mHasTimerQueries)) {
            // once we found an entry ready,
            // we know by construction that all following ones are too
            break;
        }
    }
    size_t historySize = MAX_FRAMETIME_HISTORY;
    for (; i < c && historySize; ++i, --historySize) {
        auto& entry = history[i];

        // retrieve the displayPresentTime only we don't already have it
        if (entry.displayPresent == Renderer::FrameInfo::PENDING) {
            FrameTimestamps frameTimestamps{
                .displayPresentTime = FrameTimestamps::INVALID
            };
            if (swapChain && driver.isCompositorTimingSupported()) {
                // queryFrameTimestamps could fail if this frameid is no longer available
                bool const success = driver.queryFrameTimestamps(swapChain->getHwHandle(),
                        entry.frameId, &frameTimestamps);
                if (success) {
                    assert_invariant(entry.displayPresent < 0 ||
                            entry.displayPresent == frameTimestamps.displayPresentTime);
                    entry.displayPresent = frameTimestamps.displayPresentTime;
                }
            } else {
                entry.displayPresent = Renderer::FrameInfo::INVALID;
            }
        }

        using namespace std::chrono;
        // can't throw by construction

        auto toDuration = [](details::FrameInfo::duration const d) {
            return duration_cast<nanoseconds>(d).count();
        };

        auto toTimepoint = [](FrameInfoImpl::time_point const tp) {
            return duration_cast<nanoseconds>(tp.time_since_epoch()).count();
        };

        result.push_back({
                .frameId                        = entry.frameId,
                .gpuFrameDuration               = toDuration(entry.gpuFrameDuration),
                .denoisedGpuFrameDuration       = toDuration(entry.denoisedFrameTime),
                .beginFrame                     = toTimepoint(entry.beginFrame),
                .endFrame                       = toTimepoint(entry.endFrame),
                .backendBeginFrame              = toTimepoint(entry.backendBeginFrame),
                .backendEndFrame                = toTimepoint(entry.backendEndFrame),
                .gpuFrameComplete               = toTimepoint(entry.gpuFrameComplete),
                .vsync                          = toTimepoint(entry.vsync),
                .displayPresent                 = entry.displayPresent,
                .presentDeadline                = entry.presentDeadline,
                .displayPresentInterval         = entry.displayPresentInterval,
                .compositionToPresentLatency    = entry.compositionToPresentLatency,
                .expectedPresentTime            = entry.expectedPresentTime,

        });
    }
    std::swap(mUserFrameHistory, result);
}

FixedCapacityVector<Renderer::FrameInfo> FrameInfoManager::getFrameInfoHistory(
        size_t const historySize) const {
    auto result = mUserFrameHistory;
    if (result.size() >= historySize) {
        result.resize(historySize);
    }
    return result;
}

} // namespace filament
