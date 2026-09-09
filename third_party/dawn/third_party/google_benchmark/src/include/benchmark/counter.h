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

#ifndef BENCHMARK_COUNTER_H_
#define BENCHMARK_COUNTER_H_

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include <map>
#include <string>

#include "benchmark/macros.h"
#include "benchmark/types.h"

namespace benchmark {

class BENCHMARK_EXPORT Counter {
 public:
  enum Flags {
    kDefaults = 0,
    kIsRate = 1 << 0,
    kAvgThreads = 1 << 1,
    kAvgThreadsRate = kIsRate | kAvgThreads,
    kIsIterationInvariant = 1 << 2,
    kIsIterationInvariantRate = kIsRate | kIsIterationInvariant,
    kAvgIterations = 1 << 3,
    kAvgIterationsRate = kIsRate | kAvgIterations,
    kInvert = 1 << 31
  };

  enum OneK { kIs1000 = 1000, kIs1024 = 1024 };

  double value;
  Flags flags;
  OneK oneK;

  BENCHMARK_ALWAYS_INLINE
  Counter(double v = 0., Flags f = kDefaults, OneK k = kIs1000)
      : value(v), flags(f), oneK(k) {}

  BENCHMARK_ALWAYS_INLINE operator double const&() const { return value; }
  BENCHMARK_ALWAYS_INLINE operator double&() { return value; }
};

Counter::Flags inline operator|(const Counter::Flags& LHS,
                                const Counter::Flags& RHS) {
  return static_cast<Counter::Flags>(static_cast<int>(LHS) |
                                     static_cast<int>(RHS));
}

using UserCounters = std::map<std::string, Counter>;

namespace internal {
void Finish(UserCounters* l, IterationCount iterations, double cpu_time,
            double num_threads);
void Increment(UserCounters* l, UserCounters const& r);
bool SameNames(UserCounters const& l, UserCounters const& r);
}  // namespace internal

}  // namespace benchmark

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif  // BENCHMARK_COUNTER_H_
