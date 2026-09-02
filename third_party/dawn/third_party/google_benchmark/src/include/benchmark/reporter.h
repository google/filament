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

#ifndef BENCHMARK_REPORTER_H_
#define BENCHMARK_REPORTER_H_

#if defined(_MSC_VER)
#pragma warning(push)
// C4251: <symbol> needs to have dll-interface to be used by clients of class
#pragma warning(disable : 4251)
#endif

#include <cassert>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "benchmark/counter.h"
#include "benchmark/macros.h"
#include "benchmark/managers.h"
#include "benchmark/statistics.h"
#include "benchmark/sysinfo.h"
#include "benchmark/types.h"

namespace benchmark {

struct BENCHMARK_EXPORT BenchmarkName {
  std::string function_name;
  std::string args;
  std::string min_time;
  std::string min_warmup_time;
  std::string iterations;
  std::string repetitions;
  std::string time_type;
  std::string threads;

  std::string str() const;
};

class BENCHMARK_EXPORT BenchmarkReporter {
 public:
  struct Context {
    CPUInfo const& cpu_info;
    SystemInfo const& sys_info;
    size_t name_field_width = 0;
    static const char* executable_name;
    Context();
  };

  struct BENCHMARK_EXPORT Run {
    static const int64_t no_repetition_index = -1;
    enum RunType { RT_Iteration, RT_Aggregate };

    Run()
        : run_type(RT_Iteration),
          aggregate_unit(kTime),
          skipped(internal::NotSkipped),
          iterations(1),
          threads(1),
          time_unit(kNanosecond),
          real_accumulated_time(0),
          cpu_accumulated_time(0),
          max_heapbytes_used(0),
          use_real_time_for_initial_big_o(false),
          complexity(oNone),
          complexity_lambda(),
          complexity_n(0),
          statistics(),
          report_big_o(false),
          report_rms(false),
          allocs_per_iter(0.0) {}

    std::string benchmark_name() const;
    BenchmarkName run_name;
    int64_t family_index;
    int64_t per_family_instance_index;
    RunType run_type;
    std::string aggregate_name;
    StatisticUnit aggregate_unit;
    std::string report_label;
    internal::Skipped skipped;
    std::string skip_message;

    IterationCount iterations;
    int64_t threads;
    int64_t repetition_index;
    int64_t repetitions;
    TimeUnit time_unit;
    double real_accumulated_time;
    double cpu_accumulated_time;

    double GetAdjustedRealTime() const;
    double GetAdjustedCPUTime() const;

    double max_heapbytes_used;
    bool use_real_time_for_initial_big_o;
    BigO complexity;
    BigOFunc* complexity_lambda;
    ComplexityN complexity_n;
    const std::vector<internal::Statistics>* statistics;
    bool report_big_o;
    bool report_rms;
    UserCounters counters;
    MemoryManager::Result memory_result;
    double allocs_per_iter;
  };

  struct PerFamilyRunReports {
    PerFamilyRunReports() : num_runs_total(0), num_runs_done(0) {}
    int num_runs_total;
    int num_runs_done;
    std::vector<BenchmarkReporter::Run> Runs;
  };

  BenchmarkReporter();
  virtual bool ReportContext(const Context& context) = 0;
  virtual void ReportRunsConfig(double /*min_time*/,
                                bool /*has_explicit_iters*/,
                                IterationCount /*iters*/) {}
  virtual void ReportRuns(const std::vector<Run>& report) = 0;
  virtual void Finalize() {}

  void SetOutputStream(std::ostream* out) {
    assert(out);
    output_stream_ = out;
  }
  void SetErrorStream(std::ostream* err) {
    assert(err);
    error_stream_ = err;
  }
  std::ostream& GetOutputStream() const { return *output_stream_; }
  std::ostream& GetErrorStream() const { return *error_stream_; }
  virtual ~BenchmarkReporter();
  static void PrintBasicContext(std::ostream* out, Context const& context);

 private:
  std::ostream* output_stream_;
  std::ostream* error_stream_;
};

class BENCHMARK_EXPORT ConsoleReporter : public BenchmarkReporter {
 public:
  enum OutputOptions {
    OO_None = 0,
    OO_Color = 1,
    OO_Tabular = 2,
    OO_ColorTabular = OO_Color | OO_Tabular,
    OO_Defaults = OO_ColorTabular
  };
  explicit ConsoleReporter(OutputOptions opts_ = OO_Defaults)
      : output_options_(opts_), name_field_width_(0), printed_header_(false) {}

  bool ReportContext(const Context& context) override;
  void ReportRuns(const std::vector<Run>& reports) override;

 protected:
  virtual void PrintRunData(const Run& result);
  virtual void PrintHeader(const Run& run);

  OutputOptions output_options_;
  size_t name_field_width_;
  UserCounters prev_counters_;
  bool printed_header_;
};

class BENCHMARK_EXPORT JSONReporter : public BenchmarkReporter {
 public:
  JSONReporter() : first_report_(true) {}
  bool ReportContext(const Context& context) override;
  void ReportRuns(const std::vector<Run>& reports) override;
  void Finalize() override;

 private:
  void PrintRunData(const Run& run);
  bool first_report_;
};

class BENCHMARK_EXPORT BENCHMARK_DEPRECATED_MSG(
    "The CSV Reporter will be removed in a future release") CSVReporter
    : public BenchmarkReporter {
 public:
  CSVReporter() : printed_header_(false) {}
  bool ReportContext(const Context& context) override;
  void ReportRuns(const std::vector<Run>& reports) override;

 private:
  void PrintRunData(const Run& run);
  bool printed_header_;
  std::set<std::string> user_counter_names_;
};

inline const char* GetTimeUnitString(TimeUnit unit) {
  switch (unit) {
    case kSecond:
      return "s";
    case kMillisecond:
      return "ms";
    case kMicrosecond:
      return "us";
    case kNanosecond:
      return "ns";
  }
  BENCHMARK_UNREACHABLE();
}

inline double GetTimeUnitMultiplier(TimeUnit unit) {
  switch (unit) {
    case kSecond:
      return 1;
    case kMillisecond:
      return 1e3;
    case kMicrosecond:
      return 1e6;
    case kNanosecond:
      return 1e9;
  }
  BENCHMARK_UNREACHABLE();
}

}  // namespace benchmark

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif  // BENCHMARK_REPORTER_H_
