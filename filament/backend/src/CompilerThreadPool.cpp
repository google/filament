/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "CompilerThreadPool.h"

#include <private/utils/Tracing.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Mutex.h>

#include <algorithm>
#include <memory>
#include <mutex>
#include <utility>

namespace filament::backend {

using namespace utils;

ProgramToken::~ProgramToken() = default;

CompilerThreadPool::CompilerThreadPool() noexcept = default;

CompilerThreadPool::~CompilerThreadPool() noexcept {
    assert_invariant(mCompilerThreads.empty());
    assert_invariant(mQueues[0].empty());
    assert_invariant(mQueues[1].empty());
    assert_invariant(mQueues[2].empty());
}

void CompilerThreadPool::init(uint32_t const threadCount,
        ThreadSetup&& threadSetup, ThreadCleanup&& threadCleanup) {
    auto setup = std::make_shared<ThreadSetup>(std::move(threadSetup));
    auto cleanup = std::make_shared<ThreadCleanup>(std::move(threadCleanup));

    for (size_t i = 0; i < threadCount; i++) {
        mCompilerThreads.emplace_back([this, setup, cleanup]() {
            FILAMENT_TRACING_CONTEXT(FILAMENT_TRACING_CATEGORY_FILAMENT);

            (*setup)();

            // process jobs from the queue until we're asked to exit
            while (true) {
                UniqueLock lock(mQueueLock);
                mQueueCondition.wait(lock, [this]() UTILS_NO_THREAD_SAFETY_ANALYSIS {
                    // TODO: can be replaced with std::ranges once client fully supports c++20
                    return mExitRequested || (!std::all_of(mQueues.begin(), mQueues.end(),
                                                     [](auto&& q) { return q.empty(); }));
                });

                if (UTILS_UNLIKELY(mExitRequested)) {
                    break;
                }

                FILAMENT_TRACING_VALUE(FILAMENT_TRACING_CATEGORY_FILAMENT, "CompilerThreadPool Jobs",
                        mQueues[0].size() + mQueues[1].size() + mQueues[2].size());

                Job job;
                // use the first queue that's not empty
                auto& queue = [this]() UTILS_NO_THREAD_SAFETY_ANALYSIS -> auto& {
                    for (auto& q: mQueues) {
                        if (!q.empty()) {
                            return q;
                        }
                    }
                    return mQueues[0]; // we should never end-up here.
                }();
                assert_invariant(!queue.empty());
                std::swap(job, queue.front().second);
                queue.pop_front();

                // execute the job without holding any locks
                lock.unlock();
                job();
            }

            (*cleanup)();
        });

    }
}

auto CompilerThreadPool::find(program_token_t const& token) -> std::pair<Queue&, Queue::iterator> {
    for (auto&& q: mQueues) {
        auto pos = std::find_if(q.begin(), q.end(), [&token](auto&& item) {
            return item.first == token;
        });
        if (pos != q.end()) {
            return { q, pos };
        }
    }
    // this can happen if the program is being processed right now
    return { mQueues[0], mQueues[0].end() };
}

auto CompilerThreadPool::dequeue(program_token_t const& token) -> Job {
    LockGuard const lock(mQueueLock);
    Job job;
    auto&& [q, pos] = find(token);
    if (pos != q.end()) {
        std::swap(job, pos->second);
        q.erase(pos);
    }
    return job;
}

void CompilerThreadPool::queue(CompilerPriorityQueue priorityQueue,
        program_token_t const& token, Job&& job) {
    LockGuard const lock(mQueueLock);
    mQueues[size_t(priorityQueue)].emplace_back(token, std::move(job));
    mQueueCondition.notify_one();
}

void CompilerThreadPool::terminate() noexcept {
    {
        LockGuard const lock(mQueueLock);
        mExitRequested = true;
        mQueueCondition.notify_all();
    } // lock releases automatically here!

    // We MUST unlock the queue lock before joining background compiler threads.
    // If we hold mQueueLock while blocked inside join(), a worker thread waking up
    // from its condition variable wait will attempt to re-acquire mQueueLock and block,
    // resulting in a fatal runtime deadlock.
    for (auto& thread: mCompilerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    mCompilerThreads.clear();

    // Clear all the queues, dropping the remaining jobs. This relies on the jobs being cancelable.
    // Since background threads are joined, no other thread is active. We briefly re-acquire
    // the lock solely to satisfy Clang's static capability analysis (mQueues is UTILS_GUARDED_BY).
    {
        LockGuard const lock(mQueueLock);
        for (auto&& q : mQueues) {
            q.clear();
        }
    } // lock releases automatically here!
}

} // namespace filament::backend
