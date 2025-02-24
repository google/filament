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

#include "marl_test.h"

#include "marl/thread.h"

namespace {

marl::Thread::Core core(int idx) {
  marl::Thread::Core c;
  c.pthread.index = static_cast<uint16_t>(idx);
  return c;
}

}  // anonymous namespace

TEST_F(WithoutBoundScheduler, ThreadAffinityCount) {
  auto affinity = marl::Thread::Affinity(
      {
          core(10),
          core(20),
          core(30),
          core(40),
      },
      allocator);
  EXPECT_EQ(affinity.count(), 4U);
}

TEST_F(WithoutBoundScheduler, ThreadAdd) {
  auto affinity = marl::Thread::Affinity(
      {
          core(10),
          core(20),
          core(30),
          core(40),
      },
      allocator);

  affinity
      .add(marl::Thread::Affinity(
          {
              core(25),
              core(15),
          },
          allocator))
      .add(marl::Thread::Affinity({core(35)}, allocator));

  EXPECT_EQ(affinity.count(), 7U);
  EXPECT_EQ(affinity[0], core(10));
  EXPECT_EQ(affinity[1], core(15));
  EXPECT_EQ(affinity[2], core(20));
  EXPECT_EQ(affinity[3], core(25));
  EXPECT_EQ(affinity[4], core(30));
  EXPECT_EQ(affinity[5], core(35));
  EXPECT_EQ(affinity[6], core(40));
}

TEST_F(WithoutBoundScheduler, ThreadRemove) {
  auto affinity = marl::Thread::Affinity(
      {
          core(10),
          core(20),
          core(30),
          core(40),
      },
      allocator);

  affinity
      .remove(marl::Thread::Affinity(
          {
              core(25),
              core(20),
          },
          allocator))
      .remove(marl::Thread::Affinity({core(40)}, allocator));

  EXPECT_EQ(affinity.count(), 2U);
  EXPECT_EQ(affinity[0], core(10));
  EXPECT_EQ(affinity[1], core(30));
}

TEST_F(WithoutBoundScheduler, ThreadAffinityAllCountNonzero) {
  auto affinity = marl::Thread::Affinity::all(allocator);
  if (marl::Thread::Affinity::supported) {
    EXPECT_NE(affinity.count(), 0U);
  } else {
    EXPECT_EQ(affinity.count(), 0U);
  }
}

TEST_F(WithoutBoundScheduler, ThreadAffinityFromVector) {
  marl::containers::vector<marl::Thread::Core, 32> cores(allocator);
  cores.push_back(core(10));
  cores.push_back(core(20));
  cores.push_back(core(30));
  cores.push_back(core(40));
  auto affinity = marl::Thread::Affinity(cores, allocator);
  EXPECT_EQ(affinity.count(), cores.size());
  EXPECT_EQ(affinity[0], core(10));
  EXPECT_EQ(affinity[1], core(20));
  EXPECT_EQ(affinity[2], core(30));
  EXPECT_EQ(affinity[3], core(40));
}

TEST_F(WithoutBoundScheduler, ThreadAffinityPolicyOneOf) {
  auto all = marl::Thread::Affinity(
      {
          core(10),
          core(20),
          core(30),
          core(40),
      },
      allocator);

  auto policy =
      marl::Thread::Affinity::Policy::oneOf(std::move(all), allocator);
  EXPECT_EQ(policy->get(0, allocator).count(), 1U);
  EXPECT_EQ(policy->get(0, allocator)[0].pthread.index, 10);
  EXPECT_EQ(policy->get(1, allocator).count(), 1U);
  EXPECT_EQ(policy->get(1, allocator)[0].pthread.index, 20);
  EXPECT_EQ(policy->get(2, allocator).count(), 1U);
  EXPECT_EQ(policy->get(2, allocator)[0].pthread.index, 30);
  EXPECT_EQ(policy->get(3, allocator).count(), 1U);
  EXPECT_EQ(policy->get(3, allocator)[0].pthread.index, 40);
}
