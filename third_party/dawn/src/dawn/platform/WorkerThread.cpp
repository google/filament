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

#include "dawn/common/Assert.h"

namespace {

class AsyncWaitableEvent final : public dawn::platform::WaitableEvent {
  public:
    AsyncWaitableEvent()
        : mWaitableEventImpl(std::make_shared<dawn::platform::AsyncWaitableEventImpl>()) {}

    void Wait() override { mWaitableEventImpl->Wait(); }

    bool IsComplete() override { return mWaitableEventImpl->IsComplete(); }

    std::shared_ptr<dawn::platform::AsyncWaitableEventImpl> GetWaitableEventImpl() const {
        return mWaitableEventImpl;
    }

  private:
    std::shared_ptr<dawn::platform::AsyncWaitableEventImpl> mWaitableEventImpl;
};

}  // anonymous namespace

namespace dawn::platform {

AsyncWaitableEventImpl::AsyncWaitableEventImpl() : mIsComplete(false) {}

void AsyncWaitableEventImpl::Wait() {
    std::unique_lock<std::mutex> lock(mMutex);
    mCondition.wait(lock, [this] { return mIsComplete; });
}

bool AsyncWaitableEventImpl::IsComplete() {
    std::lock_guard<std::mutex> lock(mMutex);
    return mIsComplete;
}

void AsyncWaitableEventImpl::MarkAsComplete() {
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mIsComplete = true;
    }
    mCondition.notify_all();
}

AsyncWorkerThreadPool::AsyncWorkerThreadPool(uint32_t maxThreadCount)
    : mMaxThreads(maxThreadCount) {
    mThreads.reserve(maxThreadCount);
}

AsyncWorkerThreadPool::~AsyncWorkerThreadPool() {
    {
        std::unique_lock<std::mutex> lock(mMutex);
        DAWN_ASSERT(mPendingTasks.empty());
        mIsDestroyed = true;
    }

    mCondition.notify_all();

    for (auto& thread : mThreads) {
        thread.join();
    }
}

std::unique_ptr<dawn::platform::WaitableEvent> AsyncWorkerThreadPool::PostWorkerTask(
    dawn::platform::PostWorkerTaskCallback callback,
    void* userdata) {
    std::unique_ptr<AsyncWaitableEvent> waitableEvent = std::make_unique<AsyncWaitableEvent>();

    {
        // Lock the task queue and push a new task onto it.
        std::unique_lock<std::mutex> lock(mMutex);
        mPendingTasks.emplace(callback, userdata, waitableEvent->GetWaitableEventImpl());
        EnsureThreads();
    }

    // Notify one of the waiting threads that a task is ready to be executed.
    mCondition.notify_one();

    return waitableEvent;
}

// Must only be called when mMutex is held.
void AsyncWorkerThreadPool::EnsureThreads() {
    if (mThreads.size() == mMaxThreads) {
        return;
    }

    // If we currently have more tasks than threads start a new thread up to the pool limit.
    // TODO(crbug.com/430452846): Better heuristic for this?
    if (mThreads.size() < mPendingTasks.size() && mThreads.size() < mMaxThreads) {
        mThreads.push_back(std::thread(&AsyncWorkerThreadPool::ThreadLoop, this));
    }
}

void AsyncWorkerThreadPool::ThreadLoop() {
    while (true) {
        // Wait for a new task to be available.
        std::unique_lock<std::mutex> lock(mMutex);
        mCondition.wait(lock, [this] { return !mPendingTasks.empty() || mIsDestroyed; });

        // If the thread pool is being destroyed end the thread loop.
        if (mIsDestroyed) {
            break;
        }

        // Get the first task on the queue.
        AsyncWorkerThreadPoolTask task = mPendingTasks.front();
        mPendingTasks.pop();

        // Unlock the task queue so that other tasks can be added or executed while this one is
        // running.
        lock.unlock();

        // Execute the task and mark it as complete.
        task.callback(task.userdata);
        task.waitableEventImpl->MarkAsComplete();
    }
}

}  // namespace dawn::platform
