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

#ifndef BENCHMARK_BENCHMARK_API_H_
#define BENCHMARK_BENCHMARK_API_H_

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "benchmark/counter.h"
#include "benchmark/macros.h"
#include "benchmark/state.h"
#include "benchmark/statistics.h"
#include "benchmark/types.h"

namespace benchmark {

const char kDefaultMinTimeStr[] = "0.5s";

BENCHMARK_EXPORT void MaybeReenterWithoutASLR(int, char**);

BENCHMARK_EXPORT std::string GetBenchmarkVersion();

BENCHMARK_EXPORT void PrintDefaultHelp();

BENCHMARK_EXPORT void Initialize(int* argc, char** argv,
                                 void (*HelperPrintf)() = PrintDefaultHelp);
BENCHMARK_EXPORT void Shutdown();

BENCHMARK_EXPORT bool ReportUnrecognizedArguments(int argc, char** argv);

BENCHMARK_EXPORT std::string GetBenchmarkFilter();

BENCHMARK_EXPORT void SetBenchmarkFilter(std::string value);

BENCHMARK_EXPORT int32_t GetBenchmarkVerbosity();

BENCHMARK_EXPORT BenchmarkReporter* CreateDefaultDisplayReporter();

BENCHMARK_EXPORT size_t RunSpecifiedBenchmarks();
BENCHMARK_EXPORT size_t RunSpecifiedBenchmarks(std::string spec);

BENCHMARK_EXPORT size_t
RunSpecifiedBenchmarks(BenchmarkReporter* display_reporter);
BENCHMARK_EXPORT size_t
RunSpecifiedBenchmarks(BenchmarkReporter* display_reporter, std::string spec);

BENCHMARK_EXPORT size_t RunSpecifiedBenchmarks(
    BenchmarkReporter* display_reporter, BenchmarkReporter* file_reporter);
BENCHMARK_EXPORT size_t
RunSpecifiedBenchmarks(BenchmarkReporter* display_reporter,
                       BenchmarkReporter* file_reporter, std::string spec);

BENCHMARK_EXPORT TimeUnit GetDefaultTimeUnit();

BENCHMARK_EXPORT void SetDefaultTimeUnit(TimeUnit unit);

BENCHMARK_EXPORT
void AddCustomContext(std::string key, std::string value);

struct ThreadRunnerBase {
  virtual ~ThreadRunnerBase() {}
  virtual void RunThreads(const std::function<void(int)>& fn) = 0;
};

using threadrunner_factory =
    std::function<std::unique_ptr<ThreadRunnerBase>(int)>;

namespace internal {
class BenchmarkFamilies;
class BenchmarkInstance;
}  // namespace internal

class BENCHMARK_EXPORT Benchmark {
 public:
  virtual ~Benchmark();

  Benchmark* Name(const std::string& name);
  Benchmark* Arg(int64_t x);
  Benchmark* Unit(TimeUnit unit);
  Benchmark* Range(int64_t start, int64_t limit);
  Benchmark* DenseRange(int64_t start, int64_t limit, int step = 1);
  Benchmark* Args(const std::vector<int64_t>& args);
  Benchmark* ArgPair(int64_t x, int64_t y) {
    std::vector<int64_t> args;
    args.push_back(x);
    args.push_back(y);
    return Args(args);
  }
  Benchmark* Ranges(const std::vector<std::pair<int64_t, int64_t>>& ranges);
  Benchmark* ArgsProduct(const std::vector<std::vector<int64_t>>& arglists);
  Benchmark* ArgName(const std::string& name);
  Benchmark* ArgNames(const std::vector<std::string>& names);
  Benchmark* RangePair(int64_t lo1, int64_t hi1, int64_t lo2, int64_t hi2) {
    std::vector<std::pair<int64_t, int64_t>> ranges;
    ranges.push_back(std::make_pair(lo1, hi1));
    ranges.push_back(std::make_pair(lo2, hi2));
    return Ranges(ranges);
  }
  Benchmark* Setup(callback_function&&);
  Benchmark* Setup(const callback_function&);
  Benchmark* Teardown(callback_function&&);
  Benchmark* Teardown(const callback_function&);
  Benchmark* Apply(const std::function<void(Benchmark* benchmark)>&);
  Benchmark* RangeMultiplier(int multiplier);
  Benchmark* MinTime(double t);
  Benchmark* MinWarmUpTime(double t);
  Benchmark* Iterations(IterationCount n);
  Benchmark* Repetitions(int n);
  Benchmark* ReportAggregatesOnly(bool value = true);
  Benchmark* DisplayAggregatesOnly(bool value = true);
  Benchmark* MeasureProcessCPUTime();
  Benchmark* UseRealTime();
  Benchmark* UseManualTime();
  Benchmark* Complexity(BigO complexity = benchmark::oAuto);
  Benchmark* Complexity(BigOFunc* complexity);
  Benchmark* ComputeStatistics(const std::string& name,
                               StatisticsFunc* statistics,
                               StatisticUnit unit = kTime);
  Benchmark* Threads(int t);
  Benchmark* ThreadRange(int min_threads, int max_threads);
  Benchmark* DenseThreadRange(int min_threads, int max_threads, int stride = 1);
  Benchmark* ThreadPerCpu();
  Benchmark* ThreadRunner(threadrunner_factory&& factory);

