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

#include "details/Fence.h"

#include <utils/JobSystem.h>
#include <utils/Log.h>
#include <utils/Systrace.h>

#include <math/scalar.h>

#include <cmath>

namespace filament {
using namespace utils;

// ------------------------------------------------------------------------------------------------

FrameInfoManager::FrameInfoManager(FEngine& engine)
        : mEngine(engine),
          mPoolArena("FrameInfo", sizeof(FrameInfo) * POOL_COUNT) {
    mFrameInfoHistory.resize(HISTORY_COUNT);
}

FrameInfoManager::~FrameInfoManager() noexcept = default;

void FrameInfo::beginFrame(FrameInfoManager* mgr) {
    Fence* fence = mgr->getEngine().createFence(FFence::Type::HARD);
    mgr->push([this, fence]() {
        Fence::waitAndDestroy(fence, Fence::Mode::DONT_FLUSH);
        laps[START] = clock::now();
    });
}

void FrameInfo::lap(FrameInfoManager* mgr, lap_id id) {
    Fence* fence = mgr->getEngine().createFence(FFence::Type::HARD);
    mgr->push([this, fence, id]() {
        Fence::waitAndDestroy(fence, Fence::Mode::DONT_FLUSH);
        laps[id] = clock::now();
    });
}

void FrameInfo::endFrame(FrameInfoManager* mgr) {
    Fence* fence = mgr->getEngine().createFence(FFence::Type::HARD);
    mgr->push([this, mgr, fence]() {
        char buf[256];
        snprintf(buf, 256, "GPU time [id=%u]", frame);
        SYSTRACE_NAME(buf);

        Fence::waitAndDestroy(fence, Fence::Mode::DONT_FLUSH);
        laps[FINISH] = clock::now();
        mgr->finish(this);
    });
}

// ------------------------------------------------------------------------------------------------

void FrameInfoManager::beginFrame(uint32_t frameId) {
    SYSTRACE_CONTEXT();
    SYSTRACE_ASYNC_BEGIN("frame latency", frameId);

    FrameInfo* info = obtain();
    mCurrentFrameInfo = info;
    if (info) {
        info->frame = frameId;
        info->beginFrame(this);
    }
}

void FrameInfoManager::endFrame() {
    FrameInfo* const info = mCurrentFrameInfo;
    if (info) {
        mCurrentFrameInfo = nullptr;
        info->endFrame(this);
    }
}

void FrameInfoManager::cancelFrame() {
    FrameInfo* info = mCurrentFrameInfo;
    if (info) {
        mCurrentFrameInfo = nullptr;
        push([this, info]() {
            mPoolArena.free(info);
        });
    }
}

UTILS_ALWAYS_INLINE
inline FrameInfo* FrameInfoManager::obtain() noexcept {
    return mPoolArena.alloc<FrameInfo>(1);
}

void FrameInfoManager::finish(FrameInfo* info) noexcept {
    SYSTRACE_CONTEXT();
    SYSTRACE_ASYNC_END("frame latency", info->frame);

    // store the new frame info into the history array
    std::unique_lock<std::mutex> lock(mLock);
    auto& history = mFrameInfoHistory;
    if (history.size() >= HISTORY_COUNT) {
        // if the history has grown enough, remove the oldest element
        history.erase(history.begin());
    }
    // add a copy of the new element to the history
    history.push_back(*info);
    lock.unlock();

    // return the item to the pool without the lock held
    mPoolArena.free(info);
}

// ------------------------------------------------------------------------------------------------

FrameInfoManager::SyncThread::~SyncThread() {
    if (mThread.joinable()) {
        requestExitAndWait();
    }
}

void FrameInfoManager::SyncThread::run() {
    mThread = std::thread(&SyncThread::loop, this);
}

void FrameInfoManager::SyncThread::requestExitAndWait() {
    std::unique_lock<std::mutex> lock(mLock);
    mExitRequested = true;
    lock.unlock();
    mCondition.notify_one();
    mThread.join();
}

void FrameInfoManager::SyncThread::enqueue(SyncThread::Job&& job) {
    std::unique_lock<std::mutex> lock(mLock);
    mQueue.push_back(std::forward<SyncThread::Job>(job));
    lock.unlock();
    mCondition.notify_one();
}

void FrameInfoManager::SyncThread::loop() {
    JobSystem::setThreadPriority(JobSystem::Priority::URGENT_DISPLAY);
    JobSystem::setThreadName("SyncThread");
    auto& queue = mQueue;
    bool exitRequested;
    do {
        std::unique_lock<std::mutex> lock(mLock);
        mCondition.wait(lock, [this, &queue]() -> bool { return mExitRequested || !queue.empty(); });
        exitRequested = mExitRequested;
        if (!queue.empty()) {
            Job job(queue.front());
            queue.pop_front();
            lock.unlock();
            job();
        }
    } while (!exitRequested);
}

} // namespace filament
