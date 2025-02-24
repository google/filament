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

#include "marl/event.h"
#include "marl/defer.h"
#include "marl/waitgroup.h"

#include "marl_test.h"

#include <array>

namespace std {
namespace chrono {
template <typename Rep, typename Period>
std::ostream& operator<<(std::ostream& os, const duration<Rep, Period>& d) {
  return os << chrono::duration_cast<chrono::microseconds>(d).count() << "ms";
}
}  // namespace chrono
}  // namespace std

TEST_P(WithBoundScheduler, EventIsSignalled) {
  for (auto mode : {marl::Event::Mode::Manual, marl::Event::Mode::Auto}) {
    auto event = marl::Event(mode);
    ASSERT_EQ(event.isSignalled(), false);
    event.signal();
    ASSERT_EQ(event.isSignalled(), true);
    ASSERT_EQ(event.isSignalled(), true);
    event.clear();
    ASSERT_EQ(event.isSignalled(), false);
  }
}

TEST_P(WithBoundScheduler, EventAutoTest) {
  auto event = marl::Event(marl::Event::Mode::Auto);
  ASSERT_EQ(event.test(), false);
  event.signal();
  ASSERT_EQ(event.test(), true);
  ASSERT_EQ(event.test(), false);
}

TEST_P(WithBoundScheduler, EventManualTest) {
  auto event = marl::Event(marl::Event::Mode::Manual);
  ASSERT_EQ(event.test(), false);
  event.signal();
  ASSERT_EQ(event.test(), true);
  ASSERT_EQ(event.test(), true);
}

TEST_P(WithBoundScheduler, EventAutoWait) {
  std::atomic<int> counter = {0};
  auto event = marl::Event(marl::Event::Mode::Auto);
  auto done = marl::Event(marl::Event::Mode::Auto);

  for (int i = 0; i < 3; i++) {
    marl::schedule([=, &counter] {
      event.wait();
      counter++;
      done.signal();
    });
  }

  ASSERT_EQ(counter.load(), 0);
  event.signal();
  done.wait();
  ASSERT_EQ(counter.load(), 1);
  event.signal();
  done.wait();
  ASSERT_EQ(counter.load(), 2);
  event.signal();
  done.wait();
  ASSERT_EQ(counter.load(), 3);
}

TEST_P(WithBoundScheduler, EventManualWait) {
  std::atomic<int> counter = {0};
  auto event = marl::Event(marl::Event::Mode::Manual);
  auto wg = marl::WaitGroup(3);
  for (int i = 0; i < 3; i++) {
    marl::schedule([=, &counter] {
      event.wait();
      counter++;
      wg.done();
    });
  }
  event.signal();
  wg.wait();
  ASSERT_EQ(counter.load(), 3);
}

TEST_P(WithBoundScheduler, EventSequence) {
  for (auto mode : {marl::Event::Mode::Manual, marl::Event::Mode::Auto}) {
    std::string sequence;
    auto eventA = marl::Event(mode);
    auto eventB = marl::Event(mode);
    auto eventC = marl::Event(mode);
    auto done = marl::Event(mode);
    marl::schedule([=, &sequence] {
      eventB.wait();
      sequence += "B";
      eventC.signal();
    });
    marl::schedule([=, &sequence] {
      eventA.wait();
      sequence += "A";
      eventB.signal();
    });
    marl::schedule([=, &sequence] {
      eventC.wait();
      sequence += "C";
      done.signal();
    });
    ASSERT_EQ(sequence, "");
    eventA.signal();
    done.wait();
    ASSERT_EQ(sequence, "ABC");
  }
}

TEST_P(WithBoundScheduler, EventWaitForUnblocked) {
  auto event = marl::Event(marl::Event::Mode::Manual);
  auto wg = marl::WaitGroup(1000);
  for (int i = 0; i < 1000; i++) {
    marl::schedule([=] {
      defer(wg.done());
      auto duration = std::chrono::seconds(10);
      event.wait_for(duration);
    });
  }
  event.signal();  // unblock
  wg.wait();
}

TEST_P(WithBoundScheduler, EventWaitForTimeTaken) {
  auto event = marl::Event(marl::Event::Mode::Auto);
  auto wg = marl::WaitGroup(1000);
  for (int i = 0; i < 1000; i++) {
    marl::schedule([=] {
      defer(wg.done());
      auto duration = std::chrono::milliseconds(10);
      auto start = std::chrono::system_clock::now();
      auto triggered = event.wait_for(duration);
      auto end = std::chrono::system_clock::now();
      ASSERT_FALSE(triggered);
      ASSERT_GE(end - start, duration);
    });
  }
  wg.wait();
}

TEST_P(WithBoundScheduler, EventWaitUntilUnblocked) {
  auto event = marl::Event(marl::Event::Mode::Manual);
  auto wg = marl::WaitGroup(1000);
  for (int i = 0; i < 1000; i++) {
    marl::schedule([=] {
      defer(wg.done());
      auto duration = std::chrono::seconds(10);
      auto start = std::chrono::system_clock::now();
      event.wait_until(start + duration);
    });
  }
  event.signal();  // unblock
  wg.wait();
}

TEST_P(WithBoundScheduler, EventWaitUntilTimeTaken) {
  auto event = marl::Event(marl::Event::Mode::Auto);
  auto wg = marl::WaitGroup(1000);
  for (int i = 0; i < 1000; i++) {
    marl::schedule([=] {
      defer(wg.done());
      auto duration = std::chrono::milliseconds(10);
      auto start = std::chrono::system_clock::now();
      auto triggered = event.wait_until(start + duration);
      auto end = std::chrono::system_clock::now();
      ASSERT_FALSE(triggered);
      ASSERT_GE(end - start, duration);
    });
  }
  wg.wait();
}

// EventWaitStressTest spins up a whole lot of wait_fors(), unblocking some
// with timeouts and some with an event signal, and then let's all the workers
// go to idle before repeating.
// This is testing to ensure that the scheduler handles timeouts correctly when
// they are early-unblocked. Specifically, this is to test that fibers are
// not double-placed into the idle or working lists.
TEST_P(WithBoundScheduler, EventWaitStressTest) {
  auto event = marl::Event(marl::Event::Mode::Manual);
  for (int i = 0; i < 10; i++) {
    auto wg = marl::WaitGroup(100);
    for (int j = 0; j < 100; j++) {
      marl::schedule([=] {
        defer(wg.done());
        event.wait_for(std::chrono::milliseconds(j));
      });
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    event.signal();  // unblock
    wg.wait();
  }
}

TEST_P(WithBoundScheduler, EventAny) {
  for (int i = 0; i < 3; i++) {
    std::array<marl::Event, 3> events = {
        marl::Event(marl::Event::Mode::Auto),
        marl::Event(marl::Event::Mode::Auto),
        marl::Event(marl::Event::Mode::Auto),
    };
    auto any = marl::Event::any(events.begin(), events.end());
    events[i].signal();
    ASSERT_TRUE(any.isSignalled());
  }
}