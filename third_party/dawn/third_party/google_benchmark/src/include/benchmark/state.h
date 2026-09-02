// Copyright 2015 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef BENCHMARK_STATE_H_
#define BENCHMARK_STATE_H_

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4251 4324)
#endif

#include <cassert>
#include <string>
#include <vector>

#include "benchmark/counter.h"
#include "benchmark/macros.h"
#include "benchmark/statistics.h"
#include "benchmark/types.h"

namespace benchmark {

namespace internal {
class BenchmarkInstance;
class ThreadTimer;
class ThreadManager;
class PerfCountersMeasurement;
}  // namespace internal

class ProfilerManager;

class BENCHMARK_EXPORT BENCHMARK_INTERNAL_CACHELINE_ALIGNED State {
 public:
  struct StateIterator;
  friend struct StateIterator;

  inline BENCHMARK_ALWAYS_INLINE StateIterator begin();
  inline BENCHMARK_ALWAYS_INLINE StateIterator end();

  inline bool KeepRunning();

  inline bool KeepRunningBatch(IterationCount n);

  void PauseTiming();

  void ResumeTiming();

  void SkipWithMessage(const std::string& msg);

  void SkipWithError(const std::string& msg);

  bool skipped() const { return internal::NotSkipped != skipped_; }

  bool error_occurred() const { return internal::SkippedWithError == skipped_; }

  void SetIterationTime(double seconds);

  BENCHMARK_ALWAYS_INLINE
  void SetBytesProcessed(int64_t bytes) {
    counters["bytes_per_second"] =
        Counter(static_cast<double>(bytes), Counter::kIsRate, Counter::kIs1024);
  }

  BENCHMARK_ALWAYS_INLINE
  int64_t bytes_processed() const {
    if (counters.find("bytes_per_second") != counters.end())
      return static_cast<int64_t>(counters.at("bytes_per_second"));
    return 0;
  }

  BENCHMARK_ALWAYS_INLINE
  void SetComplexityN(ComplexityN complexity_n) {
    complexity_n_ = complexity_n;
  }

  BENCHMARK_ALWAYS_INLINE
  ComplexityN complexity_length_n() const { return complexity_n_; }

  BENCHMARK_ALWAYS_INLINE
  void SetItemsProcessed(int64_t items) {
    counters["items_per_second"] =
        Counter(static_cast<double>(items), benchmark::Counter::kIsRate);
  }

  BENCHMARK_ALWAYS_INLINE
  int64_t items_processed() const {
    if (counters.find("items_per_second") != counters.end())
      return static_cast<int64_t>(counters.at("items_per_second"));
    return 0;
  }

  void SetLabel(const std::string& label);

  BENCHMARK_ALWAYS_INLINE
  int64_t range(std::size_t pos = 0) const {
    assert(range_.size() > pos);
    return range_[pos];
  }

  BENCHMARK_DEPRECATED_MSG("use 'range(0)' instead")
  int64_t range_x() const { return range(0); }

  BENCHMARK_DEPRECATED_MSG("use 'range(1)' instead")
  int64_t range_y() const { return range(1); }

  BENCHMARK_ALWAYS_INLINE
  int threads() const { return threads_; }

  BENCHMARK_ALWAYS_INLINE
  int thread_index() const { return thread_index_; }

  BENCHMARK_ALWAYS_INLINE
  IterationCount iterations() const {
    if (BENCHMARK_BUILTIN_EXPECT(!started_, false)) {
      return 0;
    }
    return max_iterations - total_iterations_ + batch_leftover_;
  }

  BENCHMARK_ALWAYS_INLINE
  std::string name() const { return name_; }

  size_t range_size() const { return range_.size(); }

 private:
  IterationCount total_iterations_;

  IterationCount batch_leftover_;

 public:
  const IterationCount max_iterations;

 private:
  bool started_;
  bool finished_;
  internal::Skipped skipped_;

  std::vector<int64_t> range_;

