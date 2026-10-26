/*
* Copyright (C) 2026 The Android Open Source Project
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
#include "TaskHandler.h"

#include <utils/debug.h>
#include <utils/Panic.h>

namespace filament::backend::fvkutils {

TaskHandler::TaskHandler()
    : mShouldStop(false),
      mThread(&TaskHandler::loop, this) {}

TaskHandler::~TaskHandler() {
    // shutdown() is the expected path; this only catches the cases where the owner never got that
    // far (e.g. a failure while initializing VulkanReadPixels). Note that the pending tasks are
    // still invoked with `executed = false` by loop().
    if (mThread.joinable()) {
        stop();
    }
}

void TaskHandler::post(Task&& task) {
    {
        utils::LockGuard const lock(mTaskQueueMutex);
        assert_invariant(!mShouldStop);
        mTaskQueue.push(std::move(task));
    }
    mHasTaskCondition.notify_one();
}

void TaskHandler::drain() {
    utils::Mutex syncPointMutex;
    utils::Condition syncCondition;
    bool done = false;
    post([&syncPointMutex, &syncCondition, &done](bool) {
        utils::LockGuard const lock(syncPointMutex);
        done = true;
        syncCondition.notify_one();
    });

    utils::UniqueLock lock(syncPointMutex);
    syncCondition.wait(lock, [&done] { return done; });
}

void TaskHandler::stop() noexcept {
    {
        utils::LockGuard const lock(mTaskQueueMutex);
        mShouldStop = true;
    }
    mHasTaskCondition.notify_one();
    mThread.join();
}

void TaskHandler::shutdown() {
    stop();
    bool isEmpty = false;
    {
        utils::LockGuard const lock(mTaskQueueMutex);
        isEmpty = mTaskQueue.empty();
    }
    FILAMENT_CHECK_POSTCONDITION(isEmpty)
            << "TaskHandler has tasks in the queue after shutdown";
}

void TaskHandler::loop() {
    while (true) {
        utils::UniqueLock lock(mTaskQueueMutex);
        mHasTaskCondition.wait(lock, [this]() UTILS_NO_THREAD_SAFETY_ANALYSIS {
            return !mTaskQueue.empty() || mShouldStop;
        });
        if (mShouldStop) {
            break;
        }
        Task task = std::move(mTaskQueue.front());
        mTaskQueue.pop();
        lock.unlock();
        task(true);
    }

    // Clean-up: the tasks we did not run still own resources, so we need to give them a chance to
    // release them.
    while (true) {
        utils::UniqueLock lock(mTaskQueueMutex);
        if (mTaskQueue.empty()) {
            break;
        }
        Task task = std::move(mTaskQueue.front());
        mTaskQueue.pop();
        lock.unlock();
        task(false);
    }
}
}// filament::backend::fvkutils
