// Copyright 2021 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_ASYNCTASK_H_
#define SRC_DAWN_NATIVE_ASYNCTASK_H_

#include <functional>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "dawn/common/MutexProtected.h"
#include "dawn/common/NonCopyable.h"
#include "dawn/common/Ref.h"
#include "dawn/common/RefCounted.h"
#include "dawn/native/Error.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::platform {
class WaitableEvent;
class WorkerTaskPool;
}  // namespace dawn::platform

namespace dawn::native {

class AsyncTaskManager;

// TODO(crbug.com/dawn/826): we'll add additional things to AsyncTask in the future, like
// Cancel() and RunNow(). Cancelling helps avoid running the task's body when we are just
// shutting down the device. RunNow() could be used for more advanced scenarios, for example
// always doing ShaderModule initial compilation asynchronously, but being able to steal the
// task if we need it for synchronous pipeline compilation.

enum class AsyncTaskState : uint8_t {
    Pending = 0,
    Completed = 1,
};

using AsyncTaskFunction = std::function<void()>;
using AsyncTaskCompletionCallback = std::function<void()>;

class AsyncTask : public RefCounted {
  public:
    AsyncTask(AsyncTaskManager* taskManager, AsyncTaskFunction task);

    bool IsCompleted() const;
    void Wait();

    void AddCompletionCallback(AsyncTaskCompletionCallback completionCallback);

  private:
    // Friends with the task manager to privately manage when tasks are executed.
    friend class AsyncTaskManager;
    void Run();

    // Async tasks are created when we post to an AsyncTaskManager. The task needs a pointer back to
    // the task manager to update the manager's state to let it know it has completed.
    raw_ptr<AsyncTaskManager> mTaskManager;

    struct State {
        explicit State(AsyncTaskFunction task);

        AsyncTaskState state = AsyncTaskState::Pending;
        AsyncTaskFunction task;
        std::vector<AsyncTaskCompletionCallback> completionCallbacks;
    };
    MutexCondVarProtected<State> mState;
};

class ErrorGeneratingAsyncTask : public AsyncTask {
  public:
    ErrorGeneratingAsyncTask(AsyncTaskManager* taskManager, std::function<MaybeError()> task);

    bool IsSuccess() const;
    bool IsError() const;
    InternalErrorType GetErrorType() const;
    std::unique_ptr<ErrorData> AcquireError();

  private:
    std::unique_ptr<ErrorData> mErrorData;
};

class AsyncTaskManager {
  public:
    explicit AsyncTaskManager(dawn::platform::WorkerTaskPool* workerTaskPool);
    ~AsyncTaskManager();

    template <typename TaskType, class... Args>
    Ref<TaskType> PostTask(Args&&... args) {
        Ref<TaskType> asyncTask = AcquireRef(new TaskType(this, std::forward<Args>(args)...));
        PostConstructedTask(asyncTask);
        return asyncTask;
    }

    void WaitAllPendingTasks();
    bool HasPendingTasks() const;

  private:
    friend class AsyncTask;

    void PostConstructedTask(Ref<AsyncTask> asyncTask);
    static void RunTask(void* task);

    using TaskSet = absl::flat_hash_set<Ref<AsyncTask>>;
    MutexProtected<TaskSet> mTasks;

    raw_ptr<dawn::platform::WorkerTaskPool> mWorkerTaskPool;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_ASYNCTASK_H_
