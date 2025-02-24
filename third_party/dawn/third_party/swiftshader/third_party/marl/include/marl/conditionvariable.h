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

#ifndef marl_condition_variable_h
#define marl_condition_variable_h

#include "containers.h"
#include "debug.h"
#include "memory.h"
#include "mutex.h"
#include "scheduler.h"
#include "tsa.h"

#include <atomic>
#include <condition_variable>

namespace marl {

// ConditionVariable is a synchronization primitive that can be used to block
// one or more fibers or threads, until another fiber or thread modifies a
// shared variable (the condition) and notifies the ConditionVariable.
//
// If the ConditionVariable is blocked on a thread with a Scheduler bound, the
// thread will work on other tasks until the ConditionVariable is unblocked.
class ConditionVariable {
 public:
  MARL_NO_EXPORT inline ConditionVariable(
      Allocator* allocator = Allocator::Default);

  // notify_one() notifies and potentially unblocks one waiting fiber or thread.
  MARL_NO_EXPORT inline void notify_one();

  // notify_all() notifies and potentially unblocks all waiting fibers and/or
  // threads.
  MARL_NO_EXPORT inline void notify_all();

  // wait() blocks the current fiber or thread until the predicate is satisfied
  // and the ConditionVariable is notified.
  template <typename Predicate>
  MARL_NO_EXPORT inline void wait(marl::lock& lock, Predicate&& pred);

  // wait_for() blocks the current fiber or thread until the predicate is
  // satisfied, and the ConditionVariable is notified, or the timeout has been
  // reached. Returns false if pred still evaluates to false after the timeout
  // has been reached, otherwise true.
  template <typename Rep, typename Period, typename Predicate>
  MARL_NO_EXPORT inline bool wait_for(
      marl::lock& lock,
      const std::chrono::duration<Rep, Period>& duration,
      Predicate&& pred);

  // wait_until() blocks the current fiber or thread until the predicate is
  // satisfied, and the ConditionVariable is notified, or the timeout has been
  // reached. Returns false if pred still evaluates to false after the timeout
  // has been reached, otherwise true.
  template <typename Clock, typename Duration, typename Predicate>
  MARL_NO_EXPORT inline bool wait_until(
      marl::lock& lock,
      const std::chrono::time_point<Clock, Duration>& timeout,
      Predicate&& pred);

 private:
  ConditionVariable(const ConditionVariable&) = delete;
  ConditionVariable(ConditionVariable&&) = delete;
  ConditionVariable& operator=(const ConditionVariable&) = delete;
  ConditionVariable& operator=(ConditionVariable&&) = delete;

  marl::mutex mutex;
  containers::list<Scheduler::Fiber*> waiting;
  std::condition_variable condition;
  std::atomic<int> numWaiting = {0};
  std::atomic<int> numWaitingOnCondition = {0};
};

ConditionVariable::ConditionVariable(
    Allocator* allocator /* = Allocator::Default */)
    : waiting(allocator) {}

void ConditionVariable::notify_one() {
  if (numWaiting == 0) {
    return;
  }
  {
    marl::lock lock(mutex);
    if (waiting.size() > 0) {
      (*waiting.begin())->notify();  // Only wake one fiber.
      return;
    }
  }
  if (numWaitingOnCondition > 0) {
    condition.notify_one();
  }
}

void ConditionVariable::notify_all() {
  if (numWaiting == 0) {
    return;
  }
  {
    marl::lock lock(mutex);
    for (auto fiber : waiting) {
      fiber->notify();
    }
  }
  if (numWaitingOnCondition > 0) {
    condition.notify_all();
  }
}

template <typename Predicate>
void ConditionVariable::wait(marl::lock& lock, Predicate&& pred) {
  if (pred()) {
    return;
  }
  numWaiting++;
  if (auto fiber = Scheduler::Fiber::current()) {
    // Currently executing on a scheduler fiber.
    // Yield to let other tasks run that can unblock this fiber.
    mutex.lock();
    auto it = waiting.emplace_front(fiber);
    mutex.unlock();

    fiber->wait(lock, pred);

    mutex.lock();
    waiting.erase(it);
    mutex.unlock();
  } else {
    // Currently running outside of the scheduler.
    // Delegate to the std::condition_variable.
    numWaitingOnCondition++;
    lock.wait(condition, pred);
    numWaitingOnCondition--;
  }
  numWaiting--;
}

template <typename Rep, typename Period, typename Predicate>
bool ConditionVariable::wait_for(
    marl::lock& lock,
    const std::chrono::duration<Rep, Period>& duration,
    Predicate&& pred) {
  return wait_until(lock, std::chrono::system_clock::now() + duration, pred);
}

template <typename Clock, typename Duration, typename Predicate>
bool ConditionVariable::wait_until(
    marl::lock& lock,
    const std::chrono::time_point<Clock, Duration>& timeout,
    Predicate&& pred) {
  if (pred()) {
    return true;
  }

  if (auto fiber = Scheduler::Fiber::current()) {
    numWaiting++;

    // Currently executing on a scheduler fiber.
    // Yield to let other tasks run that can unblock this fiber.
    mutex.lock();
    auto it = waiting.emplace_front(fiber);
    mutex.unlock();

    auto res = fiber->wait(lock, timeout, pred);

    mutex.lock();
    waiting.erase(it);
    mutex.unlock();

    numWaiting--;
    return res;
  }

  // Currently running outside of the scheduler.
  // Delegate to the std::condition_variable.
  numWaiting++;
  numWaitingOnCondition++;
  auto res = lock.wait_until(condition, timeout, pred);
  numWaitingOnCondition--;
  numWaiting--;
  return res;
}

}  // namespace marl

#endif  // marl_condition_variable_h
