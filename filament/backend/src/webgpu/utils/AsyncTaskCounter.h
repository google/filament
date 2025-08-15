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

#ifndef TNT_FILAMENT_BACKEND_UTIL_ASYNCWORKCOUNTER_H
#define TNT_FILAMENT_BACKEND_UTIL_ASYNCWORKCOUNTER_H

#include <condition_variable>
#include <mutex>

namespace filament::backend::webgpuutils {

/**
 * Counts the number of asynchronous tasks of a given type.
 * This can be used to wait for such a count to decrement to 0 (wait for work to complete) in a
 * thead safe manner.
 */
class AsyncTaskCounter final {
public:
    AsyncTaskCounter() = default;
    AsyncTaskCounter(AsyncTaskCounter const&) = delete;
    AsyncTaskCounter(AsyncTaskCounter const&&) = delete;
    AsyncTaskCounter& operator=(AsyncTaskCounter const&) = delete;
    AsyncTaskCounter& operator=(AsyncTaskCounter const&&) = delete;
    ~AsyncTaskCounter() = default;

    /**
     * Asynchronously (thread-safe) increase the work counter by one
     */
    void startTask();

    /**
     * Asynchronously (thread-safe) decrease the work counter by one
     */
    void finishTask();

    /**
     * Wait for all tasks to finish (for the work counter to reach 0)  (thread-safe)
     */
    void waitForAllToFinish();

private:
    std::mutex mMutex;
    std::condition_variable mFinishedCondition;
    int mTasksInProgress{ 0 };
};

} // namespace filament::backend::webgpuutils

#endif // TNT_FILAMENT_BACKEND_UTIL_ASYNCWORKCOUNTER_H
