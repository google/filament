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

#ifndef BENCHMARK_STATISTICS_H_
#define BENCHMARK_STATISTICS_H_

#include <string>
#include <vector>

#include "benchmark/types.h"

namespace benchmark {

enum BigO { oNone, o1, oN, oNSquared, oNCubed, oLogN, oNLogN, oAuto, oLambda };

typedef int64_t ComplexityN;

enum StatisticUnit { kTime, kPercentage };

typedef double(BigOFunc)(ComplexityN);

typedef double(StatisticsFunc)(const std::vector<double>&);

namespace internal {
struct Statistics {
  std::string name_;
  StatisticsFunc* compute_;
  StatisticUnit unit_;

  Statistics(const std::string& name, StatisticsFunc* compute,
             StatisticUnit unit = kTime)
      : name_(name), compute_(compute), unit_(unit) {}
};

enum AggregationReportMode : unsigned {
  ARM_Unspecified = 0,
  ARM_Default = 1U << 0U,
  ARM_FileReportAggregatesOnly = 1U << 1U,
  ARM_DisplayReportAggregatesOnly = 1U << 2U,
  ARM_ReportAggregatesOnly =
      ARM_FileReportAggregatesOnly | ARM_DisplayReportAggregatesOnly
};

enum Skipped : unsigned {
  NotSkipped = 0,
  SkippedWithMessage,
  SkippedWithError
};

}  // namespace internal

}  // namespace benchmark

#endif  // BENCHMARK_STATISTICS_H_
