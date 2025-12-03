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

#include <utils/JobStorage.h>

#include <atomic>
#include <thread>

using namespace utils;

TEST(JobStorage, PushAndPop) {
    JobStorage::Ptr storage = JobStorage::create();
    int v = 0;

    storage->push([&v]() { v = 1; });
    JobStorage::Job job = storage->pop(false);
    ASSERT_TRUE(job);
    job();
    EXPECT_EQ(1, v);
}

TEST(JobStorage, PopEmpty) {
    JobStorage::Ptr storage = JobStorage::create();
    JobStorage::Job job = storage->pop(false);
    ASSERT_FALSE(job);
}

TEST(JobStorage, PopBatch) {
    JobStorage::Ptr storage = JobStorage::create();
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

TEST(JobStorage, PopAll) {
    JobStorage::Ptr storage = JobStorage::create();
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

TEST(JobStorage, Cancel) {
    JobStorage::Ptr storage = JobStorage::create();
    int v = 0;

    JobStorage::JobId idToCancel = storage->push([&v]() { v = 1; });
    storage->push([&v]() { v = 2; });

    EXPECT_TRUE(storage->cancel(idToCancel));

    auto jobs = storage->popBatch(-1);
    EXPECT_EQ(1, jobs.size());
    jobs[0]();
    EXPECT_EQ(2, v);
}

TEST(JobStorage, CancelInvalid) {
    JobStorage::Ptr storage = JobStorage::create();
    EXPECT_FALSE(storage->cancel(123));
}

TEST(JobStorage, Stop) {
    JobStorage::Ptr storage = JobStorage::create();
    int v = 0;
    storage->push([&v]() { v = 1; });
    storage->stop();
    // After stop, we can't push new jobs. This should be a no-op.
    JobStorage::JobId id = storage->push([&v]() { v = 2; });
    EXPECT_EQ(JobStorage::InvalidJobId, id);

    auto job = storage->pop(false);
    EXPECT_TRUE(job);
    job();
    EXPECT_EQ(1, v);

    job = storage->pop(false);
    EXPECT_FALSE(job);
}

TEST(JobStorage, PreIssuedJobId) {
    JobStorage::Ptr storage = JobStorage::create();
    JobStorage::JobId preIssuedId = storage->issueJobId();
    JobStorage::JobId id = storage->push([]() {}, preIssuedId);
    EXPECT_EQ(id, preIssuedId);
}

TEST(JobStorage, MultipleProducersConsumers) {
    JobStorage::Ptr storage = JobStorage::create();
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
            FixedCapacityVector<JobStorage::Job> jobs = storage->popBatch(2);
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
    JobStorage::Ptr storage = JobStorage::create();
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
    JobStorage::Ptr storage = JobStorage::create();
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
    JobStorage::Ptr storage = JobStorage::create();
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
    JobStorage::Ptr storage = JobStorage::create();
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
    JobStorage::Ptr storage = JobStorage::create();
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
