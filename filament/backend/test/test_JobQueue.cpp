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

#include <gtest/gtest.h>

#include "private/backend/JobQueue.h"

#include <atomic>
#include <thread>

using namespace filament::backend;

TEST(JobQueue, PushAndPop) {
    JobQueue::Ptr queue = JobQueue::create();
    int v = 0;

    queue->push([&v]() { v = 1; });
    JobQueue::Job job = queue->pop(false);
    ASSERT_TRUE(job);
    job();
    EXPECT_EQ(1, v);
}

TEST(JobQueue, PopEmpty) {
    JobQueue::Ptr queue = JobQueue::create();
    JobQueue::Job job = queue->pop(false);
    ASSERT_FALSE(job);
}

TEST(JobQueue, PopBatch) {
    JobQueue::Ptr queue = JobQueue::create();
    int v = 0;

    queue->push([&v]() { v++; });
    queue->push([&v]() { v++; });
    queue->push([&v]() { v++; });

    auto jobs = queue->popBatch(2);
    EXPECT_EQ(2, jobs.size());
    for (auto& job : jobs) {
        job();
    }
    EXPECT_EQ(2, v);

    jobs = queue->popBatch(10);
    EXPECT_EQ(1, jobs.size());
    for (auto& job : jobs) {
        job();
    }
    EXPECT_EQ(3, v);
}

TEST(JobQueue, PopAll) {
    JobQueue::Ptr queue = JobQueue::create();
    int v = 0;

    queue->push([&v]() { v++; });
    queue->push([&v]() { v++; });
    queue->push([&v]() { v++; });

    auto jobs = queue->popBatch(-1);
    EXPECT_EQ(3, jobs.size());
    for (auto& job : jobs) {
        job();
    }
    EXPECT_EQ(3, v);
}

TEST(JobQueue, Cancel) {
    JobQueue::Ptr queue = JobQueue::create();
    int v = 0;

    JobQueue::JobId idToCancel = queue->push([&v]() { v = 1; });
    queue->push([&v]() { v = 2; });

    EXPECT_TRUE(queue->cancel(idToCancel));

    auto jobs = queue->popBatch(-1);
    EXPECT_EQ(1, jobs.size());
    jobs[0]();
    EXPECT_EQ(2, v);
}

TEST(JobQueue, CancelInvalid) {
    JobQueue::Ptr queue = JobQueue::create();
    EXPECT_FALSE(queue->cancel(123));
}

TEST(JobQueue, Stop) {
    JobQueue::Ptr queue = JobQueue::create();
    int v = 0;
    queue->push([&v]() { v = 1; });
    queue->stop();
    // After stop, we can't push new jobs. This should be a no-op.
    JobQueue::JobId id = queue->push([&v]() { v = 2; });
    EXPECT_EQ(JobQueue::InvalidJobId, id);

    auto job = queue->pop(false);
    EXPECT_TRUE(job);
    job();
    EXPECT_EQ(1, v);

    job = queue->pop(false);
    EXPECT_FALSE(job);
}

TEST(JobQueue, PreIssuedJobId) {
    JobQueue::Ptr queue = JobQueue::create();
    JobQueue::JobId preIssuedId = queue->issueJobId();
    JobQueue::JobId id = queue->push([]() {}, preIssuedId);
    EXPECT_EQ(id, preIssuedId);
}

