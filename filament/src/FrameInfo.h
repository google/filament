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

#ifndef TNT_FILAMENT_FRAMEINFO_H
#define TNT_FILAMENT_FRAMEINFO_H

#include "details/Engine.h"

#include <filament/Fence.h>

#include <utils/Allocator.h>

#include <deque>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include <assert.h>
#include <math/fast.h>

// set EXTRA_TIMING_INFO to enable and print extra timing info about the render loop
#define EXTRA_TIMING_INFO  false

namespace filament {

using namespace details;

class FrameInfoManager;

class FrameInfo {
public:
    friend class FrameInfoManager;
    using clock = std::chrono::steady_clock;
    using time_point = clock::time_point;
    using duration = std::chrono::duration<float, std::milli>;

    enum lap_id {
        START = 0,      // don't use for FrameInfo::lap()
        FINISH = 1,     // don't use for FrameInfo::lap()
        LAP_0,
        LAP_1,
        LAP_2,
        LAP_3,
        LAP_4,
        LAP_5,
    };

    void beginFrame(FrameInfoManager* mgr);
    void lap(FrameInfoManager* mgr, lap_id id);
    void endFrame(FrameInfoManager* mgr);

    static constexpr size_t MAX_LAPS_IDS = 8;

    uint32_t frame = 0;
    time_point laps[MAX_LAPS_IDS] = { time_point::max() };
};

class FrameInfoManager {
    friend class FrameInfo;
    static constexpr size_t HISTORY_COUNT = 5;
    static constexpr size_t POOL_COUNT = 8;

    // set this to true to enable extra timing info
    static constexpr bool mLapRecordsEnabled = EXTRA_TIMING_INFO;

public:
    using clock = FrameInfo::clock;
    using time_point = FrameInfo::time_point;
    using duration = FrameInfo::duration;

    explicit FrameInfoManager(FEngine& engine);
    ~FrameInfoManager() noexcept;

    FEngine& getEngine() { return mEngine; }

    void run() {
        mSyncThread.run();
    }

    void terminate() {
        mSyncThread.requestExitAndWait();
    }

    // call this immediately after "make current"
    void beginFrame(uint32_t frameId);

    // call this between beginFrame and endFrame to record a time
    void lap(FrameInfo::lap_id id) {
        if (mLapRecordsEnabled) {
            FrameInfo* const info = mCurrentFrameInfo;
            assert(info);
            info->lap(this, id);
        }
    }

    // call this immediately before "swap buffers"
    void endFrame();

    void cancelFrame();

    constexpr bool isLapRecordsEnabled() const noexcept {
        return mLapRecordsEnabled;
    }

    duration getLastFrameTime() const noexcept {
        std::unique_lock<std::mutex> lock(mLock);
        FrameInfo const& info = mFrameInfoHistory.front();
        return info.laps[FrameInfo::FINISH] - info.laps[FrameInfo::START];
    }

    std::vector<FrameInfo> getHistory() const noexcept {
        std::unique_lock<std::mutex> lock(mLock);
        return mFrameInfoHistory;
    }

    // no user serviceable part below...

    template<typename CALLABLE, typename ... ARGS>
    void push(CALLABLE&& func, ARGS&&... args) {
        mSyncThread.push(std::forward<CALLABLE>(func), std::forward<ARGS>(args)...);
    }

    static constexpr size_t getHistorySize() noexcept {
        return HISTORY_COUNT;
    }

private:

    class SyncThread {
    public:
        using Job = std::function<void()>;

        SyncThread() = default;
        ~SyncThread();

        void run();
        void requestExitAndWait();

        template<typename CALLABLE, typename ... ARGS>
        void push(CALLABLE&& func, ARGS&&... args) {
            enqueue(Job(std::bind(std::forward<CALLABLE>(func), std::forward<ARGS>(args)...)));
        }

    private:
        void enqueue(SyncThread::Job&& job);
        void loop();
        std::thread mThread;
        mutable std::mutex mLock;
        mutable std::condition_variable mCondition;
        std::deque<Job> mQueue;
        bool mExitRequested = false;
    };

    FrameInfo* obtain() noexcept;
    void finish(FrameInfo* info) noexcept;

    using PoolArena = utils::Arena<utils::ObjectPoolAllocator<FrameInfo>, utils::LockingPolicy::SpinLock>;
    FEngine& mEngine;
    PoolArena mPoolArena;
    SyncThread mSyncThread;
    FrameInfo* mCurrentFrameInfo = nullptr;

    mutable std::mutex mLock;
    std::vector<FrameInfo> mFrameInfoHistory;
};


template<typename T, size_t MEDIAN = 5, size_t HISTORY = 16>
class Series {
public:
    Series() {
        mIn.resize(MEDIAN);
        mOut.resize(HISTORY);
    }

    void push(T value, float b = 1.0f - math::fast::exp(-0.125f)) noexcept {
        mIn.push_back(value);
        mIn.pop_front();
        std::array<T, MEDIAN> median;
        std::copy_n(mIn.begin(), median.size(), median.begin());
        std::sort(median.begin(), median.end());
        mLowPass += b * (median[median.size() / 2] - mLowPass);
        mOut.push_back(mLowPass);
        mOut.pop_front();
    }

    T operator[](size_t i) const noexcept { return mOut[i]; }
    typename std::deque<T>::iterator begin() const { return mOut.begin(); }
    typename std::deque<T>::iterator end() const { return mOut.end(); }
    T const& oldest() const noexcept { return mOut.front(); }
    T const& latest() const noexcept { return mOut.back(); }

    std::deque<T> mIn;
    std::deque<T> mOut;
    T mLowPass = {};
};


} // namespace filament

#endif // TNT_FILAMENT_FRAMEINFO_H
