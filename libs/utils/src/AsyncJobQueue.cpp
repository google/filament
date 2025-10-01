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
#include <utils/JobSystem.h>
#include <utils/debug.h>

#include <mutex>
#include <utility>

namespace utils {

AsyncJobQueue::AsyncJobQueue(const char* name, Priority priority) {
    mQueue.reserve(2);
    mThread = std::thread([this, name, priority]() {
        JobSystem::setThreadName(name);
        JobSystem::setThreadPriority(priority);
        auto& queue = mQueue;
        bool exitRequested;
        do {
            std::unique_lock lock(mLock);
            mCondition.wait(lock, [this]() -> bool { return mExitRequested || !mQueue.empty(); });
            exitRequested = mExitRequested;
            if (!queue.empty()) {
                Job const job(std::move(queue.front()));
                queue.erase(queue.begin());
                lock.unlock();

                job();
            }
        } while (!exitRequested);
    });
}

AsyncJobQueue::~AsyncJobQueue() noexcept {
    assert_invariant(mQueue.empty());
}

void AsyncJobQueue::push(Job&& job) {
    std::unique_lock lock(mLock);
    if (!mExitRequested) {
        mQueue.push_back(std::move(job));
        lock.unlock();
        mCondition.notify_one();
    }
}

void AsyncJobQueue::drainAndExit() {
    std::unique_lock lock(mLock);
    mCondition.wait(lock, [this] { return mQueue.empty(); });
    mExitRequested = true;
    lock.unlock();
    mCondition.notify_one();
    if (mThread.joinable()) {
        mThread.join();
    }
}

} // namespace utils
