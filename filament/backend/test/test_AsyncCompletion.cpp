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

#include <backend/DriverEnums.h>

#include <gtest/gtest.h>

#include <memory>
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

    // A completion callback that records what it was told, in order.
    using Statuses = std::vector<AsyncCallStatus>;

    static void recordStatus(void* user, AsyncCallStatus const status) {
        static_cast<Statuses*>(user)->push_back(status);
    }

private:
    std::unique_ptr<Driver> mDriver;
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
