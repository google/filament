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

#ifndef TNT_FILAMENT_BACKEND_VULKANTASKHANDLER_H
#define TNT_FILAMENT_BACKEND_VULKANTASKHANDLER_H

#include "utils/Panic.h"

#include <functional>
#include <map>
#include <memory>
#include <queue>

namespace filament::backend {

class VulkanTaskHandler {
public:
    // The Host class is meant to be called on the thread that processes the tasks. It has access to
    // the handle function.
    class Host {
    public:
        Host(Host const&) = delete;
        Host& operator=(Host const&) = delete;
    protected:
        Host()
            : mHandler(std::make_unique<VulkanTaskHandler>()) {}

        void runTaskHandler() noexcept {
            mHandler->handle();
        }

        VulkanTaskHandler& getTaskHandler() noexcept {
            return *mHandler;
        }
    private:
        std::unique_ptr<VulkanTaskHandler> mHandler;
    };

    using TaskId = uint32_t;
    using TaskFunc = std::function<void(TaskId, void*)>;
    using Task = std::pair<TaskFunc, void*>;

    TaskId createTask(TaskFunc const& func, void* data) noexcept {
        TaskId const id = mNextTaskId++;
        mTasks[id] = std::pair(func, data);
        return id;
    }

    // Task that will never be put on the queue again should be marked as completed by calling this
    // function.
    void completed(TaskId taskId) noexcept {
        assert_invariant(mTasks.find(taskId) != mTasks.end());
        mTasks.erase(taskId);
    }

    void post(TaskId taskId) noexcept {
        assert_invariant(mTasks.find(taskId) != mTasks.end());
        mTaskQueue.push(taskId);
    }

    VulkanTaskHandler() = default;

    VulkanTaskHandler(VulkanTaskHandler const&) = delete;
    VulkanTaskHandler& operator=(VulkanTaskHandler const&) = delete;

private:
    inline void handle() {
        while (!mTaskQueue.empty()) {
            auto taskId = mTaskQueue.front();
            mTaskQueue.pop();
            // It is possible for taskIds in the queue to refer to a task that has already been
            // completed. Just ignore.
            if (mTasks.find(taskId) == mTasks.end()) {
                continue;
            }

            auto& [func, data] = mTasks[taskId];
            func(taskId, data);
        }
    }

    std::queue<TaskId> mTaskQueue;
    std::map<TaskId, Task> mTasks;
    uint32_t mNextTaskId = 0;

    friend class Host;
};

}// namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_VULKANTASKHANDLER_H
