//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WorkerThread:
//   Task running thread for ANGLE, similar to a TaskRunner in Chromium.
//   Might be implemented differently depending on platform.
//

#include "common/WorkerThread.h"

#include "common/angleutils.h"
#include "common/system_utils.h"

// Controls if our threading code uses std::async or falls back to single-threaded operations.
// Note that we can't easily use std::async in UWPs due to UWP threading restrictions.
#if !defined(ANGLE_STD_ASYNC_WORKERS) && !defined(ANGLE_ENABLE_WINDOWS_UWP)
#    define ANGLE_STD_ASYNC_WORKERS 1
#endif  // !defined(ANGLE_STD_ASYNC_WORKERS) && & !defined(ANGLE_ENABLE_WINDOWS_UWP)

#if ANGLE_DELEGATE_WORKERS || ANGLE_STD_ASYNC_WORKERS
#    include <future>
#    include <queue>
#    include <thread>
#endif  // ANGLE_DELEGATE_WORKERS || ANGLE_STD_ASYNC_WORKERS

namespace angle
{

WaitableEvent::WaitableEvent()  = default;
WaitableEvent::~WaitableEvent() = default;

void WaitableEventDone::wait() {}

bool WaitableEventDone::isReady()
{
    return true;
}

void AsyncWaitableEvent::markAsReady()
{
    std::lock_guard<std::mutex> lock(mMutex);
    mIsReady = true;
    mCondition.notify_all();
}

void AsyncWaitableEvent::wait()
{
    std::unique_lock<std::mutex> lock(mMutex);
    mCondition.wait(lock, [this] { return mIsReady; });
}

bool AsyncWaitableEvent::isReady()
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mIsReady;
}

WorkerThreadPool::WorkerThreadPool()  = default;
WorkerThreadPool::~WorkerThreadPool() = default;

class SingleThreadedWorkerPool final : public WorkerThreadPool
{
  public:
    std::shared_ptr<WaitableEvent> postWorkerTask(const std::shared_ptr<Closure> &task) override;
    bool isAsync() override;
};

// SingleThreadedWorkerPool implementation.
std::shared_ptr<WaitableEvent> SingleThreadedWorkerPool::postWorkerTask(
    const std::shared_ptr<Closure> &task)
{
    // Thread safety: This function is thread-safe because the task is run on the calling thread
    // itself.
    (*task)();
    return std::make_shared<WaitableEventDone>();
}

bool SingleThreadedWorkerPool::isAsync()
{
    return false;
}

#if ANGLE_STD_ASYNC_WORKERS

class AsyncWorkerPool final : public WorkerThreadPool
{
  public:
    AsyncWorkerPool(size_t numThreads);

    ~AsyncWorkerPool() override;

    std::shared_ptr<WaitableEvent> postWorkerTask(const std::shared_ptr<Closure> &task) override;

    bool isAsync() override;

  private:
    void createThreads();

    using Task = std::pair<std::shared_ptr<AsyncWaitableEvent>, std::shared_ptr<Closure>>;

    // Thread's main loop
    void threadLoop();

    bool mTerminated = false;
    std::mutex mMutex;                 // Protects access to the fields in this class
    std::condition_variable mCondVar;  // Signals when work is available in the queue
    std::queue<Task> mTaskQueue;
    std::deque<std::thread> mThreads;
    size_t mDesiredThreadCount;
};

// AsyncWorkerPool implementation.

AsyncWorkerPool::AsyncWorkerPool(size_t numThreads) : mDesiredThreadCount(numThreads)
{
    ASSERT(numThreads != 0);
}

AsyncWorkerPool::~AsyncWorkerPool()
{
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mTerminated = true;
    }
    mCondVar.notify_all();
    for (auto &thread : mThreads)
    {
        ASSERT(thread.get_id() != std::this_thread::get_id());
        thread.join();
    }
}

void AsyncWorkerPool::createThreads()
{
    if (mDesiredThreadCount == mThreads.size())
    {
        return;
    }
    ASSERT(mThreads.empty());

    for (size_t i = 0; i < mDesiredThreadCount; ++i)
    {
        mThreads.emplace_back(&AsyncWorkerPool::threadLoop, this);
    }
}

std::shared_ptr<WaitableEvent> AsyncWorkerPool::postWorkerTask(const std::shared_ptr<Closure> &task)
{
    // Thread safety: This function is thread-safe because access to |mTaskQueue| is protected by
    // |mMutex|.
    auto waitable = std::make_shared<AsyncWaitableEvent>();
    {
        std::lock_guard<std::mutex> lock(mMutex);

        // Lazily create the threads on first task
        createThreads();

        mTaskQueue.push(std::make_pair(waitable, task));
    }
    mCondVar.notify_one();
    return waitable;
}