  virtual void Run(State& state) = 0;

  TimeUnit GetTimeUnit() const;

 protected:
  explicit Benchmark(const std::string& name);
  void SetName(const std::string& name);

 public:
  const char* GetName() const;
  int ArgsCnt() const;
  const char* GetArgName(int arg) const;

 private:
  friend class internal::BenchmarkFamilies;
  friend class internal::BenchmarkInstance;

  std::string name_;
  internal::AggregationReportMode aggregation_report_mode_;
  std::vector<std::string> arg_names_;
  std::vector<std::vector<int64_t>> args_;

  TimeUnit time_unit_;
  bool use_default_time_unit_;

  int range_multiplier_;
  double min_time_;
  double min_warmup_time_;
  IterationCount iterations_;
  int repetitions_;
  bool measure_process_cpu_time_;
  bool use_real_time_;
  bool use_manual_time_;
  BigO complexity_;
  BigOFunc* complexity_lambda_;
  std::vector<internal::Statistics> statistics_;
  std::vector<int> thread_counts_;

  callback_function setup_;
  callback_function teardown_;

  threadrunner_factory threadrunner_;

  BENCHMARK_DISALLOW_COPY_AND_ASSIGN(Benchmark);
};

namespace internal {
typedef BENCHMARK_DEPRECATED_MSG(
    "Use ::benchmark::Benchmark instead")::benchmark::Benchmark Benchmark;
typedef BENCHMARK_DEPRECATED_MSG(
    "Use ::benchmark::threadrunner_factory instead")::benchmark::
    threadrunner_factory threadrunner_factory;

typedef void(Function)(State&);

BENCHMARK_EXPORT ::benchmark::Benchmark* RegisterBenchmarkInternal(
    std::unique_ptr<::benchmark::Benchmark>);
BENCHMARK_EXPORT std::map<std::string, std::string>*& GetGlobalContext();
BENCHMARK_EXPORT void UseCharPointer(char const volatile*);
BENCHMARK_EXPORT int InitializeStreams();
BENCHMARK_UNUSED static int stream_init_anchor = InitializeStreams();
}  // namespace internal

Benchmark* RegisterBenchmark(const std::string& name, internal::Function* fn);

template <class Lambda>
Benchmark* RegisterBenchmark(const std::string& name, Lambda&& fn);

BENCHMARK_EXPORT void ClearRegisteredBenchmarks();

namespace internal {
class BENCHMARK_EXPORT FunctionBenchmark : public benchmark::Benchmark {
 public:
  FunctionBenchmark(const std::string& name, Function* func)
      : Benchmark(name), func_(func) {}
  void Run(State& st) override;

 private:
  Function* func_;
};

template <class Lambda>
class LambdaBenchmark : public benchmark::Benchmark {
 public:
  void Run(State& st) override { lambda_(st); }
  template <class OLambda>
  LambdaBenchmark(const std::string& name, OLambda&& lam)
      : Benchmark(name), lambda_(std::forward<OLambda>(lam)) {}

 private:
  LambdaBenchmark(LambdaBenchmark const&) = delete;
  Lambda lambda_;
};
}  // namespace internal

inline Benchmark* RegisterBenchmark(const std::string& name,
                                    internal::Function* fn) {
  return internal::RegisterBenchmarkInternal(
      ::benchmark::internal::make_unique<internal::FunctionBenchmark>(name,
                                                                      fn));
}

template <class Lambda>
Benchmark* RegisterBenchmark(const std::string& name, Lambda&& fn) {
  using BenchType =
      internal::LambdaBenchmark<typename std::decay<Lambda>::type>;
  return internal::RegisterBenchmarkInternal(
      ::benchmark::internal::make_unique<BenchType>(name,
                                                    std::forward<Lambda>(fn)));
}

template <class Lambda, class... Args>
Benchmark* RegisterBenchmark(const std::string& name, Lambda&& fn,
                             Args&&... args) {
  return benchmark::RegisterBenchmark(
      name, [=](benchmark::State& st) { fn(st, args...); });
}

class Fixture : public Benchmark {
 public:
  Fixture() : Benchmark("") {}
  void Run(State& st) override {
    this->SetUp(st);
    this->BenchmarkCase(st);
    this->TearDown(st);
  }
  virtual void SetUp(const State&) {}
  virtual void TearDown(const State&) {}
  virtual void SetUp(State& st) { SetUp(const_cast<const State&>(st)); }
  virtual void TearDown(State& st) { TearDown(const_cast<const State&>(st)); }

 protected:
  virtual void BenchmarkCase(State&) = 0;
};

BENCHMARK_EXPORT
std::vector<int64_t> CreateRange(int64_t lo, int64_t hi, int multi);

BENCHMARK_EXPORT
std::vector<int64_t> CreateDenseRange(int64_t start, int64_t limit, int step);

}  // namespace benchmark

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif  // BENCHMARK_BENCHMARK_API_H_
