// Copyright 2021 The Dawn & Tint Authors
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

//
// AsyncTaskTests:
//     Simple tests for native::AsyncTask and native::AsnycTaskManager.

#include <memory>
#include <mutex>
#include <set>
#include <utility>
#include <vector>

#include "dawn/common/NonCopyable.h"
#include "dawn/native/AsyncTask.h"
#include "dawn/platform/DawnPlatform.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

struct SimpleTaskResult {
    uint32_t id;
};

// A thread-safe queue that stores the task results.
class ConcurrentTaskResultQueue : public NonCopyable {
  public:
    void AddResult(std::unique_ptr<SimpleTaskResult> result) {
        std::lock_guard<std::mutex> lock(mMutex);
        mTaskResults.push_back(std::move(result));
    }

    std::vector<std::unique_ptr<SimpleTaskResult>> GetAllResults() {
        std::vector<std::unique_ptr<SimpleTaskResult>> outputResults;
        {
            std::lock_guard<std::mutex> lock(mMutex);
            outputResults.swap(mTaskResults);
        }
        return outputResults;
    }

  private:
    std::mutex mMutex;
    std::vector<std::unique_ptr<SimpleTaskResult>> mTaskResults;
};

void DoTask(ConcurrentTaskResultQueue* resultQueue, uint32_t id) {
    std::unique_ptr<SimpleTaskResult> result = std::make_unique<SimpleTaskResult>();
    result->id = id;
    resultQueue->AddResult(std::move(result));
}

class AsyncTaskTest : public testing::Test {};

// Emulate the basic usage of worker thread pool in Create*PipelineAsync().
TEST_F(AsyncTaskTest, Basic) {
    platform::Platform platform;
    std::unique_ptr<platform::WorkerTaskPool> pool = platform.CreateWorkerTaskPool();

    native::AsyncTaskManager taskManager(pool.get());
    ConcurrentTaskResultQueue taskResultQueue;

    constexpr size_t kTaskCount = 4u;
    std::set<uint32_t> idset;
    for (uint32_t i = 0; i < kTaskCount; ++i) {
        native::AsyncTask asyncTask([&taskResultQueue, i] { DoTask(&taskResultQueue, i); });
        taskManager.PostTask(std::move(asyncTask));
        idset.insert(i);
    }

    taskManager.WaitAllPendingTasks();

    std::vector<std::unique_ptr<SimpleTaskResult>> results = taskResultQueue.GetAllResults();
    ASSERT_EQ(kTaskCount, results.size());
    for (std::unique_ptr<SimpleTaskResult>& result : results) {
        idset.erase(result->id);
    }
    ASSERT_TRUE(idset.empty());
}

}  // anonymous namespace
}  // namespace dawn
