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
#include <utils/CountDownLatch.h>
#include <utils/debug.h>
#include <utils/JobSystem.h>
#include <utils/Logger.h>

#include <memory>
#include <mutex>
#include <utility>

namespace utils {

AsyncJobQueue::AsyncJobQueue(const char* name, Priority priority) {
#if !defined(__EMSCRIPTEN__)
    mQueue.reserve(2);
    mThread = std::thread(&AsyncJobQueue::workerThreadLoop, this, name, priority);
#endif
}

#if !defined(__EMSCRIPTEN__)
void AsyncJobQueue::workerThreadLoop(const char* name, Priority priority) {
    JobSystem::setThreadName(name);
    JobSystem::setThreadPriority(priority);
    bool exitRequested;
    decltype(mQueue) tempQueue;
    do {
        UniqueLock lock(mLock);
        // wait until we get a job, or we're asked to exit
        while (!mExitRequested && mQueue.empty()) {
            mCondition.wait(lock);
        }
        exitRequested = mExitRequested;
        tempQueue.swap(mQueue);
        // here we have drained the whole queue, and if exitRequested is set, we're guaranteed
        // no more job will be added after we unlock.
        lock.unlock();

        // execute the jobs without holding a lock. These jobs must be executed in order,
        // front to back, and are allowed to be long-running (like waiting on a fence).
        for (auto& job: tempQueue) {
            job();
        }

        tempQueue.clear();
    } while (!exitRequested);
}
#endif


AsyncJobQueue::~AsyncJobQueue() noexcept {
    // wait for all pending callbacks to be called & terminate the thread
    drainAndExit();
#if !defined(__EMSCRIPTEN__)
    assert_invariant(!mThread.joinable());
    assert_invariant(mQueue.empty());
#endif
}

void AsyncJobQueue::cancelAll() noexcept {
#if !defined(__EMSCRIPTEN__)
    LockGuard const lock(mLock);
    mQueue.clear();
#endif
}

void AsyncJobQueue::push(Job&& job) {
#if !defined(__EMSCRIPTEN__)
    {
        LockGuard const lock(mLock);
        if (UTILS_UNLIKELY(mExitRequested)) {
            LOG(WARNING) << "AsyncJobQueue::push() called after drainAndExit()";
        }
        assert_invariant(!mExitRequested);
        if (UTILS_LIKELY(!mExitRequested)) {
            mQueue.push_back(std::move(job));
        }
    }
    mCondition.notify_one();
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
    {
        LockGuard const lock(mLock);
        if (mExitRequested) {
            return;
        }
        // we request the service thread to exit, but we're guaranteed that it'll only exit
        // after all current callbacks are processed. In addition, once mExitRequested is set,
        // no new jobs can be added, so we can join the thread.
        mExitRequested = true;
    }
    mCondition.notify_one();
    if (mThread.joinable()) {
        mThread.join();
    }
#endif
}

void AsyncJobQueue::drain() {
#if !defined(__EMSCRIPTEN__)
    CountDownLatch latch(1);
    bool pushed = false;
    {
        LockGuard const lock(mLock);

        // If the service thread is already asked to exit, we shouldn't push new jobs or wait,
        // as doing so will cause a deadlock.
        if (!mExitRequested) {
            // We use a shared_ptr's custom deleter to guarantee the latch is signaled EXACTLY once.
            // This elegantly handles edge cases:
            // 1. cancelAll(): If cancelAll() is called, the queue is cleared and the job is destroyed
            //    without being executed. The shared_ptr drops to 0 refs and signals the latch.
            // 2. Normal execution: The worker thread executes the empty lambda, then destroys the job.
            //    The shared_ptr drops to 0 refs and signals the latch.
            std::shared_ptr<CountDownLatch> sp(&latch, [](CountDownLatch* l) {
                l->latch();
            });
            mQueue.push_back([sp]() {});
            pushed = true;
        }
    }

    if (pushed) {
        mCondition.notify_one();
        latch.await();
    }
#endif
}

} // namespace utils
