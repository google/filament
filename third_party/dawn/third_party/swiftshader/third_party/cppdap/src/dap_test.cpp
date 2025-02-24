// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "dap/dap.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(DAP, PairedInitializeTerminate) {
  dap::initialize();
  dap::terminate();
}

TEST(DAP, NestedInitializeTerminate) {
  dap::initialize();
  dap::initialize();
  dap::initialize();
  dap::terminate();
  dap::terminate();
  dap::terminate();
}

TEST(DAP, MultiThreadedInitializeTerminate) {
  const size_t numThreads = 64;

  std::mutex mutex;
  std::condition_variable cv;
  size_t numInits = 0;

  std::vector<std::thread> threads;
  threads.reserve(numThreads);
  for (size_t i = 0; i < numThreads; i++) {
    threads.emplace_back([&] {
      dap::initialize();
      {
        std::unique_lock<std::mutex> lock(mutex);
        numInits++;
        if (numInits == numThreads) {
          cv.notify_all();
        } else {
          cv.wait(lock, [&] { return numInits == numThreads; });
        }
      }
      dap::terminate();
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }
}