TEST(JobQueue, MultipleProducersConsumers) {
    JobQueue::Ptr queue = JobQueue::create();
    std::atomic_int v = {0};
    constexpr int NUM_THREADS = 4;
    constexpr int JOBS_PER_THREAD = 200;

    // Multiple producers
    std::vector<std::thread> producers;
    std::atomic_bool doneProducing = false;
    for (int i = 0; i < NUM_THREADS; ++i) {
        producers.emplace_back([&]() {
            for (int j = 0; j < JOBS_PER_THREAD; ++j) {
                queue->push([&v]() { v++; });
            }
        });
    }

    // Multiple consumers
    std::thread blockingConsumer = std::thread([&]() {
        while (true) {
            if (auto job = queue->pop(true)) {
                job();
            } else {
                break; // This means the job queue is stopped.
            }
        }
    });
    std::thread nonBlockingConsumer = std::thread([&]() {
        while (true) {
            if (auto job = queue->pop(false)) {
                job();
            } else {
                if (doneProducing.load()) {
                    break;
                }
                std::this_thread::yield();
            }
        }
    });
    std::thread nonBlockingPopBatchConsumer = std::thread([&]() {
        while (true) {
            utils::FixedCapacityVector<JobQueue::Job> jobs = queue->popBatch(2);
            if (!jobs.empty()) {
                for (auto& job : jobs) {
                    job();
                }
            } else {
                if (doneProducing.load()) {
                    break;
                }
                std::this_thread::yield();
            }
        }
    });

    // Waiting for producers to complete pushing jobs
    for (auto& t : producers) {
        t.join();
    }
    doneProducing = true; // signal for non-blocking consumer
    queue->stop();

    // Waiting for consumers to complete handling jobs
    blockingConsumer.join();
    nonBlockingConsumer.join();
    nonBlockingPopBatchConsumer.join();

    EXPECT_EQ(NUM_THREADS * JOBS_PER_THREAD, v.load());
}


TEST(AmortizationWorker, Process) {
    JobQueue::Ptr queue = JobQueue::create();
    JobWorker::Ptr worker = AmortizationWorker::create(queue);
    int v = 0;

    queue->push([&v]() { v++; });
    queue->push([&v]() { v++; });
    queue->push([&v]() { v++; });

    worker->process(2);
    EXPECT_EQ(2, v);

    worker->process(1);
    EXPECT_EQ(3, v);

    // No pending jobs, so it should be a no-op.
    worker->process(1);
    EXPECT_EQ(3, v);
}

TEST(AmortizationWorker, ProcessAll) {
    JobQueue::Ptr queue = JobQueue::create();
    JobWorker::Ptr worker = AmortizationWorker::create(queue);
    int v = 0;

    queue->push([&v]() { v++; });
    queue->push([&v]() { v++; });
    queue->push([&v]() { v++; });

    worker->process(-1);
    EXPECT_EQ(3, v);

    // No pending jobs, so it should be a no-op.
    worker->process(1);
    EXPECT_EQ(3, v);
}

TEST(AmortizationWorker, TerminateDrainsAllJobs) {
    JobQueue::Ptr queue = JobQueue::create();
    JobWorker::Ptr worker = AmortizationWorker::create(queue);
    int v = 0;

    queue->push([&v]() { v++; });
    queue->push([&v]() { v++; });

    // `terminate` should drain all jobs
    worker->terminate();
    EXPECT_EQ(2, v);

    // After terminate, pushing new jobs should not work.
    queue->push([&v]() { v++; });
    worker->process(1);
    EXPECT_EQ(2, v);
}


TEST(ThreadWorker, Process) {
    JobQueue::Ptr queue = JobQueue::create();
    JobWorker::Ptr worker = ThreadWorker::create(queue, {});
    std::atomic_int v = {0};

    queue->push([&v]() { v++; });
    queue->push([&v]() { v++; });

    // `terminate` should drain all jobs
    worker->terminate();
    EXPECT_EQ(2, v.load());

    // After terminate, pushing new jobs should not work.
    queue->push([&v]() { v++; });
    worker->terminate();
    EXPECT_EQ(2, v.load());
}

TEST(ThreadWorker, Callbacks) {
    JobQueue::Ptr queue = JobQueue::create();
    bool beginCalled = false;
    bool endCalled = false;

    ThreadWorker::Config config = {
        .name = "TestThread",
        .priority = ThreadWorker::Priority::NORMAL,
        .onBegin = [&beginCalled]() { beginCalled = true; },
        .onEnd = [&endCalled]() { endCalled = true; }
    };
    JobWorker::Ptr worker = ThreadWorker::create(queue, std::move(config));
    worker->terminate();

    EXPECT_TRUE(beginCalled);
    EXPECT_TRUE(endCalled);
}
