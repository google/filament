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

#include <gtest/gtest.h>

#include <memory>
#include <utility>

using namespace filament::backend;

namespace {

// The noop driver is the cheapest concrete DriverBase: it needs no platform and no GPU.
class AsyncCompletionTest : public ::testing::Test {
protected:
    void SetUp() override { mDriver.reset(NoopDriver::create()); }

    void TearDown() override { mDriver.reset(); }

    DriverBase* getDriver() const { return static_cast<DriverBase*>(mDriver.get()); }

    // Runs the callbacks that were scheduled without a handler.
    void purge() const { mDriver->purge(); }

    static void countCallback(void* user) { ++*static_cast<int*>(user); }

private:
    std::unique_ptr<Driver> mDriver;
};

} // namespace

TEST_F(AsyncCompletionTest, ScheduleInvokesCallbackOnce) {
    int count = 0;
    {
        DriverBase::AsyncCompletion completion(getDriver(), nullptr, countCallback, &count);
        completion.schedule();
        purge();
        EXPECT_EQ(1, count);

        // Scheduling again is a no-op, and so is the destructor below.
        completion.schedule();
    }
    purge();
    EXPECT_EQ(1, count);
}

TEST_F(AsyncCompletionTest, DestructionInvokesCallback) {
    // This is what a canceled job relies on: the callback must be scheduled even though the job
    // never ran, otherwise the caller can't tell cancellation apart from a still-pending call.
    int count = 0;
    {
        DriverBase::AsyncCompletion const completion(getDriver(), nullptr, countCallback, &count);
    }
    purge();
    EXPECT_EQ(1, count);
}

TEST_F(AsyncCompletionTest, MovingDoesNotInvokeCallbackTwice) {
    int count = 0;
    {
        DriverBase::AsyncCompletion completion(getDriver(), nullptr, countCallback, &count);
        DriverBase::AsyncCompletion const moved(std::move(completion));
        // `completion` is now empty, so only `moved`'s destructor schedules the callback.
    }
    purge();
    EXPECT_EQ(1, count);
}

TEST_F(AsyncCompletionTest, CanceledJobInvokesCallback) {
    // The end-to-end shape of the fix, with the same JobQueue the drivers use: a job that holds an
    // AsyncCompletion notifies the caller whether it runs or is canceled.
    JobQueue::Ptr queue = JobQueue::create();
    int count = 0;

    JobQueue::JobId const idToRun = queue->push(
            [completion = DriverBase::AsyncCompletion(getDriver(), nullptr, countCallback,
                     &count)]() mutable { completion.schedule(); });
    JobQueue::JobId const idToCancel = queue->push(
            [completion = DriverBase::AsyncCompletion(getDriver(), nullptr, countCallback,
                     &count)]() mutable { completion.schedule(); });

    EXPECT_TRUE(queue->cancel(idToCancel));
    purge();
    EXPECT_EQ(1, count) << "canceling a job must schedule its completion callback";

    JobQueue::Job job = queue->pop(false);
    ASSERT_TRUE(job);
    job();
    job = nullptr; // the job holds the completion until it is destroyed
    purge();
    EXPECT_EQ(2, count) << "running a job must schedule its completion callback exactly once";

    EXPECT_FALSE(queue->cancel(idToRun));
}
