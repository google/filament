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
    JobQueue::Ptr storage = JobQueue::create();
    int v = 0;

    storage->push([&v]() { v = 1; });
    JobQueue::Job job = storage->pop(false);
    ASSERT_TRUE(job);
    job();
    EXPECT_EQ(1, v);
}

TEST(JobQueue, PopEmpty) {
    JobQueue::Ptr storage = JobQueue::create();
    JobQueue::Job job = storage->pop(false);
    ASSERT_FALSE(job);
}

TEST(JobQueue, PopBatch) {
    JobQueue::Ptr storage = JobQueue::create();
    int v = 0;

    storage->push([&v]() { v++; });
    storage->push([&v]() { v++; });
    storage->push([&v]() { v++; });

    auto jobs = storage->popBatch(2);
    EXPECT_EQ(2, jobs.size());
    for (auto& job : jobs) {
        job();
    }
    EXPECT_EQ(2, v);

    jobs = storage->popBatch(10);
    EXPECT_EQ(1, jobs.size());
    for (auto& job : jobs) {
        job();
    }
    EXPECT_EQ(3, v);
}

TEST(JobQueue, PopAll) {
    JobQueue::Ptr storage = JobQueue::create();
    int v = 0;

    storage->push([&v]() { v++; });
    storage->push([&v]() { v++; });
    storage->push([&v]() { v++; });

    auto jobs = storage->popBatch(-1);
    EXPECT_EQ(3, jobs.size());
    for (auto& job : jobs) {
        job();
    }
    EXPECT_EQ(3, v);
}

TEST(JobQueue, Cancel) {
    JobQueue::Ptr storage = JobQueue::create();
    int v = 0;

    JobQueue::JobId idToCancel = storage->push([&v]() { v = 1; });
    storage->push([&v]() { v = 2; });

    EXPECT_TRUE(storage->cancel(idToCancel));

    auto jobs = storage->popBatch(-1);
    EXPECT_EQ(1, jobs.size());
    jobs[0]();
    EXPECT_EQ(2, v);
}

TEST(JobQueue, CancelInvalid) {
    JobQueue::Ptr storage = JobQueue::create();
    EXPECT_FALSE(storage->cancel(123));
}

TEST(JobQueue, Stop) {
    JobQueue::Ptr storage = JobQueue::create();
    int v = 0;
    storage->push([&v]() { v = 1; });
    storage->stop();
    // After stop, we can't push new jobs. This should be a no-op.
    JobQueue::JobId id = storage->push([&v]() { v = 2; });
    EXPECT_EQ(JobQueue::InvalidJobId, id);

    auto job = storage->pop(false);
    EXPECT_TRUE(job);
    job();
    EXPECT_EQ(1, v);

    job = storage->pop(false);
    EXPECT_FALSE(job);
}

TEST(JobQueue, PreIssuedJobId) {
    JobQueue::Ptr storage = JobQueue::create();
    JobQueue::JobId preIssuedId = storage->issueJobId();
    JobQueue::JobId id = storage->push([]() {}, preIssuedId);
    EXPECT_EQ(id, preIssuedId);
}

TEST(JobQueue, MultipleProducersConsumers) {
    JobQueue::Ptr storage = JobQueue::create();
    std::atomic_int v = {0};
    constexpr int NUM_THREADS = 4;
    constexpr int JOBS_PER_THREAD = 200;

    // Multiple producers
    std::vector<std::thread> producers;
    std::atomic_bool doneProducing = false;
    for (int i = 0; i < NUM_THREADS; ++i) {
        producers.emplace_back([&]() {
            for (int j = 0; j < JOBS_PER_THREAD; ++j) {
                storage->push([&v]() { v++; });
            }
        });
    }

    // Multiple consumers
    std::thread blockingConsumer = std::thread([&]() {
        while (true) {
            if (auto job = storage->pop(true)) {
                job();
            } else {
                break; // This means the job storage is stopped.
            }
        }
    });
    std::thread nonBlockingConsumer = std::thread([&]() {
        while (true) {
            if (auto job = storage->pop(false)) {
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
            utils::FixedCapacityVector<JobQueue::Job> jobs = storage->popBatch(2);
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
    storage->stop();

    // Waiting for consumers to complete handling jobs
    blockingConsumer.join();
    nonBlockingConsumer.join();
    nonBlockingPopBatchConsumer.join();

    EXPECT_EQ(NUM_THREADS * JOBS_PER_THREAD, v.load());
}


TEST(AmortizationWorker, Process) {
    JobQueue::Ptr storage = JobQueue::create();
    JobWorker::Ptr worker = AmortizationWorker::create(storage);
    int v = 0;

    storage->push([&v]() { v++; });
    storage->push([&v]() { v++; });
    storage->push([&v]() { v++; });

    worker->process(2);
    EXPECT_EQ(2, v);

    worker->process(1);
    EXPECT_EQ(3, v);

    // No pending jobs, so it should be a no-op.
    worker->process(1);
    EXPECT_EQ(3, v);
}

TEST(AmortizationWorker, ProcessAll) {
    JobQueue::Ptr storage = JobQueue::create();
    JobWorker::Ptr worker = AmortizationWorker::create(storage);
    int v = 0;

    storage->push([&v]() { v++; });
    storage->push([&v]() { v++; });
    storage->push([&v]() { v++; });

    worker->process(-1);
    EXPECT_EQ(3, v);

    // No pending jobs, so it should be a no-op.
    worker->process(1);
    EXPECT_EQ(3, v);
}

TEST(AmortizationWorker, TerminateDrainsAllJobs) {
    JobQueue::Ptr storage = JobQueue::create();
    JobWorker::Ptr worker = AmortizationWorker::create(storage);
    int v = 0;

    storage->push([&v]() { v++; });
    storage->push([&v]() { v++; });

    // `terminate` should drain all jobs
    worker->terminate();
    EXPECT_EQ(2, v);

    // After terminate, pushing new jobs should not work.
    storage->push([&v]() { v++; });
    worker->process(1);
    EXPECT_EQ(2, v);
}


TEST(ThreadWorker, Process) {
    JobQueue::Ptr storage = JobQueue::create();
    JobWorker::Ptr worker = ThreadWorker::create(storage, {});
    std::atomic_int v = {0};

    storage->push([&v]() { v++; });
    storage->push([&v]() { v++; });

    // `terminate` should drain all jobs
    worker->terminate();
    EXPECT_EQ(2, v.load());

    // After terminate, pushing new jobs should not work.
    storage->push([&v]() { v++; });
    worker->terminate();
    EXPECT_EQ(2, v.load());
}

TEST(ThreadWorker, Callbacks) {
    JobQueue::Ptr storage = JobQueue::create();
    bool beginCalled = false;
    bool endCalled = false;

    ThreadWorker::Config config = {
        .name = "TestThread",
        .priority = ThreadWorker::Priority::NORMAL,
        .onBegin = [&beginCalled]() { beginCalled = true; },
        .onEnd = [&endCalled]() { endCalled = true; }
    };
    JobWorker::Ptr worker = ThreadWorker::create(storage, std::move(config));
    worker->terminate();

    EXPECT_TRUE(beginCalled);
    EXPECT_TRUE(endCalled);
}
