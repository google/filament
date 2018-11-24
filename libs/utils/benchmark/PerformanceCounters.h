/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_UTILS_BENCHMARK_PEROFRMANCECOUNTERS_H
#define TNT_UTILS_BENCHMARK_PEROFRMANCECOUNTERS_H

#include <benchmark/benchmark.h>

#include <utils/Profiler.h>

class PerformanceCounters {
    benchmark::State& state;
    utils::Profiler profiler;
    utils::Profiler::Counters counters{};

public:
    explicit PerformanceCounters(benchmark::State& state)
            : state(state) {
        profiler.resetEvents(utils::Profiler::EV_CPU_CYCLES | utils::Profiler::EV_BPU_MISSES);
        profiler.start();
    }
    ~PerformanceCounters() {
        profiler.stop();
        counters = profiler.readCounters();
        if (profiler.isValid()) {
            state.counters.insert({
                    { "C",   { (double)counters.getCpuCycles(),    benchmark::Counter::kAvgIterations }},
                    { "I",   { (double)counters.getInstructions(), benchmark::Counter::kAvgIterations }},
                    { "BPU", { (double)counters.getBranchMisses(), benchmark::Counter::kAvgIterations }},
                    { "CPI", { (double)counters.getCPI(),          benchmark::Counter::kAvgThreads }},
            });
        }
    }
};

#endif //TNT_UTILS_BENCHMARK_PEROFRMANCECOUNTERS_H
