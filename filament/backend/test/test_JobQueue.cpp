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

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <string>
#include <thread>

using namespace filament::backend;

namespace {

// Mimics what a canceled or dropped job captures in practice: something whose destructor runs user
// code that calls back into the queue (e.g. a BufferDescriptor whose release callback chains the
// next asynchronous call). Destroying the job while the queue's lock is held deadlocks.
struct ReentrantOnDestroy {
    JobQueue::Ptr queue;
    std::atomic<JobQueue::JobId>* reentrantJobId;
    ~ReentrantOnDestroy() { reentrantJobId->store(queue->issueJobId()); }
};

} // namespace

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

TEST(JobQueue, CancelDestroysJobWithoutHoldingLock) {
    JobQueue::Ptr queue = JobQueue::create();

    std::atomic<JobQueue::JobId> reentrantJobId = { JobQueue::InvalidJobId };
    JobQueue::JobId const idToCancel = queue->push(
            [guard = std::make_unique<ReentrantOnDestroy>(
                     ReentrantOnDestroy{ queue, &reentrantJobId })]() {});

    // Cancel from another thread so that a regression fails the test instead of hanging it.
    auto canceled = std::make_shared<std::promise<bool>>();
    std::future<bool> future = canceled->get_future();
    std::thread canceller([queue, idToCancel, canceled]() {
        canceled->set_value(queue->cancel(idToCancel));
    });

    if (future.wait_for(std::chrono::seconds(5)) != std::future_status::ready) {
        canceller.detach(); // the thread is stuck holding the queue's lock, it can't be joined
        FAIL() << "cancel() deadlocked: the job was destroyed while holding the queue's lock";
    }

    EXPECT_TRUE(future.get());
    canceller.join();

    // The job's destructor must have been able to use the queue.
    EXPECT_NE(JobQueue::InvalidJobId, reentrantJobId.load());
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

TEST(JobQueue, PushAfterStopReleasesPreIssuedJobId) {
    JobQueue::Ptr queue = JobQueue::create();
    JobQueue::JobId const preIssuedId = queue->issueJobId();
    queue->stop();

    EXPECT_EQ(JobQueue::InvalidJobId, queue->push([]() {}, preIssuedId));

    // The reservation went with the job: there is nothing left to cancel.
    EXPECT_FALSE(queue->cancel(preIssuedId));
}

TEST(JobQueue, PushAfterStopDestroysJobWithoutHoldingLock) {
    JobQueue::Ptr queue = JobQueue::create();
    queue->stop();

    std::atomic<JobQueue::JobId> reentrantJobId = { JobQueue::InvalidJobId };

    // Push from another thread so that a regression fails the test instead of hanging it.
    auto pushed = std::make_shared<std::promise<JobQueue::JobId>>();
    std::future<JobQueue::JobId> future = pushed->get_future();
    std::thread pusher([queue, &reentrantJobId, pushed]() {
        pushed->set_value(queue->push(
                [guard = std::make_unique<ReentrantOnDestroy>(
                         ReentrantOnDestroy{ queue, &reentrantJobId })]() {}));
    });

    if (future.wait_for(std::chrono::seconds(5)) != std::future_status::ready) {
        pusher.detach(); // the thread is stuck holding the queue's lock, it can't be joined
        FAIL() << "push() deadlocked: the dropped job was destroyed while holding the queue's lock";
    }

    EXPECT_EQ(JobQueue::InvalidJobId, future.get());
    pusher.join();

    // The dropped job's destructor must have been able to use the queue.
    EXPECT_NE(JobQueue::InvalidJobId, reentrantJobId.load());
    EXPECT_TRUE(queue->cancel(reentrantJobId.load()));
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

TEST(ThreadWorker, DestroyAfterTerminate) {
    JobQueue::Ptr queue = JobQueue::create();
    bool endCalled = false;

    {
        ThreadWorker::Config config = {
            .name = "TestThread",
            .priority = ThreadWorker::Priority::NORMAL,
            .onEnd = [&endCalled]() { endCalled = true; }
        };
        JobWorker::Ptr worker = ThreadWorker::create(queue, std::move(config));
        worker->terminate();
        // `worker` goes out of scope here. The thread has already been joined, so the destructor
        // must not abort.
    }

    EXPECT_TRUE(endCalled);
}

// Destroying a worker without calling `terminate()` first is a programming error, and the process
// must die on it. In debug builds `~ThreadWorker()` asserts, in release builds the assert is
// compiled out but `std::thread`'s destructor still calls `std::terminate()` on a joinable thread.
// Death tests are unavailable on iOS-family platforms.
#if defined(GTEST_HAS_DEATH_TEST) && GTEST_HAS_DEATH_TEST
TEST(ThreadWorkerDeathTest, DestroyWithoutTerminateAborts) {
#ifdef NDEBUG
    // `std::terminate()`'s message is toolchain-specific, so only the death itself is checked.
    constexpr char const* expected = "";
#else
    constexpr char const* expected = "failed assertion";
#endif
    // This binary is multi-threaded, and the default "fast" style forks without exec, which is
    // unsafe there. "threadsafe" re-executes the binary instead. The previous value is restored so
    // that the other death tests linked into this binary keep their default style.
    std::string const previousStyle = GTEST_FLAG_GET(death_test_style);
    GTEST_FLAG_SET(death_test_style, "threadsafe");

    EXPECT_DEATH({
        JobQueue::Ptr queue = JobQueue::create();
        JobWorker::Ptr worker = ThreadWorker::create(queue, {});
    }, expected);

    GTEST_FLAG_SET(death_test_style, previousStyle);
}
#endif
