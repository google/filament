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

#ifndef TNT_CROSSTHREADTASK_H
#define TNT_CROSSTHREADTASK_H

#include <optional>
#include <functional>
#include <mutex>
#include <atomic>

template <typename T, typename... Args>
class CrossThreadTask {
public:
    bool queueTask(std::function<T> task);
    void runTask(Args...);
    bool isClosed() const;
    void setClosed();

private:
    std::mutex mMutex;
    std::optional<std::function<T>> mTask;
    std::atomic<bool> mClosed = false;
};

template <typename T, typename... Args>
bool CrossThreadTask<T, Args...>::queueTask(std::function<T> task) {
    std::unique_lock<std::mutex> lock(mMutex);
    if (mTask.has_value()) {
        return false;
    }
    mTask = task;
    return true;
}

template <typename T, typename... Args>
void CrossThreadTask<T, Args...>::runTask(Args... args) {
    std::optional<std::function<T>> taskToRun;
    {
        std::unique_lock<std::mutex> lock(mMutex);
        std::swap(mTask, taskToRun);
    }
    if (taskToRun.has_value()) {
        (*taskToRun)(args...);
    }
}

template <typename T, typename... Args>
bool CrossThreadTask<T, Args...>::isClosed() const {
    return mClosed;
}

template <typename T, typename... Args>
void CrossThreadTask<T, Args...>::setClosed() {
    mClosed = true;
}

#endif // TNT_CROSSTHREADTASK_H
