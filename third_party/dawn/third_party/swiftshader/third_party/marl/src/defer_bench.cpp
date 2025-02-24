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

#include "marl/defer.h"

#include "benchmark/benchmark.h"

volatile int do_not_optimize_away_result = 0;

static void Defer(benchmark::State& state) {
  for (auto _ : state) {
    // Avoid benchmark::DoNotOptimize() as this is unfairly slower on Windows.
    defer(do_not_optimize_away_result++);
  }
}
BENCHMARK(Defer);
