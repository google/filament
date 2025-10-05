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

#ifndef TNT_ASYNCJOBQUEUE_H
#define TNT_ASYNCJOBQUEUE_H

#include <utils/JobSystem.h>
#include <utils/Invocable.h>
#include <utils/Mutex.h> // NOLINT(*-include-cleaner)
#include <utils/Condition.h> // NOLINT(*-include-cleaner)

#include <thread>
#include <vector>

namespace utils {

/**
 * Simple asynchronous job queue. This manages a *single*thread that executes jobs submitted
 * to it in order.
 */
class AsyncJobQueue {
public:
    using Job = Invocable<void()>;
    using Priority = JobSystem::Priority;

    // create the job queue with a name and desired priority
    AsyncJobQueue(const char* name, Priority priority);

    // drainAndExit()  must be called first
    ~AsyncJobQueue() noexcept;

    // blocks until all jobs are executed and quits the thread
    void drainAndExit();

    // adds a job to the queue. no-op if drainAndExit() was called.
    void push(Job&& job);

private:
    using Container = std::vector<Job>;
    std::thread mThread;
    Mutex mLock; // NOLINT(*-include-cleaner)
    Condition mCondition; // NOLINT(*-include-cleaner)
    Container mQueue;
    bool mExitRequested = false;
};

} // namespace utils

#endif //TNT_ASYNCJOBQUEUE_H
