/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include "DriverBase.h"
#include "JobQueue.h"

#include "noop/NoopDriver.h"

#include <backend/CallbackHandler.h>
#include <backend/DriverEnums.h>

#include <utils/compiler.h>

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

using namespace filament::backend;

namespace {

class AsyncCompletionTest : public ::testing::Test {
protected:
    void SetUp() override { mDriver.reset(NoopDriver::create()); }

    void TearDown() override { mDriver.reset(); }

    DriverBase* getDriver() const { return static_cast<DriverBase*>(mDriver.get()); }

    // Runs the callbacks that were scheduled without a handler.
    void purge() const { mDriver->purge(); }

    // Same, but keeps going until the queue stays empty.
    void purgeAll() const { mDriver->purgeAll(); }

    // A completion callback that records what it was told, in order.
    using Statuses = std::vector<AsyncCallStatus>;

    static void recordStatus(void* user, AsyncCallStatus const status) {
        static_cast<Statuses*>(user)->push_back(status);
    }

private:
    std::unique_ptr<Driver> mDriver;
};

// Records every status it is told and, while it is being dispatched, schedules the next hop. This
// is the shape CountdownCallbackHandler produces when there is no ServiceThread: its
// countdownCallback is purged, and it schedules the user callback from there.
struct CallbackChain {
    DriverBase* driver;
    int remainingHops;
    std::vector<AsyncCallStatus> statuses;

    static void hop(void* user, AsyncCallStatus const status) {
        auto* const self = static_cast<CallbackChain*>(user);
        self->statuses.push_back(status);
        if (self->remainingHops > 0) {
            --self->remainingHops;
            DriverBase::AsyncCompletion completion(self->driver, nullptr, &CallbackChain::hop,
                    self);
            completion.schedule(AsyncCallStatus::COMPLETED);
        }
    }
};

// Schedules several callbacks from a single dispatch, so the queue grows by a whole batch rather
// than one entry at a time. Draining has to keep up with width, not just depth.
struct CallbackFanout {
    DriverBase* driver;
    int width;
    int recorded = 0;

    static void burst(void* user, AsyncCallStatus) {
        auto* const self = static_cast<CallbackFanout*>(user);
        for (int i = 0; i < self->width; ++i) {
            DriverBase::AsyncCompletion completion(self->driver, nullptr, &CallbackFanout::record,
                    self);
            completion.schedule(AsyncCallStatus::COMPLETED);
        }
    }

    static void record(void* user, AsyncCallStatus) {
        ++static_cast<CallbackFanout*>(user)->recorded;
    }
};

// Stands in for a user handler that hands its callback back to the driver's no-handler queue, which
// is what CountdownCallbackHandler does with a null user handler. Runs on the ServiceThread, so it
// makes that thread a producer for the queue purgeAll() drains.
struct ServiceThreadRelay : public CallbackHandler {
    explicit ServiceThreadRelay(DriverBase* driver)
            : driver(driver) {}

    void post(void* user, Callback callback) override {
        driver->scheduleCallback(nullptr, user, callback);
        relayed.store(true, std::memory_order_release);
    }

