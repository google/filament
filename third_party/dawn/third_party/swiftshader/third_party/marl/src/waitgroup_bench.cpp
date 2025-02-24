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

#include "marl_bench.h"

#include "marl/waitgroup.h"

BENCHMARK_DEFINE_F(Schedule, WaitGroup)(benchmark::State& state) {
  run(state, [&](int numTasks) {
    for (auto _ : state) {
      marl::WaitGroup wg;
      wg.add(numTasks);
      for (auto i = 0; i < numTasks; i++) {
        marl::schedule([=] { wg.done(); });
      }
      wg.wait();
    }
  });
}
BENCHMARK_REGISTER_F(Schedule, WaitGroup)->Apply(Schedule::args);
