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

#include "marl/conditionvariable.h"
#include "marl/waitgroup.h"

#include "marl_test.h"

#include <condition_variable>

TEST_F(WithoutBoundScheduler, ConditionVariable) {
  bool trigger[3] = {false, false, false};
  bool signal[3] = {false, false, false};
  marl::mutex mutex;
  marl::ConditionVariable cv;

  std::thread thread([&] {
    for (int i = 0; i < 3; i++) {
      marl::lock lock(mutex);
      cv.wait(lock, [&] {
        EXPECT_TRUE(lock.owns_lock());
        return trigger[i];
      });
      EXPECT_TRUE(lock.owns_lock());
      signal[i] = true;
      cv.notify_one();
    }
  });

  ASSERT_FALSE(signal[0]);
  ASSERT_FALSE(signal[1]);
  ASSERT_FALSE(signal[2]);

  for (int i = 0; i < 3; i++) {
    {
      marl::lock lock(mutex);
      trigger[i] = true;
      cv.notify_one();
      cv.wait(lock, [&] {
        EXPECT_TRUE(lock.owns_lock());
        return signal[i];
      });
      EXPECT_TRUE(lock.owns_lock());
    }

    ASSERT_EQ(signal[0], 0 <= i);
    ASSERT_EQ(signal[1], 1 <= i);
    ASSERT_EQ(signal[2], 2 <= i);
  }

  thread.join();
}

TEST_P(WithBoundScheduler, ConditionVariable) {
  bool trigger[3] = {false, false, false};
  bool signal[3] = {false, false, false};
  marl::mutex mutex;
  marl::ConditionVariable cv;

  std::thread thread([&] {
    for (int i = 0; i < 3; i++) {
      marl::lock lock(mutex);
      cv.wait(lock, [&] {
        EXPECT_TRUE(lock.owns_lock());
        return trigger[i];
      });
      EXPECT_TRUE(lock.owns_lock());
      signal[i] = true;
      cv.notify_one();
    }
  });

  ASSERT_FALSE(signal[0]);
  ASSERT_FALSE(signal[1]);
  ASSERT_FALSE(signal[2]);

  for (int i = 0; i < 3; i++) {
    {
      marl::lock lock(mutex);
      trigger[i] = true;
      cv.notify_one();
      cv.wait(lock, [&] {
        EXPECT_TRUE(lock.owns_lock());
        return signal[i];
      });
      EXPECT_TRUE(lock.owns_lock());
    }

    ASSERT_EQ(signal[0], 0 <= i);
    ASSERT_EQ(signal[1], 1 <= i);
    ASSERT_EQ(signal[2], 2 <= i);
  }

  thread.join();
}

// ConditionVariableTimeouts spins up a whole lot of wait_fors(), unblocking
// some with timeouts and some with a notify, and then let's all the workers
// go to idle before repeating.
// This is testing to ensure that the scheduler handles timeouts correctly when
// they are early-unblocked, along with expected lock state.
TEST_P(WithBoundScheduler, ConditionVariableTimeouts) {
  for (int i = 0; i < 10; i++) {
    marl::mutex mutex;
    marl::ConditionVariable cv;
    bool signaled = false;  // guarded by mutex
    auto wg = marl::WaitGroup(100);
    for (int j = 0; j < 100; j++) {
      marl::schedule([=, &mutex, &cv, &signaled] {
        {
          marl::lock lock(mutex);
          cv.wait_for(lock, std::chrono::milliseconds(j), [&] {
            EXPECT_TRUE(lock.owns_lock());
            return signaled;
          });
          EXPECT_TRUE(lock.owns_lock());
        }
        // Ensure the mutex unlock happens *before* the wg.done() call,
        // otherwise the stack pointer may no longer be valid.
        wg.done();
      });
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    {
      marl::lock lock(mutex);
      signaled = true;
      cv.notify_all();
    }
    wg.wait();
  }
}
