/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_PRIVATE_JOBQUEUE_H
#define TNT_FILAMENT_BACKEND_PRIVATE_JOBQUEUE_H

#include <utils/FixedCapacityVector.h>
#include <utils/Invocable.h>
#include <utils/JobSystem.h>

#include <mutex>
#include <condition_variable>
#include <thread>
#include <memory>
#include <unordered_map>
#include <limits>
#include <queue>

namespace filament::backend {

/**
 * A thread-safe producer-consumer queue with batching capabilities.
 *
 * This class is thread-safe. All public methods can be called from any thread.
 *
 * This class is stateless regarding concurrency. The *caller* decides the blocking behavior and/or
 * batching when they call a 'pop' methods.
 *
 * A typical use case looks like this:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * #include "private/backend/JobQueue.h"
 * using namespace filament::backend;
 *
 * JobQueue::Ptr queue = JobQueue::create();
 * JobWorker::Ptr worker = AmortizationWorker::create(queue);
 * [ or JobWorker::Ptr worker = ThreadWorker::create(queue, config); ]
 *
 * void loop() {
 *     worker->process(2); // for AmortizationWorker
 * }
 *
 * void cleanup() {
 *     worker->terminate();
 * }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * JobId id = queue->push([](){ ... });
 * queue->cancel(id);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * JobId preIssuedId = queue->issueJobId();
 * JobId id = queue->push([](){ ... }, preIssuedId);
 * assert(id == preIssuedId);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
class JobQueue : public std::enable_shared_from_this<JobQueue> {
    struct PassKey {};
public:
    using Job = utils::Invocable<void()>;
    using JobId = uint32_t;
    using Ptr = std::shared_ptr<JobQueue>;
    static constexpr JobId InvalidJobId = std::numeric_limits<JobId>::max();

    /**
     * Creates an instance of JobQueue. Users should call this to create one.
     * @return An instance of JobQueue
     */
    static Ptr create() {
        return std::make_shared<JobQueue>(PassKey{});
    }

    explicit JobQueue(PassKey) {} // This can be created only via `create()`

    /**
     * Pushes a new job into queue.
     *
     * If the queue is in the process of shutting down (via a call to `stop`), this method does
     * nothing (a no-op) and returns an invalid job ID.
     *
     * @param job The function/lambda to be executed.
     * @param preIssuedJobId The previously issued job ID where this job is assigned to.
     *  If the value is `InvalidJobId`, a new job ID is generated internally.
     * @return A new job ID. If a `preIssuedJobId` is provided, that specific ID is returned instead.
     */
    JobId push(Job job, JobId preIssuedJobId = InvalidJobId);

    /**
     * Retrieves the next job from queue.
     *
     * This method returns a valid job ID if there are pending jobs in the queue, even if the queue
     * is currently in the process of shutting down (via a call to `stop`).
     *
     * @param shouldBlock If true (typically used by ThreadWorker), waits for a job and returns it.
     *  If false (typically used by AmortizationWorker), tries retrieving a job. But may return an
     *  empty job if there's no pending job.
     * @return The next job. If an empty job is returned, it has different meaning depending on the
     *  `shouldBlock` value:
     *    - true: Stop processing (shutting down).
     *    - false: No jobs currently in queue.
     */
    Job pop(bool shouldBlock);

    /**
     * Retrieves a batch of next jobs from queue. Always non-blocking.
     *
     * This method returns a valid job ID if there are pending jobs in the queue, even if the queue
     * is currently in the process of shutting down (via a call to `stop`).
     *
     * @param maxJobsToPop The maximum number of jobs to retrieve.
     *  If < 0, retrieves all pending jobs.
     * @return A FixedCapacityVector<Job> containing the retrieved jobs.
     * Returns an empty vector if the queue is empty.
     */
    utils::FixedCapacityVector<Job> popBatch(int maxJobsToPop);

    /**
     * Generate a new job ID. This newly generated ID is meant to be used for the `preIssuedJobId`
     * parameter of `push` method.
     *
     * @return A new job ID.
     */
    JobId issueJobId() noexcept;

    /**
     * Cancels a job by its ID.
     *
     * @param jobId The job ID to cancel.
     * @return true if the job was found and cancelled, false otherwise.
     */
    bool cancel(JobId jobId) noexcept;

    /**
     * Signals the queue to shut down, after which no further jobs can be added using the `push`
     * method. but all jobs already pushed can still be processed using `pop`.
     */
    void stop() noexcept;

private:
    JobQueue(const JobQueue&) = delete;
    JobQueue& operator=(const JobQueue&) = delete;

    std::mutex mQueueMutex;
    std::condition_variable mQueueCondition;
    std::unordered_map<JobId, Job> mJobsMap;
    std::queue<JobId> mJobOrder;
    JobId mNextJobId = 0;
    bool mIsStopping = false;
};

/**
 * Abstract base class for all worker types.
 */
class JobWorker {
public:
    using Ptr = std::unique_ptr<JobWorker>;

    virtual ~JobWorker();

    /**
     * Processes a batch of jobs. (For non-threaded workers)
     * @param jobCount Max jobs to process (<= 0 for all).
     */
    virtual void process(int jobCount) {}

    /**
     * Terminates the worker.
     */
    virtual void terminate();

protected:
    explicit JobWorker(JobQueue::Ptr queue) : mQueue(std::move(queue)) {}

    JobQueue::Ptr mQueue;

private:
    JobWorker(const JobWorker&) = delete;
    JobWorker& operator=(const JobWorker&) = delete;
};


/**
 * A non-threaded worker that consumes jobs in batches.
 */
class AmortizationWorker final : public JobWorker {
    struct PassKey {};
public:
    /**
     * Creates an instance of AmortizationWorker. Users should call this to create one.
     * @return An instance of AmortizationWorker
     */
    static Ptr create(JobQueue::Ptr queue) {
        return std::make_unique<AmortizationWorker>(std::move(queue), PassKey{});
    }

    explicit AmortizationWorker(JobQueue::Ptr queue, PassKey); // This can be created only via `create()`

    /**
     * Polls the queue and executes a batch of jobs.
     *
     * @param jobCount The max number of jobs to process.
     *  0 = do nothing.
     *  1 = pop one (optimized).
     *  > 1 = pop batch.
     *  <= -1 = pop all.
     */
    void process(int jobCount) override;

    /**
     * Signals the queue to stop and drain all pending jobs.
     * This is safe to call multiple times.
     */
    void terminate() override;
};


/**
 * A threaded worker that consumes jobs one by one, blocking when empty.
 */
class ThreadWorker final : public JobWorker {
    struct PassKey {};
public:
    using Priority = utils::JobSystem::Priority;

    /**
     * Config settings for the worker
     */
    struct Config {
        std::string_view name = "";
        Priority priority = Priority::NORMAL;
        utils::Invocable<void()> onBegin;  // Executed when the thread worker begins
        utils::Invocable<void()> onEnd;    // Executed when the thread worker ends
    };

    /**
     * Creates an instance of ThreadWorker. Users should call this to create one.
     * @return An instance of ThreadWorker
     */
    static Ptr create(JobQueue::Ptr queue, Config config) {
        return std::make_unique<ThreadWorker>(std::move(queue), std::move(config), PassKey{});
    }

    ThreadWorker(JobQueue::Ptr queue, Config config, PassKey); // This can be created only via `create()`

    ~ThreadWorker() override = default;

    /**
     * Signals the queue to stop and joins the worker thread.
     * This is safe to call multiple times.
     */
    void terminate() override;

private:
    Config mConfig;
    std::thread mThread;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_PRIVATE_JOBQUEUE_H
