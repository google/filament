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

// Wrappers around std::mutex and std::unique_lock that provide clang's
// Thread Safety Analysis annotations.
// See: https://clang.llvm.org/docs/ThreadSafetyAnalysis.html

#ifndef marl_mutex_h
#define marl_mutex_h

#include "export.h"
#include "tsa.h"

#include <condition_variable>
#include <mutex>

namespace marl {

// mutex is a wrapper around std::mutex that offers Thread Safety Analysis
// annotations.
// mutex also holds methods for performing std::condition_variable::wait() calls
// as these require a std::unique_lock<> which are unsupported by the TSA.
class CAPABILITY("mutex") mutex {
 public:
  MARL_NO_EXPORT inline void lock() ACQUIRE() { _.lock(); }

  MARL_NO_EXPORT inline void unlock() RELEASE() { _.unlock(); }

  MARL_NO_EXPORT inline bool try_lock() TRY_ACQUIRE(true) {
    return _.try_lock();
  }

  // wait_locked calls cv.wait() on this already locked mutex.
  template <typename Predicate>
  MARL_NO_EXPORT inline void wait_locked(std::condition_variable& cv,
                                         Predicate&& p) REQUIRES(this) {
    std::unique_lock<std::mutex> lock(_, std::adopt_lock);
    cv.wait(lock, std::forward<Predicate>(p));
    lock.release();  // Keep lock held.
  }

  // wait_until_locked calls cv.wait() on this already locked mutex.
  template <typename Predicate, typename Time>
  MARL_NO_EXPORT inline bool wait_until_locked(std::condition_variable& cv,
                                               Time&& time,
                                               Predicate&& p) REQUIRES(this) {
    std::unique_lock<std::mutex> lock(_, std::adopt_lock);
    auto res = cv.wait_until(lock, std::forward<Time>(time),
                             std::forward<Predicate>(p));
    lock.release();  // Keep lock held.
    return res;
  }

 private:
  friend class lock;
  std::mutex _;
};

// lock is a RAII lock helper that offers Thread Safety Analysis annotations.
// lock also holds methods for performing std::condition_variable::wait()
// calls as these require a std::unique_lock<> which are unsupported by the TSA.
class SCOPED_CAPABILITY lock {
 public:
  inline lock(mutex& m) ACQUIRE(m) : _(m._) {}
  inline ~lock() RELEASE() = default;

  // wait calls cv.wait() on this lock.
  template <typename Predicate>
  inline void wait(std::condition_variable& cv, Predicate&& p) {
    cv.wait(_, std::forward<Predicate>(p));
  }

  // wait_until calls cv.wait() on this lock.
  template <typename Predicate, typename Time>
  inline bool wait_until(std::condition_variable& cv,
                         Time&& time,
                         Predicate&& p) {
    return cv.wait_until(_, std::forward<Time>(time),
                         std::forward<Predicate>(p));
  }

  inline bool owns_lock() const { return _.owns_lock(); }

  // lock_no_tsa locks the mutex outside of the visiblity of the thread
  // safety analysis. Use with caution.
  inline void lock_no_tsa() { _.lock(); }

  // unlock_no_tsa unlocks the mutex outside of the visiblity of the thread
  // safety analysis. Use with caution.
  inline void unlock_no_tsa() { _.unlock(); }

 private:
  std::unique_lock<std::mutex> _;
};

}  // namespace marl

#endif  // marl_mutex_h
