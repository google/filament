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

#ifndef SRC_DAWN_PLATFORM_WORKERTHREAD_H_
#define SRC_DAWN_PLATFORM_WORKERTHREAD_H_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "dawn/common/MutexProtected.h"
#include "dawn/common/NonCopyable.h"
#include "dawn/common/RefCounted.h"
#include "dawn/platform/DawnPlatform.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::platform {

class AsyncTaskHandleImpl : public RefCounted, public NonCopyable {
  public:
    ~AsyncTaskHandleImpl() override;

    // dawn::platform::WaitableEvent API.
    void Wait();
    bool IsComplete() const;

  private:
    friend class AsyncWorkerThreadPool;

    explicit AsyncTaskHandleImpl(PostWorkerTaskCallback cb, void* userdata);
    void Complete();

    PostWorkerTaskCallback mCallback;
    raw_ptr<void> mUserdata;
    std::atomic<bool> mCompleted = false;
};

class AsyncJobHandleImpl : public RefCounted, public NonCopyable {
  public:
    ~AsyncJobHandleImpl() override;

    // dawn::platform::JobHandle API.
    void Cancel();
    void Join();

  private:
    friend class AsyncWorkerThreadPool;

    explicit AsyncJobHandleImpl(PostWorkerJobCallback cb, void* userdata);

    void JobThreadLoop(PostWorkerJobCallback cb, void* userdata);

    std::atomic<bool> mCancelled = false;
    bool mJoined = false;
    std::once_flag mJoinFlag;
    // The thread object uses other member fields so it needs to be the last member constructed.
    std::thread mThread;
};

class AsyncWorkerThreadPool : public WorkerTaskPool, public NonCopyable {
  public:
    static constexpr uint32_t kDefaultTaskHandlingJobCount = 2;

    explicit AsyncWorkerThreadPool(uint32_t maxThreadCount = kDefaultTaskHandlingJobCount);
    ~AsyncWorkerThreadPool() override;

    std::unique_ptr<WaitableEvent> PostWorkerTask(PostWorkerTaskCallback callback,
                                                  void* userdata) override;
    std::unique_ptr<JobHandle> PostWorkerJob(PostWorkerJobCallback cb, void* userdata) override;

  private:
    struct TaskTracking {
        uint32_t numJobs = 0;
        std::queue<dawn::Ref<AsyncTaskHandleImpl>> tasks;
    };

    // The task handling thread pool is implemented via jobs where each job is synonymous to a
    // thread.
    JobStatus TaskHandlingJobLoop();

    const uint32_t mMaxTaskThreads;

    // Threads used to handle potentially long-running worker jobs.
    MutexProtected<std::vector<dawn::Ref<AsyncJobHandleImpl>>> mJobHandles;

    // Threads used to handle worker tasks, and the pending tasks they are working on.
    MutexCondVarProtected<TaskTracking> mTaskTracking;
};

}  // namespace dawn::platform

#endif  // SRC_DAWN_PLATFORM_WORKERTHREAD_H_
