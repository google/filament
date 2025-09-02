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

#include "dawn/common/NonCopyable.h"
#include "dawn/platform/DawnPlatform.h"

namespace dawn::platform {

class AsyncWaitableEventImpl {
  public:
    AsyncWaitableEventImpl();

    void Wait();
    bool IsComplete();
    void MarkAsComplete();

  private:
    std::mutex mMutex;
    std::condition_variable mCondition;
    bool mIsComplete;
};

struct AsyncWorkerThreadPoolTask {
    dawn::platform::PostWorkerTaskCallback callback;
    void* userdata;
    std::shared_ptr<AsyncWaitableEventImpl> waitableEventImpl;
};

class AsyncWorkerThreadPool : public dawn::platform::WorkerTaskPool, public NonCopyable {
  public:
    static constexpr uint32_t kDefaultThreadCount = 2;

    explicit AsyncWorkerThreadPool(uint32_t maxThreadCount = kDefaultThreadCount);
    ~AsyncWorkerThreadPool() override;

    std::unique_ptr<dawn::platform::WaitableEvent> PostWorkerTask(
        dawn::platform::PostWorkerTaskCallback callback,
        void* userdata) override;

  private:
    void EnsureThreads();
    void ThreadLoop();

    const uint32_t mMaxThreads;
    std::vector<std::thread> mThreads;
    std::queue<AsyncWorkerThreadPoolTask> mPendingTasks;
    std::mutex mMutex;
    std::condition_variable mCondition;
    bool mIsDestroyed = false;
};

}  // namespace dawn::platform

#endif  // SRC_DAWN_PLATFORM_WORKERTHREAD_H_
