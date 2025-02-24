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

#include "marl/containers.h"
#include "marl/defer.h"
#include "marl/event.h"
#include "marl/waitgroup.h"

#include <atomic>

TEST_F(WithoutBoundScheduler, SchedulerConstructAndDestruct) {
  auto scheduler = std::unique_ptr<marl::Scheduler>(
      new marl::Scheduler(marl::Scheduler::Config()));
}

TEST_F(WithoutBoundScheduler, SchedulerBindGetUnbind) {
  auto scheduler = std::unique_ptr<marl::Scheduler>(
      new marl::Scheduler(marl::Scheduler::Config()));
  scheduler->bind();
  auto got = marl::Scheduler::get();
  ASSERT_EQ(scheduler.get(), got);
  scheduler->unbind();
  got = marl::Scheduler::get();
  ASSERT_EQ(got, nullptr);
}

TEST_F(WithoutBoundScheduler, CheckConfig) {
  marl::Scheduler::Config cfg;
  cfg.setAllocator(allocator).setWorkerThreadCount(10);

  auto scheduler = std::unique_ptr<marl::Scheduler>(new marl::Scheduler(cfg));

  auto gotCfg = scheduler->config();
  ASSERT_EQ(gotCfg.allocator, allocator);
  ASSERT_EQ(gotCfg.workerThread.count, 10);
}

TEST_P(WithBoundScheduler, DestructWithPendingTasks) {
  std::atomic<int> counter = {0};
  for (int i = 0; i < 1000; i++) {
    marl::schedule([&] { counter++; });
  }

  auto scheduler = marl::Scheduler::get();
  scheduler->unbind();
  delete scheduler;

  // All scheduled tasks should be completed before the scheduler is destructed.
  ASSERT_EQ(counter.load(), 1000);

  // Rebind a new scheduler so WithBoundScheduler::TearDown() is happy.
  (new marl::Scheduler(marl::Scheduler::Config()))->bind();
}

TEST_P(WithBoundScheduler, DestructWithPendingFibers) {
  std::atomic<int> counter = {0};

  marl::WaitGroup wg(1);
  for (int i = 0; i < 1000; i++) {
    marl::schedule([&] {
      wg.wait();
      counter++;
    });
  }

  // Schedule a task to unblock all the tasks scheduled above.
  // We assume that some of these tasks will not finish before the scheduler
  // destruction logic kicks in.
  marl::schedule([=] {
    wg.done();  // Ready, steady, go...
  });

  auto scheduler = marl::Scheduler::get();
  scheduler->unbind();
  delete scheduler;

  // All scheduled tasks should be completed before the scheduler is destructed.
  ASSERT_EQ(counter.load(), 1000);

  // Rebind a new scheduler so WithBoundScheduler::TearDown() is happy.
  (new marl::Scheduler(marl::Scheduler::Config()))->bind();
}

TEST_P(WithBoundScheduler, ScheduleWithArgs) {
  std::string got;
  marl::WaitGroup wg(1);
  marl::schedule(
      [wg, &got](std::string s, int i, bool b) {
        got = "s: '" + s + "', i: " + std::to_string(i) +
              ", b: " + (b ? "true" : "false");
        wg.done();
      },
      "a string", 42, true);
  wg.wait();
  ASSERT_EQ(got, "s: 'a string', i: 42, b: true");
}

TEST_P(WithBoundScheduler, FibersResumeOnSameThread) {
  marl::WaitGroup fence(1);
  marl::WaitGroup wg(1000);
  for (int i = 0; i < 1000; i++) {
    marl::schedule([=] {
      auto threadID = std::this_thread::get_id();
      fence.wait();
      ASSERT_EQ(threadID, std::this_thread::get_id());
      wg.done();
    });
  }
  // just to try and get some tasks to yield.
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  fence.done();
  wg.wait();
}

TEST_P(WithBoundScheduler, FibersResumeOnSameStdThread) {
  auto scheduler = marl::Scheduler::get();

  // on 32-bit OSs, excessive numbers of threads can run out of address space.
  constexpr auto num_threads = sizeof(void*) > 4 ? 1000 : 100;

  marl::WaitGroup fence(1);
  marl::WaitGroup wg(num_threads);

  marl::containers::vector<std::thread, 32> threads;
  for (int i = 0; i < num_threads; i++) {
    threads.emplace_back(std::thread([=] {
      scheduler->bind();
      defer(scheduler->unbind());

      auto threadID = std::this_thread::get_id();
      fence.wait();
      ASSERT_EQ(threadID, std::this_thread::get_id());
      wg.done();
    }));
  }
  // just to try and get some tasks to yield.
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  fence.done();
  wg.wait();

  for (auto& thread : threads) {
    thread.join();
  }
}

TEST_F(WithoutBoundScheduler, TasksOnlyScheduledOnWorkerThreads) {
  marl::Scheduler::Config cfg;
  cfg.setWorkerThreadCount(8);

  auto scheduler = std::unique_ptr<marl::Scheduler>(new marl::Scheduler(cfg));
  scheduler->bind();
  defer(scheduler->unbind());

  std::mutex mutex;
  marl::containers::unordered_set<std::thread::id> threads(allocator);
  marl::WaitGroup wg;
  for (int i = 0; i < 10000; i++) {
    wg.add(1);
    marl::schedule([&mutex, &threads, wg] {
      defer(wg.done());
      std::unique_lock<std::mutex> lock(mutex);
      threads.emplace(std::this_thread::get_id());
    });
  }
  wg.wait();

  ASSERT_LE(threads.size(), 8U);
  ASSERT_EQ(threads.count(std::this_thread::get_id()), 0U);
}

// Test that a marl::Scheduler *with dedicated worker threads* can be used
// without first binding to the scheduling thread.
TEST_F(WithoutBoundScheduler, ScheduleMTWWithNoBind) {
  marl::Scheduler::Config cfg;
  cfg.setWorkerThreadCount(8);
  auto scheduler = std::unique_ptr<marl::Scheduler>(new marl::Scheduler(cfg));

  marl::WaitGroup wg;
  for (int i = 0; i < 100; i++) {
    wg.add(1);

    marl::Event event;
    scheduler->enqueue(marl::Task([event, wg] {
      event.wait();  // Test that tasks can wait on other tasks.
      wg.done();
    }));

    scheduler->enqueue(marl::Task([event, &scheduler] {
      // Despite the main thread never binding the scheduler, the scheduler
      // should be automatically bound to worker threads.
      ASSERT_EQ(marl::Scheduler::get(), scheduler.get());

      event.signal();
    }));
  }

  // As the scheduler has not been bound to the main thread, the wait() call
  // here will block **without** fiber yielding.
  wg.wait();
}

// Test that a marl::Scheduler *without dedicated worker threads* cannot be used
// without first binding to the scheduling thread.
TEST_F(WithoutBoundScheduler, ScheduleSTWWithNoBind) {
  marl::Scheduler::Config cfg;
  auto scheduler = std::unique_ptr<marl::Scheduler>(new marl::Scheduler(cfg));

#if MARL_DEBUG_ENABLED && GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(scheduler->enqueue(marl::Task([] {})),
               "Did you forget to call marl::Scheduler::bind");
#elif !MARL_DEBUG_ENABLED
  scheduler->enqueue(marl::Task([] { FAIL() << "Should not be called"; }));
#endif
}