  ComplexityN complexity_n_;

 public:
  UserCounters counters;

 private:
  State(std::string name, IterationCount max_iters,
        const std::vector<int64_t>& ranges, int thread_i, int n_threads,
        internal::ThreadTimer* timer, internal::ThreadManager* manager,
        internal::PerfCountersMeasurement* perf_counters_measurement,
        ProfilerManager* profiler_manager);

  void StartKeepRunning();
  inline bool KeepRunningInternal(IterationCount n, bool is_batch);
  void FinishKeepRunning();

  const std::string name_;
  const int thread_index_;
  const int threads_;

  internal::ThreadTimer* const timer_;
  internal::ThreadManager* const manager_;
  internal::PerfCountersMeasurement* const perf_counters_measurement_;
  ProfilerManager* const profiler_manager_;

  friend class internal::BenchmarkInstance;
};

inline BENCHMARK_ALWAYS_INLINE bool State::KeepRunning() {
  return KeepRunningInternal(1, /*is_batch=*/false);
}

inline BENCHMARK_ALWAYS_INLINE bool State::KeepRunningBatch(IterationCount n) {
  return KeepRunningInternal(n, /*is_batch=*/true);
}

inline BENCHMARK_ALWAYS_INLINE bool State::KeepRunningInternal(IterationCount n,
                                                               bool is_batch) {
  assert(n > 0);
  assert(is_batch || n == 1);
  if (BENCHMARK_BUILTIN_EXPECT(total_iterations_ >= n, true)) {
    total_iterations_ -= n;
    return true;
  }
  if (!started_) {
    StartKeepRunning();
    if (!skipped() && total_iterations_ >= n) {
      total_iterations_ -= n;
      return true;
    }
  }
  if (is_batch && total_iterations_ != 0) {
    batch_leftover_ = n - total_iterations_;
    total_iterations_ = 0;
    return true;
  }
  FinishKeepRunning();
  return false;
}

struct State::StateIterator {
  struct BENCHMARK_UNUSED Value {};
  typedef std::forward_iterator_tag iterator_category;
  typedef Value value_type;
  typedef Value reference;
  typedef Value pointer;
  typedef std::ptrdiff_t difference_type;

 private:
  friend class State;
  BENCHMARK_ALWAYS_INLINE
  StateIterator() : cached_(0), parent_() {}

  BENCHMARK_ALWAYS_INLINE
  explicit StateIterator(State* st)
      : cached_(st->skipped() ? 0 : st->max_iterations), parent_(st) {}

 public:
  BENCHMARK_ALWAYS_INLINE
  Value operator*() const { return Value(); }

  BENCHMARK_ALWAYS_INLINE
  StateIterator& operator++() {
    assert(cached_ > 0);
    --cached_;
    return *this;
  }

  BENCHMARK_ALWAYS_INLINE
  bool operator!=(StateIterator const&) const {
    if (BENCHMARK_BUILTIN_EXPECT(cached_ != 0, true)) return true;
    parent_->FinishKeepRunning();
    return false;
  }

 private:
  IterationCount cached_;
  State* const parent_;
};

inline BENCHMARK_ALWAYS_INLINE State::StateIterator State::begin() {
  return StateIterator(this);
}
inline BENCHMARK_ALWAYS_INLINE State::StateIterator State::end() {
  StartKeepRunning();
  return StateIterator();
}

class ScopedPauseTiming {
 public:
  explicit ScopedPauseTiming(State& state) : state_(state) {
    state_.PauseTiming();
  }
  ~ScopedPauseTiming() { state_.ResumeTiming(); }

  ScopedPauseTiming(const ScopedPauseTiming&) = delete;
  void operator=(const ScopedPauseTiming&) = delete;

  ScopedPauseTiming(ScopedPauseTiming&&) = delete;
  void operator=(ScopedPauseTiming&&) = delete;

 private:
  State& state_;
};

}  // namespace benchmark

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif  // BENCHMARK_STATE_H_
