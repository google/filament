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

#include "marl/waitgroup.h"

TEST_F(WithoutBoundScheduler, WaitGroupDone) {
  marl::WaitGroup wg(2);  // Should not require a scheduler.
  wg.done();
  wg.done();
}

#if MARL_DEBUG_ENABLED && GTEST_HAS_DEATH_TEST
TEST_F(WithoutBoundScheduler, WaitGroupDoneTooMany) {
  marl::WaitGroup wg(2);  // Should not require a scheduler.
  wg.done();
  wg.done();
  EXPECT_DEATH(wg.done(), "done\\(\\) called too many times");
}
#endif  // MARL_DEBUG_ENABLED && GTEST_HAS_DEATH_TEST

TEST_P(WithBoundScheduler, WaitGroup_OneTask) {
  marl::WaitGroup wg(1);
  std::atomic<int> counter = {0};
  marl::schedule([&counter, wg] {
    counter++;
    wg.done();
  });
  wg.wait();
  ASSERT_EQ(counter.load(), 1);
}

TEST_P(WithBoundScheduler, WaitGroup_10Tasks) {
  marl::WaitGroup wg(10);
  std::atomic<int> counter = {0};
  for (int i = 0; i < 10; i++) {
    marl::schedule([&counter, wg] {
      counter++;
      wg.done();
    });
  }
  wg.wait();
  ASSERT_EQ(counter.load(), 10);
}
