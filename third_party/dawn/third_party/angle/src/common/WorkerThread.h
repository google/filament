//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WorkerThread:
//   Asychronous tasks/threads for ANGLE, similar to a TaskRunner in Chromium.
//   Can be implemented as different targets, depending on platform.
//

#ifndef COMMON_WORKER_THREAD_H_
#define COMMON_WORKER_THREAD_H_

#include <array>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

#include "common/debug.h"
#include "platform/PlatformMethods.h"

namespace angle
{

class WorkerThreadPool;

// A callback function with no return value and no arguments.
class Closure
{
  public:
    virtual ~Closure()        = default;
    virtual void operator()() = 0;
};

// An event that we can wait on, useful for joining worker threads.
class WaitableEvent : angle::NonCopyable
{
  public:
    WaitableEvent();
    virtual ~WaitableEvent();

    // Waits indefinitely for the event to be signaled.
    virtual void wait() = 0;

    // Peeks whether the event is ready. If ready, wait() will not block.
    virtual bool isReady() = 0;

    template <class T>
    // Waits on multiple events. T should be some container of std::shared_ptr<WaitableEvent>.
    static void WaitMany(T *waitables)
    {
        for (auto &waitable : *waitables)
        {
            waitable->wait();
        }
    }

    template <class T>
    // Checks if all events are ready. T should be some container of std::shared_ptr<WaitableEvent>.
    static bool AllReady(T *waitables)
    {
        for (auto &waitable : *waitables)
        {
            if (!waitable->isReady())
            {
                return false;
            }
        }
        return true;
    }
};

// A waitable event that is always ready.
class WaitableEventDone final : public WaitableEvent
{
  public:
    void wait() override;
    bool isReady() override;
};

// A waitable event that can be completed asynchronously
class AsyncWaitableEvent final : public WaitableEvent
{
  public:
    AsyncWaitableEvent()           = default;
    ~AsyncWaitableEvent() override = default;

    void wait() override;
    bool isReady() override;

    void markAsReady();

  private:
    // To protect the concurrent accesses from both main thread and background
    // threads to the member fields.
    std::mutex mMutex;

    bool mIsReady = false;
    std::condition_variable mCondition;
};

// Request WorkerThreads from the WorkerThreadPool. Each pool can keep worker threads around so
// we avoid the costly spin up and spin down time.
class WorkerThreadPool : angle::NonCopyable
{
  public:
    WorkerThreadPool();
    virtual ~WorkerThreadPool();

    // Creates a new thread pool.
    // If numThreads is 0, the pool will choose the best number of threads to run.
    // If numThreads is 1, the pool will be single-threaded. Tasks will run on the calling thread.
    // Other numbers indicate how many threads the pool should spawn.
    // Note that based on build options, this class may not actually run tasks in threads, or it may
    // hook into the provided PlatformMethods::postWorkerTask, in which case numThreads is ignored.
    static std::shared_ptr<WorkerThreadPool> Create(size_t numThreads, PlatformMethods *platform);

    // Returns an event to wait on for the task to finish.  If the pool fails to create the task,
    // returns null.  This function is thread-safe.
    virtual std::shared_ptr<WaitableEvent> postWorkerTask(const std::shared_ptr<Closure> &task) = 0;

    virtual bool isAsync() = 0;

  private:
};

}  // namespace angle

#endif  // COMMON_WORKER_THREAD_H_
