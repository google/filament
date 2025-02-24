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

#include "marl/defer.h"
#include "marl/scheduler.h"
#include "marl/thread.h"
#include "marl/ticket.h"

#include "benchmark/benchmark.h"

BENCHMARK_DEFINE_F(Schedule, Ticket)(benchmark::State& state) {
  run(state, [&](int numTasks) {
    for (auto _ : state) {
      marl::Ticket::Queue queue;
      for (int i = 0; i < numTasks; i++) {
        auto ticket = queue.take();
        marl::schedule([ticket] {
          ticket.wait();
          ticket.done();
        });
      }
      queue.take().wait();
    }
  });
}
BENCHMARK_REGISTER_F(Schedule, Ticket)->Apply(Schedule::args<512>);
