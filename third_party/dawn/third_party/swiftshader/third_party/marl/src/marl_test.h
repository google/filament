// Copyright 2019 The Marl Authors.
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

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "marl/scheduler.h"

// SchedulerParams holds Scheduler construction parameters for testing.
struct SchedulerParams {
  int numWorkerThreads;

  friend std::ostream& operator<<(std::ostream& os,
                                  const SchedulerParams& params) {
    return os << "SchedulerParams{"
              << "numWorkerThreads: " << params.numWorkerThreads << "}";
  }
};

// WithoutBoundScheduler is a test fixture that does not bind a scheduler.
class WithoutBoundScheduler : public testing::Test {
 public:
  void SetUp() override {
    allocator = new marl::TrackedAllocator(marl::Allocator::Default);
  }

  void TearDown() override {
    auto stats = allocator->stats();
    ASSERT_EQ(stats.numAllocations(), 0U);
    ASSERT_EQ(stats.bytesAllocated(), 0U);
    delete allocator;
  }

  marl::TrackedAllocator* allocator = nullptr;
};

// WithBoundScheduler is a parameterized test fixture that performs tests with
// a bound scheduler using a number of different configurations.
class WithBoundScheduler : public testing::TestWithParam<SchedulerParams> {
 public:
  void SetUp() override {
    allocator = new marl::TrackedAllocator(marl::Allocator::Default);

    auto& params = GetParam();

    marl::Scheduler::Config cfg;
    cfg.setAllocator(allocator);
    cfg.setWorkerThreadCount(params.numWorkerThreads);
    cfg.setFiberStackSize(0x10000);

    auto scheduler = new marl::Scheduler(cfg);
    scheduler->bind();
  }

  void TearDown() override {
    auto scheduler = marl::Scheduler::get();
    scheduler->unbind();
    delete scheduler;

    auto stats = allocator->stats();
    ASSERT_EQ(stats.numAllocations(), 0U);
    ASSERT_EQ(stats.bytesAllocated(), 0U);
    delete allocator;
  }

  marl::TrackedAllocator* allocator = nullptr;
};
