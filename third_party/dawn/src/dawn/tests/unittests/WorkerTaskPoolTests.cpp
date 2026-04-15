// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <memory>

#include "dawn/platform/DawnPlatform.h"
#include "dawn/tests/MockCallback.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

using testing::NotNull;

using MockTaskCallback = testing::MockCallback<platform::PostWorkerTaskCallback>;
using MockJobCallback = testing::MockCallback<platform::PostWorkerJobCallback>;

class WorkerTaskPoolTests : public testing::Test {
  protected:
    WorkerTaskPoolTests() : pool(platform.CreateWorkerTaskPool()) {}

    platform::Platform platform;
    std::unique_ptr<platform::WorkerTaskPool> pool;
};

// Verifies that a task does work on another thread and we can wait on it.
TEST_F(WorkerTaskPoolTests, PostTask) {
    int result = 0;

    MockTaskCallback cb;
    EXPECT_CALL(cb, Call).WillOnce([&result]() { result += 1; });

    auto event = pool->PostWorkerTask(cb.Callback(), cb.MakeUserdata(nullptr));
    ASSERT_THAT(event, NotNull());
    event->Wait();

    EXPECT_TRUE(event->IsComplete());
    EXPECT_EQ(result, 1);
}

// Verifies that a job does work and can complete.
TEST_F(WorkerTaskPoolTests, PostJob) {
    int result = 0;

    MockJobCallback cb;
    EXPECT_CALL(cb, Call)
        .WillOnce([&result]() {
            result += 1;
            return platform::JobStatus::Continue;
        })
        .WillOnce([&result]() {
            result += 1;
            return platform::JobStatus::Continue;
        })
        .WillOnce([&result]() {
            result += 1;
            return platform::JobStatus::Completed;
        });

    auto handle = pool->PostWorkerJob(cb.Callback(), cb.MakeUserdata(nullptr));
    ASSERT_THAT(handle, NotNull());
    handle->Join();

    EXPECT_EQ(result, 3);
}

// Verifies that a job that completes can be joined.
TEST_F(WorkerTaskPoolTests, PostJobComplete) {
    MockJobCallback cb;
    EXPECT_CALL(cb, Call).WillOnce([]() { return platform::JobStatus::Completed; });

    auto handle = pool->PostWorkerJob(cb.Callback(), cb.MakeUserdata(nullptr));
    ASSERT_THAT(handle, NotNull());
    handle->Join();
}

// Verifies that a non-terminating job can be cancelled and joined.
TEST_F(WorkerTaskPoolTests, PostJobCancel) {
    MockJobCallback cb;
    EXPECT_CALL(cb, Call).WillRepeatedly([]() { return platform::JobStatus::Continue; });

    auto handle = pool->PostWorkerJob(cb.Callback(), cb.MakeUserdata(nullptr));
    ASSERT_THAT(handle, NotNull());
    handle->Cancel();
    handle->Join();
}

}  // anonymous namespace
}  // namespace dawn
