// Copyright 2020 The Marl Authors.
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

// This file contains a number of benchmarks that do not use marl.
// They exist to compare marl's performance against other simple scheduler
// approaches.

#include "marl_bench.h"

#include "benchmark/benchmark.h"

#include <mutex>
#include <queue>
#include <thread>

namespace {

// Event provides a basic wait-and-signal synchronization primitive.
class Event {
 public:
  // wait blocks until the event is fired.
  void wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [&] { return signalled_; });
  }

  // signal signals the Event, unblocking any calls to wait.
  void signal() {
    std::unique_lock<std::mutex> lock(mutex_);
    signalled_ = true;
    cv_.notify_all();
  }

 private:
  std::condition_variable cv_;
  std::mutex mutex_;
  bool signalled_ = false;
};

}  // anonymous namespace

// A simple multi-thread, single-queue task executor that shares a single mutex
// across N threads. This implementation suffers from lock contention.
static void SingleQueueTaskExecutor(benchmark::State& state) {
  using Task = std::function<uint32_t(uint32_t)>;

  auto const numTasks = Schedule::numTasks(state);
  auto const numThreads = Schedule::numThreads(state);

  for (auto _ : state) {
    state.PauseTiming();

    std::mutex mutex;
    // Set everything up with the mutex locked to prevent the threads from
    // performing work while the timing is paused.
    mutex.lock();

    // Set up the tasks.
    std::queue<Task> tasks;
    for (int i = 0; i < numTasks; i++) {
      tasks.push(Schedule::doSomeWork);
    }

    auto taskRunner = [&] {
      while (true) {
        Task task;

        // Take the next task.
        // Note that this lock is likely to block while waiting for other
        // threads.
        mutex.lock();
        if (tasks.size() > 0) {
          task = tasks.front();
          tasks.pop();
        }
        mutex.unlock();

        if (task) {
          task(123);
        } else {
          return;  // done.
        }
      }
    };

    // Set up the threads.
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; i++) {
      threads.emplace_back(std::thread(taskRunner));
    }

    state.ResumeTiming();
    mutex.unlock();  // Go threads, go!

    if (numThreads > 0) {
      // Wait for all threads to finish.
      for (auto& thread : threads) {
        thread.join();
      }
    } else {
      // Single-threaded test - just run the worker.
      taskRunner();
    }
  }
}
BENCHMARK(SingleQueueTaskExecutor)->Apply(Schedule::args);

// A simple multi-thread, multi-queue task executor that avoids lock contention.
// Tasks queues are evenly balanced, and each should take an equal amount of
// time to execute.
static void MultiQueueTaskExecutor(benchmark::State& state) {
  using Task = std::function<uint32_t(uint32_t)>;
  using TaskQueue = std::vector<Task>;

  auto const numTasks = Schedule::numTasks(state);
  auto const numThreads = Schedule::numThreads(state);
  auto const numQueues = std::max(numThreads, 1);

  // Set up the tasks queues.
  std::vector<TaskQueue> taskQueues(numQueues);
  for (int i = 0; i < numTasks; i++) {
    taskQueues[i % numQueues].emplace_back(Schedule::doSomeWork);
  }

  for (auto _ : state) {
    if (numThreads > 0) {
      state.PauseTiming();
      Event start;

      // Set up the threads.
      std::vector<std::thread> threads;
      for (int i = 0; i < numThreads; i++) {
        threads.emplace_back(std::thread([&, i] {
          start.wait();
          for (auto& task : taskQueues[i]) {
            task(123);
          }
        }));
      }

      state.ResumeTiming();
      start.signal();

      // Wait for all threads to finish.
      for (auto& thread : threads) {
        thread.join();
      }
    } else {
      // Single-threaded test - just run the tasks.
      for (auto& task : taskQueues[0]) {
        task(123);
      }
    }
  }
}
BENCHMARK(MultiQueueTaskExecutor)->Apply(Schedule::args);