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

#include "dawn/platform/WorkerThread.h"

#include <functional>
#include <iterator>
#include <utility>

#include "dawn/common/Assert.h"

namespace dawn::platform {

namespace {
class AsyncWaitableEvent final : public WaitableEvent {
  public:
    explicit AsyncWaitableEvent(Ref<AsyncTaskHandleImpl> impl) : mImpl(std::move(impl)) {}

    void Wait() override { mImpl->Wait(); }
    bool IsComplete() override { return mImpl->IsComplete(); }

  private:
    dawn::Ref<AsyncTaskHandleImpl> mImpl;
};

class AsyncJobHandle final : public JobHandle, private dawn::Ref<AsyncJobHandleImpl> {
  public:
    explicit AsyncJobHandle(Ref<AsyncJobHandleImpl> impl) : mImpl(std::move(impl)) {}

    void Cancel() override { mImpl->Cancel(); }
    void Join() override { mImpl->Join(); }

  private:
    dawn::Ref<AsyncJobHandleImpl> mImpl;
};
}  // anonymous namespace

// AsyncTaskHandleImpl

AsyncTaskHandleImpl::AsyncTaskHandleImpl(PostWorkerTaskCallback cb, void* userdata)
    : mCallback(cb), mUserdata(userdata) {}

AsyncTaskHandleImpl::~AsyncTaskHandleImpl() {
    DAWN_ASSERT(mCompleted);
}

void AsyncTaskHandleImpl::Wait() {
    mCompleted.wait(false, std::memory_order::acquire);
}

bool AsyncTaskHandleImpl::IsComplete() const {
    return mCompleted;
}

void AsyncTaskHandleImpl::Complete() {
    mCallback(mUserdata.ExtractAsDangling());
    mCompleted = true;
    mCompleted.notify_all();
}

// AsyncJobHandleImpl

AsyncJobHandleImpl::AsyncJobHandleImpl(PostWorkerJobCallback cb, void* userdata)
    : mThread(&AsyncJobHandleImpl::JobThreadLoop, this, cb, userdata) {}

AsyncJobHandleImpl::~AsyncJobHandleImpl() {
    DAWN_ASSERT(mJoined);
}

void AsyncJobHandleImpl::Cancel() {
    mCancelled = true;
}

void AsyncJobHandleImpl::Join() {
    std::call_once(mJoinFlag, [&]() {
        mThread.join();
        mJoined = true;
    });
}

void AsyncJobHandleImpl::JobThreadLoop(PostWorkerJobCallback cb, void* userdata) {
    JobStatus status = JobStatus::Continue;
    while (!mCancelled && status == JobStatus::Continue) {
        status = cb(userdata);
    }
}

// AsyncWorkerThreadPool

AsyncWorkerThreadPool::AsyncWorkerThreadPool(uint32_t maxThreadCount)
    : mMaxTaskThreads(maxThreadCount) {
    mJobHandles->reserve(mMaxTaskThreads);
}

AsyncWorkerThreadPool::~AsyncWorkerThreadPool() {
    std::vector<dawn::Ref<AsyncJobHandleImpl>> jobs;
    mJobHandles.Use([&](auto jobHandles) { jobs = std::move(*jobHandles); });

    for (auto& job : jobs) {
        job->Cancel();
    }
    for (auto& job : jobs) {
        job->Join();
    }
}

std::unique_ptr<WaitableEvent> AsyncWorkerThreadPool::PostWorkerTask(
    PostWorkerTaskCallback callback,
    void* userdata) {
    Ref<AsyncTaskHandleImpl> handle = AcquireRef(new AsyncTaskHandleImpl(callback, userdata));
    Ref<AsyncJobHandleImpl> job;
    mTaskTracking.Use<NotifyType::One>([&](auto taskTracking) {
        taskTracking->tasks.emplace(handle);

        // Ensure that there are threads to process the task. This is inlined because it's a bit
        // cumbersome to pass the condition variable guard to a helper.
        if (taskTracking->numJobs == mMaxTaskThreads) {
            return;
        }

        // If we currently have more tasks than jobs start a new job up to the pool limit.
        // TODO(crbug.com/430452846): Better heuristic for this?
        if (taskTracking->numJobs < taskTracking->tasks.size()) {
            job = AcquireRef(new AsyncJobHandleImpl(
                [](void* self) {
                    return static_cast<AsyncWorkerThreadPool*>(self)->TaskHandlingJobLoop();
                },
                this));
        }
    });

    if (job) {
        mJobHandles->push_back(job);
    }

    return std::make_unique<AsyncWaitableEvent>(handle);
}

std::unique_ptr<JobHandle> AsyncWorkerThreadPool::PostWorkerJob(PostWorkerJobCallback cb,
                                                                void* userdata) {
    Ref<AsyncJobHandleImpl> job = AcquireRef(new AsyncJobHandleImpl(cb, userdata));
    mJobHandles.Use([&](auto handles) { handles->push_back(job); });
    return std::make_unique<AsyncJobHandle>(job);
}

JobStatus AsyncWorkerThreadPool::TaskHandlingJobLoop() {
    // By default, wait for 100ms between yielding.
    static constexpr Nanoseconds kWaitDuration = Nanoseconds(100000000);

    Ref<AsyncTaskHandleImpl> task = nullptr;
    mTaskTracking.Use<NotifyType::None>([&](auto taskTracking) {
        if (taskTracking.WaitFor(kWaitDuration, [](auto& x) { return !(x.tasks.empty()); })) {
            task = taskTracking->tasks.front();
            taskTracking->tasks.pop();
        }
    });

    if (task) {
        task->Complete();
    }
    return JobStatus::Continue;
}

}  // namespace dawn::platform