    // Returns false if the ServiceThread never got to us, rather than hanging the suite.
    bool waitUntilRelayed() const {
        auto const deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (!relayed.load(std::memory_order_acquire)) {
            if (std::chrono::steady_clock::now() > deadline) {
                return false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        return true;
    }

    DriverBase* driver;
    std::atomic_bool relayed{ false };
};

} // namespace

TEST_F(AsyncCompletionTest, ScheduleInvokesCallbackOnce) {
    Statuses statuses;
    {
        DriverBase::AsyncCompletion completion(getDriver(), nullptr, recordStatus, &statuses);
        completion.schedule(AsyncCallStatus::COMPLETED);
        purge();
        ASSERT_EQ(1u, statuses.size());
        EXPECT_EQ(AsyncCallStatus::COMPLETED, statuses[0]);

        // Scheduling again is a no-op, and so is the destructor below. A different status proves
        // it: if either one went through, the recorded status would change to CANCELED.
        completion.schedule(AsyncCallStatus::CANCELED);
    }
    purge();
    ASSERT_EQ(1u, statuses.size());
    EXPECT_EQ(AsyncCallStatus::COMPLETED, statuses[0]);
}

TEST_F(AsyncCompletionTest, DestructionInvokesCallbackAsCanceled) {
    // This is what a canceled job relies on: the callback must be scheduled even though the job
    // never ran, and it must say so, otherwise the caller can't tell cancellation apart from a
    // still-pending call or from a call that ran.
    Statuses statuses;
    {
        DriverBase::AsyncCompletion const completion(getDriver(), nullptr, recordStatus, &statuses);
    }
    purge();
    ASSERT_EQ(1u, statuses.size());
    EXPECT_EQ(AsyncCallStatus::CANCELED, statuses[0]);
}

TEST_F(AsyncCompletionTest, MovingDoesNotInvokeCallbackTwice) {
    Statuses statuses;
    {
        DriverBase::AsyncCompletion completion(getDriver(), nullptr, recordStatus, &statuses);
        DriverBase::AsyncCompletion const moved(std::move(completion));
        // `completion` is now empty, so only `moved`'s destructor schedules the callback.
    }
    purge();
    ASSERT_EQ(1u, statuses.size());
    EXPECT_EQ(AsyncCallStatus::CANCELED, statuses[0]);
}

TEST_F(AsyncCompletionTest, CanceledJobInvokesCallback) {
    // The end-to-end shape of the fix, with the same JobQueue the drivers use: a job that holds an
    // AsyncCompletion notifies the caller whether it runs or is canceled.
    JobQueue::Ptr queue = JobQueue::create();
    Statuses statuses;

    auto makeJob = [&]() {
        return [completion = DriverBase::AsyncCompletion(getDriver(), nullptr, recordStatus,
                        &statuses)]() mutable {
            completion.schedule(AsyncCallStatus::COMPLETED);
        };
    };

    JobQueue::JobId const idToRun = queue->push(makeJob());
    JobQueue::JobId const idToCancel = queue->push(makeJob());

    EXPECT_TRUE(queue->cancel(idToCancel));
    purge();
    ASSERT_EQ(1u, statuses.size()) << "canceling a job must schedule its completion callback";
    EXPECT_EQ(AsyncCallStatus::CANCELED, statuses[0]);

    JobQueue::Job job = queue->pop(false);
    ASSERT_TRUE(job);
    job();
    job = nullptr; // the job holds the completion until it is destroyed
    purge();
    ASSERT_EQ(2u, statuses.size())
            << "running a job must schedule its completion callback exactly once";
    EXPECT_EQ(AsyncCallStatus::COMPLETED, statuses[1]);

    EXPECT_FALSE(queue->cancel(idToRun));
}

TEST_F(AsyncCompletionTest, DroppedJobInvokesCallback) {
    // The dominant cancellation path: `cancelAsyncJob` wins the race against the `...R()` half and
    // erases the pre-issued id, so `push()` finds no entry and drops the job instead of queuing it.
    // The job is then destroyed on the backend thread, and its completion must still fire.
    JobQueue::Ptr queue = JobQueue::create();
    Statuses statuses;

    JobQueue::JobId const jobId = queue->issueJobId();
    EXPECT_TRUE(queue->cancel(jobId));

    JobQueue::JobId const pushedId = queue->push(
            [completion = DriverBase::AsyncCompletion(getDriver(), nullptr, recordStatus,
                     &statuses)]() mutable {
                completion.schedule(AsyncCallStatus::COMPLETED);
            }, jobId);
    EXPECT_EQ(JobQueue::InvalidJobId, pushedId);

    purge();
    ASSERT_EQ(1u, statuses.size()) << "a dropped job must schedule its completion callback";
    EXPECT_EQ(AsyncCallStatus::CANCELED, statuses[0]);
}

TEST_F(AsyncCompletionTest, JobDroppedByStoppingQueueInvokesCallback) {
    // The other way `push()` drops a job: the queue is stopping. That is the teardown path. An
    // asynchronous call issued afterwards must be canceled.
    JobQueue::Ptr queue = JobQueue::create();
    Statuses statuses;

    queue->stop();
    JobQueue::JobId const pushedId = queue->push(
            [completion = DriverBase::AsyncCompletion(getDriver(), nullptr, recordStatus,
                     &statuses)]() mutable {
                completion.schedule(AsyncCallStatus::COMPLETED);
            });
    EXPECT_EQ(JobQueue::InvalidJobId, pushedId);

    purge();
    ASSERT_EQ(1u, statuses.size())
            << "a job dropped by a stopping queue must schedule its completion callback";
    EXPECT_EQ(AsyncCallStatus::CANCELED, statuses[0]);
}

TEST_F(AsyncCompletionTest, PurgeDispatchesOnlyWhatWasQueued) {
    // purge() is the per-frame path. It dispatches the batch that was queued when it was called and
    // returns, leaving whatever those callbacks scheduled for the next purge -- it must not keep
    // draining, or a frame could be held hostage by callbacks the backend thread keeps producing.
    CallbackChain chain{ getDriver(), 2, {} };

    DriverBase::AsyncCompletion completion(getDriver(), nullptr, &CallbackChain::hop, &chain);
    completion.schedule(AsyncCallStatus::COMPLETED);

    purge();
    EXPECT_EQ(1u, chain.statuses.size());
    purge();
    EXPECT_EQ(2u, chain.statuses.size());

    // Leave nothing behind: ~DriverBase asserts the queue is empty. One more purge() is enough
    // because the last hop doesn't schedule anything.
    purge();
}

TEST_F(AsyncCompletionTest, PurgeAllDrainsCallbacksScheduledFromCallbacks) {
    // purgeAll() is the teardown path. FEngine::shutdown() purges exactly once, so a callback
    // scheduled while the queue is being dispatched has no later purge to pick it up: it would
    // never run, its handler would leak, and ~DriverBase's empty-queue invariant would trip.
    CallbackChain chain{ getDriver(), 2, {} };

    DriverBase::AsyncCompletion completion(getDriver(), nullptr, &CallbackChain::hop, &chain);
    completion.schedule(AsyncCallStatus::COMPLETED);

    purgeAll();

    EXPECT_EQ(3u, chain.statuses.size()) << "purgeAll() stopped before the queue was drained";
    EXPECT_EQ(0, chain.remainingHops);
}

TEST_F(AsyncCompletionTest, PurgeAllOnEmptyQueueIsANoOp) {
    // Teardown is allowed to drain a queue nothing ever landed in, and doing so must not leave the
    // driver unable to take callbacks afterwards.
    purgeAll();
    purgeAll();

    Statuses statuses;
    DriverBase::AsyncCompletion completion(getDriver(), nullptr, recordStatus, &statuses);
    completion.schedule(AsyncCallStatus::COMPLETED);
    purgeAll();

    ASSERT_EQ(1u, statuses.size());
    EXPECT_EQ(AsyncCallStatus::COMPLETED, statuses[0]);
}

TEST_F(AsyncCompletionTest, PurgeAllDrainsChainsOfDifferentDepths) {
    // Several independent chains are in flight at teardown -- one per asynchronous creation still
    // pending. Draining must follow the deepest one, not stop when the shallow ones run out.
    CallbackChain shallow{ getDriver(), 1, {} };
    CallbackChain deep{ getDriver(), 6, {} };

    for (CallbackChain* chain: { &shallow, &deep }) {
        DriverBase::AsyncCompletion completion(getDriver(), nullptr, &CallbackChain::hop, chain);
        completion.schedule(AsyncCallStatus::COMPLETED);
    }

    purgeAll();

    EXPECT_EQ(2u, shallow.statuses.size());
    EXPECT_EQ(7u, deep.statuses.size()) << "purgeAll() stopped before the deepest chain was done";
    EXPECT_EQ(0, deep.remainingHops);
}

TEST_F(AsyncCompletionTest, PurgeAllDrainsBatchesThatWidenWhileDraining) {
    // A single dispatch can enqueue several callbacks at once: a countdown reaching zero releases
    // everything that was waiting on it. The batch being drained grows sideways, not just deeper.
    constexpr int width = 32;
    CallbackFanout fanout{ getDriver(), width };

    DriverBase::AsyncCompletion completion(getDriver(), nullptr, &CallbackFanout::burst, &fanout);
    completion.schedule(AsyncCallStatus::COMPLETED);

    purgeAll();

    EXPECT_EQ(width, fanout.recorded) << "purgeAll() left part of the widened batch behind";
}

TEST_F(AsyncCompletionTest, PurgeAllDispatchesCallbacksHandedBackByTheServiceThread) {
    // The threaded counterpart of the chain above: the callback goes to the ServiceThread, which
    // hands it back to the no-handler queue (what CountdownCallbackHandler does when the user
    // didn't supply a handler). purgeAll() is what has to pick it up at teardown.
    if constexpr (!UTILS_HAS_THREADING) {
        GTEST_SKIP() << "there is no ServiceThread to hand the callback back";
    }

    Statuses statuses;
    ServiceThreadRelay relay(getDriver());

    DriverBase::AsyncCompletion completion(getDriver(), &relay, recordStatus, &statuses);
    completion.schedule(AsyncCallStatus::CANCELED);

    // Wait for the producer to be done, so this asserts on draining and not on the race that
    // purgeAll() explicitly doesn't cover.
    ASSERT_TRUE(relay.waitUntilRelayed()) << "the ServiceThread never dispatched the callback";

    purgeAll();

    ASSERT_EQ(1u, statuses.size())
            << "a callback relayed by the ServiceThread was never dispatched";
    EXPECT_EQ(AsyncCallStatus::CANCELED, statuses[0]);
}
