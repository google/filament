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

#include "AsyncTaskCounter.h"

#include "utils/debug.h"

#include <mutex>

namespace filament::backend::webgpuutils {

void AsyncTaskCounter::startTask() {
    std::lock_guard<std::mutex> lock{ mMutex };
    ++mTasksInProgress;
}

void AsyncTaskCounter::finishTask() {
    std::lock_guard<std::mutex> lock{ mMutex };
    --mTasksInProgress;
    assert_invariant(mTasksInProgress >= 0);
    if (mTasksInProgress == 0) {
        mFinishedCondition.notify_all();
    }
}

void AsyncTaskCounter::waitForAllToFinish() {
    std::unique_lock<std::mutex> lock{ mMutex };
    mFinishedCondition.wait(lock, [this] { return mTasksInProgress == 0; });
}

} // namespace filament::backend::webgpuutils

