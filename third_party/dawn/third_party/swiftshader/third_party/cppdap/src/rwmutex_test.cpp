// Copyright 2020 Google LLC
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

#include "rwmutex.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <array>
#include <thread>
#include <vector>

namespace {
constexpr const size_t NumThreads = 8;
}

// Check that WLock behaves like regular mutex.
TEST(RWMutex, WLock) {
  dap::RWMutex rwmutex;
  int counter = 0;

  std::vector<std::thread> threads;
  for (size_t i = 0; i < NumThreads; i++) {
    threads.emplace_back([&] {
      for (int j = 0; j < 1000; j++) {
        dap::WLock lock(rwmutex);
        counter++;
        EXPECT_EQ(counter, 1);
        counter--;
      }
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }

  EXPECT_EQ(counter, 0);
}

TEST(RWMutex, NoRLockWithWLock) {
  dap::RWMutex rwmutex;

  std::vector<std::thread> threads;
  std::array<int, NumThreads> counters = {};

  {  // With WLock held...
    dap::WLock wlock(rwmutex);

    for (size_t i = 0; i < counters.size(); i++) {
      int* counter = &counters[i];
      threads.emplace_back([&rwmutex, counter] {
        dap::RLock lock(rwmutex);
        for (int j = 0; j < 1000; j++) {
          (*counter)++;
        }
      });
    }

    // RLocks should block
    for (int counter : counters) {
      EXPECT_EQ(counter, 0);
    }
  }

  for (auto& thread : threads) {
    thread.join();
  }

  for (int counter : counters) {
    EXPECT_EQ(counter, 1000);
  }
}

TEST(RWMutex, NoWLockWithRLock) {
  dap::RWMutex rwmutex;

  std::vector<std::thread> threads;
  size_t counter = 0;

  {  // With RLocks held...
    dap::RLock rlockA(rwmutex);
    dap::RLock rlockB(rwmutex);
    dap::RLock rlockC(rwmutex);

    for (size_t i = 0; i < NumThreads; i++) {
      threads.emplace_back(std::thread([&] {
        dap::WLock lock(rwmutex);
        counter++;
      }));
    }

    // ... WLocks should block
    EXPECT_EQ(counter, 0U);
  }

  for (auto& thread : threads) {
    thread.join();
  }

  EXPECT_EQ(counter, NumThreads);
}
