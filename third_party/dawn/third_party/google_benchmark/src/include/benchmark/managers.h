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

#ifndef BENCHMARK_MANAGERS_H_
#define BENCHMARK_MANAGERS_H_

#include <stdint.h>

#include <limits>

#include "benchmark/macros.h"
#include "benchmark/types.h"

namespace benchmark {

class MemoryManager {
 public:
  static constexpr int64_t TombstoneValue = std::numeric_limits<int64_t>::max();

  struct Result {
    Result()
        : num_allocs(0),
          max_bytes_used(0),
          total_allocated_bytes(TombstoneValue),
          net_heap_growth(TombstoneValue),
          memory_iterations(0) {}

    int64_t num_allocs;
    int64_t max_bytes_used;
    int64_t total_allocated_bytes;
    int64_t net_heap_growth;
    IterationCount memory_iterations;
  };

  virtual ~MemoryManager() {}
  virtual void Start() = 0;
  virtual void Stop(Result& result) = 0;
};

BENCHMARK_EXPORT
void RegisterMemoryManager(MemoryManager* memory_manager);

class ProfilerManager {
 public:
  virtual ~ProfilerManager() {}
  virtual void AfterSetupStart() = 0;
  virtual void BeforeTeardownStop() = 0;
};

BENCHMARK_EXPORT
void RegisterProfilerManager(ProfilerManager* profiler_manager);

}  // namespace benchmark

#endif  // BENCHMARK_MANAGERS_H_
