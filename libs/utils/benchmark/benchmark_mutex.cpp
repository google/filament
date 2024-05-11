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

#include "PerformanceCounters.h"

#include <utils/Allocator.h>
#include <utils/Mutex.h>

#include <benchmark/benchmark.h>

using namespace utils;

static void BM_std_mutex(benchmark::State& state) {
    static std::mutex l;
    PerformanceCounters pc(state);
    for (auto _ : state) {
        l.lock();
        l.unlock();
    }
}

static void BM_utils_mutex(benchmark::State& state) {
    static Mutex l;
    PerformanceCounters pc(state);
    for (auto _ : state) {
        l.lock();
        l.unlock();
    }
}

BENCHMARK(BM_std_mutex)
    ->Threads(1)
    ->Threads(2)
    ->Threads(8)
    ->ThreadPerCpu();

BENCHMARK(BM_utils_mutex)
    ->Threads(1)
    ->Threads(2)
    ->Threads(8)
    ->ThreadPerCpu();
