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

AsyncTask::State::State(AsyncTaskFunction task) : task(task) {
    DAWN_ASSERT(task);
}

AsyncTask::AsyncTask(AsyncTaskManager* taskManager, AsyncTaskFunction task)
    : mTaskManager(taskManager), mState(task) {
    DAWN_ASSERT(mTaskManager);
}

bool AsyncTask::IsCompleted() const {
    return mState.Use([](auto state) { return state->state == AsyncTaskState::Completed; });
}

void AsyncTask::Wait() {
    mState.Use<NotifyType::None>([](auto state) {
        state.Wait([](auto& x) { return x.state == AsyncTaskState::Completed; });
    });
}

void AsyncTask::AddCompletionCallback(AsyncTaskCompletionCallback completionCallback) {
    bool completeCallbackNow = false;
    mState.Use<NotifyType::None>([&](auto state) {
        if (state->state == AsyncTaskState::Completed) {
            completeCallbackNow = true;
            return;
        }
        state->completionCallbacks.push_back(completionCallback);
    });

    // Call callbacks without holding the lock if the task was already complete.
    if (completeCallbackNow) {
        completionCallback();
    }
}

void AsyncTask::Run() {
    // To ensure we only run the task once, we synchronize it with the lock, move it out when it
    // exists, and call it without holding the lock.
    AsyncTaskFunction task = nullptr;
    mState.Use<NotifyType::None>([&task](auto state) {
        task = std::move(state->task);
        state->task = nullptr;
    });
    DAWN_ASSERT(task);

    // Complete the task and update the state of the task manager. Note we need to make sure we
    // update the state of the task manager before setting the task to Complete to ensure that at
    // teardown when the task manager is waiting on the tasks, that the tasks no longer have a
    // reference to the manager anymore.
    task();
    mTaskManager.ExtractAsDangling()->mTasks.Use([this](auto tasks) { tasks->erase(this); });

    // Update the state, notify all waiting threads, and grab the completion callbacks to call them
    // outside the lock scope.
    std::vector<AsyncTaskCompletionCallback> completionCallbacks;
    mState.Use<NotifyType::All>([&completionCallbacks](auto state) {
        state->state = AsyncTaskState::Completed;

        completionCallbacks = std::move(state->completionCallbacks);
        state->completionCallbacks.clear();
    });
    for (auto completionCallback : completionCallbacks) {
        completionCallback();
    }
}

ErrorGeneratingAsyncTask::ErrorGeneratingAsyncTask(AsyncTaskManager* taskManager,
                                                   std::function<MaybeError()> task)
    : AsyncTask(taskManager, [this, task] {
          // Wrap the task which returns a MaybeError in a void function and store the error in a
          // member.
          MaybeError taskResult = task();
          if (taskResult.IsError()) {
              mErrorData = taskResult.AcquireError();
          }
      }) {}

bool ErrorGeneratingAsyncTask::IsSuccess() const {
    DAWN_ASSERT(IsCompleted());
    return mErrorData == nullptr;
}

bool ErrorGeneratingAsyncTask::IsError() const {
    DAWN_ASSERT(IsCompleted());
    return mErrorData != nullptr;
}

InternalErrorType ErrorGeneratingAsyncTask::GetErrorType() const {
    return mErrorData ? mErrorData->GetType() : InternalErrorType::None;
}

std::unique_ptr<ErrorData> ErrorGeneratingAsyncTask::AcquireError() {
    DAWN_ASSERT(IsCompleted());
    return std::move(mErrorData);
}

AsyncTaskManager::AsyncTaskManager(dawn::platform::WorkerTaskPool* workerTaskPool)
    : mWorkerTaskPool(workerTaskPool) {}

AsyncTaskManager::~AsyncTaskManager() {
    // Pending tasks call back into this task manager. Make sure they all finish before destructing.
    WaitAllPendingTasks();
}

void AsyncTaskManager::PostConstructedTask(Ref<AsyncTask> asyncTask) {
    // Insert the new task and send it off to the workpool to have it completed.
    mTasks.Use([&asyncTask](auto tasks) { tasks->emplace(asyncTask); });
    mWorkerTaskPool->PostWorkerTask(RunTask, asyncTask.Get());
}

void AsyncTaskManager::WaitAllPendingTasks() {
    TaskSet allTasks;
    mTasks.Use([&allTasks](auto tasks) { allTasks.swap(*tasks); });

    for (auto& task : allTasks) {
        task->Wait();
    }
}

bool AsyncTaskManager::HasPendingTasks() const {
    return mTasks.Use([](auto tasks) { return !tasks->empty(); });
}

void AsyncTaskManager::RunTask(void* task) {
    // Note that we create a new Ref<AsyncTask> here because upon completion, we erase the Ref that
    // the AsyncTaskManager holds which may result in dropping the last reference before the
    // completion of this function otherwise. By explicitly creating a Ref here, we ensure that the
    // last reference is only dropped after the scope of this function.
    Ref<AsyncTask> asyncTask = static_cast<AsyncTask*>(task);
    asyncTask->Run();
}

}  // namespace dawn::native
