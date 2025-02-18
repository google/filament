// Copyright 2022 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/AsyncTask.h"

#include <utility>

#include "dawn/platform/DawnPlatform.h"

namespace dawn::native {

AsyncTaskManager::AsyncTaskManager(dawn::platform::WorkerTaskPool* workerTaskPool)
    : mWorkerTaskPool(workerTaskPool) {}

void AsyncTaskManager::PostTask(AsyncTask asyncTask) {
    // If these allocations becomes expensive, we can slab-allocate tasks.
    Ref<WaitableTask> waitableTask = AcquireRef(new WaitableTask());
    waitableTask->taskManager = this;
    waitableTask->asyncTask = std::move(asyncTask);

    {
        // We insert new waitableTask objects into mPendingTasks in main thread (PostTask()),
        // and we may remove waitableTask objects from mPendingTasks in either main thread
        // (WaitAllPendingTasks()) or sub-thread (TaskCompleted), so mPendingTasks should be
        // protected by a mutex.
        std::lock_guard<std::mutex> lock(mPendingTasksMutex);
        mPendingTasks.emplace(waitableTask.Get(), waitableTask);
    }

    // Ref the task since it is accessed inside the worker function.
    // The worker function will acquire and release the task upon completion.
    waitableTask->AddRef();
    waitableTask->waitableEvent =
        mWorkerTaskPool->PostWorkerTask(DoWaitableTask, waitableTask.Get());
}

void AsyncTaskManager::HandleTaskCompletion(WaitableTask* task) {
    std::lock_guard<std::mutex> lock(mPendingTasksMutex);
    mPendingTasks.erase(task);
}

void AsyncTaskManager::WaitAllPendingTasks() {
    absl::flat_hash_map<WaitableTask*, Ref<WaitableTask>> allPendingTasks;

    {
        std::lock_guard<std::mutex> lock(mPendingTasksMutex);
        allPendingTasks.swap(mPendingTasks);
    }

    for (auto& [_, task] : allPendingTasks) {
        task->waitableEvent->Wait();
    }
}

bool AsyncTaskManager::HasPendingTasks() {
    std::lock_guard<std::mutex> lock(mPendingTasksMutex);
    return !mPendingTasks.empty();
}

void AsyncTaskManager::DoWaitableTask(void* task) {
    Ref<WaitableTask> waitableTask = AcquireRef(static_cast<WaitableTask*>(task));
    waitableTask->asyncTask();
    waitableTask->taskManager->HandleTaskCompletion(waitableTask.Get());
}

AsyncTaskManager::WaitableTask::WaitableTask() = default;

AsyncTaskManager::WaitableTask::~WaitableTask() = default;

}  // namespace dawn::native
