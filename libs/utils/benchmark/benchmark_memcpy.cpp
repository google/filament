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

#include <benchmark/benchmark.h>


static void BM_memcpy(benchmark::State& state) {
    char* src = new char[state.range(0)];
    char* dst = new char[state.range(0)];
    memset(src, 'x', (size_t)state.range(0));

    {
        PerformanceCounters pc(state);
        for (auto _ : state) {
            memcpy(dst, src, (size_t)state.range(0));
            benchmark::DoNotOptimize(dst);
            benchmark::DoNotOptimize(src);
            benchmark::ClobberMemory();
        }
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));

    delete[] src;
    delete[] dst;
}

BENCHMARK(BM_memcpy)->Range(8, 8192<<10)->Threads(1)->Threads(8);
