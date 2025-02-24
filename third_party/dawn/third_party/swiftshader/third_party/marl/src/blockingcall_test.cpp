// Copyright 2019 The Marl Authors
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

#include "marl/blockingcall.h"

#include "marl/defer.h"

#include "marl_test.h"

#include <mutex>

TEST_P(WithBoundScheduler, BlockingCallVoidReturn) {
  auto mutex = std::make_shared<std::mutex>();
  mutex->lock();

  marl::WaitGroup wg(100);
  for (int i = 0; i < 100; i++) {
    marl::schedule([=] {
      defer(wg.done());
      marl::blocking_call([=] {
        mutex->lock();
        defer(mutex->unlock());
      });
    });
  }

  mutex->unlock();
  wg.wait();
}

TEST_P(WithBoundScheduler, BlockingCallIntReturn) {
  auto mutex = std::make_shared<std::mutex>();
  mutex->lock();

  marl::WaitGroup wg(100);
  std::atomic<int> n = {0};
  for (int i = 0; i < 100; i++) {
    marl::schedule([=, &n] {
      defer(wg.done());
      n += marl::blocking_call([=] {
        mutex->lock();
        defer(mutex->unlock());
        return i;
      });
    });
  }

  mutex->unlock();
  wg.wait();

  ASSERT_EQ(n.load(), 4950);
}

TEST_P(WithBoundScheduler, BlockingCallSchedulesTask) {
  marl::WaitGroup wg(1);
  marl::schedule([=] {
    marl::blocking_call([=] { marl::schedule([=] { wg.done(); }); });
  });
  wg.wait();
}
