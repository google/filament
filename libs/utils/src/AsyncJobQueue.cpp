/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include <utils/AsyncJobQueue.h>
#include <utils/compiler.h>
#include <utils/JobSystem.h>
#include <utils/debug.h>
#include <utils/Logger.h>

#include <mutex>
#include <utility>

namespace utils {

AsyncJobQueue::AsyncJobQueue(const char* name, Priority priority) {
#if !defined(__EMSCRIPTEN__)
    mQueue.reserve(2);
    mThread = std::thread([this, name, priority]() {
        JobSystem::setThreadName(name);
        JobSystem::setThreadPriority(priority);
        bool exitRequested;
        do {
            std::unique_lock lock(mLock);
            // wait until we get a job, or we're asked to exit
            mCondition.wait(lock, [this]() -> bool {
                return mExitRequested || !mQueue.empty();
            });
            exitRequested = mExitRequested;
            auto const queue = std::move(mQueue);
            // here we have drained the whole queue, and if exitRequested is set, we're guaranteed
            // no more job will be added after we unlock.
            lock.unlock();

            // execute the jobs without holding a lock. These jobs must be executed in order,
            // front to back, and are allowed to be long-running (like waiting on a fence).
            for (auto& job : queue) {
                job();
            }
        } while (!exitRequested);
    });
#endif
}

AsyncJobQueue::~AsyncJobQueue() noexcept {
#if !defined(__EMSCRIPTEN__)
    assert_invariant(mQueue.empty());
#endif
}

void AsyncJobQueue::cancelAll() noexcept {
#if !defined(__EMSCRIPTEN__)
    std::unique_lock const lock(mLock);
    mQueue.clear();
#endif
}

void AsyncJobQueue::push(Job&& job) {
#if !defined(__EMSCRIPTEN__)
    std::unique_lock lock(mLock);
    if (UTILS_UNLIKELY(mExitRequested)) {
        LOG(WARNING) << "AsyncJobQueue::push() called after drainAndExit()";
    }
    assert_invariant(!mExitRequested);
    if (UTILS_LIKELY(!mExitRequested)) {
        mQueue.push_back(std::move(job));
        lock.unlock();
        mCondition.notify_one();
    }
#endif
}

bool AsyncJobQueue::isValid() const noexcept {
#if !defined(__EMSCRIPTEN__)
    return mThread.joinable();
#else
    return false;
#endif
}


void AsyncJobQueue::drainAndExit() {
#if !defined(__EMSCRIPTEN__)
    std::unique_lock lock(mLock);
    // we request the service thread to exit, but we're guaranteed that it'll only exit
    // after all current callbacks are processed. In addition, once mExitRequested is set,
    // no new jobs can be added, so we can join the thread.
    mExitRequested = true;
    lock.unlock();
    mCondition.notify_one();
    if (mThread.joinable()) {
        mThread.join();
    }
#endif
}

} // namespace utils
