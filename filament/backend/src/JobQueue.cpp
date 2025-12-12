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

#include "JobQueue.h"

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Panic.h>

namespace filament::backend {

JobQueue::JobQueue(PassKey) {}

JobQueue::JobId JobQueue::push(Job job, JobId const preIssuedJobId/* = InvalidJobId*/) {
    JobId jobId = preIssuedJobId;
    {
        std::lock_guard<std::mutex> lock(mQueueMutex);
        if (mIsStopping) {
            return InvalidJobId;
        }

        if (jobId == InvalidJobId) {
            jobId = genNextJobId();
            mJobsMap[jobId] = std::move(job);
        } else {
            // Use the job ID previously issued by `issueJobId()`
            auto it = mJobsMap.find(jobId);
            if (it == mJobsMap.end()) {
                // Pre-issued job does not exist, either users passed a wrong id (unlikely)
                // or the job must have been canceled (likely)
                return InvalidJobId;
            }
            FILAMENT_CHECK_PRECONDITION(!static_cast<bool>(it->second))
                    << "pre-issued job has already been populated";
            it->second = std::move(job);
        }
        mJobOrder.push(jobId);
    }

    // Always notify. A ThreadWorker might be waiting.
    mQueueCondition.notify_one();

    return jobId;
}

JobQueue::Job JobQueue::pop(bool shouldBlock) {
    std::unique_lock<std::mutex> lock(mQueueMutex);

    decltype(mJobsMap)::iterator it;

    while (true) {
        if (shouldBlock) {
            // Wait only if we're in blocking mode and the queue is empty
            mQueueCondition.wait(lock, [this] { return !mJobOrder.empty() || mIsStopping; });
        }

        if (mJobOrder.empty()) {
            // When `shouldBlock` is true, this means the queue is stopping now.
            // When `shouldBlock` is false, this means there's no job.
            return nullptr;
        }

        JobId jobId = mJobOrder.front();
        mJobOrder.pop();

        it = mJobsMap.find(jobId);
        if (it != mJobsMap.end()) {
            break;
        }

        // If execution reaches this line, the job must have been canceled right after being added.
        // Therefore, we should continue the loop and attempt to retrieve the next available job.
    }

    Job job = std::move(it->second);
    mJobsMap.erase(it);
    return job;
}

utils::FixedCapacityVector<JobQueue::Job> JobQueue::popBatch(int const maxJobsToPop) {
    utils::FixedCapacityVector<Job> jobs;

    if (UTILS_UNLIKELY(maxJobsToPop == 0)) {
        return jobs;
    }

    std::lock_guard<std::mutex> lock(mQueueMutex);
    if (mJobOrder.empty()) {
        return jobs;
    }

    // Calculate jobs to take. If maxJobsToPop is negative, we take all jobs.
    size_t jobsToTake = mJobOrder.size();
    if (0 < maxJobsToPop && maxJobsToPop < static_cast<int>(jobsToTake)) {
        jobsToTake = maxJobsToPop;
    }
    jobs.reserve(jobsToTake);

    while (0 < jobsToTake && !mJobOrder.empty()) {
        JobId jobId = mJobOrder.front();
        mJobOrder.pop();

        auto it = mJobsMap.find(jobId);
        if (UTILS_UNLIKELY(it == mJobsMap.end())) {
            // The job was probably canceled.
            continue;
        }

        jobs.push_back(std::move(it->second));
        --jobsToTake;
        mJobsMap.erase(it);
    }

    return jobs;
}

JobQueue::JobId JobQueue::issueJobId() noexcept {
    std::lock_guard<std::mutex> lock(mQueueMutex);
    JobId const jobId = genNextJobId();
    // Preallocate a job, which serves two main purposes. It provides a valid jobId that can be
    // checked for integrity when passed to the `push` method, and it enables job cancellation for
    // tasks that are yet to be pushed.
    mJobsMap[jobId];
    return jobId;
}

bool JobQueue::cancel(JobId const jobId) noexcept {
    std::lock_guard<std::mutex> lock(mQueueMutex);

    auto it = mJobsMap.find(jobId);
    if (it == mJobsMap.end()) {
        return false; // Job not found, must have been completed or canceled.
    }

    mJobsMap.erase(it);

    return true;
}

void JobQueue::stop() noexcept {
    {
        std::lock_guard<std::mutex> lock(mQueueMutex);
        mIsStopping = true;
    }
    mQueueCondition.notify_all(); // Wake up all waiting threads
}

JobQueue::JobId JobQueue::genNextJobId() noexcept {
    // We assume this method is called within the critical section.
    JobId newJobId = mNextJobId++;
    // We assume the job ID won't overflow or wraps around to zero within the application's lifetime.
    assert_invariant(newJobId != InvalidJobId);
    return newJobId;
}

JobWorker::~JobWorker() = default;

void JobWorker::terminate() {
    // This is called from workers `terminate()`, which may hinder the concurrent use of multiple
    // workers. Consider removing this line and require the owner/caller to explicitly invoke it to
    // enable multiple worker instances.
    if (mQueue) {
        mQueue->stop();
    }
}

AmortizationWorker::AmortizationWorker(JobQueue::Ptr queue, PassKey)
    : JobWorker(std::move(queue)) {
}

AmortizationWorker::~AmortizationWorker() = default;

void AmortizationWorker::process(int const jobCount) {
    if (!mQueue || jobCount == 0) {
        return;
    }

    if (jobCount == 1) {
        // Handle single job without vector allocation.
        if (auto job = mQueue->pop(false)) {
            job();
        }
        return;
    }

    // Handle batch (jobCount > 1 or jobCount < 0 for "all pending jobs")
    utils::FixedCapacityVector<JobQueue::Job> jobs = mQueue->popBatch(jobCount);
    if (jobs.empty()) {
        return;
    }

    for (auto& job: jobs) {
        job();
    }
}

void AmortizationWorker::terminate() {
    JobWorker::terminate();

    // Drain all pending jobs.
    process(-1);
}

ThreadWorker::ThreadWorker(JobQueue::Ptr queue, Config config, PassKey)
        : JobWorker(std::move(queue)), mConfig(std::move(config)) {
    mThread = std::thread([this]() {
        utils::JobSystem::setThreadName(mConfig.name.data());
        utils::JobSystem::setThreadPriority(mConfig.priority);

        if (mConfig.onBegin) {
            mConfig.onBegin();
        }

        while (JobQueue::Job job = mQueue->pop(true)) {
            job();
        }

        if (mConfig.onEnd) {
            mConfig.onEnd();
        }
    });
}

ThreadWorker::~ThreadWorker() = default;

void ThreadWorker::terminate() {
    JobWorker::terminate();

    if (mThread.joinable()) {
        mThread.join();
    }
}

} // namespace utils
