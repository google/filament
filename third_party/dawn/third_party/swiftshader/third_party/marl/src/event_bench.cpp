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

#include "marl/containers.h"
#include "marl/event.h"

#include "benchmark/benchmark.h"

BENCHMARK_DEFINE_F(Schedule, Event)(benchmark::State& state) {
  run(state, [&](int numTasks) {
    for (auto _ : state) {
      marl::containers::vector<marl::Event, 1> events;
      events.resize(numTasks + 1);
      for (auto i = 0; i < numTasks; i++) {
        marl::Event prev = events[i];
        marl::Event next = events[i + 1];
        marl::schedule([=] {
          prev.wait();
          next.signal();
        });
      }
      events.front().signal();
      events.back().wait();
    }
  });
}
BENCHMARK_REGISTER_F(Schedule, Event)->Apply(Schedule::args<512>);

// EventBaton benchmarks alternating execution of two tasks.
BENCHMARK_DEFINE_F(Schedule, EventBaton)(benchmark::State& state) {
  run(state, [&](int numPasses) {
    for (auto _ : state) {
      marl::Event passToA(marl::Event::Mode::Auto);
      marl::Event passToB(marl::Event::Mode::Auto);
      marl::Event done(marl::Event::Mode::Auto);

      marl::schedule(marl::Task(
          [=] {
            for (int i = 0; i < numPasses; i++) {
              passToA.wait();
              passToB.signal();
            }
          },
          marl::Task::Flags::SameThread));

      marl::schedule(marl::Task(
          [=] {
            for (int i = 0; i < numPasses; i++) {
              passToB.wait();
              passToA.signal();
            }
            done.signal();
          },
          marl::Task::Flags::SameThread));

      passToA.signal();
      done.wait();
    }
  });
}
BENCHMARK_REGISTER_F(Schedule, EventBaton)->Apply(Schedule::args<262144>);