void AsyncWorkerPool::threadLoop()
{
    angle::SetCurrentThreadName("ANGLE-Worker");

    while (true)
    {
        Task task;
        {
            std::unique_lock<std::mutex> lock(mMutex);
            mCondVar.wait(lock, [this] { return !mTaskQueue.empty() || mTerminated; });
            if (mTerminated)
            {
                return;
            }
            task = mTaskQueue.front();
            mTaskQueue.pop();
        }

        auto &waitable = task.first;
        auto &closure  = task.second;

        // Note: always add an ANGLE_TRACE_EVENT* macro in the closure.  Then the job will show up
        // in traces.
        (*closure)();
        // Release shared_ptr<Closure> before notifying the event to allow for destructor based
        // dependencies (example: anglebug.com/42267099)
        task.second.reset();
        waitable->markAsReady();
    }
}

bool AsyncWorkerPool::isAsync()
{
    return true;
}

#endif  // ANGLE_STD_ASYNC_WORKERS

#if ANGLE_DELEGATE_WORKERS

class DelegateWorkerPool final : public WorkerThreadPool
{
  public:
    DelegateWorkerPool(PlatformMethods *platform) : mPlatform(platform) {}
    ~DelegateWorkerPool() override = default;

    std::shared_ptr<WaitableEvent> postWorkerTask(const std::shared_ptr<Closure> &task) override;

    bool isAsync() override;

  private:
    PlatformMethods *mPlatform;
};

// A function wrapper to execute the closure and to notify the waitable
// event after the execution.
class DelegateWorkerTask
{
  public:
    DelegateWorkerTask(const std::shared_ptr<Closure> &task,
                       std::shared_ptr<AsyncWaitableEvent> waitable)
        : mTask(task), mWaitable(waitable)
    {}
    DelegateWorkerTask()                     = delete;
    DelegateWorkerTask(DelegateWorkerTask &) = delete;

    static void RunTask(void *userData)
    {
        DelegateWorkerTask *workerTask = static_cast<DelegateWorkerTask *>(userData);
        (*workerTask->mTask)();
        workerTask->mWaitable->markAsReady();

        // Delete the task after its execution.
        delete workerTask;
    }

  private:
    ~DelegateWorkerTask() = default;

    std::shared_ptr<Closure> mTask;
    std::shared_ptr<AsyncWaitableEvent> mWaitable;
};

ANGLE_NO_SANITIZE_CFI_ICALL
std::shared_ptr<WaitableEvent> DelegateWorkerPool::postWorkerTask(
    const std::shared_ptr<Closure> &task)
{
    if (mPlatform->postWorkerTask == nullptr)
    {
        // In the unexpected case where the platform methods have been changed during execution and
        // postWorkerTask is no longer usable, simply run the task on the calling thread.
        (*task)();
        return std::make_shared<WaitableEventDone>();
    }

    // Thread safety: This function is thread-safe because the |postWorkerTask| platform method is
    // expected to be thread safe.  For Chromium, that forwards the call to the |TaskTracker| class
    // in base/task/thread_pool/task_tracker.h which is thread-safe.
    auto waitable = std::make_shared<AsyncWaitableEvent>();

    // The task will be deleted by DelegateWorkerTask::RunTask(...) after its execution.
    DelegateWorkerTask *workerTask = new DelegateWorkerTask(task, waitable);
    mPlatform->postWorkerTask(mPlatform, DelegateWorkerTask::RunTask, workerTask);

    return waitable;
}

bool DelegateWorkerPool::isAsync()
{
    return mPlatform->postWorkerTask != nullptr;
}
#endif

// static
std::shared_ptr<WorkerThreadPool> WorkerThreadPool::Create(size_t numThreads,
                                                           PlatformMethods *platform)
{
    const bool multithreaded = numThreads != 1;
    std::shared_ptr<WorkerThreadPool> pool(nullptr);

#if ANGLE_DELEGATE_WORKERS
    const bool hasPostWorkerTaskImpl = platform->postWorkerTask != nullptr;
    if (hasPostWorkerTaskImpl && multithreaded)
    {
        pool = std::shared_ptr<WorkerThreadPool>(new DelegateWorkerPool(platform));
    }
#endif
#if ANGLE_STD_ASYNC_WORKERS
    if (!pool && multithreaded)
    {
        pool = std::shared_ptr<WorkerThreadPool>(new AsyncWorkerPool(
            numThreads == 0 ? std::thread::hardware_concurrency() : numThreads));
    }
#endif
    if (!pool)
    {
        return std::shared_ptr<WorkerThreadPool>(new SingleThreadedWorkerPool());
    }
    return pool;
}
}  // namespace angle
